#ifndef PARSER_CONTEXT_HPP
#define PARSER_CONTEXT_HPP

#include <stack>
#include <vector>
#include "core/konstants.hpp"
#include "core/numeric_types.hpp"
#include "diagnostics/diagnostic_engine.hpp"
#include "parser/konstants.hpp"
#include "parser/symbol_table.hpp"
#include "parser/_parser_common.hpp"
#include "utils/misc.hpp"
#include "utils/smart_assert.h"

namespace alpha
{
class ParseCtx;

class SpaceHandler : private Immobile
{
public:
    static constexpr u32 k_initial_space = 0;
    static_assert(k_initial_space == 0, "Space mapping logic requires 0");
    static constexpr u32 k_initial_variable_offset = 0;
    static_assert(k_initial_variable_offset == 0, "Variable offset must start at 0");

    SpaceHandler();

    ~SpaceHandler();

    void enter_space();

    void exit_space();

    [[nodiscard]] VarSymbol::Space space() const noexcept;

    [[nodiscard]] u32 next_offset() noexcept;

private:
    std::stack<u32> variable_offset_stack_;
};

class ScopeHandler : private Immobile
{
public:
    ScopeHandler();

    ~ScopeHandler() = default;

    void skip_next_scope_increment() noexcept;

    void enter_scope() noexcept;

    void exit_scope() noexcept;

    [[nodiscard]] u32 scope() const noexcept { return scope_; }

private:
    ToggleSwitch skip_next_scope_increment_;
    u32 scope_;
};

class CallContextHandler : private Immobile
{
public:
    explicit CallContextHandler(ParseCtx *parse_ctx);

    void enter_call();
    void exit_call();
    bool is_in_call() const noexcept { return call_nesting_count_ > 0; }

private:
    ParseCtx *const parse_ctx_;
    u32 call_nesting_count_ = 0;
};

class FunctionCtxHandler : private Immobile
{
public:
    struct FunctionBackpatchInfo
    {
        const std::string name;
        const u32 scope;
        const SourceLocation location;
        const u32 local_var_count;
        const FuncSymbol *func_symbol;
        const LabelID funcdef_skip_jump;
    };

    explicit FunctionCtxHandler(ParseCtx *parse_ctx);

    ~FunctionCtxHandler();

    void enter_loop() noexcept;
    void exit_loop() noexcept;
    void add_function_parameter(const std::string &name, SourceLocation loc);
    void clear_function_parameters() noexcept;
    void add_local() noexcept;
    void enter_function(
        const std::string &func_name,
        SourceLocation func_loc,
        const FuncSymbol *func_symbol,
        LabelID label_of_jump);
    [[nodiscard]] FunctionBackpatchInfo exit_function() noexcept;
    [[nodiscard]] u32 function_nesting_depth() const noexcept;
    [[nodiscard]] u32 current_function_scope() const noexcept;
    [[nodiscard]] const std::string &current_function_name() const noexcept;
    [[nodiscard]] SourceLocation current_function_location() const noexcept;
    [[nodiscard]] u32 loop_depth() const noexcept;
    [[nodiscard]] const std::vector<Parameter> &function_parameters() const noexcept;
    [[nodiscard]] u32 next_function_address() noexcept { return next_function_address_++; }
    [[nodiscard]] const std::vector<LabelID> &break_list();
    void add_label_to_breaklist(LabelID jump_label); // Quad label of jump used to break.
    void add_label_to_continuelist(LabelID jump_label);
    void add_label_to_returnlist(LabelID jump_label);
    [[nodiscard]] const std::vector<LabelID> &continue_list();
    [[nodiscard]] const std::vector<LabelID> &return_list();

private:
    struct FunctionDataFrame
    {
        const std::string name;
        const u32 scope;
        const SourceLocation loc;
        const FuncSymbol *func_symbol; // Valid function ONLY IF NOT nullptr;

        u32 loop_nesting_count = 0;

