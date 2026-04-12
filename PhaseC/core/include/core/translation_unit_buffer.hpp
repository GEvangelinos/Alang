#ifndef TRANSLATION_UNIT_BUFFER_HPP
#define TRANSLATION_UNIT_BUFFER_HPP

#include <cstddef>
#include <filesystem>

#include "core/source_location_types.hpp"
#include "support/debug_tools.hpp"

namespace alpha
{
class TranslationUnitBuffer
{
public:
    const SrcBuffIdx null_padding;


    TranslationUnitBuffer(char* data, std::size_t source_size, std::size_t null_padding);
    ~TranslationUnitBuffer() = default;

    [[nodiscard]] char operator[](SrcBuffIdx i) const noexcept;
    [[nodiscard]] char& operator[](SrcBuffIdx i) noexcept;
    [[nodiscard]] char* data() noexcept { return data_.get(); }
    [[nodiscard]] const char* data() const noexcept { return data_.get(); }
    [[nodiscard]] SrcBuffIdx source_size() const noexcept { return source_size_; }
    [[nodiscard]] SrcBuffIdx size() const noexcept { return size_; }
    [[nodiscard]] const char* begin() const noexcept { return data_.get(); }      // Inclusive
    [[nodiscard]] const char* source_end() const noexcept { return source_end_; } // Exclusive
    [[nodiscard]] const char* end() const noexcept { return end_; }               // Exclusive
    [[nodiscard]] bool is_in_source(const char* addr) const noexcept;
    [[nodiscard]] bool is_in_buffer(const char* addr) const noexcept;

    [[nodiscard]] static std::size_t
    compute_tub_size(std::size_t source_size, std::size_t padding) noexcept;

private:
    const std::unique_ptr<char[]> data_;
    const SrcBuffIdx source_size_;
    const SrcBuffIdx size_;
    const char* const begin_;
    const char* const source_end_;
    const char* const end_;
};


inline char
TranslationUnitBuffer::operator[](const SrcBuffIdx i) const noexcept
{
    DMASSERT(!!data_, i < size_);
    return data_[i.value];
}

inline char&
TranslationUnitBuffer::operator[](const SrcBuffIdx i) noexcept
{
    DMASSERT(!!data_, i < size_);
    return data_[i.value];
}

inline bool
TranslationUnitBuffer::is_in_source(const char* const addr) const noexcept
{
    return addr >= begin() && addr < source_end();
}

inline bool
TranslationUnitBuffer::is_in_buffer(const char* const addr) const noexcept
{
    return addr >= begin() && addr < end();
}
} // namespace alpha
#endif //TRANSLATION_UNIT_BUFFER_HPP
