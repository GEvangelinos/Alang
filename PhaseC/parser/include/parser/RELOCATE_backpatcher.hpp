#ifndef ALPHA_BACKPATCHER_HPP
#define ALPHA_BACKPATCHER_HPP
#include "parser/symbol_table.hpp"

// TODO: remove file (too little use)
namespace Alpha::Backpatcher
{
    void set_function_local_variable_count(
        const FuncSymbol *function_symbol,
        u32 local_variable_count);
}

#endif // ALPHA_BACKPATCHER_HPP
