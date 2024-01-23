#include "argparse/argparse.hpp"

#include "global.h"

static constexpr char G_ARGUMENT_PROC_PID_[]          = "--pid";
static constexpr char G_ARGUMENT_PROC_NAME_[]         = "--name";
static constexpr char G_ARGUMENT_LOG_ENABLE_[]        = "--enable-log";
static constexpr char G_ARGUMENT_PROXY_TYPE_[]        = "--proxy-type";
static constexpr char G_ARGUMENT_PROXY_ADDRESS_V4_[]  = "--proxy-v4";
static constexpr char G_ARGUMENT_PROXY_ADDRESS_V6_[]  = "--proxy-v6";

template <std::size_t S>
constexpr std::size_t string_length(char const (&)[S]) { return S - 1; }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ExtractIPv4FromString(_In_ const std::string& address, _Out_ sockaddr_in& buffer)
{
  static constexpr char portDelimiter[] = ":";

  // Looking for port position.
  auto portPos = address.find(portDelimiter);
  if (portPos == std::string::npos)
    return false;


  // Filling sockaddr_in struct.
  buffer.sin_family = AF_INET;
  buffer.sin_port   = htons(std::atoi(address.substr(portPos + string_length(portDelimiter)).c_str()));

  return inet_pton(AF_INET, address.substr(0, portPos).c_str(), &buffer.sin_addr);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ExtractIPv6FromString(_In_ const std::string& address, _Out_ sockaddr_in6& buffer)
{
  // The address should be of the following form:
  // [fe80::637c:fe74:d6a:364e]:8080, where 8080 - port.
  static constexpr char addressBegin[] = "[";
  static constexpr char addressEnd[] = "]:";

  auto  addrStartPos  = address.find(addressBegin),
        addrEndPos    = address.find(addressEnd);

  if (addrStartPos == std::string::npos || addrEndPos == std::string::npos)
    return false;

  // Filling sockaddr_in6 struct.
  buffer.sin6_family  = AF_INET6;
  buffer.sin6_port    = htons(std::atoi(address.substr(addrEndPos + string_length(addressEnd)).c_str()));

  return inet_pton(
    AF_INET6, 
    address.substr(addrStartPos + string_length(addressBegin), addrEndPos - string_length(addressBegin)).c_str(), 
    &buffer.sin6_addr
  );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool GetConfigFromArguments(_In_ int argc, _In_ const char** argv, _Out_ std::unordered_set<DWORD>& pids, _Out_ std::unordered_set<std::string>& names, _Out_ BaseConfigManager::Config& config)
{
  auto argumentParser = argparse::ArgumentParser("client.exe");

  // Filling arguments.
  {
    argumentParser.add_argument(G_ARGUMENT_PROC_PID_)
      .help("pid of a process to inject.")
      .nargs(1, 6)
      .default_value(std::vector<int>{})
      .scan<'d', int>()
      .append();

    argumentParser.add_argument(G_ARGUMENT_PROC_NAME_)
      .help("short filename of a process with wildcard matching to inject.")
      .nargs(1, 6)
      .default_value(std::vector<std::string>{})
      .append();

    argumentParser.add_argument(G_ARGUMENT_LOG_ENABLE_)
      .help("enable logging for network connections.")
      .default_value(false)
      .implicit_value(true);

    argumentParser.add_argument(G_ARGUMENT_PROXY_TYPE_)
      .help("proxy type (socks4 or socks5).")
      .default_value(std::string{ "socks4" });

    argumentParser.add_argument(G_ARGUMENT_PROXY_ADDRESS_V4_)
      .help("set a proxy IPv4 address for network connections.")
      .default_value(std::string{});

    argumentParser.add_argument(G_ARGUMENT_PROXY_ADDRESS_V6_)
      .help("set a proxy IPv6 address for network connections.")
      .default_value(std::string{});
  }

  // Parsing arguments.
  try 
  { 
    argumentParser.parse_args(argc, argv); 
  }
  catch (const std::runtime_error& error)
  {
    std::cerr << error.what() << std::endl;
    std::cerr << argumentParser << std::endl;
    return false;
  }

  // Extracting arguments.
  auto processPids      = argumentParser.get<std::vector<int>>(G_ARGUMENT_PROC_PID_);
  auto processNames     = argumentParser.get<std::vector<std::string>>(G_ARGUMENT_PROC_NAME_);
  auto proxyAddressV4   = argumentParser.get<std::string>(G_ARGUMENT_PROXY_ADDRESS_V4_);
  auto proxyAddressV6   = argumentParser.get<std::string>(G_ARGUMENT_PROXY_ADDRESS_V6_);
  auto proxyType        = argumentParser.get<std::string>(G_ARGUMENT_PROXY_TYPE_);
  auto logging          = argumentParser.get<bool>(G_ARGUMENT_LOG_ENABLE_);

  pids.insert(processPids.begin(), processPids.end());
  names.insert(processNames.begin(), processNames.end());

  // Validating.
  if ((pids.empty() && names.empty()) || proxyType.empty() || (proxyAddressV4.empty() && proxyAddressV6.empty()))
  {
    std::cerr << "The " << G_ARGUMENT_PROC_PID_ << " or " << G_ARGUMENT_PROC_NAME_ << " and " << G_ARGUMENT_PROXY_ADDRESS_V4_ <<
      " or " << G_ARGUMENT_PROXY_ADDRESS_V6_ << " parameters must be specified." << std::endl;
    std::cerr << argumentParser << std::endl;
    return false;
  }

  config.m_LoggingEnable  = logging;
  config.m_ProxyType      = proxyType == "socks4" ? ProxyType::Socks4 : ProxyType::Socks5;

  std::memset(&config.m_ProxyV4, 0, sizeof(config.m_ProxyV4));
  std::memset(&config.m_ProxyV6, 0, sizeof(config.m_ProxyV6));

  // Parsing IPv4.
  if (!proxyAddressV4.empty()) if (!ExtractIPv4FromString(proxyAddressV4, config.m_ProxyV4))
  {
    std::cerr << "Failed to parse IPv4.";
    std::cerr << argumentParser << std::endl;
    return false;
  }

  // Parsing IPv6.
  if (!proxyAddressV6.empty()) if (!ExtractIPv6FromString(proxyAddressV6, config.m_ProxyV6))
  {
    std::cerr << "Failed to parse IPv6.";
    std::cerr << argumentParser << std::endl;
    return false;
  }

  return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
  BaseConfigManager::Config config;

  std::unordered_set<DWORD>        pids;
  std::unordered_set<std::string>  names;

  if (!GetConfigFromArguments(argc, const_cast<const char**>(argv), pids, names, config))
    return 1;

  auto core = Core(pids, names, config);

  //Sleep(INFINITE);
  core.Wait();
  return 0;
}