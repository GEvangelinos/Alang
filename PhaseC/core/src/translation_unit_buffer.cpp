#include "core/translation_unit_buffer.hpp"
#include <fstream>

#include "support/misc_tools.hpp"

namespace alpha
{
TranslationUnitBuffer::TranslationUnitBuffer(
    char* const data,
    const std::size_t source_size,
    const std::size_t null_padding)
    : null_padding(null_padding),
      source_size(source_size),
      size(source_size + null_padding),
      data_(std::unique_ptr<char[]>(support::require_ptr(data))) {}

std::size_t
TranslationUnitBuffer::compute_tub_size(
    const std::size_t source_size,
    const std::size_t padding) noexcept { return source_size + padding; }
} // namespace alpha
