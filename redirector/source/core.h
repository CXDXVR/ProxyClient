#ifndef REDIRECTOR_CORE_H_
#define REDIRECTOR_CORE_H_

// Library Core. Currently acts as an intermediary 
// between Config and SocketHook components
class Core final : public AbstractCore
{
public:
	// Deleted default constructor.
	Core() = delete;
	// Default destructor.
	~Core();
	// Deleted copy constructor.
	Core(const Core&)	= delete;
	// Deleted copy assigment.
	Core operator=(const Core&) = delete;
	
	// Core constructor.
	// @param instance - current module instance.
	Core(_In_ HMODULE instance, _In_ WinPipe::WinHandle& stopEvent);

	// Wait for termination.
	void Wait();

	// Notifying components of an event.
	// @param component - sender component.
	// @param event - event type.
	void Notify(AbstractComponent* component, Event event) override;

private:
	HMODULE															m_Instance;		// Current module instance.
	WinPipe::WinHandle&									m_StopEvent;	// Stop event.
	std::unique_ptr<BaseConfigManager>	m_Config;			// Instance of config component.
};

#endif // !REDIRECTOR_CORE_H_
