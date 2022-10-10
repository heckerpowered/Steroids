#include "../../Public/Core.h"
#include "../../Public/Bootstrap/Bootsrap.h"

#if defined(ALLOC_PRAGMA)
#pragma alloc_text(INIT, SteroidsInitialize)
#endif

/** The device object */
PDEVICE_OBJECT DeviceObject;

/*& The handle of object callback */
HANDLE CallbackHandle;

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

	// Initialize OB_CALLBACK_REGISTRATION structure
	OB_CALLBACK_REGISTRATION CallbackRegistration{ 
		.Version = ObGetFilterVersion(),
		.OperationRegistrationCount = 1,
		.RegistrationContext = nullptr,
	};

	// Initialize Altitude property
	RtlInitUnicodeString(&CallbackRegistration.Altitude, L"0");

	// Initialize OB_OPERATION_REGISTRATION structure
	OB_OPERATION_REGISTRATION OperationRegistration{
		.ObjectType = PsProcessType,
		.Operations = OB_OPERATION_HANDLE_CREATE | OB_OPERATION_HANDLE_DUPLICATE,
		.PreOperation = &PreOperation,
	};

	// Set-up the OperationRegistration property 
	CallbackRegistration.OperationRegistration = &OperationRegistration;

	// Register object callback
	Status = ObRegisterCallbacks(&CallbackRegistration, &CallbackHandle);

	// Ensures the callback is registered successfully
	ASSERT(NT_SUCCESS(Status));
	if (!NT_SUCCESS(Status)) [[unlikely]] {
		return Status;
	}

	RtlInitializeGenericTable();

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