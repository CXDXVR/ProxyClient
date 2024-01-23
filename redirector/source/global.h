#ifndef REDIRECTOR_GLOBAL_H_
#define REDIRECTOR_GLOBAL_H_

#include <winsock2.h>
#include <ws2tcpip.h>
#include <Windows.h>
#include <memory>
#include <string>
#include <stdexcept>
#include <map>
#include <unordered_map>

#include "winpipe/basepipe.hpp"
#include "winpipe/client.hpp"
#include "common/baseconfig.hpp"
#include "common/objectnames.hpp"
#include "MinHook.h"

#pragma warning(push)
#pragma warning(disable: 4996)
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#pragma warning(pop)

#include "string.hpp"
#include "hook.hpp"
#include "basecore.h"
#include "basesocks.h"
#include "socks4.hpp"
#include "socks5.hpp"
#include "sockethook.h"
#include "config.h"
#include "core.h"

#endif // !REDIRECTOR_GLOBAL_H_
