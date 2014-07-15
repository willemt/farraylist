#include <stdbool.h>
#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "CuTest.h"

#include "fixed_arraylist.h"

#if 0
static int __uint_compare(
    const void *e1,
    const void *e2,
    const void *udata __attribute__((__unused__))
)
{
    const int *i1 = e1;

    const int *i2 = e2;

    return *i2 - *i1;
}
#endif

void Testfixedarraylist_new(
    CuTest * tc
)
{
    farraylist_t *hp;

    hp = farraylist_new(10);

    CuAssertTrue(tc, 0 == farraylist_count(hp));
}
