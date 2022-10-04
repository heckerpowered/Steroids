#include "pch.hpp"
#include "Bootsrap.hpp"

#if defined(ALLOC_PRAGMA)
#pragma alloc_text(INIT, SteroidsInitialize)
#endif

PDEVICE_OBJECT DeviceObject;

_Use_decl_annotations_
extern "C"
NTSTATUS const
SteroidsInitialize(
	struct _DRIVER_OBJECT* DriverObject
) noexcept {
	ASSERT(DriverObject != nullptr && MmIsAddressValid(DriverObject));
	ASSERT(KeGetCurrentIrql() <= PASSIVE_LEVEL);

	if (DriverObject == nullptr || !MmIsAddressValid(DriverObject)) [[unlikely]] {
		return STATUS_INVALID_PARAMETER;
	}

	if (KeGetCurrentIrql() > PASSIVE_LEVEL) [[unlikely]] {
		return IRQL_NOT_LESS_OR_EQUAL;
	}

	UNICODE_STRING deviceName = RTL_CONSTANT_STRING(L"\\Device\\Steroids");
	NTSTATUS status = IoCreateDevice(DriverObject, 0, &deviceName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, false, &DeviceObject);

	ASSERT(NT_SUCCESS(status));
	if (!NT_SUCCESS(status)) [[unlikely]] {
		return status;
	}

	UNICODE_STRING symbolicLinkName = RTL_CONSTANT_STRING(L"\\??\\Steroids");
	status = IoCreateSymbolicLink(&symbolicLinkName, &deviceName);

	ASSERT(NT_SUCCESS(status));
	if (!NT_SUCCESS(status)) [[unlikely]] {
		return status;
	}

	DeviceObject->Flags |= DO_BUFFERED_IO;
	DeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

	return status;
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
}