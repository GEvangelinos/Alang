#ifndef SOURCE_LOCATION_TRACKER_HPP
#define SOURCE_LOCATION_TRACKER_HPP
#include "basics.hpp"
#include "source_location.hpp"
#include "source_location_types.hpp"

namespace alpha
{

class LocationTracker : private Immobile
{
public:
    OnceFlag lines_frozen;

    explicit LocationTracker(SrcBuffIdx max_valid_index);

    void append_line(SrcBuffIdx linestart_index);
    [[nodiscard]] SrcLineIdx find_first_line(SourceLocation loc) const;
    [[nodiscard]] SrcLineIdx find_last_line(SourceLocation loc) const;
    [[nodiscard]] SrcLineIdx find_symbol_line(SourceLocation loc) const;
    [[nodiscard]] SrcBuffIdx find_index_of_line(SrcLineIdx line) const;
    [[nodiscard]] SrcColumnIdx find_first_column(SourceLocation loc) const;
    [[nodiscard]] LineRange find_lines(SrcBuffIdx begin_idx, SrcBuffIdx end_idx) const;
    [[nodiscard]] LineRange find_lines(SourceLocation loc) const;
    [[nodiscard]] bool is_virtual_line(SrcLineIdx line) const noexcept;

private:
    const SrcBuffIdx max_valid_index_;
    std::vector<SrcBuffIdx> linestart_buffer_indices_;

    [[nodiscard]] SrcLineIdx eof_line() const noexcept;
    [[nodiscard]] SrcLineIdx find_line(SrcBuffIdx idx) const;
};
} // namespace alpha
#endif // SOURCE_LOCATION_TRACKER_HPP
