#ifndef TRANSLATION_UNIT_BUFFER_LOADER_HPP
#define TRANSLATION_UNIT_BUFFER_LOADER_HPP

#include <cstddef>
#include <filesystem>

#include "core/translation_unit_buffer.hpp"

namespace alpha
{
class TranslationUnitBufferLoader
{
public:
    [[nodiscard]] static std::unique_ptr<TranslationUnitBuffer>
    load_tub(const std::filesystem::path& path, std::size_t null_padding);

private:
    [[nodiscard]] static std::ifstream open_source(const std::filesystem::path& path);
};

} // namespace alpha

#endif // TRANSLATION_UNIT_BUFFER_LOADER_HPP
