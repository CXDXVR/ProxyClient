#ifndef REDIRECTOR_CONFIG_H_
#define REDIRECTOR_CONFIG_H_

// Application Configuration Component.
// Retrieves configuration from the server via named pipes.
class ConfigManager final : public BaseConfigManager, public AbstractComponent
{
public:
	// Deleter default constructor.
	ConfigManager() = delete;
	// Default destructor.
	~ConfigManager();
	// Deleted copy constructor.
	ConfigManager(const ConfigManager&) = delete;
	// Deleted copy assigment.
	ConfigManager& operator=(const ConfigManager&) = delete;

	// ConfigManager constructor.
	// @param mediator - pointer to core instance.
	// @param stopEvent - reference to stop event.
	ConfigManager(_In_ AbstractCore* mediator, _In_ WinPipe::WinHandle& stopEvent);

private:
	// The main flow of communication with the server.
	void CommunicationThread();

	WinPipe::NamedPipeClient	m_Pipe;		// Server named pipe.
	std::thread								m_Thread;	// Server communication thread.

};

#endif // !REDIRECTOR_CONFIG_H_
