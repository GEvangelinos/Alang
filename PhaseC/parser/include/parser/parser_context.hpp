// Note: Alpha Parser Context is implemented as a header-only class.
//       Since it consists mostly of small, frequently used functions,
//       we define all methods in the header and mark them as `inline`.
//
//       This serves two purposes:
//       1) To comply with the One Definition Rule (ODR) when the header
//          is included in multiple translation units.
//       2) To hint to the compiler to inline calls for better performance.

#ifndef PARSER_CONTEXT_HPP
#define PARSER_CONTEXT_HPP

#include <list>
#include <stack>
#include <vector>
#include "diagnostics/diagnostic_engine.hpp"
#include "core/konstants.hpp"
#include "parser/konstants.hpp"
#include "core/numeric_types.hpp"
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

    [[nodiscard("Discarding this return value breaks variable offset sequencing")]]
    u32 next_offset() noexcept;

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
        const LabelID label_to_jump; // label to jump over function def in runtime.. TODO:
        // please rename its ugly
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

    [[nodiscard]] const std::vector<LabelID> &break_list() // TODO CLEANUP
    {
        DEBUG_SMART_ASSERT(!frame_stack_.empty());
        // We must be in a LOOP
        DEBUG_SMART_ASSERT(!frame_stack_.top().function_breaklist_stack.empty());

        return frame_stack_.top().function_breaklist_stack.top();
    }

    [[nodiscard]] const std::vector<LabelID> &continue_list() // TODO CLEANUP
    {
        DEBUG_SMART_ASSERT(!frame_stack_.empty());
        // We must be in a LOOP
        DEBUG_SMART_ASSERT(!frame_stack_.top().function_continuelist_stack.empty());

        return frame_stack_.top().function_continuelist_stack.top();
    }

    void add_label_to_breaklist(LabelID jump_label) // Quad label of jump used to break.
    {
        DEBUG_SMART_ASSERT(!frame_stack_.empty()); // TODO replace 1 (due to global frame)
        // We must be in a LOOP
        DEBUG_SMART_ASSERT(!frame_stack_.top().function_breaklist_stack.empty());

        // we used Stack so each loop has its own break list
        frame_stack_.top().function_breaklist_stack.top().push_back(jump_label);
    }

    void add_label_to_continuelist(LabelID jump_label) // Quad label of jump used to break.
    {
        // TODO repetitive code (same as continue.. DRY it out).
        DEBUG_SMART_ASSERT(!frame_stack_.empty()); // TODO replace 1 (due to global frame)

        // We must be in a LOOP
        DEBUG_SMART_ASSERT(!frame_stack_.top().function_continuelist_stack.empty());

        // we used Stack so each loop has its own continue list.
        frame_stack_.top().function_continuelist_stack.top().push_back(jump_label);
    }

    [[nodiscard]] const std::vector<LabelID> &return_list() // TODO CLEANUP
    {
        // We must be in a function. // Note at size 1. it global dataframe
        // So calling this function while there is only 1 framestack is a logic issue.
        DEBUG_SMART_ASSERT(frame_stack_.size() > 1); // TODO replace 1 (due to global frame)
        return frame_stack_.top().function_returnlist;
    }

    void add_label_to_returnlist(const LabelID jump_label) // Quad label of jump used to break.
    {
        // We must be in function
        DEBUG_SMART_ASSERT(frame_stack_.size() > 1); // TODO replace 1 (due to global frame)

        frame_stack_.top().function_returnlist.push_back(jump_label);
    }

private:
    struct FunctionDataFrame
    {
        const std::string name;
        const u32 scope;
        const SourceLocation loc;
        const FuncSymbol *func_symbol; // Valid function ONLY IF NOT nullptr;

        u32 loop_nesting_count = 0;

        // This is labels of breaks per loop in function
        std::stack<std::vector<LabelID> > function_breaklist_stack;
        // This is labels of continue of loops per loop in function
        std::stack<std::vector<LabelID> > function_continuelist_stack;

        // This is labels returns per function (in this FunctionDataFrame).
        std::vector<LabelID> function_returnlist;

        const LabelID label_of_jump; // used to go over function definition in runtime.

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
              label_of_jump(label_of_jump) {}
    };

    std::stack<FunctionDataFrame> frame_stack_;
    std::vector<Parameter> function_parameters_;
    u32 next_function_address_ = 0;

    ParseCtx *const parse_ctx_;
};

class NameGenerator : private Immobile
{
public:
    [[nodiscard]] std::string new_temp_name();

    void reset_temp_names() noexcept { temp_name_counter_ = 0; }

    [[nodiscard]] std::string new_anonymous();

private:
    u32 temp_name_counter_ = 0;
    u32 anonymous_counter_ = 0;
};

