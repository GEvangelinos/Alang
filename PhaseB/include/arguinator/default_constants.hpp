#ifndef DEFAULT_CONSTANTS_HPP
#define DEFAULT_CONSTANTS_HPP

namespace Arguinator
{
    namespace ArgConsts
    {
        inline constexpr auto default_arity = 1;
        inline constexpr auto default_help_text = "";
        inline constexpr auto default_required = false;

    }

    namespace ParserConsts
    {
        inline constexpr auto default_case_sensitive = false;
        inline constexpr auto default_argument_flag_prefix = "--";
        inline constexpr auto default_help_flag = "help";
        inline constexpr auto default_help_description = "Show this help message and exit";
    }

}

#endif /* DEFAULT_CONSTANTS_HPP */