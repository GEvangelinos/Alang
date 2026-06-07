#include "vm/machine.hpp"

#include <alpha_parser.gen.hpp>

#include "bytecode/executable.hpp"
#include "core/bytecode/vm_program.hpp"
#include "L2_semantic_subsystems/core/expr_maker.hpp"
#include "vm/vm_memory.hpp"
#include "support/format_adapter.hpp"
#include "support/debug_tools.hpp"

namespace alpha::vm
{
Machine::Stack::Stack(
    const u64 size,
    const u32 global_var_count,
    const std::function<void()> on_stack_overflow)
    : on_stack_overflow_(on_stack_overflow),
      size_(size)
{
    const u32 base_offset = size - global_var_count - 1;
    top_ = topsp_ = base_offset;

    if (size_ <= 0)
        throw std::logic_error(FMT::format("Invalid size `{}`, positive size required", size_));
    data_ = std::make_unique<Memcell []>(size_);
    if (!data_)
        throw std::runtime_error("Failed initializing stack");

    const auto validate = [&]() -> bool
    {
        constexpr decltype(Memcell::data) zero_data = {};
        for (u64 i = 0; i < size_; ++i)
        {
            const auto& cell = data_[i];
            if (cell.type != Memcell::Type::UNDEF)
                return false;
            if (std::memcmp(&cell.data, &zero_data, sizeof(zero_data)))
                return false;
        }
        return true;
    };
    DMASSERT(validate());
}


Memcell& Machine::Stack::operator[](const u32 idx) noexcept
{
    DMASSERT(idx < size_);
    return data_[idx];
}

const Memcell& Machine::Stack::operator[](const u32 idx) const noexcept
{
    return const_cast<Stack&>(*this)[idx];
}

void
Machine::Stack::decrease_top()
{
    DMASSERT(!is_overflowed && "LogicError: Stack is overflowed yet we still call decrease_top()");
    if (top_ > 0) [[likely]]
        --top_;
    else
        on_stack_overflow_();
}

void
Machine::Stack::push_env_value(const AlphaInt value)
{
    Memcell& cell = data_[top_];
    cell.type = Memcell::Type::INT;
    cell.data.int_value = value;
    decrease_top();
}

void
Machine::Stack::enter_frame()
{
    // Both native and libfuncs use this to establish the base of the new frame
    topsp_ = top_;
}

void Machine::Stack::allocate_locals(const u32 count)
{
    // Only native functions call this to reserve space for their bytecode variables
    // Libfuncs pass 0 implicitly by simply never calling this
    if (count > top_) [[unlikely]]
        on_stack_overflow_();
    else
        top_ -= count;
}


Machine::Machine(const Bytes stack_size, const vm::Executable& exe)
    : stack_(stack_size.count, exe.global_var_count, [this]() { on_stack_overflow(); }),
      code_(exe.instructions.data()),
      exe_(exe),
      ending_pc_(exe.instructions.size()) {}

void Machine::on_stack_overflow()
{
    error("StackOverflow");
    throw std::runtime_error{"Stack Overflow occurred"};
}


Memcell*
Machine::translate_operand(const vm::Argument* const arg, Memcell* const reg)
{
    DMASSERT(!!arg);

    DMASSERT(
        arg->type == Argument::Type::LOCAL ||
        arg->type == Argument::Type::FORMAL ||
        arg->type == Argument::Type::GLOBAL ||
        arg->type == Argument::Type::RETVAL ||
        reg
    );
    switch (arg->type)
    {
    case Argument::Type::GLOBAL:
        return &stack_[stack_.size() - 1 - static_cast<const GlobalVariableArgument*>(arg)->offset];
    case Argument::Type::LOCAL:
        return &stack_[stack_.topsp() - static_cast<const LocalVariableArgument*>(arg)->offset];
    case Argument::Type::FORMAL:
        return &stack_[stack_.topsp() + Machine::k_stack_environment_size + 1 + static_cast<const
                           FormalVariableArgument*>(arg)->offset];
    case Argument::Type::RETVAL: return &retval_;
    case Argument::Type::CONST_INT:
        reg->type = Memcell::Type::INT;
        reg->data.int_value = static_cast<const ConstIntArgument*>(arg)->value;
        return reg;
    case Argument::Type::CONST_FLOAT:
        reg->type = Memcell::Type::FLOAT;
        reg->data.int_value = static_cast<const ConstFloatArgument*>(arg)->value;
        return reg;
    case Argument::Type::CONST_BOOL:
        reg->type = Memcell::Type::BOOL;
        reg->data.int_value = static_cast<const ConstBoolArgument*>(arg)->value;
        return reg;
    case Argument::Type::CONST_NIL:
        reg->type = Memcell::Type::NIL;
        return reg;
    case Argument::Type::PROGRAMFUNC:
        reg->type = Memcell::Type::PROGFUNC;
        reg->data.progfunc_index = static_cast<const ProgramFuncArgument*>(arg)->func_idx;
        return reg;
    case Argument::Type::LIBFUNC:
        reg->type = Memcell::Type::LIBFUNC;
        reg->data.int_value = static_cast<LibFuncIdUT>(
            static_cast<const LibFuncArgument*>(arg)->libfunc_id
        );
        return reg;
    case Argument::Type::CONST_STRING:
        {
            reg->type = Memcell::Type::STRING;
            const u32 str_idx = static_cast<const ConstStringArgument*>(arg)->pool_index;
            DMASSERT(str_idx < exe_.str_literal_cache.index_map.size());
            const StringSpan str = exe_.str_literal_cache.index_map[str_idx];
            reg->data.str_value = duplicate_to_cstring(str);
            return reg;
        }
    case Argument::Type::LABEL:
        DMASSERT(false && "Label addresses aren't mapped to memcells. they are accessed in place.");
        std::abort();
    default: DMASSERT(false);
    }
}

void
Machine::display_warning(const std::string& message) { *err_stream_ << message << std::endl; }

void
Machine::error(const std::string& message)
{
    DMASSERT(!execution_finished_ && "We should have stopped at first error");
    *err_stream_ << message << std::endl;
    execution_finished_.raise();
}

void
Machine::execute_cycle()
{
    if (execution_finished_)
        return;
    if (pc_ == exe_.instructions.size())
    {
        execution_finished_.raise();
        return;
    }
    DMASSERT(pc_ < exe_.instructions.size() && "Above,  we just just checked is not equal.");
    const vm::Instruction& instr = exe_.instructions[pc_];

    const auto instr_opcode_idx = static_cast<std::underlying_type_t<vm::Opcode>>(instr.opcode);
    DMASSERT(
        instr_opcode_idx >= 0,
        instr_opcode_idx < static_cast<std::underlying_type_t<vm::Opcode>>(vm::Opcode::__COUNT__)
    );
    const u32 old_pc = pc_;
    void (Machine::*handler)(const Instruction&) = execute_dispatch_table_[instr_opcode_idx];
    (this->*handler)(instr);
    if (pc_ == old_pc)
        ++pc_;
}

void
Machine::run()
{
    try
    {
        while (!execution_finished_)
            execute_cycle();
    }
    catch (const std::runtime_error& e) { std::cerr << e.what() << std::endl; }
}


void
Machine::assign(Memcell& lv, const Memcell& rv)
{
    if (&lv == &rv)
        return; // Ignoring self-assignment
    if (lv.type == Memcell::Type::TABLE &&
        rv.type == Memcell::Type::TABLE &&
        lv.data.table_value == rv.data.table_value
    ) { return; } // Ignoring self-assignment for tables.

    if (rv.type == Memcell::Type::UNDEF)
        display_warning("Assigning from `UNDEF` content");

    lv.clear();
    std::memcpy(&lv, &rv, sizeof(Memcell));

    if (lv.type == Memcell::Type::STRING)
        lv.data.str_value = strdup(rv.data.str_value);
    else if (lv.type == Memcell::Type::TABLE)
        lv.data.table_value->increase_ref();
}

void Machine::Stack::save_call_environment(const AlphaInt pc, const AlphaInt total_actuals)
{
    push_env_value(total_actuals);
    #warning "TODO unbcomment, pass coda as argument to make the check with [[maybe_unused]] "
    // DMASSERT(code[pc].opcode == Opcode::CALL);
    push_env_value(pc + 1);
    push_env_value(top_ + total_actuals + 2);
    push_env_value(topsp_);
}

void
Machine::Stack::add_function_environment(const ProgFuncMetadata& func_info)
{
    enter_frame();
    allocate_locals(func_info.local_count);
}

void
Machine::Stack::clear_at(const u32 idx)
{
    DMASSERT(idx < size_);
    data_[idx].clear();
}


AlphaInt
Machine::Stack::total_actuals() const noexcept
{
    return get_environment_value(topsp_ + Machine::k_actual_count_offset);
}


vm::Memcell&
Machine::Stack::get_actual(const u32 idx) const noexcept
{
    DMASSERT(idx < total_actuals());
    return data_[topsp_ + Machine::k_stack_environment_size + 1 + idx];
}


u32
Machine::Stack::restore_previous_environment() noexcept
{
    top_ = get_environment_value(topsp_ + Machine::k_saved_top_offset);
    const auto restored_pc = get_environment_value(topsp_ + Machine::k_saved_pc_offset);
    topsp_ = get_environment_value(topsp_ + Machine::k_saved_topsp_offset);
    return restored_pc;
}


AlphaInt
Machine::Stack::get_environment_value(const u32 stack_idx) const noexcept
{
    DMASSERT(stack_idx < size());
    const Memcell env_cell = data_[stack_idx];
    DMASSERT(env_cell.type == Memcell::Type::INT);
    const AlphaInt env_value = env_cell.data.int_value;
    return env_value;
}


void
Machine::execute_assign(const vm::Instruction& inst)
{
    DMASSERT(!inst.arg2);
    Memcell* const lv = DEBUG_REQUIRE_PTR(translate_operand(inst.result.get(), nullptr));
    const Memcell* const rv = DEBUG_REQUIRE_PTR(translate_operand(inst.arg1.get(), &reg_a_));
    #warning "TODO The assertion from lecturen 15 , slide 18"
    assign(*lv, *rv);
}

//
// void
// Machine::call_functor(vm::Table* table)
// {
//     reg_c_.type = Memcell::Type::STRING;
//     reg_c_.data.str_value = "()";
//     Memcell* const functor = table_get_element(table, reg_c_);
//     if (!functor)
//         error("In calling table: no `()` element found!");
//     else if (functor->type == Memcell::Type::TABLE)
//         call_functor(functor->data.table_value);
//     else if (functor->type == Memcell::Type::PROGFUNC)
//     {
//         push_table_arg(table);
//         stack_.save_call_environment(pc_, total_actuals_);
//         pc_ = functor->data.progfunc_address;
//         DMASSERT(pc_ < ending_pc(), code_[pc_].opcode == Opcode::ENTERFUNC);
//     }
//     else
//         error("In calling table: illegal `()` element value");
// }
//
//

void
Machine::execute_jump(const vm::Instruction& inst)
{
    DMASSERT(!!inst.result);
    DMASSERT(inst.result->type == Argument::Type::LABEL);
    // @PC_TAG@ -1 is required, as addresses start from 1;
    const auto jump_address = static_cast<const LabelArgument*>(inst.result.get())->value.value - 1;
    DMASSERT(jump_address <= ending_pc_ && "Only jumps can be == ending_pc_ (1 past legal inst)");
    pc_ = jump_address;
}

void
Machine::execute_call(const vm::Instruction& inst)
{
    const Memcell* func = DEBUG_REQUIRE_PTR(translate_operand(inst.arg1.get(), &reg_a_));
    switch (func->type)
    {
    case Memcell::Type::PROGFUNC:
        {
            stack_.save_call_environment(pc_, total_actuals_);
            DMASSERT(func->data.progfunc_index < exe_.progfuncs.size());
            const vm::ProgFuncMetadata metadata = exe_.progfuncs[func->data.progfunc_index];
            DMASSERT(metadata.address.value < exe_.instructions.size());
            pc_ = metadata.address.value - 1; // @PC_TAG@ -1 is required, as addresses start from 1
            DMASSERT(pc_ < ending_pc_, code_[pc_].opcode == Opcode::ENTERFUNC);
            break;
        }
    case Memcell::Type::STRING:
        call_libfunc(func->data.str_value);
        break;
    case Memcell::Type::LIBFUNC:
        call_libfunc(static_cast<LibFuncId>(func->data.int_value));
        break;
    case Memcell::Type::TABLE:
        DMASSERT(false && "uncomment below");
        // call_functor(func->data.table_value);
        break;

    default:
        error("can not bind to function");
        execution_finished_.raise();
    }
}

void
Machine::execute_enterfunc(const vm::Instruction& inst)
{
    const vm::Memcell& func = *DEBUG_REQUIRE_PTR(translate_operand(inst.arg1.get(), &reg_a_));
    const vm::ProgFuncMetadata& progfunc_metadata = exe_.progfuncs[func.data.progfunc_index];
    DMASSERT(pc_ == progfunc_metadata.address.value -1); // @PC_TAG@  (tag is for the - 1 )
    total_actuals_ = 0;
    stack_.enter_frame();
    stack_.allocate_locals(progfunc_metadata.local_count);
}

void
Machine::execute_exitfunc(const vm::Instruction& unused) { execute_exitfunc(&unused); }

void
Machine::execute_exitfunc(const vm::Instruction* const unused)
{
    const auto old_top = stack_.top();
    pc_ = stack_.restore_previous_environment();

    // Cleanup activation record:
    auto idx = old_top;
    while (++idx <= stack_.top()) // Intentionally ignoring first
        stack_.clear_at(idx);
}

void
Machine::execute_pusharg(const vm::Instruction& inst)
{
    const vm::Memcell& arg = *DEBUG_REQUIRE_PTR(translate_operand(inst.arg1.get(), &reg_a_));
    assign(stack_.top_element(), arg);
    ++total_actuals_;
    stack_.decrease_top();
}


#warning "Remove if not USED"
template <typename Op>
    requires std::is_invocable_v<Op, AlphaInt, AlphaInt>
AlphaInt make_int_op(AlphaInt lhs, AlphaInt rhs) { return Op{}(lhs, rhs); }

void
Machine::set_out_stream(std::ostream& out) noexcept { out_stream_ = &out; }

void
Machine::call_libfunc(const char* const libfunc_name)
{
    const std::optional<LibFuncId> libfunc_id =
        get_libfunc_id(StringSpan::from_cstring(libfunc_name));
    if (libfunc_id) [[unlikely]]
        call_libfunc(*libfunc_id);
    else
        error(FMT::format("Unsupported lib func `{}` called", libfunc_name));
}

void
Machine::call_libfunc(const LibFuncId libfunc_id)
{
    void (Machine::*func)() = libfunc_table_[static_cast<LibFuncIdUT>(libfunc_id)];
    if (!func)
    {
        error(FMT::format(
            "Internal error OR corruption. Unknown LibFuncID: `{}`",
            static_cast<LibFuncIdUT>(libfunc_id)
        ));
        execution_finished_.raise();
        return;
    }
    stack_.save_call_environment(pc_, total_actuals_);
    stack_.enter_frame();
    total_actuals_ = 0;
    (this->*func)();               // Call libfunc
    if (!execution_finished_)      // An error may occur naturally.
        execute_exitfunc(nullptr); // Return Sequence.
}


void
Machine::execute_newtable(const vm::Instruction& inst)
{
    vm::Memcell& lv = *DEBUG_REQUIRE_PTR(translate_operand(inst.result.get(), nullptr));
    lv.clear();
    lv.type = Memcell::Type::TABLE;
    lv.data.table_value = new Table{};
    lv.data.table_value->increase_ref();
}

[[nodiscard]] vm::Memcell*
tablegetelem(Table& t, const Memcell& i)
{
    switch (i.type)
    {
    case Memcell::Type::UNDEF:
    case Memcell::Type::NIL: break;
    case Memcell::Type::INT:
        {
            const auto it = t.int_indexed.data.find(i.data.int_value);
            if (it != t.int_indexed.data.end()) [[likely]] return &it->second;
            break;
        }
    case Memcell::Type::FLOAT:
        {
            const auto it = t.float_indexed.data.find(i.data.float_value);
            if (it != t.float_indexed.data.end()) [[likely]] return &it->second;
            break;
        }
    case Memcell::Type::STRING:
        {
            const auto it = t.str_indexed.data.find(i.data.str_value);
            if (it != t.str_indexed.data.end()) [[likely]] return &it->second;
            break;
        }
    case Memcell::Type::BOOL:
        {
            const auto it = t.bool_indexed.data.find(i.data.bool_value);
            if (it != t.bool_indexed.data.end()) [[likely]] return &it->second;
            break;
        }
    case Memcell::Type::TABLE:
        {
            const auto it = t.table_indexed.data.find(i.data.table_value);
            if (it != t.table_indexed.data.end()) [[likely]] return &it->second;
            break;
        }
    case Memcell::Type::PROGFUNC:
        {
            const auto it = t.progfunc_indexed.data.find(i.data.progfunc_index);
            if (it != t.progfunc_indexed.data.end()) [[likely]] return &it->second;
            break;
        }
    case Memcell::Type::LIBFUNC:
        {
            const auto it = t.libfunc_indexed.data.find(i.data.libfunc_id);
            if (it != t.libfunc_indexed.data.end()) [[likely]] return &it->second;
            break;
        }
    default:
        DMASSERT(false);
        std::abort();
    }
    return nullptr;
}

void
tablesetelem(Table& t, const Memcell& i, const Memcell& c)
{
    switch (i.type)
    {
        [[unlikely]] case Memcell::Type::UNDEF:
        UNREACHABLE("VM does not yet support seeting an element with key being `UNDEF`");
        break;
        [[unlikely]] case Memcell::Type::NIL:
        UNREACHABLE("VM does not yet support seeting an element with key being `nil`");
        break;
    case Memcell::Type::INT:
        t.int_indexed.data[i.data.int_value] = c;
        break;
    case Memcell::Type::FLOAT:
        t.float_indexed.data[i.data.float_value] = c;
        break;
    case Memcell::Type::STRING:
        t.str_indexed.data[i.data.str_value] = c;
        break;
    case Memcell::Type::BOOL:
        t.bool_indexed.data[i.data.bool_value] = c;
        break;
    case Memcell::Type::TABLE:
        t.table_indexed.data[i.data.table_value] = c;
        break;
    case Memcell::Type::PROGFUNC:
        t.progfunc_indexed.data[i.data.progfunc_index] = c;
        break;
    case Memcell::Type::LIBFUNC:
        t.libfunc_indexed.data[i.data.libfunc_id] = c;
        break;
    }
}

void
Machine::execute_tablegetelem(const vm::Instruction& inst)
{
    vm::Memcell& lv = *DEBUG_REQUIRE_PTR(translate_operand(inst.result.get(), nullptr));
    const vm::Memcell& t = *DEBUG_REQUIRE_PTR(translate_operand(inst.arg1.get(), nullptr));
    const vm::Memcell& i = *DEBUG_REQUIRE_PTR(translate_operand(inst.arg2.get(), &reg_a_));


    lv.clear();
    lv.type = Memcell::Type::NIL;
    if (t.type != Memcell::Type::TABLE)
    {
        error(FMT::format("Illegal use of type {} as table", to_string(t.type)));
        return;
    }
    // vm or stack must access this (it's the call avm_tablegetlem, in slide 34 )
    const vm::Memcell* const content = tablegetelem(*DEBUG_REQUIRE_PTR(t.data.table_value), i);

    if (content)
        assign(lv, *content);
    else
        display_warning(FMT::format("{}[{}] not found!", t.to_string(), i.to_string()));
}

void Machine::execute_tablesetelem(const vm::Instruction& inst)
{
    const vm::Memcell& t = *DEBUG_REQUIRE_PTR(translate_operand(inst.result.get(), nullptr));
    const vm::Memcell& i = *DEBUG_REQUIRE_PTR(translate_operand(inst.arg1.get(), &reg_a_));
    const vm::Memcell& content = *DEBUG_REQUIRE_PTR(translate_operand(inst.arg2.get(), &reg_b_));

    if (t.type != Memcell::Type::TABLE)
        error(FMT::format("Illegal use of type {} as table", to_string(t.type)));
    else
        tablesetelem(*DEBUG_REQUIRE_PTR(t.data.table_value), i, content);
}

void
Machine::impl_of_libfunc_print()
{
    const AlphaInt n = stack_.total_actuals();
    DMASSERT(n >=0 && "makes no sense");
    for (AlphaInt i = 0; i < n; ++i)
    {
        const vm::Memcell& actual = stack_.get_actual(i);
        *out_stream_ << actual.to_string();
    }
    *out_stream_ << std::endl;
    retval_.type = Memcell::Type::INT;
    retval_.data.int_value = 0;
}

void
Machine::impl_of_libfunc_typeof()
{
    const auto actuals = stack_.total_actuals();
    if (actuals != 1)
    {
        error(FMT::format("one argument (not {}) expected in `typeof` libfunc", actuals));
        return;
    }
    retval_.clear();
    retval_.type = Memcell::Type::STRING;
    retval_.data.str_value = strdup(to_string(stack_.get_actual(0).type));
}
} // namespace alpha::vm
