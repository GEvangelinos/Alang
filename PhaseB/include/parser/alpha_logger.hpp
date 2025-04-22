#ifndef LOGGER_HPP
#define LOGGER_HPP

#include "utils/format_adapter.hpp"
#include <iostream>
#include <iomanip>
#include "utils/cli_color.h"

extern bool g_show_parser_trace;

#ifndef OPTIMIZED_MODE
static void display_trace(const std::string &lhs, const std::string &rhs)
{
        if (!g_show_parser_trace)
                return;
        std::cout << COLOR_ASCII_FG_GREEN
                  << std::setw(20)
                  << lhs
                  << SGR_RESET
                  << ":\t" << rhs
                  << std::endl;
}
#endif // OPTIMIZED_MODE

#endif /* LOGGER_HPP */