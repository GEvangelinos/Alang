#ifndef FORMAT_ADAPTER_HPP
#define FORMAT_ADAPTER_HPP

#ifdef STD_FORMAT_SUPPORTED
#include <format>
namespace fmt_ns = std;
#else
#define FMT_HEADER_ONLY
#include "third_party/fmt/format.h"
namespace fmt_ns = fmt;
#endif // STD_FORMAT_SUPPORTED

#endif // FORMAT_ADAPTER_HPP
