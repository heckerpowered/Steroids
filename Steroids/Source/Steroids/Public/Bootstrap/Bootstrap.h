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

/** Initialize Steroids */
_IRQL_requires_max_(PASSIVE_LEVEL)
_Must_inspect_result_
extern "C"
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

/** Protects the supplied process, lower handle access when the process handle
	or handle of thread of the process is being open */
void
AddProtectProcess(
	_In_ _Notliteral_ HANDLE ProcessId
) noexcept;

/* Calls when a process is being create or exit */
void
CreateProcessNotify(
	_In_ HANDLE ParentId [[maybe_unused]],
	_In_ HANDLE ProcessId,
	_In_ BOOLEAN Create
) noexcept;

/** Given two such key values, the CompareRoutine returns GenericLessThan, GenericGreaterThan, or GenericEqual */
RTL_AVL_COMPARE_ROUTINE CompareRoutine;

/** Allocate memory for caller-supplied data plus some additional memory for use */
RTL_AVL_ALLOCATE_ROUTINE AllocateRoutine;

/** Deallocate memory for elements to be deleted from the avl table */
RTL_AVL_FREE_ROUTINE FreeRoutine;