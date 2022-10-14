#include "../../Public/Transmit/Transmit.h"

_Use_decl_annotations_
NTSTATUS
const
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

	__try {
		// Checks that a user-mode buffer actually resides in the user portion of the address space
		ProbeForRead(InputBuffer, InputBufferLength, sizeof(CHAR));
	}
	__except (GetExceptionCode() == STATUS_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
		return GetExceptionCode();
	}

	switch (IoGetFunctionCodeFromCtlCode(IoControlCode)) {
	case 0:
		OutputBufferLength = 1;

		return STATUS_SUCCESS;
		break;
	default:
		OutputBufferLength = 0;
		return STATUS_ILLEGAL_FUNCTION;
	}
}