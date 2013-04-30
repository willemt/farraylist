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
    arraylistf_t *hp;

    hp = arraylistf_new(10);

    CuAssertTrue(tc, 0 == arraylistf_count(hp));
}
