#ifndef PARSER_CONTEXT_HPP
#define PARSER_CONTEXT_HPP

#include <stack>
#include <vector>
#include "core/konstants.hpp"
#include "core/numeric_types.hpp"
#include "parser/symbol_table.hpp"
#include "parser/_parser_common.hpp"
#include "support/misc_tools.hpp"
#include "support/smart_assert.h"

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

    explicit SpaceHandler(const ParseCtx *host);
    ~SpaceHandler();

    void enter_space();

    void exit_space();

    [[nodiscard]] VarSymbol::Space space() const noexcept;

    [[nodiscard]] u32 next_offset() noexcept;

private:
    const ParseCtx *const host_;

    std::stack<u32> variable_offset_stack_;
};

class ScopeHandler : private Immobile
{
public:
    explicit ScopeHandler(const ParseCtx *host);
    ~ScopeHandler();

    void skip_next_scope_increment() noexcept;
    void enter_scope() noexcept;
    void exit_scope() noexcept;
    [[nodiscard]] u32 scope() const noexcept { return scope_; }

private:
    const ParseCtx *const host_;
    ToggleSwitch skip_next_scope_increment_;
    u32 scope_ = k_global_scope;
};

class CallCtxHandler : private Immobile
{
public:
    explicit CallCtxHandler(ParseCtx *host);
    ~CallCtxHandler();

    void enter_call() noexcept;
    void exit_call() noexcept;
    [[nodiscard]] bool is_in_call() const noexcept { return call_nesting_depth_ > 0; }

private:
    ParseCtx *const host_;
    u32 call_nesting_depth_ = 0;
};

class TableCtxHandler : private Immobile
{
public:
    explicit TableCtxHandler(ParseCtx *host);
    ~TableCtxHandler();

    void enter_dict_entry() noexcept;
    void exit_dict_entry() noexcept;
    [[nodiscard]] bool is_in_dict_entry() const noexcept { return dict_entry_nesting_depth_; }

private:
    ParseCtx *const host_;
    u32 dict_entry_nesting_depth_ = 0;
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
        const ProgFuncSymbol *func_symbol;
        const CodeAddress funcdef_skip_jump;
    };

    class FlowLivenessTracker
    {
    public:
        enum class FlowState : u8 { ALIVE, DEAD, RUNTIME };

        [[nodiscard]] bool is_in_dead_flow() const noexcept;
        [[nodiscard]] bool is_in_runtime_flow() const noexcept;
        void push_if_flow_state(FlowState flow_state);
        void switch_to_else();
        void push_loop_flow_state(FlowState flow_state);

        void pop_flow_state();

    private:
        std::vector<FlowState> flow_states_;
    };

    explicit FunctionCtxHandler(ParseCtx *host);

    ~FunctionCtxHandler();

    void enter_loop() noexcept;
    void exit_loop() noexcept;
    void add_function_parameter(StringSpan name, SourceLocation loc);
    void clear_function_parameters() noexcept;
    void add_local() noexcept;
    void enter_function(
        StringSpan func_name,
        SourceLocation func_loc,
        const ProgFuncSymbol *func_symbol,
        CodeAddress label_of_jump);
    [[nodiscard]] FunctionBackpatchInfo exit_function() noexcept;
    [[nodiscard]] u32 function_nesting_depth() const noexcept;
    [[nodiscard]] const std::string &current_function_name() const noexcept;
    [[nodiscard]] u32 current_function_scope() const noexcept;
    [[nodiscard]] SourceLocation current_function_location() const noexcept;
    [[nodiscard]] u32 loop_depth() const noexcept;
    [[nodiscard]] const std::vector<Parameter> &function_parameters() const noexcept;
    [[nodiscard]] const std::vector<CodeAddress> &break_list();
    void add_label_to_breaklist(CodeAddress jump_label); // Quad label of jump used to break.
    void add_label_to_continuelist(CodeAddress jump_label);
    void add_label_to_returnlist(CodeAddress jump_label);
    [[nodiscard]] const std::vector<CodeAddress> &continue_list();
    [[nodiscard]] const std::vector<CodeAddress> &return_list();
    [[nodiscard]] const FlowLivenessTracker &flow_liveness() const;
    [[nodiscard]] FlowLivenessTracker &flow_liveness();

