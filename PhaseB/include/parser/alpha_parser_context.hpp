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
        // struct StateFlags
        // {
        //         bool flag1 : 1;
        //         bool flag2 : 1;
        //         bool flag3 : 1;
        // };

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

        class ParseCtx
        {
        public:
                inline ParseCtx();
                ~ParseCtx() = default;

                ParseCtx(const ParseCtx &) = delete;
                ParseCtx(ParseCtx &&) = delete;
                ParseCtx &operator=(const ParseCtx &) = delete;
                ParseCtx &operator=(ParseCtx &&) = delete;

                inline void enter_block() noexcept;
                inline void exit_block() noexcept;
                inline u32 current_scope() const noexcept;

                inline void enter_function();
                inline void exit_function() noexcept;
                inline u32 function_depth() const noexcept;

                inline void enter_loop() noexcept;
                inline void exit_loop() noexcept;
                inline u32 loop_depth() const noexcept;

                inline void append_function_argument(const std::string &name, CodeLocation location);
                inline std::list<Parameter> extract_function_arguments();

        private:
                ToggleSwitch skip_next_scope_increment_;
                u32 current_scope_;
                using LoopCounter = u32;
                std::stack<LoopCounter> frame_stack_;
                std::list<Parameter> function_arguments_;
        };

        ParseCtx::ParseCtx()
            : current_scope_(k_global_scope)
        {
                skip_next_scope_increment_.disable();
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

        void ParseCtx::enter_function()
        {
                ++current_scope_;
                skip_next_scope_increment_.enable();
                SANITY_ASSERT_LT(frame_stack_.size(), k_max_function_nesting);
                frame_stack_.push(0);
        }

        void ParseCtx::exit_function() noexcept
        {
                SANITY_ASSERT_GT(frame_stack_.size(), 0);
                frame_stack_.pop();
        }

        void ParseCtx::enter_loop() noexcept
        {
                SANITY_ASSERT_GT(frame_stack_.size(), 0);
                SANITY_ASSERT_LT(frame_stack_.top(), k_max_loop_nesting);
                ++frame_stack_.top();
        }

        void ParseCtx::exit_loop() noexcept
        {
                SANITY_ASSERT_GT(frame_stack_.size(), 0);
                SANITY_ASSERT_GT(frame_stack_.top(), 0);
                --frame_stack_.top();
        }

        u32 ParseCtx::loop_depth() const noexcept
        {
                SANITY_ASSERT_GT(frame_stack_.size(), 0);
                return frame_stack_.top();
        }

        u32 ParseCtx::function_depth() const noexcept { return frame_stack_.size(); }

        u32 ParseCtx::current_scope() const noexcept { return current_scope_; }

        void ParseCtx::append_function_argument(const std::string &name, CodeLocation location)
        {
                function_arguments_.emplace_back(name, location);
        }

        std::list<Parameter> ParseCtx::extract_function_arguments()
        {
                return std::move(function_arguments_);
        }
} // namespace Alpha
#endif // ALPHA_PARSER_CONTEXT_HPP
