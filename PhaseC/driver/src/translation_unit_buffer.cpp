#include "../include/driver/translation_unit_buffer.hpp"
#include <fstream>
#include "core/konstants.hpp"
#include "driver/exception.hpp"

namespace alpha
{
TranslationUnitBuffer::TranslationUnitBuffer(
    const std::filesystem::path &path,
    const std::size_t null_padding)
    : null_padding(null_padding)
{
    std::ifstream ifs = open_source(path);
    const auto filesize = std::filesystem::file_size(path);
    const auto tub_size = filesize + null_padding;
    data_ = std::make_unique<char[]>(tub_size);
    size_ = tub_size;
    source_size_ = filesize;

    if (!ifs.read(data_.get(), filesize))
        throw alpha::exception::FileReadError(path.string());

    // Flex requires two NULL-bytes at the end of the buffer (End-Of-Buffer marker).
    for (auto i = filesize; i < tub_size; ++i)
        data_[i] = '\0';
}

std::ifstream
TranslationUnitBuffer::open_source(const std::filesystem::path &path)
{
    using FOMode = alpha::exception::FileOpenError::Mode;
    if (!std::filesystem::exists(path))
        throw alpha::exception::FileNotFoundError(path.string());
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
