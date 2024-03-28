#ifndef ALPHA_DEFS_HPP
#define ALPHA_DEFS_HPP

#if defined(INSIDE_FLEX_FILE)
#else
#endif /* INSIDE_FLEX_FILE */

#if defined(INSIDE_BISON_FILE)
        #include <iostream>
        /* TODO: DEFINE TYPE yyerror(msg) */

        #define DISPLAY_LOG(LHS, RHS) std::cout << LHS << ":\t" << RHS << std::endl;
#endif /* INSIDE_BISON_BILE */


#endif /* ALPHA_DEFS_HPP */