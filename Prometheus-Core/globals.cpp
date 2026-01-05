#include "globals.h"
#include <mutex>
#include <Windows.h>

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
