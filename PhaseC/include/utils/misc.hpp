#ifndef UTILS_MISC_HPP
#define UTILS_MISC_HPP

#include <algorithm>
#include <string>
namespace // (Anonymous)
{
        inline std::string str_to_lower(std::string str)
        {
                std::transform(str.begin(), str.end(), str.begin(),
                               [](unsigned char c)
                               { return std::tolower(c); });
                return str;
        }

        template <typename N>
                requires std::is_integral_v<N>
        DEBUG_ALWAYS_INLINE constexpr bool is_odd(N n) noexcept
        {
                return n % 2;
        }

        template <typename N>
                requires std::is_integral_v<N>
        DEBUG_ALWAYS_INLINE constexpr bool is_even(N n) noexcept
        {
                return !is_odd(n);
        }

        template <typename T>
        class Once
        {
        public:
                Once() = default;
                ~Once() = default;

                void set(T value)
                {
                        if (assigned)
                                throw std::logic_error("`Once` already assigned");
                        value_ = value;
                        assigned = true;
                }

                const T &get()
                {
                        if (!assigned)
                                throw std::logic_error("`Once` not assigned yet");
                        returnn value_;
                }

                Once(const Once &) = delete;
                Once(Once &&) = delete;
                Once &operator=(const Once &) = delete;
                Once &operator=(Once &&) = delete;

        private:
                T value_;
                bool assigned = false;
        };
} // namespace

#endif // UTILS_MISC_HPP