class ParseCtx : private Immobile
{
public:
    SpaceHandler space_handler;
    ScopeHandler scope_handler;
    FunctionCtxHandler func_ctx_handler;
    NameGenerator name_generator;

    explicit ParseCtx(SymbolTable &st);

    ~ParseCtx() = default;

    [[nodiscard]] const VarSymbol *new_temp();

private:
    SymbolTable &st_;
};

inline SpaceHandler::SpaceHandler()
{
    enter_space(); // We push the first scope space frame (PROGRAM_VAR)
};

inline SpaceHandler::~SpaceHandler() { DEBUG_SMART_ASSERT(variable_offset_stack_.size() == 1); }

inline void SpaceHandler::enter_space() { variable_offset_stack_.push(k_initial_variable_offset); }

inline void SpaceHandler::exit_space()
{
    constexpr auto spaces_for_closure = 2; // 1 formalArg + 1 functionLocal

    DEBUG_SMART_ASSERT(
        variable_offset_stack_.size() > spaces_for_closure,
        utils::is_odd(variable_offset_stack_.size())
    );

    for (auto i = 0; i < spaces_for_closure; ++i)
        variable_offset_stack_.pop();
}

inline VarSymbol::Space SpaceHandler::space() const noexcept
{
    DEBUG_SMART_ASSERT(!variable_offset_stack_.empty() && " A stack frame must always exist");
    const auto frame_index = variable_offset_stack_.size() - 1; // -1 for size to index

    if (frame_index == k_initial_space)
        return VarSymbol::Space::PROGRAM_VAR;
    if (utils::is_odd(frame_index))
        return VarSymbol::Space::FORMAL_ARGUMENT;
    return VarSymbol::Space::FUNCTION_LOCAL;
}

inline u32 SpaceHandler::next_offset() noexcept
{
    DEBUG_SMART_ASSERT(!variable_offset_stack_.empty());
    return variable_offset_stack_.top()++;
}

inline ScopeHandler::ScopeHandler() : scope_(k_global_scope)
{
    DEBUG_SMART_ASSERT(
        skip_next_scope_increment_.is_disabled() &&
        "A ToggleSwitch must always be initialized as disabled."
    );
}

inline void ScopeHandler::skip_next_scope_increment() noexcept
{
    skip_next_scope_increment_.enable();
}

inline void ScopeHandler::enter_scope() noexcept
{
    if (skip_next_scope_increment_.is_enabled())
    {
        skip_next_scope_increment_.disable();
        return;
    }
    DEBUG_SMART_ASSERT(scope_ < k_max_scope);
    ++scope_;
}

inline void ScopeHandler::exit_scope() noexcept
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

inline FunctionCtxHandler::FunctionCtxHandler(ParseCtx *const parse_ctx)
    : parse_ctx_(utils::require_ptr(parse_ctx))
{
    // We push a stack-frame, for loops that might occur outside functions.
    // So every frame corresponds to a function except the first.
    frame_stack_.emplace(
        k_global_data_frame_name,
        k_global_scope,
        k_no_loc,
        nullptr,
        k_no_label);
}

inline FunctionCtxHandler::~FunctionCtxHandler()
{
    DEBUG_SMART_ASSERT(                                   //
        frame_stack_.size() == k_global_data_frame_count, //
        function_parameters_.empty()                      // All parameters must be used.
    );
}

/**
 * @brief Push a new function context frame and adjust scope handling for its body.
 *
 * @param func_name     Name of the function being entered (parsed identifier).
 * @param func_loc      Source location of the function definition.
 * @param func_symbol   Associated FuncSymbol, or nullptr if invalid/redeclared.
 * @param label_of_jump Label for the runtime jump over the function body.
 *
 * @details
 * This is called before parameter registration. Parameter scope is +1 relative to
 * the function symbol's scope, but the opening brace of the function body would also
 * increment the scope, which would double-increment it. To prevent this, we mark the
 * next scope increment as skipped. skip_next_scope_increment() is a one-shot toggle.
 *
 * @rationale
 * Avoiding the double increment ensures that local variables and parameters share the
 * same correct scope nesting level, preventing subtle name resolution bugs.
 */
inline void FunctionCtxHandler::enter_function(
    const std::string &func_name,
    const SourceLocation func_loc,
    const FuncSymbol *const func_symbol,
    const LabelID label_of_jump)
{
    DEBUG_SMART_ASSERT(frame_stack_.size() < k_max_function_nesting && "A safe small sanity limit");
    DEBUG(
        if (!!func_symbol) DEBUG_SMART_ASSERT(
            func_symbol->name == func_name,
            func_symbol->scope == parse_ctx_->scope_handler.scope() &&
            "FuncSymbol's scope must match the parser scope at the point of entering the function" ,
            func_symbol->loc == func_loc,
            func_symbol->is_function(),
            func_symbol->type == Symbol::Type::PROGRAM_FUNCTION &&
            "Source-defined functions are always PROGRAM_FUNCTION by definition."
        );
    )

    frame_stack_.emplace(FunctionDataFrame(
        func_name,
        parse_ctx_->scope_handler.scope(),
        func_loc,
        func_symbol,
        label_of_jump
    ));
    parse_ctx_->scope_handler.enter_scope();
    parse_ctx_->scope_handler.skip_next_scope_increment();
}

