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

#include "../../Public/Bootstrap/Bootsrap.h"
#include "../../Source/Utility/AVLTree/Public/AVLTree.h"

#if defined(ALLOC_PRAGMA)
#pragma alloc_text(INIT, SteroidsInitialize)
#endif

/** The device object */
PDEVICE_OBJECT DeviceObject;

/** Object callback handle */
HANDLE CallbackHandle;

/* The table of the protected processes */
RTL_AVL_TABLE ProcessTable;

_Use_decl_annotations_
extern "C"
NTSTATUS
SteroidsInitialize(
	struct _DRIVER_OBJECT* DriverObject
) noexcept {
	// Initialize device name
	UNICODE_STRING DeviceName = RTL_CONSTANT_STRING(L"\\Device\\Steroids");

	// Create device
	NTSTATUS Status = IoCreateDevice(DriverObject, 0, &DeviceName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, false, &DeviceObject);

	// Ensure the device is initialized successfully
	ASSERT(NT_SUCCESS(Status) /** IoCreateDevice */);
	if (!NT_SUCCESS(Status)) [[unlikely]] {
		return Status;
	}

	// Initialize symbolic link name
	UNICODE_STRING SymbolicLinkName = RTL_CONSTANT_STRING(L"\\??\\Steroids");

	// Create symbolic link
	Status = IoCreateSymbolicLink(&SymbolicLinkName, &DeviceName);

	// Ensures the symbolic link is created successfully
	ASSERT(NT_SUCCESS(Status) /** IoCreateSymbolicLink */);
	if (!NT_SUCCESS(Status)) [[unlikely]] {
		IoDeleteDevice(DeviceObject);
		return Status;
	}

	// Use buffered io when transmitting with application
	DeviceObject->Flags |= DO_BUFFERED_IO;

	// Initialize generic table
	RtlInitializeGenericTableAvl(&ProcessTable, &CompareRoutine, &AllocateRoutine, &FreeRoutine, nullptr);

	// Initialize OB_CALLBACK_REGISTRATION structure
	OB_CALLBACK_REGISTRATION CallbackRegistration{ 
		.Version = ObGetFilterVersion(),
		.OperationRegistrationCount = 2,
		.RegistrationContext = nullptr,
	};

	// Initialize Altitude property
	RtlInitUnicodeString(&CallbackRegistration.Altitude, L"0");

	OB_OPERATION_REGISTRATION OperationRegistration[]{
		{
			.ObjectType = PsProcessType,
			.Operations = OB_OPERATION_HANDLE_CREATE | OB_OPERATION_HANDLE_DUPLICATE,
			.PreOperation = &PreOperation,
		} ,
		{
			.ObjectType = PsThreadType,
			.Operations = OB_OPERATION_HANDLE_CREATE | OB_OPERATION_HANDLE_DUPLICATE,
			.PreOperation = &PreOperation,
		}
	};

	// Set-up the OperationRegistration property 
	CallbackRegistration.OperationRegistration = static_cast<OB_OPERATION_REGISTRATION*>(OperationRegistration);

	// Register object callback
	Status = ObRegisterCallbacks(&CallbackRegistration, &CallbackHandle);

	// Ensures the callback is registered successfully
	ASSERT(NT_SUCCESS(Status) /** ObRegisterCallbacks */);
	if (!NT_SUCCESS(Status)) [[unlikely]] {
		NT_VERIFY(NT_SUCCESS(IoDeleteSymbolicLink(&SymbolicLinkName)));
		IoDeleteDevice(DeviceObject);
		return Status;
	}

	return Status;
}

_Use_decl_annotations_
void
SteroidsFinalize() noexcept {
	ASSERT(DeviceObject != nullptr);
	ASSERT(KeGetCurrentIrql() <= PASSIVE_LEVEL);

	if (KeGetCurrentIrql() > PASSIVE_LEVEL) [[unlikely]]
	{
		return;
	}

	UNICODE_STRING SymbolicLinkName = RTL_CONSTANT_STRING(L"\\??\\Steroids");
	[[maybe_unused]] NTSTATUS const Status = IoDeleteSymbolicLink(&SymbolicLinkName);

	ASSERT(NT_SUCCESS(Status) /** IoDeleteSymbolicLink */);

	IoDeleteDevice(DeviceObject);

	// Determines if the callback handle is null
	// The driver may be unloaded without register callback successfully
	if (CallbackHandle != nullptr) [[unlikely]] {
		ObUnRegisterCallbacks(CallbackHandle);
	}
}

