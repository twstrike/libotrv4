#ifndef OTRV4_STR_H
#define OTRV4_STR_H

#include <stddef.h>

#define string_t char *

/*@null@*/ char *otrv4_strndup(const char *s, size_t s_len);

char *otrv4_strdup(const char *s);

#endif
