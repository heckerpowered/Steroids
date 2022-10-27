#pragma once

#include "../Core.h"

/** Initialize Steroids */
_IRQL_requires_max_(PASSIVE_LEVEL)
_Must_inspect_result_
extern "C"
[[nodiscard]]
NTSTATUS
SteroidsInitialize(
	_In_ _Notnull_ struct _DRIVER_OBJECT* DriverObject
) noexcept;

/** Finalize Steroids, called when driver is being unloaded */
_IRQL_requires_max_(PASSIVE_LEVEL)
void
SteroidsFinalize() noexcept;

/** Called when any process handle is created or duplicated */
[[nodiscard]]
OB_PREOP_CALLBACK_STATUS 
PreOperation(
	_In_ PVOID RegistrationContext [[maybe_unused]],
	_Inout_ POB_PRE_OPERATION_INFORMATION OperationInformation
) noexcept;

/** Given two such key values, the CompareRoutine returns GenericLessThan, GenericGreaterThan, or GenericEqual */
RTL_AVL_COMPARE_ROUTINE CompareRoutine;

/** Allocate memory for caller-supplied data plus some additional memory for use */
RTL_AVL_ALLOCATE_ROUTINE AllocateRoutine;

/** Deallocate memory for elements to be deleted from the avl table */
RTL_AVL_FREE_ROUTINE FreeRoutine;