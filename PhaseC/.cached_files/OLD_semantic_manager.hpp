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

inline void SemanticManager::N(Location n_loc, const int N_index)
{
    auto &qh = parse_ctx_.quad_handler;
    if (N_index == 1)
        parse_ctx_.cache.n.quads_to_patch_1.push(qh.next_quad_label());
    if (N_index == 2)
        parse_ctx_.cache.n.quads_to_patch_2.push(qh.next_quad_label());
    if (N_index == 3)
        parse_ctx_.cache.n.quads_to_patch_3.push(qh.next_quad_label());
    qh.emit_quad_labelless(IOPCode::JUMP, nullptr, nullptr, nullptr, n_loc);
}

inline void SemanticManager::M()
{
    auto &qh = parse_ctx_.quad_handler;
    parse_ctx_.cache.m.quads_to_patch.push(qh.next_quad_label());
}

inline void SemanticManager::forStmt__forHeader() noexcept
{
    parse_ctx_.function_ctx_handler.enter_loop();
}

inline void SemanticManager::forHeader__for_lparen_elist_semicolon_m_expr_semicolon(
    Expr *expr, Location expr_loc)
{
    // TODO you probably dont need stack here. (like 99%) M is just a variable..
    // as we use it before any new for-loops can begin.
    DEBUG_SMART_ASSERT(parse_ctx_.cache.m.quads_to_patch.size() > 0);
    parse_ctx_.cache.for_header.test_quads_to_patch.push(
        parse_ctx_.cache.m.quads_to_patch.top());
    parse_ctx_.cache.m.quads_to_patch.pop();

    parse_ctx_.cache.for_header.enter_quads_to_patch.push(
        parse_ctx_.quad_handler.next_quad_label());

    Expr *true_expr = parse_ctx_.expr_handler.make_expr_const_bool(true, expr_loc);
    parse_ctx_.quad_handler.emit_quad_labelless(IOPCode::IF_EQ, expr, true_expr, nullptr,
                                                expr_loc);
}

// MASSIVE TODO: REFACTOR WHOLE SEMANTIC_BUILDER/MANAGER.. its a HUGE MESS..
// for HANDLING IS A HUGE MESS TOO.. UNREADABLE UNCLEAR WHAT YOU DO.
// GOTO SLEEP AND TAKE SOME TIME TO FIX THIS.. ITS A NICE PROJECT... SO DO YOURSELF A FAVOR TO FIX
// THIS DEAR TA. IF  YOU ARE SEEING THIS.. I AM SORRY.. THIS IS JUST HOW I WRITE THINGS.. AND HOW I
// TALK TO MYSELF
inline void SemanticManager::forStmt__forHeader_stmt() noexcept
{
    // TODO: just a though.. In code it shows using IF_EQ.. but we used IF_NOTEQ
    // in order to reduces quads.. (while keeping same program behavior..)
    // The this is.. that in IF, ELSe and WHILE I ommitted a jump.. What about here?
    // Is any jump redundant?  // Track generated code. and see is some QUAD is never run.
    // On the following test: generated quads are used (I check manually.. no one was left out,
    // but keep searching... weird to just use IF_NOTEQ from IF_EQ, and not needing to make
    // something else different) (Then again maybe its a behavior of the loops.. because we
    // always return up (cycle)) .. Maybe in while I did just that.. I am sleepless.. So I might
    // very well be wrong

    // for ( i = 0; i < N; ++i )
    //        print("*");

    auto &qh = parse_ctx_.quad_handler;

    DEBUG_SMART_ASSERT(                                              //
        parse_ctx_.cache.for_header.test_quads_to_patch.size() > 0,  //
        parse_ctx_.cache.for_header.enter_quads_to_patch.size() > 0, //
        parse_ctx_.cache.n.quads_to_patch_3.size() > 0,              //
        parse_ctx_.cache.n.quads_to_patch_2.size() > 0,              //
        parse_ctx_.cache.n.quads_to_patch_1.size() > 0,              //

    );

    qh.patch_quad(                                              //
        parse_ctx_.cache.for_header.enter_quads_to_patch.top(), //
        parse_ctx_.cache.n.quads_to_patch_2.top() + 1           //
    );

    qh.patch_quad(                                 //
        parse_ctx_.cache.n.quads_to_patch_1.top(), //
        qh.next_quad_label()                       //
    );

    qh.patch_quad(                                            //
        parse_ctx_.cache.n.quads_to_patch_2.top(),            //
        parse_ctx_.cache.for_header.test_quads_to_patch.top() //
    );

    qh.patch_quad(                                    //
        parse_ctx_.cache.n.quads_to_patch_3.top(),    //
        parse_ctx_.cache.n.quads_to_patch_1.top() + 1 //
    );

    qh.patch_list(parse_ctx_.function_ctx_handler.get_breaklist(), qh.next_quad_label());
    qh.patch_list(parse_ctx_.function_ctx_handler.get_continuelist(),
                  parse_ctx_.cache.n.quads_to_patch_1.top() + 1);

    parse_ctx_.cache.for_header.test_quads_to_patch.pop();
    parse_ctx_.cache.for_header.enter_quads_to_patch.pop();
    parse_ctx_.cache.n.quads_to_patch_3.pop();
    parse_ctx_.cache.n.quads_to_patch_2.pop();
    parse_ctx_.cache.n.quads_to_patch_1.pop();

    parse_ctx_.function_ctx_handler.exit_loop(); // This kills break and continue lists.
}

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
