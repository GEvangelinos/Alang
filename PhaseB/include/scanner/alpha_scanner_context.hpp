#ifndef SCANNER_CONTEXT_HPP
#define SCANNER_CONTEXT_HPP

#include <string>
#include "core/alpha_location.hpp"
#include "core/alpha_types.hpp"
#include "scanner/alpha_token.hpp"

namespace Alpha
{
        struct LexerCtx
        {
                u32 index_;
                const std::string filename_;

                LexerCtx() = delete;
                LexerCtx(const std::string &filename)
                    : index_(0),
                      filename_(filename) {}

                LexerCtx(const LexerCtx &) = delete;
                LexerCtx(const LexerCtx &&) = delete;
                LexerCtx operator=(const LexerCtx &) = delete;
                LexerCtx operator=(LexerCtx &&) = delete;
                ~LexerCtx() { TokenID::clearLastId(); }
        };
}
#endif // SCANNER_CONTEXT_HPP