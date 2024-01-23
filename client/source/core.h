#ifndef CLIENT_CORE_H_
#define CLIENT_CORE_H_

// Abstract core class.
// The main task of the kernel is to start the server and inject the 
// proxy library into the target processes.
class Core : public AbstractCore
{
	static constexpr wchar_t PAYLOAD_NAME_[] = L"redirector.dll";

public:
	// Deleted default constructor.
	Core() = delete;
	// Default destructor.
	~Core();

	// Core constructor.
	// @param pids - target processess ids.
	// @param names - target processess names.
	// @param config - app base config.
	Core(const std::unordered_set<DWORD>& pids, const std::unordered_set<std::string>& names, const BaseConfigManager::Config& config);

	// Waiting for all sessions to be terminated.
	// @param timeout - time in milliseconds. default INFINITE.
	void Wait(_In_opt_ DWORD timeout = INFINITE) override {
		m_Server->Wait(timeout);
	}

	// Sends config update event.
	// @param config - new configuration.
	void UpdateConfig(const BaseConfigManager::Config& config) override {
		m_Server->UpdateConfig(config);
	}

	// Returns count of sessions.
	size_t GetSessionsCount() const override {
		return m_Server->CountOfSessions();
	}

private:
	// Returns a list of process identifiers by process name.
	// @param names - processes names.
	std::unordered_set<DWORD> GetPidsFromNames(_In_ const std::unordered_set<std::string>& names);

	// Injects the proxy module into the process list.
	// @param pids - processess ids.
	// @param config - app base config.
	// @returns list of injected processes ids.
	std::unordered_set<DWORD> InjectIntoProcesses(_In_ const std::unordered_set<DWORD>& pids, _In_ const BaseConfigManager::Config& config);

	// Injects the specified module into the specified process.
	// @param pid - target process id.
	// @param module - full path to module to be loaded.
	// @returns true if success.
	bool DoInject(_In_ DWORD pid, _In_ const std::wstring& module);

	// Returns full path to payload module.
	std::wstring GetPayloadFullPath();

	std::shared_ptr<AbstractServer> m_Server; // Server instance.
};

#endif // !CLIENT_CORE_H_
