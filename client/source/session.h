#ifndef CLIENT_SESSION_H_
#define CLIENT_SESSION_H_

class Session : public AbstractSession, public std::enable_shared_from_this<Session>
{
	// Session constructor.
	// Throws runtime_error if the StopEvent event is not created.
	// @param id - session id.
	// @param server - server instance.
	Session(_In_ DWORD id, _In_ std::weak_ptr<AbstractServer> server);

public:
	// Deleted default constructor.
	Session() = delete;
	// Default destructor.
	~Session() = default;

	// Returns instance of session class.
	// Throws runtime_error if the StopEvent event is not created.
	// @param id - session id.
	// @param server - server instance.
	static std::shared_ptr<Session> Create(_In_ DWORD id, _In_ std::weak_ptr<AbstractServer> server) {
		return std::shared_ptr<Session>(new Session(id, server));
	}

	// Stopping client session.
	void Stop() override;

	// Sends new config to client.
	void UpdateConfig(const BaseConfigManager::Config& config) override;

private:
	// Report thread routine.
	void ReportThread();

	WinPipe::WinError SendConfig(const BaseConfigManager::Config& config);

	WinPipe::NamedPipeServer									m_PipeConfig;
	std::unique_ptr<WinPipe::NamedPipeServer> m_PipeReport;
	std::thread																m_ReportThread;
};

#endif // !CLIENT_SESSION_H_
