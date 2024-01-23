#ifndef CLIENT_BASE_CORE_H_
#define CLIENT_BASE_CORE_H_

// Abstract core class.
// The main task of the core is to start the server and add clients to it.
// It also implements methods to update the configuration
class AbstractCore
{
public:
	// Default virtual destructor.
	virtual ~AbstractCore() = default;
	// Waiting for the work to be completed.
	// @param timeout - time in milliseconds. default INFINITE.
	virtual void Wait(_In_opt_ DWORD timeout = INFINITE) = 0;
	// Sends config update event.
	// @param config - new configuration.
	virtual void UpdateConfig(_In_ const BaseConfigManager::Config& config) = 0;
	// Returns count of sessions.
	virtual size_t GetSessionsCount() const = 0;
};

#endif // !CLIENT_BASE_CORE_H_
