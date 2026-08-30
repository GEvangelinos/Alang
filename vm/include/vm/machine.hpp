#ifndef MACHINE_HPP
#define MACHINE_HPP

#include <functional>
#include <memory>
#include <optional>

#include "vm_memory.hpp"
#include "bytecode/executable.hpp"
#include "core/numeric_types.hpp"
#include "core/bytecode/vm_instruction.hpp"
#include "core/bytecode/vm_program.hpp"
#include "core/libfunc/mappings.hpp"

namespace alpha::vm
{
struct Executable;

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
    using ExecuteFuncType = void (Machine::*)(const vm::Instruction &);
    using LibfuncImplFuncType = void (Machine::*)();

    Machine(Bytes stack_size, const vm::Executable &exe);

    void run();

    [[nodiscard]] Memcell *translate_operand(const vm::Argument *arg, Memcell *reg);
    void display_warning(const std::string &message);
    void error(const std::string &message);


    void set_out_stream(std::ostream &out) noexcept;
    void set_err_stream(std::ostream &err) noexcept;
    void set_in_stream(std::istream &in) noexcept;

    void execute_cycle();

    void execute_trap(const vm::Instruction &inst);

    void execute_assign(const vm::Instruction &inst);
    void execute_call(const vm::Instruction &inst);
    void execute_jump(const vm::Instruction &inst);
    void call_functor(vm::Table *table);
    void execute_enterfunc(const vm::Instruction &inst);
    void execute_pusharg(const vm::Instruction &inst);
    void execute_newtable(const vm::Instruction &inst);
    void execute_tablegetelem(const vm::Instruction &inst);
    void execute_tablesetelem(const vm::Instruction &inst);
    void tablesetelem(Table &t, const Memcell &i, const Memcell &c);

    struct DecodedOperands
    {
        const Memcell &rv1;
        const Memcell &rv2;
        std::underlying_type_t<Memcell::Type> op_idx;
        u8 t1_idx;
        u8 t2_idx;
    };

    template<vm::Opcode first, vm::Opcode last>
    [[nodiscard]] std::optional<DecodedOperands> decode_arithmetic_operands(
        const vm::Instruction &inst);
    void execute_arithmetic(const vm::Instruction &inst);
    void execute_relational_branch(const vm::Instruction &inst);
    void execute_equality_branch(const vm::Instruction &inst);


    // inst is unused, but we require for uniformity anyway.
    void execute_exitfunc([[maybe_unused]] const vm::Instruction &unused);
    void execute_exitfunc([[maybe_unused]] const vm::Instruction *unused);

    void on_stack_overflow();

    void call_libfunc(LibFuncId libfunc_id);
    void call_libfunc(const char *libfunc_name);

    void impl_of_libfunc_print();
    void impl_of_libfunc_typeof();
    void impl_of_libfunc_input();
    void impl_of_libfunc_objecttotalmembers();
    void impl_of_libfunc_objectmemberkeys();
    void impl_of_libfunc_totalarguments();
    void impl_of_libfunc_objectcopy();
    void impl_of_libfunc_strtonum();
    void impl_of_libfunc_argument();
    void impl_of_libfunc_sin();
    void impl_of_libfunc_cos();
    void impl_of_libfunc_sqrt();

private:
    class Stack
    {
    public:
        struct Index : StrongType<Index, u32> { using StrongType::StrongType; };

        Stack(u64 size, u32 global_var_count, std::function<void()> on_stack_overflow);

        [[nodiscard]] auto size() const noexcept { return size_; }
        [[nodiscard]] Memcell *get_global_argument(u32 global_offset) const noexcept;
        [[nodiscard]] Memcell *get_formal_argument(u32 formal_offset) const noexcept;
        [[nodiscard]] Memcell *get_local_argument(u32 local_offset) const const noexcept;
        [[nodiscard]] Memcell *get_memcell_at(Stack::Index idx) const noexcept;
        void push_env_value(AlphaInt value);
        void enter_frame();
        void allocate_locals(u32 count);
        void save_call_environment(AlphaInt pc, AlphaInt total_actuals);
        void add_function_environment(const ProgFuncMetadata &func_info);
        [[nodiscard]] AlphaInt get_environment_value(Stack::Index stack_idx) const noexcept;
        [[nodiscard]] Stack::Index top() const noexcept { return top_; }
        [[nodiscard]] Stack::Index topsp() const noexcept { return topsp_; }
        [[nodiscard]] Memcell &top_element() noexcept { return data_[top_.value]; }


        [[nodiscard]] auto global_top() const noexcept { return global_top_; }
        [[nodiscard]] std::optional<Stack::Index> calc_prev_topsp() const noexcept;

        [[nodiscard]] u32 restore_previous_environment() noexcept;
        void display_stack() const noexcept;
        void clear_at(Stack::Index idx);
        [[nodiscard]] AlphaInt total_actuals() const noexcept;
        [[nodiscard]] vm::Memcell &get_actual(Stack::Index idx) const noexcept;
        void decrease_top();


        [[nodiscard]] const Memcell &operator[](u32 idx) const noexcept;
        [[nodiscard]] Memcell &operator[](u32 idx) noexcept;

    private:
        const u64 size_ = 0;
        const u32 global_var_count_;
        const Stack::Index global_top_;
        Stack::Index top_, topsp_;
        std::function<void()> on_stack_overflow_;
        std::unique_ptr<Memcell []> data_;
        DEBUG(OnceFlag is_overflowed;)

        template<std::integral N>
        [[nodiscard]] static Stack::Index to_stack_index(N index) noexcept;
    };


    static constexpr auto k_saved_topsp_offset = 1;
    static constexpr auto k_saved_top_offset = 2;
    static constexpr auto k_saved_pc_offset = 3;
    static constexpr auto k_actual_count_offset = 4;
    static constexpr auto k_stack_environment_size = 4;

    Stack stack_;
    Memcell reg_a_{}, reg_b_{}, reg_c_{}, retval_{};
    const vm::Instruction *code_ = nullptr;
    const vm::Executable &exe_;
    const u32 ending_pc_;
    u32 total_actuals_ = 0;
    CodeAddress::UnderlyingType pc_ = 1;
    u32 curr_line_ = 0;
    OnceFlag execution_finished_;

    std::ostream *out_stream_ = &std::cout;
    std::ostream *err_stream_ = &std::cerr;
    std::istream *in_stream_ = &std::cin;

    static constinit std::array<ExecuteFuncType, 256> execute_dispatch_table_;
    static constinit std::array<LibfuncImplFuncType, 12> libfunc_table_;

    void assign(Memcell &lv, const Memcell &rv);
};
} // namespace alpha::vm
#endif //MACHINE_HPP
