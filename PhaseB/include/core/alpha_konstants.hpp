#ifndef ALPHA_KONSTANTS_HPP
#define ALPHA_KONSTANTS_HPP

#include "core/alpha_types.hpp"

namespace Alpha
{
        // ParseCtx's contants
        static constexpr u32 k_max_function_nesting = 256; // Function nesting sanity limit.
        static constexpr u32 k_max_loop_nesting = 256;     // Loop nesting sannity limit.
        static constexpr u32 k_max_scope = 1024;           // Block nesting sanity limit.

        static constexpr u32 k_data_frames_outside_functions = 1; 
        static constexpr char k_global_data_frame_name[] = "__GLOBAL_DATA_FRAME__";

        static_assert(k_data_frames_outside_functions == 1, "This constant must always be set to 1");

        // SymbolTable's contants
        static constexpr char k_private_anonymous_prefix[] = "SavvidisLemeKaiKleme#";
        static constexpr char k_public_anonymous_prefix[] = "anonymous#";
        static constexpr u32 k_global_scope = 0;

}

#endif // ALPHA_KONSTANTS_HPP