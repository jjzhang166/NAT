/*++

Module Name:

    queue.c

Abstract:

    This file contains the queue entry points and callbacks.

Environment:

    Kernel-mode Driver Framework

--*/

#include "precomp.h"
#include "driver.h"
#include "queue.tmh"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, nattraverse_kmdffilterQueueInitialize)
#endif

NTSTATUS
nattraverse_kmdffilterQueueInitialize(
    _In_ WDFDEVICE Device
    )
/*++

Routine Description:


     The I/O dispatch callbacks for the frameworks device object
     are configured in this function.

     A single default I/O Queue is configured for parallel request
     processing, and a driver context memory allocation is created
     to hold our structure QUEUE_CONTEXT.

Arguments:

    Device - Handle to a framework device object.

Return Value:

    VOID

--*/
{
    WDFQUEUE            queue;
    NTSTATUS            status;
    WDF_IO_QUEUE_CONFIG queueConfig;
    PAGED_CODE();
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_QUEUE, "%!FUNC! Entry");
    //
    // Configure a default queue so that requests that are not
    // configure-fowarded using WdfDeviceConfigureRequestDispatching to goto
    // other queues get dispatched here.
    //
    WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(
         &queueConfig,
        WdfIoQueueDispatchParallel
        );
    queueConfig.EvtIoDeviceControl = nattraverse_kmdffilterEvtIoDeviceControl;
    status = WdfIoQueueCreate(
                 Device,
                 &queueConfig,
                 WDF_NO_OBJECT_ATTRIBUTES,
                 &queue
                 );
    if( !NT_SUCCESS(status) )
    {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_QUEUE, "WdfIoQueueCreate failed %!STATUS!", status);
        return status;
    }
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_QUEUE, "%!FUNC! Exit");
    return status;
}

VOID
nattraverse_kmdffilterEvtIoDeviceControl(
    _In_ WDFQUEUE Queue,
    _In_ WDFREQUEST Request,
    _In_ size_t OutputBufferLength,
    _In_ size_t InputBufferLength,
    _In_ ULONG IoControlCode
    )
