/*
* Copyright (c) Heckerpowered Corporation.
* Licensed under the MIT license.
*/

#include "..\Public\Core.h"
#include "..\Public\DriverEntry.h"
#include "..\Public\Bootstrap\Bootsrap.h"

#if defined(ALLOC_PRAGMA)
#pragma alloc_text(INIT, DriverEntry)
#endif

_Use_decl_annotations_
extern "C"
NTSTATUS
DriverEntry(
    struct _DRIVER_OBJECT* DriverObject,
    PUNICODE_STRING RegistryPath [[maybe_unused]]
) {
    // Ensures the current thread is running at an IRQL that is low enough to permit paging
    // Also allows this function to be in the INIT code section
    PAGED_CODE();

#ifdef DBG
    // Set-up an breakpoint in the driver entry 
    __debugbreak();
#endif

    // Set-up the unload function to make the driver able to be unload
    DriverObject->DriverUnload = DriverUnload;

    // Request NX Non-Paged Pool when available
    ExInitializeDriverRuntime(DRIVER_RUNTIME_INIT_FLAGS::DrvRtPoolNxOptIn);

    // Initialize device in order to transmit with application
    NTSTATUS const Status = SteroidsInitialize(DriverObject);

    // Ensures the device is initialized successfully
    ASSERT(NT_SUCCESS(Status));

    // ASSERT macro only applicable in debug configuration
    if (!NT_SUCCESS(Status)) {
        // Returning an non-successful status code will cause the driver to be unloaded
        // even if there is no unload functionn
        return Status;
    }

    // Set-up create major function so that the application can get the handle of the driver
    DriverObject->MajorFunction[IRP_MJ_CREATE] =
        DriverObject->MajorFunction[IRP_MJ_CLOSE] = &DefaultDispatcher;

    // Set-up device control function so that the application can transmit with the driver via IoDeviceControl function
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = &DispatchDeviceControl;

    // Ensure callback registration won't fail by modifying the flags
    static_cast<PLDR_DATA_TABLE_ENTRY64>(DriverObject->DriverSection)->Flags |= 0x20;

    // The driver is loaded successfully
    return Status;
}

_Use_decl_annotations_
void
DriverUnload(
    struct _DRIVER_OBJECT* DriverObject [[maybe_unused]]
) {
    // Delete symbolic link and device
    SteroidsFinalize();
}

_Use_decl_annotations_
NTSTATUS
DefaultDispatcher(
    struct _DEVICE_OBJECT* DeviceObject [[maybe_unused]],
    struct _IRP* Irp
) noexcept
{
    // Set the Status field of the input IRP's I/O status block with an appropriate NTSTATUS, usually STATUS_SUCCESS.
    Irp->IoStatus.Status = STATUS_SUCCESS;

    // Set the Information field of the input IRP's I/O status block to zero.
    Irp->IoStatus.Information = 0;

    // Call IoCompleteRequest with the IRP and a PriorityBoost of IO_NO_INCREMENT.
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    // Return the NTSTATUS that it set in the Status field of the IRP's I/O status block.
    return Irp->IoStatus.Status;
}

_Use_decl_annotations_
NTSTATUS
DispatchDeviceControl(
    struct _DEVICE_OBJECT* DeviceObject [[maybe_unused]],
    struct _IRP* Irp
) noexcept
{
    // Set the Status field of the input IRP's I/O status block with an appropriate NTSTATUS, usually STATUS_SUCCESS.
    Irp->IoStatus.Status = STATUS_SUCCESS;

    // Set the Information field of the input IRP's I/O status block to zero.
    Irp->IoStatus.Information = 0;

    // Call IoCompleteRequest with the IRP and a PriorityBoost of IO_NO_INCREMENT.
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    // Return the NTSTATUS that it set in the Status field of the IRP's I/O status block.
    return Irp->IoStatus.Status;
}