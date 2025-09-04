#include "parser/parser_context.hpp"

namespace alpha
{
SpaceHandler::SpaceHandler()
{
    enter_space(); // We push the first scope space frame (PROGRAM_VAR)
};

SpaceHandler::~SpaceHandler()
{
    // The following check is only valid if there was no syntax error.
    DEBUG_SMART_ASSERT(variable_offset_stack_.size() == 1);
}

ScopeHandler::ScopeHandler() : scope_(k_global_scope)
{
    DEBUG_SMART_ASSERT(
        skip_next_scope_increment_.is_disabled() &&
        "A ToggleSwitch must always be initialized as disabled."
    );
}

FunctionCtxHandler::FunctionCtxHandler(ParseCtx *const parse_ctx)
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

FunctionCtxHandler::~FunctionCtxHandler()
{
    DEBUG_SMART_ASSERT(
        frame_stack_.size() == k_global_data_frame_count,
        function_parameters_.empty() // All parameters must be used.
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
void
FunctionCtxHandler::enter_function(
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

FunctionCtxHandler::FunctionBackpatchInfo
FunctionCtxHandler::exit_function() noexcept
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
        .funcdef_skip_jump = top_frame.funcdef_skip_jump,
    };
}

ParseCtx::ParseCtx(SymbolTable *const symbol_table)
    : call_ctx_handler(this),
      func_ctx_handler(this),
      symbol_table_(utils::require_ptr(symbol_table)) {}

const VarSymbol *
ParseCtx::new_temp()
{
    const std::string temp_name = temp_ctx_handler.new_name();
    const Symbol *symbol = symbol_table_->lookup_local(temp_name, scope_handler.scope());

    // We register new temp, only if current scope doesn't have that temp.
    if (!symbol)
    {
        const VarSymbol::Type var_type =
                scope_handler.scope() == k_global_scope
                ? VarSymbol::Type::GLOBAL_VARIABLE
                : VarSymbol::Type::LOCAL_VARIABLE;

        symbol = symbol_table_->insert_variable(
            temp_name,
            scope_handler.scope(),
            var_type,
            space_handler.space(),
            space_handler.next_offset(),
            k_no_loc
        );
    }
    DEBUG_SMART_ASSERT(symbol->is_variable());
    return static_cast<const VarSymbol *>(symbol);
}
} // namespace alpha
