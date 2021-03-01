#include "stdlib-add.h"

size_t strnlen(const char *str, size_t len)
{
    for (size_t size = 0; size < len; size++)
    {
        if (str[size] == '\0')
            return size;
    }
    return len;
}
