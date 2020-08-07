#include <stdio.h>
#include <vcrtos/assert.h>

void vcassert_failure(const char *file, unsigned line)
{
#ifdef UNITTEST
    printf("%s:%u => ASSERT FAILED\r\n", file, line);
#else
    (void) file;
    (void) line;
#endif
    while (1);
}
