#ifndef SEMANTIC_ACTIONS_HPP
#define SEMANTIC_ACTIONS_HPP

#include "core/alpha_error_tracker.hpp"
#include "core/alpha_location.hpp"
#include "parser/alpha_parser_context.hpp"
#include "parser/alpha_symbol_table.hpp"

namespace Alpha
{

    void loopcontrol_stmt__break(
        const PrsrCTX &prsr_ctx,
        CodeLocation code_location,
        ErrorTracker &error_tracker);

    void loopcontrolstmt__continue(
        const PrsrCTX &prsr_ctx,
        CodeLocation code_location,
        ErrorTracker &error_tracker);

    void funcctrl_stmt__return(
        const PrsrCTX &prsr_ctx,
        CodeLocation code_location,
        ErrorTracker &error_tracker);

} /* namespace Alpha */

#endif /* SEMANTIC_ACTIONS_HPP */