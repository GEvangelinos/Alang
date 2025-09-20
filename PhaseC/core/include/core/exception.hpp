#ifndef CORE_EXCEPTION_HPP
#define CORE_EXCEPTION_HPP

#include <stdexcept>

namespace alpha::exception
{
class CoreError : public std::runtime_error
{
protected:
    using std::runtime_error::runtime_error;
};

class SanityLimitError final : public CoreError
{
    explicit SanityLimitError(const std::string &error_message)
        : CoreError(error_message) {}
};
} // namespace alpha::exception
#endif // CORE_EXCEPTION_HPP
