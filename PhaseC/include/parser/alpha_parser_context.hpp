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
#include "parser/alpha_symbol_table.hpp"
#include "core/alpha_konstants.hpp"
#include "utils/misc.hpp"
#include "core/alpha_types.hpp"
#include "utils/smart_assert.h"
#include "core/alpha_error.hpp"
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

                struct methodCallIdState
                {
                        std::string id;
                        Location id_location;
                        Location method_call_location;
                } method_call_id;

                struct ifPrefixState
                {
                        std::stack<u32> quads_to_patch;
                } if_prefix;

                struct elsePrefixState
                {
                        std::stack<u32> quads_to_patch;
                } else_prefix;

                struct orHookState
                {
                        std::stack<u32> next_quad_stack;
                } or_hook;

                struct andHookState
                {
                        std::stack<u32> next_quad_stack;
                } and_hook;
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

                FunctionCtxHandler(ParseCtx &parse_ctx);
                ~FunctionCtxHandler();

                void enter_loop() noexcept;
                void exit_loop() noexcept;
                void add_function_parameter(const std::string &name, Location loc);
                void clear_function_parameters() noexcept;
                void add_local() noexcept;
                void enter_function(const Function *function_symbol);
                [[nodiscard]] FunctionBackpatchInfo exit_function() noexcept;
                [[nodiscard]] u32 function_nesting_depth() const noexcept;
                [[nodiscard]] u32 current_function_scope() const noexcept;
                [[nodiscard]] const std::string &current_function_name() const noexcept;
                [[nodiscard]] Location current_function_location() const noexcept;
                [[nodiscard]] u32 loop_depth() const noexcept;
                [[nodiscard]] const std::list<Parameter> &function_parameters() const noexcept;
                [[nodiscard]] u32 next_function_address() noexcept { return next_function_address_++; }

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

                ParseCtx &parse_ctx_;
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
                    IOPCode iopc,
                    const Expr *arg1,
                    const Expr *arg2,
                    const Expr *result,
                    Location loc);
                void emit_quad_w_jump_step(
                    IOPCode iopc,
                    const Expr *arg1,
                    const Expr *arg2,
                    u32 jump_step,
                    Location loc);
                void emit_quad_labelless(
                    IOPCode iopc,
                    const Expr *arg1,
                    const Expr *arg2,
                    const Expr *result,
                    Location loc);
                void patch_quad(u32 target_quad_label, u32 destination_label);
                void patch_bool_list(const std::vector<u32> &patch_list, u32 destination_label);
                [[nodiscard]] const std::vector<Quad> &quads() const { return quads_; }
                [[nodiscard]] u32 next_quad_label() const { return next_quad_label_; }

        private:
                [[nodiscard]] static bool requires_label(IOPCode iopc) noexcept;
                void emit_quad_impl(
                    IOPCode iopc,
                    const Expr *arg1,
                    const Expr *arg2,
                    const Expr *result,
                    u32 label,
                    Location loc);
                std::vector<Quad> quads_;
                u32 next_quad_label_ = 1; // First quad_label is always 1, (0 for backpatching)
        };

        class ExprHandler : private Immobile
        {
        public:
                ExprHandler(ParseCtx &parse_ctx);
                ~ExprHandler() noexcept;

                [[nodiscard]] Expr *emit_quad_if_table_item(Expr *lvalue);
                [[nodiscard]] Expr *make_expr_variable(const Symbol *symbol, Location var_loc);
                [[nodiscard]] Expr *make_expr_const_string(const char *str_value, Location str_loc);
                [[nodiscard]] Expr *make_expr_const_real(f64 real_value, Location real_loc);
                [[nodiscard]] Expr *make_expr_const_int(i64 int_value, Location int_loc);
                [[nodiscard]] Expr *make_expr_const_bool(bool bool_value, Location bool_loc);
                [[nodiscard]] Expr *make_expr_const_nil(Location nil_loc);
                [[nodiscard]] Expr *make_expr_program_function(const Function *function_symbol);
                [[nodiscard]] Expr *make_expr_assign(Expr *rvalue, Location assign_loc);         // TODO: !! Why two make assign expr?
                [[nodiscard]] Expr *make_expr_assign(const Symbol *symbol, Location assign_loc); // TODO: WHy 2? make_assign_expr?
                [[nodiscard]] Expr *make_expr_new_table(Location new_table_loc);
                [[nodiscard]] Expr *make_expr_arithmetic(Location arithmetic_loc);
                [[nodiscard]] Expr *make_expr_boolean(Location bool_expr_loc);
                [[nodiscard]] Expr *make_expr_table_item(
                    Expr *&lvalue,
                    const std::string &id,
                    Location id_loc,
                    Location table_item_loc);
                [[nodiscard]] Expr *make_expr_table_item(
                    Expr *&lvalue,
                    Expr *expr,
                    Location table_tem_Loc);

        private:
                std::vector<const Expr *> expr_sink_;
                ParseCtx &parse_ctx_;
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

                ParseCtx(SymbolTable &st, ErrorTracker &et);
                ~ParseCtx() = default;

                [[nodiscard]] const Symbol *new_temp();

        private:
                SymbolTable &st_;
                [[maybe_unused]] ErrorTracker &et_; // TODO: remove if unused
        };

        inline SpaceHandler::SpaceHandler()
        {
                enter_space(); // We push the first scope space frame (PROGRAM_VAR)
        };

        inline SpaceHandler::~SpaceHandler()
        {
                std::cerr << "VARIABLE OFFSET_STACK_SIZE = " << variable_offset_stack_.size() << std::endl;
                DEBUG_SMART_ASSERT(variable_offset_stack_.size() == 1);
        }

        inline void
        SpaceHandler::enter_space()
        {
                variable_offset_stack_.push(k_initial_variable_offset);
        }

        inline void
        SpaceHandler::exit_space()
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

        inline Variable::Space
        SpaceHandler::space() const noexcept
        {
                DEBUG_SMART_ASSERT(variable_offset_stack_.size() > 0);      // A stack frame must always exist
                const auto frame_index = variable_offset_stack_.size() - 1; // -1 for size to index

                if (frame_index == k_initial_space)
                        return Variable::Space::PROGRAM_VAR;
                if (is_odd(frame_index))
                        return Variable::Space::FORMAL_ARGUMENT;
                return Variable::Space::FUNCTION_LOCAL;
        }

        inline u32
        SpaceHandler::next_offset() noexcept
        {
                DEBUG_SMART_ASSERT(variable_offset_stack_.size() > 0);
                return variable_offset_stack_.top()++;
        }

        inline ScopeHandler::ScopeHandler() : scope_(k_global_scope)
        {
                // A ToggleSwitch must always be initialized as disabled.
                SMART_ASSERT(skip_next_scope_increment_.is_disabled());
        }

        inline void
        ScopeHandler::skip_next_scope_increment() noexcept
        {
                skip_next_scope_increment_.enable();
        }

        inline void
        ScopeHandler::enter_scope() noexcept
        {
                if (skip_next_scope_increment_.is_enabled())
                {
                        skip_next_scope_increment_.disable();
                        return;
                }
                DEBUG_SMART_ASSERT(scope_ < k_max_scope);
                ++scope_;
        }

        inline void
        ScopeHandler::exit_scope() noexcept
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

        inline FunctionCtxHandler::FunctionCtxHandler(ParseCtx &parse_ctx)
            : parse_ctx_(parse_ctx)
        {
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

        inline void
        FunctionCtxHandler::enter_function(const Function *function_symbol)
        {
#ifdef DEBUG_MODE
                DEBUG_SMART_ASSERT(frame_stack_.size() < k_max_function_nesting);
                if (!!function_symbol)
                        DEBUG_SMART_ASSERT(
                            function_symbol->name == parse_ctx_.cache.func_prefix.id,
                            function_symbol->scope == parse_ctx_.scope_handler.scope(),
                            function_symbol->location == parse_ctx_.cache.func_prefix.location,
                            function_symbol->is_function(),
                            function_symbol->type == Symbol::Type::PROGRAM_FUNCTION
                            // Only library functions are defined in source code.
                        );
#endif // DEBUG_MODE

                frame_stack_.emplace(FunctionDataFrame{
                    .name = parse_ctx_.cache.func_prefix.id,
                    .scope = parse_ctx_.scope_handler.scope(),
                    .location = parse_ctx_.cache.func_prefix.location,
                    .function_symbol = function_symbol //
                });

                // Function scope is entered here.
                // We skip the next `{` block’s scope to avoid double scoping.
                parse_ctx_.scope_handler.enter_scope();
                parse_ctx_.scope_handler.skip_next_scope_increment();
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

        inline u32
        FunctionCtxHandler::function_nesting_depth() const noexcept
        {
                return frame_stack_.size() - k_global_data_frame_count;
        }

        inline u32
        FunctionCtxHandler::current_function_scope() const noexcept
        {
                DEBUG_SMART_ASSERT(frame_stack_.size() > 0);
                return frame_stack_.top().scope;
        }

        inline const std::string &
        FunctionCtxHandler::current_function_name() const noexcept
        {
                DEBUG_SMART_ASSERT(frame_stack_.size() > 0);
                return frame_stack_.top().name;
        }

        inline Location
        FunctionCtxHandler::current_function_location() const noexcept
        {
                DEBUG_SMART_ASSERT(frame_stack_.size() > 0);
                return frame_stack_.top().location;
        }

        inline void
        FunctionCtxHandler::enter_loop() noexcept
        {
                DEBUG_SMART_ASSERT(frame_stack_.size() > 0);
                DEBUG_SMART_ASSERT(frame_stack_.top().loop_nesting_count < k_max_loop_nesting);
                ++frame_stack_.top().loop_nesting_count;
        }

        inline void
        FunctionCtxHandler::exit_loop() noexcept
        {
                DEBUG_SMART_ASSERT(frame_stack_.size() > 0);
                DEBUG_SMART_ASSERT(frame_stack_.top().loop_nesting_count > 0);
                --frame_stack_.top().loop_nesting_count;
        }

        inline u32
        FunctionCtxHandler::loop_depth() const noexcept
        {
                DEBUG_SMART_ASSERT(frame_stack_.size() > 0);
                return frame_stack_.top().loop_nesting_count;
        }

        inline void
        FunctionCtxHandler::add_function_parameter(const std::string &name, Location loc)
        {
                function_parameters_.emplace_back(name, loc);
        }

        inline const std::list<Parameter> &
        FunctionCtxHandler::function_parameters() const noexcept
        {
                return function_parameters_;
        }

        inline void
        FunctionCtxHandler::clear_function_parameters() noexcept
        {
                function_parameters_.clear();
        }

        inline void
        FunctionCtxHandler::add_local() noexcept
        {
                ++frame_stack_.top().local_variable_count;
        }

        inline std::string
        NameGenerator::new_temp_name()
        {
                return k_temp_variable_prefix + std::to_string(temp_name_counter_++);
        }
        inline std::string
        NameGenerator::new_anonymous()
        {
                return k_private_anonymous_prefix + std::to_string(anonymous_counter_++);
        }

        inline void
        QuadHandler::emit_quad(
            const IOPCode iopc,
            const Expr *arg1,
            const Expr *arg2,
            const Expr *result,
            const Location loc)
        {
                // TODO. IF only used by non required-IOPCs, emit requires_label check and just put k_no_label.
                emit_quad_impl(iopc, arg1, arg2, result, requires_label(iopc) ? next_quad_label_ : k_no_label, loc);
        }

        inline void
        QuadHandler::emit_quad_w_jump_step(
            const IOPCode iopc,
            const Expr *arg1,
            const Expr *arg2,
            const u32 jump_step,
            const Location loc)
        {
                DEBUG_SMART_ASSERT(requires_label(iopc)); // This emit_quad overload is used for JUMP IOPCs
                emit_quad_impl(iopc, arg1, arg2, nullptr, next_quad_label_ + jump_step, loc);
        }

        inline void
        QuadHandler::emit_quad_labelless(
            IOPCode iopc,
            const Expr *arg1,
            const Expr *arg2,
            const Expr *result,
            Location loc)
        {
                emit_quad_impl(iopc, arg1, arg2, result, k_no_label, loc);
        }

        inline void
        QuadHandler::patch_quad(u32 target_quad_label, u32 destination_label)
        {
                const u32 quad_index = target_quad_label - 1; // First quad at index 0, has quad with label 1.
                DEBUG_SMART_ASSERT(
                    target_quad_label > 0,
                    quad_index < quads_.size(),
                    quads_[quad_index].label == k_no_label,
                    destination_label != k_no_label //
                );
                quads_[quad_index].label = destination_label;
        }

        inline void
        QuadHandler::patch_bool_list(const std::vector<u32> &patch_list, u32 destination_label)
        {
                DEBUG_SMART_ASSERT(destination_label != k_no_label);
                for (u32 target_quad_label : patch_list)
                        patch_quad(target_quad_label, destination_label);
        }

        inline void
        QuadHandler::emit_quad_impl(
            IOPCode iopc,
            const Expr *arg1,
            const Expr *arg2,
            const Expr *result,
            u32 label,
            Location loc)
        {
                DEBUG_SMART_ASSERT(quads_.size() + 1 == next_quad_label_);
                // TODO. IF only used by non required-IOPCs, emit requires_label check and just put 0.
                quads_.emplace_back(Quad{
                    .iopcode = iopc,
                    .arg1 = arg1,
                    .arg2 = arg2,
                    .result = result,
                    .label = label,
                    .location = loc,
                });

                ++next_quad_label_;
        }

        inline bool
        QuadHandler::requires_label(IOPCode iopc) noexcept
        {
                // clang-format off
                switch (iopc)
                {
                #define X(iopcode) case Alpha::IOPCode::iopcode: return true;
                        IOPCODES_WITH_LABEL
                #undef  X
                #define X(iopcode) case Alpha::IOPCode::iopcode: return false;
                        IOPCODES_WITHOUT_LABEL
                #undef  X
                default: [[unlikely]] SMART_ASSERT(false);
                }
                // clang-format on
        }

        inline Expr *
        ExprHandler::make_expr_variable(const Symbol *const symbol, Location var_loc)
        {
                DEBUG_SMART_ASSERT(!!symbol);

                Expr *expr_lvalue = new Expr{
                    .type = Expr::Type::VARIABLE,
                    .symbol = symbol,
                    .location = var_loc,
                    .index = nullptr,
                };
                expr_sink_.push_back(expr_lvalue);
                return expr_lvalue;
        }

        inline Expr *
        ExprHandler::make_expr_const_string(const char *str_value, Location str_loc)
        {
                DEBUG_SMART_ASSERT(str_loc != k_no_location);

                Expr *expr_str = new Expr{
                    .type = Expr::Type::CONST_STRING,
                    .symbol = nullptr,
                    .location = str_loc,
                    .const_str = Utils::cstrdup(str_value),
                };
                expr_sink_.push_back(expr_str);
                return expr_str;
        }

        inline Expr *
        ExprHandler::make_expr_const_int(i64 int_value, Location int_loc)
        {
                DEBUG_SMART_ASSERT(int_loc != k_no_location);

                Expr *expr_num = new Expr{
                    .type = Expr::Type::CONST_INT,
                    .symbol = nullptr,
                    .location = int_loc,
                    .const_int = int_value,
                };
                expr_sink_.push_back(expr_num);
                return expr_num;
        }

        inline Expr *
        ExprHandler::make_expr_const_real(f64 real_value, Location real_loc)
        {
                DEBUG_SMART_ASSERT(real_loc != k_no_location);

                Expr *expr_num = new Expr{
                    .type = Expr::Type::CONST_REAL,
                    .symbol = nullptr,
                    .location = real_loc,
                    .const_real = real_value,
                };
                expr_sink_.push_back(expr_num);
                return expr_num;
        }

        inline Expr *
        ExprHandler::make_expr_const_bool(bool bool_value, Location bool_loc)
        {
                DEBUG_SMART_ASSERT(bool_loc != k_no_location);

                Expr *expr_bool = new Expr{
                    .type = Expr::Type::CONST_BOOL,
                    .symbol = nullptr,
                    .location = bool_loc,
                    .const_bool = bool_value,
                };
                expr_sink_.push_back(expr_bool);
                return expr_bool;
        }

        inline Expr *
        ExprHandler::make_expr_const_nil(Location nil_loc)
        {
                DEBUG_SMART_ASSERT(nil_loc != k_no_location);

                Expr *expr_nil = new Expr{
                    .type = Expr::Type::CONST_NIL,
                    .symbol = nullptr,
                    .location = nil_loc,
                    .index = nullptr,
                };
                expr_sink_.push_back(expr_nil);
                return expr_nil;
        }

        inline Expr *
        ExprHandler::make_expr_program_function(const Function *function_symbol)
        {
                DEBUG_SMART_ASSERT(!!function_symbol);

                Expr *expr_progfunc = new Expr{
                    .type = Expr::Type::PROGRAM_FUNCTION,
                    .symbol = function_symbol,
                    .location = function_symbol->location,
                    .index = nullptr,
                };
                expr_sink_.push_back(expr_progfunc);
                return expr_progfunc;
        }

        inline Expr *
        ExprHandler::make_expr_table_item(
            Expr *&lvalue,
            const std::string &id,
            Location id_loc,
            Location table_item_loc)
        {
                DEBUG_SMART_ASSERT(!!lvalue);
                lvalue = emit_quad_if_table_item(lvalue);

                Expr *expr_table_item = new Expr{
                    .type = Expr::Type::TABLE_ITEM,
                    .symbol = lvalue->symbol,
                    .location = table_item_loc,
                    .index = make_expr_const_string(id.c_str(), id_loc),
                };
                expr_sink_.push_back(expr_table_item);
                return expr_table_item;
        }

        inline Expr *
        ExprHandler::make_expr_table_item(Expr *&lvalue, Expr *expr, const Location table_item_loc)
        {
                DEBUG_SMART_ASSERT(!!lvalue, !!expr);
                lvalue = emit_quad_if_table_item(lvalue);

                Expr *expr_table_item = new Expr{
                    .type = Expr::Type::TABLE_ITEM,
                    .symbol = lvalue->symbol,
                    .location = table_item_loc,
                    .index = expr,
                };
                expr_sink_.push_back(expr_table_item);
                return expr_table_item;
        }

        inline Expr *
        ExprHandler::make_expr_new_table(const Location new_table_loc)
        {
                Expr *expr_new_table = new Expr{
                    .type = Expr::Type::NEW_TABLE,
                    .symbol = parse_ctx_.new_temp(),
                    .location = new_table_loc,
                    .index = nullptr,
                };
                expr_sink_.push_back(expr_new_table);
                return expr_new_table;
        }

        inline Expr *
        ExprHandler::make_expr_arithmetic(Location arithmetic_loc)
        {
                Expr *expr_arithmetic = new Expr{
                    .type = Expr::Type::ARITHMETIC_EXPR,
                    .symbol = parse_ctx_.new_temp(),
                    .location = arithmetic_loc,
                    .index = nullptr,
                };
                expr_sink_.push_back(expr_arithmetic);
                return expr_arithmetic;
        }

        inline Expr *
        ExprHandler::make_expr_boolean(Location bool_expr_loc)
        {
                Expr *bool_expr = new Expr{
                    .type = Expr::Type::BOOLEAN_EXPR,
                    .symbol = parse_ctx_.new_temp(),
                    .location = bool_expr_loc,
                    .index = nullptr,
                };
                expr_sink_.push_back(bool_expr);
                return bool_expr;
        }

        inline Expr *
        ExprHandler::make_expr_assign(const Symbol *symbol, Location assign_loc)
        {
                DEBUG_SMART_ASSERT(!!symbol);
                Expr *expr_assign = new Expr{
                    .type = Expr::Type::ASSIGN_EXPR,
                    .symbol = symbol,
                    .location = assign_loc,
                    .index = nullptr,
                };
                expr_sink_.push_back(expr_assign);
                return expr_assign;
        }

        // TODO: I dont like we make assign but we say rvalue... wtf.. UNDERSTAND IT BETTER
        // TODO 2: DONT MAKE THE SAME NAME make_expr functions (AKA DONT OVERLOAD THEM...)!!! (VERY BAD DESIGN (BOMB WAITING TO EXPLODE!!!!))
        inline Expr *
        ExprHandler::make_expr_assign(Expr *rvalue, Location assign_loc)
        {
                DEBUG_SMART_ASSERT(!!rvalue);
                Expr *expr_assign = new Expr{
                    .type = Expr::Type::ASSIGN_EXPR,
                    .symbol = rvalue->symbol,
                    .location = assign_loc,
                    .index = rvalue->index,
                };
                expr_sink_.push_back(expr_assign);
                return expr_assign;
        }

        inline Expr *
        ExprHandler::emit_quad_if_table_item(Expr *expr)
        {
                DEBUG_SMART_ASSERT(!!expr);
                if (expr->type != Expr::Type::TABLE_ITEM)
                        return expr;

                Expr *expr_temp_var = make_expr_variable(parse_ctx_.new_temp(), k_no_location);

                parse_ctx_.quad_handler.emit_quad(
                    IOPCode::TABLEGETELEM,
                    expr,
                    expr->index,
                    expr_temp_var,
                    k_no_location //

                    //  expr_location_founder(expr) // TODO: REMOVE (if you dont want location here)
                );

                return expr_temp_var;
        }

        inline ExprHandler::ExprHandler(ParseCtx &parse_ctx) : parse_ctx_(parse_ctx) {}

        inline ExprHandler::~ExprHandler() noexcept
        {
                for (const Expr *e : expr_sink_)
                {
                        if (e->type == Expr::Type::CONST_STRING)
                                delete[] e->const_str;
                        delete e;
                }
        }

        inline ParseCtx::ParseCtx(SymbolTable &st, ErrorTracker &et)
            : function_ctx_handler(*this),
              expr_handler(*this),
              st_(st),
              et_(et) {}

        inline const Symbol *
        ParseCtx::new_temp()
        {
                const std::string temp_name = name_generator.new_temp_name();
                const Symbol *symbol = st_.lookup_local(temp_name, scope_handler.scope());

                // We register new temp, only if current scope doesnt have that temp.
                if (!symbol)
                {
                        Variable::Type var_type =
                            scope_handler.scope() == k_global_scope
                                ? Variable::Type::GLOBAL_VARIABLE
                                : Variable::Type::LOCAL_VARIABLE;

                        symbol = st_.insert_variable(
                            temp_name,
                            scope_handler.scope(),
                            var_type,
                            space_handler.space(),
                            space_handler.next_offset(),
                            k_no_location //
                        );                // TODO: Remove locations from temps and auto generated
                }
                // variables and values.. known the line a temp was generated is useless...
                return symbol;
        }
} // namespace Alpha
#endif // ALPHA_PARSER_CONTEXT_HPP