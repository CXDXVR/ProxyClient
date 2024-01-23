#ifndef REDIRECTOR_SOCKS5_H_
#define REDIRECTOR_SOCKS5_H_

class Socks5 : public AbstractSocks
{
#	pragma pack(push, 1)
	// Authorization message header.
	struct AuthorizationMessage
	{
		BYTE version;		// Socks version.
		union
		{
			BYTE count;		// Count of methods.
			BYTE method;	// Selected method.
		};
	};

	// TCP Message header.
	struct Message
	{
		BYTE version;			// Socks version.
		union
		{
			BYTE status;		// Status code.
			BYTE command;		// Command type.
		};
		BYTE reserved;		// Reserved.
		BYTE addressType;	// Address type.
	};

	// IPv4 address.
	struct AddressV4
	{
		DWORD address;
		WORD	port;
	};

	// IPv6 address.
	struct AddressV6
	{
		BYTE address[16];
		WORD port;
	};
#	pragma pack(pop)
	
	// Socks version.
	static constexpr BYTE VERSION_ = 0x5;

	// Authorization methods.
	enum class AuthorizationMethod : BYTE
	{
		NoAuth				= 0x00,	// NO AUTHENTICATION REQUIRED
		GSSAPI				= 0x01,	// GSSAPI
		UserPassword	= 0x02, // USERNAME/PASSWORD
		IANA					= 0x03,	// to X'7F' IANA ASSIGNED
		Private				= 0x80, // to X'FE' RESERVED FOR PRIVATE METHODS
		NoAcceptable	= 0xFF,	// NO ACCEPTABLE METHODS
	};

	// Addresses types.
	enum class AddressType : BYTE
	{
		IPv4				= 0x1,
		DomainName	= 0x3,
		IPv6				= 0x4,
	};

	// Command types.
	enum class Command : BYTE
	{
		Connect				= 0x01,
		Bind					= 0x02,
		UdpAssociate	= 0x03
	};

	// Reply codes.
	enum class ReplyCode : BYTE
	{
		Ok							= 0x00, // succeeded
		Error						= 0x01, // general SOCKS server failure
		NotAllowed			= 0x02,	// connection not allowed by ruleset
		ErrorNet				= 0x03,	// Network unreachable
		ErrorHost				= 0x04,	// Host unreachable
		Refused					= 0x05,	// Connection refused
		TTL							= 0x06, // TTL expired
		UnknownCommand	= 0x07,	// Command not supported
		UnknownAddress	= 0x08,	// Address type not supported
		Unknown					= 0x09	// X'09' to X'FF' unassigned
	};

public:
	// Deleted default constructor.
	Socks5() = delete;
	// Default destructor.
	~Socks5() = default;
	// Deleted copy constructor.
	Socks5(const Socks5&) = delete;
	// Deleted copy assigment.
	Socks5& operator=(const Socks5&) = delete;

	// Socks5 constructor.
	// @param config - app config.
	// @param socket - connected socket.
	// @param address - target app address.
	Socks5(_In_ const BaseConfigManager::Config& config, _In_ SOCKET& socket, _In_ const sockaddr* address) :
		AbstractSocks{ config, socket, address }
	{ }

	// Sends request to socks server.
	// @returns true if success.
	bool Request() override
	{
		// Process server authorization.
		if (!DoAuthorization())
			return false;

		auto message	= Message{ VERSION_, static_cast<BYTE>(Command::Connect), 0x0,  static_cast<BYTE>(m_AddressApp->sa_family == AF_INET ? AddressType::IPv4 : AddressType::IPv6)};
		auto buffer		= std::vector<BYTE>();

		// Inserting message header.
		buffer.insert(
			buffer.begin(),
			reinterpret_cast<const BYTE*>(&message),
			reinterpret_cast<const BYTE*>(&message) + sizeof(message)
		);

		// Insering address.
		if (m_AddressApp->sa_family == AF_INET)
		{
			auto ipv4			= reinterpret_cast<const sockaddr_in*>(m_AddressApp);
			auto address	= AddressV4{ ipv4->sin_addr.S_un.S_addr, ipv4->sin_port };

			buffer.insert(
				buffer.end(),
				reinterpret_cast<const BYTE*>(&address),
				reinterpret_cast<const BYTE*>(&address) + sizeof(address)
			);
		}
		else if (m_AddressApp->sa_family == AF_INET6)
		{
			auto ipv6			= reinterpret_cast<const sockaddr_in6*>(m_AddressApp);
			auto address	= AddressV6{};

			std::memcpy(address.address, ipv6->sin6_addr.u.Byte, sizeof(address.address));
			address.port = ipv6->sin6_port;

			buffer.insert(
				buffer.end(),
				reinterpret_cast<const BYTE*>(&address),
				reinterpret_cast<const BYTE*>(&address) + sizeof(address)
			);
		}

		// Sending request to socks server.
		if (send(m_Socket, reinterpret_cast<const char*>(buffer.data()), buffer.size(), 0) != buffer.size())
		{
			spdlog::error("Failed to send socks5 request message. WSAGetLastError={}", WSAGetLastError());
			return false;
		}

		// Receiving response from socks server.
		if (recv(m_Socket, reinterpret_cast<char*>(buffer.data()), buffer.size(), 0) == SOCKET_ERROR)
		{
			spdlog::error("Failed to read socks5 response. WSAGetLastError={}", WSAGetLastError());
			return false;
		}

		return reinterpret_cast<Message*>(buffer.data())->status == static_cast<BYTE>(ReplyCode::Ok);
	}

private:
	// Performs authorization with the server.
	// @reutrns true if success.
	bool DoAuthorization()
	{
		auto message		= AuthorizationMessage{ VERSION_, 1 };
		auto buffer			= std::vector<BYTE>();
		auto authMethod	= static_cast<BYTE>(AuthorizationMethod::NoAuth);

		// Insert request header.
		buffer.insert(
			buffer.begin(),
			reinterpret_cast<const BYTE*>(&message),
			reinterpret_cast<const BYTE*>(&message) + sizeof(message)
		);

		// Insert auth methods.
		buffer.push_back(authMethod);

		// Sending supported auth methods to server.
		if (send(m_Socket, reinterpret_cast<const char*>(buffer.data()), buffer.size(), 0) != buffer.size())
		{
			spdlog::error("Failed to send socks5 authorization message. WSAGetLastError={}", WSAGetLastError());
			return false;
		}

		// Receiving server selected auth method.
		if (recv(m_Socket, reinterpret_cast<char*>(&message), sizeof(message), 0) == SOCKET_ERROR)
		{
			spdlog::error("Failed to receive socks5 authorization message header. WSAGetLastError={}", WSAGetLastError());
			return false;
		}

		if (message.method != authMethod)
		{
			spdlog::error("No acceptable socks5 authorization methods.");
			return false;
		}

		return true;
	}
};


#endif // !REDIRECTOR_SOCKS5_H_