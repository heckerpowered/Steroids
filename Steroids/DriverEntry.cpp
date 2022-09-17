/*
* Copyright (c) Heckerpowered Corporation.
* Licensed under the MIT license.
*/

#include "DriverEntry.hpp"

#if defined(ALLOC_PRAGMA)
#pragma alloc_text(INIT, DriverEntry)
#endif

_Use_decl_annotations_
extern "C"
NTSTATUS
DriverEntry(
    struct _DRIVER_OBJECT* DriverObject [[maybe_unused]],
    PUNICODE_STRING RegistryPath [[maybe_unused]]
) {
    PAGED_CODE();

#ifdef DBG
    __debugbreak();
#endif

    DriverObject->DriverUnload = DriverUnload;

    //
    // Request NX Non-Paged Pool when available
    //
    ExInitializeDriverRuntime(DRIVER_RUNTIME_INIT_FLAGS::DrvRtPoolNxOptIn);

    return STATUS_SUCCESS;
}

_Use_decl_annotations_
void
DriverUnload(
    struct _DRIVER_OBJECT* DriverObject [[maybe_unused]]
) {

}