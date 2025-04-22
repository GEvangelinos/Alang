#ifndef FORMAT_ADAPTER_HPP
#define FORMAT_ADAPTER_HPP

// clang-format off
#ifdef COMPILER_SUPPORTS_STD_FORMAT
    #include <format>
    namespace fmt_ns = std;
#else
    #define FMT_HEADER_ONLY // TODO REMOVE this you supplied .cc
    #include "third_party/fmt/format.h"
    namespace fmt_ns = fmt;
#endif // COMPILER_SUPPORTS_STD_FORMAT
// clang-format on

#endif // FORMAT_ADAPTER_HPP