inline FunctionCtxHandler::FunctionBackpatchInfo FunctionCtxHandler::exit_function() noexcept
{
    DEBUG_SMART_ASSERT(
        frame_stack_.size() > k_global_data_frame_count &&
        "A function frame must always exist for loops outside functions."
    );
    DEBUG_SMART_ASSERT(
        frame_stack_.top().loop_nesting_count == 0 &&
        "All loops must be closed before exiting a function."
    );

    const FunctionDataFrame top_frame = std::move(frame_stack_.top());
    frame_stack_.pop();

    return {
        .name = top_frame.name,
        .scope = top_frame.scope,
        .location = top_frame.loc,
        .local_var_count = top_frame.local_variable_count,
        .func_symbol = top_frame.func_symbol,
        // TODO: wtf is this? Rename to something more meaningful
        .label_to_jump = top_frame.label_of_jump,
    };
}

inline u32 FunctionCtxHandler::function_nesting_depth() const noexcept
{
    return frame_stack_.size() - k_global_data_frame_count;
}

inline u32 FunctionCtxHandler::current_function_scope() const noexcept
{
    DEBUG_SMART_ASSERT(!frame_stack_.empty());
    return frame_stack_.top().scope;
}

inline const std::string &FunctionCtxHandler::current_function_name() const noexcept
{
    DEBUG_SMART_ASSERT(!frame_stack_.empty());
    return frame_stack_.top().name;
}

inline SourceLocation FunctionCtxHandler::current_function_location() const noexcept
{
    DEBUG_SMART_ASSERT(!frame_stack_.empty());
    return frame_stack_.top().loc;
}

inline void FunctionCtxHandler::enter_loop() noexcept
{
    DEBUG_SMART_ASSERT(!frame_stack_.empty());
    DEBUG_SMART_ASSERT(frame_stack_.top().loop_nesting_count < k_max_loop_nesting);

    ++frame_stack_.top().loop_nesting_count;

    // Emplace empty breaklist (vector)
    frame_stack_.top().function_breaklist_stack.emplace();

    // Emplace empty continuelist (vector)
    frame_stack_.top().function_continuelist_stack.emplace();

    std::cout << "At enter_loop function_continuelist_stack_size == "
            << frame_stack_.top().function_continuelist_stack.size() << std::endl;
}

inline void FunctionCtxHandler::exit_loop() noexcept
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

inline u32 FunctionCtxHandler::loop_depth() const noexcept
{
    DEBUG_SMART_ASSERT(!frame_stack_.empty());
    return frame_stack_.top().loop_nesting_count;
}

inline void FunctionCtxHandler::add_function_parameter(const std::string &name, SourceLocation loc)
{
    function_parameters_.emplace_back(name, loc);
}

inline const std::vector<Parameter> &FunctionCtxHandler::function_parameters() const noexcept
{
    return function_parameters_;
}

inline void FunctionCtxHandler::clear_function_parameters() noexcept
{
    function_parameters_.clear();
}

inline void FunctionCtxHandler::add_local() noexcept { ++frame_stack_.top().local_variable_count; }

inline std::string NameGenerator::new_temp_name()
{
    return k_temp_variable_prefix + std::to_string(temp_name_counter_++);
}

inline std::string NameGenerator::new_anonymous()
{
    return k_private_anonymous_prefix + std::to_string(anonymous_counter_++);
}

inline ParseCtx::ParseCtx(SymbolTable &st)
    : func_ctx_handler(this), st_(st) {}

inline const VarSymbol *ParseCtx::new_temp()
{
    const std::string temp_name = name_generator.new_temp_name();
    const Symbol *symbol = st_.lookup_local(temp_name, scope_handler.scope());

    // We register new temp, only if current scope doesn't have that temp.
    if (!symbol)
    {
        const VarSymbol::Type var_type =
                scope_handler.scope() == k_global_scope
                ? VarSymbol::Type::GLOBAL_VARIABLE
                : VarSymbol::Type::LOCAL_VARIABLE;

        symbol = st_.insert_variable(
            temp_name,
            scope_handler.scope(),
            var_type,
            space_handler.space(),
            space_handler.next_offset(),
            k_no_loc
        );
    }
    // variables and values. known the line a temp was generated is useless...
    DEBUG_SMART_ASSERT(symbol->is_variable());
    return static_cast<const VarSymbol *>(symbol);
}
} // namespace alpha
#endif // PARSER_CONTEXT_HPP
