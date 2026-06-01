#include "vm/machine.hpp"

#include "core/bytecode/vm_program.hpp"
#include "L2_semantic_subsystems/core/expr_maker.hpp"
#include "vm/vm_memory.hpp"
#include "support/format_adapter.hpp"
#include "support/debug_tools.hpp"
#include "support/string_tools.hpp"

namespace alpha::vm
{
Machine::Stack::Stack(const u64 size, const std::function<void()> on_stack_overflow)
    : on_stack_overflow_(on_stack_overflow),
      size_(size)
{
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
    if (!top_)
        on_stack_overflow_();
    else
        --top_;
}

void
Machine::Stack::push_env_value(const AlphaInt value)
{
    Memcell& cell = data_[top_];
    cell.type = Memcell::Type::INT;
    cell.data.int_value = value;
    decrease_top();
}


Machine::Machine(const Bytes stack_size)
    : stack_(stack_size.count, [this]() { on_stack_overflow(); }) {}

void Machine::on_stack_overflow()
{
    execution_finished_.raise();
    error("StackOverflow");
}


Memcell*
Machine::translate_operand(const vm::Argument* const arg, Memcell* const reg)
{
    DMASSERT(!!arg);

    DMASSERT((
            arg->type != Argument::Type::GLOBAL &&
            arg->type != Argument::Type::LOCAL &&
            arg->type != Argument::Type::FORMAL &&
            arg->type != Argument::Type::RETVAL) || reg
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
    case Argument::Type::CONST_NIL:
        reg->type = Memcell::Type::NIL;
        return reg;
    case Argument::Type::PROGRAMFUNC:
        reg->type = Memcell::Type::PROGFUNC;
        reg->data.progfunc_address = static_cast<const ProgramFuncArgument*>(arg)->address.value;
        return reg;
    case Argument::Type::LIBFUNC:
        reg->type = Memcell::Type::LIBFUNC;
        return reg;
    case Argument::Type::CONST_STRING:
        reg->type = Memcell::Type::STRING;
        #warning "Fetch string from the string map"
        // reg->data.str_value = strdup(
        //     static_cast<const ConstStringArgument*>(arg)->pool_index
        // );
    case Argument::Type::LABEL:
        DMASSERT(false && "Label addresses aren't mapped to memcells. they are accessed in place.");
        std::abort();
    default: DMASSERT(false);
    }
}

void
Machine::display_warning(const std::string& message) { *err_stream_ << message << std::endl; }

void
Machine::error(const std::string& message) { *err_stream_ << message << std::endl; }

void
Machine::execute_cycle()
{
    if (execution_finished_)
        return;
    if (pc_ == ending_pc())
    {
        execution_finished_.raise();
        return;
    }
    DMASSERT(pc_ < ending_pc());
    vm::Instruction* const instr = code_ + pc_;

    const auto instr_opcode_idx = static_cast<std::underlying_type_t<vm::Opcode>>(instr->opcode);
    DMASSERT(
        instr_opcode_idx >= 0,
        instr_opcode_idx >= static_cast<std::underlying_type_t<vm::Opcode>>(vm::Opcode::__COUNT__)
    );
    const u32 old_pc = pc_;
    (*execute_dispatch_table_[instr_opcode_idx])(instr);
    if (pc_ == old_pc)
        ++pc_;
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
Machine::Stack::add_function_environment(const Program::ProgFunc& func_info)
{
    topsp_ = top_;
    top_ -= func_info.local_size;
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
Machine::Stack::get_actual(u32 idx) const noexcept
{
    DMASSERT(idx < total_actuals());
    return data_[topsp_ + Machine::k_stack_environment_size + 1 + idx];
}


u32
Machine::Stack::restore_previous_environment() noexcept
{
    top_ = get_environment_value(topsp_ + Machine::k_saved_top_offset);
    topsp_ = get_environment_value(topsp_ + Machine::k_saved_topsp_offset);
    const auto restored_pc = top_ = get_environment_value(topsp_ + Machine::k_saved_pc_offset);
    return restored_pc;
}


AlphaInt
Machine::Stack::get_environment_value(const u32 stack_idx) const noexcept
{
    const Memcell env_cell = data_[stack_idx];
    DMASSERT(env_cell.type == Memcell::Type::INT);
    const AlphaInt env_value = env_cell.data.int_value;
    return env_value;
}


void
Machine::execute_assign(vm::Instruction& inst)
{
    DMASSERT(!inst.arg2);
    Memcell* const lv = DEBUG_REQUIRE_PTR(translate_operand(inst.result, nullptr));
    const Memcell* const rv = DEBUG_REQUIRE_PTR(translate_operand(inst.arg1, &reg_a_));
    #warning "TODO The assertion from lecturen 15 , slide 18"
    assign(*lv, *rv);
}

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
// void
// Machine::execute_call(vm::Instruction& inst)
// {
//     Memcell* func = DEBUG_REQUIRE_PTR(translate_operand(inst.arg1, &reg_a_));
//     switch (func->type)
//     {
//     case Memcell::Type::PROGFUNC:
//         save_call_environment();
//         pc_ = func->data.progfunc_address;
//         DMASSERT(pc_ < ending_pc(), code_[pc_].opcode == Opcode::ENTERFUNC);
//         break;
//     case Memcell::Type::STRING:
//         call_libfunc(func->data.str_value);
//         break;
//     case Memcell::Type::LIBFUNC:
//         call_libfunc(func->data.libfunc_name);
//         break;
//     case Memcell::Type::TABLE:
//         call_functor(func->data.table_value);
//         break;
//
//     default:
//         error("can not bind to function");
//         execution_finished_.raise();
//     }
// }
//
// void
// Machine::execute_funcenter(vm::Instruction* inst)
// {
//     vm::Memcell* const func = DEBUG_REQUIRE_PTR(translate_operand(inst->result, &reg_a_));
//     DMASSERT(pc_ == func->data.progfunc_address);
//     total_actuals_ = 0;
//     Program::ProgFunc func_info = get_func_info(pc_);
// }
//
// void
// Machine::execute_funcexit([[maybe_unused]] vm::Instruction* const unused)
// {
//     const auto old_top = stack_.top();
//     pc_ = stack_.restore_previous_environment();
//
//     // Cleanup activation_record:
//     auto idx = old_top;
//     while (++idx <= stack_.top())
//         stack_.clear_at(idx);
// }
//
// void
// Machine::execute_pusharg(vm::Instruction* inst)
// {
//     const vm::Memcell& arg = *DEBUG_REQUIRE_PTR(translate_operand(inst->arg1, &reg_a_));
//     assign(stack_.top_element(), arg);
//     ++total_actuals_;
//     stack_.decrease_top();
// }
//
// template <typename Op>
//     requires std::is_invocable_v<Op, AlphaInt, AlphaInt>
// AlphaInt make_int_op(AlphaInt lhs, AlphaInt rhs) { return Op{}(lhs, rhs); }
//
//
// void
// Machine::set_out_stream(std::ostream& out) noexcept { out_stream_ = &out; }
//
//
// void
// Machine::call_libfunc(const char* const id)
// {
//     const auto func = get_libfunc();
//     if (!func)
//     {
//         error(FMT::format("Unsupported lib func {} called!", id));
//         execution_finished_.raise();
//         return;
//     }
//     stack_.save_call_environment(pc_, total_actuals_);
//     topsp_ = top_;
//     total_actuals_ = 0;
//     (func)();                      // Call libfunc
//     if (!execution_finished_)      // An error may occur naturally.
//         execute_funcexit(nullptr); // Return Sequence.
// }
//
void
Machine::impl_of_libfunc_print()
{
    const AlphaInt n = stack_.total_actuals();
    DMASSERT(n >=0 && "makes no sense");
    for (AlphaInt i = 0; i < n; ++i)
        *out_stream_ << stack_.get_actual(i).to_string();
}
} // namespace alpha::vm
