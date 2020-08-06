#include <assert.h>
#include <stdio.h>

void assert_failure(const char *file, unsigned line)
{
    (void) file;
    (void) line;

    return;
}
