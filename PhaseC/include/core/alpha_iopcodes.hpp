#ifndef ALPHA_IOPCODES_HPP
#define ALPHA_IOPCODES_HPP

#include <string>

#define IOPCODES        \
        X(ASSIGN)       \
        X(ADD)          \
        X(SUB)          \
        X(MUL)          \
        X(DIV)          \
        X(MOD)          \
        X(UMINUS)       \
        X(AND)          \
        X(OR)           \
        X(NOT)          \
        X(IF_EQ)        \
        X(IF_NEQ)       \
        X(IF_LESS)      \
        X(IF_GREATER)   \
        X(IF_LESSEQ)    \
        X(IF_GREATEREQ) \
        X(CALL)         \
        X(PARAM)        \
        X(RET)          \
        X(GETRETVAL)    \
        X(FUNCSTART)    \
        X(FUNCEND)      \
        X(TABLECREATE)  \
        X(TABLEGETITEM) \
        X(TABLESETITEM)

namespace Alpha
{
        enum class IOPCODE
        {
#define X(code) code,
                IOPCODES
#undef X
        };

        std::string to_string(IOPCODE code)
        {
                switch (code)
                {
#define X(code)             \
        case IOPCODE::code: \
                return #code;
                        IOPCODES
#undef X
                }
        }
}

#undef IOPCODES

#endif // ALPHA_IOPCODES_HPP
