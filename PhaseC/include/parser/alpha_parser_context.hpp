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

namespace Alpha
{
        // Classes define here:
        struct FunctionDataFrame;
        class SpaceHandler;
        class ScopeHandler;
        class FunctionCtxHandler;
        class TempGenerator;
        class ParseCtx;

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
                Variable::Space space() const noexcept;

                void offset_inc() noexcept;
                u32 offset() const noexcept;

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
                u32 scope() const noexcept { return scope_; }

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
                        const u32 locals;
                        const bool backpatch_locals;
                };

                std::string new_function_id;
                Location new_function_location = k_no_location;
                bool new_function_is_anonymous = false;

                FunctionCtxHandler(ParseCtx *parse_ctx);
                ~FunctionCtxHandler();

                void enter_function(bool backpatch_locals);
                FunctionBackpatchInfo exit_function() noexcept;

                u32 function_nesting_depth() const noexcept;
                u32 function_scope() const noexcept;
                const std::string &function_name() const noexcept;
                Location function_location() const noexcept;

                void enter_loop() noexcept;
                void exit_loop() noexcept;
                u32 loop_depth() const noexcept;

                void add_function_parameter(const std::string &name, Location location);
                const std::list<Parameter> &function_parameters() const noexcept;
                std::list<Parameter> extract_function_parameters();

                DEBUG_ALWAYS_INLINE u32 anonymous_counter() { return anonymous_counter_; }
                DEBUG_ALWAYS_INLINE u32 function_counter() { return function_counter_; }

                void add_local();

        private:
                struct FunctionDataFrame
                {
                        const std::string name;
                        const u32 scope;
                        const Location location;
                        const bool backpatch_locals; // If not valid back-patching local_var count is skipped.

                        u32 loop_nesting_counter = 0;
                        u32 local_variables_counter = 0;
                };
                std::stack<FunctionDataFrame> frame_stack_;
                std::list<Parameter> function_parameters_;
                u32 anonymous_counter_ = 0;
                u32 function_counter_ = 0;

                ParseCtx *const parse_ctx_;
        };

        class TempGenerator : private Immobile
        {
        public:
                [[nodiscard]] std::string new_temp();
                void reset() { temp_counter_ = 0; }

        private:
                u32 temp_counter_ = 0;
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

                const std::vector<Quad> &emmited_quads() { return emitted_quads_; }

        private:
                std::vector<Quad> emitted_quads_;
                u32 next_quad_label_ = 1; // First quad_label is always 1, (0 for backpatching)
        };

        class ParseCtx : private Immobile
        {
        public:
                SpaceHandler space_handler;
                ScopeHandler scope_handler;
                FunctionCtxHandler function_ctx_handler;
                QuadHandler quad_handler;
                TempGenerator temp_generator;

                ParseCtx() = default;
                ~ParseCtx() = default;

                void register_temp(SymbolTable &st);
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

        inline void SpaceHandler::offset_inc() noexcept
        {
                DEBUG_SMART_ASSERT(variable_offset_stack_.size() > 0);
                ++variable_offset_stack_.top();
        }

        inline u32 SpaceHandler::offset() const noexcept
        {
                DEBUG_SMART_ASSERT(variable_offset_stack_.size() > 0);
                return variable_offset_stack_.top();
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
                SMART_ASSERT(parse_ctx_ != nullptr);
                // We push a stackframe, for loops that might occur outside
                // functions. So every frame corresponds to a function except the
                // first.
                frame_stack_.emplace(FunctionDataFrame{
                    .name = k_global_data_frame_name,
                    .scope = k_global_scope,
                    .location = k_no_location,
                    .backpatch_locals = false // There is no function to backpatch its local var counter.
                });
        }

        inline FunctionCtxHandler::~FunctionCtxHandler()
        {
                DEBUG_SMART_ASSERT(                                   //
                    frame_stack_.size() == k_global_data_frame_count, //
                    function_parameters_.size() == 0                  // All parameters must be used.
                );
        }

        inline void FunctionCtxHandler::enter_function(bool backpatch_locals)
        {
                DEBUG_SMART_ASSERT(frame_stack_.size() < k_max_function_nesting);

                frame_stack_.emplace(FunctionDataFrame{
                    .name = new_function_id,
                    .scope = parse_ctx_->scope_handler.scope(),
                    .location = new_function_location,
                    .backpatch_locals = backpatch_locals //
                });

                parse_ctx_->scope_handler.enter_scope();
                parse_ctx_->scope_handler.skip_next_scope_increment();
                anonymous_counter_ += new_function_is_anonymous;
                function_counter_ += 1 - new_function_is_anonymous;
        }

        // Returns the number of locals of the current function.
        inline FunctionCtxHandler::FunctionBackpatchInfo
        FunctionCtxHandler::exit_function() noexcept
        {
                // A frame always exist for loops outside functions.
                DEBUG_SMART_ASSERT(frame_stack_.size() > k_global_data_frame_count);
                // All loops must be closed before exiting function.
                DEBUG_SMART_ASSERT(frame_stack_.top().loop_nesting_counter == 0);

                FunctionDataFrame top_frame = std::move(frame_stack_.top());
                frame_stack_.pop();
                return {
                    .name = top_frame.name,
                    .scope = top_frame.scope,
                    .locals = top_frame.local_variables_counter,
                    .backpatch_locals = top_frame.backpatch_locals //
                };
        }

        inline u32 FunctionCtxHandler::function_nesting_depth() const noexcept
        {
                return frame_stack_.size() - k_global_data_frame_count;
        }

        inline u32 FunctionCtxHandler::function_scope() const noexcept
        {
                DEBUG_SMART_ASSERT(frame_stack_.size() > 0);
                return frame_stack_.top().scope;
        }

        inline const std::string &FunctionCtxHandler::function_name() const noexcept
        {
                DEBUG_SMART_ASSERT(frame_stack_.size() > 0);
                return frame_stack_.top().name;
        }

        inline Location FunctionCtxHandler::function_location() const noexcept
        {
                DEBUG_SMART_ASSERT(frame_stack_.size() > 0);
                return frame_stack_.top().location;
        }

        inline void FunctionCtxHandler::enter_loop() noexcept
        {
                DEBUG_SMART_ASSERT(frame_stack_.size() > 0);
                DEBUG_SMART_ASSERT(frame_stack_.top().loop_nesting_counter < k_max_loop_nesting);
                ++frame_stack_.top().loop_nesting_counter;
        }

        inline void FunctionCtxHandler::exit_loop() noexcept
        {
                DEBUG_SMART_ASSERT(frame_stack_.size() > 0);
                DEBUG_SMART_ASSERT(frame_stack_.top().loop_nesting_counter > 0);
                --frame_stack_.top().loop_nesting_counter;
        }

        inline u32 FunctionCtxHandler::loop_depth() const noexcept
        {
                DEBUG_SMART_ASSERT(frame_stack_.size() > 0);
                return frame_stack_.top().loop_nesting_counter;
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

        inline std::list<Parameter> FunctionCtxHandler::extract_function_parameters()
        {
#ifdef OPTIMIZED_MODE
                return std::move(function_parameters_);
#else
                std::list<Parameter> out(std::move(function_parameters_));
                function_parameters_.clear();
                return out; // NVRO
#endif
        }

        inline void FunctionCtxHandler::add_local()
        {
                ++frame_stack_.top().local_variables_counter;
        }

        inline std::string TempGenerator::new_temp()
        {
                return std::string(k_temp_variable_prefix) + std::to_string(temp_counter_++);
        }

        inline void ParseCtx::register_temp(SymbolTable &st)
        {
                const std::string temp_name = temp_generator.new_temp();
                const Symbol *found_symbol = st.lookup_local(temp_name, scope_handler.scope());

                // We register new temp, only if current scope doesnt have that temp.
                if (found_symbol)
                        return;

                st.insert_variable(
                    temp_name,
                    scope_handler.scope(),
                    space_handler.space(),
                    space_handler.offset(),
                    k_no_location);
        }

        inline void QuadHandler::emit_quad(
            const IOPCode iopcode,
            const Expr *arg1,
            const Expr *arg2,
            const Expr *result,
            const Location location)
        {
                DEBUG_SMART_ASSERT(emitted_quads_.size() + 1 == next_quad_label_);

                emitted_quads_.emplace_back(Quad{.iopcode = iopcode,
                                                 .arg1 = arg1,
                                                 .arg2 = arg2,
                                                 .result = result,
                                                 .label = next_quad_label_++,
                                                 .location = location});
        }
} // namespace Alpha
#endif // ALPHA_PARSER_CONTEXT_HPP
