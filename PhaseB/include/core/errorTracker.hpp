#ifndef ERROR_TRACKER_HPP
#define ERROR_TRACKER_HPP

#include <string>
#include <vector>
#include <cstdint>
namespace Alpha
{
        class CompileTimeError
        {
        protected:
                uint32_t line;
                uint32_t column;
                std::string errorMessage;

        public:
                CompileTimeError(uint32_t line, uint32_t column, std::string errorMessage);
                CompileTimeError() = delete;
                virtual std::string getErrorType() const = 0;
                virtual std::string toString() const;
        };

        class LexerError : public CompileTimeError
        {
        private:
        public:
                LexerError(uint32_t line, uint32_t column, std::string errorMessage);
                LexerError() = delete;

                std::string getErrorType() const override { return "Lexer Error"; }
        };

        class SyntaxError : public CompileTimeError
        {
        public:
                SyntaxError(uint32_t line, uint32_t column, std::string errorMessage);
                SyntaxError() = delete;

                std::string getErrorType() const override { return "Syntax Error"; }
        };

        class CompileTimeErrorTracker
        {
        private:
                std::vector<const CompileTimeError *> errorVector;

        public:
                CompileTimeErrorTracker() = default;
                void registerCompileTimeError(const CompileTimeError *error);
                const std::vector<const CompileTimeError *> &gerErrorVector() const;
        };
}

#endif /* ERROR_TRACKER_HPP */
