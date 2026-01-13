// Hazno - 2026

#pragma once

#include <windows.h>

#define MS_VC_EXCEPTION 0x406D1388

namespace Utility::Exception
{
    PVECTORED_EXCEPTION_HANDLER GetExceptionHandler();

    const char* W32ErrorToString(DWORD errorCode);
}