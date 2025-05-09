#include "core/alpha_error.hpp"
#include <algorithm> // for count
#include <cstring>   // for size_t, strchr, strlen
#include <sstream>   // for basic_stringstream, basic_ostream
#include <utility>   // for move
#include "core/alpha_macros.hpp"
#include "core/alpha_macros.hpp"
#include "core/alpha_location.hpp"  // for Location, LocationTracker
#include "utils/cli_color.h"        // for COLOR_ASCII_BOLD_DEFAULT, SGR_RESET
#include "utils/format_adapter.hpp" // for format, FMT
#include "utils/smart_assert.h"     // for DEBUG_SMART_ASSERT

namespace // (Anonymous)
{
        std::vector<std::string_view> extract_line_views(const char *buffer, std::size_t start_index, std::size_t end_index)
        {
                std::vector<std::string_view> lines;
                const char *start = buffer + start_index;
                const char *current = start;
                const char *end_target = buffer + end_index;

                while (*current)
                {
                        const char *line_end = std::strchr(current, '\n');
                        if (!line_end)
                                line_end = current + std::strlen(current); // to '\0' if no newline

                        lines.emplace_back(current, line_end - current); // view [current, line_end)

                        if (end_target <= line_end)
                                break; // stop if end_index is in this line

                        current = (*line_end == '\n') ? line_end + 1 : line_end;
                }
                return lines;
        }

        std::string expand_tabs(std::string_view line, int tab_width = 8)
        {
                std::string result;
                result.reserve(line.size() + std::count(line.begin(), line.end(), '\t') * (tab_width - 1));

                int col = 0;
                for (char ch : line)
                {
                        if (ch == '\t')
                        {
                                int spaces = tab_width - (col % tab_width);
                                result.append(spaces, ' ');
                                col += spaces;
                        }
                        else
                        {
                                result += ch;
                                ++col;
                        }
                }
                return result;
        }

        int compute_visual_caret_offset(std::string_view line, int raw_offset, int tab_width = 8)
        {
                int col = 0;
                for (int i = 0; i < raw_offset; ++i)
                {
                        col += (line[i] == '\t') ? (tab_width - (col % tab_width)) : 1;
                }
                return col;
        }

} // namespace (Anonymous)

namespace Alpha
{
        Diagnostic::Diagnostic(Diagnostic::Type type, const std::string &message, Location location)
            : type(type), message(message), location(location) {}

        u32 Diagnostic::line(const LocationTracker &lt) const
        {
                return lt.find_first_line(location);
        }

        u32 Diagnostic::column(const LocationTracker &lt) const
        {
                return lt.find_first_column(location);
        }

        std::string_view Diagnostic::type_to_string() const noexcept
        {
                switch (type)
                {
                case Type::ERROR:
                        return "error";
                case Type::NOTE:
                        return "note";
                }
                UNREACHABLE("Some field of Diagnostic::Type is not registred");
        }

        std::string_view Diagnostic::pretty_color() const noexcept
        {
                switch (type)
                {
                case Type::ERROR:
                        return COLOR_ASCII_BOLD_RED;
                case Type::NOTE:
                        return COLOR_ASCII_BOLD_CYAN;
                }
                UNREACHABLE("Some field of Diagnostic::Type is not registred");
        }

        std::string_view CTError::type_to_string() const noexcept
        {
                switch (type)
                {
                case Type::LEXER:
                        return "lexer";
                case Type::SEMANTIC:
                        return "semantic";
                case Type::SYNTAX:
                        return "syntax";
                }
                UNREACHABLE("Some field of CTError::Type is not registred");
        }

        std::string CTError::make_pretty_diagnostic(
            const std::string &source_filename,
            const LocationTracker &lt,
            const char *input_buffer) const
        {
                std::stringstream ss;
                ss << make_pretty_diagnostic_impl(source_filename, lt, input_buffer, error);

                for (auto note : note_list)
                        ss << make_pretty_diagnostic_impl(source_filename, lt, input_buffer, note);

                return ss.str();
        }

