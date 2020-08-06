#ifndef ASSERT_H
#define ASSERT_H

#ifdef __cplusplus
extern "C" {
#endif

extern void assert_failure(const char *file, unsigned line);

#define assert(cond) ((cond) ? (void)0 : assert_failure(__FILE__, __LINE__))

#ifdef __cplusplus
}
#endif

#endif /* ASSERT_H */
