#ifndef REDIRECTOR_BASE_SOCKS_H_
#define REDIRECTOR_BASE_SOCKS_H_

class AbstractSocks
{
public:
	// Deleted default constructor.
	AbstractSocks() = delete;
	// Default virtual destructor.
	virtual ~AbstractSocks() = default;

	// AbstractSocks constructor.
	// @param config - app config.
	// @param socket - socket connected to the proxy server.
	// @param address - target app address.
	AbstractSocks(_In_ const BaseConfigManager::Config& config, _In_ SOCKET& socket, _In_ const sockaddr* address) :
		m_Config{ config },
		m_Socket{ socket },
		m_AddressProxy{ nullptr },
		m_AddressApp{ address }
	{ 
		if (m_Config.m_ProxyType == ProxyType::Socks4)
			m_AddressProxy = reinterpret_cast<const sockaddr*>(&m_Config.m_ProxyV4);
		else
		{
			if (BaseConfigManager::IsValidIPv4Address(config))
				m_AddressProxy = reinterpret_cast<const sockaddr*>(&m_Config.m_ProxyV4);
			else 
				m_AddressProxy = reinterpret_cast<const sockaddr*>(&m_Config.m_ProxyV6);
		}
	}
	
	// Sends request to socks server.
	// @returns true if success.
	virtual bool Request() = 0;

	// Returns proxy address.
	const sockaddr* GetProxyAddress() const noexcept {
		return m_AddressProxy;
	}

protected:
	const BaseConfigManager::Config&	m_Config;					// App configuration.
	SOCKET														m_Socket;					// Socket connected to the proxy server.
	const sockaddr*										m_AddressProxy;		// Proxy address.
	const sockaddr*										m_AddressApp;			// Target app address.
};

#endif // !REDIRECTOR_BASE_SOCKS_H_
