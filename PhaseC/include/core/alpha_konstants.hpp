#ifndef ALPHA_KONSTANTS_HPP
#define ALPHA_KONSTANTS_HPP

#include "core/alpha_types.hpp"
#include "core/alpha_location.hpp"

namespace Alpha
{
        // ParseCtx's contants
        static constexpr u32 k_max_function_nesting = 256; // Function nesting sanity limit.
        static constexpr u32 k_max_loop_nesting = 256;     // Loop nesting sannity limit.
        static constexpr u32 k_max_scope = 1024;           // Block nesting sanity limit.

        static constexpr u32 k_global_data_frame_count = 1;
        static constexpr char k_global_data_frame_name[] = "__GLOBAL_DATA_FRAME__";

        static_assert(k_global_data_frame_count == 1, "This constant must always be set to 1");

        // SymbolTable's constants
        // static constexpr char k_private_anonymous_prefix[] = "SavvidisLemeKaiKleme#";
        static constexpr char k_private_anonymous_prefix[] = "@f";
        static constexpr char k_public_anonymous_prefix[] = "@nonymous";
        static constexpr char k_temp_variable_prefix[] = "$t";

        static constexpr u32 k_global_scope = 0;
        static constexpr u32 k_no_line = 0;
        static constexpr Location k_no_location = {0, 0};

}

#endif // ALPHA_KONSTANTS_HPP