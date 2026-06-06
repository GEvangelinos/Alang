#ifndef MACHINE_HPP
#define MACHINE_HPP

#include <functional>
#include <memory>
#include <optional>

#include "vm_memory.hpp"
#include "core/numeric_types.hpp"
#include "core/bytecode/vm_instruction.hpp"
#include "core/bytecode/vm_program.hpp"

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
    explicit Machine(Bytes stack_size);

    void run();

    [[nodiscard]] Memcell* translate_operand(const vm::Argument* arg, Memcell* reg);
    void display_warning(const std::string& message);
    void error(const std::string& message);

    void set_out_stream(std::ostream& out) noexcept;
    void set_err_stream(std::ostream& err) noexcept;

    void execute_cycle();

    void execute_assign(const vm::Instruction& inst);
    void execute_call(const vm::Instruction& inst);
    void call_functor(vm::Table* table);
    void execute_funcenter(const vm::Instruction &inst);
    void execute_pusharg(const vm::Instruction & inst);
    void execute_newtable(const vm::Instruction& inst);
    void execute_tablegetelem(const vm::Instruction&inst);
    void execute_tablesetelem(const vm::Instruction& inst);

    struct DecodedOperands
    {
        const Memcell& rv1;
        const Memcell& rv2;
        std::underlying_type_t<Memcell::Type> op_idx;
        u8 t1_idx;
        u8 t2_idx;
    };

    template <vm::Opcode first, vm::Opcode last>
    [[nodiscard]] std::optional<DecodedOperands> decode_arithmetic_operands(const vm::Instruction& inst);
    void execute_arithmetic(const vm::Instruction& inst);
    void execute_relational_branch(const vm::Instruction&  inst);
    void execute_equality_branch(const vm::Instruction& inst);


    // inst is unused, but we require for uniformity anyway.
    void execute_funcexit([[maybe_unused]] const vm::Instruction& unused);

    void on_stack_overflow();

    void call_libfunc(const char* id);

    void impl_of_libfunc_print();
    void impl_of_libfunc_typeof();

private:
    class Stack
    {
    public:
        Stack(u64 size, std::function<void()> on_stack_overflow);

        [[nodiscard]] auto size() const noexcept { return size_; }
        void push_env_value(AlphaInt value);
        void save_call_environment(AlphaInt pc, AlphaInt total_actuals);
        void add_function_environment(const ProgFunc& func_info);
        [[nodiscard]] AlphaInt get_environment_value(u32 stack_idx) const noexcept;
        [[nodiscard]] auto top() const noexcept { return top_; }
        [[nodiscard]] auto topsp() const noexcept { return topsp_; }
        [[nodiscard]] Memcell& top_element() noexcept { return data_[top_]; }

        [[nodiscard]] u32 restore_previous_environment() noexcept;
        void clear_at(u32 idx);
        [[nodiscard]] AlphaInt total_actuals() const noexcept;
        [[nodiscard]] vm::Memcell& get_actual(u32 idx) const noexcept;
        void decrease_top();

        [[nodiscard]] const Memcell& operator[](u32 idx) const noexcept;
        [[nodiscard]] Memcell& operator[](u32 idx) noexcept;

    private:
        std::function<void()> on_stack_overflow_;
        std::unique_ptr<Memcell []> data_;
        const u64 size_ = 0;
        u32 top_ = 0, topsp_ = 0;
        DEBUG(OnceFlag is_overflowed;)
    };


    static constexpr auto k_saved_topsp_offset = 1;
    static constexpr auto k_saved_top_offset = 2;
    static constexpr auto k_saved_pc_offset = 3;
    static constexpr auto k_actual_count_offset = 4;
    static constexpr auto k_stack_environment_size = 4;

    Stack stack_;
    Memcell reg_a_{}, reg_b_{}, reg_c_{}, retval_{};
    vm::Instruction* code_ = nullptr;
    u32 total_actuals_ = 0;
    u32 pc_ = 0;
    u32 curr_line_ = 0;
    u32 code_size_ = 0;
    OnceFlag execution_finished_;
    std::ostream* out_stream_ = &std::cout;
    std::ostream* err_stream_ = &std::cerr;

    [[nodiscard]] auto ending_pc() const noexcept { return code_size_; }
    void assign(Memcell& lv, const Memcell& rv);

    using ExecuteFuncType = void (*)(vm::Instruction*);
    std::array<ExecuteFuncType, 20> execute_dispatch_table_;
};
} // namespace alpha::vm
#endif //MACHINE_HPP
