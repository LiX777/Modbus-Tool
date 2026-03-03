#include "common.h"
#ifdef NDEBUG

VOID  DbgPrint(ULONG level, LPCTSTR lpFormat, ...) {}

#else

VOID  DbgPrint(ULONG level, LPCTSTR lpFormat, ...)
{
	TCHAR TextBufferTmp[500] = _T("");

	if (level < DebugLevel)
	{
		va_list arglist;
		va_start(arglist, lpFormat);
		_vstprintf_s(TextBufferTmp, lpFormat, arglist);
		va_end(arglist);
		OutputDebugString(TextBufferTmp);
		OutputDebugString("\n");
	}
	return;
}

#endif