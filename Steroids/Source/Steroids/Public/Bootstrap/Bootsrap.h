#pragma once

#include "../Core.h"

/** Initialize Steroids */
_IRQL_requires_max_(PASSIVE_LEVEL)
_Must_inspect_result_
extern "C"
[[nodiscard]]
NTSTATUS const
SteroidsInitialize(
	_In_ _Notnull_ struct _DRIVER_OBJECT* DriverObject
) noexcept;

/** Finalize Steroids, called when driver is being unloaded */
_IRQL_requires_max_(PASSIVE_LEVEL)
[[nodiscard]]
void
SteroidsFinalize() noexcept;

/** Called when any process handle is created or duplicated */
[[nodiscard]]
OB_PREOP_CALLBACK_STATUS PreOperation(
	_In_ PVOID RegistrationContext,
	_Inout_ POB_PRE_OPERATION_INFORMATION OperationInformation
) noexcept;

/**/
RTL_GENERIC_COMPARE_ROUTINE CompareRoutine;