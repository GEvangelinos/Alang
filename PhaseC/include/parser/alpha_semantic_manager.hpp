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

} // namespace (Anonymous)

namespace Alpha
{

        class SemanticManager
        {
        public:
                SemanticManager(ParseCtx &parse_ctx, SymbolTable &st, ErrorTracker &et);
                void loopCtrlStmt__break(Location break_loc);
                void loopCtrlStmt__continue(Location continue_loc);
                void term__inc_lvalue(const Symbol *lvalue, Location term_loc);
                void term__lvalue_inc(const Symbol *lvalue, Location term_loc);
                void term__dec_lvalue(const Symbol *lvalue, Location term_loc);
                void term__lvalue_dec(const Symbol *lvalue, Location term_loc);
                void lvalue__id(Expr *&lvalue, const char *id_name, Location id_loc);
                void lvalue__local_id(Expr *&lvalue, const char *id_name, Location id_loc);
                void lvalue__global_id(Expr *&lvalue, const char *id_name, Location id_loc);
                void blockBegin__lbrace() noexcept;
                void blockEnd__rbrace() noexcept;
                void funcPrefix__function(Location anonymous_loc);
                void funcPrefix__function_id(const char *id_name, Location id_loc);
                void funcSignature__funcPrefix_funcArgList(const Function *&funcSignature);
                void funcDef__funcSignature_block(const BlockLocation &block_loc) noexcept;
                void funcArgs__id(const char *id_name, Location id_loc);
                void whileStmt__whileHeader() noexcept;
                void whileStmt__whileHeader_stmt() noexcept;
                void forStmt__forHeader() noexcept;
                void forStmt__forHeader_stmt() noexcept;
                void funcCtrlStmt__return(Location return_loc);

                // Tool functions:
                static void update_expr_location(Expr *expr, Location new_expr_loc);

        private:
                ParseCtx &parse_ctx_;
                SymbolTable &st_;
                ErrorTracker &et_;

                void loopCtrlStmt__loopkeyword_impl(Loop::Keyword keyword, Location keyword_loc);
                void term__lvalue_op(const char *op_name, const Symbol *lvalue, Location term_loc);
                void report_out_of_scope_variable(
                    const char *id_name,
                    const std::string &current_function_name,
                    const Symbol *found_symbol,
                    Location id_loc,
                    Location current_function_loc);
                [[nodiscard]] bool reported_function_name_conflict(
                    const std::string &function_name,
                    u32 current_scope,
                    Location id_loc);
                void insert_gathered_function_parameters();
                [[nodiscard]] bool reported_parameter_name_conflict(
                    u32 current_scope,
                    const Parameter &parameter);
        }; // class SemanticManager

        inline SemanticManager::SemanticManager(ParseCtx &parse_ctx, SymbolTable &st, ErrorTracker &et)
            : parse_ctx_(parse_ctx),
              st_(st),
              et_(et) {}

        inline void
        SemanticManager::loopCtrlStmt__loopkeyword_impl(
            const Loop::Keyword keyword,
            const Location keyword_loc)
        {
                if (parse_ctx_.function_ctx_handler.loop_depth() > 0)
                        return;

                std::string keyword_name = Loop::to_string(keyword);
                std::string error = FMT::format("`{}` statement not in a loop statement", keyword_name);
                et_.report_error(CTError::Type::SEMANTIC, error, keyword_loc);
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
                et_.report_error(CTError::Type::SEMANTIC, error, term_loc);
        }

        inline void
        SemanticManager::report_out_of_scope_variable(
            const char *id_name,
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
                et_.report_error(
                    CTError::Type::SEMANTIC,
                    error, id_loc,
                    std::list<Diagnostic>{
                        {DT::NOTE, note1, current_function_loc},
                        {DT::NOTE, note2, found_symbol->location} //
                    } //
                );
        }

