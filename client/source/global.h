#ifndef CLIENT_GLOBAL_H_
#define CLIENT_GLOBAL_H_

#include <WS2tcpip.h>
#include <Windows.h>
#include <Shlwapi.h>
#include <TlHelp32.h>
#include <memory>
#include <string>
#include <thread>
#include <unordered_set>
#include <stdexcept>

#include "winpipe/basepipe.hpp"
#include "winpipe/server.hpp"
#include "common/baseconfig.hpp"
#include "common/objectnames.hpp"

#pragma warning(push)
#pragma warning(disable: 4996)
#include "spdlog/spdlog.h"
#pragma warning(pop)

#include "process.h"
#include "basesession.h"
#include "baseserver.h"
#include "basecore.h"
#include "session.h"
#include "server.h"
#include "core.h"

#endif // !CLIENT_GLOBAL_H_
