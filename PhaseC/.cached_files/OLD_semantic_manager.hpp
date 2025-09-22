#ifndef OLD_SEMANTIC_MANAGER_HPP
#define OLD_SEMANTIC_MANAGER_HPP
#ifndef SEMANTIC_MANAGER_HPP
#define SEMANTIC_MANAGER_HPP

#include "core/alpha_error.hpp"            // for ErrorTracker
#include "core/alpha_location.hpp"         // for Location
#include "parser/alpha_parser_context.hpp" // for ParseCtx
#include "parser/alpha_symbol_table.hpp"   // for Symbol, SymbolTable
#include <string>                          // for string

#include <list> // for list, _List_const_iterator

#include "core/alpha_error.hpp"      // for ErrorTracker, Diagnostic
#include "core/alpha_konstants.hpp"  // for k_global_scope, k_public_...
#include "core/alpha_location.hpp"   // for Location
#include "core/alpha_types.hpp"      // for u32
#include "parser/_parser_common.hpp" // for Parameter
#include "parser/alpha_backpatcher.hpp"
#include "parser/alpha_parser_context.hpp" // for ParseCtx
#include "support/format_adapter.hpp"        // for format, FMT
#include "support/misc_tools.hpp"                  // for DEBUG_ALWAYS_INLINE
#include "support/smart_assert.h"            // for DEBUG_SMART_ASSERT

namespace Alpha
{
class SemanticManager
{
public:
    SemanticManager(ParseCtx &parse_ctx, SymbolTable &st, ErrorTracker &et);
    void funcCtrlStmt__return(Location return_loc);
    void returnStmt__return(Location returnStmt_loc, Location return_loc);
    void returnStmt__return_expr(Expr *expr, Location returnStmt_loc, Location return_loc);
}; // class SemanticManager

// MASSIVE TODO: REFACTOR WHOLE SEMANTIC_MODEER/MANAGER.. its a HUGE MESS..
// for HANDLING IS A HUGE MESS TOO.. UNREADABLE UNCLEAR WHAT YOU DO.
// GOTO SLEEP AND TAKE SOME TIME TO FIX THIS.. ITS A NICE PROJECT... SO DO YOURSELF A FAVOR TO FIX
// THIS DEAR TA. IF  YOU ARE SEEING THIS.. I AM SORRY.. THIS IS JUST HOW I WRITE THINGS.. AND HOW I
// TALK TO MYSELF
//FUTURE COMMENT BEFORE I DELETE:
// (Well it took me 2-3 months :D (well i added lots of stuff.. and had lots of personal problems.. but STILL,)
// WE FREAKING MADE IT!

} // namespace alpha

#endif /* SEMANTIC_MANAGER_HPP */

#endif //OLD_SEMANTIC_MANAGER_HPP
