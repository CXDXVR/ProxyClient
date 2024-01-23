#include "global.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ConfigManager::ConfigManager(_In_ AbstractCore* mediator, _In_ WinPipe::WinHandle& stopEvent) :
	BaseConfigManager{ },
	AbstractComponent{ mediator },
	m_Pipe{ stopEvent, ObjectNames::GetConfigPipeName(GetCurrentProcessId()) },
	m_Thread{ &ConfigManager::CommunicationThread, this }
{ }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ConfigManager::~ConfigManager()
{
	if (m_Thread.joinable())
		m_Thread.join();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ConfigManager::CommunicationThread()
{
	Config	newConfig;
	auto		status = WinPipe::WinError(ERROR_SUCCESS);

	do
	{
		status = m_Pipe.Read(newConfig);

		if (status == ERROR_SUCCESS) 
		{
			m_Config = newConfig;
			m_Mediator->Notify(this, m_Mediator->UpdateConfig);
		}
	} while (status == ERROR_SUCCESS || status == WAIT_TIMEOUT);

	m_Mediator->Notify(this, AbstractCore::Event::StopEvent);
}