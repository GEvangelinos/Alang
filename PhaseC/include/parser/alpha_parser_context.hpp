// Note: Alpha Parser Context is implemented as a header-only class.
//       Since it consists mostly of small, frequently used functions,
//       we define all methods in the header and mark them as `inline`.
//
//       This serves two purposes:
//       1) To comply with the One Definition Rule (ODR) when the header
//          is included in multiple translation units.
//       2) To hint to the compiler to inline calls for better performance.

#ifndef ALPHA_PARSER_CONTEXT_HPP
#define ALPHA_PARSER_CONTEXT_HPP

#include "_parser_common.hpp"
#include "core/alpha_ir.hpp"
#include "core/alpha_konstants.hpp"
#include "utils/misc.hpp"
#include "core/alpha_types.hpp"
#include "utils/smart_assert.h"
#include <limits>
#include <list>
#include <stack>
#include <vector>
#include <memory>

namespace Alpha
{
        // Classes define here:
        struct ParseCache;
        class SpaceHandler;
        class SpaceHandler;
        class ScopeHandler;
        class FunctionCtxHandler;
        struct FunctionBackpatchInfo;
        struct FunctionDataFrame;
        class NameGenerator;
        class QuadHandler;
        class ExprHandler;
        class ParseCtx;

        /**
         * @brief Temporary semantic state used during parsing.
         *
         * ParseCache holds intermediate data needed across specific grammar rules
         * during Bison parsing. It stores complex semantic values that cannot
         * safely or cleanly be represented in the %union (e.g., std::string,
         * structs with non-trivial constructors).
         *
         * This separation exists because the parser uses Bison's C backend,
         * which relies on a raw union for token values. To maintain clean
         * memory safety and modern C++ idioms (RAII, strong typing), ParseCache
         * acts as a companion scratchpad for semantic actions that require
         * richer state tracking.
         *
         * Each substructure in ParseCache typically maps to a specific grammar rule
         * or parsing context (e.g., function headers, block spans), allowing
         * clean separation, clarity, and scalability.
         *
         * Lifetime: ParseCache is owned by ParseCtx and lives for the duration
         * of parsing a single alpha source file.
         */
        struct ParseCache
        {
                struct funcPrefixState
                {
                        std::string id;
                        Location location;
                } func_prefix;
        };

        class SpaceHandler : private Immobile
        {
        public:
                static constexpr u32 k_initial_space = 0;
                static_assert(k_initial_space == 0, "Space mapping logic requires 0");
                static constexpr u32 k_initial_variable_offset = 0;
                static_assert(k_initial_variable_offset == 0, "Variable offset must start at 0");

                SpaceHandler();
                ~SpaceHandler();

                void enter_space();
                void exit_space();
                [[nodiscard]] Variable::Space space() const noexcept;

                [[nodiscard("Discarding this return value breaks variable offset sequencing")]]
                u32 next_offset() noexcept;

        private:
                std::stack<u32> variable_offset_stack_;
        };

        class ScopeHandler : private Immobile
        {
        public:
                ScopeHandler();
                ~ScopeHandler() = default;

                void skip_next_scope_increment() noexcept;
                void enter_scope() noexcept;
                void exit_scope() noexcept;
                [[nodiscard]] u32 scope() const noexcept { return scope_; }

        private:
                ToggleSwitch skip_next_scope_increment_;
                u32 scope_;
        };

        class FunctionCtxHandler : private Immobile
        {
        public:
                struct FunctionBackpatchInfo
                {
                        const std::string name;
                        const u32 scope;
                        const Location location;
                        const u32 local_variable_count;
                        const Function *function_symbol;
                };

                FunctionCtxHandler(ParseCtx *parse_ctx);
                ~FunctionCtxHandler();

                void enter_function(const Function *function_symbol);
                [[nodiscard]] FunctionBackpatchInfo exit_function() noexcept;

                [[nodiscard]] u32 function_nesting_depth() const noexcept;
                [[nodiscard]] u32 current_function_scope() const noexcept;
                [[nodiscard]] const std::string &current_function_name() const noexcept;
                [[nodiscard]] Location current_function_location() const noexcept;

                void enter_loop() noexcept;
                void exit_loop() noexcept;
                [[nodiscard]] u32 loop_depth() const noexcept;

                void add_function_parameter(const std::string &name, Location location);
                [[nodiscard]] const std::list<Parameter> &function_parameters() const noexcept;
                void clear_function_parameters() noexcept;

                [[nodiscard]] u32 next_function_address() noexcept { return next_function_address_++; }

                void add_local() noexcept;

