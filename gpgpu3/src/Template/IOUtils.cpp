#include "stdfax.h"
#include "IOUtils.h"
#include <stdio.h>
#include <strsafe.h>
#include <tchar.h>
#include <algorithm>


namespace fio
{

	io_flags_and_attributes operator|(io_flags_and_attributes lhs, io_flags_and_attributes rhs)
	{
		return (io_flags_and_attributes)((DWORD)lhs | (DWORD)rhs);
	}

	io_flags_and_attributes operator&(io_flags_and_attributes lhs, io_flags_and_attributes rhs)
	{
		return io_flags_and_attributes();
	}


	FileHandle CreateNewFile(const char* path, io_share_mode share_mode, io_flags_and_attributes attr_flgs)
	{
		FileHandle hFile = CreateFileA((LPCSTR)path, GENERIC_READ | GENERIC_WRITE, (DWORD)share_mode, NULL, CREATE_NEW, (DWORD)attr_flgs, NULL);

		// Check if invalid handle.
		if (hFile != INVALID_HANDLE_VALUE) return hFile;

		return NULL;
	}

	FileHandle OpenFileReadOnly(const char* path, io_share_mode share_mode, io_flags_and_attributes attr_flgs)
	{
		FileHandle hFile = CreateFileA((LPCSTR)path, GENERIC_READ, (DWORD)share_mode, NULL, OPEN_EXISTING, (DWORD)attr_flgs, NULL);

		// Check if invalid handle.
		if (hFile != INVALID_HANDLE_VALUE) return hFile;

		return NULL;
	}

	FileHandle OpenFileWriteOnly(const char* path, io_share_mode share_mode, io_flags_and_attributes attr_flgs)
	{
		FileHandle hFile = CreateFileA((LPCSTR)path, GENERIC_WRITE, (DWORD)share_mode, NULL, OPEN_EXISTING, (DWORD)attr_flgs, NULL);

		// Check if invalid handle.
		if (hFile != INVALID_HANDLE_VALUE) return hFile;

		return NULL;
	}

	FileHandle OpenFileReadWrite(const char* path, io_share_mode share_mode, io_flags_and_attributes attr_flgs)
	{
		FileHandle hFile = CreateFileA((LPCSTR)path, GENERIC_READ | GENERIC_WRITE, (DWORD)share_mode, NULL, OPEN_EXISTING, (DWORD)attr_flgs, NULL);

		// Check if invalid handle.
		if (hFile != INVALID_HANDLE_VALUE) return hFile;

		return NULL;
	}


	int CloseFileHandle(FileHandle file)
	{
		return CloseHandle(file);
	}

	int DeleteExistingFile(const char* path)
	{
		return DeleteFileA((LPCSTR)path);
	}

	void SetFilePtrPos(FileHandle file, ulong pos)
	{
		SetFilePointer(file, pos, NULL, FILE_BEGIN);
	}


	int WriteToFile(FileHandle file, void* buffer, ulong nBytes)
	{
		return WriteFile(file, buffer, nBytes, NULL, NULL);
	}

	int WriteToFile(FileHandle file, void* buffer, ulong nBytes, ulong& nBytesWritten)
	{
		return WriteFile(file, buffer, nBytes, (LPDWORD)&nBytesWritten, NULL);
	}

	int ReadFromFile(FileHandle file, void* buffer, ulong nBytes)
	{
		return ReadFile(file, buffer, nBytes, NULL, NULL);
	}

	int ReadFromFile(FileHandle file, void* buffer, ulong nBytes, ulong& nBytesRead)
	{
		return ReadFile(file, buffer, nBytes, (LPDWORD)&nBytesRead, NULL);
	}


	int FileSize(FileHandle file)
	{
		return GetFileSize(file, NULL);
	}

	bool CreateNewDirectory(const char* path)
	{
		return CreateDirectoryA(path, NULL);
	}

	bool CreateDirectoryRecursively(const char* path)
	{

		std::string p = std::string(path);
		int pos = 0;

		do {
			int current = glm::min(p.find('\\', pos), p.find('/', pos));
			std::string sub = p.substr(0, current);

			if (!fio::DirectoryExists(sub.c_str()))
				if (!fio::CreateNewDirectory(sub.c_str()))
					return false;

			if (current < 0) break;

			pos = current + 1;

		} while (true);

		return true;
	}

	// https://stackoverflow.com/questions/8233842/how-to-check-if-directory-exist-using-c-and-winapi
	bool DirectoryExists(const char* path)
	{
		DWORD ftyp = GetFileAttributesA(path);
		if (ftyp == INVALID_FILE_ATTRIBUTES) return false;  //something is wrong with your path!
		if (ftyp & FILE_ATTRIBUTE_DIRECTORY) return true;   // this is a directory!
		return false;    // this is not a directory
	}

	void PrintLastIOError()
	{

		DWORD errorMessageID = GetLastError();

		LPSTR messageBuffer = nullptr;

		//Ask Win32 to give us the string version of that message ID.
		//The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
		size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

		//Copy the error message into a std::string.
		std::string message(messageBuffer, size);

		//Free the Win32's string's buffer.
		LocalFree(messageBuffer);

		std::cout << "Error: " << message << std::endl;
	}
}