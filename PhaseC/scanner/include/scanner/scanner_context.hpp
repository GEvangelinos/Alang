#ifndef SCANNER_CONTEXT_HPP
#define SCANNER_CONTEXT_HPP

#include <string>
#include "core/source_location.hpp"
#include "core/numeric_types.hpp"
#include "scanner/scanner_token.hpp"
#include "utils/misc.hpp"

namespace Alpha
{
        class LexerCtx : private Immobile
        {
        public:
                u32 index_;
                const std::string filename_;

                LexerCtx(const std::string &filename)
                    : index_(0),
                      filename_(filename) {}

                ~LexerCtx() { TokenID::clearLastId(); }
        };
}
#endif // SCANNER_CONTEXT_HPP