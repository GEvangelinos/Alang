#ifndef ALPHA_EXCEPTIONS_HPP
#define ALPHA_EXCEPTIONS_HPP

#include <stdexcept>

namespace Alpha
{
        class UnexpectedEOF : public std::runtime_error
        {
        public:
                [[deprecated("Too general")]] UnexpectedEOF(std::string runtimeMessage)
                    : std::runtime_error(runtimeMessage)
                {
                }
        };

        class BlockCommentEOF : public std::runtime_error
        {
        public:
                BlockCommentEOF(std::string exceptionMessage)
                    : std::runtime_error(exceptionMessage)
                {
                }
        };

        class StringEOF : public std::runtime_error
        {
        public:
                StringEOF(std::string exceptionMessage)
                    : std::runtime_error(exceptionMessage)
                {
                }
        };

        class InvalidCharacter : public std::runtime_error
        {
        public:
                InvalidCharacter(std::string exceptionMessage)
                    : std::runtime_error(exceptionMessage)
                {
                }
        };

} /* namespace Alpha */

#endif /* ALPHA_EXCEPTIONS_HPP */