        private:
                struct FunctionDataFrame
                {
                        const std::string name;
                        const u32 scope;
                        const Location location;
                        const Function *function_symbol; // Valid function ONLY IF NOT nullptr;

                        u32 loop_nesting_count = 0;
                        u32 local_variable_count = 0;
                };
                std::stack<FunctionDataFrame> frame_stack_;
                std::list<Parameter> function_parameters_;
                u32 next_function_address_ = 0;

                ParseCtx *const parse_ctx_;
        };

        class NameGenerator : private Immobile
        {
        public:
                [[nodiscard]] std::string new_temp_name();
                void reset_temp_names() { temp_name_counter_ = 0; }

                [[nodiscard]] std::string new_anonymous();

        private:
                u32 temp_name_counter_ = 0;
                u32 anonymous_counter_ = 0;
        };

        class QuadHandler : private Immobile
        {
        public:
                void emit_quad(
                    IOPCode iopcode,
                    const Expr *arg1,
                    const Expr *arg2,
                    const Expr *result,
                    Location location);

                [[nodiscard]] const std::vector<Quad> &quads() const { return quads_; }

        private:
                std::vector<Quad> quads_;
                u32 next_quad_label_ = 1; // First quad_label is always 1, (0 for backpatching)
        };

        class ExprHandler : private Immobile
        {
        public:
                ExprHandler(ParseCtx *parse_ctx);
                ~ExprHandler() noexcept;

                [[nodiscard]] Expr *make_expr_lvalue(const Symbol *symbol);
                [[nodiscard]] Expr *make_expr_const_string(const char *str_value, Location str_location);
                [[nodiscard]] Expr *make_expr_const_real(decltype(Expr::const_real) real_value, Location real_location);
                [[nodiscard]] Expr *make_expr_const_int(decltype(Expr::const_int) int_value, Location int_location);
                [[nodiscard]] Expr *make_expr_const_bool(bool bool_value, Location bool_location);
                [[nodiscard]] Expr *make_expr_const_nil(Location nil_location);
                [[nodiscard]] Expr *make_expr_assign(Expr *rvalue);
                [[nodiscard]] Expr *make_expr_assign(const Symbol *symbol);
                [[nodiscard]] Expr *make_expr_table_item(
                    SymbolTable &st,
                    Expr *&lvalue,
                    const char *id,
                    Location id_location);
                [[nodiscard]] Expr *make_expr_table_item(
                    SymbolTable &st,
                    Expr *&lvalue,
                    Expr *expr);

                Expr *emit_quad_if_table_item(SymbolTable &st, Expr *lvalue);

        private:
                std::vector<const Expr *> expr_sink_;
                ParseCtx *const parse_ctx_;
        };

        class ParseCtx : private Immobile
        {
        public:
                ParseCache cache;
                SpaceHandler space_handler;
                ScopeHandler scope_handler;
                FunctionCtxHandler function_ctx_handler;
                QuadHandler quad_handler;
                ExprHandler expr_handler;
                NameGenerator name_generator;

                ParseCtx();
                ~ParseCtx() = default;

                const Symbol *new_temp(SymbolTable &st);
        };

        inline SpaceHandler::SpaceHandler()
        {
                enter_space(); // We push the first scope space frame (PROGRAM_VAR)
        };

        inline SpaceHandler::~SpaceHandler()
        {
                DEBUG_SMART_ASSERT(variable_offset_stack_.size() == 1);
        }

        inline void SpaceHandler::enter_space()
        {
                variable_offset_stack_.push(k_initial_variable_offset);
        }

        inline void SpaceHandler::exit_space()
        {
                constexpr auto spaces_for_closure = 2; // 1 formalArg + 1 functionLocal

                DEBUG_SMART_ASSERT(                                     //
                    variable_offset_stack_.size() > spaces_for_closure, //
                    is_odd(variable_offset_stack_.size())               //
                );

                // clang-format off
                #pragma unroll
                for (auto i = 0; i < spaces_for_closure; ++i)
                        variable_offset_stack_.pop();
                // clang-format on
        }

        inline Variable::Space SpaceHandler::space() const noexcept
        {
                DEBUG_SMART_ASSERT(variable_offset_stack_.size() > 0);      // A stack frame must always exist
                const auto frame_index = variable_offset_stack_.size() - 1; // -1 for size to index

                if (frame_index == k_initial_space)
                        return Variable::Space::PROGRAM_VAR;
                if (is_odd(frame_index))
                        return Variable::Space::FORMAL_ARGUMENT;
                return Variable::Space::FUNCTION_LOCAL;
        }

