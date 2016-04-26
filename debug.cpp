#include "common.h"
#include "debug.h"

#ifdef __DEBUG__
void printfDebug(const char* fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vprintf(fmt, ap);
  va_end(ap);
}

void fprintfDebug(FILE* debugFile, const char* fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vfprintf(debugFile, fmt, ap);
  va_end(ap);
}
#else
void printfDebug(const char* fmt, ...)
{
}

void fprintfDebug(FILE* debugFile, const char* fmt, ...)
{
}
#endif
