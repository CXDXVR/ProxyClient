#ifndef REDIRECTOR_SOCKS4_HPP_
#define REDIRECTOR_SOCKS4_HPP_

class Socks4 : public AbstractSocks
{
#	pragma pack(push, 1)
	// Message.
	struct Message
	{
		BYTE	version;		// Socks version.
		union {
			BYTE	command;	// Connection type.
			BYTE status;		// Status type.
		};
		WORD	port;				// Target port.
		DWORD address;		// Target ip.
	};
#	pragma pack(pop)

	// Socks4 version.
	static constexpr BYTE VERSION_ = 0x4;

	// Socks4 reply codes.
	enum class ReplyCode : BYTE
	{
		Granted						= 90,	// Request granted.
		Rejected					= 91,	// Request rejected or failed.
		ConnectionFailed	= 92, // Request rejected becasue SOCKS server cannot connect to identd on the client.
		ClientConflict		= 93	// Request rejected because the client program and identd report different user - ids.
	};

	// Socks4 command types.
	enum class Command : BYTE
	{
		Connect = 1,
		Bind		= 2
	};

public:
	// Deleted default constructor.
	Socks4() = delete;
	// Default destructor.
	~Socks4() = default;
	// Deleted copy constructor.
	Socks4(const Socks4&) = delete;
	// Deleted copy assigment.
	Socks4& operator=(const Socks4&) = delete;

	// Socks4 constructor.
	// @param config - app config.
	// @param socket - connected socket.
	// @param address - target app address.
	Socks4(_In_ const BaseConfigManager::Config& config, _In_ SOCKET& socket, _In_ const sockaddr* address) :
		AbstractSocks{ config, socket, address }
	{ }

	// Sends request to socks server.
	// @returns true if success.
	bool Request() override
	{
		if (m_AddressApp->sa_family != AF_INET) 
		{
			spdlog::error("Unsupported socks4 address type.");
			return false;
		}

		auto ipv4				= reinterpret_cast<const sockaddr_in*>(m_AddressApp);
		auto message		= Message{ VERSION_, static_cast<BYTE>(Command::Connect), ipv4->sin_port, ipv4->sin_addr.S_un.S_addr };
		auto dataToSend = std::vector<BYTE>(sizeof(message) + 1/*user-id*/);

		// Insert socks request structure.
		dataToSend.insert(
			dataToSend.begin(), 
			reinterpret_cast<const BYTE*>(&message),
			reinterpret_cast<const BYTE*>(&message) + sizeof(message)
		);

		// Sending request to socks server.
		if (send(m_Socket, reinterpret_cast<const char*>(dataToSend.data()), static_cast<int>(dataToSend.size()), 0) != dataToSend.size())
		{
			spdlog::error("Failed to send socks4 request message. WSAGetLastError={}", WSAGetLastError());
			return false;
		}

		// Receiving response from socks server.
		if (recv(m_Socket, reinterpret_cast<char*>(&message), sizeof(message), 0) == SOCKET_ERROR) 
		{
			spdlog::error("Failed to read socks4 response. WSAGetLastError={}", WSAGetLastError());
			return false;
		}

		return message.status == static_cast<BYTE>(ReplyCode::Granted);
	}
};

#endif // !REDIRECTOR_SOCKS4_HPP_