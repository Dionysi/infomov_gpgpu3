#pragma once
#include "stdfax.h"

/*
* Namespace fio (file io).
*/
namespace fio
{
	/*
	* Our own definition for a file pointer.
	*/
	typedef HANDLE FileHandle;

	enum class io_share_mode : DWORD
	{
		/*
		* Other threads and processes can only access the file for reading actions.
		*/
		share_read = FILE_SHARE_READ,
		/*
		* Other threads and processes can only access the file for writing actions.
		*/
		share_write = FILE_SHARE_WRITE,
		/*
		* Other threads and processes can only access the file for reading and/or writing actions.
		*/
		share_read_write = FILE_SHARE_READ | FILE_SHARE_WRITE,
		/*
		* Other threads and processes can only access the file for deleting and renaming actions.
		*/
		share_delete = FILE_SHARE_DELETE,
		/*
		* Other threads and processes can always open the file.
		*/
		share_all = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		/*
		* Other threads and processes can never open the file while in use.
		*/
		share_none = 0
	};

	/*
	* Note, this list is limited to a few basic file attribute flags!
	*/
	enum class io_flags_and_attributes : DWORD
	{
		/*
		* The file does not have other attributes set. This attribute is valid only if used alone.
		*/
		attribute_normal = FILE_ATTRIBUTE_NORMAL,
		/*
		* The file is hidden. Do not include it in an ordinary directory listing.
		*/
		attribute_hidden = FILE_ATTRIBUTE_HIDDEN,
		/*
		* The file is to be deleted immediately after all of its handles are closed, which includes the specified handle and any other open or duplicated handles.
		* If there are existing open handles to a file, the call fails unless they were all opened with the FILE_SHARE_DELETE share mode.
		*/
		flag_delete_on_close = FILE_FLAG_DELETE_ON_CLOSE,
		/*
		* Access is intended to be random. The system can use this as a hint to optimize file caching.
		*/
		flag_random_access = FILE_FLAG_RANDOM_ACCESS,
		/*
		* Access is intended to be sequential from beginning to end. The system can use this as a hint to optimize file caching.
		*/
		flag_sequential_scan = FILE_FLAG_SEQUENTIAL_SCAN,
		/*
		* Write operations will not go through any intermediate cache, they will go directly to disk.
		*/
		flag_write_through = FILE_FLAG_WRITE_THROUGH,
		/*
		* The file or device is being opened or created for asynchronous I/O.
		*/
		flag_overlapped = FILE_FLAG_OVERLAPPED
	};

	io_flags_and_attributes operator|(io_flags_and_attributes lhs, io_flags_and_attributes rhs);
	io_flags_and_attributes operator&(io_flags_and_attributes lhs, io_flags_and_attributes rhs);

	/*
	* Create a new file and return a handle to the created file. The newly created
	* file is always opened with reading and writing access.
	*
	* @param[in] path				File path.
	* @param[in] share_mode			Sharing mode with other instances of the same file.
	* @param[in] attr_flgs			File attribute and flags for opening the file.
	* @returns						A file handle to a valid file. NULL when an error was encountered.
	*/
	FileHandle CreateNewFile(const char* path, io_share_mode share_mode, io_flags_and_attributes attr_flgs);
	/*
	* Opens an existing file for reading only. If an instance of this file has previously been
	* opened by another thread or process, it should have been opened with the
	* FILE_SHARE_READ mode flag.
	*
	* @param[in] path				File path.
	* @param[in] share_mode			Sharing mode with other instances of the same file.
	* @param[in] attr_flgs			File attribute and flags for opening the file.
	* @returns						A file handle to a valid file. NULL when an error was encountered.
	*/
	FileHandle OpenFileReadOnly(const char* path, io_share_mode share_mode, io_flags_and_attributes attr_flgs);
	/*
	* Opens an existing file for writing only. If an instance of this file has previously been
	* opened by another thread or process, it should have been opened with the
	* FILE_SHARE_WRITE mode flag.
	*
	* @param[in] path				File path.
	* @param[in] share_mode			Sharing mode with other instances of the same file.
	* @param[in] attr_flgs			File attribute and flags for opening the file.
	* @returns						A file handle to a valid file. NULL when an error was encountered.
	*/
	FileHandle OpenFileWriteOnly(const char* path, io_share_mode share_mode, io_flags_and_attributes attr_flgs);
	/*
	* Opens an existing file for writing only. If an instance of this file has previously been
	* opened by another thread or process, it should have been opened with the
	* FILE_SHARE_READ and FILE_SHARE_WRITE mode flags.
	*
	* @param[in] path				File path.
	* @param[in] share_mode			Sharing mode with other instances of the same file.
	* @param[in] attr_flgs			File attribute and flags for opening the file.
	* @returns						A file handle to a valid file. NULL when an error was encountered.
	*/
	FileHandle OpenFileReadWrite(const char* path, io_share_mode share_mode, io_flags_and_attributes attr_flgs);

