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

        [[nodiscard]] inline std::string to_string(const IOPCode iopcode)
        {
                switch (iopcode)
                {
#define X(iopcode)             \
        case IOPCode::iopcode: \
                return str_to_lower(#iopcode);
                        IOPCODES
#undef X
                default:
                        [[unlikely]] SMART_ASSERT(false);
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

        struct Expr
        {
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

                const Type type_;
                const Symbol *symbol_;

                Expr(Type type, const Symbol *symbol)
                    : type_(type), symbol_(symbol) {}
        };
        // Expr *index;
        // union
        // {
        //         double const_num;
        //         char *const_str;
        //         bool const_bool;
        // };

        constexpr Expr::Type to_expr_type(Symbol::Type symbol_type)
        {
                switch (symbol_type)
                {
                case Symbol::Type::VARIABLE:
                        return Expr::Type::VARIABLE;
                case Symbol::Type::LIBRARY_FUNCTION:
                        return Expr::Type::LIBRARY_FUNCTION;
                case Symbol::Type::PROGRAM_FUNCTION:
                        return Expr::Type::PROGRAM_FUNCTION;
                default:
                        [[unlikely]] SMART_ASSERT(false);
                }
        }

        struct ExprLvalue : public Expr
        {
                ExprLvalue(const Symbol *symbol)
                    : Expr(to_expr_type(symbol->type), symbol) {}
        };

        struct ExprConst : protected Expr
        {
                union
                {
                        double const_num;
                        char *const_str;
                        bool const_bool;
                };
        };

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
