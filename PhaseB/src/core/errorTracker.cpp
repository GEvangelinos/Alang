#include <format>
#include "core/errorTracker.hpp"


namespace Alpha
{
    CompileTimeError::CompileTimeError(uint32_t line, uint32_t column, std::string errorMessage)
        : line(line), column(column), errorMessage(errorMessage)
    {
    }

    std::string CompileTimeError::toString() const
    {
        return std::format("{} [{}:{}] {}", this->getErrorType(), this->line, this->column, this->errorMessage);
    }

    LexerError::LexerError(uint32_t line, uint32_t column, std::string errorMessage)
        : CompileTimeError(line, column, errorMessage)
    {
    }

    SyntaxError::SyntaxError(uint32_t line, uint32_t column, std::string errorMessage)
        : CompileTimeError(line, column, errorMessage)
    {
    }

    void CompileTimeErrorTracker::registerCompileTimeError(const CompileTimeError *const error)
    {
        this->errorVector.push_back(error);
    }

    const std::vector<const CompileTimeError *> &CompileTimeErrorTracker::gerErrorVector() const
    {
        return this->errorVector;
    }
}