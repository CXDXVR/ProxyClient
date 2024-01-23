#ifndef WINPIPE_BASE_H_
#define WINPIPE_BASE_H_

#include <Windows.h>
#include <memory>
#include <vector>
#include <iostream>

namespace WinPipe
{
	struct DefaultHandleDeleter
	{
		void operator()(_In_ HANDLE _Handle)
		{
			if (_Handle && _Handle != INVALID_HANDLE_VALUE)
				CloseHandle(_Handle);
		}
	};

	using WinHandle = std::unique_ptr<std::remove_pointer_t<HANDLE>, DefaultHandleDeleter>;
	using WinError	= DWORD;

	class BaseNamedPipe
	{
		static constexpr DWORD PIPE_WAIT_TIMEOUT_ = 10000;

#		pragma pack(push, 1)
		struct Message
		{
			WORD	id;
			DWORD size;
		};
#		pragma pack(pop)

	public:
		// Deleted default constructor.
		BaseNamedPipe() = delete;
		// Default virtual destructor.
		virtual ~BaseNamedPipe() = default;

		// Deleted copy constructor.
		BaseNamedPipe(const BaseNamedPipe&) = delete;
		// Deleted copy assigment.
		BaseNamedPipe& operator=(const BaseNamedPipe&)	= delete;

		// BaseNamedPipe constructor.
		// @param stopEvent - reference to stop event.
		// @param pipe - pipe handle. can be nullptr.
		BaseNamedPipe(_In_ WinHandle& stopEvent, _In_opt_ WinHandle pipe = nullptr) :
			m_PipeHandle{ std::move(pipe) },
			m_StopEvent{ stopEvent }
		{ }

		// Returns true if named pipe connected.
		bool IsOpen() const noexcept {
			return m_PipeHandle.get() != nullptr && m_PipeHandle.get() != INVALID_HANDLE_VALUE;
		}

		// Writes primitive data to pipe.
		// @param data - data to send.
		// @reutnrs ERROR_SUCCESS if success.
		template <typename T, std::enable_if_t<std::is_arithmetic_v<T> || std::is_trivial_v<T>, bool> = true>
		WinError Write(_In_ const T& data) { return PipeIO((BYTE*)&data, sizeof(data)); }

		// Reads primitive data from pipe.
		// @param data - buffer.
		// @reutnrs ERROR_SUCCESS if success.
		template <typename T, std::enable_if_t<std::is_arithmetic_v<T> || std::is_trivial_v<T>, bool> = true>
		WinError Read(_Out_ T& data) { return PipeIO(reinterpret_cast<BYTE*>(&data), sizeof(data), false); }

		// Writes raw data to pipe.
		// @param data - data to send.
		// @param size - size of data.
		// @reutnrs ERROR_SUCCESS if success.
		WinError WriteRaw(_In_ const BYTE* data, _In_ DWORD size) {
			return PipeIO(const_cast<BYTE*>(data), size);
		}

		// Reads raw data from pipe.
		// @param data - buffer.
		// @param size - buffer size.
		// @reutnrs ERROR_SUCCESS if success.
		WinError ReadRaw(_Out_ BYTE* data, _Out_ DWORD size) {
			return PipeIO(data, size, false);
		}

		// Writes message to pipe.
		// @parma id - message id.
		// @param data - data to send.
		// @param size - size of data.
		// @reutnrs ERROR_SUCCESS if success.
		WinError WriteMessage(_In_ WORD id, _In_ const BYTE* data, _In_ DWORD size)
		{
			auto dataToSend = std::vector<BYTE>();
			auto header			= Message{ id, static_cast<uint32_t>(size) };

			// Insert message header.
			dataToSend.insert(
				dataToSend.begin(),
				reinterpret_cast<BYTE*>(&header), 
				reinterpret_cast<BYTE*>(&header) + sizeof(header)
			);

			// Insert message data.
			dataToSend.insert(
				(dataToSend.begin() + sizeof(header)), 
				data, 
				data + size
			);

			return WriteRaw(dataToSend.data(), static_cast<DWORD>(dataToSend.size()));
		}

		// Allocates memory and reads the message from the pipe.
		// The caller must call delete[] data.
		// @parma id - message id.
		// @param data - buffer.
		// @param size - buffer size.
		// @reutnrs ERROR_SUCCESS if success.
		WinError ReadMessage(_Out_ WORD& id, _Out_ BYTE** data, _Out_ DWORD& size)
		{
			auto message	= Message{};
			auto status		= Read(message);

			id		= 0;
			*data = nullptr;
			size	= 0;

			if (status == ERROR_SUCCESS) 
			{
				*data = new BYTE[message.size];
				if ((status = ReadRaw(*data, message.size)) == ERROR_SUCCESS) 
				{
					id		= message.id;
					size	= message.size;
				}
				else 
					delete[] (*data);
			}

			return status;
		}

		// Closes the named pipe.
		void Close() { m_PipeHandle.reset(); }

	private:
		WinError PipeIO(BYTE* data, DWORD size, bool write = true)
		{
			OVERLAPPED	overlapped{ 0 };
			WinError		status			= ERROR_SUCCESS;
			DWORD				transferred = 0, originalSize = size;

			// Creating overlapped event.
			if ((overlapped.hEvent = CreateEventW(nullptr, true, false, nullptr)))
			{
				do
				{
					if (write)	status = WriteFile(m_PipeHandle.get(), (data + (originalSize - size)), size, &transferred, &overlapped);
					else				status = ReadFile(m_PipeHandle.get(), (data + (originalSize - size)), size, &transferred, &overlapped);

					if (status == FALSE)
					{
						HANDLE objects[] = { m_StopEvent.get(), overlapped.hEvent };

						if ((status = GetLastError()) != ERROR_IO_PENDING)
							break;

						// Waiting for PIPE_WAIT_TIMEOUT_ seconds to complete.
						status = WaitForMultipleObjects(sizeof(objects) / sizeof(objects[0]), objects, FALSE, PIPE_WAIT_TIMEOUT_);
						if (status != (WAIT_OBJECT_0 + 1)) 
						{
							CancelIo(m_PipeHandle.get());
							break;
						}

						GetOverlappedResult(m_PipeHandle.get(), &overlapped, &transferred, FALSE);
					}
					size -= transferred;
				} while (size);

				if (!size)
					status = ERROR_SUCCESS;

				CloseHandle(overlapped.hEvent);
			}
			else
				status = GetLastError();

			return status;
		}

	protected:
		WinHandle&	m_StopEvent;	// Stop event.
		WinHandle		m_PipeHandle;	// Pipe handle.
	};
}

#endif // !WINPIPE_BASE_H_
