#ifndef ALPHA_IR_HPP
#define ALPHA_IR_HPP

#include "parser/alpha_symbols.hpp"
#include "utils/misc.hpp"
#include <string>

#define IOPCODES_WITH_LABEL \
        X(IF_EQ)            \
        X(IF_NOTEQ)         \
        X(IF_LESS)          \
        X(IF_GREATER)       \
        X(IF_LESSEQ)        \
        X(IF_GREATEREQ)     \
        X(JUMP)

#define IOPCODES_WITHOUT_LABEL \
        X(ASSIGN)              \
        X(ADD)                 \
        X(SUB)                 \
        X(MUL)                 \
        X(DIV)                 \
        X(MOD)                 \
        X(UMINUS)              \
        X(AND)                 \
        X(OR)                  \
        X(NOT)                 \
        X(CALL)                \
        X(PARAM)               \
        X(RETURN)              \
        X(GETRETVAL)           \
        X(FUNCSTART)           \
        X(FUNCEND)             \
        X(TABLECREATE)         \
        X(TABLEGETELEM)        \
        X(TABLESETELEM)

#define ALL_IOPCODES        \
        IOPCODES_WITH_LABEL \
        IOPCODES_WITHOUT_LABEL

namespace Alpha
{
        // clang-format off
        enum class IOPCode
        {
        #define X(code) code,
                ALL_IOPCODES
        #undef  X
        };
        // clang-format on

        [[nodiscard]] inline std::string to_string(const IOPCode iopcode)
        {
                // clang-format off
                switch (iopcode)
                {
                #define X(iopcode) case IOPCode::iopcode: return str_to_lower(#iopcode);
                        ALL_IOPCODES
                #undef  X
                default: [[unlikely]] SMART_ASSERT(false);
                }
                // clang-format on
        }
#undef IOPCODES

        enum class RValueType
        {
                CONST_BOOLEAN,
                CONST_INT,
                CONST_REAL,
                CONST_STRING,
                CONST_NIL,
                FUNCTION_ADDRESS,
                LIBRARY_FUNCTION,
        };

        struct Expr // Tagged Union
        {
                enum class Type : u8
                {
                        ARITHMETIC_EXPR,
                        ASSIGN_EXPR,
                        BOOLEAN_EXPR,
                        CONST_BOOL,
                        CONST_INT,
                        CONST_NIL,
                        CONST_REAL,
                        CONST_STRING,
                        LIBRARY_FUNCTION,
                        NEW_TABLE,
                        PROGRAM_FUNCTION,
                        TABLE_ITEM,
                        VARIABLE,

                };

                const Type type;
                const Symbol *symbol;
                Location location;

                union
                {
                        const Expr *index;
                        const f64 const_real;
                        const i64 const_int;
                        const char *const_str;
                        const bool const_bool;
                };
        };

        constexpr Expr::Type to_expr_type(Symbol::Type symbol_type)
        {
                switch (symbol_type)
                {
                case Symbol::Type::GLOBAL_VARIABLE:
                case Symbol::Type::FORMAL_ARGUMENT:
                case Symbol::Type::LOCAL_VARIABLE:
                        return Expr::Type::VARIABLE;
                case Symbol::Type::LIBRARY_FUNCTION:
                        return Expr::Type::LIBRARY_FUNCTION;
                case Symbol::Type::PROGRAM_FUNCTION:
                        return Expr::Type::PROGRAM_FUNCTION;
                default:
                        [[unlikely]] SMART_ASSERT(false);
                }
        }

        // TODO: Which of the following fields can you make const?
        // TODO: Also if some fields are initialized only once,
        //       but after the construction, create an object,
        //       that can be initialized only once (like const),
        //       but at arbitrary time.
        struct Quad
        {
                const IOPCode iopcode;
                const Expr *arg1;
                const Expr *arg2;
                const Expr *result;
                u32 label; // TODO: do we need this? Maybe we can figure this out based on index on vector.
                           // First quad_label is always 1, (0 for backpatching)
                const Location location;
                // We pass location instead of plain line, so we can extract line and column.
                // It will make diagnostic messages in runtime, much more accurate.
        };
} // namespace Alpha
#endif // ALPHA_IR_HPP
