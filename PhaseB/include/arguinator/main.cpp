#include "arguinator.hpp"
#include <iostream>

int main(int argc, char **argv)
{
    std::string header_description = "THIS is the HEADER description!";
    Arguinator::Parser parser(argc, argv, header_description, true);
    parser.set_argument("a").set_required().set_arity(1).set_help("Help text of a");
    parser.set_argument("input-file").set_required().set_arity(3).set_help("Help text of input");
    parser.set_argument("ouput").set_help("Help with output");
    parser.set_argument("log-fn").set_required().set_help("Name of the logging function to inject");

    std::cerr << parser.generate_help_text();
}