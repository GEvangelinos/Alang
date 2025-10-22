#include "L1_driver/semantic_system.hpp"

#include <set>

#include "L2_semantic_subsystems/core/expr_normalizer.hpp"

namespace alpha
{
SemanticSystem::SemanticSystem(
    const settings::ExprOpts &expr_opts,
    ParseCtx *const parse_ctx,
    SymbolTable *const symbol_table,
    DiagnosticReporter *const dr)
    :
    // External components, required to initialize class.
    parse_ctx_(support::require_ptr(parse_ctx)),
    symbol_table_(support::require_ptr(symbol_table)),
    dr_(support::require_ptr(dr)),

    // Private components, used by public submodules.
    expr_maker_(std::make_unique<ExprMaker>(parse_ctx)),
    quad_handler_(std::make_unique<QuadHandler>()),
    quad_interceptor_(std::make_unique<QuadInterceptor>(quad_handler_.get(), parse_ctx)),
    quad_yielder_(std::make_unique<QuadYielder>(
        parse_ctx,
        symbol_table,
        expr_maker_.get(),
        quad_handler_.get(),
        quad_interceptor_.get()
    )),
    expr_normalizer_(std::make_unique<ExprNormalizer>(
        parse_ctx,
        expr_maker_.get(),
        quad_handler_.get(),
        quad_interceptor_.get(),
        quad_yielder_.get()
    )),
    expr_optimizer_(std::make_unique<ExprOptimizer>(expr_opts, expr_maker_.get())),

    // public (through call() dispatcher) servicers, used by users of semantic driver.
    aggregate_builder(create_semantic_system_services()),
    assign_builder(get_assign_builder_options(expr_opts), create_semantic_system_services()),
    basic_builder(get_basic_builder_options(expr_opts), create_semantic_system_services()),
    block_manager(create_semantic_system_services()),
    call_builder(create_semantic_system_services()),
    const_builder(create_semantic_system_services()),
    control_flow_manager(create_semantic_system_services()),
    lvalue_resolver(create_semantic_system_services()),
    function_builder(create_semantic_system_services()),
    table_access_builder(create_semantic_system_services()),

    // public resources used by external components.
    gateway(std::unique_ptr<Gateway>(new Gateway(this))),
    context_inspector(std::unique_ptr<ContextInspector>(new ContextInspector(this))) {}

SemanticSystemServices
SemanticSystem::create_semantic_system_services()
{
    return {
        .symbol_table = support::require_ptr(symbol_table_),
        .parse_ctx = support::require_ptr(parse_ctx_),
        .dr = support::require_ptr(dr_),
        .expr_maker = support::require_ptr(expr_maker_.get()),
        .quad_handler = support::require_ptr(quad_handler_.get()),
        .quad_yielder = support::require_ptr(quad_yielder_.get()),
        .expr_normalizer = support::require_ptr(expr_normalizer_.get()),
        .expr_optimizer = support::require_ptr(expr_optimizer_.get()),
    };
}

AssignBuilder::Options
SemanticSystem::get_assign_builder_options(const settings::ExprOpts &expr_opts)
{
    return {
        // constant propagation requires recording of constants inside Expr(essions)
        .record_constant_variables = expr_opts.opt_const_propagation
    };
}

BasicBuilder::Options
SemanticSystem::get_basic_builder_options(const settings::ExprOpts &expr_opts)
{
    return {
        .fold_static_bools = expr_opts.opt_const_eval
    };
}

void
SemanticSystem::reset_stmt_context() noexcept
{
    parse_ctx_->temp_ctx_handler.reset_temp_ctx_frame();
}

void
SemanticSystem::consume_stmt_expr(const Expr *expr)
{
    // TODO: Why materialize? Remove it when you have enough tests, to see response.
    expr = expr_normalizer_->materialize_if_table_item(expr);
    expr_normalizer_->resolve_bool_short_circuit(expr);
    quad_yielder_->release_temp_handle_if_active(expr);
}

void
SemanticSystem::commit_expr_of_elist(const Expr *expr)
{
    const auto opt_region = parse_ctx_->elist_ctx_handler.region();

    DEBUG_SMART_ASSERT(opt_region.has_value() && "Without a region value routing is impossible");
    switch (opt_region.value())
    {
    case ElistCtxHandler::Region::CALL:
        call_builder.commit_call_argument(expr);
        break;
    case ElistCtxHandler::Region::FORLOOP_CLAUSE:
        control_flow_manager.commit_forloop_header_expr(expr);
        break;
    case ElistCtxHandler::Region::TABLE:
        aggregate_builder.commit_list_element(expr);
        break;
    default: UNREACHABLE("Unknown Region, please register");
    }
}

const Expr *
SemanticSystem::force_rvalue_cast(const Expr *const expr, const SourceLocation cast_loc)
{
    const Expr *const result = expr_maker_->clone_with_updated_location(cast_loc, expr);
    result->rvalue_cast();
    return result;
}

SemanticSystem::Gateway::Gateway(SemanticSystem *const ss)
    : host_(support::require_ptr(ss)) {}

void
SemanticSystem::Gateway::notify_hard_error() noexcept
{
    host_->ss_status_ = SemanticSystem::Status::ERROR;
    host_->parse_ctx_->hard_error_occurred_.raise();
}

std::vector<Quad>
SemanticSystem::Gateway::extract_quads()
{
    if (extracted_quads)
        throw std::logic_error(ATTACH_CONTEXT(
            "Quad extraction must only happen once at the end of parsing (Quads are already extracted)"
        ));
    extracted_quads.raise();
    return host_->quad_handler_->extract_quads();
}

SemanticSystem::ContextInspector::ContextInspector(SemanticSystem *const ss)
    : host_(support::require_ptr(ss)) {}

bool
SemanticSystem::ContextInspector::is_in_call_arg_list() const noexcept
{
    return host_->parse_ctx_->call_ctx_handler.is_in_call();
}

bool
SemanticSystem::ContextInspector::is_in_forloop_clause() const noexcept
{
    const auto &region = host_->parse_ctx_->elist_ctx_handler.region();
    return region.has_value() && *region == ElistCtxHandler::Region::FORLOOP_CLAUSE;
}

bool
SemanticSystem::ContextInspector::is_in_func_param_list() const noexcept
{
    return host_->parse_ctx_->space_handler.space() == VarSymbol::Space::FORMAL_ARGUMENT;
}

bool
SemanticSystem::ContextInspector::is_in_table_dict() const noexcept
{
    if (!host_->parse_ctx_->elist_ctx_handler.region().has_value())
        return false;
    return host_->parse_ctx_->elist_ctx_handler.region().value() == ElistCtxHandler::Region::TABLE;
}
} // namespace alpha
