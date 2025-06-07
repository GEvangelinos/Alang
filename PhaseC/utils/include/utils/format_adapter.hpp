#ifndef FORMAT_ADAPTER_HPP
#define FORMAT_ADAPTER_HPP

#ifdef STD_FORMAT_SUPPORTED
#include <format>
namespace FMT = std;
#else
#define FMT_HEADER_ONLY
#include "third_party/fmt/include/fmt/format.h" // TODO: fix too long...
namespace FMT = fmt;
#endif // STD_FORMAT_SUPPORTED

#endif // FORMAT_ADAPTER_HPP
