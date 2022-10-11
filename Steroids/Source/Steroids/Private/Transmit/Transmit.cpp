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
}