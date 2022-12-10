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

#include "Transmit/Transmit.h"
#include "Process/Process.h"
#include "Bootstrap/Bootstrap.h"
#include "Utility/Result/Result.h"

_Use_decl_annotations_
NTSTATUS
HandleTransmit(
	ULONG const IoControlCode,
	ULONG const InputBufferLength,
	ULONG& OutputBufferLength,
	PVOID InputBuffer,
	PVOID OutputBuffer
) noexcept {
	// Ensures input buffer or output buffer is not null
	switch (static_cast<SteroidsFunction>(IoGetFunctionCodeFromCtlCode(IoControlCode))) {
	case SteroidsFunction::SteroidsAvailable: // Check is steroids available
		REQUIRE_OUTPUTBUFFER(sizeof(bool)); // Make sure output buffer is not null 

		*static_cast<bool*>(OutputBuffer) = true;
		return STATUS_SUCCESS;

	case SteroidsFunction::TerminateProcess: // Terminate process
		REQUIRE_INPUTBUFFER(sizeof(HANDLE));

#pragma warning(disable: 28118) //	This function will fail when irql is greater than PASSIVE_LEVEL
		return HandleTerminateProcess(*static_cast<HANDLE const*>(InputBuffer));
#pragma warning(default: 28118)

	case SteroidsFunction::ProtectProcess:  // Protect Process
		REQUIRE_INPUTBUFFER(sizeof(HANDLE));

		AddProtectProcess(*static_cast<HANDLE const*>(InputBuffer));

		return STATUS_SUCCESS;

	case SteroidsFunction::ReadProcessMemory: // Read process memory
		REQUIRE_INPUTBUFFER(sizeof(ReadProcessMemoryFunction));

#pragma warning(disable: 28118) // This function will fail when irql is greater than APC_LEVEL
		return HandleReadProcessMemory(*static_cast<ReadProcessMemoryFunction const*>(InputBuffer));
#pragma warning(default: 28118)

	default:
		return STATUS_ILLEGAL_FUNCTION;
	}

}

_Use_decl_annotations_
NTSTATUS
HandleTerminateProcess(
	HANDLE const ProcessId
) noexcept {
	ASSERT(KeGetCurrentIrql() <= PASSIVE_LEVEL);

	if (KeGetCurrentIrql() > PASSIVE_LEVEL) [[unlikely]] {
		return STATUS_RDBSS_POST_OPERATION;
	}

	return Process::GetProcessById(ProcessId).IfSuccess([](Process& Process) { Process.Terminate(); });
}

_Use_decl_annotations_
NTSTATUS
HandleReadProcessMemory(
	struct ReadProcessMemoryFunction const& Function
) noexcept {
	ASSERT(KeGetCurrentIrql() <= APC_LEVEL);

	if (KeGetCurrentIrql() > APC_LEVEL) [[unlikely]] {
		return STATUS_RDBSS_POST_OPERATION;
	}

	SIZE_T* NumberOfBytesRead;

	// Determine if this parameter is null
	if (Function.NumberOfBytesRead == nullptr) {
		SIZE_T NumberOfBytesTransferred{};
		NumberOfBytesRead = &NumberOfBytesTransferred;
	}
	else {
		__try {
			// Checks that the buffer resides in the user-mode portion of the address space, is writable
			ProbeForWrite(Function.NumberOfBytesRead, sizeof(SIZE_T), TYPE_ALIGNMENT(SIZE_T*));

			// The buffer is writeable
			NumberOfBytesRead = Function.NumberOfBytesRead;
		}
		__except (GetExceptionCode() == STATUS_ACCESS_VIOLATION ||
			GetExceptionCode() == STATUS_DATATYPE_MISALIGNMENT ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
			ASSERT(GetExceptionCode() != STATUS_DATATYPE_MISALIGNMENT);

			SIZE_T NumberOfBytesTransferred{};
			NumberOfBytesRead = &NumberOfBytesTransferred;
		}
	}
	
	// Determine if the process with the memory that is being read is already attached
	if (PsGetCurrentProcessId() == Function.ProcessId) {
		// TODO:
		return MmCopyMemory(Function.Buffer, {.VirtualAddress = Function.BaseAddress}, Function.Size, MM_COPY_MEMORY_VIRTUAL, NumberOfBytesRead);
	}
	else {

		// Lookup Process
		PEPROCESS Process;
		NTSTATUS const Status = PsLookupProcessByProcessId(Function.ProcessId, &Process);

		// Ensures the process is looked up successfully
		if (!NT_SUCCESS(Status)) [[unlikely]] {
			return Status;
		}

		KAPC_STATE ApcState;

		// Attaches the current thread to the address space of the target process
		KeStackAttachProcess(Process, &ApcState);

		// Note  Attaching a thread to a different process can prevent asynchronous I/O
		// operations from completing and can potentially cause deadlocks. In general, the
		// lines of code between the call to KeStackAttachProcess and the call to
		// KeUnstackDetachProcess should be very simple and should not call complex
		// routines or send IRPs to other drivers

		// Get physical address of supplied virtual address
		PHYSICAL_ADDRESS const PhysicalAddress = MmGetPhysicalAddress(Function.BaseAddress);

		// Detaches the current thread from the address space of a process and restores the previous attach state
		KeUnstackDetachProcess(&ApcState);

		return MmCopyMemory(Function.Buffer, { .PhysicalAddress = PhysicalAddress }, Function.Size, MM_COPY_MEMORY_PHYSICAL, NumberOfBytesRead);
	}
}