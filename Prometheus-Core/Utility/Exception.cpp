// Hazno - 2026

#include "Exception.h"

#include <tlhelp32.h>
#include <algorithm>
#include <winternl.h>

#include "ExceptionFormatter.h"
#include "Logs/Logs.h"

namespace Utility::Exception
{
    namespace
    {
        int __stdcall VectoredExceptionHandler(EXCEPTION_POINTERS* exception)
        {
            if (exception->ExceptionRecord->ExceptionCode == EXCEPTION_BREAKPOINT ||
                exception->ExceptionRecord->ExceptionCode == EXCEPTION_SINGLE_STEP ||
                exception->ExceptionRecord->ExceptionCode == EXCEPTION_GUARD_PAGE ||
                exception->ExceptionRecord->ExceptionCode == MS_VC_EXCEPTION)
                return EXCEPTION_CONTINUE_SEARCH;
            if (exception->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION) {
                if (exception->ContextRecord->Rip > reinterpret_cast<DWORD64>(std::addressof(IsBadReadPtr)) && exception->ContextRecord->Rip - reinterpret_cast<DWORD64>(std::addressof(IsBadReadPtr)) < 0x100)
                    return EXCEPTION_CONTINUE_SEARCH;
            }
            if (exception->ExceptionRecord->ExceptionCode == 0x40010006)
                return EXCEPTION_CONTINUE_SEARCH;
            if (exception->ExceptionRecord->ExceptionCode == 0xc0000008)
                return EXCEPTION_CONTINUE_EXECUTION;
            LOG_CORE_e(exception);
            return EXCEPTION_CONTINUE_SEARCH;
        }
    }

    PVECTORED_EXCEPTION_HANDLER GetExceptionHandler()
    {
        return reinterpret_cast<PVECTORED_EXCEPTION_HANDLER>(VectoredExceptionHandler);
    }

    const char* W32ErrorToString(const DWORD errorCode)
    {
        switch (errorCode) {
            case EXCEPTION_ACCESS_VIOLATION:            return "Access violation. Tried to read or write to a virtual address without access.";
            case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:       return "Array access out of bounds.";
            case EXCEPTION_BREAKPOINT:                  return "Breakpoint reached.";
            case EXCEPTION_DATATYPE_MISALIGNMENT:       return "Tried to read or write data that is misaligned on hardware that does not provide alignment.";
            case EXCEPTION_FLT_DENORMAL_OPERAND:        return "Floating point operand is denormal.";
            case EXCEPTION_FLT_DIVIDE_BY_ZERO:          return "Floating point division by zero.";
            case EXCEPTION_FLT_INEXACT_RESULT:          return "Floating point inaccurate result.";
            case EXCEPTION_FLT_INVALID_OPERATION:       return "Floating point invalid operation.";
            case EXCEPTION_FLT_OVERFLOW:                return "Floating point overflow. Exponent is greater than the magnitude allowed by the corresponding type.";
            case EXCEPTION_FLT_STACK_CHECK:             return "Floating point operation caused a stack overflow/underflow";
            case EXCEPTION_FLT_UNDERFLOW:               return "Floating point underflow. Exponent is less than the magnitude allowed by the corresponding type.";
            case EXCEPTION_ILLEGAL_INSTRUCTION:         return "Attempted to execute an illegal instruction.";
            case EXCEPTION_IN_PAGE_ERROR:               return "Tried to access a page that was not present, and the system was unable to load the page.";
            case EXCEPTION_INT_DIVIDE_BY_ZERO:          return "Int divided by zero.";
            case EXCEPTION_INT_OVERFLOW:                return "Int overflow.";
            case EXCEPTION_INVALID_DISPOSITION:         return "Exception handler sent an invalid disposition to the exception dispatcher.";
            case EXCEPTION_INVALID_HANDLE:              return "Invalid handle.";
            case EXCEPTION_NONCONTINUABLE_EXCEPTION:    return "Tried to continue execution after a noncontinuable exception occurred.";
            case EXCEPTION_PRIV_INSTRUCTION:            return "Tried to execute an instruction whose operation is not allowed in the current machine mode.";
            case EXCEPTION_GUARD_PAGE:                  return "Attempted guard page access.";
            case EXCEPTION_SINGLE_STEP:                 return "Single debug step. One instruction has been executed.";
            case EXCEPTION_STACK_OVERFLOW:              return "Stack overflow.";
            case MS_VC_EXCEPTION:                       return "Caller raised to set a thread name on the Visual Studio debugger.";
            default:                                    return "Unknown exception code.";
        }
    }
}

