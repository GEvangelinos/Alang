#include "arguinator.hpp"

int main(int argc, char ** argv)
{
    Parser a = Parser(argc, argv, "DddESCR", true);
    a.add_argument("33 ", "dd");
}