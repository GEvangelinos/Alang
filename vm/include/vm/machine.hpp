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
    Machine(Bytes stack_size, const vm::Executable& exe);

    void run();

    [[nodiscard]] Memcell* translate_operand(const vm::Argument* arg, Memcell* reg);
    void display_warning(const std::string& message);
    void error(const std::string& message);


    void set_out_stream(std::ostream& out) noexcept;
    void set_err_stream(std::ostream& err) noexcept;
    void set_in_stream(std::istream& in) noexcept;

    void execute_cycle();

    void execute_assign(const vm::Instruction& inst);
    void execute_call(const vm::Instruction& inst);
    void execute_jump(const vm::Instruction& inst);
    void call_functor(vm::Table* table);
    void execute_enterfunc(const vm::Instruction& inst);
    void execute_pusharg(const vm::Instruction& inst);
    void execute_newtable(const vm::Instruction& inst);
    void execute_tablegetelem(const vm::Instruction& inst);
    void execute_tablesetelem(const vm::Instruction& inst);
    void tablesetelem(Table& t, const Memcell& i, const Memcell& c);

    struct DecodedOperands
    {
        const Memcell& rv1;
        const Memcell& rv2;
        std::underlying_type_t<Memcell::Type> op_idx;
        u8 t1_idx;
        u8 t2_idx;
    };

    template <vm::Opcode first, vm::Opcode last>
    [[nodiscard]] std::optional<DecodedOperands> decode_arithmetic_operands(
        const vm::Instruction& inst);
    void execute_arithmetic(const vm::Instruction& inst);
    void execute_relational_branch(const vm::Instruction& inst);
    void execute_equality_branch(const vm::Instruction& inst);


    // inst is unused, but we require for uniformity anyway.
    void execute_exitfunc([[maybe_unused]] const vm::Instruction& unused);
    void execute_exitfunc([[maybe_unused]] const vm::Instruction* unused);

    void on_stack_overflow();

    void call_libfunc(LibFuncId libfunc_id);
    void call_libfunc(const char* libfunc_name);

    void impl_of_libfunc_print();
    void impl_of_libfunc_typeof();
    void impl_of_libfunc_input();
    void impl_of_libfunc_objecttotalmembers();
    void impl_of_libfunc_objectmemberkeys();
    void impl_of_libfunc_totalarguments();

