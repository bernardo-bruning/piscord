#ifndef TAP_H
#define TAP_H

/**
 * tap.h - A minimalist Test Anything Protocol (TAP) producer for C.
 *
 * This is free and unencumbered software released into the public domain.
 *
 * Anyone is free to copy, modify, publish, use, compile, sell, or
 * distribute this software, either in source code form or as a compiled
 * binary, for any purpose, commercial or non-commercial, and by any
 * means.
 *
 * In jurisdictions that recognize copyright laws, the author or authors
 * of this software dedicate any and all copyright interest in the
 * software to the public domain. We make this dedication for the benefit
 * of the public at large and to the detriment of our heirs and
 * successors. We intend this dedication to be an overt act of
 * relinquishment in perpetuity of all present and future rights to this
 * software under copyright law.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * For more information, please refer to <https://unlicense.org>
 *
 * Usage Example:
 *
 *     #include "tap.h"
 *
 *     int main() {
 *         plan(2);
 *         ok(1 == 1, "math works");
 *         ok(2 + 2 == 4, "more math");
 *         return done_testing();
 *     }
 *
 * API:
 *     void plan(int count)           - Set the number of tests to be run.
 *     void ok(int condition, fmt, ...) - Record a test result.
 *     void diag(fmt, ...)            - Output a diagnostic message (comment).
 *     int done_testing()             - End tests and return 1 if any failed, 0 otherwise.
 */

/*
 * TAP Dependency Injection
 */
#ifndef TAP_OUT
  #include <stdio.h>
  #define TAP_OUT printf
#endif

#ifndef TAP_PRINTF
  #include <stdio.h>
  /* Default TAP_PRINTF adds # for comments/diagnostics */
  #define TAP_PRINTF(fmt, ...) printf("# " fmt, ##__VA_ARGS__)
#endif

#ifndef TAP_VPRINTF
  #include <stdio.h>
  #define TAP_VPRINTF(fmt, args) vprintf(fmt, args)
#endif

#ifndef TAP_VA_LIST
  #include <stdarg.h>
  #define TAP_VA_LIST va_list
  #define TAP_VA_START va_start
  #define TAP_VA_END va_end
#endif

static int _tap_test_count = 0;
static int _tap_test_failed = 0;
static int _tap_plan_count = 0;

static void plan(int count) {
    _tap_plan_count = count;
    TAP_OUT("1..%d\n", count);
}

static void ok(int condition, const char *fmt, ...) {
    _tap_test_count++;
    TAP_VA_LIST args;
    TAP_VA_START(args, fmt);
    
    if (condition) {
        TAP_OUT("ok %d - ", _tap_test_count);
    } else {
        TAP_OUT("not ok %d - ", _tap_test_count);
        _tap_test_failed++;
    }
    
    TAP_VPRINTF(fmt, args);
    TAP_OUT("\n");
    TAP_VA_END(args);
}

static void diag(const char *fmt, ...) {
    TAP_VA_LIST args;
    TAP_VA_START(args, fmt);
    TAP_OUT("# ");
    TAP_VPRINTF(fmt, args);
    TAP_OUT("\n");
    TAP_VA_END(args);
}

static int done_testing() {
    if (_tap_plan_count == 0) {
        TAP_OUT("1..%d\n", _tap_test_count);
    }
    
    if (_tap_test_failed > 0) {
        diag("Failed %d tests out of %d", _tap_test_failed, _tap_test_count);
        return 1;
    }
    diag("All %d tests passed", _tap_test_count);
    return 0;
}

#endif /* TAP_H */
