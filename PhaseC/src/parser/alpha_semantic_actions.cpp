#include "parser/alpha_semantic_actions.hpp"

#include <list> // for list, _List_const_iterator

#include "core/alpha_error.hpp"            // for ErrorTracker, Diagnostic
#include "core/alpha_konstants.hpp"        // for k_global_scope, k_public_...
#include "core/alpha_location.hpp"         // for Location
#include "core/alpha_macros.hpp"           // for DEBUG_ALWAYS_INLINE
#include "core/alpha_types.hpp"            // for u32
#include "parser/_parser_common.hpp"       // for Parameter
#include "parser/alpha_parser_context.hpp" // for ParseCtx
#include "utils/format_adapter.hpp"        // for format, FMT
#include "utils/smart_assert.h"            // for DEBUG_SMART_ASSERT

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

                std::string to_string(const Keyword keyword) noexcept
                {
                        switch (keyword)
                        {
                        case Keyword::BREAK:
                                return "break";
                        case Keyword::CONTINUE:
                                return "continue";
                        }
                        UNREACHABLE("Some field of Keyword is not registred");
                }
        }; // namespace Loop

        DEBUG_ALWAYS_INLINE
        void loopCtrlStmt__loopkeyword_impl(
            const ParseCtx &parse_ctx,
            const Location keyword_location,
            ErrorTracker &et,
            const Loop::Keyword keyword)
        {
                if (parse_ctx.function_ctx_handler.loop_depth() > 0)
                        return;

                std::string keyword_name = Loop::to_string(keyword);
                std::string error = FMT::format("`{}` statement not in a loop statement", keyword_name);
                et.report_error(CTError::Type::SEMANTIC, error, keyword_location);
        }

        DEBUG_ALWAYS_INLINE
        bool reported_parameter_name_conflict(
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
                        DEBUG_SMART_ASSERT(                                             //
                            (dynamic_cast<const Variable *>(formal_symbol) != nullptr), //
                            (formal_symbol->is_variable())                              //
                        );

                        const std::string error = FMT::format("redefinition of parameter `{}`", parameter.name);
                        const std::string note = FMT::format("previous definition of `{}` here", parameter.name);
                        et.report_error(
                            CTError::Type::SEMANTIC, error, parameter.location, note, formal_symbol->location);
                        return true;
                }
                return false;
        }

        void insert_function_parameters(SymbolTable &st, const ParseCtx &parse_ctx, ErrorTracker &et)
        {
                DEBUG_SMART_ASSERT(parse_ctx.space_handler.space() == Variable::Space::FORMAL_ARGUMENT);

                auto scope = parse_ctx.scope_handler.scope();
                auto offset = parse_ctx.space_handler.offset();
                constexpr auto space = Variable::Space::FORMAL_ARGUMENT;

                for (const Parameter &param : parse_ctx.function_ctx_handler.function_parameters())
                        if (!reported_parameter_name_conflict(st, scope, param, et))
                                st.insert_variable(param.name, scope, space, offset, param.location);
        }

        bool reported_function_name_conflict(
            SymbolTable &st,
            const u32 current_scope,
            const std::string function_name,
            const Location id_location,
            ErrorTracker &et)
        {
                if (st.is_lib_function(function_name))
                {
                        const std::string error =
                            FMT::format("redefinition of library function `{}`", function_name);
                        et.report_error(CTError::Type::SEMANTIC, error, id_location);
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
                            CTError::Type::SEMANTIC, error, id_location, note, found_symbol->location);
                }
                else if (found_symbol->is_variable())
                {
                        const std::string error = FMT::format("`{}` redefined as a function", function_name);
                        const std::string note =
                            FMT::format("`{}` previously defined as a variable here", function_name);
                        et.report_error(
                            CTError::Type::SEMANTIC, error, id_location, note, found_symbol->location);
                }
                return true;
        }

        void report_out_of_scope_variable(
            const std::string &id_name,
            const Location id_location,
            const std::string &current_function_name,
            const Location current_function_location,
            const Symbol *found_symbol,
            ErrorTracker &et)
        {
                using DT = Diagnostic::Type;
                DEBUG_SMART_ASSERT(found_symbol != nullptr);
                const std::string error = FMT::format("variable `{}` is not accessible in function `{}`",
                                                      id_name, current_function_name);
                const std::string note1 = FMT::format("function `{}` declared here", current_function_name);
                const std::string note2 = FMT::format("variable `{}` declared here", id_name);
                et.report_error(CTError::Type::SEMANTIC, error, id_location,
                                std::list<Diagnostic>{{DT::NOTE, note1, current_function_location},
                                                      {DT::NOTE, note2, found_symbol->location}});
        }

        DEBUG_ALWAYS_INLINE
        bool is_modifiable_lvalue(const Symbol *const lvalue)
        {
                if (lvalue == nullptr) // nullptr implies runtime-evaluated lvalue (e.g. member access)
                        return true;
                return lvalue->is_variable();
        }

        DEBUG_ALWAYS_INLINE
        void term__lvalue_op(
            const std::string &op_name,
            const Symbol *lvalue,
            const Location term_location,
            ErrorTracker &et)
        {
                // lvalue is valid to be nullptr (runtime evaluation).
                DEBUG_SMART_ASSERT(op_name == "increment" || op_name == "decrement");
                if (is_modifiable_lvalue(lvalue))
                        return;
                std::string error = FMT::format("{} operator can not be used on function", op_name);
                et.report_error(CTError::Type::SEMANTIC, error, term_location);
        }

} // namespace

