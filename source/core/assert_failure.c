#include <stdio.h>
#include <vcrtos/assert.h>

void vcassert_failure(const char *file, unsigned line)
{
#ifdef UNITTEST
    printf("%s:%u => ASSERT FAILED\r\n", file, line);
#endif
    while (1);
}
