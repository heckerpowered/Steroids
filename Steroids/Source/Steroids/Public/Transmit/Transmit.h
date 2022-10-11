#pragma once

#include "../Core.h"

/** Handle transmit, this function is called when application calls IoDeviceControl */
_Must_inspect_result_
_Function_class_(DRIVER_DISPATCH)
_IRQL_requires_max_(DISPATCH_LEVEL)
_IRQL_requires_same_
_Dispatch_type_(IRP_MJ_DEVICE_CONTROL)
[[nodiscard]]
NTSTATUS
const
HandleTransmit(
	_In_ _Const_ ULONG const IoControlCode,
	_In_ _Const_ ULONG const InputBufferLength,
	_Inout_ ULONG& OutputBufferLength,
	_In_ _Notnull_ _Readable_bytes_(InputBufferLength) PVOID InputBuffer,
	_Out_ _Notnull_ _Writable_bytes_(OutputBufferLength) PVOID OutputBuffer
) noexcept;