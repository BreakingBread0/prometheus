#pragma once

// TODO add SEH support on mingw
#ifdef _MSC_VER
#define __ow_try(pHandler) __try1(pHandler);
#define __ow_except __except1;
#else
#define __ow_try(pHandler);
#define __ow_except;
#endif
