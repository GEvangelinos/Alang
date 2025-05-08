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
#include "core/alpha_macros.hpp"
#include "core/alpha_types.hpp"
#include "utils/smart_assert.h"
#include <limits>
#include <list>
#include <stack>
#include <vector>

namespace Alpha
{
        // Classes define here:
        class ToggleSwitch;
        class FunctionDataFrame;
        class ParseCtx;

        class ToggleSwitch
        {
        public:
                DEBUG_ALWAYS_INLINE void enable() noexcept
                {
                        DEBUG_SMART_ASSERT(is_disabled());
                        state_ = true;
                }

                DEBUG_ALWAYS_INLINE void disable() noexcept
                {
                        DEBUG_SMART_ASSERT(is_enabled());
                        state_ = false;
                }

                DEBUG_ALWAYS_INLINE bool is_enabled() const noexcept { return state_; }

                DEBUG_ALWAYS_INLINE bool is_disabled() const noexcept { return !state_; }

        private:
                bool state_ = false; // Initially the switch is off.
        };

        class FunctionDataFrame
        {
        public:
                const std::string name;
                const u32 scope;
                const Location location;

                FunctionDataFrame(const std::string &name, u32 scope, Location location)
                    : name(name), scope(scope), location(location), loop_counter_(0)
                {
                }

                DEBUG_ALWAYS_INLINE u32 loop_counter() const noexcept { return loop_counter_; }
                DEBUG_ALWAYS_INLINE void loop_counter_inc() noexcept { ++loop_counter_; }
                DEBUG_ALWAYS_INLINE void loop_counter_dec() noexcept { --loop_counter_; }

        private:
                u32 loop_counter_;
        };

        class ScopeSpaceHandler
        {
        public:
                static constexpr u32 k_initial_scope_space = 0;
                static_assert(k_initial_scope_space == 0, "Initial space must be 0 to maintain"
                                                          "correct Space mapping logic");
                static constexpr u32 k_initial_variable_offset = 0;

                ScopeSpaceHandler();
                ~ScopeSpaceHandler();

                void enter_scope_space();
                void exit_scope_space();
                Variable::Space space() const noexcept;

                void offset_inc() noexcept;
                u32 offset() const noexcept;

                ScopeSpaceHandler(const ScopeSpaceHandler &) = delete;
                ScopeSpaceHandler(ScopeSpaceHandler &&) = delete;
                ScopeSpaceHandler &operator=(const ScopeSpaceHandler &) = delete;
                ScopeSpaceHandler &operator=(ScopeSpaceHandler &&) = delete;

        private:
                std::stack<u32> variable_offset_stack_;
        };

        class ScopeHandler
        {
        public:
                ScopeHandler();
                ~ScopeHandler() = default;

                void skip_next_scope_increment() noexcept;
                void enter_scope() noexcept;
                void exit_scope() noexcept;
                u32 scope() const noexcept { return scope_; }

                ScopeHandler(const ScopeHandler &) = delete;
                ScopeHandler(ScopeHandler &&) = delete;
                ScopeHandler &operator=(const ScopeHandler &) = delete;
                ScopeHandler &operator=(ScopeHandler &&) = delete;

        private:
                ToggleSwitch skip_next_scope_increment_;
                u32 scope_;
        };

        class FunctionCtxHandler
        {
        public:
                std::string last_function_id;
                Location last_function_location;
                u32 anonymous_counter = 0;

                FunctionCtxHandler();
                ~FunctionCtxHandler();

                void enter_function(const std::string &function_name, Location function_location,
                                    ScopeHandler &scope_handler);
                void exit_function() noexcept;

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

                FunctionCtxHandler(const FunctionCtxHandler &) = delete;
                FunctionCtxHandler(FunctionCtxHandler &&) = delete;
                FunctionCtxHandler &operator=(const FunctionCtxHandler &) = delete;
                FunctionCtxHandler &operator=(FunctionCtxHandler &&) = delete;

        private:
                std::stack<FunctionDataFrame> frame_stack_;
                std::list<Parameter> function_parameters_;
        };

        class ParseCtx
        {
        public:
                ScopeSpaceHandler space_handler;
                ScopeHandler scope_handler;
                FunctionCtxHandler function_ctx_handler;

                ParseCtx() = default;
                ~ParseCtx() = default;

                void emit_quad(IOPCode iopcode, const Expr *arg1, const Expr *arg2,
                               const Expr *result, Location emit_location);

                ParseCtx(const ParseCtx &) = delete;
                ParseCtx(ParseCtx &&) = delete;
                ParseCtx &operator=(const ParseCtx &) = delete;
                ParseCtx &operator=(ParseCtx &&) = delete;

        private:
                std::vector<Quad> emitted_quads_;
                u32 next_quad_label_ = 1; // First quad_label is always 1, (0 for backpatching)
        };