        inline u32 SpaceHandler::next_offset() noexcept
        {
                DEBUG_SMART_ASSERT(variable_offset_stack_.size() > 0);
                return variable_offset_stack_.top()++;
        }

        inline ScopeHandler::ScopeHandler() : scope_(k_global_scope)
        {
                // A ToggleSwitch must always be initialized as disabled.
                SMART_ASSERT(skip_next_scope_increment_.is_disabled());
        }

        inline void ScopeHandler::skip_next_scope_increment() noexcept
        {
                skip_next_scope_increment_.enable();
        }

        inline void ScopeHandler::enter_scope() noexcept
        {
                if (skip_next_scope_increment_.is_enabled())
                {
                        skip_next_scope_increment_.disable();
                        return;
                }
                DEBUG_SMART_ASSERT(scope_ < k_max_scope);
                ++scope_;
        }

        inline void ScopeHandler::exit_scope() noexcept
        {
                // We expect `skip_next_scope_increment` ToggleSwitch to be
                // disabled. That is because if you exit a block, it means you first
                // entered it. So if you exit a block and the
                // `skip_next_scope_increment` switch is enabled, there is a logic
                // error.
                DEBUG_SMART_ASSERT(                          //
                    scope_ > k_global_scope,                 //
                    skip_next_scope_increment_.is_disabled() //
                );
                --scope_;
        }

        inline FunctionCtxHandler::FunctionCtxHandler(ParseCtx *const parse_ctx)
            : parse_ctx_(parse_ctx)
        {
                SMART_ASSERT(!!parse_ctx_);
                // We push a stackframe, for loops that might occur outside
                // functions. So every frame corresponds to a function except the
                // first.
                frame_stack_.emplace(FunctionDataFrame{
                    .name = k_global_data_frame_name,
                    .scope = k_global_scope,
                    .location = k_no_location,
                    .function_symbol = nullptr // There is no function, so no function symbol.
                });
        }

        inline FunctionCtxHandler::~FunctionCtxHandler()
        {
                DEBUG_SMART_ASSERT(                                   //
                    frame_stack_.size() == k_global_data_frame_count, //
                    function_parameters_.size() == 0                  // All parameters must be used.
                );
        }

        inline void FunctionCtxHandler::enter_function(const Function *function_symbol)
        {
#ifdef DEBUG_MODE
                DEBUG_SMART_ASSERT(frame_stack_.size() < k_max_function_nesting);
                if (!!function_symbol)
                        DEBUG_SMART_ASSERT(
                            function_symbol->name == parse_ctx_->cache.func_prefix.id,
                            function_symbol->scope == parse_ctx_->scope_handler.scope(),
                            function_symbol->location == parse_ctx_->cache.func_prefix.location,
                            function_symbol->is_function(),
                            function_symbol->type == Symbol::Type::PROGRAM_FUNCTION
                            // Only library functions are defined in source code.
                        );
#endif // DEBUG_MODE

                frame_stack_.emplace(FunctionDataFrame{
                    .name = parse_ctx_->cache.func_prefix.id,
                    .scope = parse_ctx_->scope_handler.scope(),
                    .location = parse_ctx_->cache.func_prefix.location,
                    .function_symbol = function_symbol //
                });

                // Function scope is entered here.
                // We skip the next `{` block’s scope to avoid double scoping.
                parse_ctx_->scope_handler.enter_scope();
                parse_ctx_->scope_handler.skip_next_scope_increment();
        }

        inline FunctionCtxHandler::FunctionBackpatchInfo
        FunctionCtxHandler::exit_function() noexcept
        {
                // A frame always exist for loops outside functions.
                DEBUG_SMART_ASSERT(frame_stack_.size() > k_global_data_frame_count);
                // All loops must be closed before exiting function.
                DEBUG_SMART_ASSERT(frame_stack_.top().loop_nesting_count == 0);

                FunctionDataFrame top_frame = std::move(frame_stack_.top());
                frame_stack_.pop();

                return {
                    .name = top_frame.name,
                    .scope = top_frame.scope,
                    .location = top_frame.location,
                    .local_variable_count = top_frame.local_variable_count,
                    .function_symbol = top_frame.function_symbol};
        }

        inline u32 FunctionCtxHandler::function_nesting_depth() const noexcept
        {
                return frame_stack_.size() - k_global_data_frame_count;
        }

        inline u32 FunctionCtxHandler::current_function_scope() const noexcept
        {
                DEBUG_SMART_ASSERT(frame_stack_.size() > 0);
                return frame_stack_.top().scope;
        }

