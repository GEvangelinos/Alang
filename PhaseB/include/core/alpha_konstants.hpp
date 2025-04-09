#ifndef ALPHA_KONSTANTS_HPP
#define ALPHA_KONSTANTS_HPP
#include "core/alpha_types.hpp"

namespace Alpha
{
        // Sanity check limits
        static constexpr u32 k_frame_stack_max_size = 256; // Function nesting limit.
        static constexpr u32 k_loop_max_nesting = 256;     // Loop nesting limit.

        // SymbolTable's contants
        static constexpr char k_anonymous_function_prefix[] = "SavvidisLemeKaiKleme#";
        static constexpr u32 k_global_scope_depth = 0;


}

#endif // ALPHA_KONSTANTS_HPP