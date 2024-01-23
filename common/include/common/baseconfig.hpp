#ifndef COMMON_BASE_CONFIG_H_
#define COMMON_BASE_CONFIG_H_

enum class ProxyType : uint8_t
{
	Unknown,
	Socks4,
	Socks5
};

// Application Configuration Base Class.
// It implements methods for checking the correctness of the configuration.
class BaseConfigManager
{
public:
#	pragma pack(push)
#	pragma pack(1)
	// Configuration data.
	struct Config
	{
		ProxyType			m_ProxyType;			// Proxy type.
		sockaddr_in		m_ProxyV4;				// IPv4 address of proxy server.
		sockaddr_in6	m_ProxyV6;				// IPv6 address of proxy server.
		bool					m_LoggingEnable;	// true - enable client logging.
	};
#	pragma pack(pop)

	// BaseConfigManager constructor.
	// @param proxyType - proxy type. by default is unknown.
	// @param proxyAddress - proxy address. by default is 0.
	// @param proxyPort - proxy port. by default is 0.
	// @param logginEnable - true - enable logging, false - disable. by default false.
	BaseConfigManager(_In_opt_ ProxyType proxyType = ProxyType::Unknown, _In_opt_ sockaddr_in proxyV4 = { 0 }, _In_opt_ sockaddr_in6 proxyV6 = { 0 }, _In_opt_ bool loggingEnable = false) :
		m_Config{ proxyType, proxyV4, proxyV6, loggingEnable }
	{ }

	// BaseConfigManager constructor.
	// @param config - source config.
	explicit BaseConfigManager(_In_ Config&& config) :
		m_Config{ std::move(config) }
	{ }

	// Default destructor.
	virtual ~BaseConfigManager() = default;

	// Returns current config.
	const Config& GetConfig() const noexcept {
		return m_Config;
	}

	// Returns true if current config are valid.
	bool IsValid() const noexcept {
		return Validate(m_Config);
	}

	// Returns true if address v4 of socks server are valid.
	static bool IsValidIPv4Address(const Config& config) {
		return config.m_ProxyV4.sin_family == AF_INET || config.m_ProxyV4.sin_port != 0;
	}

	// Returns true if address v6 of socks server are valid.
	static bool IsValidIPv6Address(const Config& config) {
		return config.m_ProxyV6.sin6_family == AF_INET6 || config.m_ProxyV6.sin6_port != 0;
	}

	// Returns true if specified config are valid.
	static bool Validate(const Config& config) 
	{
		return	config.m_ProxyType != ProxyType::Unknown &&
						(config.m_ProxyType == ProxyType::Socks4 && IsValidIPv4Address(config)) ||
						(config.m_ProxyType == ProxyType::Socks5 && (IsValidIPv4Address(config) || IsValidIPv6Address(config)));
	}

protected:
	Config m_Config;
};

#endif // !COMMON_BASE_CONFIG_H_
