#ifndef WINPIPE_SERVER_H_
#define WINPIPE_SERVER_H_

#include <string>
#include <stdexcept>

#include "basepipe.hpp"

namespace WinPipe
{
	class NamedPipeServer : public BaseNamedPipe
	{
		static constexpr DWORD PIPE_BUFFER_SIZE_ = 4096;
	public:
		// Deleted default constructor.
		NamedPipeServer()	= delete;
		// Default destructor.
		~NamedPipeServer() = default;
		// Deleted copy constructor.
		NamedPipeServer(const NamedPipeServer&) = delete;
		// Deleted copy assigment.
		NamedPipeServer& operator=(const NamedPipeServer&) = delete;

		// NamedPipeServer constructor.
		// @param stopEvent - reference to stop event.
		// @param name - if not empty, creates a named pipe.
		NamedPipeServer(_In_ WinHandle& stopEvent, _In_opt_ const std::wstring& name = std::wstring()) :
			BaseNamedPipe{ stopEvent }
		{
			if (!name.empty()) 
				Create(name);
		}

		// Creates a named pipe.
		// @param name - pipe name.
		// @returns ERROR_SUCCESS if success.
		WinError Create(_In_ const std::wstring& name)
		{
			WinError status = ERROR_SUCCESS;

			m_PipeHandle = WinHandle(CreateNamedPipeW(name.c_str(), PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, PIPE_READMODE_BYTE | PIPE_WAIT,
				PIPE_UNLIMITED_INSTANCES, PIPE_BUFFER_SIZE_, PIPE_BUFFER_SIZE_, 0, nullptr));

			if (m_PipeHandle.get() == INVALID_HANDLE_VALUE)
				status = GetLastError();

			return status;
		}
		
		// Waiting for client connection.
		// @returns ERROR_SUCCESS or ERROR_PIPE_CONNECTED if success.
		WinError Connect() 
		{
			OVERLAPPED	overlapped{ 0 };
			WinError		status = ERROR_SUCCESS;

			// Creating overlapped event.
			if ((overlapped.hEvent = CreateEventW(nullptr, false, false, nullptr)))
			{
				status = ConnectNamedPipe(m_PipeHandle.get(), &overlapped);
				if (status == FALSE)
				{
					status = GetLastError();

					// If the function fails, GetLastError returns a value 
					// other than ERROR_IO_PENDING or ERROR_PIPE_CONNECTED.
					if (status != ERROR_IO_PENDING && status != ERROR_PIPE_CONNECTED)
						return status;

					// If the operation has not been completed yet, 
					// GetLastError returns ERROR_IO_PENDING.
					if (status == ERROR_IO_PENDING)
					{
						// Now we need to waiting someone event.
						HANDLE objects[] = { m_StopEvent.get(), overlapped.hEvent };
						status = WaitForMultipleObjects(sizeof(objects) / sizeof(objects[0]), objects, false, INFINITE);

						// m_StopEvent fired or other error.
						if (status != (WAIT_OBJECT_0 + 1))
							status = ERROR_INVALID_HANDLE;
						else
							status = ERROR_SUCCESS;
					}
				}
				else
					status = ERROR_SUCCESS;
			}
			else
				status = GetLastError();

			return status;
		}
	};
}

#endif // !WINPIPE_SERVER_H_
