#ifndef DRIVER_ERROR_HPP
#define DRIVER_ERROR_HPP

#include <stdexcept>
#include <string_view>

#include "support/format_adapter.hpp"
#include "support/size_format_tools.hpp"

namespace alpha::exception
{
class DriverError : public std::runtime_error
{
protected:
    using std::runtime_error::runtime_error;
};

class FileNotFoundError final : public DriverError
{
public:
    explicit FileNotFoundError(const std::string_view filename)
        : DriverError(FMT::format("no such file or directory: `{}`", filename)) {}
};

class FileNotRegularError final : public DriverError
{
public:
    explicit FileNotRegularError(const std::string_view filename)
        : DriverError(FMT::format("is not regular file: `{}`", filename)) {}
};

class FileIsADirectoryError final : public DriverError
{
public:
    explicit FileIsADirectoryError(const std::string_view filename)
        : DriverError(FMT::format("is a directory: `{}`", filename)) {}
};

class FileOpenError final : public DriverError
{
public:
    enum class Mode { READ, WRITE };

    explicit FileOpenError(const std::string_view filename, const Mode mode)
        : DriverError(FMT::format(
            "failed opening for {}: `{}`",
            [mode]()
            {
                switch (mode)
                {
                case Mode::READ: return "reading";
                case Mode::WRITE: return "writing";
                default: throw std::logic_error("Unknown file opening mode");
                }
            }(),
            filename
        )) {}
};

class FileTooLargeError final : public DriverError
{
public:
    explicit FileTooLargeError(
        const std::string_view filename, const std::size_t filesize, const std::size_t limit)
        : DriverError(FMT::format(
            "file too large (size = {}, limit = {}): `{}`",
            support::format_bytesize(filesize).to_string(),
            support::format_bytesize(limit).to_string(),
            filename
        )) {}
};

class UnknownCLIOptionError final : public DriverError
{
public:
    explicit UnknownCLIOptionError(const std::string_view cli_option)
        : DriverError(FMT::format("unknown command-line option: `{}`", cli_option)) {}
};
} // namespace alpha::exception

#endif //DRIVER_ERROR_HPP
