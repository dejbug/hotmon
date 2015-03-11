#ifndef _debug_h_
#define _debug_h_

#include <stdarg.h>
#include <stdio.h>

void vprintb(char* buf, size_t size, const char* format, va_list list);
void debugf(const char* format, ...);


#endif //_debug_h_
