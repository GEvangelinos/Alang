#ifndef SCANNER_CONTEXT_HPP
#define SCANNER_CONTEXT_HPP

#include <string>
#include "core/alpha_location.hpp"
#include "core/alpha_types.hpp"

namespace Alpha
{
    struct ScannerContext
    {
        u32 line_;
        u32 column_;
        u32 index_;
        const std::string filename_;

        ScannerContext() = delete;
        ScannerContext(const std::string &filename)
            : line_(1),
              column_(1),
              index_(0),
              filename_(filename) {}
    };
}
#endif /* SCANNER_CONTEXT */