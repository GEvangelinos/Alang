#include "parser/alpha_semantic_actions.hpp"
#include "parser/alpha_parser_context.hpp"
#include "core/alpha_location.hpp"
#include "core/alpha_types.hpp"
#include "core/alpha_error_tracker.hpp"
#include "utils/smart_assert.h"
#include "core/alpha_macros.hpp"
#include "utils/format_adapter.hpp"
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
                std::string error = fmt_ns::format("`{}` statement not in a loop statement", keyword_name);
                et.report_syntax_error(error, keyword_location);
        }

        DEBUG_ALWAYS_INLINE bool reported_parameter_name_conflict(
            const SymbolTable &st,
            u32 current_scope,
            const Parameter &parameter,
            ErrorTracker &et)
        {
                // Library‐function conflict
                if (st.is_lib_function(parameter.name()))
                {
                        const std::string error = fmt_ns::format(
                            "`{}` is a library function, can't declare it as formal", parameter.name());
                        et.report_syntax_error(error, parameter.location());
                        return true;
                }
                const Symbol *formal_symbol = st.lookup_local(parameter.name(), current_scope);
                // Parameter‐redeclared conflict
                if (formal_symbol)
                {
                        // Parameter should produce name conflicts only with themselves.
                        DEBUG_SMART_ASSERT(formal_symbol->type() == SymbolType::FORMAL);
                        const std::string error = fmt_ns::format(
                            "redefinition of parameter `{}`", parameter.name());
                        const std::string note = fmt_ns::format(
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
                        if (!reported_parameter_name_conflict(st, current_scope, parameter, et))
                                st.insert_formal(parameter.name(), current_scope, parameter.location());
        }

        DEBUG_ALWAYS_INLINE bool reported_function_name_conflict(
            SymbolTable &st,
            u32 current_scope,
            const std::string function_name,
            Location id_location,
            ErrorTracker &et)
        {
                if (st.is_lib_function(function_name))
                {
                        const std::string error = fmt_ns::format("redefinition of library function `{}`", function_name);
                        et.report_syntax_error(error, id_location);
                        return true;
                }

                const Symbol *resolved_symbol = st.lookup_local(function_name, current_scope);
                if (!resolved_symbol)
                        return false;
                if (resolved_symbol->is_function())
                {
                        const std::string error = fmt_ns::format("redefinition of `function {}`", function_name);
                        const std::string note = fmt_ns::format("previous definition of `function {}` here", function_name);
                        et.report_syntax_error(error, id_location, note, resolved_symbol->location());
                }
                else if (resolved_symbol->is_variable())
                {
                        const std::string error = fmt_ns::format("`{}` redefined as a function", function_name);
                        const std::string note = fmt_ns::format("`{}` previously defined as a variable here", function_name);
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
                using DT = Diagnostic::Type;
                DEBUG_SMART_ASSERT(resolved_symbol != nullptr);
                const std::string error = fmt_ns::format(
                    "variable `{}` is not accessible in function `{}`.",
                    id_name, current_function_name);
                const std::string note1 = fmt_ns::format(
                    "function `{}` declared here", current_function_name);
                const std::string note2 = fmt_ns::format(
                    "variable `{}` declared here", id_name);
                et.report_syntax_error(
                    error, id_location,
                    std::list<Diagnostic>{{note1, current_function_location, DT::NOTE},
                                          {note2, resolved_symbol->location(), DT::NOTE}});
        }

        DEBUG_ALWAYS_INLINE bool is_modifiable_lvalue(const Symbol *const lvalue)
        {
                if (lvalue == nullptr) // nullptr implies runtime-evaluated lvalue (e.g. member access)
                        return true;
                return lvalue->is_variable();
        }

        void term__lvalue_op(
            const std::string &op_name,
            const Symbol *lvalue,
            Location term_location,
            ErrorTracker &et)
        {
                // lvalue is valid to be nullptr (runtime evaluation).
                DEBUG_SMART_ASSERT(op_name == "increment" || op_name == "decrement");
                if (is_modifiable_lvalue(lvalue))
                        return;
                std::string error = fmt_ns::format("{} operator can not be used on function", op_name);
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
    const std::string &id_name,
    Location id_location,
    const Symbol **const lvalue_addr,
    ErrorTracker &et)
{
        DEBUG_SMART_ASSERT(lvalue_addr != nullptr);
        const Symbol *resolved_symbol = st.lookup_chain(id_name, parse_ctx.current_scope());
        if (!resolved_symbol)
        {
                if (parse_ctx.current_scope() == k_global_scope)
                        *lvalue_addr = st.insert_global(id_name, id_location);
                else
                        *lvalue_addr = st.insert_local(id_name, parse_ctx.current_scope(), id_location);
                return;
        }
        if (resolved_symbol->is_variable())
        {
                if (resolved_symbol->scope() > k_global_scope &&
                    resolved_symbol->scope() <= parse_ctx.current_function_scope())
                        report_out_of_scope(id_name, id_location, parse_ctx.current_function_name(),
                                            parse_ctx.current_function_location(), resolved_symbol, et);
        }
        *lvalue_addr = resolved_symbol;
}

void lvalue__local_id(
    SymbolTable &st,
    ParseCtx &parse_ctx,
    const std::string &id_name,
    Location id_location,
    const Symbol **const lvalue_addr,
    ErrorTracker &et)
{
        DEBUG_SMART_ASSERT(lvalue_addr != nullptr);
        if (st.is_lib_function(id_name))
        {
                std::string error = fmt_ns::format("shadowing library function `{}`", id_name);
                et.report_syntax_error(error, id_location);
                *lvalue_addr = st.lookup_global(id_name);
                DEBUG_SMART_ASSERT(*lvalue_addr != nullptr); // a library function is always resolved at global scope.
                return;
        }
        const Symbol *resolved_symbol = st.lookup_local(id_name, parse_ctx.current_scope());
        if (resolved_symbol)
                *lvalue_addr = resolved_symbol;
        else if (parse_ctx.current_scope() == k_global_scope)
                *lvalue_addr = st.insert_global(id_name, id_location);
        else
                *lvalue_addr = st.insert_local(id_name, parse_ctx.current_scope(), id_location);
}

void assignExpr__lvalue_assign_expr(
    const Symbol *const lvalue,
    Location assign_location,
    ErrorTracker &et)
{
        if (is_modifiable_lvalue(lvalue))
                return;

        DEBUG_SMART_ASSERT(lvalue != nullptr);
        DEBUG_SMART_ASSERT(lvalue->type() == SymbolType::LIBFUNC ||
                           lvalue->type() == SymbolType::USERFUNC);

        if (lvalue->type() == SymbolType::LIBFUNC)
        {
                std::string error = fmt_ns::format("assignment of library function `{}`", lvalue->name());
                et.report_syntax_error(error, assign_location);
        }
        else
        {
                std::string error = fmt_ns::format("assignment of function `{}`", lvalue->name());
                std::string note = fmt_ns::format("function {} declared here", lvalue->name());
                et.report_syntax_error(error, assign_location, note, lvalue->location());
        }
}

void term__inc_lvalue(
    const Symbol *lvalue,
    Location term_location,
    ErrorTracker &et)
{
        term__lvalue_op("increment", lvalue, term_location, et);
}

void term__lvalue_inc(
    const Symbol *lvalue,
    Location term_location,
    ErrorTracker &et)
{
        term__lvalue_op("increment", lvalue, term_location, et);
}

void term__dec_lvalue(
    const Symbol *lvalue,
    Location term_location,
    ErrorTracker &et)
{
        term__lvalue_op("decrement", lvalue, term_location, et);
}

void term__lvalue_dec(
    const Symbol *lvalue,
    Location term_location,
    ErrorTracker &et)
{
        term__lvalue_op("decrement", lvalue, term_location, et);
}

void lvalue__global_id(
    SymbolTable &st,
    const std::string &id_name,
    Location id_location,
    const Symbol **lvalue_addr,
    ErrorTracker &et)
{
        DEBUG_SMART_ASSERT(lvalue_addr != nullptr);
        *lvalue_addr = st.lookup_global(id_name);
        if (*lvalue_addr)
                return;
        std::string error = fmt_ns::format("variable `::{}` not found in global scope", id_name);
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

void funcDef__function_id_lparen_funcArgList_rparen_block(ParseCtx &parse_ctx) noexcept
{
        parse_ctx.exit_function();
}

void funcDef__function_lparen_funcArgList_rparen_block(ParseCtx &parse_ctx) noexcept
{
        parse_ctx.exit_function();
}

void funcdef__function_id(ParseCtx &parse_ctx, const std::string &id_name, Location id_location)
{
        parse_ctx.last_function_id = id_name;
        parse_ctx.last_function_location = id_location;
}
void funcDef__function_id_lparen_funcArgList_rparen(
    SymbolTable &st,
    ParseCtx &parse_ctx,
    ErrorTracker &et)
{
        if (!reported_function_name_conflict(st, parse_ctx.current_scope(), parse_ctx.last_function_id, parse_ctx.last_function_location, et))
                st.insert_function(parse_ctx.last_function_id, SymbolType::USERFUNC,
                                   parse_ctx.current_scope(), parse_ctx.last_function_location, parse_ctx.retrieve_function_parameters());
        parse_ctx.enter_function(parse_ctx.last_function_id, parse_ctx.last_function_location);
        insert_function_parameters(st, parse_ctx.current_scope(),
                                   parse_ctx.extract_function_parameters(), et);
        parse_ctx.clear_function_arguments();
}

void funcDef__function_lparen_funcArgList_rparen(
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

void funcArgs__id(ParseCtx &parse_ctx, const std::string &id_name, Location id_location)
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

void lvalue__member(const Symbol **const lvalue_addr) noexcept
{
        DEBUG_SMART_ASSERT(lvalue_addr != nullptr);
        *lvalue_addr = nullptr; // We can not resolve members at compile time.
}