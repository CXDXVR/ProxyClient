#ifndef CLIENT_SERVER_H_
#define CLIENT_SERVER_H_

// Session Server. Implements functions for client interaction, 
// such as adding, deleting, and updating configurations.
class Server : public AbstractServer, public std::enable_shared_from_this<Server>
{
	// Default constructor.
	Server() = default;

public:
	// Default destructor.
	~Server() = default;

	// Creates and returns server instance.
	static std::shared_ptr<Server> Create() {
		return std::shared_ptr<Server>(new Server);
	}

	// Stops all active sessions.
	void Stop();

	// Adding a session.
	// @param session - session to be added.
	bool AddSession(_In_ std::shared_ptr<AbstractSession> session) override;

	// Deleting a session.
	// @param id - session ID to be deleted.
	void DeleteSession(_In_ DWORD id) override {
		m_Sessions.erase(id);
	}

	// Updating client configurations.
	void UpdateConfig(_In_ const BaseConfigManager::Config& config) override;

	// Waiting for all sessions to complete
	// @param timeout - waiting timeout. by default is INFINITE.
	void Wait(_In_opt_ DWORD timeout = INFINITE) override;

	// Returns true if the specified session exists.
	bool SessionExists(_In_ DWORD id) const override {
		return m_Sessions.find(id) != m_Sessions.end();
	}

	// Returns session.
	std::shared_ptr<AbstractSession> GetSession(_In_ DWORD id) override {
		return m_Sessions[id];
	}

	// Returns count of sessions.
	size_t CountOfSessions() const noexcept override {
		return m_Sessions.size();
	}

private:
	std::unordered_map<DWORD, std::shared_ptr<AbstractSession>> m_Sessions; // List sessions.
};

#endif // !CLIENT_SERVER_H_
