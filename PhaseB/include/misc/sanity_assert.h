#ifndef SANITY_ASSERT_H
#define SANITY_ASSERT_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define FMT_CAST(_value) ((long long)(_value))
#define FMT_SPEC "%lld"

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#ifdef SANITY_MODE
        // Used ZZ instead of G so it stays on bottom in autocomplete suggestions
        #define SANITY_ASSERT_ZZENERATOR(_assert_suffix, _rel_op, _value, _limit)                                                    \
                do                                                                                                                  \
                {                                                                                                                   \
                        if ((_value)_rel_op(_limit))                                                                                \
                                break;                                                                                              \
                        const char *error_message = "SANITY_ASSERT_" #_assert_suffix "(" #_value ", " #_limit "): failed!";         \
                        fprintf(stderr, "%s:%d: %s(): %s. Reason (" FMT_SPEC " %s " FMT_SPEC ") is false.\n",                       \
                                __FILENAME__, __LINE__, __func__, error_message, FMT_CAST((_value)), #_rel_op, FMT_CAST((_limit))); \
                        fflush(stderr);                                                                                             \
                        abort();                                                                                                    \
                } while (0)
#else
        #define SANITY_ASSERT_ZZENERATOR(_assert_suffix, _rel_op, _value, _limit)  ((void)0)
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

#define SANITY_ASSERT_TRUE(_value) \
        SANITY_ASSERT_ZZENERATOR(TRUE, ==, _value, 1)

#define SANITY_ASSERT_FALSE(_value) \
        SANITY_ASSERT_ZZENERATOR(FALSE, ==, _value, 0)

#endif // SANITY_ASSERT_H