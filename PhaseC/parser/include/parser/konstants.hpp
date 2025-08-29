#ifndef PARSER_KONSTANTS_HPP
#define PARSER_KONSTANTS_HPP
#include "internal_typedefs.hpp"

namespace alpha
{
static constexpr LabelID k_no_label = 0;
static constexpr LabelID k_first_label = 1;

static_assert(k_first_label > k_no_label);
} // namespace alpha
#endif // PARSER_KONSTANTS_HPP
