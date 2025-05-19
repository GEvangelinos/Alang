#ifndef SEMANTIC_MANAGER_HPP
#define SEMANTIC_MANAGER_HPP

#include <string>                          // for string
#include "core/alpha_error.hpp"            // for ErrorTracker
#include "core/alpha_location.hpp"         // for Location
#include "parser/alpha_parser_context.hpp" // for ParseCtx
#include "parser/alpha_symbol_table.hpp"   // for Symbol, SymbolTable

#include <list> // for list, _List_const_iterator

#include "core/alpha_error.hpp"            // for ErrorTracker, Diagnostic
#include "core/alpha_konstants.hpp"        // for k_global_scope, k_public_...
#include "core/alpha_location.hpp"         // for Location
#include "utils/misc.hpp"                  // for DEBUG_ALWAYS_INLINE
#include "core/alpha_types.hpp"            // for u32
#include "parser/_parser_common.hpp"       // for Parameter
#include "parser/alpha_parser_context.hpp" // for ParseCtx
#include "utils/format_adapter.hpp"        // for format, FMT
#include "utils/smart_assert.h"            // for DEBUG_SMART_ASSERT
#include "parser/alpha_backpatcher.hpp"
#include "parser/alpha_semantic_action_funcs.hpp"

using namespace Alpha;

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
                        case Keyword::BREAK:
                                return "break";
                        case Keyword::CONTINUE:
                                return "continue";
                        default:
                                [[unlikely]] SMART_ASSERT(false);
                        }
                }
        }; // namespace Loop

        [[nodiscard]] DEBUG_ALWAYS_INLINE bool
        reported_parameter_name_conflict(
            const SymbolTable &st,
            const u32 current_scope,
            const Parameter &parameter,
            ErrorTracker &et)
        {
                // Library‐function conflict
                if (st.is_lib_function(parameter.name))
                {
                        const std::string error = FMT::format(
                            "`{}` is a library function, can't declare it as formal", parameter.name);
                        et.report_error(CTError::Type::SEMANTIC, error, parameter.location);
                        return true;
                }
                const Symbol *formal_symbol = st.lookup_local(parameter.name, current_scope);
                // Parameter‐redeclared conflict
                if (formal_symbol)
                {
                        // Parameter should produce name conflicts only with themselves.
                        DEBUG_SMART_ASSERT(                                  //
                            !!dynamic_cast<const Variable *>(formal_symbol), //
                            formal_symbol->is_variable()                     //
                        );

                        const std::string error = FMT::format("redefinition of parameter `{}`", parameter.name);
                        const std::string note = FMT::format("previous definition of `{}` here", parameter.name);
                        et.report_error(
                            CTError::Type::SEMANTIC, error, parameter.location, note, formal_symbol->location);
                        return true;
                }
                return false;
        }

        [[nodiscard]] DEBUG_ALWAYS_INLINE bool
        reported_function_name_conflict(
            SymbolTable &st,
            const u32 current_scope,
            const std::string function_name,
            const Location id_loc,
            ErrorTracker &et)
        {
                if (st.is_lib_function(function_name))
                {
                        const std::string error =
                            FMT::format("redefinition of library function `{}`", function_name);
                        et.report_error(CTError::Type::SEMANTIC, error, id_loc);
                        return true;
                }

                const Symbol *found_symbol = st.lookup_local(function_name, current_scope);
                if (!found_symbol)
                        return false;
                if (found_symbol->is_function())
                {
                        const std::string error = FMT::format("redefinition of `function {}`", function_name);
                        const std::string note =
                            FMT::format("previous definition of `function {}` here", function_name);
                        et.report_error(
                            CTError::Type::SEMANTIC, error, id_loc, note, found_symbol->location);
                }
                else if (found_symbol->is_variable())
                {
                        const std::string error = FMT::format("`{}` redefined as a function", function_name);
                        const std::string note =
                            FMT::format("`{}` previously defined as a variable here", function_name);
                        et.report_error(
                            CTError::Type::SEMANTIC, error, id_loc, note, found_symbol->location);
                }
                return true;
        }

        DEBUG_ALWAYS_INLINE void
        insert_function_parameters(SymbolTable &st, ParseCtx &parse_ctx, ErrorTracker &et)
        {

                auto scope = parse_ctx.scope_handler.scope();
                constexpr auto space = Variable::Space::FORMAL_ARGUMENT;
                DEBUG_SMART_ASSERT(parse_ctx.space_handler.space() == Variable::Space::FORMAL_ARGUMENT);

                for (const Parameter &param : parse_ctx.function_ctx_handler.function_parameters())
                        if (!reported_parameter_name_conflict(st, scope, param, et))
                                st.insert_variable(
                                    param.name,
                                    scope,
                                    space,
                                    parse_ctx.space_handler.next_offset(),
                                    param.location);
        }
} // namespace (Anonymous)

