#include "parser/alpha_semantic_actions.hpp"
#include "parser/alpha_parser_context.hpp"
#include "core/alpha_location.hpp"
#include "core/alpha_types.hpp"
#include "core/alpha_error_tracker.hpp"
#include "misc/smart_assert.h"
#include "core/alpha_macros.hpp"
#include <format>
#include <iostream>
#include <utility>


#define STRINGIFY(_x) #_x

using namespace Alpha;

namespace // Anonymous
{
        class Loop
        {
        public:
                enum class Keyword
                {
                        BREAK,
                        CONTINUE,
                };

                static std::string to_string(Keyword keyword) noexcept
                {
                        switch (keyword)
                        {
                        case Keyword::BREAK:
                                return "break";
                        case Keyword::CONTINUE:
                                return "continue";
                        }
                        UNREACHABLE("Control flow should never reach here");
                }
        };

        void loopcontrol_stmt__loopkeyword_impl(const ParseCtx &parse_ctx, CodeLocation location,
                                                ErrorTracker &error_tracker, Loop::Keyword keyword) noexcept
        {
                if (parse_ctx.ctrl_flow_ctx.loop_depth() > 0)
                        return;

                std::string keyword_name = Loop::to_string(keyword);
                std::string error = std::format("`{}` statement not in a loop statement", keyword_name);
                error_tracker.register_syntax_error(error, location);
        }
} // namespace Anonymous

// +-----------------------------------------------------------------+
// |---------------- SEMANTIC_ACTION_FUNCTIONS_BELOW ----------------|
// +-----------------------------------------------------------------+
void loopcontrol_stmt__break(const ParseCtx &parse_ctx, CodeLocation location,
                             ErrorTracker &error_tracker)
{
        loopcontrol_stmt__loopkeyword_impl(parse_ctx, location, error_tracker, Loop::Keyword::BREAK);
}

void loopcontrol_stmt__continue(const ParseCtx &parse_ctx, CodeLocation location,
                                ErrorTracker &error_tracker)
{
        loopcontrol_stmt__loopkeyword_impl(parse_ctx, location, error_tracker, Loop::Keyword::CONTINUE);
}

void funcctrl_stmt__return(const ParseCtx &parse_ctx, CodeLocation location,
                           ErrorTracker &error_tracker)
{
        if (parse_ctx.ctrl_flow_ctx.function_depth() > 0)
                return;

        std::string error = "`return` statement not in a function statement";
        error_tracker.register_syntax_error(error, location);
}

void lvalue__id(SymbolTable &symbol_table, const ParseCtx &parse_ctx,
                Symbol **lvalue, const std::string &id_name,
                CodeLocation id_location, ErrorTracker &error_tracker)
{
        DEBUG_SMART_ASSERT(!id_name.empty()); // INTERNAL_ERROR: `id_name` must never be empty.

        const Symbol *local_symbol = symbol_table.lookup_local(id_name, parse_ctx.current_scope.value());
        if (local_symbol && local_symbol->is_variable())
                return; // Found in local/formal scope.

        const Symbol *resolved_symbol = symbol_table.lookup_between(id_name, parse_ctx.current_scope.value());
        if (resolved_symbol && resolved_symbol->is_variable())
        {
                std::string error = std::format("variable `{}` outside access region", id_name);
                std::string note = std::format("variable `{}` previously declared here", id_name);
                error_tracker.register_syntax_error(error, id_location, note, resolved_symbol->location());
        }
}