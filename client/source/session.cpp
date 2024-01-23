#include "global.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Session::Session(_In_ DWORD id, _In_ std::weak_ptr<AbstractServer> server) :
	AbstractSession{ id, server, ObjectNames::GetStopEventName(id) },
	m_PipeConfig{ m_StopEvent, ObjectNames::GetConfigPipeName(m_Id) },
	m_PipeReport{ nullptr }
{ }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Session::Stop()
{
	SetEvent(m_StopEvent.get());

	if (m_ReportThread.joinable())
		m_ReportThread.join();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Session::UpdateConfig(const BaseConfigManager::Config& config)
{
	// If logging is enabled, create a named pipe for reports.
	if (config.m_LoggingEnable && !m_PipeReport.get()) 
	{
		m_PipeReport		= std::make_unique<WinPipe::NamedPipeServer>(m_StopEvent, ObjectNames::GetReportPipeName(m_Id));
		m_ReportThread	= std::thread(&Session::ReportThread, this);
	}
	// If logging is disabled, close the named report pipe.
	else if (!config.m_LoggingEnable) 
	{
		m_PipeReport.reset();
		if (m_ReportThread.joinable()) m_ReportThread.join();
	}

	// Sending new config.
	auto status = SendConfig(config);
	if (status != ERROR_SUCCESS) 
	{
		spdlog::error("Failed to send new configuration. GetLastError={}.", status);
		return;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Session::ReportThread()
{
	auto status	= m_PipeReport->Connect();

	if (status == ERROR_SUCCESS || status == ERROR_PIPE_CONNECTED) do
	{
		WORD	messageId		= 0;
		BYTE*	messageData = nullptr;
		DWORD	messageSize = 0;

		if ((status = m_PipeReport->ReadMessage(messageId, &messageData, messageSize)) == ERROR_SUCCESS)
		{
			if (messageId == AF_INET) 
			{
				auto address = reinterpret_cast<const sockaddr_in*>(messageData);
				spdlog::info("Session {} connecting to IPv4 {}:{}", m_Id, inet_ntoa(address->sin_addr), htons(address->sin_port));
			}
			else if (messageId == AF_INET6)
			{
				auto address = reinterpret_cast<const sockaddr_in6*>(messageData);
				char ipv6[INET6_ADDRSTRLEN];
				inet_ntop(AF_INET6, &address->sin6_addr, ipv6, INET6_ADDRSTRLEN);
				spdlog::info("Session {} connecting to IPv6 [{}]:{}", m_Id, ipv6, htons(address->sin6_port));
			}

			delete[] messageData;
		}

	} while (status == ERROR_SUCCESS || status == WAIT_TIMEOUT);
	else 
		spdlog::error("Failed to connect report named pipe in session {}. GetLastError={}.", m_Id, status);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
WinPipe::WinError Session::SendConfig(const BaseConfigManager::Config& config)
{
	auto status = m_PipeConfig.Write(config);

	if (status == ERROR_PIPE_LISTENING)
	{
		status = m_PipeConfig.Connect();
		if (status == ERROR_SUCCESS || status == ERROR_PIPE_CONNECTED)
			status = m_PipeConfig.Write(config);
	}

	return status;
}
