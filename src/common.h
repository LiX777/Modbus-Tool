#pragma once
#include <iostream>
#include <Windows.h>
#include <stdio.h>      // 提供 sprintf_s 等格式化函数
#include <tchar.h>      // 提供 _T, _stprintf_s 等TCHAR宏和函数
#define DebugLevel 3
VOID  DbgPrint(ULONG level, LPCTSTR lpFormat, ...);