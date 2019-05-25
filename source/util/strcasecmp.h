/* So, as strcasecmp is not standard C, we're rolling our own implementation */

#ifndef __STRNCASECMP_INC
#define __STRNCASECMP_INC

#include <stddef.h>

int strcasecmp(const char* s1, const char* s2);
int strncasecmp(const char* s1, const char* s2, size_t n);

#endif