#include "global.h"

static WinPipe::WinHandle	m_StopEvent;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL WINAPI DllMain(_In_ HINSTANCE instance, _In_ DWORD reason, _In_ LPVOID reserved)
{
  switch (reason)
  {
    case DLL_PROCESS_ATTACH:
    {
#     if (0)
      // Logger initialization.
      spdlog::set_default_logger(
        spdlog::basic_logger_mt("redirector", R"(C:\Logs\proxy_logger.txt)")
      );
      spdlog::flush_on(spdlog::level::info);
#     endif

      // MinHook initialization.
      if (auto result = MinHook::Initialize(); result != MH_OK) 
      {
        spdlog::error("Failed to initialize MinHook {}.", result);
        return FALSE;
      }

			m_StopEvent = WinPipe::WinHandle(CreateEventW(nullptr, false, false, ObjectNames::GetStopEventName(GetCurrentProcessId()).c_str()));
			std::thread([](_In_ HINSTANCE instance) { Core(instance, m_StopEvent).Wait(); FreeLibrary(instance); }, instance).detach();

      break;
    }
    case DLL_THREAD_ATTACH:
      // Do thread-specific initialization.
      break;
    case DLL_THREAD_DETACH:
      // Do thread-specific cleanup.
      break;
    case DLL_PROCESS_DETACH:
    {
			SetEvent(m_StopEvent.get());

			MinHook::Uninitialize();
			m_StopEvent.reset();

			spdlog::set_level(spdlog::level::off);
			spdlog::shutdown();

      break;
    }
  }

  return TRUE;
}
