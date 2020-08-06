#ifndef TEST_HELPER_H
#define TEST_HELPER_H

/* unit test helper functions */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void test_helper_set_cpu_in_isr(void);

void test_helper_reset_cpu_in_isr(void);

int test_helper_is_pendsv_interrupt_triggered(void);

void test_helper_reset_pendsv_trigger(void);

#ifdef __cplusplus
}
#endif

#endif /* TEST_HELPER_H */
