#ifndef ALPHA_DRIVER_EXCEPTIONS_HPP
#define ALPHA_DRIVER_EXCEPTIONS_HPP

#include <stdexcept>

namespace alpha::exceptions
{
class DiagnosticFatalError {};

class DiagnosticErrorLimitExceeded {};

class SanityLimitExceededError final : public std::runtime_error
{
public:
    explicit SanityLimitExceededError(const std::string &sanity_limit_description);
};

class StartupError final : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};


} // alpha::exceptions

#endif // ALPHA_DRIVER_EXCEPTIONS_HPP
