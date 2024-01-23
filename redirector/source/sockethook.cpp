#include "global.h"

namespace
{
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	bool IsInet(_In_ const sockaddr* address) 
	{
		return address->sa_family == AF_INET || address->sa_family == AF_INET6;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	bool IsLocalHost(_In_ const sockaddr* address)
	{
		if (address->sa_family == AF_INET)
		{
			auto v4 = (const sockaddr_in*)address;
			return v4->sin_addr.s_net == 0x7f;
		}
		else if (address->sa_family == AF_INET6)
		{
			auto v6 = (const sockaddr_in6*)address;
			return	v6->sin6_addr.u.Word[0] == 0 && v6->sin6_addr.u.Word[1] == 0 &&
							v6->sin6_addr.u.Word[2] == 0 && v6->sin6_addr.u.Word[3] == 0 &&
							v6->sin6_addr.u.Word[4] == 0 && v6->sin6_addr.u.Word[5] == 0 &&
							v6->sin6_addr.u.Word[6] == 0 && v6->sin6_addr.u.Byte[14] == 0 &&
							v6->sin6_addr.u.Byte[15] == 1;
		}

		return false;
	}
	
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	bool IsAddressEquals(_In_ const sockaddr* address1, _In_ const sockaddr* address2) 
	{
		if (address1->sa_family == address2->sa_family)
		{
			if (address1->sa_family == AF_INET)
			{
				auto l4 = (const sockaddr_in*)address1;
				auto r4 = (const sockaddr_in*)address2;

				return	l4->sin_addr.S_un.S_addr == r4->sin_addr.S_un.S_addr &&
								l4->sin_port == r4->sin_port;
			}
			else if (address1->sa_family == AF_INET6)
			{
				auto l6 = (const sockaddr_in6*)address1;
				auto r6 = (const sockaddr_in6*)address2;

				return	l6->sin6_port == r6->sin6_port &&
								std::equal(l6->sin6_addr.u.Byte, l6->sin6_addr.u.Byte + 16, r6->sin6_addr.u.Byte);
			}
		}

		return false;
	}
}

std::atomic<size_t>			SocketHook::s_UsersCount;
std::mutex							SocketHook::s_UsersMutex;
std::condition_variable	SocketHook::s_UsersCondition;

std::unique_ptr<WinPipe::NamedPipeClient>	SocketHook::s_Pipe;
BaseConfigManager::Config									SocketHook::s_Config;
std::unordered_map<SOCKET, bool>					SocketHook::s_BlockIO;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool SocketHook::Initialize()
{
	if (s_HookConnect.CreateAndEnable()					!= MH_OK)	spdlog::warn("Failed to create hook connect function.");
	if (s_HookWSAConnect.CreateAndEnable()			!= MH_OK) spdlog::warn("Failed to create hook WSAConnect function.");
	if (s_HookIoctlsocket.CreateAndEnable()			!= MH_OK) spdlog::warn("Failed to create hook ioctlsocket function.");
	if (s_HookWSAAsyncSelect.CreateAndEnable()	!= MH_OK) spdlog::warn("Failed to create hook WSAAsyncSelect function.");
	if (s_HookWSAEventSelect.CreateAndEnable()	!= MH_OK) spdlog::warn("Failed to create hook WSAEventSelect function.");

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SocketHook::Uninitialize()
{
	std::unique_lock<std::mutex> usersLock(s_UsersMutex);

	s_HookWSAEventSelect.Disable();
	s_HookWSAAsyncSelect.Disable();
	s_HookIoctlsocket.Disable();
	s_HookWSAConnect.Disable();
	s_HookConnect.Disable();

	s_UsersCondition.wait(usersLock, [] { return s_UsersCount.load(std::memory_order_relaxed) == 0; });

	s_Pipe.reset();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SocketHook::UpdateConfig(_In_ const BaseConfigManager::Config& config, _In_ WinPipe::WinHandle& stopEvent)
{
	s_Config = config;

	// Creating report named pipe.
	if (s_Config.m_LoggingEnable) 
	{
		if (!s_Pipe.get()) s_Pipe = std::make_unique<WinPipe::NamedPipeClient>(stopEvent);

		if (!s_Pipe->IsOpen()) 
		{
			if (auto status = s_Pipe->Connect(ObjectNames::GetReportPipeName(GetCurrentProcessId())); status != ERROR_SUCCESS)
				spdlog::warn("Failed to create report named pipe. GetLastError={}", status);
		}
	}
	else if (!s_Config.m_LoggingEnable)
	{
		if (s_Pipe.get())
			s_Pipe->Close();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const sockaddr* SocketHook::GetProxyAddress(ADDRESS_FAMILY family)
{
	switch (family) 
	{
		case AF_INET:		return reinterpret_cast<const sockaddr*>(&s_Config.m_ProxyV4);
		case AF_INET6:	return reinterpret_cast<const sockaddr*>(&s_Config.m_ProxyV6);
	}

	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::unique_ptr<AbstractSocks> SocketHook::GetProxyInstance(_In_ SOCKET& socket, _In_ const sockaddr* address)
{
	switch (s_Config.m_ProxyType)
	{
		case ProxyType::Socks4: return std::make_unique<Socks4>(s_Config, socket, address);
		case ProxyType::Socks5: return std::make_unique<Socks5>(s_Config, socket, address);
	}

	return nullptr;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int WSAAPI SocketHook::hook_connect(SOCKET s, const sockaddr* name, int namelen)
{
	auto hookScope = UserHookScope(s_UsersCount, s_UsersCondition);

	if (IsInet(name) && !IsLocalHost(name) && BaseConfigManager::Validate(s_Config))
	{
		auto socks				= GetProxyInstance(s, name);
		auto socksAddress = socks->GetProxyAddress();

		// Sending report to named channel if logging is specified.
		if (s_Config.m_LoggingEnable && s_Pipe.get() && s_Pipe->IsOpen())
			s_Pipe->WriteMessage(name->sa_family, reinterpret_cast<const BYTE*>(name), namelen);

		// If the address of the target application is not equal to the
		// address of the proxy server, start proxying
		if (!IsAddressEquals(socksAddress, name))
		{
			auto socketScope = SocketLockScope(s);

			// Connecting to proxy server.
			if (auto status = s_HookConnect.s_Original(s, socksAddress, namelen); status != 0) 
			{
				shutdown(s, SD_BOTH);
				return status;
			}

			// Sending request to server.
			if (!socks->Request()) 
			{
				shutdown(s, SD_BOTH);
				return SOCKET_ERROR;
			}

			return 0;
		}
	}

	return s_HookConnect.s_Original(s, name, namelen);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int WSAAPI SocketHook::hook_WSAConnect(SOCKET s, const sockaddr* name, int namelen, LPWSABUF lpCallerData, LPWSABUF lpCalleeData, LPQOS lpSQOS, LPQOS lpGQOS)
{
	auto hookScope = UserHookScope(s_UsersCount, s_UsersCondition);

	if (IsInet(name) && !IsLocalHost(name) && BaseConfigManager::Validate(s_Config))
	{
		auto socks				= GetProxyInstance(s, name);
		auto socksAddress = socks->GetProxyAddress();

		// Sending report to named channel if logging is specified.
		if (s_Config.m_LoggingEnable && s_Pipe.get() && s_Pipe->IsOpen())
			s_Pipe->WriteMessage(name->sa_family, reinterpret_cast<const BYTE*>(name), namelen);

		// If the address of the target application is not equal to the
		// address of the proxy server, start proxying.
		if (!IsAddressEquals(socksAddress, name))
		{
			auto socketScope = SocketLockScope(s);

			// Connecting to proxy server.
			if (auto status = s_HookWSAConnect.s_Original(s, socksAddress, namelen, lpCallerData, lpCalleeData, lpSQOS, lpGQOS); status != 0) 
			{
				shutdown(s, SD_BOTH);
				return status;
			}

			// Sending request to server.
			if (!socks->Request()) 
			{
				shutdown(s, SD_BOTH);
				return SOCKET_ERROR;
			}

			return 0;
		}
	}

	return s_HookWSAConnect.s_Original(s, name, namelen, lpCallerData, lpCalleeData, lpSQOS, lpGQOS);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int WSAAPI SocketHook::hook_ioctlsocket(SOCKET s, long cmd, u_long* argp)
{
	if (cmd == FIONBIO)
		s_BlockIO[s] = *argp;

	return s_HookIoctlsocket.s_Original(s, cmd, argp);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int WSAAPI SocketHook::hook_WSAAsyncSelect(SOCKET s, HWND hWnd, u_int wMsg, long lEvent)
{
	s_BlockIO[s] = true;

	return s_HookWSAAsyncSelect.s_Original(s, hWnd, wMsg, lEvent);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int WSAAPI SocketHook::hook_WSAEventSelect(SOCKET s, WSAEVENT hEventObject, long lNetworkEvents)
{
	s_BlockIO[s] = true;

	return s_HookWSAEventSelect.s_Original(s, hEventObject, lNetworkEvents);
}
