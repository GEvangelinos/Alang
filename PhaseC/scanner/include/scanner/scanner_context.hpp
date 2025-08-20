#ifndef SCANNER_CONTEXT_HPP
#define SCANNER_CONTEXT_HPP

#include <string>

#include "core/konstants.hpp"
#include "core/source_location.hpp"
#include "core/numeric_types.hpp"
#include "scanner/scanner_token.hpp"
#include "utils/misc.hpp"

namespace alpha
{
        class LexerCtx : private Immobile
        {
        public:
                u32 index_;

                explicit LexerCtx()
                    : index_(0){}

                ~LexerCtx() { TokenID::clearLastId(); }
        };
}
#endif // SCANNER_CONTEXT_HPP