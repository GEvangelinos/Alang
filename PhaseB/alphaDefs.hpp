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
        #include <iostream>
        #include <iomanip>

        #define DISPLAY_LOG(LHS, RHS) std::cout << UNIX_COLOR_GREEN << std::setw(20) << LHS << UNIX_COLOR_RESET << ":\t" << RHS << std::endl;
        void yyerror(std::string message);
        //FIXME: If bison files are not singular... This will cause linking problems. (So far we are good...)
        void yyerror(std::string errorMessage)
        {
                std::cerr << UNIX_COLOR_RED << "Syntax error, line: " << alpha_yylineno << UNIX_COLOR_RESET << " " << errorMessage << std::endl;
                /* TODO: what else does this function do ? */
        }
#endif /* INSIDE_BISON_BILE */

#endif /* ALPHA_DEFS_HPP */