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

namespace // (Anonymous)
{
namespace Loop
{
    enum class Keyword
    {
        BREAK,
        CONTINUE,
    };

    [[nodiscard]] std::string to_string(const Keyword keyword) noexcept
    {
        switch (keyword)
        {
        case Keyword::BREAK: return "break";
        case Keyword::CONTINUE:
            return "continue";
            [[unlikely]]
        default:
            SMART_ASSERT(false);
        }
    }
}; // namespace Loop
}  // namespace

namespace Alpha
{
class SemanticManager
{
public:
    SemanticManager(ParseCtx &parse_ctx, SymbolTable &st, ErrorTracker &et);
    void N(Location n_loc, const int N_index);
    void M();
    void forHeader__for_lparen_elist_semicolon_m_expr_semicolon(Expr *expr, Location expr_loc);
    void methodCallId__methodcall_id(const char *id, Location id_loc, Location method_call_loc);
    void funcPrefix__function(Location anonymous_loc);
    void funcPrefix__function_id(const char *id_name, Location id_loc);
    void funcSignature__funcPrefix_funcArgList(const Function *&funcSignature);
    void funcDef__funcSignature_block(const BlockLocation &block_loc) noexcept;
    void funcArgs__id(const char *id_name, Location id_loc);
    void ifPrefix__if_lparen_expr_rparen(Expr *expr, Location expr_loc);
    void ifStmt__ifPrefix_stmt_then();
    void elsePrefix__else(Location else_loc);
    void ifStmt__ifPrefix_stmt_elsePrefix_stmt();
    void whileStart__while();
    void whileCondition__lparen_expr_rparen(Expr *expr, Location expr_loc,
                                            Location while_cond_loc);
    void whileStmt__whileHeader() noexcept;
    void whileStmt__whileHeader_stmt(Location while_stmt_header);
    void forStmt__forHeader() noexcept;
    void forStmt__forHeader_stmt() noexcept;
    void funcCtrlStmt__return(Location return_loc);
    void returnStmt__return(Location returnStmt_loc, Location return_loc);
    void returnStmt__return_expr(Expr *expr, Location returnStmt_loc, Location return_loc);

private:
    [[nodiscard]] bool reported_function_name_conflict(const std::string &function_name,
                                                       u32 current_scope, Location id_loc);
    void insert_gathered_function_parameters();
    [[nodiscard]] bool reported_parameter_name_conflict(u32 current_scope,
                                                        const Parameter &parameter);
}; // class SemanticManager

inline bool SemanticManager::reported_function_name_conflict(const std::string &function_name,
                                                             const u32 current_scope,
                                                             const Location id_loc)
{
    if (st_.is_lib_function(function_name))
    {
        const std::string error =
                FMT::format("redefinition of library function `{}`", function_name);
        et_.report_error(CTError::Type::SEMANTIC, error, id_loc);
        return true;
    }

    const Symbol *found_symbol = st_.lookup_local(function_name, current_scope);
    if (!found_symbol)
        return false;
    if (found_symbol->is_function())
    {
        const std::string error =
                FMT::format("redefinition of `function {}`", function_name);
        const std::string note =
                FMT::format("previous definition of `function {}` here", function_name);
        et_.report_error(CTError::Type::SEMANTIC, error, id_loc, note,
                         found_symbol->location);
    }
    else if (found_symbol->is_variable())
    {
        const std::string error =
                FMT::format("`{}` redefined as a function", function_name);
        const std::string note =
                FMT::format("`{}` previously defined as a variable here", function_name);
        et_.report_error(CTError::Type::SEMANTIC, error, id_loc, note,
                         found_symbol->location);
    }
    return true;
}

inline void SemanticManager::insert_gathered_function_parameters()
{
    auto current_scope = parse_ctx_.scope_handler.scope();
    constexpr auto space = Variable::Space::FORMAL_ARGUMENT;
    DEBUG_SMART_ASSERT(parse_ctx_.space_handler.space() == Variable::Space::FORMAL_ARGUMENT);

    for (const Parameter &param: parse_ctx_.function_ctx_handler.function_parameters())
        if (!reported_parameter_name_conflict(current_scope, param))
            st_.insert_variable(param.name, current_scope,
                                Variable::Type::FORMAL_ARGUMENT, space,
                                parse_ctx_.space_handler.next_offset(), param.location);
}

inline bool SemanticManager::reported_parameter_name_conflict(const u32 current_scope,
                                                              const Parameter &parameter)
{
    // Library‐function conflict
    if (st_.is_lib_function(parameter.name))
    {
        const std::string error = FMT::format(
            "`{}` is a library function, can't declare it as formal", parameter.name);
        et_.report_error(CTError::Type::SEMANTIC, error, parameter.location);
        return true;
    }
    const Symbol *formal_symbol = st_.lookup_local(parameter.name, current_scope);
    // Parameter‐redeclared conflict
    if (formal_symbol)
    {
        // Parameter should produce name conflicts only with themselves.
        DEBUG_SMART_ASSERT(                                  //
            !!dynamic_cast<const Variable *>(formal_symbol), //
            formal_symbol->is_variable()                     //
        );

        const std::string error =
                FMT::format("redefinition of parameter `{}`", parameter.name);
        const std::string note =
                FMT::format("previous definition of `{}` here", parameter.name);
        et_.report_error(CTError::Type::SEMANTIC, error, parameter.location, note,
                         formal_symbol->location);
        return true;
    }
    return false;
}

inline void SemanticManager::methodCallId__methodcall_id(const char *id, const Location id_loc,
                                                         const Location method_call_loc)
{
    parse_ctx_.cache.method_call_id.id = id;
    parse_ctx_.cache.method_call_id.id_location = id_loc;
    parse_ctx_.cache.method_call_id.method_call_location = method_call_loc;
}

inline void SemanticManager::funcPrefix__function(const Location anonymous_loc)
{
    // Update ParseCache:
    parse_ctx_.cache.func_prefix.id = parse_ctx_.name_generator.new_anonymous();
    parse_ctx_.cache.func_prefix.location = anonymous_loc;

    parse_ctx_.space_handler.enter_space();
}

inline void SemanticManager::funcPrefix__function_id(const char *id_name, const Location id_loc)
{
    // Update ParseCache
    parse_ctx_.cache.func_prefix.id = id_name;
    parse_ctx_.cache.func_prefix.location = id_loc;

    parse_ctx_.space_handler.enter_space();
}

/// Handles a function signature’s prefix + argument list.
///
/// If a name conflict is detected, we still need to call
/// enter_function() (to keep our frame‐stack balanced), but
/// we must *not* back-patch the local-variable count or we
/// ’ll end up polluting the original function’s frame with
/// local_variable_count from the redefinition.
inline void SemanticManager::funcSignature__funcPrefix_funcArgList(const Function *&funcSignature)
{
    const Location func_loc = parse_ctx_.cache.func_prefix.location;
    bool conflicting_name = reported_function_name_conflict(
        parse_ctx_.cache.func_prefix.id, parse_ctx_.scope_handler.scope(), func_loc);

    // TODO: TRY putting jump only when there is no name_conflict...
    // as now we jump for no reason. But thenwe throw error.
    // So quads dont really matter.. But anyway. Tidy thisfunction up.
    // Its pure eye-pain.

    const u32 label_of_jump = parse_ctx_.quad_handler.next_quad_label();
    parse_ctx_.quad_handler.emit_quad_labelless(IOPCode::JUMP, nullptr, nullptr, nullptr,
                                                func_loc);
    const Function *function_symbol = nullptr;
    if (!conflicting_name)
    {
        function_symbol = st_.insert_function(
            parse_ctx_.cache.func_prefix.id, parse_ctx_.scope_handler.scope(),
            parse_ctx_.function_ctx_handler.next_function_address(),
            parse_ctx_.function_ctx_handler.function_parameters(), func_loc);

        parse_ctx_.quad_handler.emit_quad(
            IOPCode::FUNCSTART, nullptr, nullptr,
            parse_ctx_.expr_handler.make_expr_variable(function_symbol, func_loc),
            func_loc);
    }

    parse_ctx_.function_ctx_handler.enter_function(function_symbol, label_of_jump);
    insert_gathered_function_parameters();
    parse_ctx_.function_ctx_handler.clear_function_parameters();
    parse_ctx_.space_handler
              .enter_space(); // IMPORTANT: This line is after parameter insertion!

    funcSignature = function_symbol;
}

inline void SemanticManager::funcDef__funcSignature_block(const BlockLocation &block_loc) noexcept
{
    parse_ctx_.quad_handler.patch_list(parse_ctx_.function_ctx_handler.get_returnlist(),
                                       parse_ctx_.quad_handler.next_quad_label());
    auto fbi = parse_ctx_.function_ctx_handler.exit_function();
    if (!!fbi.function_symbol)
    {
        Backpatcher::set_function_local_variable_count(fbi.function_symbol,
                                                       fbi.local_variable_count);

        parse_ctx_.quad_handler.emit_quad(
            IOPCode::FUNCEND, nullptr, nullptr,
            parse_ctx_.expr_handler.make_expr_variable(
                fbi.function_symbol, k_no_location), // TODO: what location here?
            block_loc.end);
    }

    parse_ctx_.quad_handler.patch_quad(fbi.label_to_jump,
                                       parse_ctx_.quad_handler.next_quad_label());

    parse_ctx_.space_handler.exit_space();
}

inline void SemanticManager::funcArgs__id(const char *id_name, const Location id_loc)
{
    parse_ctx_.function_ctx_handler.add_function_parameter(id_name, id_loc);
}

inline void SemanticManager::ifPrefix__if_lparen_expr_rparen(Expr *expr, const Location expr_loc)
{
    auto &eh = parse_ctx_.expr_handler;
    auto &qh = parse_ctx_.quad_handler;

    const Expr *true_expr = eh.make_expr_const_bool(true, expr_loc);

    // TODO: when you inverseto IF_NOTEQ +2  cause we want to go over jump.
    // I've done the flow graph on paper.
    qh.emit_quad_w_jump_step(IOPCode::IF_EQ, expr, true_expr, 2, expr_loc);

    parse_ctx_.cache.if_prefix.quads_to_patch.push(qh.next_quad_label());
    qh.emit_quad_labelless(IOPCode::JUMP, nullptr, nullptr, nullptr, expr_loc);
}

inline void SemanticManager::ifStmt__ifPrefix_stmt_then()
{
    auto &qh = parse_ctx_.quad_handler;
    u32 quad_to_patch = parse_ctx_.cache.if_prefix.quads_to_patch.top();
    parse_ctx_.cache.if_prefix.quads_to_patch.pop();
    qh.patch_quad(quad_to_patch, qh.next_quad_label());
}

inline void SemanticManager::elsePrefix__else(Location else_loc)
{
    auto &qh = parse_ctx_.quad_handler;
    parse_ctx_.cache.else_prefix.quads_to_patch.push(qh.next_quad_label());
    qh.emit_quad_labelless(IOPCode::JUMP, nullptr, nullptr, nullptr, else_loc);

    // TODO for if_noteq:
    // u32 if_noteq_quad_to_patch = parse_ctx_.cache.if_prefix.quads_to_patch.top();
    // parse_ctx_.cache.if_prefix.quads_to_patch.pop();
    // qh.patch_quad(if_noteq_quad_to_patch, qh.next_quad_label());
}

inline void SemanticManager::ifStmt__ifPrefix_stmt_elsePrefix_stmt()
{
    auto &qh = parse_ctx_.quad_handler;

    // patchlabel($ifprefix, $elseprefix + 1);
    qh.patch_quad(parse_ctx_.cache.if_prefix.quads_to_patch.top(),
                  parse_ctx_.cache.else_prefix.quads_to_patch.top());

    u32 quad_to_patch = parse_ctx_.cache.else_prefix.quads_to_patch.top();
    parse_ctx_.cache.else_prefix.quads_to_patch.pop();
    qh.patch_quad(quad_to_patch, qh.next_quad_label());
}

inline void SemanticManager::whileStart__while()
{
    parse_ctx_.cache.while_start.next_quad_stack.push(
        parse_ctx_.quad_handler.next_quad_label());
}

inline void SemanticManager::whileCondition__lparen_expr_rparen(Expr *expr, Location expr_loc,
                                                                Location while_cond_loc)
{
    auto &qh = parse_ctx_.quad_handler;
    auto &eh = parse_ctx_.expr_handler;
    Expr *true_expr = eh.make_expr_const_bool(true, expr_loc);
    qh.emit_quad_w_jump_step(IOPCode::IF_EQ, expr, true_expr, +2, while_cond_loc);
    parse_ctx_.cache.while_condition.quads_to_patch.push(
        parse_ctx_.quad_handler.next_quad_label());

    qh.emit_quad_labelless(IOPCode::JUMP, nullptr, nullptr, nullptr, while_cond_loc);
}

inline void SemanticManager::whileStmt__whileHeader() noexcept
{
    parse_ctx_.function_ctx_handler.enter_loop();
}

inline void SemanticManager::whileStmt__whileHeader_stmt(const Location while_stmt_loc)
{
    auto &qh = parse_ctx_.quad_handler;

    DEBUG_SMART_ASSERT(parse_ctx_.cache.while_start.next_quad_stack.size() > 0);
    qh.emit_quad_w_label(IOPCode::JUMP, nullptr, nullptr, nullptr,
                         parse_ctx_.cache.while_start.next_quad_stack.top(), while_stmt_loc);

    DEBUG_SMART_ASSERT(parse_ctx_.cache.while_condition.quads_to_patch.size() > 0);
    qh.patch_quad(parse_ctx_.cache.while_condition.quads_to_patch.top(), qh.next_quad_label());
    parse_ctx_.cache.while_condition.quads_to_patch.pop();

    qh.patch_list(parse_ctx_.function_ctx_handler.get_breaklist(), qh.next_quad_label());
    qh.patch_list(parse_ctx_.function_ctx_handler.get_continuelist(),
                  parse_ctx_.cache.while_start.next_quad_stack.top());

    parse_ctx_.cache.while_start.next_quad_stack.pop();

    parse_ctx_.function_ctx_handler.exit_loop(); // This kills break and continue lists.
}

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

} // namespace Alpha

#endif /* SEMANTIC_MANAGER_HPP */

#endif //OLD_SEMANTIC_MANAGER_HPP
