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

} // namespace

namespace Alpha
{

class SemanticManager
{
public:
        SemanticManager(ParseCtx &parse_ctx, SymbolTable &st, ErrorTracker &et);
        void N(Location n_loc, const int N_index);
        void M();
        void forHeader__for_lparen_elist_semicolon_m_expr_semicolon(Expr *expr, Location expr_loc);
        void multiStmt__stmt();
        void loopCtrlStmt__break(Location break_loc);
        void loopCtrlStmt__continue(Location continue_loc);
        void term__inc_lvalue(Expr *&term, Expr *lvalue, Location term_loc);
        void term__lvalue_inc(Expr *&term, Expr *lvalue, Location term_loc);
        void term__dec_lvalue(Expr *&term, Expr *lvalue, Location term_loc);
        void term__lvalue_dec(Expr *&term, Expr *lvalue, Location term_loc);
        void lvalue__id(Expr *&lvalue, const char *id_name, Location id_loc);
        void lvalue__local_id(Expr *&lvalue, const char *id_name, Location id_loc);
        void lvalue__global_id(Expr *&lvalue, const char *id_name, Location id_loc);
        void methodCallId__methodcall_id(const char *id, Location id_loc, Location method_call_loc);
        void blockBegin__lbrace() noexcept;
        void blockEnd__rbrace() noexcept;
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
        void backpatch_bool_expr(Expr *expr, Location expr_loc);
        void saveNextQuadHook();

private:
        ParseCtx &parse_ctx_;
        SymbolTable &st_;
        ErrorTracker &et_;

        [[nodiscard]] bool assert_loop_context_or_error(Loop::Keyword keyword,
                                                        Location keyword_loc);
        void term__lvalue_op(const char *op_name, const Expr *lvalue, Location term_loc);
        void report_out_of_scope_variable(const char *id_name,
                                          const std::string &current_function_name,
                                          const Symbol *found_symbol, Location id_loc,
                                          Location current_function_loc);
        [[nodiscard]] bool reported_function_name_conflict(const std::string &function_name,
                                                           u32 current_scope, Location id_loc);
        void insert_gathered_function_parameters();
        [[nodiscard]] bool reported_parameter_name_conflict(u32 current_scope,
                                                            const Parameter &parameter);
        void report_error_if_not_arithmetic(const Expr *expr, Location expr_loc,
                                            const char *context);
}; // class SemanticManager

inline SemanticManager::SemanticManager(ParseCtx &parse_ctx, SymbolTable &st, ErrorTracker &et)
    : parse_ctx_(parse_ctx), st_(st), et_(et)
{}

// TODO split function in 2 parts, and put these you parts back to caller function..
//  this functions does 2 things.. and thus it break Single Responsibility...
inline bool SemanticManager::assert_loop_context_or_error(const Loop::Keyword keyword,
                                                          const Location keyword_loc)
{
        if (parse_ctx_.function_ctx_handler.loop_depth() > 0)
                return true; // we are in loop context

        std::string keyword_name = Loop::to_string(keyword);
        std::string error = FMT::format("`{}` statement not in a loop statement", keyword_name);
        et_.report_error(CTError::Type::SEMANTIC, error, keyword_loc);
        return false;
}

inline void SemanticManager::term__lvalue_op(const char *op_name, const Expr *lvalue,
                                             const Location term_loc)
{
        DEBUG_SMART_ASSERT(!!op_name, !!lvalue);
        DEBUG_SMART_ASSERT(!!lvalue->symbol);

        DEBUG_SMART_ASSERT(std::strcmp(op_name, "increment") == 0 ||
                           std::strcmp(op_name, "decrement") == 0 //
        );

        if (Symbol::is_modifiable_symbol(lvalue->symbol))
                return;
        std::string error = FMT::format("{} operator can not be used on function", op_name);
        et_.report_error(CTError::Type::SEMANTIC, error, term_loc);
}

inline void SemanticManager::report_out_of_scope_variable(const char *id_name,
                                                          const std::string &current_function_name,
                                                          const Symbol *found_symbol,
                                                          const Location id_loc,
                                                          const Location current_function_loc)
{
        using DT = Diagnostic::Type;
        DEBUG_SMART_ASSERT(!!found_symbol);
        const std::string error = FMT::format("variable `{}` is not accessible in function `{}`",
                                              id_name, current_function_name);
        const std::string note1 = FMT::format("function `{}` declared here", current_function_name);
        const std::string note2 = FMT::format("variable `{}` declared here", id_name);
        et_.report_error(CTError::Type::SEMANTIC, error, id_loc,
                         std::list<Diagnostic>{
                             {DT::NOTE, note1, current_function_loc},
                             {DT::NOTE, note2, found_symbol->location} //
                         } //
        );
}

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