        CTError::CTError(CTError::Type error_type, const std::string &error_message, Location error_location)
            : type(error_type), error(Diagnostic::Type::ERROR, error_message, error_location) {}

        CTError::CTError(CTError::Type error_type, const std::string &error_message, Location error_location,
                         const std::string &note_message, Location note_location)
            : type(error_type),
              error(Diagnostic::Type::ERROR, error_message, error_location),
              note_list{{Diagnostic::Type::NOTE, note_message, note_location}} {}

        CTError::CTError(CTError::Type error_type, const std::string &error_message, Location error_location,
                         std::list<Diagnostic> &&note_list_)
            : type(error_type),
              error(Diagnostic::Type::ERROR, error_message, error_location),
              note_list(std::move(note_list_)) {}

        // TODO: Fix.. its ugly AF
        std::string CTError::make_pretty_diagnostic_impl(
            const std::string &source_filename,
            const LocationTracker &lt,
            const char *input_buffer,
            const Diagnostic &diagnostic) const
        {
                constexpr u32 line_box_width = 8;
                std::stringstream ss;

                /* Error header: */
                const u32 diagnostic_line = diagnostic.line(lt);
                const u32 diagnostic_column = diagnostic.column(lt);
                ss << COLOR_ASCII_BOLD_DEFAULT
                   << FMT::format(
                          "{}:{}:{}: {}{}{}: {}\n",
                          source_filename, diagnostic_line, diagnostic_column,
                          diagnostic.pretty_color(), diagnostic.type_to_string(), COLOR_ASCII_BOLD_DEFAULT,
                          diagnostic.message)
                   << SGR_RESET;

                auto line_views = extract_line_views(
                    input_buffer, lt.find_index_of_line(diagnostic_line), diagnostic.location.last_index);
                for (std::size_t i = 0; i < line_views.size(); i++)
                {
                        std::string visual_line = expand_tabs(line_views[i]);
                        ss << FMT::format("{:>{}} | {}\n",
                                          i != 0 ? "" : std::to_string(diagnostic_line),
                                          line_box_width,
                                          visual_line);
                        if (i != 0) // Caret marking is only for first line.
                                continue;

                        DEBUG_SMART_ASSERT(diagnostic.location.last_index > diagnostic.location.first_index);

                        auto raw_caret_offset =
                            diagnostic.location.first_index - lt.find_index_of_line(lt.find_first_line(diagnostic.location));
                        auto visual_caret_offset = compute_visual_caret_offset(line_views[i], raw_caret_offset);
                        auto highlight_length = diagnostic.location.last_index - diagnostic.location.first_index - 1;

                        ss << FMT::format(
                            "{} | {}{}^{}\n",
                            std::string(line_box_width, ' '),      // Spaces pre  |
                            std::string(visual_caret_offset, ' '), // spaces post | to move caret
                            diagnostic.pretty_color(),
                            std::string(highlight_length, '~'));
                        ss << SGR_RESET;
                }

                return ss.str();
        }

        void ErrorTracker::report_error(CTError::Type error_type, const std::string &error_message,
                                        Location error_location)
        {
                // new ptr is passed to unique_ptr for managing, do NOT manual delete.
                cterrors_.push_back(std::unique_ptr<const CTError>(
                    new CTError(error_type, error_message, error_location)));
        }

        void ErrorTracker::report_error(CTError::Type error_type, const std::string &error_message,
                                        Location error_location, const std::string &note, Location note_location)
        {
                // new ptr is passed to unique_ptr for managing, do NOT manual delete.
                cterrors_.push_back(std::unique_ptr<const CTError>(
                    new CTError(error_type, error_message, error_location, note, note_location)));
        }

        void ErrorTracker::report_error(CTError::Type error_type, const std::string &error_message,
                                        Location error_location, std::list<Diagnostic> &&note_list_)
        {
                // new ptr is passed to unique_ptr for managing, do NOT manual delete.
                cterrors_.push_back(std::unique_ptr<const CTError>(
                    new CTError(error_type, error_message, error_location, std::move(note_list_))));
        }
} // namespace Alpha
