#include "global.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Core::~Core()
{
	m_Server->Stop();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Core::Core(const std::unordered_set<DWORD>& pids, const std::unordered_set<std::string>& names, const BaseConfigManager::Config& config) :
	m_Server{ Server::Create() }
{
	auto processIds = pids;
	auto namesIds		= GetPidsFromNames(names);

	processIds.insert(namesIds.begin(), namesIds.end());

	auto injectedPids = InjectIntoProcesses(processIds, config);
	if (injectedPids.empty())
		spdlog::error("No one process is proxied.");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::unordered_set<DWORD> Core::GetPidsFromNames(_In_ const std::unordered_set<std::string>& names)
{
	auto result = std::unordered_set<DWORD>();

	for (const auto& name : names)
	{
		auto pid = Process::GetProcessIdByName(name.c_str());

		if (pid == Process::G_INVALID_PROCESS_ID) 
			spdlog::warn("Failed to get ID of process {}.", name);
		else
			result.insert(pid);
	}

	return result;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::unordered_set<DWORD> Core::InjectIntoProcesses(_In_ const std::unordered_set<DWORD>& pids, _In_ const BaseConfigManager::Config& config)
{
	auto	injectedPids	= std::unordered_set<DWORD>();
	auto	payloadPath		= GetPayloadFullPath();

	if (!PathFileExistsW(payloadPath.c_str()))
	{
		spdlog::error("Module redirector.dll not found.");
		return {};
	}

	for (const auto& pid : pids)
	{
		if (Process::GetRemoteLibraryBase(pid, PathFindFileNameW(const_cast<LPWSTR>(payloadPath.c_str()))))
		{
			spdlog::warn("Process {} already proxied.", pid);
			continue;
		}

		try
		{
			auto session = Session::Create(pid, m_Server);
			if (DoInject(pid, payloadPath))
			{
				spdlog::info("Proxy module injection success into process {}.", pid);
				session->UpdateConfig(config);
				m_Server->AddSession(session);
				injectedPids.insert(pid);
			}
		}
		catch (const std::runtime_error& error)
		{
			spdlog::error("RuntimeError: {}.", error.what());
		}
	}

	return injectedPids;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool Core::DoInject(_In_ DWORD pid, _In_ const std::wstring& module)
{
	auto targetProcess = Process();

	if (!targetProcess.Open(pid, PROCESS_ALL_ACCESS))
	{
		spdlog::error("Failed to open process {}.", pid);
		return false;
	}

	if (!targetProcess.RemoteLoadLibrary(module.c_str()))
	{
		spdlog::error("Failed to load module into process {}.", pid);
		return false;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::wstring Core::GetPayloadFullPath()
{
	if (auto length = GetFullPathNameW(const_cast<LPWSTR>(PAYLOAD_NAME_), 0, nullptr, nullptr); length)
	{
		std::wstring fullPath(length + 1, 0);
		GetFullPathNameW(const_cast<LPWSTR>(PAYLOAD_NAME_), fullPath.capacity(), fullPath.data(), nullptr);
		return fullPath;
	}

	return std::wstring();
}
