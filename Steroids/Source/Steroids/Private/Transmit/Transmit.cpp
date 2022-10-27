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
	ASSERT(InputBuffer != nullptr);
	ASSERT(OutputBuffer != nullptr);

	if (InputBuffer == nullptr || OutputBuffer == nullptr) [[unlikely]] {
		return STATUS_INVALID_USER_BUFFER;
	}

	switch (IoGetFunctionCodeFromCtlCode(IoControlCode)) {
	case 0: // Terminate process
		if (InputBufferLength != sizeof(HANDLE)) [[unlikely]] {
			return STATUS_INVALID_BUFFER_SIZE;
		}
#pragma warning(disable: 28118) // This function will fail when irql is greater than PASSIVE_LEVEL
		return HandleTerminateProcess(*static_cast<HANDLE const*>(InputBuffer));
#pragma warning(default: 28118)

	default:
		OutputBufferLength = 0;
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
	if (!NT_SUCCESS(Status)) {
		NT_VERIFY(NT_SUCCESS(ZwClose(ProcessHandle)));
	}

	return Status;
}