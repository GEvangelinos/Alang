#ifndef ALPHA_DRIVER_EXCEPTIONS_HPP
#define ALPHA_DRIVER_EXCEPTIONS_HPP

class DiagnosticFatalError {};

class SanityLimitExceeded final : public std::runtime_error
{
public:
    explicit SanityLimitExceeded(const std::string &sanity_limit_description)
        : std::runtime_error(FMT::format("SanityLimitExceeded: {}", sanity_limit_description)) {}
};

#endif // ALPHA_DRIVER_EXCEPTIONS_HPP
