#pragma once
#include <windows.h>
#include <string>

namespace ExceptionFormatter {
	std::string FormatException(_EXCEPTION_POINTERS* ex);
}