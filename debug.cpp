#include "debug.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>

#ifndef NDEBUG
void vprintb(char* buf, size_t size, const char* format, va_list list) {
	_vsnprintf(buf, size-1, format, list);
	buf[size-1] = '\0';
}

void debugf(const char* format, ...) {
	char buf[1024];
	va_list list;
	va_start(list, format);
	vprintb(buf, sizeof(buf), format, list);
	va_end(list);
	OutputDebugString(buf);
}
#else
void vprintb(char*, size_t, const char*, va_list) {
}

void debugf(const char*, ...) {
}
#endif
