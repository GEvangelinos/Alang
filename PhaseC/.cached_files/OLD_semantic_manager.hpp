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
#include "utils/format_adapter.hpp"        // for format, FMT
#include "utils/misc.hpp"                  // for DEBUG_ALWAYS_INLINE
#include "utils/smart_assert.h"            // for DEBUG_SMART_ASSERT

namespace Alpha
{
class SemanticManager
{
public:
    SemanticManager(ParseCtx &parse_ctx, SymbolTable &st, ErrorTracker &et);
    void N(Location n_loc, const int N_index);
    void M();
    void forHeader__for_lparen_elist_semicolon_m_expr_semicolon(Expr *expr, Location expr_loc);
    void forStmt__forHeader() noexcept;
    void forStmt__forHeader_stmt() noexcept;
    void funcCtrlStmt__return(Location return_loc);
    void returnStmt__return(Location returnStmt_loc, Location return_loc);
    void returnStmt__return_expr(Expr *expr, Location returnStmt_loc, Location return_loc);
}; // class SemanticManager

// MASSIVE TODO: REFACTOR WHOLE SEMANTIC_BUILDER/MANAGER.. its a HUGE MESS..
// for HANDLING IS A HUGE MESS TOO.. UNREADABLE UNCLEAR WHAT YOU DO.
// GOTO SLEEP AND TAKE SOME TIME TO FIX THIS.. ITS A NICE PROJECT... SO DO YOURSELF A FAVOR TO FIX
// THIS DEAR TA. IF  YOU ARE SEEING THIS.. I AM SORRY.. THIS IS JUST HOW I WRITE THINGS.. AND HOW I
// TALK TO MYSELF

inline void SemanticManager::funcCtrlStmt__return(const Location return_loc)
{
    // TODO functionize like you will do with continue and return...
    if (parse_ctx_.function_ctx_handler.function_nesting_depth() > 0)
        return;
    std::string error = "`return` statement not in a function statement";
    et_.report_error(CTError::Type::SEMANTIC, error, return_loc);

    parse_ctx_.function_ctx_handler.add_label_to_returnlist(
        parse_ctx_.quad_handler.next_quad_label());

    parse_ctx_.quad_handler.emit_quad_labelless(IOPCode::JUMP, nullptr, nullptr, nullptr,
                                                return_loc);
}

inline void SemanticManager::returnStmt__return(Location returnStmt_loc, Location return_loc)
{
    // TODO functionize like you will do with continue and return...
    if (parse_ctx_.function_ctx_handler.function_nesting_depth() > 0)
    {
        parse_ctx_.quad_handler.emit_quad(IOPCode::RETURN, nullptr, nullptr, nullptr,
                                          returnStmt_loc);

        // Label goes to JUMP IOPC not RETURN
        parse_ctx_.function_ctx_handler.add_label_to_returnlist(
            parse_ctx_.quad_handler.next_quad_label());
        parse_ctx_.quad_handler.emit_quad_labelless(IOPCode::JUMP, nullptr, nullptr,
                                                    nullptr, return_loc);
        return;
    }
    std::string error = "`return` statement not in a function statement";
    et_.report_error(CTError::Type::SEMANTIC, error, return_loc);
}

inline void SemanticManager::returnStmt__return_expr(Expr *expr, Location returnStmt_loc,
                                                     Location return_loc)
{
    // TODO functionize like you will do with continue and return...
    if (parse_ctx_.function_ctx_handler.function_nesting_depth() > 0)
    {
        parse_ctx_.quad_handler.emit_quad(IOPCode::RETURN, expr, nullptr, nullptr,
                                          returnStmt_loc);

        // Label goes to JUMP IOPC not RETURN
        parse_ctx_.function_ctx_handler.add_label_to_returnlist(
            parse_ctx_.quad_handler.next_quad_label());
        parse_ctx_.quad_handler.emit_quad_labelless(IOPCode::JUMP, nullptr, nullptr,
                                                    nullptr, return_loc);
        return;
    }
    std::string error = "`return` statement not in a function statement";
    et_.report_error(CTError::Type::SEMANTIC, error, return_loc);
}

} // namespace alpha

#endif /* SEMANTIC_MANAGER_HPP */

#endif //OLD_SEMANTIC_MANAGER_HPP