/*++

Routine Description:

    This event is invoked when the framework receives IRP_MJ_DEVICE_CONTROL request.

Arguments:

    Queue -  Handle to the framework queue object that is associated with the
             I/O request.

    Request - Handle to a framework request object.

    OutputBufferLength - Size of the output buffer in bytes

    InputBufferLength - Size of the input buffer in bytes

    IoControlCode - I/O control code.

Return Value:

    VOID

--*/
{
    NTSTATUS    status = STATUS_SUCCESS;
    ULONG_PTR   ulInformation = 0;
    KLOCK_QUEUE_HANDLE  stFilterDataLock;
    TraceEvents(TRACE_LEVEL_INFORMATION, 
                TRACE_QUEUE, 
                "!FUNC! Queue 0x%p, Request 0x%p OutputBufferLength %d InputBufferLength %d IoControlCode %d", 
                Queue, Request, (int) OutputBufferLength, (int) InputBufferLength, IoControlCode);
    switch(IoControlCode)
    {
    case NATTRAVERSE_IOCTL_INIT_FILTERS:
        {
            int     nRequestedSize;
            PBYTE   pCurrentPosition;
            PFILTER_INIT_STRUCT pFilterInitStructure;
            if(InputBufferLength < sizeof(FILTER_INIT_STRUCT))
            {
                status = STATUS_BUFFER_TOO_SMALL;
                TraceEvents(
                    TRACE_LEVEL_ERROR,
                    TRACE_QUEUE,
                    "Input Buffer Length = %d too small",
                    (int)InputBufferLength
                    );
                break;
            }
            KeAcquireInStackQueuedSpinLock(
                &g_nFilterDataLock,
                &stFilterDataLock
                );
            if(NULL != g_pFilterInitStruct)
            {
                FilterDataCleanup();
            }
            status = WdfRequestRetrieveInputBuffer(
                Request,
                InputBufferLength,
                (PVOID*)&pFilterInitStructure,
                NULL
                );
            if (!NT_SUCCESS(status)) {
                TraceEvents(
                    TRACE_LEVEL_ERROR,
                    TRACE_QUEUE,
                    "WdfRequestRetrieveInputBuffer failed %!STATUS!",
                    status
                    );
                KeReleaseInStackQueuedSpinLock(&stFilterDataLock);
                break;
            }
            nRequestedSize = sizeof(FILTER_INIT_STRUCT);
            if(nRequestedSize > (int)InputBufferLength)
            {
                status = STATUS_INVALID_PARAMETER;
                TraceEvents(
                    TRACE_LEVEL_ERROR,
                    TRACE_QUEUE,
                    "Requested buffer size greater then Input Buffer Length, parameter is invalid."
                    );
                KeReleaseInStackQueuedSpinLock(&stFilterDataLock);
                break;
            }
            nRequestedSize += pFilterInitStructure->_nSinglePublicIpCount*sizeof(UINT32)
                + pFilterInitStructure->_nSinglePrivateIpCount*sizeof(UINT32)
                + pFilterInitStructure->_nPublicRangeIpCount*sizeof(IP4ADDR_RANGE_STRUCT)
                + pFilterInitStructure->_nPrivateRangeIpCount*sizeof(IP4ADDR_RANGE_STRUCT)
                + pFilterInitStructure->_nPublicSubnetIpCount*sizeof(IP4SUBNET_STRUCT)
                + pFilterInitStructure->_nPrivateSubnetIpCount*sizeof(IP4SUBNET_STRUCT)
                + pFilterInitStructure->_nSinglePortCount*sizeof(UINT16)
                + pFilterInitStructure->_nRangePortCount*sizeof(IP_PORT_RANGE_STRUCT);
            if(nRequestedSize != (int)InputBufferLength)
            {
                status = STATUS_INVALID_PARAMETER;
                TraceEvents(
                    TRACE_LEVEL_ERROR,
                    TRACE_QUEUE,
                    "Requested buffer size not equel to Input Buffer Length, parameter is invalid."
                    );
                KeReleaseInStackQueuedSpinLock(&stFilterDataLock);
                break;
            }
#pragma warning( suppress : 28197 )
            g_pFilterInitStruct = (PFILTER_INIT_STRUCT)ExAllocatePoolWithTag(
                NonPagedPool,
                nRequestedSize,
                POOL_TAG
                );
            if(NULL == g_pFilterInitStruct)
            {
                status = STATUS_NO_MEMORY;
                TraceEvents(
                    TRACE_LEVEL_ERROR,
                    TRACE_QUEUE,
                    "Allocate Filter Data failed"
                    );
                KeReleaseInStackQueuedSpinLock(&stFilterDataLock);
                break;
            }
            RtlCopyMemory(
                g_pFilterInitStruct,
                pFilterInitStructure,
                nRequestedSize
                );
            pCurrentPosition  = (PBYTE)g_pFilterInitStruct;
            pCurrentPosition += sizeof(FILTER_INIT_STRUCT);
            if(0 == g_pFilterInitStruct->_nSinglePublicIpCount)
            {
                g_pPublicSingleIpArray = NULL;
            }
            else
            {
                g_pPublicSingleIpArray = (PUINT32)pCurrentPosition;
                pCurrentPosition += g_pFilterInitStruct->_nSinglePublicIpCount*sizeof(UINT32);
            }
            if(0 == g_pFilterInitStruct->_nSinglePrivateIpCount)
            {
                g_pPrivateSingleIpArray = NULL;
            }
            else
            {
                g_pPrivateSingleIpArray = (PUINT32)pCurrentPosition;
                pCurrentPosition += g_pFilterInitStruct->_nSinglePrivateIpCount*sizeof(UINT32);
            }
            if(0 == g_pFilterInitStruct->_nPublicRangeIpCount)
            {
                g_pPublicRangeIpArray = NULL;
            }
            else
            {
                g_pPublicRangeIpArray = (PIP4ADDR_RANGE_STRUCT)pCurrentPosition;
                pCurrentPosition += g_pFilterInitStruct->_nPublicRangeIpCount*sizeof(IP4ADDR_RANGE_STRUCT);
            }
            if(0 == g_pFilterInitStruct->_nPrivateRangeIpCount)
            {
                g_pPrivateRangeIpArray = NULL;
            }
            else
            {
                g_pPrivateRangeIpArray = (PIP4ADDR_RANGE_STRUCT)pCurrentPosition;
                pCurrentPosition += g_pFilterInitStruct->_nPrivateRangeIpCount*sizeof(IP4ADDR_RANGE_STRUCT);
            }
            if(0 == g_pFilterInitStruct->_nPublicSubnetIpCount)
            {
                g_pPublicSubnetIpArray = NULL;
            }
            else
            {
                g_pPublicSubnetIpArray = (PIP4SUBNET_STRUCT)pCurrentPosition;
                pCurrentPosition += g_pFilterInitStruct->_nPublicSubnetIpCount*sizeof(IP4SUBNET_STRUCT);
            }
            if(0 == g_pFilterInitStruct->_nPrivateSubnetIpCount)
            {
                g_pPrivateSubnetIpArray = NULL;
            }
            else
            {
                g_pPrivateSubnetIpArray = (PIP4SUBNET_STRUCT)pCurrentPosition;
                pCurrentPosition += g_pFilterInitStruct->_nPrivateSubnetIpCount*sizeof(IP4SUBNET_STRUCT);
            }
            if(0 == g_pFilterInitStruct->_nSinglePortCount)
            {
                g_pDisabledSinglePortArray = NULL;
            }
            else
            {
                g_pDisabledSinglePortArray = (PUINT16)pCurrentPosition;
                pCurrentPosition += g_pFilterInitStruct->_nSinglePortCount*sizeof(UINT16);
            }
            if(0 == g_pFilterInitStruct->_nRangePortCount)
            {
                g_pPortRangeArray = NULL;
            }
            else
            {
                g_pPortRangeArray = (PIP_PORT_RANGE_STRUCT)pCurrentPosition;
                pCurrentPosition += g_pFilterInitStruct->_nRangePortCount*sizeof(IP_PORT_RANGE_STRUCT);
            }
            KeReleaseInStackQueuedSpinLock(&stFilterDataLock);
            ulInformation = InputBufferLength;
        }
		break;
    case NATTRAVERSE_IOCTL_RELEASE_RESORCE:
        KeAcquireInStackQueuedSpinLock(
            &g_nFilterDataLock,
            &stFilterDataLock
            );
        FilterDataCleanup();
        KeReleaseInStackQueuedSpinLock(&stFilterDataLock);
        break;
    default:
        status = STATUS_INVALID_DEVICE_REQUEST;
        TraceEvents(
            TRACE_LEVEL_ERROR,
            TRACE_QUEUE,
            "Invalid IO Control Code = 0x%08X",
            IoControlCode
            );
    }
    WdfRequestCompleteWithInformation(
        Request,
        status,
        ulInformation
        );
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_QUEUE, "%!FUNC! Exit");
    return;
}

