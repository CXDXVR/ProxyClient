#ifndef CLIENT_BASE_SESSION_H_
#define CLIENT_BASE_SESSION_H_

class AbstractServer;

class AbstractSession
{
public:
	// AbstractSession constructor.
	// Throws runtime_error if the StopEvent event is not created.
	// @param id - session id.
	// @param server - server instance.
	// @param eventName - session stopEvent name.
	AbstractSession(_In_ DWORD id, _In_ std::weak_ptr<AbstractServer> server, _In_ const std::wstring& eventName) :
		m_Id{ id },
		m_Server{ server },
		m_StopEvent{ CreateEventW(nullptr, false, false, eventName.c_str()) }
	{
		if (!m_StopEvent.get())
			throw std::runtime_error("Failed to create termination event.");
	}

	// Default destructor.
	virtual ~AbstractSession() = default;

	// Stoppint client session.
	virtual void Stop() = 0;

	// Sends new config to client.
	virtual void UpdateConfig(const BaseConfigManager::Config& config) = 0;

	// Returns true if session are active.
	virtual bool IsActive() noexcept {
		return WaitForSingleObject(m_StopEvent.get(), 0) == WAIT_OBJECT_0;
	}

	// Returns session ID.
	DWORD GetId() const noexcept {
		return m_Id;
	}

	// Returns stop event handle.
	const WinPipe::WinHandle& GetStopEvent() const noexcept {
		return m_StopEvent;
	}

protected:
	DWORD													m_Id;					// Session ID.
	std::weak_ptr<AbstractServer>	m_Server;			// Server instance.
	WinPipe::WinHandle						m_StopEvent;	// Stop event handle.
};

#endif // !CLIENT_BASE_SESSION_H_
