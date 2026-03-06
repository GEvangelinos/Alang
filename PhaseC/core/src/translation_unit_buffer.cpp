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
      data_(std::unique_ptr<char[]>(support::require_ptr(data))),
      source_size_(source_size),
      size_(source_size + null_padding),
      begin_(support::require_ptr(data_.get())),
      source_end_(data_.get() + source_size),
      end_([&]()
      {
          DEBUG_SMART_ASSERT(size_ != SrcBuffIdx::none());
          if (size_ == SrcBuffIdx::none())
              throw std::logic_error{"Bad initialization order in TranslationUnitBuffer detected"};
          return data_.get() + size_.value;
      }()) {}

std::size_t
TranslationUnitBuffer::compute_tub_size(
    const std::size_t source_size,
    const std::size_t padding) noexcept { return source_size + padding; }
} // namespace alpha