        for (const Parameter &param : parse_ctx_.function_ctx_handler.function_parameters())
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

inline void SemanticManager::report_error_if_not_arithmetic(const Expr *expr,
                                                            const Location expr_loc,
                                                            const char *const context)
{
        using ET = Expr::Type;
        switch (expr->type)
        {
        case ET::BOOLEAN_EXPR:
        case ET::NEW_TABLE:
        case ET::LIBRARY_FUNCTION:
        case ET::PROGRAM_FUNCTION:
        case ET::CONST_BOOL:
        case ET::CONST_NIL:
        case ET::CONST_STRING:
                et_.report_error(CTError::Type::SEMANTIC,
                                 FMT::format("{} {}", "Invalid arithmetic expr: ",
                                             context), // TODO: fix ugly AF!
                                 expr_loc);
                break;
        default: break; // No error to report for the other Expr Types.
        }
}

inline void SemanticManager::multiStmt__stmt()
{
        parse_ctx_.name_generator.reset_temp_names();
}

inline void SemanticManager::loopCtrlStmt__break(Location break_loc)
{
        // TODO: Split that function in 2
        if (!assert_loop_context_or_error(Loop::Keyword::BREAK, break_loc))
                return;

        parse_ctx_.function_ctx_handler.add_label_to_breaklist(
            parse_ctx_.quad_handler.next_quad_label());

        parse_ctx_.quad_handler.emit_quad_labelless(IOPCode::JUMP, nullptr, nullptr, nullptr,
                                                    break_loc);
}

inline void SemanticManager::loopCtrlStmt__continue(const Location continue_loc)
{
        // TODO: Split that function in 2
        if (!assert_loop_context_or_error(Loop::Keyword::CONTINUE, continue_loc))
                return;

        // TODO: Consider this in the end of the project. This code could removed from here.
        // If we put the label of jump. technically we can know the label.. As continue just returns
        // to the top.. and if continue is validly existing.. then there was a loop.. and that.
        // that quad is already emitted.. (loop condition) , thus we can know JUMP's label.
        // Although this creates assymetric behavior with breaklist and when we patch continues.
        parse_ctx_.function_ctx_handler.add_label_to_continuelist(
            parse_ctx_.quad_handler.next_quad_label());

        parse_ctx_.quad_handler.emit_quad_labelless(IOPCode::JUMP, nullptr, nullptr, nullptr,
                                                    continue_loc);
}

inline void SemanticManager::term__inc_lvalue(Expr *&term, Expr *const lvalue,
                                              const Location term_loc)
{
        term__lvalue_op("increment", lvalue, term_loc);
        report_error_if_not_arithmetic(lvalue, term_loc, "++lvalue");
        if (lvalue->type == Expr::Type::TABLE_ITEM)
        {
                term = parse_ctx_.expr_handler.emit_quad_if_table_item(lvalue);
                parse_ctx_.quad_handler.emit_quad(
                    IOPCode::ADD, term,
                    parse_ctx_.expr_handler.make_expr_const_int(
                        1, term_loc), // TODO, Use a global const_int expr (with value 1) Dont
                                      // recreate it all the time
                    term, term_loc);
                parse_ctx_.quad_handler.emit_quad(IOPCode::TABLESETELEM, lvalue, lvalue->index,
                                                  term, term_loc);
        }
        else
        {
                parse_ctx_.quad_handler.emit_quad(
                    IOPCode::ADD, lvalue,
                    parse_ctx_.expr_handler.make_expr_const_int(
                        1, term_loc), // TODO, Use a global const_int expr (with value 1) Dont
                                      // recreate it all the time
                    lvalue, term_loc);
                term = parse_ctx_.expr_handler.make_expr_arithmetic(term_loc);
                parse_ctx_.quad_handler.emit_quad(IOPCode::ASSIGN, lvalue, nullptr, term, term_loc);
        }
}

inline void SemanticManager::term__lvalue_inc(Expr *&term, Expr *lvalue, const Location term_loc)
{
        term__lvalue_op("increment", lvalue, term_loc);
        report_error_if_not_arithmetic(lvalue, term_loc,
                                       "lvalue++"); // TODO: context variable is silly fix .
        term = parse_ctx_.expr_handler.make_expr_variable(parse_ctx_.new_temp(), term_loc);
        if (lvalue->type == Expr::Type::TABLE_ITEM)
        {
                Expr *val = parse_ctx_.expr_handler.emit_quad_if_table_item(lvalue);
                parse_ctx_.quad_handler.emit_quad(IOPCode::ASSIGN, val, nullptr, term, term_loc);
                parse_ctx_.quad_handler.emit_quad(
                    IOPCode::ADD, val, parse_ctx_.expr_handler.make_expr_const_int(1, term_loc),
                    val, term_loc);
                parse_ctx_.quad_handler.emit_quad(IOPCode::TABLESETELEM, lvalue, lvalue->index, val,
                                                  term_loc);
        }
        else
        {
                parse_ctx_.quad_handler.emit_quad(IOPCode::ASSIGN, lvalue, nullptr, term, term_loc);
                parse_ctx_.quad_handler.emit_quad(
                    IOPCode::ADD, lvalue, parse_ctx_.expr_handler.make_expr_const_int(1, term_loc),
                    lvalue, term_loc);
        }
}

inline void SemanticManager::term__dec_lvalue(Expr *&term, Expr *lvalue, const Location term_loc)
{
        term__lvalue_op("decrement", lvalue, term_loc);
        report_error_if_not_arithmetic(lvalue, term_loc, "--lvalue");
        if (lvalue->type == Expr::Type::TABLE_ITEM)
        {
                term = parse_ctx_.expr_handler.emit_quad_if_table_item(lvalue);
                parse_ctx_.quad_handler.emit_quad(
                    IOPCode::SUB, term,
                    parse_ctx_.expr_handler.make_expr_const_int(
                        1, term_loc), // TODO, Use a global const_int expr (with value 1) Dont
                                      // recreate it all the time
                    term, term_loc);
                parse_ctx_.quad_handler.emit_quad(IOPCode::TABLESETELEM, lvalue, lvalue->index,
                                                  term, term_loc);
        }
        else
        {
                parse_ctx_.quad_handler.emit_quad(
                    IOPCode::SUB, lvalue,
                    parse_ctx_.expr_handler.make_expr_const_int(
                        1, term_loc), // TODO, Use a global const_int expr (with value 1) Dont
                                      // recreate it all the time
                    lvalue, term_loc);
                term = parse_ctx_.expr_handler.make_expr_arithmetic(term_loc);
                parse_ctx_.quad_handler.emit_quad(IOPCode::ASSIGN, lvalue, nullptr, term, term_loc);
        }
}

inline void SemanticManager::term__lvalue_dec(Expr *&term, Expr *lvalue, const Location term_loc)
{
        term__lvalue_op("decrement", lvalue, term_loc);
        report_error_if_not_arithmetic(lvalue, term_loc,
                                       "lvalue--"); // TODO: context variable is silly fix .
        term = parse_ctx_.expr_handler.make_expr_variable(parse_ctx_.new_temp(), term_loc);
        if (lvalue->type == Expr::Type::TABLE_ITEM)
        {
                Expr *val = parse_ctx_.expr_handler.emit_quad_if_table_item(lvalue);
                parse_ctx_.quad_handler.emit_quad(IOPCode::ASSIGN, val, nullptr, term, term_loc);
                parse_ctx_.quad_handler.emit_quad(
                    IOPCode::SUB, val, parse_ctx_.expr_handler.make_expr_const_int(1, term_loc),
                    val, term_loc);
                parse_ctx_.quad_handler.emit_quad(IOPCode::TABLESETELEM, lvalue, lvalue->index, val,
                                                  term_loc);
        }
        else
        {
                parse_ctx_.quad_handler.emit_quad(IOPCode::ASSIGN, lvalue, nullptr, term, term_loc);
                parse_ctx_.quad_handler.emit_quad(
                    IOPCode::SUB, lvalue, parse_ctx_.expr_handler.make_expr_const_int(1, term_loc),
                    lvalue, term_loc);
        }
}

inline void SemanticManager::lvalue__id(Expr *&lvalue, const char *id_name, const Location id_loc)
{
        const Symbol *symbol = st_.lookup_chain(id_name,
                                                parse_ctx_.scope_handler.scope() //
        );

        if (!symbol)
        {
                Variable::Type var_type = parse_ctx_.scope_handler.scope() == k_global_scope
                                              ? Variable::Type::GLOBAL_VARIABLE
                                              : Variable::Type::LOCAL_VARIABLE;
                symbol = st_.insert_variable(id_name, parse_ctx_.scope_handler.scope(), var_type,
                                             parse_ctx_.space_handler.space(),
                                             parse_ctx_.space_handler.next_offset(), id_loc);
        }
        else if (symbol->is_variable() && symbol->scope > k_global_scope &&
                 symbol->scope <= parse_ctx_.function_ctx_handler.current_function_scope())
                report_out_of_scope_variable(
                    id_name, parse_ctx_.function_ctx_handler.current_function_name(), symbol,
                    id_loc, parse_ctx_.function_ctx_handler.current_function_location());

        lvalue = parse_ctx_.expr_handler.make_expr_variable(
            symbol, symbol->location); // TODO : PROBABLY not symbol's location
}

inline void SemanticManager::lvalue__local_id(Expr *&lvalue, const char *id_name,
                                              const Location id_loc)
{
        const Symbol *symbol = nullptr;
        if (st_.is_lib_function(id_name))
        {
                std::string error = FMT::format("shadowing library function `{}`", id_name);
                et_.report_error(CTError::Type::SEMANTIC, error, id_loc);
                symbol = st_.lookup_global(id_name);
                DEBUG_SMART_ASSERT(
                    !!symbol); // a library function is always resolved at global scope.
        }
        else
        {
                symbol = st_.lookup_local(id_name, parse_ctx_.scope_handler.scope());
                if (!symbol)
                        symbol = st_.insert_variable(
                            id_name, parse_ctx_.scope_handler.scope(),
                            Variable::Type::LOCAL_VARIABLE, parse_ctx_.space_handler.space(),
                            parse_ctx_.space_handler.next_offset(), id_loc);
        }

        lvalue = parse_ctx_.expr_handler.make_expr_variable(
            symbol, symbol->location); // TODO : PROBABLY not symbol's location
}

inline void SemanticManager::lvalue__global_id(Expr *&lvalue, const char *id_name,
                                               const Location id_loc)
{
        // TODO: I dont like producing shit if global symbol doesnt exist.
        // TODO: we must find an anchor/hook point, where we can reset, and continue
        // TODO : producing quads even if they will never be used.
        // TODO: Or disable quad emission all together even if a single error is found. (?)
        const Symbol *symbol = st_.lookup_global(id_name);
        if (symbol)
        {
                lvalue = parse_ctx_.expr_handler.make_expr_variable(
                    symbol, symbol->location); // TODO : PROBABLY not symbol's location
                return;
        }
        std::string error = FMT::format("variable `::{}` not found in global scope", id_name);
        et_.report_error(CTError::Type::SEMANTIC, error, id_loc);
}

inline void SemanticManager::methodCallId__methodcall_id(const char *id, const Location id_loc,
                                                         const Location method_call_loc)
{
        parse_ctx_.cache.method_call_id.id = id;
        parse_ctx_.cache.method_call_id.id_location = id_loc;
        parse_ctx_.cache.method_call_id.method_call_location = method_call_loc;
}

inline void SemanticManager::blockBegin__lbrace() noexcept
{
        parse_ctx_.scope_handler.enter_scope();
}

inline void SemanticManager::blockEnd__rbrace() noexcept
{
        st_.hide_scope_symbols(parse_ctx_.scope_handler.scope());
        parse_ctx_.scope_handler.exit_scope();
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
        parse_ctx_.function_ctx_handler.enter_function(function_symbol);
        insert_gathered_function_parameters();
        parse_ctx_.function_ctx_handler.clear_function_parameters();
        parse_ctx_.space_handler
            .enter_space(); // IMPORTANT: This line is after parameter insertion!

        funcSignature = function_symbol;
}

inline void SemanticManager::funcDef__funcSignature_block(const BlockLocation &block_loc) noexcept
{
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
        if (parse_ctx_.function_ctx_handler.function_nesting_depth() > 0)
                return;
        std::string error = "`return` statement not in a function statement";
        et_.report_error(CTError::Type::SEMANTIC, error, return_loc);
}

inline void SemanticManager::saveNextQuadHook()
{
        parse_ctx_.cache.logical_marker.next_quad_stack.push(
            parse_ctx_.quad_handler.next_quad_label());
}

inline void SemanticManager::backpatch_bool_expr(Expr *expr, Location expr_loc)
{
        DEBUG_SMART_ASSERT(!!expr);
        if (expr->type != Expr::Type::BOOLEAN_EXPR)
                return; // Nothing to finalize (backpatch).
        DEBUG_SMART_ASSERT(!!expr->symbol);

        auto &qh = parse_ctx_.quad_handler;
        auto &eh = parse_ctx_.expr_handler;
        const Expr *true_expr = eh.make_expr_const_bool(true, expr_loc);
        const Expr *false_expr = eh.make_expr_const_bool(false, expr_loc);

        qh.patch_list(expr->backpatch_info->true_list, qh.next_quad_label());
        qh.emit_quad(IOPCode::ASSIGN, true_expr, nullptr, expr, expr_loc);
        qh.emit_quad_w_jump_step(IOPCode::JUMP, nullptr, nullptr, +2, expr_loc);
        qh.patch_list(expr->backpatch_info->false_list, qh.next_quad_label());
        qh.emit_quad(IOPCode::ASSIGN, false_expr, nullptr, expr, expr_loc);
}

} // namespace Alpha

#endif /* SEMANTIC_MANAGER_HPP */