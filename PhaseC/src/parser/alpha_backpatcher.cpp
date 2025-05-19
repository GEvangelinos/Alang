#include "parser/alpha_backpatcher.hpp"

namespace Alpha::Backpatcher
{
        void set_function_local_variable_count(
            const Function *function_symbol,
            u32 local_variable_count)
        {
                DEBUG_SMART_ASSERT(!!function_symbol);
                DEBUG_SMART_ASSERT(function_symbol->is_function());
                const_cast<Function *>(function_symbol)->local_variable_count.set(local_variable_count);
        }
}