        inline const std::string &FunctionCtxHandler::current_function_name() const noexcept
        {
                DEBUG_SMART_ASSERT(frame_stack_.size() > 0);
                return frame_stack_.top().name;
        }

        inline Location FunctionCtxHandler::current_function_location() const noexcept
        {
                DEBUG_SMART_ASSERT(frame_stack_.size() > 0);
                return frame_stack_.top().location;
        }

        inline void FunctionCtxHandler::enter_loop() noexcept
        {
                DEBUG_SMART_ASSERT(frame_stack_.size() > 0);
                DEBUG_SMART_ASSERT(frame_stack_.top().loop_nesting_count < k_max_loop_nesting);
                ++frame_stack_.top().loop_nesting_count;
        }

        inline void FunctionCtxHandler::exit_loop() noexcept
        {
                DEBUG_SMART_ASSERT(frame_stack_.size() > 0);
                DEBUG_SMART_ASSERT(frame_stack_.top().loop_nesting_count > 0);
                --frame_stack_.top().loop_nesting_count;
        }

        inline u32 FunctionCtxHandler::loop_depth() const noexcept
        {
                DEBUG_SMART_ASSERT(frame_stack_.size() > 0);
                return frame_stack_.top().loop_nesting_count;
        }

        inline void FunctionCtxHandler::add_function_parameter(
            const std::string &name,
            Location location)
        {
                function_parameters_.emplace_back(name, location);
        }

        inline const std::list<Parameter> &FunctionCtxHandler::function_parameters() const noexcept
        {
                return function_parameters_;
        }

        inline void FunctionCtxHandler::clear_function_parameters() noexcept
        {
                function_parameters_.clear();
        }

        inline void FunctionCtxHandler::add_local() noexcept
        {
                ++frame_stack_.top().local_variable_count;
        }

        inline std::string NameGenerator::new_temp_name()
        {
                return k_temp_variable_prefix + std::to_string(temp_name_counter_++);
        }
        inline std::string NameGenerator::new_anonymous()
        {
                return k_private_anonymous_prefix + std::to_string(anonymous_counter_++);
        }

        inline void QuadHandler::emit_quad(
            const IOPCode iopcode,
            const Expr *arg1,
            const Expr *arg2,
            const Expr *result,
            const Location location)
        {
                DEBUG_SMART_ASSERT(quads_.size() + 1 == next_quad_label_);

                quads_.emplace_back(Quad{
                    .iopcode = iopcode,
                    .arg1 = arg1,
                    .arg2 = arg2,
                    .result = result,
                    .label = next_quad_label_++,
                    .location = location,
                });
        }

        inline Expr *ExprHandler::make_expr_lvalue(const Symbol *const symbol)
        {
                DEBUG_SMART_ASSERT(!!symbol);
                DEBUG_SMART_ASSERT(
                    symbol->type != Symbol::Type::LIBRARY_FUNCTION,
                    symbol->type != Symbol::Type::PROGRAM_FUNCTION,
                    symbol->type == Symbol::Type::VARIABLE //
                );

                Expr *expr_lvalue = new Expr{
                    .type = Expr::Type::VARIABLE,
                    .symbol = symbol,
                    .index = nullptr,
                    .next = nullptr,
                };
                expr_sink_.push_back(expr_lvalue);
                return expr_lvalue;
        }

        inline Expr *ExprHandler::make_expr_const_string(const char *str_value, Location str_location)
        {
                Expr *expr_str = new Expr{
                    .type = Expr::Type::CONST_STRING,
                    .location = str_location,
                    .const_str = Utils::cstrdup(str_value),
                    .next = nullptr,
                };
                expr_sink_.push_back(expr_str);
                return expr_str;
        }

        inline Expr *ExprHandler::make_expr_const_int(decltype(Expr::const_int) int_value, Location int_location)
        {
                Expr *expr_num = new Expr{
                    .type = Expr::Type::CONST_INT,
                    .location = int_location,
                    .const_int = int_value,
                    .next = nullptr,
                };
                expr_sink_.push_back(expr_num);
                return expr_num;
        }

        inline Expr *ExprHandler::make_expr_const_real(decltype(Expr::const_real) real_value, Location real_location)
        {
                Expr *expr_num = new Expr{
                    .type = Expr::Type::CONST_REAL,
                    .location = real_location,
                    .const_real = real_value,
                    .next = nullptr,
                };
                expr_sink_.push_back(expr_num);
                return expr_num;
        }

