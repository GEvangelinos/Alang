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
        if (host_->hard_error_occurred()) return;
        DEBUG_SMART_ASSERT(variable_offset_stack_.size() == 1);
    )
}

ScopeHandler::ScopeHandler(const ParseCtx *host)
    : host_(support::require_ptr(host))
{
    DEBUG_SMART_ASSERT(
        scope_ == k_global_scope &&
        "Scope should always start from global scope",
        !skip_next_scope_increment_ &&
        "A ToggleSwitch must always be initialized as disabled."
    );
}

ScopeHandler::~ScopeHandler()
{
    DEBUG(
        if (host_->hard_error_occurred()) return;
        DEBUG_SMART_ASSERT(scope_ == k_global_scope);
    )
}

CallCtxHandler::CallCtxHandler(ParseCtx *const host)
    : host_(support::require_ptr(host)) {}

CallCtxHandler::~CallCtxHandler()
{
    DEBUG(
        if (host_->hard_error_occurred()) return;
        DEBUG_SMART_ASSERT(call_nesting_depth_ == 0);
    )
}

TableCtxHandler::TableCtxHandler(ParseCtx *const host)
    : host_(support::require_ptr(host)) {}

TableCtxHandler::~TableCtxHandler()
{
    DEBUG(
        if (host_->hard_error_occurred()) return;
        DEBUG_SMART_ASSERT(dict_entry_nesting_depth_ == 0);
    )
}

bool
FunctionCtxHandler::FlowLivenessTracker::is_in_dead_flow() const noexcept
{
    if (flow_states_.empty())
        return false;
    switch (flow_states_.back())
    {
    case FlowState::ALIVE:
    case FlowState::RUNTIME: return false;
    case FlowState::DEAD: return true;
    default: UNREACHABLE("Unknown flow state");
    }
}

bool
FunctionCtxHandler::FlowLivenessTracker::is_in_runtime_flow() const noexcept
{
    return flow_states_.empty() || flow_states_.back() == FlowState::RUNTIME;
}

void
FunctionCtxHandler::FlowLivenessTracker::push_if_flow_state(const FlowState flow_state)
{
    if (!flow_states_.empty() && flow_states_.back() == FlowState::DEAD)
        flow_states_.push_back(FlowState::DEAD);
    else
        flow_states_.push_back(flow_state);
}

void
FunctionCtxHandler::FlowLivenessTracker::switch_to_else()
{
    DEBUG_SMART_ASSERT(!flow_states_.empty() && "At least last if-branch must have pushed frame");

    if (flow_states_.size() > 1) // Parent exists
    {
        const auto lastest_parent_index = flow_states_.size() - 2;
        if (flow_states_[lastest_parent_index] == FlowState::DEAD)
        {
            DEBUG_SMART_ASSERT(flow_states_.back() == FlowState::DEAD && "dead parent==dead child");
            return; // If parent is dead, then both if and else cases are dead.
        }
    }

    switch (flow_states_.back())
    {
    case FlowState::ALIVE:
        flow_states_.back() = FlowState::DEAD;
        break;
    case FlowState::DEAD:
        flow_states_.back() = FlowState::ALIVE;
        break;
    case FlowState::RUNTIME: // We keep it runtime as its unknown at Compile Time.
        break;
    default: UNREACHABLE("Unknown FlowState");
    }
}

void
FunctionCtxHandler::FlowLivenessTracker::push_loop_flow_state(const FlowState flow_state)
{
    // Dead-code wise, loops are just like if branches... basically both "container" for code
    // You enter at least once, only if their condition holds true at compile time.
    push_if_flow_state(flow_state);
}

void
FunctionCtxHandler::FlowLivenessTracker::pop_flow_state()
{
    DEBUG_SMART_ASSERT(!flow_states_.empty() && "Sync error occurred");
    flow_states_.pop_back();
}

FunctionCtxHandler::FunctionCtxHandler(ParseCtx *const host)
    : host_(support::require_ptr(host))
{
    // We push a stack-frame, for loops that might occur outside functions.
    // So every frame corresponds to a function except the first.
    frame_stack_.emplace(
        k_global_data_frame_name,
        k_global_scope,
        SourceLocation::none(),
        nullptr,
        k_no_label);
}

FunctionCtxHandler::~FunctionCtxHandler()
{
    DEBUG(
        if (host_->hard_error_occurred()) return;
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
    const ProgFuncSymbol *const func_symbol,
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
        if (host_->hard_error_occurred()) return;
        // On normal termination, there should be exactly one slot frame left:
        // the initial frame pushed at construction.
        DEBUG_SMART_ASSERT(temp_frames.size() == 1);
    )

    // The stack of slot handlers should never be completely empty.
    // Even in error cases, grammar actions that push frames run before
    // any possible mismatched symbols, so at least the initial frame survives.
    DEBUG_SMART_ASSERT(!temp_frames.empty());
}

