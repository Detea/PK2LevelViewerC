#include "util.h"
#include <ctype.h>
#include <SDL2/SDL_log.h>

void str_tolower(char* target, size_t target_len, const char* source) {
	int source_len = strlen(source);

	int len = source_len <= target_len ? source_len : target_len;

	for (int i = 0; i < len; ++i) {
		target[i] = tolower(source[i]);
	}
}
