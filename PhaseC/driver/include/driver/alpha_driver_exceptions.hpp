#ifndef ALPHA_DRIVER_EXCEPTIONS_HPP
#define ALPHA_DRIVER_EXCEPTIONS_HPP

namespace alpha::exceptions
{
class DiagnosticFatalError {};

class DiagnosticErrorLimitExceededError {};

class SanityLimitExceededError final : public std::runtime_error
{
public:
    explicit SanityLimitExceededError(const std::string &sanity_limit_description)
        : std::runtime_error(FMT::format("SanityLimitExceeded: {}", sanity_limit_description)) {}
};

class StartupError : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};
} // alpha::exceptions

#endif // ALPHA_DRIVER_EXCEPTIONS_HPP