        inline bool
        SemanticManager::reported_function_name_conflict(
            const std::string &function_name,
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
                        const std::string error = FMT::format("redefinition of `function {}`", function_name);
                        const std::string note =
                            FMT::format("previous definition of `function {}` here", function_name);
                        et_.report_error(
                            CTError::Type::SEMANTIC, error, id_loc, note, found_symbol->location);
                }
                else if (found_symbol->is_variable())
                {
                        const std::string error = FMT::format("`{}` redefined as a function", function_name);
                        const std::string note =
                            FMT::format("`{}` previously defined as a variable here", function_name);
                        et_.report_error(
                            CTError::Type::SEMANTIC, error, id_loc, note, found_symbol->location);
                }
                return true;
        }
        inline void
        SemanticManager::insert_gathered_function_parameters()
        {

                auto current_scope = parse_ctx_.scope_handler.scope();
                constexpr auto space = Variable::Space::FORMAL_ARGUMENT;
                DEBUG_SMART_ASSERT(parse_ctx.space_handler.space() == Variable::Space::FORMAL_ARGUMENT);

                for (const Parameter &param : parse_ctx_.function_ctx_handler.function_parameters())
                        if (!reported_parameter_name_conflict(current_scope, param))
                                st_.insert_variable(
                                    param.name,
                                    current_scope,
                                    space,
                                    parse_ctx_.space_handler.next_offset(),
                                    param.location);
        }

        inline bool
        SemanticManager::reported_parameter_name_conflict(
            const u32 current_scope,
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

                        const std::string error = FMT::format("redefinition of parameter `{}`", parameter.name);
                        const std::string note = FMT::format("previous definition of `{}` here", parameter.name);
                        et_.report_error(
                            CTError::Type::SEMANTIC, error, parameter.location, note, formal_symbol->location);
                        return true;
                }
                return false;
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
                const Symbol *symbol = st_.lookup_chain(
                    id_name,
                    parse_ctx_.scope_handler.scope() //
                );

                if (!symbol)
                        symbol = st_.insert_variable(
                            id_name,
                            parse_ctx_.scope_handler.scope(),
                            parse_ctx_.space_handler.space(),
                            parse_ctx_.space_handler.next_offset(),
                            id_loc);
                else if (symbol->is_variable() &&
                         symbol->scope > k_global_scope &&
                         symbol->scope <= parse_ctx_.function_ctx_handler.current_function_scope())
                        report_out_of_scope_variable(
                            id_name,
                            parse_ctx_.function_ctx_handler.current_function_name(),
                            symbol,
                            id_loc,
                            parse_ctx_.function_ctx_handler.current_function_location());

                lvalue = parse_ctx_.expr_handler.make_expr_variable(symbol, symbol->location); // TODO : PROBABLY not symbol's location
        }

        inline void
        SemanticManager::lvalue__local_id(Expr *&lvalue, const char *id_name, const Location id_loc)
        {
                const Symbol *symbol = nullptr;
                if (st_.is_lib_function(id_name))
                {
                        std::string error = FMT::format("shadowing library function `{}`", id_name);
                        et_.report_error(CTError::Type::SEMANTIC, error, id_loc);
                        symbol = st_.lookup_global(id_name);
                        DEBUG_SMART_ASSERT(!!symbol); // a library function is always resolved at global scope.
                }
                else
                {
                        symbol = st_.lookup_local(id_name, parse_ctx_.scope_handler.scope());
                        if (!symbol)
                                symbol = st_.insert_variable(
                                    id_name,
                                    parse_ctx_.scope_handler.scope(),
                                    parse_ctx_.space_handler.space(),
                                    parse_ctx_.space_handler.next_offset(),
                                    id_loc);
                }

                lvalue = parse_ctx_.expr_handler.make_expr_variable(symbol, symbol->location); // TODO : PROBABLY not symbol's location
        }

        inline void
        SemanticManager::lvalue__global_id(Expr *&lvalue, const char *id_name, const Location id_loc)
        {
                // TODO: I dont like producing shit if global symbol doesnt exist.
                // TODO: we must find an anchor/hook point, where we can reset, and continue
                // TODO : producing quads even if they will never be used.
                // TODO: Or disable quad emission all together even if a single error is found. (?)
                const Symbol *symbol = st_.lookup_global(id_name);
                if (symbol)
                {
                        lvalue = parse_ctx_.expr_handler.make_expr_variable(symbol, symbol->location); // TODO : PROBABLY not symbol's location
                        return;
                }
                std::string error = FMT::format("variable `::{}` not found in global scope", id_name);
                et_.report_error(CTError::Type::SEMANTIC, error, id_loc);
        }

        inline void
        SemanticManager::blockBegin__lbrace() noexcept
        {
                parse_ctx_.scope_handler.enter_scope();
        }

        inline void
        SemanticManager::blockEnd__rbrace() noexcept
        {
                st_.hide_scope_symbols(parse_ctx_.scope_handler.scope());
                parse_ctx_.scope_handler.exit_scope();
        }

        inline void
        SemanticManager::funcPrefix__function(const Location anonymous_loc)
        {
                // Update ParseCache:
                parse_ctx_.cache.func_prefix.id = parse_ctx_.name_generator.new_anonymous();
                parse_ctx_.cache.func_prefix.location = anonymous_loc;

                parse_ctx_.space_handler.enter_space();
        }

        inline void
        SemanticManager::funcPrefix__function_id(const char *id_name, const Location id_loc)
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
        inline void
        SemanticManager::funcSignature__funcPrefix_funcArgList(const Function *&funcSignature)
        {
                const Location func_loc = parse_ctx_.cache.func_prefix.location;
                bool conflicting_name = reported_function_name_conflict(
                    parse_ctx_.cache.func_prefix.id,
                    parse_ctx_.scope_handler.scope(),
                    func_loc);

                const Function *function_symbol = nullptr;
                if (!conflicting_name)
                {
                        function_symbol = st_.insert_function(
                            parse_ctx_.cache.func_prefix.id,
                            parse_ctx_.scope_handler.scope(),
                            parse_ctx_.function_ctx_handler.next_function_address(),
                            parse_ctx_.function_ctx_handler.function_parameters(),
                            func_loc);

                        parse_ctx_.quad_handler.emit_quad(
                            IOPCode::FUNCSTART,
                            nullptr,
                            nullptr,
                            parse_ctx_.expr_handler.make_expr_variable(function_symbol, func_loc),
                            func_loc);
                }
                parse_ctx_.function_ctx_handler.enter_function(function_symbol);
                insert_gathered_function_parameters();
                parse_ctx_.function_ctx_handler.clear_function_parameters();
                parse_ctx_.space_handler.enter_space(); // IMPORTANT: This line is after parameter insertion!

                funcSignature = function_symbol;
        }

        inline void
        SemanticManager::funcDef__funcSignature_block(const BlockLocation &block_loc) noexcept
        {
                auto fbi = parse_ctx_.function_ctx_handler.exit_function();
                if (!!fbi.function_symbol)
                {
                        Backpatcher::set_function_local_variable_count(
                            fbi.function_symbol,
                            fbi.local_variable_count);

                        parse_ctx_.quad_handler.emit_quad(
                            IOPCode::FUNCEND,
                            nullptr,
                            nullptr,
                            parse_ctx_.expr_handler.make_expr_variable(fbi.function_symbol, k_no_location), // TODO: what location here?
                            block_loc.end);
                }

                parse_ctx_.space_handler.exit_space();
        }

        inline void
        SemanticManager::funcArgs__id(const char *id_name, const Location id_loc)
        {
                parse_ctx_.function_ctx_handler.add_function_parameter(id_name, id_loc);
        }

        inline void
        SemanticManager::whileStmt__whileHeader() noexcept
        {
                parse_ctx_.function_ctx_handler.enter_loop();
        }

        inline void
        SemanticManager::whileStmt__whileHeader_stmt() noexcept
        {
                parse_ctx_.function_ctx_handler.exit_loop();
        }

        inline void
        SemanticManager::forStmt__forHeader() noexcept
        {
                parse_ctx_.function_ctx_handler.enter_loop();
        }

        inline void
        SemanticManager::forStmt__forHeader_stmt() noexcept
        {
                parse_ctx_.function_ctx_handler.exit_loop();
        }

        inline void
        SemanticManager::funcCtrlStmt__return(const Location return_loc)
        {
                if (parse_ctx_.function_ctx_handler.function_nesting_depth() > 0)
                        return;
                std::string error = "`return` statement not in a function statement";
                et_.report_error(CTError::Type::SEMANTIC, error, return_loc);
        }
} // namespace Alpha

#endif /* SEMANTIC_MANAGER_HPP */