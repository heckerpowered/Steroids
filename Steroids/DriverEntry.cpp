/*
* Copyright (c) Heckerpowered Corporation.
* Licensed under the MIT license.
*/

#include "pch.hpp"
#include "DriverEntry.hpp"
#include "Bootsrap.hpp"

#if defined(ALLOC_PRAGMA)
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(INIT, DriverUnload)
#endif

_Use_decl_annotations_
extern "C"
NTSTATUS
DriverEntry(
    struct _DRIVER_OBJECT* DriverObject,
    PUNICODE_STRING RegistryPath [[maybe_unused]]
) {
    PAGED_CODE();

#ifdef DBG
    __debugbreak();
#endif

    DriverObject->DriverUnload = DriverUnload;

    // Request NX Non-Paged Pool when available
    ExInitializeDriverRuntime(DRIVER_RUNTIME_INIT_FLAGS::DrvRtPoolNxOptIn);

    NTSTATUS const status = SteroidsInitialize(DriverObject);

    return STATUS_SUCCESS;
}

_Use_decl_annotations_
extern "C"
void
DriverUnload(
    struct _DRIVER_OBJECT* DriverObject [[maybe_unused]]
) {
    SteroidsFinalize();
}