namespace Alpha::SemanticTransformer
{
        void update_expr_location(Expr *expr, Location new_expr_loc)
        {
                expr->location = new_expr_loc;
        }
}

class SemanticManager
{
public:
        SemanticManager(ParseCtx *const parse_ctx) : parse_ctx_(parse_ctx) {}
        void loopCtrlStmt__break(Location break_loc);
        void loopCtrlStmt__continue(Location continue_loc);
        void term__inc_lvalue(const Symbol *lvalue, Location term_loc);
        void term__lvalue_inc(const Symbol *lvalue, Location term_loc);
        void term__dec_lvalue(const Symbol *lvalue, Location term_loc);
        void term__lvalue_dec(const Symbol *lvalue, Location term_loc);
        void assignExpr__lvalue_assign_expr(
            Expr *&assignExpr,
            Expr *lvalue,
            Expr *expr,
            Location assign_loc);
        void lvalue__id(Expr *&lvalue, const char *id_name, Location id_loc);

private:
        ParseCtx *const parse_ctx_;

        void loopCtrlStmt__loopkeyword_impl(Loop::Keyword keyword, Location keyword_loc);
        void term__lvalue_op(const char *op_name, const Symbol *lvalue, Location term_loc);
        void report_out_of_scope_variable(const char *id_name,
                                          const std::string &current_function_name,
                                          const Location id_loc,
                                          const Location current_function_loc,
                                          const Symbol *found_symbol)
};

inline void
SemanticManager::loopCtrlStmt__loopkeyword_impl(
    const Loop::Keyword keyword,
    const Location keyword_loc)
{
        if (parse_ctx_->function_ctx_handler.loop_depth() > 0)
                return;

        std::string keyword_name = Loop::to_string(keyword);
        std::string error = FMT::format("`{}` statement not in a loop statement", keyword_name);
        parse_ctx_->et->report_error(CTError::Type::SEMANTIC, error, keyword_loc);
}

inline void
SemanticManager::term__lvalue_op(
    const char *op_name,
    const Symbol *lvalue,
    const Location term_loc)
{
        // lvalue is valid to be nullptr (runtime evaluation).
        DEBUG_SMART_ASSERT(op_name == "increment" || op_name == "decrement");
        if (is_modifiable_symbol(lvalue))
                return;
        std::string error = FMT::format("{} operator can not be used on function", op_name);
        parse_ctx_->et->report_error(CTError::Type::SEMANTIC, error, term_loc);
}

inline void
SemanticManager::report_out_of_scope_variable(
    const char *id_name,
    const Location id_loc,
    const std::string &current_function_name,
    const Location current_function_loc,
    const Symbol *found_symbol)
{
        using DT = Diagnostic::Type;
        DEBUG_SMART_ASSERT(!!found_symbol);
        const std::string error = FMT::format("variable `{}` is not accessible in function `{}`",
                                              id_name, current_function_name);
        const std::string note1 = FMT::format("function `{}` declared here", current_function_name);
        const std::string note2 = FMT::format("variable `{}` declared here", id_name);
        et.report_error(CTError::Type::SEMANTIC, error, id_loc,
                        std::list<Diagnostic>{{DT::NOTE, note1, current_function_loc},
                                              {DT::NOTE, note2, found_symbol->location}});
}
inline void
SemanticManager::loopCtrlStmt__break(Location break_loc)
{
        loopCtrlStmt__loopkeyword_impl(Loop::Keyword::BREAK, break_loc);
}

