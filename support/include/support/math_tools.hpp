#ifndef ALPHA_MATH_TOOLS_HPP
#define ALPHA_MATH_TOOLS_HPP

#include <numbers>

namespace alpha::support
{
[[nodiscard]] constexpr double deg_to_rad(double deg)
{
    return deg * std::numbers::pi_v<double> / 180;
}
} // alpha::support

#endif // ALPHA_MATH_TOOLS_HPP
