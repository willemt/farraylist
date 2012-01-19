#include <stdbool.h>
#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "CuTest.h"

#include "fixed_arraylist.h"

static int __uint_compare(
    const void *e1,
    const void *e2,
    const void *udata
)
{
    const int *i1 = e1;

    const int *i2 = e2;

    return *i2 - *i1;
}

void Testfixedarraylist_new(
    CuTest * tc
)
{
#if 0
    arraylistf_t *hp;

    hp = arraylistf_new(NULL, NULL);

    CuAssertTrue(tc, 0 == arraylistf_count(hp));
#endif
}
