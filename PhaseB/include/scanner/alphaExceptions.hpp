#ifndef ALPHA_EXCEPTIONS_HPP
#define ALPHA_EXCEPTIONS_HPP

#include <stdexcept>

namespace Alpha
{
        class [[maybe_unused]] UnexpectedEOF : public std::runtime_error
        {
        public:
                UnexpectedEOF(std::string runtimeMessage)
                    : std::runtime_error(runtimeMessage)
                {
                }
        };

        class [[maybe_unused]] BlockCommentEOF : public std::runtime_error
        {
        public:
                BlockCommentEOF(std::string exceptionMessage)
                    : std::runtime_error(exceptionMessage)
                {
                }
        };

        class [[maybe_unused]] StringEOF : public std::runtime_error
        {
        public:
                StringEOF(std::string exceptionMessage)
                    : std::runtime_error(exceptionMessage)
                {
                }
        };

        class [[maybe_unused]] InvalidCharacter : public std::runtime_error
        {
        public:
                InvalidCharacter(std::string exceptionMessage)
                    : std::runtime_error(exceptionMessage)
                {
                }
        };

} /* namespace Alpha */

#endif /* ALPHA_EXCEPTIONS_HPP */