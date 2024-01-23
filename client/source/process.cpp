#include "global.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Process::Process(_In_ DWORD _Pid, _In_ DWORD _DesiredAccess)
{
	if (!Open(_Pid, _DesiredAccess))
		throw std::runtime_error("Failed to open process.");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool Process::Open(_In_ DWORD _Pid, _In_ DWORD _DesiredAccess)
{
	return (m_Process = WinPipe::WinHandle(OpenProcess(_DesiredAccess, false, _Pid))).get();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool Process::RemoteLoadLibrary(_In_ const wchar_t* _Module)
{
	// Allocate memory for the string containing the path to the module.
	auto moduleLength = lstrlenW(_Module);
	auto memory				= VirtualAllocEx(m_Process.get(), nullptr, (moduleLength + 1) * sizeof(wchar_t), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (!memory) return false;

	// Write a module path string to the process.
	if (!WriteProcessMemory(m_Process.get(), memory, _Module, moduleLength * sizeof(wchar_t), nullptr))
	{
		VirtualFreeEx(m_Process.get(), memory, 0, MEM_RELEASE);
		return false;
	}

	// Load the module into the process.
	auto thread = WinPipe::WinHandle(CreateRemoteThread(m_Process.get(), nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(LoadLibraryW), memory, 0, nullptr));
	if (!thread.get())
	{
		VirtualFreeEx(m_Process.get(), memory, 0, MEM_RELEASE);
		return false;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool Process::IsWow64() const noexcept
{
	typedef BOOL(WINAPI* T_IsWow64Process)(HANDLE, PBOOL);

	BOOL status		= false;
	auto function = GetProcAddress(GetModuleHandleA("kernel32.dll"), "IsWow64Process");

	if (function) reinterpret_cast<T_IsWow64Process>(function)(m_Process.get(), &status);

	return status;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DWORD Process::GetProcessIdByName(_In_ const char* _Name)
{
	auto snapshot = WinPipe::WinHandle(CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0));
	if (snapshot.get() == INVALID_HANDLE_VALUE)
		return G_INVALID_PROCESS_ID;

	PROCESSENTRY32 entry{ 0 };
	entry.dwSize = sizeof(PROCESSENTRY32);

	if (Process32First(snapshot.get(), &entry)) do
	{
		if (lstrcmpA(_Name, entry.szExeFile) == 0)
			return entry.th32ProcessID;
	} while (Process32Next(snapshot.get(), &entry));

	return G_INVALID_PROCESS_ID;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
HMODULE Process::GetRemoteLibraryBase(_In_ DWORD _Process, _In_ const wchar_t*	_Name)
{
	auto snapshot = WinPipe::WinHandle(CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, _Process));
	if (snapshot.get() == INVALID_HANDLE_VALUE)
		return nullptr;

	MODULEENTRY32W entry{ 0 };
	entry.dwSize = sizeof(MODULEENTRY32W);
	if (Module32FirstW(snapshot.get(), &entry)) do
	{
		if (lstrcmpW(_Name, entry.szModule) == 0)
			return reinterpret_cast<HMODULE>(entry.modBaseAddr);
	} while (Module32NextW(snapshot.get(), &entry));

	return nullptr;
}
