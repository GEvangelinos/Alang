#ifndef ESCAPE_CODE_LIST_HPP
#define ESCAPE_CODE_LIST_HPP

// X(char, escape)
#define ESCAPE_CODE_LIST(X) \
    X('n', '\n') \
    X('r', '\r') \
    X('t', '\t') \
    X('v', '\v') \
    X('f', '\f') \
    X('b', '\b') \
    X('\\','\\') \
    X('\"', '\"')

#endif //ESCAPE_CODE_LIST_HPP
