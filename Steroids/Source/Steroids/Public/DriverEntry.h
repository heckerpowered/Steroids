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

#include "Core.h"

//
// DriverEntry Routine:
// 
//  Each driver must have a DriverEntry routine, which initializes driver-wide data structures and resources.
//  The I/O manager calls the DriverEntry routine when it loads the driver.
// 
//	In a driver that supports Plug and Play (PnP), as all drivers should, the DriverEntry routine is responsible for driver
//  initialization, while the AddDevice routine (and, possibly, the dispatch routine that handles a PnP IRP_MN_START_DEVICE
//  request) is responsible for device initialization. Driver initialization includes exporting the driver's other entry points,
//  initializing certain objects the driver uses, and setting up various per-driver system resources. (Non-PnP drivers have
//  significantly different requirements, as described in the Driver Development Kit [DDK] for Microsoft Windows NT 4.0 and
//  earlier.)
// 
//  DriverEntry routines are called in the context of a system thread at IRQL = PASSIVE_LEVEL.
// 
//  A DriverEntry routine can be pageable and should be in an INIT segment so it will be discarded. Use an alloc_text pragma
//  directive.
// 
// DriverEntry's Required Responsibilities:
// 
//	The required, ordered responsibilities of a DriverEntry routine are as follows:
// 
//		1. Supply entry points for the driver's standard routines.
// 
//			The driver stores entry points for many of its standard routines in the driver object or driver extension. Such entry
//			points include those for the driver's AddDevice routine, dispatch routines, StartIo routine, and Unload routine. For
//			example, a driver would set the entry points for its AddDevice, DispatchPnP, and DispatchPower routines with
//			statements like the following (Xxx is a placeholder for a vendor-supplied prefix identifying the driver):
// 
//			:
//			DriverObject->DriverExtension->AddDevice = XxxAddDevice;
//			DriverObject->MajorFunction[IRP_MJ_PNP] = XxxDispatchPnp;
//			DriverObject->MajorFunction[IRP_MJ_POWER] = XxxDispatchPower;
//			:
// 
//			Additional standard routines, such as ISRs or IoCompletion routines, are specified by calling system support routines.
// 
//		2. Create and/or initialize various driver-wide objects, types, or resources the driver uses. Note that most standard
//		   routines use objects on a per-device basis, so drivers should set up such objects in their AddDevice routines or after
//		   receiving an IRP_MN_START_DEVICE request.
// 
//		   If the driver has a device-dedicated thread or waits on any kernel-defined dispatcher objects, the DriverEntry
//		   routine might initialize kernel dispatcher objects. (Depending on how the driver uses the object(s), it might instead
//		   perform this task in its AddDevice routine or after receiving an IRP_MN_START_DEVICE request.)
// 
//		3. Free any memory that it allocated and is no longer required.
// 
//		4. Return NTSTATUS indicating whether the driver successfully loaded and can accept and process requests from the
//		   PnP manager to configure, add, and start its devices.
// 
//

/** Called when the driver is being loaded */
extern "C" DRIVER_INITIALIZE DriverEntry;

/** Called when the driver is being unloaded */
DRIVER_UNLOAD DriverUnload;

/** Called when the application created or closed the file */
DRIVER_DISPATCH DefaultDispatcher;

/** Called when the application calls the IoDeviceControl */
DRIVER_DISPATCH DispatchDeviceControl;