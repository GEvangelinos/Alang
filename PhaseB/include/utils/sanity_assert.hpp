#ifndef SANITY_ASSERT_H
#define SANITY_ASSERT_H

#include "utils/format_adapter.hpp"
#include <iostream>
#include <cstring>

// clang-format off
#ifdef SANITY_MODE
        #ifndef __FILENAME__
        #define __FILENAME__ (std::strrchr(__FILE__, '/') ? std::strrchr(__FILE__, '/') + 1 : __FILE__)
        #endif // __FILENAME__

        // Used ZZ instead of G so it stays on bottom in autocomplete suggestions
        #define SANITY_ASSERT_ZZENERATOR(_assert_suffix, _rel_op, _value, _limit)                                           \
                do                                                                                                          \
                {                                                                                                           \
                        if ((_value)_rel_op(_limit))                                                                        \
                                break;                                                                                      \
                        const char *error_message = "SANITY_ASSERT_" #_assert_suffix "(" #_value ", " #_limit "): failed!"; \
                        std::cerr << fmt::format("{}:{}: {}(): {}. Reason ({} {} {}) is false.\n",                          \
                                                __FILENAME__, __LINE__, __func__,                                           \
                                                error_message, (_value), #_rel_op, (_limit))                                \
                                << std::endl;                                                                               \
                        abort();                                                                                            \
                } while (0)
#else
        #define SANITY_ASSERT_ZZENERATOR(_assert_suffix, _rel_op, _value, _limit) ((void)0)
#endif // SANITY_MODE

#define SANITY_ASSERT_LT(_value, _limit) \
        SANITY_ASSERT_ZZENERATOR(LT, <, _value, _limit)

#define SANITY_ASSERT_GT(_value, _limit) \
        SANITY_ASSERT_ZZENERATOR(GT, >, _value, _limit)

#define SANITY_ASSERT_LTE(_value, _limit) \
        SANITY_ASSERT_ZZENERATOR(LTE, <=, _value, _limit)

#define SANITY_ASSERT_GTE(_value, _limit) \
        SANITY_ASSERT_ZZENERATOR(GTE, >=, _value, _limit)

#define SANITY_ASSERT_NEQ(_val1, _val2) \
        SANITY_ASSERT_ZZENERATOR(NEQ, !=, _val1, _val2)

#define SANITY_ASSERT_EQ(_val1, _val2) \
        SANITY_ASSERT_ZZENERATOR(EQ, ==, _val1, _val2)

#define SANITY_ASSERT_TRUE(_condition) \
        SANITY_ASSERT_ZZENERATOR(TRUE, ==, _condition, true)

#define SANITY_ASSERT_FALSE(_condition) \
        SANITY_ASSERT_ZZENERATOR(FALSE, ==, _condition, false)

#ifdef SANITY_MODE
        #define SANITY_CODE(_code) _code
#else
        #define SANITY_CODE(_code) ((void)0)
#endif // SANITY_MODE

#endif // SANITY_ASSERT_H