inline void
SemanticManager::loopCtrlStmt__continue(const Location continue_loc)
{
        loopCtrlStmt__loopkeyword_impl(Loop::Keyword::CONTINUE, continue_loc);
}

inline void
SemanticManager::term__inc_lvalue(const Symbol *lvalue, const Location term_loc)
{
        term__lvalue_op("increment", lvalue, term_loc);
}

inline void
SemanticManager::term__lvalue_inc(const Symbol *lvalue, const Location term_loc)
{
        term__lvalue_op("increment", lvalue, term_loc);
}

inline void
SemanticManager::term__dec_lvalue(const Symbol *lvalue, const Location term_loc)
{
        term__lvalue_op("decrement", lvalue, term_loc);
}

inline void
SemanticManager::term__lvalue_dec(const Symbol *lvalue, const Location term_loc)
{
        term__lvalue_op("decrement", lvalue, term_loc);
}

inline void
SemanticManager::lvalue__id(Expr *&lvalue, const char *id_name, const Location id_loc)
{
        const Symbol *symbol = parse_ctx_->st->lookup_chain(
            id_name,
            parse_ctx_->scope_handler.scope() //
        );

        if (!symbol)
                symbol = parse_ctx_->st->insert_variable(
                    id_name,
                    parse_ctx_->scope_handler.scope(),
                    parse_ctx_->space_handler.space(),
                    parse_ctx_->space_handler.next_offset(),
                    id_loc);
        else if (symbol->is_variable() &&
                 symbol->scope > k_global_scope &&
                 symbol->scope <= parse_ctx_->function_ctx_handler.current_function_scope())
                report_out_of_scope_variable(
                    id_name,
                    id_loc, parse_ctx_->function_ctx_handler.current_function_name(),
                    parse_ctx_->function_ctx_handler.current_function_location(),
                    symbol,
                    et);

        lvalue = parse_ctx.expr_handler.make_expr_variable(symbol, symbol->location); // TODO : PROBABLY not symbol's location
}

ALWAYS_INLINE void lvalue__local_id(
    SymbolTable &st,
    ParseCtx &parse_ctx,
    const char *id_name,
    const Location id_loc,
    Expr *&lvalue,
    ErrorTracker &et)
{
        const Symbol *symbol = nullptr;
        if (st.is_lib_function(id_name))
        {
                std::string error = FMT::format("shadowing library function `{}`", id_name);
                et.report_error(CTError::Type::SEMANTIC, error, id_loc);
                symbol = st.lookup_global(id_name);
                DEBUG_SMART_ASSERT(!!symbol); // a library function is always resolved at global scope.
        }
        else
        {
                symbol = st.lookup_local(id_name, parse_ctx.scope_handler.scope());
                if (!symbol)
                        symbol = st.insert_variable(
                            id_name,
                            parse_ctx.scope_handler.scope(),
                            parse_ctx.space_handler.space(),
                            parse_ctx.space_handler.next_offset(),
                            id_loc);
        }

        lvalue = parse_ctx.expr_handler.make_expr_variable(symbol, symbol->location); // TODO : PROBABLY not symbol's location
}

