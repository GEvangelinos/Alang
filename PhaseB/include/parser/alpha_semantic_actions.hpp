#ifndef SEMANTIC_ACTIONS_HPP
#define SEMANTIC_ACTIONS_HPP

#include "core/alpha_error_tracker.hpp"
#include "core/alpha_location.hpp"
#include "parser/alpha_parser_context.hpp"
#include "parser/alpha_symbol_table.hpp"

using namespace Alpha;

void loopcontrol_stmt__break(
    const ParseCtx &parse_ctx,
    CodeLocation code_location,
    ErrorTracker &error_tracker);

void loopcontrol_stmt__continue(
    const ParseCtx &parse_ctx,
    CodeLocation code_location,
    ErrorTracker &error_tracker);

void funcctrl_stmt__return(
    const ParseCtx &parse_ctx,
    CodeLocation code_location,
    ErrorTracker &error_tracker);

#endif /* SEMANTIC_ACTIONS_HPP */