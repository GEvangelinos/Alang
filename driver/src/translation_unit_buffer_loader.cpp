#include "driver/translation_unit_buffer_loader.hpp"
#include <fstream>
#include <unistd.h>

#include "core/konstants.hpp"
#include "driver/exception.hpp"

namespace alpha
{
std::unique_ptr<TranslationUnitBuffer>
TranslationUnitBufferLoader::load_tub(
    const std::filesystem::path& path,
    const std::size_t null_padding)
{
    std::ifstream ifs = open_source(path);
    const auto filesize = std::filesystem::file_size(path);
    const auto tub_size = TranslationUnitBuffer::compute_tub_size(filesize, null_padding);
    auto data = std::make_unique<char[]>(tub_size);
    if (!ifs.read(data.get(), filesize))
        throw alpha::exception::FileReadError(path.string());

    // Flex requires two NULL-bytes at the end of the buffer (End-Of-Buffer marker).
    for (auto i = filesize; i < tub_size; ++i)
        data[i] = '\0';

    DMASSERT(!!data);
    return std::make_unique<TranslationUnitBuffer>(data.release(), filesize, null_padding);
}

std::ifstream
TranslationUnitBufferLoader::open_source(const std::filesystem::path& path)
{
    using FOMode = alpha::exception::FileOpenError::Mode;
    if (!std::filesystem::exists(path))
        throw alpha::exception::FileNotFoundError(path.string());
    if (access(path.c_str(), R_OK) != 0)
        throw alpha::exception::FilePermissionError(path.string(), R_OK);
    if (std::filesystem::is_directory(path))
        throw alpha::exception::FileIsADirectoryError(path.string());
    if (!std::filesystem::is_regular_file(path))
        throw alpha::exception::FileNotRegularError(path.string());
    if (const auto filesize = std::filesystem::file_size(path); filesize > k_max_source_filesize)
        throw alpha::exception::FileTooLargeError(path.string(), filesize, k_max_source_filesize);
    std::ifstream ifs(path);
    if (!ifs)
        throw alpha::exception::FileOpenError(path.string(), FOMode::READ);
    return ifs;
}
} // namespace alpha
