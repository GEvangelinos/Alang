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
#include "core/alpha_macros.hpp"

#define STRINGIFY(_x) #_x

using namespace Alpha;

namespace // Anonymous
{
        namespace Loop
        {
                enum class Keyword
                {
                        BREAK,
                        CONTINUE,
                };

                std::string to_string(Keyword keyword) noexcept
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

        void loopCtrlStmt__loopkeyword_impl(
            const ParseCtx &parse_ctx,
            Location keyword_location,
            ErrorTracker &et,
            Loop::Keyword keyword)
        {
                if (parse_ctx.loop_depth() > 0)
                        return;

                std::string keyword_name = Loop::to_string(keyword);
                std::string error = std::format("`{}` statement not in a loop statement", keyword_name);
                et.report_syntax_error(error, keyword_location);
        }

        bool DEBUG_ALWAYS_INLINE reported_parameter_name_conflict(
            const SymbolTable &st,
            u32 current_scope,
            const Parameter &parameter,
            ErrorTracker &et)
        {
                // Library‐function conflict
                if (st.is_lib_function(parameter.name()))
                {
                        const std::string error = std::format(
                            "`{}` is a library function, can't declare it as formal", parameter.name());
                        et.report_syntax_error(error, parameter.location());
                        return true;
                }
                const Symbol *formal_symbol = st.lookup_local(parameter.name(), current_scope);
                // Parameter‐redeclared conflict
                if (formal_symbol)
                {
                        // Parameter should produce name conflicts only with themselves.
                        SANITY_ASSERT_TRUE(formal_symbol->type() == SymbolType::FORMAL);
                        const std::string error = std::format(
                            "redefinition of parameter `{}`", parameter.name());
                        const std::string note = std::format(
                            "previous definition of `{}` here", parameter.name());
                        et.report_syntax_error(
                            error, parameter.location(), note, formal_symbol->location());
                        return true;
                }
                return false;
        }

        void insert_function_parameters(
            SymbolTable &st,
            u32 current_scope,
            const std::list<Parameter> &parameter_list,
            ErrorTracker &et)
        {
                for (const Parameter &parameter : parameter_list)
                {
                        if (reported_parameter_name_conflict(st, current_scope, parameter, et))
                                continue;

                        st.insert_formal(parameter.name(), current_scope, parameter.location());
                }
        }

        bool DEBUG_ALWAYS_INLINE reported_function_name_conflict(
            SymbolTable &st,
            ParseCtx &parse_ctx,
            const char *id_name,
            Location id_location,
            ErrorTracker &et)
        {
                if (st.is_lib_function(id_name))
                {
                        const std::string error = std::format("redefinition of library function `{}`", id_name);
                        et.report_syntax_error(error, id_location);
                        return true;
                }

                const Symbol *resolved_symbol = st.lookup_local(id_name, parse_ctx.current_scope());
                if (!resolved_symbol)
                        return false;
                if (resolved_symbol->is_function())
                {
                        const std::string error = std::format("redefinition of `function {}`", id_name);
                        const std::string note = std::format("previous definition of `function {}` here", id_name);
                        et.report_syntax_error(error, id_location, note, resolved_symbol->location());
                }
                else if (resolved_symbol->is_variable())
                {
                        const std::string error = std::format("`{}` defined as a function", id_name);
                        const std::string note = std::format("`{}` previously defined as a variable here", id_name);
                        et.report_syntax_error(error, id_location, note, resolved_symbol->location());
                }
                return true;
        }

        void report_out_of_scope(
            const std::string &id_name,
            Location id_location,
            const std::string &current_function_name,
            Location current_function_location,
            const Symbol *resolved_symbol,
            ErrorTracker &et)
        {
                const std::string error = std::format(
                    "variable `{}` is declared outside current function `{}`.",
                    id_name, current_function_name);
                const std::string note1 = std::format(
                    "current function `{}` declared here", current_function_name);
                const std::string note2 = std::format(
                    "variable `{}` declared here", id_name);
                et.report_syntax_error(
                    error, id_location,
                    std::list<CodeMessage>{{note1, current_function_location},
                                           {note2, resolved_symbol->location()}});
        }