void
TempCtxHandler::push_temp_ctx_frame() { temp_frames.emplace(); }

void
TempCtxHandler::pop_temp_ctx_frame()
{
    DEBUG_SMART_ASSERT(!temp_frames.empty());
    DEBUG(
        if (!host_->hard_error_occurred())
        /**/for (const auto temp_handle : temp_frames.top().temp_handles_)
        /******/DEBUG_SMART_ASSERT(temp_handle == TempFrame::Handle::RELEASED);
    )
    temp_frames.pop();
}

void
TempCtxHandler::reset_temp_ctx_frame()
{
    DEBUG_SMART_ASSERT(!temp_frames.empty());
    pop_temp_ctx_frame();
    push_temp_ctx_frame();
}

TempHandleID
TempCtxHandler::acquire_temp_handle()
{
    static_assert(
        std::numeric_limits<TempHandleID>::min() == 0,
        "TempHandleID must begin at 0 since IDs are used to generate temporary variable names."
    );
    DEBUG_SMART_ASSERT(!temp_frames.empty());

    auto &temp_handles = temp_frames.top().temp_handles_;

    // Scan for available
    for (TempHandleID id = 0; id < temp_handles.size(); ++id)
        if (temp_handles[id] == TempFrame::Handle::RELEASED)
        {
            temp_handles[id] = TempFrame::Handle::ACQUIRED;
            return id;
        }

    // If no available push new.
    temp_handles.emplace_back(TempFrame::Handle::ACQUIRED);
    const auto new_id = temp_handles.size() - 1;
    DEBUG_SMART_ASSERT(static_cast<TempHandleID>(new_id) == new_id && "`new_id` out of range");
    return static_cast<TempHandleID>(new_id);
}

void
TempCtxHandler::release_temp_handle(const TempHandleID id)
{
    DEBUG_SMART_ASSERT(!temp_frames.empty());
    auto &temp_handles = temp_frames.top().temp_handles_;
    DEBUG_SMART_ASSERT(!temp_handles.empty(), id < temp_handles.size());
    temp_handles[id] = TempFrame::Handle::RELEASED;
}

ParseCtx::ParseCtx(SymbolTable *const symbol_table)
    : space_handler(this),
      scope_handler(this),
      table_ctx_handler(this),
      call_ctx_handler(this),
      func_ctx_handler(this),
      temp_ctx_handler(this),
      symbol_table_(support::require_ptr(symbol_table)) {}

std::string
ParseCtx::generate_temp_name(const TempHandleID temp_handle)
    noexcept(noexcept(std::to_string(temp_handle)))
{
    return k_temp_variable_prefix + std::to_string(temp_handle);
}

const VarSymbol *
ParseCtx::new_temp()
{
    const TempHandleID temp_handle = temp_ctx_handler.acquire_temp_handle();
    const std::string temp_name = ParseCtx::generate_temp_name(temp_handle);

    const Symbol *symbol = symbol_table_->lookup_local(temp_name, scope_handler.scope());
    DEBUG_SMART_ASSERT(
        !symbol || symbol->is_variable(),
        !symbol || symbol->is_temp_variable()
    );
    const VarSymbol *var_symbol = static_cast<const VarSymbol *>(symbol);

    // We register new temp, only if current scope doesn't have that temp.
    if (!var_symbol)
    {
        const VarSymbol::Type var_type =
            scope_handler.scope() == k_global_scope
            ? VarSymbol::Type::GLOBAL_VARIABLE
            : VarSymbol::Type::LOCAL_VARIABLE;

        var_symbol = symbol_table_->insert_variable(
            temp_name,
            scope_handler.scope(),
            var_type,
            space_handler.space(),
            space_handler.next_offset(),
            SourceLocation::none()
        );
    }
    symbol_table_->attach_temp_handle(var_symbol, temp_handle);
    return var_symbol;
}

void
ElistCtxHandler::enter_region(const Region r) { region_stack_.push(r); }

void
ElistCtxHandler::exit_region(DEBUG(const Region r))
{
    DEBUG_SMART_ASSERT(!region_stack_.empty());

    // Ensure we are exiting the same region we most recently entered.
    // Regions must be balanced: every enter_region() must be matched
    // with a corresponding exit_region() for the same Region.
    DEBUG_SMART_ASSERT(region() == r && "Mismatched region exit");
    region_stack_.pop();
}

std::optional<ElistCtxHandler::Region>
ElistCtxHandler::region() const
{
    if (region_stack_.empty())
        return std::nullopt;
    return region_stack_.top();
}
} // namespace alpha
