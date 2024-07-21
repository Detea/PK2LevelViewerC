#ifndef LEVELVIEWER_UTIL_H
#define LEVELVIEWER_UTIL_H
#include <string.h>

/*
* Converts all characters in source to lower case and stores the result in target.
*
* target_len has to be specified, in case target is empty.
*/
void str_tolower(char* target, size_t target_len, const char* source);

#endif
