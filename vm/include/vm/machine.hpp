#ifndef MACHINE_HPP
#define MACHINE_HPP

#include <memory>

#include "vm_memcell.hpp"
#include "core/numeric_types.hpp"
#include "core/bytecode/vm_instruction.hpp"

namespace alpha::vm
{
struct Bytes
{
    u64 count;

    // clang-format off
    static constexpr Bytes from_KB(const u64 kilobytes) noexcept { return {(1ULL << 10) * kilobytes}; }
    static constexpr Bytes from_MB(const u64 megabytes) noexcept { return {(1ULL << 20) * megabytes}; }
    static constexpr Bytes from_GB(const u64 gigabytes) noexcept { return {(1ULL << 30) * gigabytes}; }
    static constexpr Bytes from_TB(const u64 terabytes) noexcept { return {(1ULL << 40) * terabytes}; }
    // clang-format on
};

class Machine
{
public:
    class Stack
    {
    public:
        explicit Stack(u64 size);

        [[nodiscard]] auto size() const noexcept { return size_; }

    private:
        std::unique_ptr<Memcell []> data_;
        u64 size_ = 0;
        u32 top_, topsp_;
    };

    explicit Machine(Bytes stack_size);

    [[nodiscard]] const Memcell* translate_operand(const vm::Argument* arg, Memcell* reg);

    void execute_assign(vm::Instruction& inst);

    void execute_cycle();


private:
    Stack stack_;
    Memcell reg_a_{}, reg_b_{}, reg_c_{}, retval_{};

    ToggleSwitch execution_finished_{ToggleSwitch::State::OFF};
    u32 pc_ = 0;
    u32 curr_line_ = 0;
    u32 code_size_ = 0;
    vm::Instruction *code_ = nullptr;

    [[nodiscard]] auto ending_pc() const noexcept { return code_size_; }
    void assign();

    using ExecuteFuncType = void (*)(vm::Instruction *);
    std::array<ExecuteFuncType, 20> execute_dispatch_table_;
};

} // namespace alpha::vm
#endif //MACHINE_HPP
