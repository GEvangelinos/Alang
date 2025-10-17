#ifndef TRANSLATION_UNIT_BUFFER_HPP
#define TRANSLATION_UNIT_BUFFER_HPP
#include <cstddef>
#include <filesystem>

#include "../../../core/include/core/source_location_types.hpp"

namespace alpha
{
class TranslationUnitBuffer
{
public:
    const std::size_t null_padding;

    explicit TranslationUnitBuffer(const std::filesystem::path &path, std::size_t null_padding);
    ~TranslationUnitBuffer() = default;

    [[nodiscard]] const char &operator[](const SrcBufferIdx i) const noexcept { return data_[i.value]; }
    [[nodiscard]] char &operator[](const SrcBufferIdx i) noexcept { return data_[i.value]; }

    [[nodiscard]] char *data() noexcept { return data_.get(); }

    // We don't want entities having a const pointer or reference
    // to TUB to be able to access data without using SrcBufferIdx.
    // Only entities allowed to mutate TUB like Flex (lexer) can access arbitrarily, and that's
    // modifying lexer would be too much work :D
    const char *data() const noexcept = delete;

    [[nodiscard]] std::size_t size() const noexcept { return size_; }
    [[nodiscard]] std::size_t source_size() const noexcept { return source_size_; }

private:
    std::unique_ptr<char[]> data_;
    std::size_t size_ = 0;
    std::size_t source_size_ = 0;

    [[nodiscard]] static std::ifstream open_source(const std::filesystem::path &path);
};
} // namespace alpha
#endif //TRANSLATION_UNIT_BUFFER_HPP
