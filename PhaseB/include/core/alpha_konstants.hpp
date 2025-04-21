#ifndef ALPHA_KONSTANTS_HPP
#define ALPHA_KONSTANTS_HPP

#include "core/alpha_types.hpp"

namespace Alpha
{
        // Driver's contants (TODO: If you get more flags, move in separate file)
        static constexpr char k_alpha_driver_description[] = "A tool for syntactical analysis on programming language Alpha";
        static constexpr char k_symtable_csv_export_prefix[] = "ST_";
        static constexpr char k_symtable_csv_exports_dir[] = "symtable_exports";

        static constexpr char k_flag_input_file[] = "input-file";
        static constexpr char k_flag_input_file_help[] = "Use flag to provide the alpha file you want to parse.";

        static constexpr char k_flag_export_csv[] = "export-csv";
        static constexpr char k_flag_export_csv_help[] = "Enable CSV export of the symbol table after parsing.";

        // ParseCtx's contants
        static constexpr u32 k_max_function_nesting = 256; // Function nesting sanity limit.
        static constexpr u32 k_max_loop_nesting = 256;     // Loop nesting sannity limit.
        static constexpr u32 k_max_scope = 1024;           // Block nesting sanity limit.

        static constexpr u32 k_data_frames_outside_functions = 1;
        static constexpr char k_global_data_frame_name[] = "__GLOBAL_DATA_FRAME__";

        static_assert(k_data_frames_outside_functions == 1, "This constant must always be set to 1");

        // SymbolTable's constants
        // static constexpr char k_private_anonymous_prefix[] = "SavvidisLemeKaiKleme#";
        static constexpr char k_private_anonymous_prefix[] = "#f_";
        static constexpr char k_public_anonymous_prefix[] = "anonymous#";
        static constexpr u32 k_global_scope = 0;

        static constexpr u32 k_no_line = 0;

}

#endif // ALPHA_KONSTANTS_HPP