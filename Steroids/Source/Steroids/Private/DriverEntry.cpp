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

#include "..\Public\DriverEntry.h"
#include "..\Public\Bootstrap\Bootsrap.h"
#include "..\Public\Transmit\Transmit.h"

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

#pragma warning(disable: 28175) // We have to modify flags in Driver Section to make sure that 
                                // we can register object callbacks without certificates
    // Get Driver Section
    PLDR_DATA_TABLE_ENTRY64 DriverSection = static_cast<PLDR_DATA_TABLE_ENTRY64>(DriverObject->DriverSection);
#pragma warning(default: 28175)

    ULONG const PreviousFlag = DriverSection->Flags;

    // Make sure that Object callback registration won't fail
    DriverSection->Flags |= 0x20;

    // Initialize device in order to transmit with application
    NTSTATUS const Status = SteroidsInitialize(DriverObject);

    // Restore flags to previous flags
    DriverSection->Flags = PreviousFlag;

    // Ensures the device is initialized successfully
    if (!NT_VERIFY(NT_SUCCESS(Status) /** SteroidsInitialize */)) {
        // Returning an non-successful status code will cause the driver to be unloaded
        // even if there is no unload functionn
        return Status;
    }
    
    // Set-up create major function so that the application can get the handle of the driver
    DriverObject->MajorFunction[IRP_MJ_CREATE] =
        DriverObject->MajorFunction[IRP_MJ_CLOSE] = &DefaultDispatcher;

    // Set-up device control function so that the application can transmit with the driver via IoDeviceControl function
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = &DispatchDeviceControl;

    // The driver is loaded successfully
    return Status;
}

_Use_decl_annotations_
void
#pragma warning(disable: 26440) // DriverUnload cannot be declared 'noexcept'
DriverUnload(
#pragma warning(default: 26440)
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
) {
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
) {
    // Get the current irp stack location
    PIO_STACK_LOCATION IoStackLocation = IoGetCurrentIrpStackLocation(Irp);

    // Get the Io control code
    ULONG const IoControlCode = IoStackLocation->Parameters.DeviceIoControl.IoControlCode;

    // Get the input buffer length
    ULONG const InputBufferLength = IoStackLocation->Parameters.DeviceIoControl.InputBufferLength;

    // Get the output buffer length, declare as a reference in order to modify
    ULONG& OutputBufferLength = IoStackLocation->Parameters.DeviceIoControl.OutputBufferLength;

    // Exract the transmit method
    switch (IoControlCode & 3) {
    case METHOD_BUFFERED:
        Irp->IoStatus.Status = HandleTransmit(IoControlCode, InputBufferLength, OutputBufferLength, Irp->AssociatedIrp.SystemBuffer, Irp->AssociatedIrp.SystemBuffer);
        break;

    case METHOD_IN_DIRECT:
    case METHOD_OUT_DIRECT:
#pragma warning(disable: 6387) // HandleTransmit function will fail when input buffer or output buffer is null
#pragma warning(disable: 26476)
        Irp->IoStatus.Status = HandleTransmit(IoControlCode, InputBufferLength, OutputBufferLength, Irp->AssociatedIrp.SystemBuffer, 
            Irp->MdlAddress ? MmGetSystemAddressForMdlSafe(Irp->MdlAddress, MM_PAGE_PRIORITY::NormalPagePriority | MdlMappingNoExecute) : nullptr);
#pragma warning(default: 6387)
#pragma warning(default: 26476)
        break;

    case METHOD_NEITHER:
        Irp->IoStatus.Status = HandleTransmit(IoControlCode, InputBufferLength, OutputBufferLength, IoStackLocation->Parameters.DeviceIoControl.Type3InputBuffer, Irp->UserBuffer);
        break;

    default:
        // Invalid transmit method
        Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
        break;
    }

    // Set the Information field of the input IRP's I/O status block to zero.
    Irp->IoStatus.Information = 0;

    // Call IoCompleteRequest with the IRP and a PriorityBoost of IO_NO_INCREMENT.
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    // Return the NTSTATUS that it set in the Status field of the IRP's I/O status block.
    return Irp->IoStatus.Status;
}