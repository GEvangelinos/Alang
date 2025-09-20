#ifndef DRIVER_EXCEPTIONS_HPP
#define DRIVER_EXCEPTIONS_HPP

#include <stdexcept>

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
    explicit FileNotFoundError(std::string_view filename);
};

class FileNotRegularError final : public DriverError
{
public:
    explicit FileNotRegularError(std::string_view filename);
};

class FileIsADirectoryError final : public DriverError
{
public:
    explicit FileIsADirectoryError(std::string_view filename);
};

class FileOpenError final : public DriverError
{
public:
    enum class Mode { READ, WRITE };

    FileOpenError(std::string_view filename, Mode mode);
};

class FileTooLargeError final : public DriverError
{
public:
    FileTooLargeError(std::string_view filename, std::size_t filesize, std::size_t limit);
};

class FileReadError final : public DriverError
{
public:
    explicit FileReadError(std::string_view filename);
};

class CLIOptionUnknownError final : public DriverError
{
public:
    explicit CLIOptionUnknownError(std::string_view cli_option);
};

class CLIOptionParseError final : public DriverError
{
public:
    CLIOptionParseError(
        std::string_view flag_name, std::string_view value, std::string_view type_name);
};

class CLIOptionRangeError final : public DriverError
{
public:
    CLIOptionRangeError(
        std::string_view flag_name, std::string_view value, std::string_view type_name);
};

class DiagnosticError {};

class DiagnosticFatalError final : DiagnosticError
{
public:
    DiagnosticFatalError() {}
};

class DiagnosticLimitError final : DiagnosticError
{
public:
    DiagnosticLimitError() {}
};
} // namespace alpha::exception

#endif // DRIVER_EXCEPTIONS_HPP