        inline Expr *ExprHandler::make_expr_const_bool(bool bool_value, Location bool_location)
        {
                Expr *expr_bool = new Expr{
                    .type = Expr::Type::CONST_BOOLEAN,
                    .location = bool_location,
                    .const_bool = bool_value,
                    .next = nullptr,
                };
                expr_sink_.push_back(expr_bool);
                return expr_bool;
        }

        inline Expr *ExprHandler::make_expr_const_nil(Location nil_location)
        {
                Expr *expr_nil = new Expr{
                    .type = Expr::Type::CONST_NIL,
                    .location = nil_location,
                    .index = nullptr,
                    .next = nullptr,
                };
                expr_sink_.push_back(expr_nil);
                return expr_nil;
        }

        inline Expr *ExprHandler::make_expr_table_item(
            SymbolTable &st,
            Expr *&lvalue,
            const char *id,
            Location id_location)
        {
                DEBUG_SMART_ASSERT(!!lvalue, !!id);
                lvalue = emit_quad_if_table_item(st, lvalue);

                Expr *expr_table_item = new Expr{
                    .type = Expr::Type::TABLE_ITEM,
                    .symbol = lvalue->symbol,
                    .index = make_expr_const_string(id, id_location),
                    .next = nullptr,
                };
                expr_sink_.push_back(expr_table_item);
                return expr_table_item;
        }

        inline Expr *ExprHandler::make_expr_table_item(
            SymbolTable &st,
            Expr *&lvalue,
            Expr *expr)
        {
                DEBUG_SMART_ASSERT(!!lvalue, !!expr);
                lvalue = emit_quad_if_table_item(st, lvalue);

                Expr *expr_table_item = new Expr{
                    .type = Expr::Type::TABLE_ITEM,
                    .symbol = lvalue->symbol,
                    .index = expr,
                    .next = nullptr,
                };

                expr_sink_.push_back(expr_table_item);
                return expr_table_item;
        }

        inline Expr *ExprHandler::make_expr_assign(const Symbol *symbol)
        {
                DEBUG_SMART_ASSERT(!!symbol);
                Expr *expr_assign = new Expr{
                    .type = Expr::Type::ASSIGN,
                    .symbol = symbol,
                    .index = nullptr,
                    .next = nullptr,
                };
                expr_sink_.push_back(expr_assign);
                return expr_assign;
        }

        // TODO: I dont like we make assign but we say rvalue... wtf.. UNDERSTAND IT BETTER
        // TODO 2: DONT MAKE THE SAME NAME make_expr functions (AKA DONT OVERLOAD THEM...)!!! (VERY BAD DESIGN (BOMB WAITING TO EXPLODE!!!!))
        inline Expr *ExprHandler::make_expr_assign(Expr *rvalue)
        {
                DEBUG_SMART_ASSERT(!!rvalue);
                Expr *expr_assign = new Expr{
                    .type = Expr::Type::ASSIGN,
                    .symbol = rvalue->symbol,
                    .index = rvalue->index,
                    .next = rvalue->next,
                };
                expr_sink_.push_back(expr_assign);
                return expr_assign;
        }

        inline Expr *ExprHandler::emit_quad_if_table_item(SymbolTable &st, Expr *expr)
        {
                DEBUG_SMART_ASSERT(!!expr);
                if (expr->type != Expr::Type::TABLE_ITEM)
                        return expr;

                Expr *expr_temp_var = make_expr_lvalue(parse_ctx_->new_temp(st));

                parse_ctx_->quad_handler.emit_quad(
                    IOPCode::TABLEGETELEM,
                    expr,
                    expr->index,
                    expr_temp_var,
                    k_no_location //
                );

                return expr_temp_var;
        }

        inline ExprHandler::ExprHandler(ParseCtx *parse_ctx) : parse_ctx_(parse_ctx) {}

        inline ExprHandler::~ExprHandler() noexcept
        {
                for (const Expr *e : expr_sink_)
                        delete e;
        }

        inline ParseCtx::ParseCtx()
            : function_ctx_handler(this),
              expr_handler(this) {}

        inline const Symbol *ParseCtx::new_temp(SymbolTable &st)
        {
                const std::string temp_name = name_generator.new_temp_name();
                const Symbol *symbol = st.lookup_local(temp_name, scope_handler.scope());

                // We register new temp, only if current scope doesnt have that temp.
                if (!symbol)
                        symbol = st.insert_variable(
                            temp_name,
                            scope_handler.scope(),
                            space_handler.space(),
                            space_handler.next_offset(),
                            k_no_location);

                return symbol;
        }
} // namespace Alpha
#endif // ALPHA_PARSER_CONTEXT_HPP