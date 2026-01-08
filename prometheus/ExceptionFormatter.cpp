#include <windows.h>
#include <string>
#include <sstream>
#include <psapi.h>
#include "game.h"

namespace ExceptionFormatter {
	std::string formatPtr(DWORD64 value) {
		char data[20] = { 0 };
		snprintf(data, 20, "%p", value);
		return std::string(data);
	}

	inline void dumpExceptionRecord(std::stringstream& out, PEXCEPTION_RECORD ctx) {
		
		std::string exceptionCode;
		switch (ctx->ExceptionCode) {
		case EXCEPTION_ACCESS_VIOLATION:
			exceptionCode = "EXCEPTION_ACCESS_VIOLATION";
			break;
		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
			exceptionCode = "EXCEPTION_ARRAY_BOUNDS_EXCEEDED";
			break;
		case EXCEPTION_BREAKPOINT:
			exceptionCode = "EXCEPTION_BREAKPOINT";
			break;
		case EXCEPTION_DATATYPE_MISALIGNMENT:
			exceptionCode = "EXCEPTION_DATATYPE_MISALIGNMENT";
			break;
		case EXCEPTION_FLT_DENORMAL_OPERAND:
			exceptionCode = "EXCEPTION_FLT_DENORMAL_OPERAND";
			break;
		case EXCEPTION_FLT_DIVIDE_BY_ZERO:
			exceptionCode = "EXCEPTION_FLT_DIVIDE_BY_ZERO";
			break;
		case EXCEPTION_FLT_INEXACT_RESULT:
			exceptionCode = "EXCEPTION_FLT_INEXACT_RESULT";
			break;
		case EXCEPTION_FLT_INVALID_OPERATION:
			exceptionCode = "EXCEPTION_FLT_INVALID_OPERATION";
			break;
		case EXCEPTION_FLT_OVERFLOW:
			exceptionCode = "EXCEPTION_FLT_OVERFLOW";
			break;
		case EXCEPTION_FLT_STACK_CHECK:
			exceptionCode = "EXCEPTION_FLT_STACK_CHECK";
			break;
		case EXCEPTION_FLT_UNDERFLOW:
			exceptionCode = "EXCEPTION_FLT_UNDERFLOW";
			break;
		case EXCEPTION_ILLEGAL_INSTRUCTION:
			exceptionCode = "EXCEPTION_ILLEGAL_INSTRUCTION";
			break;
		case EXCEPTION_IN_PAGE_ERROR:
			exceptionCode = "EXCEPTION_IN_PAGE_ERROR";
			break;
		case EXCEPTION_INT_DIVIDE_BY_ZERO:
			exceptionCode = "EXCEPTION_INT_DIVIDE_BY_ZERO";
			break;
		case EXCEPTION_INT_OVERFLOW:
			exceptionCode = "EXCEPTION_INT_OVERFLOW";
			break;
		case EXCEPTION_INVALID_DISPOSITION:
			exceptionCode = "EXCEPTION_INVALID_DISPOSITION";
			break;
		case EXCEPTION_NONCONTINUABLE_EXCEPTION:
			exceptionCode = "EXCEPTION_NONCONTINUABLE_EXCEPTION";
			break;
		case EXCEPTION_PRIV_INSTRUCTION:
			exceptionCode = "EXCEPTION_PRIV_INSTRUCTION";
			break;
		case EXCEPTION_SINGLE_STEP:
			exceptionCode = "EXCEPTION_SINGLE_STEP";
			break;
		case EXCEPTION_STACK_OVERFLOW:
			exceptionCode = "EXCEPTION_STACK_OVERFLOW";
			break;
		}
		out << "  ExceptionCode: " << exceptionCode << " (0x" << std::hex << ctx->ExceptionCode << std::dec << ")" << std::endl;
		out << "  ExceptionFlags: " << ctx->ExceptionFlags << (ctx->ExceptionFlags & EXCEPTION_NONCONTINUABLE ? " (EXCEPTION_NONCONTINUABLE)" : "") << std::endl;
		std::string moduleName = "";
		DWORD64 moduleRel = 0;

		/*HMODULE resultArr[64];
		DWORD bytesNeeded = 0;
		if (!EnumProcessModules(GetCurrentProcess(), resultArr, 64, &bytesNeeded)) {
			out << "Failed to EnumProcessModules " << GetLastError() << std::endl;
		}
		for (int i = 0; i < bytesNeeded / sizeof(HMODULE); i++) {
			_MODULEINFO info{};
			GetModuleInformation(GetCurrentProcess(), resultArr[i], &info, sizeof(MODULEINFO));
			printf("%p %p %p\n", info.lpBaseOfDll, info.SizeOfImage, info.EntryPoint);
			if (ctx->ExceptionAddress > info.EntryPoint && (DWORD64)ctx->ExceptionAddress < ((DWORD64)info.EntryPoint + info.SizeOfImage)) {
				char szModName[MAX_PATH];
				GetModuleFileNameExA(GetCurrentProcess(), resultArr[i], szModName, sizeof(szModName));
				moduleName = szModName;
				moduleRel = (DWORD64)ctx->ExceptionAddress - (DWORD64)info.lpBaseOfDll;
				break;
			}
		}*/

		out << "  ExceptionAddress: 0x" << std::hex << ctx->ExceptionAddress << " (" << ctx->ExceptionAddress << ", " << moduleName << " rel " << formatPtr(moduleRel) << std::dec << ")" << std::endl;
		out << "  NumverParameters: " << ctx->NumberParameters << std::endl;
		for (int i = 0; i < ctx->NumberParameters; i++) {
			out << "  " << i << ": " << std::hex << ctx->ExceptionInformation[i] << std::dec << " (" << ctx->ExceptionInformation[i] << ")" << std::endl;
		}
		if (ctx->ExceptionRecord) {
			out << "  Another Exception Record: " << std::endl;
			dumpExceptionRecord(out, ctx->ExceptionRecord);
		}

		printf("Stack trace:\n");
	}