private:
    struct FunctionDataFrame
    {
        const std::string name;
        const u32 scope;
        const SourceLocation loc;
        const ProgFuncSymbol *func_symbol; // Valid function ONLY IF NOT nullptr;
        const CodeAddress funcdef_skip_jump;   // used to go over function definition in runtime.
        FlowLivenessTracker flow_liveness_tracker;

        u32 loop_nesting_count = 0;
        // This is labels of breaks per loop in function
        std::stack<std::vector<CodeAddress>> function_breaklist_stack;
        // This is labels of continue of loops per loop in function
        std::stack<std::vector<CodeAddress>> function_continuelist_stack;
        // This is labels returns per function (in this FunctionDataFrame).
        std::vector<CodeAddress> function_returnlist;
        u32 local_variable_count = 0;

        FunctionDataFrame(
            const StringSpan name,
            const u32 scope,
            const SourceLocation loc,
            const ProgFuncSymbol *const func_symbol,
            const CodeAddress funcdef_skip_jump)
            : name(name.to_string()),
              scope(scope),
              loc(loc),
              func_symbol(func_symbol),
              funcdef_skip_jump(funcdef_skip_jump) {}
    };

    VectorStack<FunctionDataFrame> frame_stack_;
    std::vector<Parameter> function_parameters_;
    ParseCtx *const host_;
};

class AnonymousGenerator : private Immobile
{
public:
    [[nodiscard]] StringSpan new_anonymous();

private:
    std::deque<std::string> anonymous_name_cache_;
    u32 anonymous_counter_ = 0;
};

/// As I am writing testfiles, I come to realize, that a linear "stack-like" vector of temp handles,
/// will be insufficient. Although we mostly acquire and release the handle with the highest ID.
/// There are cases that we might acquire handle ID=1 but be able to reuse handle ID=0.
/// Thus, we need a pool-based system for handle management. Where acquiring a handle requires
/// a linear scan of the hande pool, where we get the first FREEed handle or add a new one.
class TempCtxHandler
{
public:
    explicit TempCtxHandler(const ParseCtx *host);
    ~TempCtxHandler();

    void push_temp_ctx_frame();
    void pop_temp_ctx_frame();

    [[nodiscard]] TempHandleID acquire_temp_handle();
    void release_temp_handle(TempHandleID id);

    void reset_temp_ctx_frame();

private:
    // Note: You could add a Free-list for faster acquisition, but then you would grab which ever
    // handle is available (or add a new one) and no the available handle with the least ID num.
    struct TempFrame
    {
        enum class Handle : u8 { RELEASED, ACQUIRED };

        std::vector<Handle> temp_handles_;
    };

    const ParseCtx *const host_;
    VectorStack<TempFrame> temp_frames;
};

class ElistCtxHandler
{
public:
    enum class Region : u8 { CALL, FORLOOP_CLAUSE, TABLE };

    std::optional<Region> region() const;

    void enter_region(Region r);
    void exit_region(DEBUG(Region r));

private:
    VectorStack<Region> region_stack_;
};

class ParseCtx : private Immobile
{
    friend class SemanticSystem;

public:
    SpaceHandler space_handler;
    ScopeHandler scope_handler;
    TableCtxHandler table_ctx_handler;
    CallCtxHandler call_ctx_handler;
    FunctionCtxHandler func_ctx_handler;
    AnonymousGenerator anonymous_generator;
    TempCtxHandler temp_ctx_handler;
    ElistCtxHandler elist_ctx_handler;

    explicit ParseCtx(SymbolTable *symbol_table);
    ~ParseCtx() = default;

    [[nodiscard]] const VarSymbol *new_temp();
    [[nodiscard]] bool hard_error_occurred() const noexcept;

private:
    SymbolTable *const symbol_table_;
    OnceFlag hard_error_occurred_;

