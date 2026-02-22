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
    const SrcBuffIdx source_size;
    const SrcBuffIdx size;

    TranslationUnitBuffer(char* data, std::size_t source_size, std::size_t null_padding);
    ~TranslationUnitBuffer() = default;

    [[nodiscard]] char operator[](SrcBuffIdx i) const noexcept;
    [[nodiscard]] char& operator[](SrcBuffIdx i) noexcept;
    [[nodiscard]] const char* address_at(SrcBuffIdx i) const noexcept;

    [[nodiscard]] char* data() noexcept { return data_.get(); }

    // We don't want entities having a const pointer or reference
    // to TUB to be able to access data without using SrcBufferIdx.
    // Only entities allowed to mutate TUB like Flex (lexer) can access arbitrarily, and that's
    // modifying lexer would be too much work :D
    const char* data() const noexcept = delete;

    [[nodiscard]] static std::size_t
    compute_tub_size(std::size_t source_size, std::size_t padding) noexcept;

private:
    std::unique_ptr<char[]> data_;
};


inline char
TranslationUnitBuffer::operator[](const SrcBuffIdx i) const noexcept
{
    DEBUG_SMART_ASSERT(!!data_, i < size);
    return data_[i.value];
}

inline char&
TranslationUnitBuffer::operator[](const SrcBuffIdx i) noexcept
{
    DEBUG_SMART_ASSERT(!!data_, i < size);
    return data_[i.value];
}

inline const char*
TranslationUnitBuffer::address_at(const SrcBuffIdx i) const noexcept
{
    DEBUG_SMART_ASSERT(!!data_, i < size);
    return data_.get() + i.value;
}
} // namespace alpha
#endif //TRANSLATION_UNIT_BUFFER_HPP
