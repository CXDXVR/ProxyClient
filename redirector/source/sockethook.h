#ifndef REDIRECTOR_SOCKET_HOOK_H_
#define REDIRECTOR_SOCKET_HOOK_H_

class SocketHook
{
	// RAII over socket blocking.
	// Switches the socket to the blocking mode.
	struct SocketLockScope
	{
		SocketLockScope(SOCKET s) :
			m_Socket{ s }
		{
			u_long nb = FALSE;
			s_HookIoctlsocket.s_Original(m_Socket, FIONBIO, &nb);
		}

		~SocketLockScope()
		{
			u_long nb = FALSE;

			if (auto iter = s_BlockIO.find(m_Socket); iter != s_BlockIO.end())
				nb = (u_long)iter->second;

			s_HookIoctlsocket.s_Original(m_Socket, FIONBIO, &nb);
		}

		SocketLockScope(const SocketLockScope&) = delete;
		SocketLockScope(SocketLockScope&&) = delete;

	private:
		SOCKET m_Socket;
	};

	// Wrapper over the number of users of the hooked functions.
	struct UserHookScope
	{
	public:
		UserHookScope(std::atomic<size_t>& counter, std::condition_variable& cvCounter) :
			m_Counter{ counter },
			m_CVCounter{ cvCounter }
		{
			m_Counter.fetch_add(1, std::memory_order_relaxed);
		}

		~UserHookScope()
		{
			m_Counter.fetch_sub(1, std::memory_order_relaxed);
			m_CVCounter.notify_all();
		}

		UserHookScope(const UserHookScope&) = delete;
		UserHookScope(UserHookScope&&) = delete;

	private:
		std::atomic<size_t>& m_Counter;
		std::condition_variable& m_CVCounter;
	};

public:
	// Hooks initialization.
	// @returns true if success.
	static bool Initialize();

	// Hooks uninitialization.
	static void Uninitialize();

	// Updates the configuration.
	// Also creates a named report pipe if logging is specified.
	// @param config - config to update.
	// @param stopEvent - stop event for report named pipe.
	static void UpdateConfig(_In_ const BaseConfigManager::Config& config, _In_ WinPipe::WinHandle& stopEvent);

private:
	// Returns the address of the proxy server for the specified address type.
	// @param family - address family.
	static const sockaddr* GetProxyAddress(ADDRESS_FAMILY family);

	// Creates an instance of the proxy client
	// @param socket - socks socket.
	// @param address - target app address.
	static std::unique_ptr<AbstractSocks> GetProxyInstance(_In_ SOCKET& socket, _In_ const sockaddr* address);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// Hooks
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	static int WSAAPI hook_connect(SOCKET s, const sockaddr* name, int namelen);
	static int WSAAPI hook_WSAConnect(SOCKET s, const sockaddr* name, int namelen, LPWSABUF lpCallerData, LPWSABUF lpCalleeData, LPQOS lpSQOS, LPQOS lpGQOS);
	static int WSAAPI hook_ioctlsocket(SOCKET s, long cmd, u_long FAR* argp);
	static int WSAAPI hook_WSAAsyncSelect(SOCKET s, HWND hWnd, u_int wMsg, long lEvent);
	static int WSAAPI hook_WSAEventSelect(SOCKET s, WSAEVENT hEventObject, long lNetworkEvents);

	static MinHook::FunctionHook<connect, hook_connect>								s_HookConnect;
	static MinHook::FunctionHook<WSAConnect, hook_WSAConnect>					s_HookWSAConnect;
	static MinHook::FunctionHook<ioctlsocket, hook_ioctlsocket>				s_HookIoctlsocket;
	static MinHook::FunctionHook<WSAAsyncSelect, hook_WSAAsyncSelect> s_HookWSAAsyncSelect;
	static MinHook::FunctionHook<WSAEventSelect, hook_WSAEventSelect> s_HookWSAEventSelect;

	static std::atomic<size_t>												s_UsersCount;			// Count of users of hooked functions.
	static std::mutex																	s_UsersMutex;			// Locks of users of hooked functions.
	static std::condition_variable										s_UsersCondition;	// CV of users of hooked functions.

	static std::unique_ptr<WinPipe::NamedPipeClient>	s_Pipe;						// Report named pipe.
	static BaseConfigManager::Config									s_Config;					// App config.
	static std::unordered_map<SOCKET, bool>						s_BlockIO;				// List of block/unlock sockets.
};

#endif // !REDIRECTOR_SOCKET_HOOK_H_