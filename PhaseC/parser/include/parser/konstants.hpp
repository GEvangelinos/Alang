#ifndef PARSER_KONSTANTS_HPP
#define PARSER_KONSTANTS_HPP
#include "internal_typedefs.hpp"
#include "core/code_address.hpp"

namespace alpha
{
// Function addresses are positive integers, so we start from 1.
static constexpr CodeAddress k_first_label = CodeAddress{1};
static constexpr u32 k_first_function_address = 0; // TODO: Why 1? Why not 0?

// static_assert(k_first_label > LabelID::none());
} // namespace alpha
#endif // PARSER_KONSTANTS_HPP
