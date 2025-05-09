#ifndef ALPHA_TRACE_LOGGER_HPP
#define ALPHA_TRACE_LOGGER_HPP

#include "utils/format_adapter.hpp"
#include <iostream>
#include <iomanip>
#include <vector>
#include "utils/cli_color.h"

extern bool g_show_parser_trace;

#if defined(OPTIMIZED_MODE) || defined(HATE_PYTHON_MODE)
#define display_trace(lhs, rhs) ((void)0)
#else
static void display_trace(const std::string &lhs, const std::string &rhs)
{
        if (!g_show_parser_trace)
                return;
        std::cout << COLOR_ASCII_BOLD_GREEN
                  << std::setw(20)
                  << lhs
                  << SGR_RESET
                  << " ⟶ "
                  << rhs
                  << std::endl;
}
#endif // OPTIMIZED_MODE
#endif // ALPHA_TRACE_LOGGER_HPP
