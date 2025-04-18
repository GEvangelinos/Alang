#ifndef ALPHA_PARSER_CONTEXT_HPP
#define ALPHA_PARSER_CONTEXT_HPP

#include <stack>
#include <limits>
#include <list>
#include "core/alpha_types.hpp"
#include "misc/smart_assert.h"
#include "core/alpha_types.hpp"
#include "core/alpha_konstants.hpp"
#include "misc/sanity_assert.h"
#include "core/alpha_macros.hpp"
#include "_parser_common.hpp"

namespace Alpha
{
        class ToggleSwitch
        {
        public:
                DEBUG_ALWAYS_INLINE void enable() noexcept
                {
                        SANITY_ASSERT_TRUE(is_disabled());
                        state_ = true;
                }
                DEBUG_ALWAYS_INLINE void disable() noexcept
                {
                        SANITY_ASSERT_TRUE(is_enabled());
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
                FunctionDataFrame(const std::string &name, u32 scope, Location location)
                    : name_(name),
                      scope_(scope),
                      location_(location),
                      loop_counter_(0) {}

                DEBUG_ALWAYS_INLINE const std::string &name() const noexcept { return name_; }
                DEBUG_ALWAYS_INLINE u32 scope() const noexcept { return scope_; }
                DEBUG_ALWAYS_INLINE Location location() const noexcept { return location_; }

                DEBUG_ALWAYS_INLINE u32 loop_counter() const noexcept { return loop_counter_; }
                DEBUG_ALWAYS_INLINE void loop_counter_inc() noexcept { ++loop_counter_; }
                DEBUG_ALWAYS_INLINE void loop_counter_dec() noexcept { --loop_counter_; }

        private:
                const std::string name_;
                const u32 scope_;
                const Location location_;
                u32 loop_counter_;
        };

        class ParseCtx
        {
        public:
                inline ParseCtx();
                inline ~ParseCtx();

                ParseCtx(const ParseCtx &) = delete;
                ParseCtx(ParseCtx &&) = delete;
                ParseCtx &operator=(const ParseCtx &) = delete;
                ParseCtx &operator=(ParseCtx &&) = delete;

                inline void enter_block() noexcept;
                inline void exit_block() noexcept;
                inline u32 current_scope() const noexcept;

                inline void enter_function(const std::string &function_name, Location function_location);
                inline void exit_function() noexcept;
                inline u32 current_function_nesting_depth() const noexcept;
                inline u32 current_function_scope() const noexcept;
                inline const std::string &current_function_name() const noexcept;
                inline Location current_function_location() const noexcept;

                inline void enter_loop() noexcept;
                inline void exit_loop() noexcept;
                inline u32 loop_depth() const noexcept;

                inline void append_function_parameter(const std::string &name, Location location);
                inline std::list<Parameter> retrieve_function_parameters();
                inline std::list<Parameter> extract_function_parameters();
                inline void clear_function_arguments();

        private:
                ToggleSwitch skip_next_scope_increment_;
                u32 current_scope_;
                std::stack<FunctionDataFrame> frame_stack_;
                std::list<Parameter> function_parameters_;
        };

        ParseCtx::ParseCtx()
            : current_scope_(k_global_scope)
        {
                // We push a stackframe, for loops that might occur outside functions.
                // So every frame corresponds to a function except the first.
                frame_stack_.emplace(FunctionDataFrame(k_global_data_frame_name, k_global_scope, {0, 0}));
        }

        ParseCtx::~ParseCtx()
        {
                // Constructor had pushed a stackframe for loops that might occur outside functions.
                // So at the end we expect a single frame to exist.
                SANITY_ASSERT_EQ(frame_stack_.size(), k_data_frames_outside_functions);
        }

        void ParseCtx::enter_block() noexcept
        {
                if (skip_next_scope_increment_.is_enabled())
                {
                        skip_next_scope_increment_.disable();
                        return;
                }
                SANITY_ASSERT_LT(current_scope_, k_max_scope);
                ++current_scope_;
        }

        void ParseCtx::exit_block() noexcept
        {
                // We expect `skip_next_scope_increment` ToggleSwitch to be disabled.
                // That is because if you exit a block, it means you first entered it.
                // So if you exit a block and the `skip_next_scope_increment` switch
                // is enabled, there is a logic error.
                SANITY_ASSERT_TRUE(skip_next_scope_increment_.is_disabled());
                SANITY_ASSERT_GT(current_scope_, 0);
                --current_scope_;
        }

        void ParseCtx::enter_function(const std::string &function_name, Location function_location)
        {
                SANITY_ASSERT_LT(frame_stack_.size(), k_max_function_nesting);
                frame_stack_.emplace(FunctionDataFrame(function_name, current_scope_++, function_location));
                skip_next_scope_increment_.enable();
        }

        void ParseCtx::exit_function() noexcept
        {
                // A frame always exist for loops outside functions.
                SANITY_ASSERT_GT(frame_stack_.size(), k_data_frames_outside_functions);
                // All loops must be closed before exiting function.
                SANITY_ASSERT_EQ(frame_stack_.top().loop_counter(), 0);
                frame_stack_.pop();
        }

        void ParseCtx::enter_loop() noexcept
        {
                SANITY_ASSERT_GT(frame_stack_.size(), 0);
                SANITY_ASSERT_LT(frame_stack_.top().loop_counter(), k_max_loop_nesting);
                frame_stack_.top().loop_counter_inc();
        }

        void ParseCtx::exit_loop() noexcept
        {
                SANITY_ASSERT_GT(frame_stack_.size(), 0);
                SANITY_ASSERT_GT(frame_stack_.top().loop_counter(), 0);
                frame_stack_.top().loop_counter_dec();
        }

        u32 ParseCtx::loop_depth() const noexcept
        {
                SANITY_ASSERT_GT(frame_stack_.size(), 0);
                return frame_stack_.top().loop_counter();
        }

        u32 ParseCtx::current_function_scope() const noexcept
        {
                return frame_stack_.top().scope();
        }

        Location ParseCtx::current_function_location() const noexcept
        {
                return frame_stack_.top().location();
        }

        const std::string &ParseCtx::current_function_name() const noexcept
        {
                return frame_stack_.top().name();
        }

        u32 ParseCtx::current_function_nesting_depth() const noexcept
        {
                return frame_stack_.size() - k_data_frames_outside_functions;
        }

        u32 ParseCtx::current_scope() const noexcept
        {
                return current_scope_;
        }

        void ParseCtx::append_function_parameter(const std::string &name, Location location)
        {
                function_parameters_.emplace_back(name, location);
        }

        std::list<Parameter> ParseCtx::retrieve_function_parameters()
        {
                return function_parameters_;
        }

        std::list<Parameter> ParseCtx::extract_function_parameters()
        {
                return std::move(function_parameters_);
        }

        void ParseCtx::clear_function_arguments()
        {
                function_parameters_.clear();
        }
} // namespace Alpha
#endif // ALPHA_PARSER_CONTEXT_HPP
