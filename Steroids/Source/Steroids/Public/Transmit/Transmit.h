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

#include "Core/Core.h"

#define REQUIRE_INPUTBUFFER(INPUT_BUFFER_LENGTH) if (InputBuffer == nullptr) [[unlikely]] { return STATUS_INVALID_USER_BUFFER; } if(InputBufferLength != INPUT_BUFFER_LENGTH) [[unlikely]] { return STATUS_INVALID_BUFFER_SIZE; }
#define REQUIRE_OUTPUTBUFFER(OUTPUT_BUFFER_LENGTH) if (OutputBuffer == nullptr) [[unlikely]] { return STATUS_INVALID_USER_BUFFER; } if(OutputBufferLength != OUTPUT_BUFFER_LENGTH) [[unlikely]] { return STATUS_INVALID_BUFFER_SIZE; } OutputBufferLength = OUTPUT_BUFFER_LENGTH;
#define REQUIRE_INOUTBUFFER(INPUT_BUFFER_LENGTH, OUTPUT_BUFFER_LENGTH) if (InputBuffer == nullptr || OutputBuffer == nullptr) [[unlikely]] { return STATUS_INVALID_USER_BUFFER; } if(InputBufferLength != INPUT_BUFFER_LENGTH || OutputBufferLength != OUTPUT_BUFFER_LENGTH) [[unlikely]] { return STATUS_INVALID_BUFFER_SIZE; } OutputBufferLength = OUTPUT_BUFFER_LENGTH;

/** Handle transmit, this function is called when application calls IoDeviceControl */
[[nodiscard]]
_Must_inspect_result_
_Function_class_(DRIVER_DISPATCH)
_IRQL_requires_max_(DISPATCH_LEVEL)
_IRQL_requires_same_
_Dispatch_type_(IRP_MJ_DEVICE_CONTROL)
NTSTATUS
HandleTransmit(
	_In_ _Const_ ULONG const IoControlCode,
	_In_ _Const_ ULONG const InputBufferLength,
	_Inout_ ULONG& OutputBufferLength,
	_In_ _Notnull_ _Readable_bytes_(InputBufferLength) PVOID InputBuffer,
	_Out_ _Writable_bytes_(OutputBufferLength) PVOID OutputBuffer
) noexcept;

[[nodiscard]]
_Must_inspect_result_
_IRQL_requires_max_(PASSIVE_LEVEL)
_IRQL_requires_same_
NTSTATUS
HandleTerminateProcess(
	_In_ _Const_ HANDLE const ProcessId
) noexcept;

[[nodiscard]]
_Must_inspect_result_
_IRQL_requires_max_(APC_LEVEL)
_IRQL_requires_same_
NTSTATUS
HandleReadProcessMemory(
	_In_ _Const_ struct ReadProcessMemoryFunction const& Function
) noexcept;

/** Copies the data in the specificed address range from the address space of the specificed process
	into the specificed buffer of the current process. requiring a ReadProcessMemoryFunction */
_Struct_size_bytes_(sizeof(ReadProcessMemoryFunction))
struct ReadProcessMemoryFunction {
	/** An id to the process with the memory that is being read */
	HANDLE ProcessId;

	/** A pointer to the base address in the specified process from which to read.
		Before any data transfer occurs, the system verifies that all data in the 
		base address and memory of the specified size is accessible for read access,
		and if it is not accessible the function fails. If memory at the virtual address 
		specified by this parameter is not resident, this function will try to make it resident */
	PVOID const BaseAddress;

	/** A pointer to a buffer that receives the contents from the address space of the specified process */
	PVOID Buffer;

	/** The number of bytes to be read from the specified process */
	SIZE_T Size;

	/** A pointer to a variable that receives the number of bytes transferred into the specified buffer,
		If NumberOfBytesRead is null, the parameter is ignored. This parameter is also ignored if the pointer 
		is not actually resides in the user-mode portion of the address space, or is not writable, or is not 
		correctly aligned */
	SIZE_T* NumberOfBytesRead;
};

/** Steroids function codes */
enum class SteroidsFunction : int {
	/** Returns a bool that indicates if the Steroids is available */
	SteroidsAvailable,

	/** Terminates the specificd process, requiring a process ID in 64-bit unsigned integer */
	TerminateProcess,

	/** Protects the specified process, lower handle access when the process handle or handle of 
		thread of the process is being open, requiring a process ID in 64-bit unsigned integer */
	ProtectProcess,

	/** Copies the data in the specificed address range from the address space of the specificed process
		into the specificed buffer of the current process. requiring a ReadProcessMemoryFunction structure.
		If the memory at the specified virtual address is not resident, this function will try to make it resident.
		This operation ignores the protection of the memory at the specified virtual address. */
	ReadProcessMemory,

	/** Writes data to an area of memory in a specified process. */
	WriteProcessMemory
};