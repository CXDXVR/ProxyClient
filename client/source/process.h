#ifndef CLIENT_PROCESS_H_
#define CLIENT_PROCESS_H_

class Process
{
public:
	static constexpr DWORD G_INVALID_PROCESS_ID = -1;

	// Process default constructor.
	Process() = default;
	// Process default destructor.
	~Process() = default;
	// Deleted copy constructor.
	Process(const Process&)	= delete;
	// Deleted copy assigment.
	Process& operator=(const Process&)	= delete;

	// Opens specified process.
	// Throws runtime_error if opening failed.
	// @param _Pid - target process id.
	// @param _DesiredAccess - access to the target process.
	Process(_In_ DWORD _Pid, _In_ DWORD _DesiredAccess);

	// Opens specified process.
	// @param _Pid - target process id.
	// @param _DesiredAccess - access to the target process.
	// @returns true if opening success.
	bool Open(_In_ DWORD _Pid, _In_ DWORD _DesiredAccess);

	// Loads a module into a remote process.
	// @param _Module - full path to module to be loaded.
	// @return true if success, false if module alredy loaded or other error.
	bool RemoteLoadLibrary(_In_ const wchar_t*	_Module);

	// Returns true if process under WOW64.
	bool IsWow64() const noexcept;

	// Returns current process handle.
	inline const WinPipe::WinHandle& GetHandle() const& {
		return m_Process;
	}

	// Searches for a process by name and returns its ID.
	// @param _Name - process name.
	// @returns Process id or G_INVALID_PROCESS_ID if process not found.
	static DWORD GetProcessIdByName(_In_ const char* _Name);

	// Searches for a loaded module in a remote process.
	// @param _Process - target process id.
	// @param _Name - target module name.
	// @returns Pointer to module base or nullptr if module not found or other error.
	static HMODULE GetRemoteLibraryBase(_In_ DWORD _Process, _In_ const wchar_t* _Name);

private:
	WinPipe::WinHandle m_Process; // Process handle.
};

#endif // !CLIENT_PROCESS_H_