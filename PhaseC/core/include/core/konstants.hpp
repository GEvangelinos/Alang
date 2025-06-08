#ifndef ALPHA_KONSTANTS_HPP
#define ALPHA_KONSTANTS_HPP

#include "core/numeric_types.hpp"
#include "core/source_location.hpp"

namespace Alpha
{
// ParseCtx's constants
static constexpr u32 k_max_function_nesting = 256; // Function nesting sanity limit.
static constexpr u32 k_max_loop_nesting = 256;     // Loop nesting sanity limit.
static constexpr u32 k_max_scope = 1024;           // Block nesting sanity limit.

static constexpr u32 k_global_data_frame_count = 1;
static constexpr char k_global_data_frame_name[] = "__GLOBAL_DATA_FRAME__";

static_assert(k_global_data_frame_count == 1, "This constant must always be set to 1");

// SymbolTable's constants
static constexpr char k_private_anonymous_prefix[] = "@f";
static constexpr char k_public_anonymous_prefix[] = "@nonymous";
static constexpr char k_temp_variable_prefix[] = "_t";

static constexpr u32 k_global_scope = 0;
static constexpr u32 k_libfunc_local_variable_count = 0;
static constexpr u32 k_no_line = 0;
static constexpr SourceLocation k_no_loc = {0, 0};

constexpr char k_not_available_marker[] = "-";
}

#endif // ALPHA_KONSTANTS_HPP
