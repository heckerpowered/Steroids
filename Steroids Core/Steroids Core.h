/**
 * Copyright (C) 2022 Heckerpowered Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the ¡°Software¡±), to deal in the Software without
 * restriction, including without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED ¡°AS IS¡±, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#ifdef STEROIDS_CORE
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __declspec(dllimport)
#endif // STEROIDS_CORE

class EXPORT Steroids {

public:

	/** Type of process ID */
	using ProcessIDType = unsigned __int64;

	/** Type of file handle */
	using FileHandleType = void*;

	/** Pointer type */
	using PointerType = void* __ptr64;

	/** Size type */
	using SizeType = unsigned __int64;

	/* Construct a new Steroids instance, Steroids remain unavailable after being constructed,
		call 'Initialize' method to initialize Steroids */
	[[nodiscard]] Steroids() noexcept;
	~Steroids() noexcept;

private:

	Steroids(Steroids const&) = delete;
	Steroids(Steroids&&) = delete;
	Steroids& operator=(Steroids const&) = delete;
	Steroids& operator=(Steroids&&) = delete;

	/** Steroids's file handle used to transmit with kernel driver, The kernel driver is unable to unload as long
		as the file handle is not closed. The field is valid when the value is not equal to 'INVALID_HANDLE_VALUE'(aka -1)
		and is required for any operations related to Steroids except initializing, this value is initialized with 
		'INVALID_HANDLE_VALUE' by default */
	FileHandleType SteroidsHandle;

public:

	/** Initialize Steroids, this operation needs elevated privilege if the kernel driver is not loaded.
		call 'GetLastError' to get more information if this method fails */
	[[nodiscard]] bool Initialize() noexcept;

	/** Stop Steroids and unload kernel driver, this operation will fail if Steroids is not initialized even
		if the kernel driver is loaded or the Steroids is still using by other processes */
	[[nodiscard]] bool Stop() noexcept;

	/** Returns a bool that indicates if the Steroids is available */
	[[nodiscard]] bool IsAvailable() noexcept;

	/** Copies the data in the specificed address range from the address space of the specificed process
		into the specificed buffer of the current process. requiring a ReadProcessMemoryFunction structure.
		If the memory at the specified virtual address is not resident, this function will try to make it resident.
		This operation ignores the protection of the memory at the specified virtual address */
	[[nodiscard]] bool ReadProcessMemory(
		/** An id to the process with the memory that is being read */
		ProcessIDType const ProcessID,

		/** A pointer to the base address in the specified process from which to read.
			Before any data transfer occurs, the system verifies that all data in the
			base address and memory of the specified size is accessible for read access,
			and if it is not accessible the function fails */
		PointerType const BaseAddress,

		/** A pointer to a buffer that receives the contents from the address space of the specified process */
		PointerType Buffer,

		/** The number of bytes to be read from the specified process */
		SizeType Size,

		/** A pointer to a variable that receives the number of bytes transferred into the specified buffer,
			If NumberOfBytesRead is null, the parameter is ignored */
		SizeType* NumberOfBytesRead) noexcept;

	/** Protects the specified process, lower handle access when the process handle or handle of 
		thread of the process is being open */
	[[nodiscard]] bool ProtectProcess(
		/** The ID of the process to protect */
		ProcessIDType const ProcessID
	) noexcept;

	/** Terminates the specificd process */
	[[nodiscard]] bool TerminateProcess(
		/** The ID of the process to terminate */
		ProcessIDType const ProcessID
	) noexcept;
};