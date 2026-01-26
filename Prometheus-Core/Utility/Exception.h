// Hazno - 2026

#pragma once

#include <string>
#include <windows.h>

#define MS_VC_EXCEPTION 0x406D1388
#define EXCEPTION_UNCAUGHT_CXX_EXCEPTION 0xE06D7363

namespace Utility::Exception
{
    PVECTORED_EXCEPTION_HANDLER GetExceptionHandler();

    const char* W32ErrorToString(DWORD errorCode);
    const char* CXXErrorToString(uintptr_t thrownPtr);
}