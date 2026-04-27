#include "globals.h"
#include <mutex>

#include "Logs/Logs.h"

namespace globals {
	DWORD_PTR gameBase = 0;
	DWORD_PTR gameWindow = 0;
	DWORD_PTR gameSize = 0;
	DWORD_PTR modBase = 0;
	DWORD_PTR modSize = 0;
	bool switchGameEA = false;
	bool exit_normal = false;
	bool pauseLogHook = false;
	bool isDemo = false;

	std::once_flag entrypoint_mutex;
	void ensure_console_allocated() {
		std::call_once(entrypoint_mutex, [] {
			AllocConsole();
			freopen("CONOUT$", "w", stdout);
			freopen("CONOUT$", "w", stderr);
			freopen("CONIN$", "w", stdin);
		});
	}
}

Vector3::Vector3(const Vector4& vec)
{
	X = vec.X;
	Y = vec.Y;
	Z = vec.Z;
}

Vector4::Vector4(const Vector3& vec)
{
	X = vec.X;
	Y = vec.Y;
	Z = vec.Z;
	W = 0;
}
