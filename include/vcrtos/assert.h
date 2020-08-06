#ifndef VCRTOS_ASSERT_H
#define VCRTOS_ASSERT_H

#ifdef __cplusplus
extern "C" {
#endif

extern void vcassert_failure(const char *file, unsigned line);

#define vcassert(cond) ((cond) ? (void)0 : vcassert_failure(__FILE__, __LINE__))

#ifdef __cplusplus
}
#endif

#endif /* ASSERT_H */