ALWAYS_INLINE void lvalue__global_id(
    SymbolTable &st,
    ParseCtx &parse_ctx,
    const char *id_name,
    const Location id_loc,
    Expr *&lvalue,
    ErrorTracker &et)
{
        // TODO: I dont like producing shit if global symbol doesnt exist.
        // TODO: we must find an anchor/hook point, where we can reset, and continue
        // TODO : producing quads even if they will never be used.
        // TODO: Or disable quad emission all together even if a single error is found. (?)
        const Symbol *symbol = st.lookup_global(id_name);
        if (symbol)
        {
                lvalue = parse_ctx.expr_handler.make_expr_variable(symbol, symbol->location); // TODO : PROBABLY not symbol's location
                return;
        }
        std::string error = FMT::format("variable `::{}` not found in global scope", id_name);
        et.report_error(CTError::Type::SEMANTIC, error, id_loc);
}

inline void tableItem__lvalue_dot_id(
    SymbolTable &st,
    ParseCtx &parse_ctx,
    Expr *&tableItem,
    Expr *&lvalue,
    const char *id,
    Location id_loc,
    Location table_item_loc)
{
        tableItem = parse_ctx.expr_handler.make_expr_table_item(lvalue, id, id_loc, table_item_loc);
}

inline void tableItem__lvalue_lbracket_expr_rbracket(
    SymbolTable &st,
    ParseCtx &parse_ctx,
    Expr *&table_item,
    Expr *&lvalue,
    Expr *expr,
    Location table_item_loc)
{
        table_item = parse_ctx.expr_handler.make_expr_table_item(lvalue, expr, table_item_loc);
}

inline void call__lvalue_lparen_elist_rparen(
    SymbolTable &st,
    ParseCtx &parse_ctx,
    Expr *&call,
    Expr *&lvalue,
    ExprList *elist,
    Location call_loc)
{
        // TODO rethink position of make_call() .. we would like procs not using fns and vice versa.
        namespace ASF = Alpha::SemanticFunctions;
        lvalue = parse_ctx.expr_handler.emit_quad_if_table_item(lvalue);
        call = ASF::make_call(st, parse_ctx, lvalue, elist, call_loc);
}

inline void call__lvalue_ddot_id_lparen_elist_rparen(
    SymbolTable &st,
    ParseCtx &parse_ctx,
    Expr *&call,
    Expr *&lvalue,
    const char *id,
    Location id_loc,
    ExprList *elist,
    Location call_loc)
{
        lvalue = parse_ctx.expr_handler.emit_quad_if_table_item(lvalue);

        // BEGIN_BOUND: CODE RUNNING ONLY FOR METHODS // TODO: remove comment bounds.. or parameterize (DRY)
        elist->push_back(lvalue);
        // TODO: Understand what this name should be... And also understand first emit_quad_if...
        Expr *temp_var = parse_ctx.expr_handler.make_expr_table_item(lvalue, id, id_loc, k_no_location);
        lvalue = parse_ctx.expr_handler.emit_quad_if_table_item(temp_var);
        // END_BOUND: CODE RUNNING ONLY FOR METHODS

        namespace ASF = Alpha::SemanticFunctions; // TODO REMOVE (UGLY RETHINK POSITION OF make_call)
        call = ASF::make_call(st, parse_ctx, lvalue, elist, call_loc);
}

inline void blockBegin__lbrace(ParseCtx &parse_ctx) noexcept
{
        parse_ctx.scope_handler.enter_scope();
}

inline void blockEnd__rbrace(SymbolTable &st, ParseCtx &parse_ctx) noexcept
{
        st.hide_scope_symbols(parse_ctx.scope_handler.scope());
        parse_ctx.scope_handler.exit_scope();
}

inline void funcPrefix__function(ParseCtx &parse_ctx, const Location anonymous_loc)
{
        // Update ParseCache:
        parse_ctx.cache.func_prefix.id = parse_ctx.name_generator.new_anonymous();
        parse_ctx.cache.func_prefix.location = anonymous_loc;

        parse_ctx.space_handler.enter_space();
}

inline void funcPrefix__function_id(
    ParseCtx &parse_ctx,
    const char *id_name,
    Location id_loc)
{
        // Update ParseCache
        parse_ctx.cache.func_prefix.id = id_name;
        parse_ctx.cache.func_prefix.location = id_loc;

        parse_ctx.space_handler.enter_space();
}

