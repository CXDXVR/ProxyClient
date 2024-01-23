#ifndef CLIENT_BASE_SERVER_H_
#define CLIENT_BASE_SERVER_H_

// Session Server. Implements functions for client interaction, 
// such as adding, deleting, and updating configurations.
class AbstractServer
{
public:
	virtual ~AbstractServer() = default;

	// Stops all active sessions.
	virtual void Stop() = 0;

	// Adding a session.
	// @param _Client - client to be added.
	virtual bool AddSession(_In_ std::shared_ptr<AbstractSession> session) = 0;

	// Deleting a session.
	// @param _Id - client ID to be deleted.
	virtual void DeleteSession(_In_ DWORD id) = 0;

	// Updating client configurations.
	virtual void UpdateConfig(_In_ const BaseConfigManager::Config& config) = 0;

	// Waiting for all sessions to complete
	// @param timeout - waiting timeout. by default is INFINITE.
	virtual void Wait(_In_opt_ DWORD timeout = INFINITE) = 0;

	// Returns true if the specified session exists.
	virtual bool SessionExists(_In_ DWORD id) const = 0;

	// Returns session.
	virtual std::shared_ptr<AbstractSession> GetSession(_In_ DWORD id) = 0;

	// Returns count of sessions.
	virtual size_t CountOfSessions() const noexcept = 0;

	// Returns true if list of session are empty.
	virtual bool Empty() const noexcept {
		return CountOfSessions() == 0;
	}
};

#endif // !CLIENT_BASE_SERVER_H_
