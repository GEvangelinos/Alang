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

        void loopCtrlStmt__loopkeyword_impl(const ParseCtx &parse_ctx, CodeLocation keyword_location,
                                            ErrorTracker &error_tracker, Loop::Keyword keyword)
        {
                if (parse_ctx.loop_depth() > 0)
                        return;

                std::string keyword_name = Loop::to_string(keyword);
                std::string error = std::format("`{}` statement not in a loop statement", keyword_name);
                error_tracker.register_syntax_error(error, keyword_location);
        }

        bool DEBUG_ALWAYS_INLINE registered_parameter_name_conflict(const SymbolTable &symbol_table,
                                                                    u32 current_scope,
                                                                    const Parameter &parameter,
                                                                    ErrorTracker &error_tracker)
        {
                // 1) Library‐function conflict
                if (symbol_table.is_lib_function(parameter.name()))
                {
                        const std::string error = std::format(
                            "`{}` is a library function, cant declare it as formal");
                        error_tracker.register_syntax_error(error, parameter.location());
                        return true;
                }
                const Symbol *local_symbol = symbol_table.lookup_local(parameter.name(), current_scope);
                // 2) Parameter‐redeclared conflict
                if (local_symbol)
                {
                        const std::string error = std::format(
                            "redefinition of parameter `{}`", parameter.name());
                        const std::string note = std::format(
                            "previous definition of `{}` here", parameter.name());
                        error_tracker.register_syntax_error(
                            error, parameter.location(), note, local_symbol->location());
                        return true;
                }
                return false;
        }

        void insert_function_parameters(SymbolTable &symbol_table, u32 current_scope,
                                        const std::list<Parameter> &parameter_list, ErrorTracker &error_tracker)
        {
                for (const Parameter &parameter : parameter_list)
                {
                        if (registered_parameter_name_conflict(symbol_table, current_scope, parameter, error_tracker))
                                continue;

                        symbol_table.insert_variable(parameter.name(), SymbolType::FORMAL,
                                                     current_scope, parameter.location());
                }
        }

        bool DEBUG_ALWAYS_INLINE registered_function_name_conflict(SymbolTable &symbol_table, ParseCtx &parse_ctx,
                                                                   const char *id_name, CodeLocation id_location,
                                                                   ErrorTracker &error_tracker)
        {
                if (symbol_table.is_lib_function(id_name))
                {
                        const std::string error = std::format("redefinition of library function `{}`", id_name);
                        error_tracker.register_syntax_error(error, id_location);
                        return true;
                }

                const Symbol *local_symbol = symbol_table.lookup_local(id_name, parse_ctx.current_scope());
                if (!local_symbol)
                        return false;
                if (local_symbol->is_function())
                {
                        const std::string error = std::format("redefinition of `function {}`", id_name);
                        const std::string note = std::format("previous definition of `function {}` here", id_name);
                        error_tracker.register_syntax_error(error, id_location, note, local_symbol->location());
                }
                else if (local_symbol->is_variable())
                {
                        const std::string error = std::format("`{}` defined as a function", id_name);
                        const std::string note = std::format("`{}` previously defined as a variable here", id_name);
                        error_tracker.register_syntax_error(error, id_location, note, local_symbol->location());
                }
                return true;
        }

} // namespace Anonymous

// +-----------------------------------------------------------------+
// |---------------- SEMANTIC_ACTION_FUNCTIONS_BELOW ----------------|
// +-----------------------------------------------------------------+
void loopCtrlStmt__break(const ParseCtx &parse_ctx, CodeLocation break_location,
                         ErrorTracker &error_tracker)
{
        loopCtrlStmt__loopkeyword_impl(parse_ctx, break_location, error_tracker, Loop::Keyword::BREAK);
}

void loopCtrlStmt__continue(const ParseCtx &parse_ctx, CodeLocation continue_location,
                            ErrorTracker &error_tracker)
{
        loopCtrlStmt__loopkeyword_impl(parse_ctx, continue_location, error_tracker, Loop::Keyword::CONTINUE);
}

void funcCtrlStmt__return(const ParseCtx &parse_ctx, CodeLocation return_location,
                          ErrorTracker &error_tracker)
{
        if (parse_ctx.function_depth() > 0)
                return;

        std::string error = "`return` statement not in a function statement";
        error_tracker.register_syntax_error(error, return_location);
}

void lvalue__id(SymbolTable &symbol_table, ParseCtx &parse_ctx,
                const char *id_name, CodeLocation id_location,
                Symbol **lvalue, ErrorTracker &error_tracker)
{
        const u32 current_scope = parse_ctx.current_scope();
        const Symbol *chain_symbol = symbol_table.lookup_chain(id_name, current_scope);
        if ()
}

void lvalue__local_id(
    SymbolTable &symbol_table,
    ParseCtx &parse_ctx,
    const char *id_name,
    CodeLocation id_location,
    ErrorTracker &error_tracker);

void lvalue__global_id(
    SymbolTable &symbol_table,
    ParseCtx &parse_ctx,
    const char *id_name,
    CodeLocation id_location,
    ErrorTracker &error_tracker);

void block__lbrace(ParseCtx &parse_ctx)
{
        parse_ctx.enter_block();
}
void block_lbrace_multiStmt_rbrace(ParseCtx &parse_ctx)
{
        parse_ctx.exit_block();
}

void block_lbrace_rbrace(ParseCtx &parse_ctx)
{
        parse_ctx.exit_block();
}

void funcDef__function_id_lparen_idList_rparen_block(ParseCtx &parse_ctx)
{
        parse_ctx.exit_function();
}

void funcDef__function_lparen_idList_rparen_block(ParseCtx &parse_ctx)
{
        parse_ctx.exit_function();
}

void funcDef__function_id_lparen_idList_rparen(SymbolTable &symbol_table, ParseCtx &parse_ctx,
                                               const char *id_name, CodeLocation id_location,
                                               ErrorTracker &error_tracker)
{
        if (!registered_function_name_conflict(symbol_table, parse_ctx, id_name, id_location, error_tracker))
                symbol_table.insert_function(id_name, SymbolType::USERFUNC,
                                             parse_ctx.current_scope(), id_location,
                                             parse_ctx.retrieve_function_parameters());
        parse_ctx.enter_function();
        insert_function_parameters(symbol_table, parse_ctx.current_scope(),
                                   parse_ctx.extract_function_parameters(), error_tracker);
        parse_ctx.clear_function_arguments();
}

void funcDef__function_lparen_idList_rparen(SymbolTable &symbol_table, ParseCtx &parse_ctx,
                                            CodeLocation function_location, ErrorTracker &error_tracker)
{
        symbol_table.insert_anonymous(parse_ctx.current_scope(), function_location,
                                      parse_ctx.retrieve_function_parameters());
        parse_ctx.enter_function();
        insert_function_parameters(symbol_table, parse_ctx.current_scope(),
                                   parse_ctx.extract_function_parameters(), error_tracker);
        parse_ctx.clear_function_arguments();
}

void funcArgs__id(ParseCtx &parse_ctx, const char *id_name, CodeLocation id_location)
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