	std::string FormatException(_EXCEPTION_POINTERS* ex) {
		std::stringstream out;
		dumpExceptionRecord(out, ex->ExceptionRecord);
		auto ctx = ex->ContextRecord;

		out << "  Dr0: " << formatPtr(ctx->Dr0) << " Dr3: " << formatPtr(ctx->Dr3) << std::endl;
		out << "  Dr1: " << formatPtr(ctx->Dr1) << " Dr6: " << formatPtr(ctx->Dr6) << std::endl;
		out << "  Dr2: " << formatPtr(ctx->Dr2) << " Dr7: " << formatPtr(ctx->Dr7) << std::endl;


		out << "  RAX: " << formatPtr(ctx->Rax) << " RBP: " << formatPtr(ctx->Rbp) << " R10: " << formatPtr(ctx->R10) << std::endl;
		out << "  RCX: " << formatPtr(ctx->Rcx) << " RSI: " << formatPtr(ctx->Rsi) << " R11: " << formatPtr(ctx->R11) << std::endl;
		out << "  RDX: " << formatPtr(ctx->Rdx) << " RDI: " << formatPtr(ctx->Rdi) << " R12: " << formatPtr(ctx->R12) << std::endl;
		out << "  RBX: " << formatPtr(ctx->Rbx) << " R8:  " << formatPtr(ctx->R8) << " R13: " << formatPtr(ctx->R13) << std::endl;
		out << "  RSP: " << formatPtr(ctx->Rsp) << " R9:  " << formatPtr(ctx->R9) << " R14: " << formatPtr(ctx->R14) << std::endl;
		out << "  R15: " << formatPtr(ctx->R15) << " RIP: " << formatPtr(ctx->Rip);
		out << std::endl << stacktrace_str();

		return out.str();
	}
}
