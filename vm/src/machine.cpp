#include "vm/machine.hpp"

#include "vm/vm_memcell.hpp"
#include "support/format_adapter.hpp"
#include "support/debug_tools.hpp"

namespace alpha::vm
{
Machine::Stack::Stack(const u64 size)
    : size_(size)
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

Machine::Machine(const Bytes stack_size)
    : stack_(stack_size.count) {}

const Memcell*
Machine::translate_operand(const vm::Argument* const arg, Memcell* const reg)
{
    switch (arg->type)
    {
    case Argument::Type::GLOBAL:
    case Argument::Type::LOCAL:
    case Argument::Type::FORMAL: DMASSERT(false);
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
        reg->data.progfunc_address = static_cast<const ProgramFuncArgument*>(arg)->address;
        return reg;
    case Argument::Type::LIBFUNC:
        reg->type = Memcell::Type::LIBFUNC;
        return reg;
    default: DMASSERT(false);
    }
}

void
Machine::execute_cycle()
{
    if (execution_finished_)
        return;
    if (pc_ == ending_pc())
    {
        execution_finished_.enable();
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
Machine::assign(Memcell& lv, Memcell& rv)
{
    if (&lv == &rv)
        return; // Ignoring self-assignment
    if (lv.type == Memcell::Type::TABLE &&
        rv.type == Memcell::Type::TABLE &&
        lv.data.table_value == rv.data.table_value
    ) { return; } // Ignoring self-assignment for tables.

    if (rv.type == Memcell::Type::UNDEF)
    {
        #warning "Add warning system"
        std::cerr << "Assigning from `UNDEF` content." << std::endl;
    }

    lv.clear();
    std::memcpy(&lv, &rv, sizeof(Memcell));

    DMASSERT(false && "implement content form slide 19 (lecture 15 )");

}
} // namespace alpha::vm
