#include "../../Public/Core.h"
#include "../../Public/Bootstrap/Bootsrap.h"

#if defined(ALLOC_PRAGMA)
#pragma alloc_text(INIT, SteroidsInitialize)
#endif

/** The device object */
PDEVICE_OBJECT DeviceObject;

HANDLE CallbackHandle;

/* The table of the protected processes */
RTL_AVL_TABLE Table;

_Use_decl_annotations_
extern "C"
NTSTATUS const
SteroidsInitialize(
	struct _DRIVER_OBJECT* DriverObject
) noexcept {
	// Ensure Driver Object is not null and valid
	ASSERT(DriverObject != nullptr && MmIsAddressValid(DriverObject));

	// Ensure current irql is less than or equal to passive level
	ASSERT(KeGetCurrentIrql() <= PASSIVE_LEVEL);

	// ASSERT macro only applicable in debug configuration
	if (DriverObject == nullptr || !MmIsAddressValid(DriverObject)) [[unlikely]] {
		return STATUS_INVALID_PARAMETER;
	}

	if (KeGetCurrentIrql() > PASSIVE_LEVEL) [[unlikely]] {
		return IRQL_NOT_LESS_OR_EQUAL;
	}

	// Initialize device name
	UNICODE_STRING DeviceName = RTL_CONSTANT_STRING(L"\\Device\\Steroids");

	// Create device
	NTSTATUS Status = IoCreateDevice(DriverObject, 0, &DeviceName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, false, &DeviceObject);

	// Ensure the device is initialized successfully
	ASSERT(NT_SUCCESS(Status));
	if (!NT_SUCCESS(Status)) [[unlikely]] {
		return Status;
	}

	// Initialize symbolic link name
	UNICODE_STRING SymbolicLinkName = RTL_CONSTANT_STRING(L"\\??\\Steroids");

	// Create symbolic link
	Status = IoCreateSymbolicLink(&SymbolicLinkName, &DeviceName);

	// Ensures the symbolic link is created successfully
	ASSERT(NT_SUCCESS(Status));
	if (!NT_SUCCESS(Status)) [[unlikely]] {
		return Status;
	}

	// Use buffered io when transmitting with application
	DeviceObject->Flags |= DO_BUFFERED_IO;

	// Initialize generic table
	RtlInitializeGenericTableAvl(&Table, &CompareRoutine, &AllocateRoutine, &FreeRoutine, nullptr);

	// Initialize OB_CALLBACK_REGISTRATION structure
	OB_CALLBACK_REGISTRATION CallbackRegistration{ 
		.Version = ObGetFilterVersion(),
		.OperationRegistrationCount = 1,
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

	// Register object callback
	Status = ObRegisterCallbacks(&CallbackRegistration, &CallbackHandle);

	// Ensures the callback is registered successfully
	ASSERT(NT_SUCCESS(Status));
	if (!NT_SUCCESS(Status)) [[unlikely]] {
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
		KeLowerIrql(PASSIVE_LEVEL);
	}

	UNICODE_STRING symbolicLinkName = RTL_CONSTANT_STRING(L"\\??\\Steroids");
	[[maybe_unused]] NTSTATUS const status = IoDeleteSymbolicLink(&symbolicLinkName);

	ASSERT(NT_SUCCESS(status));

	IoDeleteDevice(DeviceObject);

	// Determines if the callback handle is null
	// The driver may be unloaded without register callback successfully
	if (CallbackHandle != nullptr) [[unlikely]] {
		ObUnRegisterCallbacks(CallbackHandle);
	}
}

_Use_decl_annotations_
OB_PREOP_CALLBACK_STATUS PreOperation(
	PVOID RegistrationContext, /** The context that the driver specifies as the CallBackRegistration->RegistrationContext 
									parameter of the ObRegisterCallbacks routine. The meaning of this value is driver-defined */
	POB_PRE_OPERATION_INFORMATION OperationInformation /** A pointer to an OB_PRE_OPERATION_INFORMATION structure that specifies the parameters of the handle operation */
) noexcept {
	if (OperationInformation->ObjectType == *PsProcessType) {
		// Cast the object to PEPROCESS
		PEPROCESS Process = static_cast<PEPROCESS>(OperationInformation->Object);

		// Get the process id of the process
		HANDLE ProcessId = PsGetProcessId(Process);

		// Determine if the process is being protected
		if (RtlLookupElementGenericTableAvl(&Table, &ProcessId)) [[unlikely]] {
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
		if (RtlLookupElementGenericTableAvl(&Table, &ProcessId)) [[unlikely]] {
			// Clear access bits
			OperationInformation->Parameters->CreateHandleInformation.DesiredAccess = 0;
		}
	}

	// We can only return success
	return OB_PREOP_CALLBACK_STATUS::OB_PREOP_SUCCESS;
}

_Use_decl_annotations_
RTL_GENERIC_COMPARE_RESULTS
CompareRoutine(
	struct _RTL_AVL_TABLE* Table, /** A pointer to the avl table */
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
	struct _RTL_AVL_TABLE* Table, /** A pointer to the generic table */
	CLONG ByteSize /** The number of bytes to allocate */
) {
	return ExAllocatePool2(POOL_FLAG_NON_PAGED, ByteSize, 'VAL');
}

_Use_decl_annotations_
void
FreeRoutine(
	struct _RTL_AVL_TABLE* Table, /** A pointer to the generic table */
	PVOID Buffer /** A pointer to the element that is being deleted */
) {
	// Free memory
	ExFreePool(Buffer);
}