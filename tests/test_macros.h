#pragma once

#include <stdio.h>


#define __TEST_STRINGIFY_IMPL(x) #x
#define __TEST_STRINGIFY(x) __TEST_STRINGIFY_IMPL(x)


typedef struct {
    int num_total;
    int num_failed;
} test_suite_t;

typedef test_suite_t (*test_suite_run_t)(void);


#define FAIL() {\
    __current_test_failed = 1; \
    __test_suite->num_failed++; \
    printf("\e[1;31m => Assertion failed at %s:%d:\e[0m\r\n", __FILE__, __LINE__); \
    continue; \
}

#define EXPECT(expr) \
    if (!(expr)) { \
        printf("\e[1;31m => Expected '%s' to be true.\e[0m\r\n", __TEST_STRINGIFY(expr)); \
        FAIL(); \
    }

#define EXPECT_EQ(a, b) \
    if ((a) != (b)) { \
        printf("\e[1;31m => Expected '%s' to equal '%s'.\e[0m\r\n", __TEST_STRINGIFY(a), __TEST_STRINGIFY(b)); \
        FAIL(); \
    }

#define EXPECT_NE(a, b) \
    if ((a) == (b)) { \
        printf("\e[1;31m => Expected '%s' not to equal '%s'.\e[0m\r\n", __TEST_STRINGIFY(a), __TEST_STRINGIFY(b)); \
        FAIL(); \
    }

#define __TEST_DECLARE_TEST(name) for( \
    int __loop_break = 0, __current_test_failed = 0; \
    !__loop_break; \
    __loop_break = 1, __test_suite->num_total++, printf("  %s %s\r\n", __current_test_failed ? "\e[0;31m✗\e[0m" : "\e[0;32m✓\e[0m", (name)) \
)

#define __TEST_DECLARE_TEST_SUITE(label, name) \
    test_suite_t __##label##_wrapper(void); \
    void __##label##_impl(test_suite_t *__test_suite); \
    \
    const test_suite_run_t label = __##label##_wrapper; \
    \
    test_suite_t __##label##_wrapper(void) { \
        test_suite_t suite = { .num_total = 0, .num_failed = 0 }; \
        printf("\r\n%s\r\n", (name)); \
        __##label##_impl(&suite); \
        return suite; \
    } \
    void __##label##_impl(test_suite_t *__test_suite)


#define TEST(name) __TEST_DECLARE_TEST(name)
#define IT(name) __TEST_DECLARE_TEST(name)
#define DESCRIBE(label, name) __TEST_DECLARE_TEST_SUITE(label, name)


#define TEST_MAIN(...) \
    extern test_suite_run_t __VA_ARGS__; \
int main(int argc, const char *argv[]) { \
    const test_suite_run_t test_suites[] = { __VA_ARGS__ }; \
    \
    int suites_total = 0; \
    int suites_failed = 0; \
    int tests_total = 0; \
    int tests_failed = 0; \
    \
    for (int i = 0; i < sizeof(test_suites) / sizeof(test_suite_run_t); i++) { \
        const test_suite_t result = test_suites[i](); \
        suites_total++; \
        suites_failed += result.num_failed > 0 ? 1 : 0; \
        tests_total += result.num_total; \
        tests_failed += result.num_failed; \
    } \
    printf("\r\n"); \
    if (suites_failed == 0) { \
        printf("\e[1;30;42m PASSED \e[0m\r\n"); \
        printf("Test Suites: \e[1;32m%d passed\e[0m, %d total.\r\n", suites_total - suites_failed, suites_total); \
        printf("Tests:       \e[1;32m%d passed\e[0m, %d total.\r\n", tests_total - tests_failed, tests_total); \
        return 0; \
    } else { \
        printf("\e[1;30;41m FAILED \e[0m\r\n"); \
        printf("Test Suites: %d passed, \e[1;31m%d failed\e[0m, %d total.\r\n", suites_total - suites_failed, suites_failed, suites_total); \
        printf("Tests:       %d passed, \e[1;31m%d failed\e[0m, %d total.\r\n", tests_total - tests_failed, tests_failed, tests_total); \
        return 1; \
    } \
}
