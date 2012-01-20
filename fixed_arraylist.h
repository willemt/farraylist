
typedef struct
{
    int arraySize;
    void *in;
    void **array;
} arraylistf_t;


arraylistf_t *arraylistf_new(
);

void arraylistf_insert(
    arraylistf_t * alist,
    void *item,
    const int idx
);

int arraylistf_get_arraysize(
    arraylistf_t * alist
);

void *arraylistf_get(
    arraylistf_t * alist,
    const int idx
);

void *arraylistf_remove(
    arraylistf_t * alist,
    const int idx
);

int arraylistf_add(
    arraylistf_t * alist,
    void *item
);

void arraylistf_clear(
    arraylistf_t * alist
);

void arraylistf_free(
    arraylistf_t * alist
);

int arraylistf_count(
    arraylistf_t * alist
);

#if 0
int arraylistf_contains(
    arraylistf_t * alist,
    const void *key
);

void *arraylistf_get_item(
    arraylistf_t * alist,
    const void *key
);


void *arraylistf_remove_Item(
    arraylistf_t * alist,
    const void *key
);

int arraylistf_item_index(
    arraylistf_t * alist,
    const void *key
);
#endif
