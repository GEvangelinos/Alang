#include "bytecode_generator/bytecode_generator.hpp"

#include <assert.h>

#include "core/ir/ir_expr.hpp"

namespace alpha
{
u32
BytecodeGenerator::intern_string_literal(const ConstStringExpr& string_expr)
{
    const auto retval = string_literal_pool_.size();
    string_literal_pool_.push_back(string_expr.value);
    return retval;
}

u32
BytecodeGenerator::intern_libfunc_name(const LibFuncExpr& libfunc_expr)
{
    const auto retval = libfunc_name_pool_.size();
    const std::string& name = libfunc_expr.libfunc_symbol->name;
    libfunc_name_pool_.push_back(StringSpan{.dataa = name.data(), .size = name.size()});
    return retval;
}


const vm::Argument*
BytecodeGenerator::make_operand(const Expr& expr)
{
    using ET = Expr::Type;
    switch (expr.type)
    {
    case ET::ASSIGN:
    case ET::CONST_BOOL:
        return new vm::ConstBoolArgument{static_cast<const ConstBoolExpr&>(expr).value};
    case ET::CONST_INT:
        return new vm::ConstIntArgument{static_cast<const ConstIntExpr&>(expr).value};
    case ET::CONST_FLOAT:
        return new vm::ConstFloatArgument{static_cast<const ConstFloatExpr&>(expr).value};
    case ET::CONST_STRING:
        return new vm::ConstStringArgument{
            intern_string_literal(static_cast<const ConstStringExpr&>(expr))
        };
    case ET::CONST_NIL:
        return new vm::ConstNilArgument{};
    case ET::LIBRARY_FUNCTION:
        return new vm::LibFuncArgument{intern_libfunc_name(static_cast<const LibFuncExpr&>(expr))};
    case ET::PROGRAM_FUNCTION:
        return new vm::ProgramFuncArgument{static_cast<const ProgFuncExpr &>(expr).progfunc_symbol->address};

    case ET::ARITHMETIC:
    case ET::BOOL:
    case ET::NEW_TABLE:
    case ET::TABLE_ITEM:
    case ET::VARIABLE:
        DEBUG_SMART_ASSERT(expr.has_var_symbol());
        switch (const auto var = static_cast<const ExprWVarSymbol&>(expr).var_symbol; var->space)
        {
        case VarSymbol::Space::PROGRAM_VAR: return new vm::GlobalVariableArgument{var->offset};
        case VarSymbol::Space::FUNCTION_LOCAL: return new vm::LocalVariableArgument{var->offset};
        case VarSymbol::Space::FORMAL_ARGUMENT: return new vm::FormalVariableArgument{var->offset};
        default: DEBUG_UNREACHABLE("Unknown VarSymbol::Space");
        }
    default:
        UNREACHABLE(FMT::format("Unknown Expr::Type: int(type) = {}", static_cast<int>(expr.type)));
    }
}
} // namespace alpha
