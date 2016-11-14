/*++

Module Name:

    device.h

Abstract:

    This file contains the device definitions.

Environment:

    Kernel-mode Driver Framework

--*/

#pragma once

#include "../include/nattraverse_public.h"

#define NATTRAVERSE_DEVICE_NAME   L"\\Device\\nattraverse_kmdffilter"
#define NATTRAVERSE_SYMBOLIC_NAME L"\\DosDevices\\Global\\nattraverse_kmdffilter"
#define POOL_TAG                  'NATF'

//
// The device context performs the same job as
// a WDM device extension in the driver frameworks
//
typedef struct _DEVICE_CONTEXT
{
    ULONG PrivateDeviceData;  // just a placeholder

} DEVICE_CONTEXT, *PDEVICE_CONTEXT;

//
// This macro will generate an inline function called WdfObjectGet_DEVICE_CONTEXT
// which will be used to get a pointer to the device context memory
// in a type safe manner.
//
WDF_DECLARE_CONTEXT_TYPE(DEVICE_CONTEXT)

//
// Function to initialize the device and its callbacks
//
NTSTATUS
nattraverse_kmdffilterCreateDevice(
    _Inout_ PWDFDEVICE_INIT DeviceInit
    );

EVT_WDF_DEVICE_SHUTDOWN_NOTIFICATION    nattraverse_kmdffilterShutdown;
