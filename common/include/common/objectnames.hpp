#ifndef COMMON_OBJECT_NAMES_H_
#define COMMON_OBJECT_NAMES_H_

namespace ObjectNames
{
	// Returns name of stop event.
	// @param id - event ID.
	inline std::wstring GetStopEventName(_In_ DWORD id) {
		return L"PROXY_CLIENT_STOP_" + std::to_wstring(id);
	}

	// Returns config pipe name.
	// @param id - pipe ID.
	inline std::wstring GetConfigPipeName(_In_ DWORD id) {
		return LR"(\\.\pipe\PROXY_CLIENT_CONFIG_)" + std::to_wstring(id);
	}

	// Returns report pipe name.
	// @param id - pipe ID.
	inline std::wstring GetReportPipeName(_In_ DWORD id) {
		return LR"(\\.\pipe\PROXY_CLIENT_REPORT_)" + std::to_wstring(id);
	}
}

#endif // !COMMON_OBJECT_NAMES_H_
