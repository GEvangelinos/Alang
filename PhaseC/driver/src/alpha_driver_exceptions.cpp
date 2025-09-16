// Implementation of alpha::exceptions is placed here (not in the header) because they
// rely on FMT, which is relatively heavy. This avoids polluting the header and keeps
// compile times and dependencies cleaner.

#include "driver/alpha_driver_exceptions.hpp"
#include "support/format_adapter.hpp"

namespace alpha::exceptions
{
SanityLimitExceededError::SanityLimitExceededError(const std::string &sanity_limit_description)
    : std::runtime_error(FMT::format("SanityLimitExceeded: {}", sanity_limit_description)) {}
} // namespace alpha
