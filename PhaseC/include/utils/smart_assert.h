/*
 * smart_assert.h
 * ---------------------
 * Smart multi-condition assertion macro for C/C++
 * Author: Georgios Evangelinos (https://github.com/GEvangelinos)
 * License: MIT
 * Version: 1.0.0
 *
 * Supports C99 and C++20 or later (pre C++20 throws warnings)
 */

#ifndef SMART_ASSERT_H
#define SMART_ASSERT_H

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#ifdef __cplusplus
#define _Bool bool
#define _Static_assert static_assert
extern "C"
{
#endif /* __cplusplus*/

#ifndef __FILENAME__
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif /* __FILENAME__ */

        static inline int _get_nth_comma_position(const char *_string, int _target_comma_count)
        {
                int string_index = 0;
                int found_commas = 0;
                for (; (_string)[string_index] != '\0'; string_index++)
                {
                        if ((_string)[string_index] == ',')
                                found_commas++;
                        if (found_commas == (_target_comma_count))
                                break;
                }
                return (found_commas == _target_comma_count) ? string_index : 0;
        }

        static inline int _get_leading_spaces(const char *_string)
        {
                int offset = 0;
                while (((char *)(_string))[offset] == ' ')
                        offset++;
                return offset;
        }

#if defined(__cplusplus) && __cplusplus >= 202002L
#define _PRINT_SMART_ASSERT_FORMAT(_format, ...) \
        fprintf(stderr, _format __VA_OPT__(, ) __VA_ARGS__);
#elif defined(__GNUC__)
#define _PRINT_SMART_ASSERT_FORMAT(_format, ...) \
        fprintf(stderr, _format, ##__VA_ARGS__);
#else
#error SMART_ASSERT() is working with GNU compatible compilers, with C99 or new, and C++20 newer (older produces warnings.. but still works).
#endif

#define _PRINT_SMART_ASSERT(_format, ...)                                                           \
        do                                                                                          \
        {                                                                                           \
                fprintf(stderr, "%s:%d: %s(): SMART_ASSERT(): ", __FILENAME__, __LINE__, __func__); \
                _PRINT_SMART_ASSERT_FORMAT(_format, __VA_ARGS__); /* Compiler specific. */          \
                fprintf(stderr, "\n");                                                              \
        } while (0);

#define SMART_ASSERT(...)                                                                                                                                   \
        do                                                                                                                                                  \
        {                                                                                                                                                   \
                const char *assertion_text = #__VA_ARGS__;                                                                                                  \
                const _Bool assertions[] = {__VA_ARGS__};                                                                                                   \
                const int assertion_count = sizeof(assertions) / sizeof(assertions[0]);                                                                     \
                _Static_assert(sizeof(assertions) != 0, "SMART_ASSERT() empty parameter list. At least one condition required");                            \
                _Bool assert_failure = false;                                                                                                               \
                for (int assertion_index = 0; assertion_index < assertion_count; assertion_index++)                                                         \
                {                                                                                                                                           \
                        if (assertions[assertion_index] == true)                                                                                            \
                                continue;                                                                                                                   \
                        assert_failure = true;                                                                                                              \
                        if (assertion_count == 1)                                                                                                           \
                        {                                                                                                                                   \
                                _PRINT_SMART_ASSERT("Condition `%s` failed.", assertion_text);                                                              \
                                continue;                                                                                                                   \
                        }                                                                                                                                   \
                        if (assertion_index == 0) /* First in list. */                                                                                      \
                        {                                                                                                                                   \
                                int suffix_comma_position = _get_nth_comma_position(assertion_text, 1);                                                     \
                                _PRINT_SMART_ASSERT("Condition #%d: `%.*s` failed.", assertion_index + 1, suffix_comma_position, assertion_text);           \
                                continue;                                                                                                                   \
                        }                                                                                                                                   \
                        if (assertion_index == assertion_count - 1) /* Last in list. */                                                                     \
                        {                                                                                                                                   \
                                int prefix_comma_position = _get_nth_comma_position(assertion_text, assertion_index);                                       \
                                int leading_spaces = _get_leading_spaces(assertion_text + prefix_comma_position + 1);                                       \
                                const char *assertion_address = assertion_text + prefix_comma_position + 1 + leading_spaces; /* +1 to skip comma itself. */ \
                                _PRINT_SMART_ASSERT("Condition %d: `%s` failed.", assertion_index + 1, assertion_address)                                   \
                                continue;                                                                                                                   \
                        }                                                                                                                                   \
                        /*Not first not last... --> Intermidiate */                                                                                         \
                        int prefix_comma_position = _get_nth_comma_position(assertion_text, assertion_index);                                               \
                        int suffix_comma_position = _get_nth_comma_position(assertion_text, assertion_index + 1);                                           \
                        int leading_spaces = _get_leading_spaces(assertion_text + prefix_comma_position + 1);                                               \
                        int condition_string_size = suffix_comma_position - prefix_comma_position - 1 - leading_spaces;                                     \
                        const char *assertion_address = assertion_text + prefix_comma_position + 1 + leading_spaces;                                        \
                        _PRINT_SMART_ASSERT("Condition %d: `%.*s` failed.", assertion_index + 1, condition_string_size, assertion_address);                 \
                }                                                                                                                                           \
                if (assert_failure == true)                                                                                                                 \
                {                                                                                                                                           \
                        fflush(stderr);                                                                                                                     \
                        abort();                                                                                                                            \
                }                                                                                                                                           \
        } while (0)

#ifdef DEBUG_MODE
// ✅ Active in debug mode: evaluates and runs full SMART_ASSERT logic
#define DEBUG_SMART_ASSERT(...) SMART_ASSERT(__VA_ARGS__)
// ✅ Also active in debug mode: same as above, runs full logic
#define DEBUG_SMART_ASSERT_EVAL(...) SMART_ASSERT(__VA_ARGS__)
#else
// 🟡 In release mode: disables assertion logic, but still evaluates expressions (preserves side effects)
#define DEBUG_SMART_ASSERT_EVAL(...) ((void)(__VA_ARGS__))
// 🛑 In release mode: disables assertion and also skips evaluating expressions (no side effects)
#define DEBUG_SMART_ASSERT(...) ((void)0)
#endif /* DEBUG_MODE */

#ifdef __cplusplus
} /* Closing brace for extern "C" */
#endif /* __cplusplus */

#endif /* SMART_ASSERT_H */