private:
    class Stack
    {
    public:
        Stack(u64 size, u32 global_var_count, std::function<void()> on_stack_overflow);

        [[nodiscard]] auto size() const noexcept { return size_; }
        [[nodiscard]] Memcell* get_global_argument(u32 global_offset) noexcept;
        [[nodiscard]] Memcell* get_formal_argument(u32 formal_offset) noexcept;
        [[nodiscard]] Memcell* get_local_argument(u32 local_offset) noexcept;
        void push_env_value(AlphaInt value);
        void enter_frame();
        void allocate_locals(u32 count);
        void save_call_environment(AlphaInt pc, AlphaInt total_actuals);
        void add_function_environment(const ProgFuncMetadata& func_info);
        [[nodiscard]] AlphaInt get_environment_value(u32 stack_idx) const noexcept;
        [[nodiscard]] auto top() const noexcept { return top_; }
        [[nodiscard]] auto topsp() const noexcept { return topsp_; }
        [[nodiscard]] Memcell& top_element() noexcept { return data_[top_]; }

        [[nodiscard]] u32 restore_previous_environment() noexcept;
        void display_stack() const noexcept;
        void clear_at(u32 idx);
        [[nodiscard]] AlphaInt total_actuals() const noexcept;
        [[nodiscard]] vm::Memcell& get_actual(u32 idx) const noexcept;
        void decrease_top();

        [[nodiscard]] const Memcell& operator[](u32 idx) const noexcept;
        [[nodiscard]] Memcell& operator[](u32 idx) noexcept;

    private:
        const u64 size_ = 0;
        const u32 global_var_count_;
        std::function<void()> on_stack_overflow_;
        std::unique_ptr<Memcell []> data_;
        u32 top_, topsp_;
        DEBUG(OnceFlag is_overflowed;)
    };


    static constexpr auto k_saved_topsp_offset = 1;
    static constexpr auto k_saved_top_offset = 2;
    static constexpr auto k_saved_pc_offset = 3;
    static constexpr auto k_actual_count_offset = 4;
    static constexpr auto k_stack_environment_size = 4;

    Stack stack_;
    Memcell reg_a_{}, reg_b_{}, reg_c_{}, retval_{};
    const vm::Instruction* code_ = nullptr;
    const vm::Executable& exe_;
    const u32 ending_pc_;
    u32 total_actuals_ = 0;
    CodeAddress::UnderlyingType pc_ = 0;
    u32 curr_line_ = 0;
    OnceFlag execution_finished_;
    std::ostream* out_stream_ = &std::cout;
    std::ostream* err_stream_ = &std::cerr;
    std::istream* in_stream_ = &std::cin;

    void assign(Memcell& lv, const Memcell& rv);

    using ExecuteFuncType = void (Machine::*)(const vm::Instruction&);
    static constexpr std::array execute_dispatch_table_ = []() consteval
    {
        constexpr auto opcode_count = static_cast<std::size_t>(vm::Opcode::__COUNT__);
        std::array<ExecuteFuncType, opcode_count> result{};
        using OpcodeUT = std::underlying_type_t<Opcode>;
        result[static_cast<OpcodeUT>(Opcode::ASSIGN)] = &Machine::execute_assign;
        result[static_cast<OpcodeUT>(Opcode::ADD)] =
            result[static_cast<OpcodeUT>(Opcode::SUB)] =
            result[static_cast<OpcodeUT>(Opcode::MUL)] =
            result[static_cast<OpcodeUT>(Opcode::DIV)] =
            result[static_cast<OpcodeUT>(Opcode::MOD)] = &Machine::execute_arithmetic;
        result[static_cast<OpcodeUT>(Opcode::JUMP)] = &Machine::execute_jump;
        result[static_cast<OpcodeUT>(Opcode::JEQ)] =
            result[static_cast<OpcodeUT>(Opcode::JNE)] = &Machine::execute_equality_branch;
        result[static_cast<OpcodeUT>(Opcode::JLT)] =
            result[static_cast<OpcodeUT>(Opcode::JLE)] =
            result[static_cast<OpcodeUT>(Opcode::JGT)] =
            result[static_cast<OpcodeUT>(Opcode::JGE)] = &Machine::execute_relational_branch;
        result[static_cast<OpcodeUT>(Opcode::CALL)] = &Machine::execute_call;
        result[static_cast<OpcodeUT>(Opcode::PUSHARG)] = &Machine::execute_pusharg;
        result[static_cast<OpcodeUT>(Opcode::ENTERFUNC)] = &Machine::execute_enterfunc;
        result[static_cast<OpcodeUT>(Opcode::EXITFUNC)] = &Machine::execute_exitfunc;
        result[static_cast<OpcodeUT>(Opcode::NEWTABLE)] = &Machine::execute_newtable;
        result[static_cast<OpcodeUT>(Opcode::TABLEGETELEM)] = &Machine::execute_tablegetelem;
        result[static_cast<OpcodeUT>(Opcode::TABLESETELEM)] = &Machine::execute_tablesetelem;
        return result;
    }();

    using LibfuncImplFuncType = void (Machine::*)();
    static constexpr std::array libfunc_table_ = []() consteval
    {
        constexpr auto lib_func_count = static_cast<LibFuncIdUT>(LibFuncId::__COUNT__);
        std::array<LibfuncImplFuncType, lib_func_count> result{};

        const auto get_index_of = []<u64 size>(const char (&libfunc_name)[size])
        {
            return static_cast<LibFuncIdUT>(
                get_libfunc_id(StringSpan::from_literal(libfunc_name)).value()
            );
        };

        #define REGISTER_LIBFUNC(libfunc_name) result[get_index_of(#libfunc_name)] = &Machine::impl_of_libfunc_##libfunc_name
        REGISTER_LIBFUNC(print);
        REGISTER_LIBFUNC(input);
        REGISTER_LIBFUNC(typeof);
        REGISTER_LIBFUNC(objectmemberkeys);
        REGISTER_LIBFUNC(objecttotalmembers);
        REGISTER_LIBFUNC(totalarguments);
        #undef REGISTER_LIBFUNC
        return result;
    }();
};
} // namespace alpha::vm
#endif //MACHINE_HPP
