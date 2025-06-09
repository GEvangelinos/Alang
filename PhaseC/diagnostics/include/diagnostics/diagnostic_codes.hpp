#ifndef DIAGNOSTIC_CODES_HPP
#define DIAGNOSTIC_CODES_HPP

namespace Alpha
{
    enum class WarningCodes {};

    enum class ErrorCodes
    {
        ASSIGNMENT_LHS_NOT_LVALUE,
        ASSIGNMENT_LHS_IS_LIB_FUNC,
        ASSIGNMENT_LHS_IS_FUNC,
    };

    enum class FatalCodes {};
} // namespace Alpha

#endif // DIAGNOSTIC_CODES_HPP
