#include "arguinator.hpp"
#include <iostream>

int main(int argc, char **argv)
{
    std::string header_description = "THIS is the HEADER description!";
    Arguinator::Parser parser(argc, argv, header_description, true);
    parser.set_flag("input-file").set_required().set_arity(2).set_help("Help text of input");
    parser.set_flag("output").set_help("Help with output");

    parser.parse_flags();

    std::cerr << "HAS? " << parser.found("input-file");

}