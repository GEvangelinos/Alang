// This file should contain all overloads of to_string for project's type and more
// The generated diagnostic system relies heavily on this file.

#ifndef TO_STRING_HPP
#define TO_STRING_HPP
#include  <string>
#include <parser/ir.hpp>

namespace Alpha
{
// This enforces intentionality in formatting and fails clearly
// at compile time if we try to log an unsupported type.
template<typename T>
std::string to_string(const T &) = delete;

[[nodiscard]] inline const std::string &to_string(const std::string &s) { return s; }
[[nodiscard]] inline const std::string &to_string(const char *const s) { return s; }
[[nodiscard]] std::string to_string(IOPCode iopcode);
[[nodiscard]] const char *to_string(Expr::Type type);
[[nodiscard]] const char *to_string(OperandSide pos);
}
#endif // TO_STRING_HPP
