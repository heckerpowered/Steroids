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

#include "../../Public/Transmit/Transmit.h"

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
	switch (IoGetFunctionCodeFromCtlCode(IoControlCode)) {
	case 0: // Check is steroids available
		REQUIRE_OUTPUTBUFFER(sizeof(bool)); // Make sure output buffer is not null 

		*static_cast<bool*>(OutputBuffer) = true;
		return STATUS_SUCCESS;

	case 1: // Terminate process
		REQUIRE_INPUTBUFFER(sizeof(HANDLE));

#pragma warning(disable: 28118) // This function will fail when irql is greater than PASSIVE_LEVEL
		return HandleTerminateProcess(*static_cast<HANDLE const*>(InputBuffer));
#pragma warning(default: 28118)

	case 2: // Read process memory
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

	// Lookup Process
	PEPROCESS Process;
	NTSTATUS Status = PsLookupProcessByProcessId(ProcessId, &Process);

	// Ensures the process is looked up successfully
	if (!NT_SUCCESS(Status)) [[unlikely]] {
		return Status;
	}

	// Open process handle
	HANDLE ProcessHandle;
	Status = ObOpenObjectByPointer(&Process, OBJ_KERNEL_HANDLE, nullptr, PROCESS_ALL_ACCESS, *PsProcessType, MODE::KernelMode, &ProcessHandle);

	// Dereference process object before return
	ObDereferenceObjectDeferDelete(Process);
	if (!NT_SUCCESS(Status)) [[unlikely]] {
		return Status;
	}

	// Terminate Process
	Status = ZwTerminateProcess(ProcessHandle, 0);
	if (!NT_SUCCESS(Status)) [[unlikely]] {
		NT_VERIFY(NT_SUCCESS(ZwClose(ProcessHandle)));
	}

	return Status;
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

	// Determine if the process with the memory that is being read is already attached
	if (PsGetCurrentProcessId() == Function.ProcessId) {
		// TODO:
		return MmCopyMemory(Function.Buffer, {.VirtualAddress = Function.BaseAddress}, Function.Size, MM_COPY_MEMORY_VIRTUAL, Function.NumberOfBytesRead);
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

		return MmCopyMemory(Function.Buffer, { .PhysicalAddress = PhysicalAddress }, Function.Size, MM_COPY_MEMORY_PHYSICAL, Function.NumberOfBytesRead);
	}
}