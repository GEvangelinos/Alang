#ifndef DEFAULT_CONSTANTS_HPP
#define DEFAULT_CONSTANTS_HPP

namespace Arguinator
{
    namespace FlagConsts
    {
        inline constexpr auto default_arity = 1;
        inline constexpr char default_help_text[] = "";
        inline constexpr auto default_required = false;

    }

    namespace ParserConsts
    {
        inline constexpr auto default_case_sensitive = true;
        inline constexpr char default_flag_prefix[] = "--";
        inline constexpr char default_help_flag[] = "help";
        inline constexpr char default_help_text[] = "Show this help message and exit";

        inline constexpr auto default_flag_field_size = 20;
    }

}

#endif /* DEFAULT_CONSTANTS_HPP */