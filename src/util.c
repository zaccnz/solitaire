#include "util.h"

#include <stdlib.h>

int ntlen(void **array)
{
    int i = 0;
    while (array[i] != NULL)
    {
        i++;
    }
    return i;
}