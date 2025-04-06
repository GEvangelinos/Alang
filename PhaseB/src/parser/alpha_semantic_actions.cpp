#include "parser/alpha_semantic_actions.hpp"
#include "parser/alpha_parser_context.hpp"
#include "core/alpha_location.hpp"
#include "core/alpha_types.hpp"
#include "core/alpha_error_tracker.hpp"
#include "misc/smart_assert.h"
#include <format>
#include <iostream>
#include <utility>
#include <iostream>

#define REPORT_INTERNAL_ERROR(_message)                                                                 \
        do                                                                                              \
        {                                                                                               \
                std::cerr << std::format("{}:{} | {}() --> {}", __FILE__, __LINE__, __func__, _message) \
                          << std::endl;                                                                 \
        } while (0)

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
                        KEYWORD_COUNT
                };

                static std::string to_string(Keyword keyword) noexcept
                {
                        static_assert(static_cast<int>(Keyword::KEYWORD_COUNT) == 2);
                        switch (keyword)
                        {
                        case Keyword::BREAK:
                                return "break";
                        case Keyword::CONTINUE:
                                return "continue";
                        case Keyword::KEYWORD_COUNT:
                                REPORT_INTERNAL_ERROR(STRINGIFY(Keyword::KEYWORD_COUNT) " is a counter for keyword, should not be used");
                                UNREACHABLE();
                        }
                        REPORT_INTERNAL_ERROR("Control flow should never reach here");
                        UNREACHABLE();
                }
        };

        void loopcontrol_stmt__loopkeyword_impl(const PrsrCTX &prsr_ctx, CodeLocation location,
                                                ErrorTracker &error_tracker, Loop::Keyword keyword) noexcept
        {
                if (prsr_ctx.loop_depth() > INITIAL_LOOP_NESTING_DEPTH)
                        return;

                std::string keyword_name = Loop::to_string(keyword);
                std::string error = std::format("`{}` statement not in a loop statement", keyword_name);
                error_tracker.register_syntax_error(error, location);
        }
} // namespace Anonymous

// +-----------------------------------------------------------------+
// |---------------- SEMANTIC_ACTION_FUNCTIONS_BELOW ----------------|
// +-----------------------------------------------------------------+
void loopcontrol_stmt__break(const PrsrCTX &prsr_ctx, CodeLocation location,
                             ErrorTracker &error_tracker)
{
        loopcontrol_stmt__loopkeyword_impl(prsr_ctx, location, error_tracker, Loop::Keyword::BREAK);
}

void loopcontrol_stmt__continue(const PrsrCTX &prsr_ctx, CodeLocation location,
                                ErrorTracker &error_tracker)
{
        loopcontrol_stmt__loopkeyword_impl(prsr_ctx, location, error_tracker, Loop::Keyword::CONTINUE);
}

void funcctrl_stmt__return(const PrsrCTX &prsr_ctx, CodeLocation location,
                           ErrorTracker &error_tracker)
{
        if (prsr_ctx.function_nesting_depth() > INITIAL_FUNCTION_NESTING_DEPTH)
                return;

        std::string error = "`return` statement not in a function statement";
        error_tracker.register_syntax_error(error, location);
}

void lvalue__id(SymbolTable &symbol_table, const PrsrCTX &prsr_ctx,
                Symbol **lvalue, const std::string &id_name,
                CodeLocation id_location, ErrorTracker &error_tracker)
{
        DEBUG_SMART_ASSERT(!id_name.empty()); // INTERNAL_ERROR: `id_name` must never be empty.
        // For lookup to success:
        // 1) ID can be in current (local/formal scope) (reference)
        // 2) ID can be in global scope. (reference)
        // 3) If not in local/formal scope and global,  and in outer middle scopes (not global)
        //    then found and error.

        // Or search with LOOKUP_VARIABLE and then check its scope,
        // if local what? if mid what? if global what?

        const Symbol *local_symbol = symbol_table.lookup_local(id_name, prsr_ctx.current_scope());
        if (local_symbol && local_symbol->is_variable())
                return; // Found in local/formal scope.

        const Symbol *resolved_symbol = symbol_table.lookup_between(id_name, prsr_ctx.current_scope());
        if (resolved_symbol && resolved_symbol->is_variable())
        {
                std::string error = std::format("variable `{}` outside access region", id_name);
                std::string note = std::format("variable `{}` previously declared here", resolved_symbol->name());
                error_tracker.register_syntax_error(error, id_location, note, resolved_symbol->location());
        }
}