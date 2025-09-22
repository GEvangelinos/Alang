#include "parser/parser_context.hpp"

namespace alpha
{
SpaceHandler::SpaceHandler(const ParseCtx *const host)
    : host_(support::require_ptr(host))
{
    enter_space(); // We push the first scope space frame (PROGRAM_VAR)
}

SpaceHandler::~SpaceHandler()
{
    // Only check invariants if no fatal errors occurred.
    // Parse error recovery may leave stacks inconsistent,
    // so assertions are skipped in that case.
    DEBUG(
        if (host_->hard_error_occurred.raised()) return;
        DEBUG_SMART_ASSERT(variable_offset_stack_.size() == 1);
    )
}

ScopeHandler::ScopeHandler(const ParseCtx *host)
    : host_(support::require_ptr(host))
{
    DEBUG_SMART_ASSERT(
        scope_ == k_global_scope &&
        "Scope should always start from global scope",
        skip_next_scope_increment_.is_disabled() &&
        "A ToggleSwitch must always be initialized as disabled."
    );
}

ScopeHandler::~ScopeHandler()
{
    DEBUG(
        if (host_->hard_error_occurred.raised()) return;
        DEBUG_SMART_ASSERT(scope_ == k_global_scope);
    )
}

CallContextHandler::CallContextHandler(ParseCtx *const host)
    : host_(support::require_ptr(host)) {}

CallContextHandler::~CallContextHandler()
{
    DEBUG(
        if (host_->hard_error_occurred.raised()) return;
        DEBUG_SMART_ASSERT(call_nesting_count_ == 0);
    )
}

FunctionCtxHandler::FunctionCtxHandler(ParseCtx *const host)
    : host_(support::require_ptr(host))
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
    DEBUG(
        if (host_->hard_error_occurred.raised()) return;
        DEBUG_SMART_ASSERT(
            frame_stack_.size() == k_global_data_frame_count,
            function_parameters_.empty() // All parameters must be used.
        );
    )
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
            func_symbol->scope == host_->scope_handler.scope() &&
            "FuncSymbol's scope must match the parser scope at the point of entering the function" ,
            func_symbol->loc == func_loc,
            func_symbol->is_function(),
            func_symbol->type == Symbol::Type::PROGRAM_FUNCTION &&
            "Source-defined functions are always PROGRAM_FUNCTION by definition."
        );
    )

    frame_stack_.emplace(FunctionDataFrame(
        func_name,
        host_->scope_handler.scope(),
        func_loc,
        func_symbol,
        label_of_jump
    ));
    host_->scope_handler.enter_scope();
    host_->scope_handler.skip_next_scope_increment();
    host_->temp_ctx_handler.push_temp_ctx_frame();
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

    host_->temp_ctx_handler.pop_temp_ctx_frame();

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

TempCtxHandler::TempCtxHandler(const ParseCtx *const host)
    : host_(support::require_ptr(host)) { push_temp_ctx_frame(); }

TempCtxHandler::~TempCtxHandler()
{
    DEBUG(
        if (host_->hard_error_occurred.raised()) return;
        // At the end (before destruction) if everything went right, there should be only a single frame.
        // The one pushed at construction.
        DEBUG_SMART_ASSERT(temp_ctx_frame_stack_.size() == 1);
    )
}

void
TempCtxHandler::push_temp_ctx_frame() { temp_ctx_frame_stack_.emplace(); }

void
TempCtxHandler::pop_temp_ctx_frame()
{
    DEBUG_SMART_ASSERT(!temp_ctx_frame_stack_.empty());
    temp_ctx_frame_stack_.pop();
}

void
TempCtxHandler::reset_current_frame()
{
    DEBUG_SMART_ASSERT(!temp_ctx_frame_stack_.empty());
    pop_temp_ctx_frame();
    push_temp_ctx_frame();
}

std::optional<TempCtxHandler::CriticalRegion>
TempCtxHandler::current_critical_region() const
{
    DEBUG_SMART_ASSERT(!temp_ctx_frame_stack_.empty());
    const TempCtxFrame &current_frame = temp_ctx_frame_stack_.top();

    if (current_frame.critical_region_stack.empty())
        return std::nullopt;
    return current_frame.critical_region_stack.top();
}

void
TempCtxHandler::enter_critical_region(const TempCtxHandler::CriticalRegion region_to_enter)
{
    DEBUG_SMART_ASSERT(!temp_ctx_frame_stack_.empty());
    temp_ctx_frame_stack_.top().critical_region_stack.push(region_to_enter);
}

void
TempCtxHandler::exit_critical_region()
{
    DEBUG_SMART_ASSERT(!temp_ctx_frame_stack_.empty());
    temp_ctx_frame_stack_.top().critical_region_stack.pop();
}

void
TempCtxHandler::reset_to_checkpoint()
{
    DEBUG_SMART_ASSERT(!temp_ctx_frame_stack_.empty());
    TempCtxFrame &current_frame = temp_ctx_frame_stack_.top();
    current_frame.temp_counter_ = current_frame.checkpoints_.empty()
                                  ? 0
                                  : current_frame.checkpoints_.back();
}

void
TempCtxHandler::push_checkpoint()
{
    DEBUG_SMART_ASSERT(!temp_ctx_frame_stack_.empty());
    TempCtxFrame &current_frame = temp_ctx_frame_stack_.top();
    current_frame.checkpoints_.push_back(current_frame.temp_counter_);
}

void
TempCtxHandler::pop_checkpoint()
{
    DEBUG_SMART_ASSERT(!temp_ctx_frame_stack_.empty());
    TempCtxFrame &current_frame = temp_ctx_frame_stack_.top();
    DEBUG_SMART_ASSERT(!current_frame.checkpoints_.empty());
    current_frame.checkpoints_.pop_back();
}

void
TempCtxHandler::push_checkpoint_barrier()
{
    DEBUG_SMART_ASSERT(!temp_ctx_frame_stack_.empty());
    TempCtxFrame &current_frame = temp_ctx_frame_stack_.top();
    current_frame.checkpoint_barriers_.push(current_frame.checkpoints_.size());
}

void
TempCtxHandler::pop_checkpoint_barrier()
{
    DEBUG_SMART_ASSERT(!temp_ctx_frame_stack_.empty());
    TempCtxFrame &current_frame = temp_ctx_frame_stack_.top();
    DEBUG_SMART_ASSERT(!current_frame.checkpoint_barriers_.empty());
    while (current_frame.checkpoints_.size() > current_frame.checkpoint_barriers_.top())
        pop_checkpoint();
    current_frame.checkpoint_barriers_.pop();
    reset_to_checkpoint();
}

std::string
TempCtxHandler::new_name()
{
    DEBUG_SMART_ASSERT(!temp_ctx_frame_stack_.empty());
    return k_temp_variable_prefix + std::to_string(temp_ctx_frame_stack_.top().temp_counter_++);
}

ParseCtx::ParseCtx(SymbolTable *const symbol_table)
    : space_handler(this),
      scope_handler(this),
      call_ctx_handler(this),
      func_ctx_handler(this),
      temp_ctx_handler(this),
      symbol_table_(support::require_ptr(symbol_table)) {}

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