    std::deque<std::string> temp_name_cache;

    [[nodiscard]] StringSpan generate_temp_name(TempHandleID temp_handle);
};

inline void
SpaceHandler::enter_space() { variable_offset_stack_.push(k_initial_variable_offset); }

inline void
SpaceHandler::exit_space()
{
    constexpr auto spaces_for_closure = 2; // 1 formalArg + 1 functionLocal

    DMASSERT(
        variable_offset_stack_.size() > spaces_for_closure,
        support::is_odd(variable_offset_stack_.size())
    );

    for (auto i = 0; i < spaces_for_closure; ++i)
        variable_offset_stack_.pop();
}

inline VarSymbol::Space
SpaceHandler::space() const noexcept
{
    DMASSERT(!variable_offset_stack_.empty() && " A stack frame must always exist");
    const auto frame_index = variable_offset_stack_.size() - 1; // -1 for size to index

    if (frame_index == k_initial_space)
        return VarSymbol::Space::PROGRAM_VAR;
    if (support::is_odd(frame_index))
        return VarSymbol::Space::FORMAL_ARGUMENT;
    return VarSymbol::Space::FUNCTION_LOCAL;
}

inline u32
SpaceHandler::next_offset() noexcept
{
    DMASSERT(!variable_offset_stack_.empty());
    return variable_offset_stack_.top()++;
}

inline void
ScopeHandler::skip_next_scope_increment() noexcept { skip_next_scope_increment_.enable(); }

inline void
ScopeHandler::enter_scope() noexcept
{
    if (skip_next_scope_increment_)
    {
        skip_next_scope_increment_.disable();
        return;
    }
    DMASSERT(scope_ < k_max_scope);
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
    DMASSERT(
        scope_ > k_global_scope,
        !skip_next_scope_increment_
    );
    --scope_;
}

inline void
CallCtxHandler::enter_call() noexcept
{
    DMASSERT(call_nesting_depth_< k_max_call_nesting && "A safe small sanity limit");
    ++call_nesting_depth_;
}

inline void
CallCtxHandler::exit_call() noexcept
{
    DMASSERT(call_nesting_depth_ > 0);
    --call_nesting_depth_;
}

inline void
TableCtxHandler::enter_dict_entry() noexcept
{
    DMASSERT(dict_entry_nesting_depth_< k_max_dict_nesting && "A safe small sanity limit")
    ;
    ++dict_entry_nesting_depth_;
}

inline void
TableCtxHandler::exit_dict_entry() noexcept
{
    DMASSERT(dict_entry_nesting_depth_ > 0);
    --dict_entry_nesting_depth_;
}

inline u32
FunctionCtxHandler::function_nesting_depth() const noexcept
{
    return frame_stack_.size() - k_global_data_frame_count;
}

inline u32
FunctionCtxHandler::current_function_scope() const noexcept
{
    DMASSERT(!frame_stack_.empty());
    return frame_stack_.top().scope;
}

inline const std::string &
FunctionCtxHandler::current_function_name() const noexcept
{
    DMASSERT(!frame_stack_.empty());
    return frame_stack_.top().name;
}

inline SourceLocation
FunctionCtxHandler::current_function_location() const noexcept
{
    DMASSERT(!frame_stack_.empty());
    return frame_stack_.top().loc;
}

inline void
FunctionCtxHandler::enter_loop() noexcept
{
    DMASSERT(!frame_stack_.empty());
    DMASSERT(frame_stack_.top().loop_nesting_count < k_max_loop_nesting);

    ++frame_stack_.top().loop_nesting_count;

    // Emplace empty breaklist (vector)
    frame_stack_.top().function_breaklist_stack.emplace();

    // Emplace empty continuelist (vector)
    frame_stack_.top().function_continuelist_stack.emplace();
}

