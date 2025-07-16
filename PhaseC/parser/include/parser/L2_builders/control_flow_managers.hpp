#ifndef CONTROL_FLOW_HANDLERS_HPP
#define CONTROL_FLOW_HANDLERS_HPP
#include <diagnostics/diagnostic_reporter.gen.hpp>

#include "parser_context.hpp"
#include "core/source_location.hpp"
#include "L1_driver/semantic_system_support.hpp"
#include "L3_ir_infra/quad_handler.hpp"

namespace Alpha
{
class BranchManager {};

class LoopManager
{
public:
    explicit LoopManager(const SemanticSystemServices &services);

    void process_break(SourceLocation break_loc);
    void process_continue(SourceLocation continue_loc);

private:
    enum class LoopKeyword { CONTINUE, BREAK };

    DiagnosticReporter *const dr_;
    ParseCtx *const parse_ctx_;
    QuadHandler *const quad_handler_;

    void process_loop_keyword(LoopKeyword keyword, SourceLocation keyword_loc);
    bool is_in_loop();

    static const char *to_string(LoopKeyword lk);
};

inline
LoopManager::LoopManager(const SemanticSystemServices &services)
    : dr_(REQUIRE_PTR(services.dr)),
      parse_ctx_(REQUIRE_PTR(services.parse_ctx)),
      quad_handler_(REQUIRE_PTR(services.quad_handler)) {}

inline void
LoopManager::process_break(const SourceLocation break_loc)
{
    process_loop_keyword(LoopKeyword::BREAK, break_loc);
}

inline void
LoopManager::process_continue(const SourceLocation continue_loc)
{
    process_loop_keyword(LoopKeyword::CONTINUE, continue_loc);
}

inline void
LoopManager::process_loop_keyword(const LoopKeyword keyword, const SourceLocation keyword_loc)
{
    if (!is_in_loop())
    {
        dr_->report_loop_ctrl_outside_loop(to_string(keyword), keyword_loc);
        return;
    }
    switch (keyword)
    {
    case LoopKeyword::BREAK:
        parse_ctx_->func_ctx_handler.add_label_to_breaklist(quad_handler_->next_quad_label());
        break;
    case LoopKeyword::CONTINUE:
        parse_ctx_->func_ctx_handler.add_label_to_continuelist(quad_handler_->next_quad_label());
        break;
    default:
        UNREACHABLE(FMT::format("Unknown `LoopKeyword`: int(lk) = ", static_cast<int>(keyword)));
    }
    quad_handler_->emit_labelless(IOPCode::JUMP, nullptr, nullptr, nullptr, keyword_loc);
}

inline bool
LoopManager::is_in_loop() { return parse_ctx_->func_ctx_handler.loop_depth() > 0; }

inline const char *
LoopManager::to_string(const LoopKeyword lk)
{
    switch (lk)
    {
    case LoopKeyword::BREAK: return "break";
    case LoopKeyword::CONTINUE: return "continue";
    default: UNREACHABLE(FMT::format("Unknown `LoopKeyword`: int(lk) = ", static_cast<int>(lk)));
    }
}
} // namespace Alpha
#endif // CONTROL_FLOW_HANDLERS_HPP
