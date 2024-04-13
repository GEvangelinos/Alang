#ifndef ALPHA_DEFS_HPP
#define ALPHA_DEFS_HPP

#define UNIX_COLOR_RESET "\033[0m"
#define UNIX_COLOR_RED "\033[91m"
#define UNIX_COLOR_GREEN "\033[92m"
#define UNIX_COLOR_BLUE "\033[94m"

#if defined(INSIDE_FLEX_FILE)
// PASS
#else
// PASS
#endif /* INSIDE_FLEX_FILE */

#if defined(INSIDE_BISON_FILE)
#include "./symbolTable.hpp"
#include <iostream>
#include <iomanip>

inline void displayLog(const std::string lhs, const std::string rhs)
{
        std::cout << UNIX_COLOR_GREEN << std::setw(20) << lhs << UNIX_COLOR_RESET << ":\t" << rhs << std::endl;
}

inline void lookUpVariableAndReport(const Alpha::SymbolTable &symbolTable, std::string name, uint32_t lineNumber)
{
        auto resultPair = symbolTable.lookUpVariable(name);
        if (resultPair.first == Alpha::OperationResult::Success)
                return; // All went well... Nothing to cry about.

        std::cout << UNIX_COLOR_RED;
        if (resultPair.first == Alpha::OperationResult::SymbolNotVariable)
                std::cerr << "Line " << lineNumber << ": token " << name << "is not a variable." << std::endl;
        else if (resultPair.first == Alpha::OperationResult::SymbolNotFound)
                std::cerr << "Line " << lineNumber << ": token " << name << "is undeclared." << std::endl;
        else
                std::cerr << "Line " << lineNumber << ": token " << name << "unknown error." << std::endl;
        std::cout << UNIX_COLOR_RESET;
}

inline void lookUpAndInsertVariable(const Alpha::SymbolTable &symbolTable, std::string name, uint32_t lineNumber)
{
        auto currentScope = symbolTable.getCurrentScope();
        auto resultPair = symbolTable.lookUpVariable(name);
        if (resultPair.first == Alpha::OperationResult::Success)
        ;// PASS 


}

// FIXME: If bison files are not singular... This will cause linking problems. (So far we are good...)
void yyerror(std::string errorMessage)
{
        std::cerr << UNIX_COLOR_RED << "Syntax error, line: " << alpha_yylineno << UNIX_COLOR_RESET << " " << errorMessage << std::endl;
        /* TODO: what else does this function do ? */
}
#endif /* INSIDE_BISON_BILE */

#endif /* ALPHA_DEFS_HPP */