        // This is labels of breaks per loop in function
        std::stack<std::vector<LabelID>> function_breaklist_stack;
        // This is labels of continue of loops per loop in function
        std::stack<std::vector<LabelID>> function_continuelist_stack;

        // This is labels returns per function (in this FunctionDataFrame).
        std::vector<LabelID> function_returnlist;

        const LabelID funcdef_skip_jump; // used to go over function definition in runtime.

        u32 local_variable_count = 0;

        FunctionDataFrame(
            const std::string &name,
            const u32 scope,
            const SourceLocation loc,
            const FuncSymbol *const func_symbol,
            const u32 label_of_jump)
            : name(std::move(name)),
              scope(scope),
              loc(loc),
              func_symbol(func_symbol),
              funcdef_skip_jump(label_of_jump) {}
    };

    std::stack<FunctionDataFrame> frame_stack_;
    std::vector<Parameter> function_parameters_;
    u32 next_function_address_ = 1; // Function addresses are positive integers, so we start from 1.

    ParseCtx *const parse_ctx_;
};

class AnonymousGenerator : private Immobile
{
public:
    [[nodiscard]] std::string new_anonymous();

private:
    u32 anonymous_counter_ = 0;
};

class TempCtxHandler
{
public:
    enum class TempRegion{CALL, TABLE, FORLOOP_CLAUSE};
    VectorStack<TempRegion> region_stack;

    [[nodiscard]] std::string new_name();

    void reset_all();
    void reset_to_checkpoint();
    void push_checkpoint();
    void pop_checkpoint();
    void push_checkpoint_barrier();
    void pop_checkpoint_barrier();


private:
    u32 temp_name_counter_ = 0;

    std::vector<u32> checkpoints_;
    VectorStack<u32> checkpoint_barriers_;
};

class ParseCtx : private Immobile
{
public:
    SpaceHandler space_handler;
    ScopeHandler scope_handler;
    CallContextHandler call_ctx_handler;
    FunctionCtxHandler func_ctx_handler;
    AnonymousGenerator anonymous_generator;
    TempCtxHandler temp_ctx_handler;

    explicit ParseCtx(SymbolTable *symbol_table);
    ~ParseCtx() = default;

