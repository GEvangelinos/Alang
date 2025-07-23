#ifndef CONTROL_FLOW_HANDLERS_HPP
#define CONTROL_FLOW_HANDLERS_HPP
#include <diagnostics/diagnostic_reporter.gen.hpp>

#include "parser_context.hpp"
#include "core/source_location.hpp"
#include "L1_driver/semantic_system_support.hpp"
#include "L3_ir_infra/quad_handler.hpp"
#include <L1_driver/semantic_system_dispatcher_dsl.hpp>

namespace Alpha
{
class BranchManager {};

class LoopManager
{
public:
    explicit LoopManager(const SemanticSystemServices &services);

    DISPATCH_DECLARE_HANDLER();

    void process_break(SourceLocation break_loc);
    void process_continue(SourceLocation continue_loc);

private:
    enum class LoopKeyword { CONTINUE, BREAK };

    DiagnosticReporter *const dr_;
    ParseCtx *const parse_ctx_;
    QuadHandler *const quad_handler_;

    bool is_in_loop();

    template<LoopKeyword keyword>
    void process_loop_keyword(SourceLocation keyword_loc);

    static const char *to_string(LoopKeyword lk);
};

inline
LoopManager::LoopManager(const SemanticSystemServices &services)
    : dr_(REQUIRE_PTR(services.dr)),
      parse_ctx_(REQUIRE_PTR(services.parse_ctx)),
      quad_handler_(REQUIRE_PTR(services.quad_handler)) {}

DISPATCH_DEFINE_HANDLER_BEGIN(LoopManager);
    DISPATCH_BEGIN_CALLS();
    DISPATCH_SLAVE_METHOD_CALL(process_break);
    DISPATCH_SLAVE_METHOD_CALL(process_continue);
    DISPATCH_END_CALLS();
DISPATCH_DEFINE_HANDLER_END(LoopManager);

inline void
LoopManager::process_break(const SourceLocation break_loc)
{
    process_loop_keyword<LoopKeyword::BREAK>(break_loc);
}

inline void
LoopManager::process_continue(const SourceLocation continue_loc)
{
    process_loop_keyword<LoopKeyword::CONTINUE>(continue_loc);
}

inline bool
LoopManager::is_in_loop() { return parse_ctx_->func_ctx_handler.loop_depth() > 0; }

template<LoopManager::LoopKeyword keyword>
void LoopManager::process_loop_keyword(const SourceLocation keyword_loc)
{
    if (!is_in_loop())
    {
        dr_->report_loop_ctrl_keyword_outside_loop(to_string(keyword), keyword_loc);
        return;
    }
    if constexpr (keyword == LoopKeyword::BREAK)
        parse_ctx_->func_ctx_handler.add_label_to_breaklist(quad_handler_->next_quad_label());
    else if constexpr (keyword == LoopKeyword::CONTINUE)
        parse_ctx_->func_ctx_handler.add_label_to_continuelist(quad_handler_->next_quad_label());
    else
        static_assert([]() { return false; }(), "Unknown keyword");

    quad_handler_->emit_labelless(IOPCode::JUMP, nullptr, nullptr, nullptr, keyword_loc);
}
} // namespace Alpha
#endif // CONTROL_FLOW_HANDLERS_HPP
