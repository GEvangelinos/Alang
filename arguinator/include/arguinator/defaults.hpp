#ifndef DEFAULT_CONSTANTS_HPP
#define DEFAULT_CONSTANTS_HPP

namespace arguinator
{
namespace FlagConsts
{
static constexpr auto default_arity = 1;
static constexpr char default_help_text[] = "";
static constexpr auto default_required = false;

} // namespace FlagConsts

namespace ParserConsts
{
static constexpr auto default_case_sensitive = true;
static constexpr char default_flag_prefix[] = "--";
static constexpr char default_help_flag[] = "help";
static constexpr char default_help_text[] = "Show this help message and exit";

static constexpr auto default_flag_field_size = 20;
} // namespace ParserConsts

} // namespace Arguinator

#endif /* DEFAULT_CONSTANTS_HPP */