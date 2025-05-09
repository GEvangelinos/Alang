#ifndef ALPHA_IR_HPP
#define ALPHA_IR_HPP

#include "parser/alpha_symbol_table.hpp"
#include "utils/misc.hpp"
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
        X(IF_NOTEQ)     \
        X(IF_LESS)      \
        X(IF_GREATER)   \
        X(IF_LESSEQ)    \
        X(IF_GREATEREQ) \
        X(JUMP)         \
        X(CALL)         \
        X(PARAM)        \
        X(RETURN)       \
        X(GETRETVAL)    \
        X(FUNCSTART)    \
        X(FUNCEND)      \
        X(TABLECREATE)  \
        X(TABLEGETELEM) \
        X(TABLESETELEM)

namespace Alpha
{
        enum class IOPCode
        {
#define X(code) code,
                IOPCODES
#undef X
        };

        inline std::string to_string(const IOPCode iopcode)
        {
                switch (iopcode)
                {
#define X(iopcode)             \
        case IOPCode::iopcode: \
                return str_to_lower(#iopcode);
                        IOPCODES
#undef X
                default:
                        return "UNKNOWN_IOPCODE";
                }
        }
#undef IOPCODES

        enum class RValueType
        {
                NUMBER,
                STRING,
                BOOLEAN,
                NIL,
                FUNCTION_ADDRESS,
                LIBRARY_FUNCTION,
        };

        class Expr
        {
        public:
                enum class Type
                {
                        VARIABLE,
                        TABLE_ITEM,
                        PROGRAM_FUNCTION,
                        LIBRARY_FUNCTION,
                        ARITHMETIC,
                        BOOLEAN,
                        ASSIGN,
                        NEW_TABLE,
                        CONST_NUMBER,
                        CONST_BOOLEAN,
                        CONST_STRING,
                        NIL,
                };
        };

        // TODO: Which of the following fields can you make const?
        // TODO: Also if some fields are initialized only once,
        //       but after the construction, create an object,
        //       that can be initialized only once (like const),
        //       but at arbitrary time.

        struct Quad
        {
                IOPCode iopcode;
                const Expr *arg1;
                const Expr *arg2;
                const Expr *result;
                u32 label; // TODO: do we need this? Maybe we can figure this out based on index on vector.
                Location location;
                // We pass location instead of plain line, so we can extract line and column.
                // It will make diagnostic messages in runtime, much more accurate.
        };
} // namespace Alpha
#endif // ALPHA_IR_HPP