	/*
	* Closes a previously opened file.
	* @param[in] handle			Handle to a previously opened file.
	* @returns					1 if file was successfuly closed, 0 otherwise.
	*/
	int CloseFileHandle(FileHandle file);
	/*
	* Deletes an existing file. If the file has been previously opened by another
	* process or thread, then it should be opened using at least the FILE_SHARE_DELETE
	* sharing mode flag.
	*
	* @param[in] path			Path to the file that should be deleted.
	* @returns					1 if successfully deleted, 0 otherwise.
	*/
	int DeleteExistingFile(const char* path);

	/*
	* Set the file pointer position.
	* @param[in] pos		New file pointer position.
	*/
	void SetFilePtrPos(FileHandle file, ulong pos);

	/*
	* Write a chunk of bytes to file.
	* @param[in] file			Handle to a valid file.
	* @param[in] buffer				Buffer containing the data to be written.
	* @param[in] nBytes			Number of bytes to write to disk.
	* @returns					1 if written successfully, 0 otherwise.
	*/
	int WriteToFile(FileHandle file, void* buffer, ulong nBytes);
	/*
	* Writes a chunk of bytes to file.
	* @param[in] file				Handle to a valid file.
	* @param[in] buffer				Buffer containing the data to be written.
	* @param[in] nBytes				Number of bytes to write to disk.
	* @param[out] nBytesWritten		Number of bytes written to disk.
	* @returns						1 if written successfully, 0 otherwise.
	*/
	int WriteToFile(FileHandle file, void* buffer, ulong nBytes, ulong& nBytesWritten);

	/*
	* Reads a chunk of bytes from file.
	* @param[in] file			Handle to a valid file.
	* @param[in] buffer			Buffer in which the data is stored.
	* @param[in] nBytes			Number of bytes to write to disk.
	* @returns					1 if written successfully, 0 otherwise.
	*/
	int ReadFromFile(FileHandle file, void* buffer, ulong nBytes);
	/*
	* Reads a chunk of bytes from file.
	* @param[in] file				Handle to a valid file.
	* @param[in] buffer				Buffer in which the data is stored.
	* @param[in] nBytes				Number of bytes to write to disk.
	* @param[out] nBytesRead		Number of bytes actually written to disk.
	* @returns						1 if written successfully, 0 otherwise.
	*/
	int ReadFromFile(FileHandle file, void* buffer, ulong nBytes, ulong& nBytesRead);

	/*
	* Retrieve the file's size in bytes.
	* @param[in] file			Valid filehandle to a previously opened file.
	* @returns					Size of the file in bytes.
	*/
	int FileSize(FileHandle file);

	/*
	* Creates a new directory.
	* @param[in] path		Path to the new directory.
	* @returns				True if directory was created succesfully.
	*/
	bool CreateNewDirectory(const char* path);

	/*
	* Creates a new directory recursively.
	* @param[in] path			Path to the new directory.
	* @returns				True if directory was created succesfully.
	*/
	bool CreateDirectoryRecursively(const char* path);

	/*
	* Checks if the a directory currently exists.
	* @param[in] path			Path to the given directory.
	* @returns					True if the directory exists.
	*/
	bool DirectoryExists(const char* path);

	/*
	* Prints the last IO error to the console.
	*/
	void PrintLastIOError();
};

