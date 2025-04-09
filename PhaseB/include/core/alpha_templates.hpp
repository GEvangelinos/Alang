#ifndef ALPHA_TEMPLATES_HPP
#define ALPHA_TEMPLATES_HPP

#include <limits>
#include "misc/sanity_assert.h"
#include "core/alpha_types.hpp"

namespace Alpha
{
        template <typename Tag>
        class Counter
        {
        public:
                Counter() : value_(0) {}
                Counter(u32 initial_value) : value_(initial_value) {}

                ~Counter() = default;
                Counter(const Counter &) = delete;
                Counter(Counter &&) = delete;
                Counter &operator=(const Counter &) = delete;
                Counter &operator=(Counter &&) = delete;

                u32 value() const noexcept { return value_; }

                void inc() noexcept
                {
                        SANITY_ASSERT_LT(value_, std::numeric_limits<decltype(value_)>::max());
                        ++value_;
                }
                u32 pre_inc()
                {
                        inc();
                        return value();
                }

                u32 post_inc()
                {
                        inc();
                        return value() - 1;
                }

                void dec() noexcept
                {
                        SANITY_ASSERT_GT(value_, std::numeric_limits<decltype(value_)>::min());
                        --value_;
                }
                u32 pre_dec()
                {
                        dec();
                        return value();
                }
                u32 post_dec()
                {
                        dec();
                        return value() + 1;
                }

        private:
                u32 value_;
        };
} // namespace Alpha

#endif // ALPHA_TEMPLATES_HPP