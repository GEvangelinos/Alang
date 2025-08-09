#ifndef CONTROL_FLOW_HANDLERS_HPP
#define CONTROL_FLOW_HANDLERS_HPP

#include <diagnostics/diagnostic_reporter.gen.hpp>
#include <L1_driver/semantic_system_dispatcher_dsl.hpp>
#include "parser_context.hpp"
#include "core/source_location.hpp"
#include "L1_driver/semantic_system_support.hpp"
#include "L3_ir_infra/quad_handler.hpp"

#include  <parser/ir_opcode.hpp>
#include "semantic_subsystem.hpp"

namespace alpha
{
class LoopManager
{
    friend class SemanticSystem;

private:
    enum class LoopKeyword { CONTINUE, BREAK };

    // to_string() function is declared outside class LoopManager so its visible by ADL, to be used by DiagnosticReporter.
    friend const char *to_string(const LoopKeyword lk)
    {
        switch (lk)
        {
        case LoopKeyword::BREAK: return "break";
        case LoopKeyword::CONTINUE: return "continue";
        default: UNREACHABLE(FMT::format("Unknown `LoopKeyword`: int(lk) = ", static_cast<int>(lk)))
            ;
        }
    }

    class Restricted final : private SemanticSubsystem
    {
        friend class LoopManager;

    private:
        explicit Restricted(const SemanticSystemServices &ss_services);

        void process_break(SourceLocation break_loc);
        void process_continue(SourceLocation continue_loc);

        bool is_in_loop();

        template<LoopKeyword keyword>
        void process_loop_keyword(SourceLocation keyword_loc);
    };

    Restricted DISPATCH_TARGET;

    explicit LoopManager(const SemanticSystemServices &ss_services);

    DISPATCH_DEFINE_HANDLER_BEGIN();
    DISPATCH_SLAVE_METHOD_CALL(process_break);
    DISPATCH_SLAVE_METHOD_CALL(process_continue);
    DISPATCH_DEFINE_HANDLER_END();
};

inline void
LoopManager::Restricted::process_break(const SourceLocation break_loc)
{
    process_loop_keyword<LoopKeyword::BREAK>(break_loc);
}

inline void
LoopManager::Restricted::process_continue(const SourceLocation continue_loc)
{
    process_loop_keyword<LoopKeyword::CONTINUE>(continue_loc);
}

inline bool
LoopManager::Restricted::is_in_loop() { return parse_ctx_->func_ctx_handler.loop_depth() > 0; }

template<LoopManager::LoopKeyword keyword>
void LoopManager::Restricted::process_loop_keyword(const SourceLocation keyword_loc)
{
    if (!is_in_loop())
    {
        dr_->report_loop_ctrl_keyword_outside_loop(keyword, keyword_loc);
        return;
    }
    if constexpr (keyword == LoopKeyword::BREAK)
        parse_ctx_->func_ctx_handler.add_label_to_breaklist(quad_handler_->next_quad_label());
    else if constexpr (keyword == LoopKeyword::CONTINUE)
        parse_ctx_->func_ctx_handler.add_label_to_continuelist(quad_handler_->next_quad_label());
    else
        static_assert([]() { return false; }(), "Unknown keyword");

    quad_handler_->emit_labelless(ir::Opcode::JUMP, nullptr, nullptr, nullptr, keyword_loc);
}
} // namespace alpha
#endif // CONTROL_FLOW_HANDLERS_HPP