inline void
FunctionCtxHandler::exit_loop() noexcept
{
    DMASSERT(!frame_stack_.empty());
    DMASSERT(frame_stack_.top().loop_nesting_count > 0);
    --frame_stack_.top().loop_nesting_count;

    DMASSERT(!frame_stack_.top().function_breaklist_stack.empty());
    DMASSERT(!frame_stack_.top().function_continuelist_stack.empty());
    // Emplace empty breaklist (vector)
    frame_stack_.top().function_breaklist_stack.pop();

    // Emplace empty continuelist (vector)
    frame_stack_.top().function_continuelist_stack.pop();
}

inline u32
FunctionCtxHandler::loop_depth() const noexcept
{
    DMASSERT(!frame_stack_.empty());
    return frame_stack_.top().loop_nesting_count;
}

inline void
FunctionCtxHandler::add_function_parameter(const StringSpan name, SourceLocation loc)
{
    function_parameters_.emplace_back(name, loc);
}

inline const std::vector<Parameter> &
FunctionCtxHandler::function_parameters() const noexcept { return function_parameters_; }

inline const std::vector<CodeAddress> &
FunctionCtxHandler::break_list()
{
    DMASSERT(!frame_stack_.empty());
    // We must be in a LOOP
    DMASSERT(!frame_stack_.top().function_breaklist_stack.empty());

    return frame_stack_.top().function_breaklist_stack.top();
}

inline const std::vector<CodeAddress> &
FunctionCtxHandler::continue_list()
{
    DMASSERT(!frame_stack_.empty());
    // We must be in a LOOP
    DMASSERT(!frame_stack_.top().function_continuelist_stack.empty());

    return frame_stack_.top().function_continuelist_stack.top();
}

inline void
FunctionCtxHandler::add_label_to_breaklist(const CodeAddress jump_label)
// Quad label of jump used to break.
{
    DMASSERT(!frame_stack_.empty());
    // We must be in a LOOP
    DMASSERT(!frame_stack_.top().function_breaklist_stack.empty());

    // we used Stack so each loop has its own break list
    frame_stack_.top().function_breaklist_stack.top().push_back(jump_label);
}

inline void
FunctionCtxHandler::add_label_to_continuelist(const CodeAddress jump_label)
// Quad label of jump used to break.
{
    DMASSERT(!frame_stack_.empty());

    // We must be in a LOOP
    DMASSERT(!frame_stack_.top().function_continuelist_stack.empty());

    // we used Stack so each loop has its own continue list.
    frame_stack_.top().function_continuelist_stack.top().push_back(jump_label);
}

inline const std::vector<CodeAddress> &
FunctionCtxHandler::return_list()
{
    DMASSERT(!frame_stack_.empty());
    return frame_stack_.top().function_returnlist;
}

inline const FunctionCtxHandler::FlowLivenessTracker &
FunctionCtxHandler::flow_liveness() const
{
    DMASSERT(!frame_stack_.empty());
    return frame_stack_.top().flow_liveness_tracker;
}

inline FunctionCtxHandler::FlowLivenessTracker &
FunctionCtxHandler::flow_liveness()
{
    DMASSERT(!frame_stack_.empty());
    return frame_stack_.top().flow_liveness_tracker;
}

inline void
FunctionCtxHandler::add_label_to_returnlist(const CodeAddress jump_label)
{
    // We must be in function
    DMASSERT(frame_stack_.size() > 1);
    frame_stack_.top().function_returnlist.push_back(jump_label);
}

inline void
FunctionCtxHandler::clear_function_parameters() noexcept { function_parameters_.clear(); }

inline void
FunctionCtxHandler::add_local() noexcept
{
    DMASSERT(!frame_stack_.empty());
    ++frame_stack_.top().local_variable_count;
}

inline StringSpan
AnonymousGenerator::new_anonymous()
{
    anonymous_name_cache_.emplace_back(FMT::format(ANONYMOUS_PREFIX"{}", anonymous_counter_++));
    const std::string& name = anonymous_name_cache_.back();
    return {name.data(), name.size()};
}

inline bool
ParseCtx::hard_error_occurred() const noexcept { return hard_error_occurred_.is_raised(); }
} // namespace alpha
#endif // PARSER_CONTEXT_HPP
