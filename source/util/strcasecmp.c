#include "strcasecmp.h"
#include <ctype.h>

int strcasecmp(const char* s1, const char* s2)
{
	for (;; s1++, s2++)
	{
		int d = tolower((unsigned char)*s1) - tolower((unsigned char)*s2);
        if (d != 0 || !*s2)
            return d;
	}
}

int strncasecmp(const char* s1, const char* s2, size_t n)
{
	size_t count = 0;
	for (;; s1++, s2++)
	{
		int d = tolower((unsigned char)*s1) - tolower((unsigned char)*s2);
		if (d != 0 || !*s2 || count++ >= n)
			return d;
	}
}