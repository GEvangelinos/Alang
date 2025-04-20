#ifndef LOGGER_HPP
#define LOGGER_HPP

#include "utils/format_adapter.hpp"
#include <iostream>
#include <iomanip>
#include "utils/cli_color.h"

static void display_log(const std::string &lhs, const std::string &rhs)
{
        std::cout << COLOR_ASCII_FG_GREEN
                  << std::setw(20)
                  << lhs
                  << SGR_RESET
                  << ":\t" << rhs
                  << std::endl;
}

#endif /* LOGGER_HPP */