//
// Created by stygian on 8/22/25.
//

#ifndef DIAGNOSTIC_PRINTER_HPP
#define DIAGNOSTIC_PRINTER_HPP

std::vector<DiagnosticFormatter::Underline>
DiagnosticFormatter::build_underlines(const Issue &issue)
{
    // Find the first and last source line numbers that the offending issue spans
    const u32 first_line = loc_tracker_.find_first_line(issue.loc);
    const u32 last_line = loc_tracker_.find_last_line(issue.loc);

    // Prepare storage: one HighlightLine per affected line
    std::vector<Underline> underlines;
    underlines.reserve(last_line - first_line + 1);

    for (auto line_no = first_line; line_no <= last_line; ++line_no)
    {
        std::string marker;

        // Get the buffer index at which this line starts
        const u32 line_start = loc_tracker_.find_index_of_line(line_no);

        // Walk characters until newline, building a highlight string
        for (auto idx = line_start; source_buffer_[idx] != '\n'; ++idx)
        {
            const bool outside_issue = idx < issue.loc.first_index || idx >= issue.loc.last_index;
            const char ch = source_buffer_[idx];
            marker += outside_issue || std::isspace(static_cast<unsigned char>(ch))
                      ? ' '
                      : '~';
        }
        underlines.emplace_back(std::move(marker), line_no);
    }

    // Replace the first offending '~' with a caret '^' (this visually points to the start of the offensive code)
    for (auto &underline: underlines)
        if (const auto pos = underline.marker.find('~'); pos != std::string::npos)
        {
            underline.marker[pos] = '^';
            break; // only mark the very first offending character
        }
    return underlines;
}

#endif //DIAGNOSTIC_PRINTER_HPP
