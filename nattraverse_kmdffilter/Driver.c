/*++

Module Name:

    driver.c

Abstract:

    This file contains the driver entry points and callbacks.

Environment:

    Kernel-mode Driver Framework

--*/

#include "precomp.h"
#include "driver.h"
#include "driver.tmh"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#pragma alloc_text (PAGE, nattraverse_kmdffilterEvtDriverUnload)
#pragma alloc_text (PAGE, nattraverse_kmdffilterEvtDriverContextCleanup)
#pragma alloc_text (PAGE, nattraverse_kmdffilterEvtDeviceAdd)
#endif

NTSTATUS
DriverEntry(
    _In_ PDRIVER_OBJECT  DriverObject,
    _In_ PUNICODE_STRING RegistryPath
    )
/*++

Routine Description:
    DriverEntry initializes the driver and is the first routine called by the
    system after the driver is loaded. DriverEntry specifies the other entry
    points in the function driver, such as EvtDevice and DriverUnload.

Parameters Description:

    DriverObject - represents the instance of the function driver that is loaded
    into memory. DriverEntry must initialize members of DriverObject before it
    returns to the caller. DriverObject is allocated by the system before the
    driver is loaded, and it is released by the system after the system unloads
    the function driver from memory.

    RegistryPath - represents the driver specific path in the Registry.
    The function driver can use the path to store driver related data between
    reboots. The path does not store hardware instance specific data.

Return Value:

    STATUS_SUCCESS if successful,
    STATUS_UNSUCCESSFUL otherwise.

--*/
{
    WDF_DRIVER_CONFIG       config;
    NTSTATUS                status;
    WDF_OBJECT_ATTRIBUTES   attributes;
    PWDFDEVICE_INIT         pInit = NULL;
    WDFDRIVER               hDriver;
    //
    // Initialize WPP Tracing
    //
    WPP_INIT_TRACING( DriverObject, RegistryPath );
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Entry");
    //
    // Register a cleanup callback so that we can call WPP_CLEANUP when
    // the framework driver object is deleted during driver unload.
    //
    WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
    attributes.EvtCleanupCallback = nattraverse_kmdffilterEvtDriverContextCleanup;
    WDF_DRIVER_CONFIG_INIT(&config,
                           WDF_NO_EVENT_CALLBACK // This is a non-pnp driver.
                           );
    //
    // Tell the framework that this is non-pnp driver so that it doesn't
    // set the default AddDevice routine.
    //
    config.DriverInitFlags |= WdfDriverInitNonPnpDriver;
    //
    // NonPnp driver must explicitly register an unload routine for
    // the driver to be unloaded.
    //
    config.EvtDriverUnload = nattraverse_kmdffilterEvtDriverUnload;
    status = WdfDriverCreate(DriverObject,
                             RegistryPath,
                             &attributes,
                             &config,
                             &hDriver
                             );
    if (!NT_SUCCESS(status)) {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "WdfDriverCreate failed %!STATUS!", status);
        WPP_CLEANUP(DriverObject);
        return status;
    }
    //
    //
    // In order to create a control device, we first need to allocate a
    // WDFDEVICE_INIT structure and set all properties.
    //
    pInit = WdfControlDeviceInitAllocate(
                            hDriver,
                            &SDDL_DEVOBJ_SYS_ALL_ADM_RWX_WORLD_RW_RES_R
                            );
    if (pInit == NULL)
    {
        status = STATUS_INSUFFICIENT_RESOURCES;
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "WdfControlDeviceInitAllocate failed %!STATUS!", status);
        WPP_CLEANUP(DriverObject);
        return status;
    }
    KeInitializeSpinLock(&g_nFilterDataLock);
    //
    // Call NonPnpDeviceAdd to create a deviceobject to represent our
    // software device.
    //
    status = nattraverse_kmdffilterEvtDeviceAdd(hDriver, pInit);
    if (!NT_SUCCESS(status))
    {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "nattraverse_kmdffilterEvtDeviceAdd failed %!STATUS!", status);
        WPP_CLEANUP(DriverObject);
        return status;
    }
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Exit");
    return status;
}

VOID
nattraverse_kmdffilterEvtDriverContextCleanup(
    _In_ WDFOBJECT DriverObject
    )
/*++
Routine Description:

    Free all the resources allocated in DriverEntry.

Arguments:

    DriverObject - handle to a WDF Driver object.

Return Value:

    VOID.

--*/
{
    NTSTATUS    status;
    KLOCK_QUEUE_HANDLE  stFilterDataLock;
    UNREFERENCED_PARAMETER(DriverObject);
    PAGED_CODE ();
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Entry");
    KeAcquireInStackQueuedSpinLock(
        &g_nFilterDataLock,
        &stFilterDataLock
        );
    if(NULL != g_pFilterInitStruct)
    {
        FilterDataCleanup();
    }
    if(0 != g_nIndoundIpPacketId)
    {
        status = FwpsCalloutUnregisterById0(g_nIndoundIpPacketId);
        g_nIndoundIpPacketId = 0;
    }
    if(0 != g_nIpForwardId)
    {
        status = FwpsCalloutUnregisterById0(g_nIpForwardId);
        g_nIpForwardId = 0;
    }
    KeReleaseInStackQueuedSpinLock(&stFilterDataLock);
    //
    // Stop WPP Tracing
    //
    WPP_CLEANUP( WdfDriverWdmGetDriverObject(DriverObject) );

}

VOID
nattraverse_kmdffilterEvtDriverUnload(
    IN WDFDRIVER Driver
    )
/*++
Routine Description:

   Called by the I/O subsystem just before unloading the driver.
   You can free the resources created in the DriverEntry either
   in this routine or in the EvtDriverContextCleanup callback.

Arguments:

    Driver - Handle to a framework driver object created in DriverEntry

Return Value:

    NTSTATUS

--*/
{
    NTSTATUS    status;
    KLOCK_QUEUE_HANDLE  stFilterDataLock;
    UNREFERENCED_PARAMETER(Driver);
    PAGED_CODE();
    TraceEvents(TRACE_LEVEL_VERBOSE, TRACE_DRIVER, "Entered nattraverse_kmdffilterEvtDriverUnload\n");
    KeAcquireInStackQueuedSpinLock(
        &g_nFilterDataLock,
        &stFilterDataLock
        );
    if(NULL != g_pFilterInitStruct)
    {
        FilterDataCleanup();
    }
    if(0 != g_nIndoundIpPacketId)
    {
        status = FwpsCalloutUnregisterById0(g_nIndoundIpPacketId);
        g_nIndoundIpPacketId = 0;
    }
    if(0 != g_nIpForwardId)
    {
        status = FwpsCalloutUnregisterById0(g_nIpForwardId);
        g_nIpForwardId = 0;
    }
    KeReleaseInStackQueuedSpinLock(&stFilterDataLock);
    return;
}

NTSTATUS
nattraverse_kmdffilterEvtDeviceAdd(
    _In_    WDFDRIVER       Driver,
    _Inout_ PWDFDEVICE_INIT DeviceInit
    )
/*++
Routine Description:

    Called by the DriverEntry to create a control-device. This call is
    responsible for freeing the memory for DeviceInit.

Arguments:

    DriverObject - a pointer to the object that represents this device
    driver.

    DeviceInit - Pointer to a driver-allocated WDFDEVICE_INIT structure.

Return Value:

    NTSTATUS

--*/
{
    NTSTATUS status;
    UNREFERENCED_PARAMETER(Driver);
    PAGED_CODE();
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Entry");
    status = nattraverse_kmdffilterCreateDevice(DeviceInit);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Exit");
    return status;
}