        inline constexpr char k_increment_str[] = "increment";
        inline constexpr char k_decrement_str[] = "decrement";

        template <const char * op_name>
        void term__lvalue_op(
            const Symbol *lvalue,
            Location term_location,
            ErrorTracker &et)
        {
                if (lvalue == nullptr)
                        return;
                if (lvalue->is_variable())
                        return;
                std::string error = std::format("{} operator can not be used on function", op_name);
                et.report_syntax_error(error, term_location);
        }
} // namespace Anonymous

// +-----------------------------------------------------------------+
// |---------------- SEMANTIC_ACTION_FUNCTIONS_BELOW ----------------|
// +-----------------------------------------------------------------+
void loopCtrlStmt__break(
    const ParseCtx &parse_ctx,
    Location break_location,
    ErrorTracker &et)
{
        loopCtrlStmt__loopkeyword_impl(parse_ctx, break_location, et, Loop::Keyword::BREAK);
}

void loopCtrlStmt__continue(
    const ParseCtx &parse_ctx,
    Location continue_location,
    ErrorTracker &et)
{
        loopCtrlStmt__loopkeyword_impl(parse_ctx, continue_location, et, Loop::Keyword::CONTINUE);
}

void funcCtrlStmt__return(
    const ParseCtx &parse_ctx,
    Location return_location,
    ErrorTracker &et)
{
        if (parse_ctx.current_function_nesting_depth() > 0)
                return;

        std::string error = "`return` statement not in a function statement";
        et.report_syntax_error(error, return_location);
}

void lvalue__id(
    SymbolTable &st,
    ParseCtx &parse_ctx,
    const char *id_name,
    Location id_location,
    const Symbol **const lvalue,
    ErrorTracker &et)
{
        const Symbol *resolved_symbol = st.lookup_chain(id_name, parse_ctx.current_scope());
        if (!resolved_symbol)
        {
                if (parse_ctx.current_scope() == k_global_scope)
                        *lvalue = st.insert_global(id_name, id_location);
                else
                        *lvalue = st.insert_local(id_name, parse_ctx.current_scope(), id_location);
                return;
        }
        if (resolved_symbol->is_variable())
        {
                if (resolved_symbol->scope() <= parse_ctx.current_function_scope())
                        report_out_of_scope(id_name, id_location, parse_ctx.current_function_name(),
                                            parse_ctx.current_function_location(), resolved_symbol, et);
        }
        *lvalue = resolved_symbol;
}

void lvalue__local_id(
    SymbolTable &st,
    ParseCtx &parse_ctx,
    const char *id_name,
    Location id_location,
    const Symbol **const lvalue,
    ErrorTracker &et)
{
        if (st.is_lib_function(id_name))
        {
                std::string error = std::format("shadowing of library function `{}`", id_name);
                et.report_syntax_error(error, id_location);
                *lvalue = st.lookup_global(id_name);
                SANITY_ASSERT_TRUE(*lvalue != nullptr); // a library function is always resolved at global scope.
                return;
        }
        const Symbol *resolved_symbol = st.lookup_local(id_name, parse_ctx.current_scope());
        if (resolved_symbol)
                *lvalue = resolved_symbol;
        else if (parse_ctx.current_scope() == k_global_scope)
                *lvalue = st.insert_global(id_name, id_location);
        else
                *lvalue = st.insert_local(id_name, parse_ctx.current_scope(), id_location);
}

void assignExpr__lvalue_assign_expr(
    const Symbol *const lvalue,
    Location assignExpr_location,
    ErrorTracker &et)
{
        if (!lvalue->is_function())
                return;

        std::string error = std::format("assignment of function `{}`", lvalue->name());
        et.report_syntax_error(error, assignExpr_location);
}

void term__inc_lvalue(
    const Symbol *lvalue,
    Location term_location,
    ErrorTracker &et)
{
        term__lvalue_op<k_increment_str>(lvalue, term_location, et);
}

void term__lvalue_inc(
    const Symbol *lvalue,
    Location term_location,
    ErrorTracker &et)
{
        term__lvalue_op<k_increment_str>(lvalue, term_location, et);
}

void term__dec_lvalue(
    const Symbol *lvalue,
    Location term_location,
    ErrorTracker &et)
{
        term__lvalue_op<k_decrement_str>(lvalue, term_location, et);
}

void term__lvalue_dec(
    const Symbol *lvalue,
    Location term_location,
    ErrorTracker &et)
{
        term__lvalue_op<k_decrement_str>(lvalue, term_location, et);
}

void lvalue__global_id(
    SymbolTable &st,
    const char *id_name,
    Location id_location,
    const Symbol **lvalue,
    ErrorTracker &et)
{
        *lvalue = st.lookup_global(id_name);
        if (*lvalue != nullptr)
                return;

        std::string error = std::format("variable `::{}` not found in global scope", id_name);
        et.report_syntax_error(error, id_location);
}

void block__lbrace(ParseCtx &parse_ctx) noexcept
{
        parse_ctx.enter_block();
}

void block__lbrace_multiStmt_rbrace(SymbolTable &st, ParseCtx &parse_ctx) noexcept
{
        st.hide_scope_symbols(parse_ctx.current_scope());
        parse_ctx.exit_block();
}

void block__lbrace_rbrace(SymbolTable &st, ParseCtx &parse_ctx)
{
        st.hide_scope_symbols(parse_ctx.current_scope());
        parse_ctx.exit_block();
}

void funcDef__function_id_lparen_idList_rparen_block(ParseCtx &parse_ctx) noexcept
{
        parse_ctx.exit_function();
}

void funcDef__function_lparen_idList_rparen_block(ParseCtx &parse_ctx) noexcept
{
        parse_ctx.exit_function();
}

void funcDef__function_id_lparen_idList_rparen(
    SymbolTable &st,
    ParseCtx &parse_ctx,
    const char *id_name,
    Location id_location,
    ErrorTracker &et)
{
        if (!reported_function_name_conflict(st, parse_ctx, id_name, id_location, et))
                st.insert_function(id_name, SymbolType::USERFUNC,
                                   parse_ctx.current_scope(), id_location,
                                   parse_ctx.retrieve_function_parameters());
        parse_ctx.enter_function(id_name, id_location);
        insert_function_parameters(st, parse_ctx.current_scope(),
                                   parse_ctx.extract_function_parameters(), et);
        parse_ctx.clear_function_arguments();
}

void funcDef__function_lparen_idList_rparen(
    SymbolTable &st,
    ParseCtx &parse_ctx,
    Location anonymous_location,
    ErrorTracker &et)
{
        st.insert_anonymous(parse_ctx.current_scope(), anonymous_location,
                            parse_ctx.retrieve_function_parameters());
        parse_ctx.enter_function(k_public_anonymous_prefix, anonymous_location);
        insert_function_parameters(st, parse_ctx.current_scope(),
                                   parse_ctx.extract_function_parameters(), et);
        parse_ctx.clear_function_arguments();
}

void funcArgs__id(ParseCtx &parse_ctx, const char *id_name, Location id_location)
{
        parse_ctx.append_function_parameter(id_name, id_location);
}

void whileStmt__whileHeader(ParseCtx &parse_ctx) noexcept
{
        parse_ctx.enter_loop();
}

void whileStmt__whileHeader_stmt(ParseCtx &parse_ctx) noexcept
{
        parse_ctx.exit_loop();
}

void forStmt__forHeader(ParseCtx &parse_ctx) noexcept
{
        parse_ctx.enter_loop();
}

void forStmt__forHeader_stmt(ParseCtx &parse_ctx) noexcept
{
        parse_ctx.exit_loop();
}

void lvalue__member(const Symbol **const lvalue) noexcept
{
        // We can not resolve members at compile time.
        *lvalue = nullptr;
}