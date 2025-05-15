#ifndef ALPHA_BACKPATCHER_HPP
#define ALPHA_BACKPATCHER_HPP
#include "parser/alpha_symbol_table.hpp"

namespace Alpha::Backpatcher
{
        void set_function_local_variable_count(
            const Function *function_symbol,
            u32 local_variable_count);
}

#endif // ALPHA_BACKPATCHER_HPP
