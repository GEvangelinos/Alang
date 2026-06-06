#ifndef STRING_CACHE_HPP_H
#define STRING_CACHE_HPP_H

#include <cstring>
#include <memory>
#include <vector>

#include "core/numeric_types.hpp"
#include "core/string_span.hpp"

namespace alpha::vm
{

class StringCache
{
public:
    std::vector<StringSpan> index_map;

    StringCache(const char* const copy_buffer_from, const std::size_t buffer_size)
        : buffer_(std::make_unique_for_overwrite<u8[]>(buffer_size))
    {
        std::memcpy(buffer_.get(), copy_buffer_from, buffer_size);
    }

    StringCache(StringCache&&) noexcept = default;
    StringCache(const StringCache&) = delete;



private:
    std::unique_ptr<u8[]> buffer_;
};

} // namespace alpha::vm
#endif //STRING_CACHE_HPP_H