// +-----------------------------------------------------------------+
// |---------------- SEMANTIC_ACTION_FUNCTIONS_BELOW ----------------|
// +-----------------------------------------------------------------+
void loopCtrlStmt__break(const ParseCtx &parse_ctx, const Location break_location, ErrorTracker &et)
{
        loopCtrlStmt__loopkeyword_impl(parse_ctx, break_location, et, Loop::Keyword::BREAK);
}

void loopCtrlStmt__continue(const ParseCtx &parse_ctx, const Location continue_location, ErrorTracker &et)
{
        loopCtrlStmt__loopkeyword_impl(parse_ctx, continue_location, et, Loop::Keyword::CONTINUE);
}

void term__inc_lvalue(const Symbol *lvalue, const Location term_location, ErrorTracker &et)
{
        term__lvalue_op("increment", lvalue, term_location, et);
}

void term__lvalue_inc(const Symbol *lvalue, const Location term_location, ErrorTracker &et)
{
        term__lvalue_op("increment", lvalue, term_location, et);
}

void term__dec_lvalue(const Symbol *lvalue, const Location term_location, ErrorTracker &et)
{
        term__lvalue_op("decrement", lvalue, term_location, et);
}

void term__lvalue_dec(const Symbol *lvalue, const Location term_location, ErrorTracker &et)
{
        term__lvalue_op("decrement", lvalue, term_location, et);
}

void assignExpr__lvalue_assign_expr(
    const Symbol *lvalue,
    const Location assign_location,
    ErrorTracker &et)
{
        if (is_modifiable_lvalue(lvalue))
                return;

        DEBUG_SMART_ASSERT(lvalue != nullptr);
        DEBUG_SMART_ASSERT(lvalue->is_function());

        if (lvalue->type == Symbol::Type::LIBRARY_FUNCTION)
        {
                std::string error = FMT::format("assignment of library function `{}`", lvalue->name);
                et.report_error(CTError::Type::SEMANTIC, error, assign_location);
        }
        else
        {
                std::string error = FMT::format("assignment of function `{}`", lvalue->name);
                std::string note = FMT::format("function {} declared here", lvalue->name);
                et.report_error(CTError::Type::SEMANTIC, error, assign_location, note, lvalue->location);
        }
}

void lvalue__id(
    SymbolTable &st,
    const ParseCtx &parse_ctx,
    const std::string &id_name,
    const Location id_location,
    const Symbol *&lvalue,
    ErrorTracker &et)
{
        const Symbol *found_symbol = st.lookup_chain(id_name, parse_ctx.scope_handler.scope());
        if (!found_symbol)
        {
                lvalue = st.insert_variable(
                    id_name,
                    parse_ctx.scope_handler.scope(),
                    parse_ctx.space_handler.space(),
                    parse_ctx.space_handler.offset(),
                    id_location);
                return;
        }
        if (found_symbol->is_variable() &&
            found_symbol->scope > k_global_scope &&
            found_symbol->scope <= parse_ctx.function_ctx_handler.function_scope())
                report_out_of_scope_variable(
                    id_name,
                    id_location, parse_ctx.function_ctx_handler.function_name(),
                    parse_ctx.function_ctx_handler.function_location(),
                    found_symbol,
                    et);
        lvalue = found_symbol;
}