    [[nodiscard]] const VarSymbol *new_temp();

private:
    SymbolTable *const symbol_table_;
};

inline void
SpaceHandler::enter_space() { variable_offset_stack_.push(k_initial_variable_offset); }

inline void
SpaceHandler::exit_space()
{
    constexpr auto spaces_for_closure = 2; // 1 formalArg + 1 functionLocal

    DEBUG_SMART_ASSERT(
        variable_offset_stack_.size() > spaces_for_closure,
        utils::is_odd(variable_offset_stack_.size())
    );

    for (auto i = 0; i < spaces_for_closure; ++i)
        variable_offset_stack_.pop();
}

inline VarSymbol::Space
SpaceHandler::space() const noexcept
{
    DEBUG_SMART_ASSERT(!variable_offset_stack_.empty() && " A stack frame must always exist");
    const auto frame_index = variable_offset_stack_.size() - 1; // -1 for size to index

    if (frame_index == k_initial_space)
        return VarSymbol::Space::PROGRAM_VAR;
    if (utils::is_odd(frame_index))
        return VarSymbol::Space::FORMAL_ARGUMENT;
    return VarSymbol::Space::FUNCTION_LOCAL;
}

inline u32
SpaceHandler::next_offset() noexcept
{
    DEBUG_SMART_ASSERT(!variable_offset_stack_.empty());
    return variable_offset_stack_.top()++;
}

inline void
ScopeHandler::skip_next_scope_increment() noexcept { skip_next_scope_increment_.enable(); }

inline void
ScopeHandler::enter_scope() noexcept
{
    if (skip_next_scope_increment_.is_enabled())
    {
        skip_next_scope_increment_.disable();
        return;
    }
    DEBUG_SMART_ASSERT(scope_ < k_max_scope);
    ++scope_;
}

inline void
ScopeHandler::exit_scope() noexcept
{
    // We expect `skip_next_scope_increment` ToggleSwitch to be
    // disabled. That is because if you exit a block, it means you first
    // entered it. So if you exit a block and the
    // `skip_next_scope_increment` switch is enabled, there is a logic
    // issue.
    DEBUG_SMART_ASSERT(                          //
        scope_ > k_global_scope,                 //
        skip_next_scope_increment_.is_disabled() //
    );
    --scope_;
}

inline
CallContextHandler::CallContextHandler(ParseCtx *const parse_ctx)
    : parse_ctx_(utils::require_ptr(parse_ctx)) {}

inline void
CallContextHandler::enter_call()
{
    DEBUG_SMART_ASSERT(call_nesting_count_< k_max_call_nesting && "A safe small sanity limit");
    ++call_nesting_count_;
}

inline void
CallContextHandler::exit_call()
{
    DEBUG_SMART_ASSERT(call_nesting_count_ > 0);
    --call_nesting_count_;
}

inline u32
FunctionCtxHandler::function_nesting_depth() const noexcept
{
    return frame_stack_.size() - k_global_data_frame_count;
}

inline u32
FunctionCtxHandler::current_function_scope() const noexcept
{
    DEBUG_SMART_ASSERT(!frame_stack_.empty());
    return frame_stack_.top().scope;
}

inline const std::string &
FunctionCtxHandler::current_function_name() const noexcept
{
    DEBUG_SMART_ASSERT(!frame_stack_.empty());
    return frame_stack_.top().name;
}

inline SourceLocation
FunctionCtxHandler::current_function_location() const noexcept
{
    DEBUG_SMART_ASSERT(!frame_stack_.empty());
    return frame_stack_.top().loc;
}

inline void
FunctionCtxHandler::enter_loop() noexcept
{
    DEBUG_SMART_ASSERT(!frame_stack_.empty());
    DEBUG_SMART_ASSERT(frame_stack_.top().loop_nesting_count < k_max_loop_nesting);

    ++frame_stack_.top().loop_nesting_count;

    // Emplace empty breaklist (vector)
    frame_stack_.top().function_breaklist_stack.emplace();

    // Emplace empty continuelist (vector)
    frame_stack_.top().function_continuelist_stack.emplace();
}

inline void
FunctionCtxHandler::exit_loop() noexcept
{
    DEBUG_SMART_ASSERT(!frame_stack_.empty());
    DEBUG_SMART_ASSERT(frame_stack_.top().loop_nesting_count > 0);
    --frame_stack_.top().loop_nesting_count;

    DEBUG_SMART_ASSERT(!frame_stack_.top().function_breaklist_stack.empty());
    DEBUG_SMART_ASSERT(!frame_stack_.top().function_continuelist_stack.empty());
    // Emplace empty breaklist (vector)
    frame_stack_.top().function_breaklist_stack.pop();

    // Emplace empty continuelist (vector)
    frame_stack_.top().function_continuelist_stack.pop();
}

inline u32
FunctionCtxHandler::loop_depth() const noexcept
{
    DEBUG_SMART_ASSERT(!frame_stack_.empty());
    return frame_stack_.top().loop_nesting_count;
}

inline void
FunctionCtxHandler::add_function_parameter(const std::string &name, SourceLocation loc)
{
    function_parameters_.emplace_back(name, loc);
}

inline const std::vector<Parameter> &
FunctionCtxHandler::function_parameters() const noexcept { return function_parameters_; }

inline const std::vector<LabelID> &
FunctionCtxHandler::break_list()
{
    DEBUG_SMART_ASSERT(!frame_stack_.empty());
    // We must be in a LOOP
    DEBUG_SMART_ASSERT(!frame_stack_.top().function_breaklist_stack.empty());

    return frame_stack_.top().function_breaklist_stack.top();
}

inline const std::vector<LabelID> &
FunctionCtxHandler::continue_list()
{
    DEBUG_SMART_ASSERT(!frame_stack_.empty());
    // We must be in a LOOP
    DEBUG_SMART_ASSERT(!frame_stack_.top().function_continuelist_stack.empty());

    return frame_stack_.top().function_continuelist_stack.top();
}

inline void
FunctionCtxHandler::add_label_to_breaklist(const LabelID jump_label)
// Quad label of jump used to break.
{
    DEBUG_SMART_ASSERT(!frame_stack_.empty());
    // We must be in a LOOP
    DEBUG_SMART_ASSERT(!frame_stack_.top().function_breaklist_stack.empty());

    // we used Stack so each loop has its own break list
    frame_stack_.top().function_breaklist_stack.top().push_back(jump_label);
}

inline void
FunctionCtxHandler::add_label_to_continuelist(const LabelID jump_label)
// Quad label of jump used to break.
{
    DEBUG_SMART_ASSERT(!frame_stack_.empty());

    // We must be in a LOOP
    DEBUG_SMART_ASSERT(!frame_stack_.top().function_continuelist_stack.empty());

    // we used Stack so each loop has its own continue list.
    frame_stack_.top().function_continuelist_stack.top().push_back(jump_label);
}

inline const std::vector<LabelID> &
FunctionCtxHandler::return_list()
{
    // We must be in a function. // Note at size 1. it global dataframe
    // So calling this function while there is only 1 framestack is a logic issue.
    DEBUG_SMART_ASSERT(frame_stack_.size() > 1);
    return frame_stack_.top().function_returnlist;
}

inline void
FunctionCtxHandler::add_label_to_returnlist(const LabelID jump_label)
{
    // We must be in function
    DEBUG_SMART_ASSERT(frame_stack_.size() > 1);
    frame_stack_.top().function_returnlist.push_back(jump_label);
}

inline void
FunctionCtxHandler::clear_function_parameters() noexcept { function_parameters_.clear(); }

inline void
FunctionCtxHandler::add_local() noexcept
{
    DEBUG_SMART_ASSERT(!frame_stack_.empty());
    ++frame_stack_.top().local_variable_count;
}

inline std::string
TempCtxHandler::new_name() { return k_temp_variable_prefix + std::to_string(temp_name_counter_++); }

inline void
TempCtxHandler::reset_all()
{
    checkpoints_.clear();
    temp_name_counter_ = 0;
}

inline void
TempCtxHandler::reset_to_checkpoint()
{
    #ifndef CYA_MODE
    temp_name_counter_ = checkpoints_.empty() ? 0 : checkpoints_.back();
    #endif
}

inline void
TempCtxHandler::push_checkpoint()
{
    #ifndef CYA_MODE
    checkpoints_.push_back(temp_name_counter_);
    #endif
}

inline void
TempCtxHandler::pop_checkpoint()
{
    #ifndef CYA_MODE
    DEBUG_SMART_ASSERT(!checkpoints_.empty());
    checkpoints_.pop_back();
    #endif
}

inline void TempCtxHandler::push_checkpoint_barrier()
{
    #ifndef CYA_MODE
    checkpoint_barriers_.push(checkpoints_.size());
    #endif

}

inline void TempCtxHandler::pop_checkpoint_barrier()
{
    #ifndef CYA_MODE
    DEBUG_SMART_ASSERT(!checkpoint_barriers_.empty());
    while (checkpoints_.size() > checkpoint_barriers_.top())
        pop_checkpoint();
    checkpoint_barriers_.pop();
    reset_to_checkpoint();
    #endif
}

inline std::string
AnonymousGenerator::new_anonymous() { return k_anonymous_prefix + std::to_string(anonymous_counter_++); }
} // namespace alpha
#endif // PARSER_CONTEXT_HPP
