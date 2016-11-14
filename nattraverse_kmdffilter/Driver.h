/*++

Module Name:

    driver.h

Abstract:

    This file contains the driver definitions.

Environment:

    Kernel-mode Driver Framework

--*/

#pragma once

#include "device.h"
#include "queue.h"
#include "trace.h"
#include "nattraverse_filter.h"

//
// WDFDRIVER Events
//

DRIVER_INITIALIZE DriverEntry;
EVT_WDF_DRIVER_UNLOAD                   nattraverse_kmdffilterEvtDriverUnload;
EVT_WDF_OBJECT_CONTEXT_CLEANUP          nattraverse_kmdffilterEvtDriverContextCleanup;
//
// Don't use EVT_WDF_DRIVER_DEVICE_ADD for NonPnpDeviceAdd even though 
// the signature is same because this is not an event called by the 
// framework.
//
NTSTATUS
nattraverse_kmdffilterEvtDeviceAdd(
    IN WDFDRIVER Driver,
    IN PWDFDEVICE_INIT DeviceInit
    );
