#include "parser/alpha_parser_context.hpp"
#include "misc/sanity_assert.h"
#include <limits>

namespace Alpha
{
        template <typename Tag>
        u32 Counter<Tag>::value() const noexcept { return value_; }

        template <typename Tag>
        void Counter<Tag>::inc() noexcept
        {
                SANITY_ASSERT_LT(value_, std::numeric_limits<decltype(value_)>::max());
                ++value_;
        }

        template <typename Tag>
        void Counter<Tag>::dec() noexcept
        {
                SANITY_ASSERT_GT(value_, std::numeric_limits<decltype(value_)>::min());
                --value_;
        }

        template class Counter<ScopeTag>;
        template class Counter<FunctionTag>;
        template class Counter<LoopTag>;

        void ControlFlowCtx::enter_function()
        {
                SANITY_ASSERT_LT(frame_stack_.size(), k_frame_stack_max_size);
                frame_stack_.emplace(0);
        }
        void ControlFlowCtx::exit_function()
        {
                SANITY_ASSERT_GT(frame_stack_.size(), 0);
                frame_stack_.pop();
        }

        void ControlFlowCtx::enter_loop() noexcept
        {
                SANITY_ASSERT_GT(frame_stack_.size(), 0);
                SANITY_ASSERT_LT(frame_stack_.top().value(), k_loop_max_nesting);
                frame_stack_.top().inc();
        }

        void ControlFlowCtx::exit_loop() noexcept
        {
                SANITY_ASSERT_GT(frame_stack_.size(), 0);
                SANITY_ASSERT_GT(frame_stack_.top().value(), 0);
                frame_stack_.top().dec();
        }

        u32 ControlFlowCtx::loop_depth() const noexcept
        {
                SANITY_ASSERT_GT(frame_stack_.size(), 0);
                return frame_stack_.top().value();
        }

        u32 ControlFlowCtx::function_depth() const noexcept { return frame_stack_.size(); }
} // namespace Alpha