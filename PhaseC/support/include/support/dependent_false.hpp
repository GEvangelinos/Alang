#ifndef DEPENDENT_FALSE_HPP
#define DEPENDENT_FALSE_HPP

template<typename>
inline constexpr bool always_false_v = false;
template<typename>
inline constexpr bool always_true_v = true;

#endif //DEPENDENT_FALSE_HPP