/// Handles a function signature’s prefix + argument list.
///
/// If a name conflict is detected, we still need to call
/// enter_function() (to keep our frame‐stack balanced), but
/// we must *not* back-patch the local-variable count or we
/// ’ll end up polluting the original function’s frame with
/// local_variable_count from the redefinition.
inline void funcSignature__funcPrefix_funcArgList(
    SymbolTable &st,
    ParseCtx &parse_ctx,
    ErrorTracker &et,
    const Function *&funcSignature)
{
        const Location func_loc = parse_ctx.cache.func_prefix.location;
        bool conflicting_name = reported_function_name_conflict(
            st,
            parse_ctx.scope_handler.scope(),
            parse_ctx.cache.func_prefix.id,
            func_loc,
            et);

        const Function *function_symbol = nullptr;
        if (!conflicting_name)
        {
                function_symbol = st.insert_function(
                    parse_ctx.cache.func_prefix.id,
                    parse_ctx.scope_handler.scope(),
                    parse_ctx.function_ctx_handler.next_function_address(),
                    parse_ctx.function_ctx_handler.function_parameters(),
                    func_loc);

                parse_ctx.quad_handler.emit_quad(
                    IOPCode::FUNCSTART,
                    nullptr,
                    nullptr,
                    parse_ctx.expr_handler.make_expr_variable(function_symbol, func_loc),
                    func_loc);
        }
        parse_ctx.function_ctx_handler.enter_function(function_symbol);
        insert_function_parameters(st, parse_ctx, et);
        parse_ctx.function_ctx_handler.clear_function_parameters();
        parse_ctx.space_handler.enter_space(); // IMPORTANT: This line is after parameter insertion!

        funcSignature = function_symbol;
}

inline void funcDef__funcSignature_block(
    ParseCtx &parse_ctx,
    const BlockLocation &block_loc) noexcept
{
        auto fbi = parse_ctx.function_ctx_handler.exit_function();
        if (!!fbi.function_symbol)
        {
                Backpatcher::set_function_local_variable_count(
                    fbi.function_symbol,
                    fbi.local_variable_count);

                parse_ctx.quad_handler.emit_quad(
                    IOPCode::FUNCEND,
                    nullptr,
                    nullptr,
                    parse_ctx.expr_handler.make_expr_variable(fbi.function_symbol, k_no_location), // TODO: what location here?
                    block_loc.end);
        }

        parse_ctx.space_handler.exit_space();
}

inline void const__stringliteral(char *&string_literal)
{
        delete[] string_literal;
        string_literal = nullptr;
}

inline void funcArgs__id(ParseCtx &parse_ctx, const char *id_name, const Location id_loc)
{
        parse_ctx.function_ctx_handler.add_function_parameter(id_name, id_loc);
}

inline void whileStmt__whileHeader(ParseCtx &parse_ctx) noexcept
{
        parse_ctx.function_ctx_handler.enter_loop();
}

inline void whileStmt__whileHeader_stmt(ParseCtx &parse_ctx) noexcept
{
        parse_ctx.function_ctx_handler.exit_loop();
}

inline void forStmt__forHeader(ParseCtx &parse_ctx) noexcept
{
        parse_ctx.function_ctx_handler.enter_loop();
}

inline void forStmt__forHeader_stmt(ParseCtx &parse_ctx) noexcept
{
        parse_ctx.function_ctx_handler.exit_loop();
}

inline void funcCtrlStmt__return(
    const ParseCtx &parse_ctx,
    const Location return_loc,
    ErrorTracker &et)
{
        if (parse_ctx.function_ctx_handler.function_nesting_depth() > 0)
                return;
        std::string error = "`return` statement not in a function statement";
        et.report_error(CTError::Type::SEMANTIC, error, return_loc);
}

#endif /* SEMANTIC_MANAGER_HPP */