_Use_decl_annotations_
OB_PREOP_CALLBACK_STATUS 
PreOperation(
	PVOID RegistrationContext [[maybe_unused]], /** The context that the driver specifies as the CallBackRegistration->RegistrationContext parameter of 
													the ObRegisterCallbacks routine. The meaning of this value is driver-defined */
	POB_PRE_OPERATION_INFORMATION OperationInformation /** A pointer to an OB_PRE_OPERATION_INFORMATION structure that specifies the parameters of the handle operation */
) noexcept {
	if (OperationInformation->ObjectType == *PsProcessType) {
		// Cast the object to PEPROCESS
		PEPROCESS Process = static_cast<PEPROCESS>(OperationInformation->Object);

		// Get the process id of the process
		HANDLE ProcessId = PsGetProcessId(Process);

		// Determine if the process is being protected
		if (RtlLookupElementGenericTableAvl(&ProcessTable, &ProcessId)) [[unlikely]] {
			// Clear access bits
			OperationInformation->Parameters->CreateHandleInformation.DesiredAccess = 0;
		}
	}
	else if (OperationInformation->ObjectType == *PsThreadType) {
		// Cast object to PETHREAD
		PETHREAD Thread = static_cast<PETHREAD>(OperationInformation->Object);

		// Get the process id of the thread
		HANDLE ProcessId = PsGetThreadProcessId(Thread);

		// Determine if the process is being protected
		if (RtlLookupElementGenericTableAvl(&ProcessTable, &ProcessId)) [[unlikely]] {
			// Clear access bits
			OperationInformation->Parameters->CreateHandleInformation.DesiredAccess = 0;
		}
	}

	// We can only return success
	return OB_PREOP_CALLBACK_STATUS::OB_PREOP_SUCCESS;
}

_Use_decl_annotations_
RTL_GENERIC_COMPARE_RESULTS
#pragma warning(disable: 26429)
#pragma warning(disable: 26440)
CompareRoutine(
#pragma warning(default: 26440)
#pragma warning(default: 26429)
	struct _RTL_AVL_TABLE* Table [[maybe_unused]], /** A pointer to the avl table */
	PVOID FirstStruct, /** A pointer to the first item to be compared */
	PVOID SecondStruct /** A pointer to the second item to be compared */
) {
	// Cast pointer to integer to compare
	INT64 const FirstValue = *static_cast<PINT64>(FirstStruct);
	INT64 const SecondValue = *static_cast<PINT64>(SecondStruct);
	if (FirstValue == SecondValue) [[unlikely]] {
		return RTL_GENERIC_COMPARE_RESULTS::GenericEqual; // First value equals to second value
	}
	else if (FirstValue < SecondValue) {
		return RTL_GENERIC_COMPARE_RESULTS::GenericLessThan; // First value less than second value
	}
	else {
		return RTL_GENERIC_COMPARE_RESULTS::GenericGreaterThan; // First value greater than second value
	}
}

_Use_decl_annotations_
PVOID
AllocateRoutine(
	struct _RTL_AVL_TABLE* Table [[maybe_unused]], /** A pointer to the generic table */
	CLONG ByteSize /** The number of bytes to allocate */
) {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmultichar"
	return ExAllocatePool2(POOL_FLAG_NON_PAGED, ByteSize, 'VAL');
#pragma clang diagnostic pop
}

_Use_decl_annotations_
void
FreeRoutine(
	struct _RTL_AVL_TABLE* Table [[maybe_unused]], /** A pointer to the generic table */
	PVOID Buffer /** A pointer to the element that is being deleted */
) {
	// Free memory
	ExFreePool(Buffer);
}