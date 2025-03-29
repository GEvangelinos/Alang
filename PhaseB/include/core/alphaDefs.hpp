#ifndef ALPHA_DEFS_HPP
#define ALPHA_DEFS_HPP

#include "symbolTable.hpp"
#include "errorTracker.hpp"


// FIXME: If bison files are not singular... This will cause linking problems. (So far we are good...)
void yyerror(std::string errorMessage)
{
        extern Alpha::SymbolTable symbolTable;
        extern int alpha_yylineno;
        symbolTable.errorTracker.registerCompileTimeError(new Alpha::SyntaxError(alpha_yylineno, 0, errorMessage));
        /* TODO: what else does this function do ? */
}

#endif /* ALPHA_DEFS_HPP */