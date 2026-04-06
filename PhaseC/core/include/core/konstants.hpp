#ifndef CORE_KONSTANTS_HPP
#define CORE_KONSTANTS_HPP

#include "core/numeric_types.hpp"
#include "core/source_location.hpp"

namespace alpha
{
static constexpr u32 k_max_source_filesize = static_cast<u32>(-1) / 2;

// ParseCtx's constants
static constexpr u32 k_max_function_nesting = 256; // Function nesting sanity limit.
static constexpr u32 k_max_call_nesting = 256;     // Function call nesting sanity limit.
static constexpr u32 k_max_dict_nesting = 256;     // Dict nesting sanity limit.
static constexpr u32 k_max_loop_nesting = 256;     // Loop nesting sanity limit.
static constexpr u32 k_max_scope = 1024;           // Block nesting sanity limit.

static constexpr u32 k_global_data_frame_count = 1;
static constexpr char k_global_data_frame_name[] = "__GLOBAL_DATA_FRAME__";

static_assert(k_global_data_frame_count == 1, "This constant must always be set to 1");

// SymbolTable's constants
#define ANONYMOUS_PREFIX "@"
#define TEMP_VARIABLE_PREFIX "$"

static constexpr u32 k_global_scope = 0;
static constexpr u32 k_libfunc_scope = k_global_scope;
static constexpr u32 k_libfunc_local_variable_count = 0;

constexpr char k_not_available_marker[] = "NA";
constexpr char k_not_available_pretty_marker[] = "-";
}

#endif // CORE_KONSTANTS_HPP
