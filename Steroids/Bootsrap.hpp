#pragma once

_IRQL_requires_max_(PASSIVE_LEVEL)
_Must_inspect_result_
extern "C"
[[nodiscard]]
NTSTATUS const
SetroidsInitialize(
	_In_ _Notnull_ struct _DRIVER_OBJECT* DriverObject
) noexcept;

_IRQL_requires_max_(PASSIVE_LEVEL)
_Must_inspect_result_
extern "C"
[[nodiscard]]
void
SteroidsFinalize() noexcept;