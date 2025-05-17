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
                CONST_NUMBER,
                CONST_STRING,
                CONST_BOOLEAN,
                CONST_NIL,
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
                        CONST_NIL,
                };

                const Type type;
                Expr(Type type) : type(type) {}
        };

        struct ExprConst : public Expr
        {
                ExprConst(Expr::Type type) : Expr(type)
                {
                        DEBUG_SMART_ASSERT(
                            type == Expr::Type::CONST_BOOLEAN ||
                            type == Expr::Type::CONST_NUMBER ||
                            type == Expr::Type::CONST_STRING ||
                            type == Expr::Type::CONST_NIL //
                        );
                }
        };

        struct ExprConstBoolean : public ExprConst
        {
                const bool value;

                explicit ExprConstBoolean(bool value)
                    : ExprConst(Expr::Type::CONST_BOOLEAN),
                      value(value) {}
        };

        struct ExprConstNumber : public ExprConst
        {
                const double value;

                ExprConstNumber(double value)
                    : ExprConst(Expr::Type::CONST_NUMBER),
                      value(value) {}
        };

        struct ExprConstString : public ExprConst
        {
                const std::string value;

                explicit ExprConstString(const char *value)
                    : ExprConst(Expr::Type::CONST_STRING),
                      value(value) {}

                ExprConstString(const std::string &value)
                    : ExprConst(Expr::Type::CONST_STRING),
                      value(value) {}
        };

        struct ExprConstNil : public ExprConst
        {
                ExprConstNil() : ExprConst(Expr::Type::CONST_NIL) {}
        };

        struct ExprLvalue : public Expr
        {
                const Symbol *symbol;
                ExprLvalue(Expr::Type type, const Symbol *symbol)
                    : Expr(type), symbol(symbol)
                {
                        DEBUG_SMART_ASSERT( // TODO: what else it shouldnt be? Like ArithmEXPR maybe?
                            type != Expr::Type::CONST_BOOLEAN,
                            type != Expr::Type::CONST_NUMBER,
                            type != Expr::Type::CONST_STRING,
                            type != Expr::Type::NIL //
                        );
                        DEBUG_SMART_ASSERT(symbol != nullptr);
                }

                ExprLvalue(const Symbol *symbol)
                    : ExprLvalue(to_expr_type(symbol->type), symbol)
                {
                        DEBUG_SMART_ASSERT(symbol != nullptr);
                }
        };
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

        struct ExprTableItem : public ExprLvalue
        {
                const ExprConst *table_index;

                ExprTableItem(const ExprLvalue *lvalue, const ExprConst *table_index)
                    : ExprLvalue(lvalue->type, lvalue->symbol),
                      table_index(table_index)
                {
                        DEBUG_SMART_ASSERT(lvalue != nullptr, table_index != nullptr);
                }
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
