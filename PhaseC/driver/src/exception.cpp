#include "driver/exception.hpp"
#include <iostream>
#include "support/format_adapter.hpp"
#include "support/size_format_tools.hpp"

namespace alpha::exception
{
FileNotFoundError::FileNotFoundError(const std::string_view filename)
    : DriverError(FMT::format("no such file or directory: `{}`", filename)) {}

FileNotRegularError::FileNotRegularError(const std::string_view filename)
    : DriverError(FMT::format("is not regular file: `{}`", filename)) {}

FileIsADirectoryError::FileIsADirectoryError(const std::string_view filename)
    : DriverError(FMT::format("is a directory: `{}`", filename)) {}

FileOpenError::FileOpenError(const std::string_view filename, const Mode mode)
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

FileTooLargeError::FileTooLargeError(
    const std::string_view filename, const std::size_t filesize, const std::size_t limit)
    : DriverError(FMT::format(
        "file too large (size = {}, limit = {}): `{}`",
        support::format_bytesize(filesize).to_string(),
        support::format_bytesize(limit).to_string(),
        filename
    )) {}

FileReadError::FileReadError(const std::string_view filename)
    : DriverError(FMT::format("Failed reading file: `{}`", filename)) {}

CLIOptionUnknownError::CLIOptionUnknownError(const std::string_view cli_option)
    : DriverError(FMT::format("unknown command-line option: `{}`", cli_option)) {}

CLIOptionParseError::CLIOptionParseError(
    const std::string_view flag_name,
    const std::string_view value,
    const std::string_view type_name)
    : DriverError(FMT::format(
        "Option '--{}' with value '{}' cannot be parsed as {}.",
        flag_name, value, type_name
    )) {}

CLIOptionRangeError::CLIOptionRangeError(
    const std::string_view flag_name,
    const std::string_view value,
    const std::string_view type_name)
    : DriverError(FMT::format(
        "Option '--{}' with value '{}' is out of range for {}.",
        flag_name, value, type_name
    )) {}
} // namespace alpha::exception
