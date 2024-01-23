#ifndef WINPIPE_CLIENT_H_
#define WINPIPE_CLIENT_H_

#include <string>
#include <stdexcept>
#include "basepipe.hpp"

namespace WinPipe
{
	class NamedPipeClient final : public BaseNamedPipe
	{
	public:
		// Deleted default constructor.
		NamedPipeClient() = default;
		// Default destructor.
		~NamedPipeClient()	= default;
		// Deleted copy constructor.
		NamedPipeClient(const NamedPipeClient&) = delete;
		// Deleted copy assigment.
		NamedPipeClient& operator=(const NamedPipeClient&) = delete;

		// NamedPipeClient constructor.
		// @param stopEvent - reference to stop event.
		// @param name - if not empty, connects to named pipe server.
		NamedPipeClient(_In_ WinHandle& stopEvent, _In_opt_ const std::wstring& name = std::wstring()) :
			BaseNamedPipe{ stopEvent }
		{
			if (!name.empty())
				Connect(name);
		}

		// Connects to named pipe server.
		// @param name - pipe name.
		// @returns ERROR_SUCCESS if success.
		WinError Connect(_In_ const std::wstring& name) 
		{
			WinError status = ERROR_SUCCESS;

			m_PipeHandle = WinHandle(CreateFileW(name.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0));

			if (m_PipeHandle.get() == INVALID_HANDLE_VALUE)
				status = GetLastError();

			return status;
		}
	};
}

#endif // !WINPIPE_CLIENT_H_