        inline ScopeSpaceHandler::ScopeSpaceHandler()
        {
                enter_scope_space(); // We push the first scope space frame (PROGRAM_VAR)
        };

        inline ScopeSpaceHandler::~ScopeSpaceHandler()
        {
                DEBUG_SMART_ASSERT(variable_offset_stack_.size() == 1);
        }

        inline void ScopeSpaceHandler::enter_scope_space()
        {
                variable_offset_stack_.push(k_initial_variable_offset);
        }

        // clang-format off
inline void ScopeSpaceHandler::exit_scope_space()
{
        constexpr auto scope_spaces_for_closure = 2; // 1 formalArg + 1 functionLocal
        DEBUG_SMART_ASSERT(variable_offset_stack_.size() > scope_spaces_for_closure);
        DEBUG_SMART_ASSERT(is_odd(variable_offset_stack_.size()));

        #pragma unroll
        for (auto i = 0; i < scope_spaces_for_closure; ++i)
                variable_offset_stack_.pop();
}
        // clang-format on

        inline Variable::Space ScopeSpaceHandler::space() const noexcept
        {
                DEBUG_SMART_ASSERT(variable_offset_stack_.size() > 0);      // A stack frame must always exist
                const auto frame_index = variable_offset_stack_.size() - 1; // -1 for size to index

                if (frame_index == k_initial_scope_space)
                        return Variable::Space::PROGRAM_VAR;
                if (is_odd(frame_index))
                        return Variable::Space::FORMAL_ARGUMENT;
                return Variable::Space::FUNCTION_LOCAL;
        }

        inline void ScopeSpaceHandler::offset_inc() noexcept
        {
                DEBUG_SMART_ASSERT(variable_offset_stack_.size() > 0);
                ++variable_offset_stack_.top();
        }

        inline u32 ScopeSpaceHandler::offset() const noexcept
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
                DEBUG_SMART_ASSERT(skip_next_scope_increment_.is_disabled());
                DEBUG_SMART_ASSERT(scope_ > k_global_scope);
                --scope_;
        }

        inline FunctionCtxHandler::FunctionCtxHandler()
        {
                // We push a stackframe, for loops that might occur outside
                // functions. So every frame corresponds to a function except the
                // first.
                frame_stack_.emplace(
                    FunctionDataFrame(k_global_data_frame_name, k_global_scope, k_no_location));
        }

        inline FunctionCtxHandler::~FunctionCtxHandler()
        {
                DEBUG_SMART_ASSERT(frame_stack_.size() == k_global_data_frame_count);
                DEBUG_SMART_ASSERT(function_parameters_.size() == 0); // All parameters must be used.
        }

        inline void FunctionCtxHandler::enter_function(
            const std::string &function_name,
            Location function_location,
            ScopeHandler &scope_handler)
        {
                DEBUG_SMART_ASSERT(frame_stack_.size() < k_max_function_nesting);

                frame_stack_.emplace(
                    FunctionDataFrame(function_name, scope_handler.scope(), function_location));
                scope_handler.enter_scope();
                scope_handler.skip_next_scope_increment();
        }

        inline void FunctionCtxHandler::exit_function() noexcept
        {
                // A frame always exist for loops outside functions.
                DEBUG_SMART_ASSERT(frame_stack_.size() > k_global_data_frame_count);
                // All loops must be closed before exiting function.
                DEBUG_SMART_ASSERT(frame_stack_.top().loop_counter() == 0);
                frame_stack_.pop();
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
                DEBUG_SMART_ASSERT(frame_stack_.top().loop_counter() < k_max_loop_nesting);
                frame_stack_.top().loop_counter_inc();
        }

        inline void FunctionCtxHandler::exit_loop() noexcept
        {
                DEBUG_SMART_ASSERT(frame_stack_.size() > 0);
                DEBUG_SMART_ASSERT(frame_stack_.top().loop_counter() > 0);
                frame_stack_.top().loop_counter_dec();
        }

        inline u32 FunctionCtxHandler::loop_depth() const noexcept
        {
                DEBUG_SMART_ASSERT(frame_stack_.size() > 0);
                return frame_stack_.top().loop_counter();
        }

        inline void FunctionCtxHandler::add_function_parameter(const std::string &name,
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

        inline void ParseCtx::emit_quad(IOPCode iopcode, const Expr *arg1, const Expr *arg2,
                                        const Expr *result, Location emit_location)
        {
                DEBUG_SMART_ASSERT(emitted_quads_.size() + 1 == next_quad_label_);

                emitted_quads_.emplace_back(Quad{.iopcode = iopcode,
                                                 .arg1 = arg1,
                                                 .arg2 = arg2,
                                                 .result = result,
                                                 .label = next_quad_label_++,
                                                 .emit_location = emit_location});
        }
} // namespace Alpha
#endif // ALPHA_PARSER_CONTEXT_HPP