void lvalue__local_id(
    SymbolTable &st,
    ParseCtx &parse_ctx,
    const std::string &id_name,
    const Location id_location,
    const Symbol *&lvalue,
    ErrorTracker &et)
{
        if (st.is_lib_function(id_name))
        {
                std::string error = FMT::format("shadowing library function `{}`", id_name);
                et.report_error(CTError::Type::SEMANTIC, error, id_location);
                lvalue = st.lookup_global(id_name);
                DEBUG_SMART_ASSERT(lvalue != nullptr); // a library function is always resolved at global scope.
                return;
        }
        const Symbol *found_symbol = st.lookup_local(id_name, parse_ctx.scope_handler.scope());
        if (found_symbol)
                lvalue = found_symbol;
        else
                lvalue = st.insert_variable(
                    id_name,
                    parse_ctx.scope_handler.scope(),
                    parse_ctx.space_handler.space(),
                    parse_ctx.space_handler.offset(),
                    id_location);
}

void lvalue__global_id(
    SymbolTable &st,
    const std::string &id_name,
    const Location id_location,
    const Symbol *&lvalue,
    ErrorTracker &et)
{
        lvalue = st.lookup_global(id_name);
        if (lvalue)
                return;
        std::string error = FMT::format("variable `::{}` not found in global scope", id_name);
        et.report_error(CTError::Type::SEMANTIC, error, id_location);
}

void lvalue__member(const Symbol *&lvalue) noexcept
{
        lvalue = nullptr; // We can not resolve members at compile time.
}

void blockOpen__lbrace(ParseCtx &parse_ctx) noexcept
{
        parse_ctx.scope_handler.enter_scope();
}

void blockClose__rbrace(SymbolTable &st, ParseCtx &parse_ctx) noexcept
{
        st.hide_scope_symbols(parse_ctx.scope_handler.scope());
        parse_ctx.scope_handler.exit_scope();
}

void funcPrefix__function(ParseCtx &parse_ctx, const Location anonymous_location)
{
        parse_ctx.function_ctx_handler.last_function_id =
            std::string(k_private_anonymous_prefix) +
            std::to_string(parse_ctx.function_ctx_handler.anonymous_counter());
        parse_ctx.function_ctx_handler.last_function_location = anonymous_location;
        parse_ctx.function_ctx_handler.last_function_is_anonymous = true;
}

void funcPrefix__function_id(ParseCtx &parse_ctx, const std::string &id_name, Location id_location)
{
        parse_ctx.function_ctx_handler.last_function_id = id_name;
        parse_ctx.function_ctx_handler.last_function_location = id_location;
        parse_ctx.function_ctx_handler.last_function_is_anonymous = false;
}

void funcSignature__funcPrefix_funcArgList(SymbolTable &st, ParseCtx &parse_ctx, ErrorTracker &et)
{

        bool conflicting_name = reported_function_name_conflict(
            st,
            parse_ctx.scope_handler.scope(),
            parse_ctx.function_ctx_handler.last_function_id,
            parse_ctx.function_ctx_handler.last_function_location,
            et);

        if (!conflicting_name)
                st.insert_function(
                    parse_ctx.function_ctx_handler.last_function_id,
                    parse_ctx.scope_handler.scope(),
                    parse_ctx.function_ctx_handler.function_parameters(),
                    parse_ctx.function_ctx_handler.last_function_location);

        parse_ctx.function_ctx_handler.enter_function(parse_ctx.scope_handler);

        insert_function_parameters(st, parse_ctx, et);
}

void funcDef__funcSignature_block(ParseCtx &parse_ctx) noexcept
{
        parse_ctx.function_ctx_handler.exit_function();
}

void const__stringliteral(char *&string_literal)
{
        delete[] string_literal;
        string_literal = nullptr;
}

void funcArgs__id(ParseCtx &parse_ctx, const std::string &id_name, const Location id_location)
{
        parse_ctx.function_ctx_handler.add_function_parameter(id_name, id_location);
}

void whileStmt__whileHeader(ParseCtx &parse_ctx) noexcept
{
        parse_ctx.function_ctx_handler.enter_loop();
}

void whileStmt__whileHeader_stmt(ParseCtx &parse_ctx) noexcept
{
        parse_ctx.function_ctx_handler.exit_loop();
}

void forStmt__forHeader(ParseCtx &parse_ctx) noexcept
{
        parse_ctx.function_ctx_handler.enter_loop();
}

void forStmt__forHeader_stmt(ParseCtx &parse_ctx) noexcept
{
        parse_ctx.function_ctx_handler.exit_loop();
}

void funcCtrlStmt__return(
    const ParseCtx &parse_ctx,
    const Location return_location,
    ErrorTracker &et)
{
        if (parse_ctx.function_ctx_handler.function_nesting_depth() > 0)
                return;
        std::string error = "`return` statement not in a function statement";
        et.report_error(CTError::Type::SEMANTIC, error, return_location);
}
