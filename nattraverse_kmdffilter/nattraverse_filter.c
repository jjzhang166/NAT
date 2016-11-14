
#include "precomp.h"
#include "driver.h"
#include "nattraverse_filter.tmh"

PFILTER_INIT_STRUCT      g_pFilterInitStruct        = NULL;
PUINT32                  g_pPublicSingleIpArray     = NULL;
PUINT32                  g_pPrivateSingleIpArray    = NULL;
PIP4ADDR_RANGE_STRUCT    g_pPublicRangeIpArray      = NULL;
PIP4ADDR_RANGE_STRUCT    g_pPrivateRangeIpArray     = NULL;
PIP4SUBNET_STRUCT        g_pPublicSubnetIpArray     = NULL;
PIP4SUBNET_STRUCT        g_pPrivateSubnetIpArray    = NULL;
PUINT16                  g_pDisabledSinglePortArray = NULL;
PIP_PORT_RANGE_STRUCT    g_pPortRangeArray          = NULL;
KSPIN_LOCK               g_nFilterDataLock          = 0;
UINT32                   g_nIndoundIpPacketId       = 0;
UINT32                   g_nIpForwardId             = 0;

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, nattravfilterCoInitialize)
#pragma alloc_text (PAGE, nattravfilterCoRegisterCallouts)
#pragma alloc_text (PAGE, nattravfilterCoRegisterCallout)
#endif

NTSTATUS
nattravfilterCoInitialize(PDEVICE_OBJECT deviceObject)
{
	NTSTATUS    status;
    PAGED_CODE();
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_CALLOUT, "%!FUNC! Entry");
	status = nattravfilterCoRegisterCallouts(deviceObject);
    if (!NT_SUCCESS(status))
    {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_CALLOUT, "nattravfilterCoRegisterCallouts failed %!STATUS!", status);
        return status;
    }
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_CALLOUT, "%!FUNC! Exit");
    return status;
}

NTSTATUS 
nattravfilterCoRegisterCallouts(PDEVICE_OBJECT deviceObject)
{
	NTSTATUS    status;
    PAGED_CODE();
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_CALLOUT, "%!FUNC! Entry");
    status = nattravfilterCoRegisterCallout(
        deviceObject,
        onIpForwardClassify,
        onFiltersChangeNotify,
        (FWPS_CALLOUT_FLOW_DELETE_NOTIFY_FN0)NULL, // We don't need a flow delete function at this layer.
        &NATTRAVERSE_IPFORWARD_CALLOUT_V4,
        0, // No flags.
        &g_nIpForwardId
        );
    if (!NT_SUCCESS(status))
    {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_CALLOUT, "Register IP Forward Callout failed %!STATUS!", status);
        return status;
    }
    status = nattravfilterCoRegisterCallout(
        deviceObject,
        onIndoundIpPacketClassify,
        onFiltersChangeNotify,
        (FWPS_CALLOUT_FLOW_DELETE_NOTIFY_FN0)NULL, // We don't need a flow delete function at this layer.
        &NATTRAVERSE_INBOUND_IPPACKET_CALLOUT_V4,
        0, // No flags.
        &g_nIndoundIpPacketId
        );
    if (!NT_SUCCESS(status))
    {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_CALLOUT, "Register Inbound IP Packets Callout failed %!STATUS!", status);
        return status;
    }
    status = nattravfilterCoRegisterCallout(
        deviceObject,
        onIndoundTransportClassify,
        onFiltersChangeNotify,
        (FWPS_CALLOUT_FLOW_DELETE_NOTIFY_FN0)NULL, // We don't need a flow delete function at this layer.
        &NATTRAVERSE_INBOUND_TRANSPORT_CALLOUT_V4,
        0, // No flags.
        &g_nIndoundIpPacketId
        );
    if (!NT_SUCCESS(status))
    {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_CALLOUT, "Register Inbound Transport Callout failed %!STATUS!", status);
        return status;
    }
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_CALLOUT, "%!FUNC! Exit");
    return status;
}

NTSTATUS
nattravfilterCoRegisterCallout(
   IN OUT PDEVICE_OBJECT deviceObject,
   IN  FWPS_CALLOUT_CLASSIFY_FN0 ClassifyFunction,
   IN  FWPS_CALLOUT_NOTIFY_FN0   NotifyFunction,
   IN  FWPS_CALLOUT_FLOW_DELETE_NOTIFY_FN0 FlowDeleteFunction,
   IN  GUID const* calloutKey,
   IN  UINT32 flags,
   OUT UINT32* calloutId
   )
{
	NTSTATUS        status = STATUS_SUCCESS;
    FWPS_CALLOUT0   stCallout;
    PAGED_CODE();
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_CALLOUT, "%!FUNC! Entry");
    RtlZeroMemory(
        &stCallout,
        sizeof(FWPS_CALLOUT0)
        );
    RtlCopyMemory(
        &stCallout.calloutKey,
        calloutKey,
        sizeof(GUID)
        );
    stCallout.flags = flags;
    stCallout.classifyFn = ClassifyFunction;
    stCallout.notifyFn = NotifyFunction;
    stCallout.flowDeleteFn = FlowDeleteFunction;
    status = FwpsCalloutRegister0(
        deviceObject,
        &stCallout,
        calloutId
        );
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_CALLOUT, "%!FUNC! Exit");
    return status;
}

void NTAPI onIpForwardClassify(
    __in const FWPS_INCOMING_VALUES0* inFixedValues,
    __in const FWPS_INCOMING_METADATA_VALUES0* inMetaValues,
    __inout_opt void* layerData,
    __in const FWPS_FILTER0* filter,
    __in UINT64 flowContext,
    __out FWPS_CLASSIFY_OUT0* classifyOut
    )
{
    int     i;
    PNET_BUFFER_LIST        pNetBufferList = (PNET_BUFFER_LIST)layerData;
    PNET_BUFFER             pNetBuffer;
    PMDL                    _pMDL;
    SIZE_T                  nDataSize;
    PIP_HEADER_PORT_PART    pPortsPart;
    UINT8                   nProtocol;
    UINT16                  wPortNumber;
    BOOLEAN                 bPortDisabled = FALSE;
    UINT32                  nIpHeaderOffset;
    PBYTE                   pByteBuffer;
    KLOCK_QUEUE_HANDLE      stFilterDataLock;
    UINT32                  ulRemoteAddress;
    UNREFERENCED_PARAMETER(inMetaValues);
    UNREFERENCED_PARAMETER(filter);
    UNREFERENCED_PARAMETER(flowContext);
    ASSERT(NULL != pNetBufferList);
    if(!(classifyOut->rights & FWPS_RIGHT_ACTION_WRITE))
    {
        return;
    }
    KeAcquireInStackQueuedSpinLock(
        &g_nFilterDataLock,
        &stFilterDataLock
        );
    if(NULL == g_pFilterInitStruct)
    {
        KeReleaseInStackQueuedSpinLock(&stFilterDataLock);
        classifyOut->actionType = FWP_ACTION_PERMIT;
        return;
    }
    ulRemoteAddress = inFixedValues->incomingValue[FWPS_FIELD_IPFORWARD_V4_IP_SOURCE_ADDRESS].value.uint32;
    //{
    //    PBYTE   pByteAddr = (PBYTE)&ulRemoteAddress;
    //    UINT    naAddrArray[4];
    //    int     j;
    //    for(j=0;j<4;j++)
    //    {
    //        naAddrArray[j] = pByteAddr[j];
    //    }
    //    TraceEvents(
    //        TRACE_LEVEL_INFORMATION,
    //        TRACE_CALLOUT,
    //        "Source Address = %u.%u.%u.%u",
    //        naAddrArray[3],
    //        naAddrArray[2],
    //        naAddrArray[1],
    //        naAddrArray[0]
    //        );
    //}
    if(NULL != g_pPublicSingleIpArray)
    {
        for(i=0;i<g_pFilterInitStruct->_nSinglePublicIpCount;i++)
        {
            if(ulRemoteAddress == g_pPublicSingleIpArray[i])
            {
                KeReleaseInStackQueuedSpinLock(&stFilterDataLock);
                classifyOut->actionType = FWP_ACTION_PERMIT;
                return;
            }
        }
    }
    if(NULL != g_pPrivateSingleIpArray)
    {
        for(i=0;i<g_pFilterInitStruct->_nSinglePrivateIpCount;i++)
        {
            if(ulRemoteAddress == g_pPrivateSingleIpArray[i])
            {
                KeReleaseInStackQueuedSpinLock(&stFilterDataLock);
                classifyOut->actionType = FWP_ACTION_PERMIT;
                return;
            }
        }
    }
    if(NULL != g_pPublicRangeIpArray)
    {
        for(i=0;i<g_pFilterInitStruct->_nPublicRangeIpCount;i++)
        {
            if((ulRemoteAddress >= g_pPublicRangeIpArray[i]._ulLowValue)
                && (ulRemoteAddress <= g_pPublicRangeIpArray[i]._ulHighValue)
                )
            {
                KeReleaseInStackQueuedSpinLock(&stFilterDataLock);
                classifyOut->actionType = FWP_ACTION_PERMIT;
                return;
            }
        }
    }
    if(NULL != g_pPrivateRangeIpArray)
    {
        for(i=0;i<g_pFilterInitStruct->_nPrivateRangeIpCount;i++)
        {
            if((ulRemoteAddress >= g_pPrivateRangeIpArray[i]._ulLowValue)
                && (ulRemoteAddress <= g_pPrivateRangeIpArray[i]._ulHighValue)
                )
            {
                KeReleaseInStackQueuedSpinLock(&stFilterDataLock);
                classifyOut->actionType = FWP_ACTION_PERMIT;
                return;
            }
        }
    }
    if(NULL != g_pPublicSubnetIpArray)
    {
        for(i=0;i<g_pFilterInitStruct->_nPublicSubnetIpCount;i++)
        {
            UINT32  ulI4AddrPart = (ulRemoteAddress & g_pPublicSubnetIpArray[i]._ulIpMask);
            if(ulI4AddrPart == g_pPublicSubnetIpArray[i]._ulIpAddr)
            {
                KeReleaseInStackQueuedSpinLock(&stFilterDataLock);
                classifyOut->actionType = FWP_ACTION_PERMIT;
                return;
            }
        }
    }
    if(NULL != g_pPrivateSubnetIpArray)
    {
        for(i=0;i<g_pFilterInitStruct->_nPrivateSubnetIpCount;i++)
        {
            UINT32  ulI4AddrPart = (ulRemoteAddress & g_pPrivateSubnetIpArray[i]._ulIpMask);
            if(ulI4AddrPart == g_pPrivateSubnetIpArray[i]._ulIpAddr)
            {
                KeReleaseInStackQueuedSpinLock(&stFilterDataLock);
                classifyOut->actionType = FWP_ACTION_PERMIT;
                return;
            }
        }
    }
    pNetBuffer = NET_BUFFER_LIST_FIRST_NB(pNetBufferList);
    nIpHeaderOffset = NET_BUFFER_DATA_OFFSET(pNetBuffer);
    _pMDL = NET_BUFFER_FIRST_MDL(pNetBuffer);
    nDataSize = (SIZE_T)MmGetMdlByteCount(_pMDL) - nIpHeaderOffset;
    ASSERT(nDataSize >= sizeof(IP_HEADER));
    pByteBuffer = (PBYTE)MmGetMdlVirtualAddress(_pMDL);
    pPortsPart = (PIP_HEADER_PORT_PART)&pByteBuffer[nIpHeaderOffset];
    nProtocol = pPortsPart->_Protocol;
    if((IPPROTO_TCP != nProtocol) && (IPPROTO_UDP != nProtocol))
    {
        KeReleaseInStackQueuedSpinLock(&stFilterDataLock);
        classifyOut->actionType = FWP_ACTION_PERMIT;
        return;
    }
    if(nDataSize < sizeof(IP_HEADER_PORT_PART))
    {
        KeReleaseInStackQueuedSpinLock(&stFilterDataLock);
        classifyOut->actionType = FWP_ACTION_PERMIT;
        return;
    }
    wPortNumber = RtlUshortByteSwap(pPortsPart->destPort);
    if(NULL != g_pDisabledSinglePortArray)
    {
        for(i=0;i<g_pFilterInitStruct->_nSinglePortCount;i++)
        {
            if(wPortNumber == g_pDisabledSinglePortArray[i])
            {
                bPortDisabled = TRUE;
                break;
            }
        }
    }
    if((!bPortDisabled) && (NULL != g_pPortRangeArray))
    {
        for(i=0;i<g_pFilterInitStruct->_nRangePortCount;i++)
        {
            if((wPortNumber >= g_pPortRangeArray[i]._usLowValue)
                && (wPortNumber <= g_pPortRangeArray[i]._usHighValue)
                )
            {
                bPortDisabled = TRUE;
                break;
            }
        }
    }
    if(bPortDisabled)
    {
        classifyOut->actionType = FWP_ACTION_BLOCK;
        classifyOut->rights &= ~FWPS_RIGHT_ACTION_WRITE;
    }
    else
    {
        classifyOut->actionType = FWP_ACTION_PERMIT;
    }
    KeReleaseInStackQueuedSpinLock(&stFilterDataLock);
}

NTSTATUS NTAPI onFiltersChangeNotify(
    __in FWPS_CALLOUT_NOTIFY_TYPE notifyType,
    __in const GUID* filterKey,
    __inout FWPS_FILTER0* filter
    )
{
    UNREFERENCED_PARAMETER(notifyType);
    UNREFERENCED_PARAMETER(filterKey);
    UNREFERENCED_PARAMETER(filter);
    return STATUS_SUCCESS;
}

void NTAPI onIndoundIpPacketClassify(
    __in const FWPS_INCOMING_VALUES0* inFixedValues,
    __in const FWPS_INCOMING_METADATA_VALUES0* inMetaValues,
    __inout_opt void* layerData,
    __in const FWPS_FILTER0* filter,
    __in UINT64 flowContext,
    __out FWPS_CLASSIFY_OUT0* classifyOut
    )
{
    int     i;
    PNET_BUFFER_LIST        pNetBufferList = (PNET_BUFFER_LIST)layerData;
    PNET_BUFFER             pNetBuffer;
    PMDL                    _pMDL;
    SIZE_T                  nDataSize;
    PIP_HEADER_PORT_PART    pPortsPart;
    UINT8                   nProtocol;
    UINT16                  wPortNumber;
    BOOLEAN                 bPortDisabled = FALSE;
    UINT32                  nNblOffset;
    UINT32                  nIpHeaderSize;
    UINT32                  nIpHeaderOffset;
    PBYTE                   pByteBuffer;
    KLOCK_QUEUE_HANDLE      stFilterDataLock;
    UINT32                  ulRemoteAddress;
    UINT32                  ulLocalAddress;
    UINT32                  ulFlags;
    UNREFERENCED_PARAMETER(filter);
    UNREFERENCED_PARAMETER(flowContext);
    if(!(classifyOut->rights & FWPS_RIGHT_ACTION_WRITE))
    {
        return;
    }
    KeAcquireInStackQueuedSpinLock(
        &g_nFilterDataLock,
        &stFilterDataLock
        );
    if(NULL == g_pFilterInitStruct)
    {
        KeReleaseInStackQueuedSpinLock(&stFilterDataLock);
        classifyOut->actionType = FWP_ACTION_PERMIT;
        return;
    }
    ulLocalAddress = inFixedValues->incomingValue[FWPS_FIELD_INBOUND_IPPACKET_V4_IP_LOCAL_ADDRESS].value.uint32;
    if(g_pFilterInitStruct->_ulRouterAddress != ulLocalAddress)
    {
        KeReleaseInStackQueuedSpinLock(&stFilterDataLock);
        classifyOut->actionType = FWP_ACTION_PERMIT;
        return;
    }
    ulFlags = inFixedValues->incomingValue[FWPS_FIELD_INBOUND_IPPACKET_V4_FLAGS].value.uint32;
    if(0 != (ulFlags & (FWP_CONDITION_FLAG_IS_FRAGMENT | FWP_CONDITION_FLAG_IS_FRAGMENT)))
    {
        KeReleaseInStackQueuedSpinLock(&stFilterDataLock);
        classifyOut->actionType = FWP_ACTION_PERMIT;
        return;
    }
    ulRemoteAddress = inFixedValues->incomingValue[FWPS_FIELD_INBOUND_IPPACKET_V4_IP_REMOTE_ADDRESS].value.uint32;
    if(NULL != g_pPublicSingleIpArray)
    {
        for(i=0;i<g_pFilterInitStruct->_nSinglePublicIpCount;i++)
        {
            if(ulRemoteAddress == g_pPublicSingleIpArray[i])
            {
                KeReleaseInStackQueuedSpinLock(&stFilterDataLock);
                classifyOut->actionType = FWP_ACTION_PERMIT;
                return;
            }
        }
    }
    if(NULL != g_pPrivateSingleIpArray)
    {
        for(i=0;i<g_pFilterInitStruct->_nSinglePrivateIpCount;i++)
        {
            if(ulRemoteAddress == g_pPrivateSingleIpArray[i])
            {
                KeReleaseInStackQueuedSpinLock(&stFilterDataLock);
                classifyOut->actionType = FWP_ACTION_PERMIT;
                return;
            }
        }
    }
    if(NULL != g_pPublicRangeIpArray)
    {
        for(i=0;i<g_pFilterInitStruct->_nPublicRangeIpCount;i++)
        {
            if((ulRemoteAddress >= g_pPublicRangeIpArray[i]._ulLowValue)
                && (ulRemoteAddress <= g_pPublicRangeIpArray[i]._ulHighValue)
                )
            {
                KeReleaseInStackQueuedSpinLock(&stFilterDataLock);
                classifyOut->actionType = FWP_ACTION_PERMIT;
                return;
            }
        }
    }
    if(NULL != g_pPrivateRangeIpArray)
    {
        for(i=0;i<g_pFilterInitStruct->_nPrivateRangeIpCount;i++)
        {
            if((ulRemoteAddress >= g_pPrivateRangeIpArray[i]._ulLowValue)
                && (ulRemoteAddress <= g_pPrivateRangeIpArray[i]._ulHighValue)
                )
            {
                KeReleaseInStackQueuedSpinLock(&stFilterDataLock);
                classifyOut->actionType = FWP_ACTION_PERMIT;
                return;
            }
        }
    }
    if(NULL != g_pPublicSubnetIpArray)
    {
        for(i=0;i<g_pFilterInitStruct->_nPublicSubnetIpCount;i++)
        {
            UINT32  ulI4AddrPart = (ulRemoteAddress & g_pPublicSubnetIpArray[i]._ulIpMask);
            if(ulI4AddrPart == g_pPublicSubnetIpArray[i]._ulIpAddr)
            {
                KeReleaseInStackQueuedSpinLock(&stFilterDataLock);
                classifyOut->actionType = FWP_ACTION_PERMIT;
                return;
            }
        }
    }
    if(NULL != g_pPrivateSubnetIpArray)
    {
        for(i=0;i<g_pFilterInitStruct->_nPrivateSubnetIpCount;i++)
        {
            UINT32  ulI4AddrPart = (ulRemoteAddress & g_pPrivateSubnetIpArray[i]._ulIpMask);
            if(ulI4AddrPart == g_pPrivateSubnetIpArray[i]._ulIpAddr)
            {
                KeReleaseInStackQueuedSpinLock(&stFilterDataLock);
                classifyOut->actionType = FWP_ACTION_PERMIT;
                return;
            }
        }
    }
    pNetBuffer = NET_BUFFER_LIST_FIRST_NB(pNetBufferList);
    nNblOffset = NET_BUFFER_DATA_OFFSET(pNetBuffer);
    ASSERT(FWPS_IS_METADATA_FIELD_PRESENT(inMetaValues, 
                                          FWPS_METADATA_FIELD_IP_HEADER_SIZE));
    nIpHeaderSize = inMetaValues->ipHeaderSize;
    nIpHeaderOffset = nNblOffset - nIpHeaderSize;
    _pMDL = NET_BUFFER_FIRST_MDL(pNetBuffer);
    nDataSize = (SIZE_T)MmGetMdlByteCount(_pMDL) - nIpHeaderOffset;
    ASSERT(nDataSize >= sizeof(IP_HEADER));
    pByteBuffer = (PBYTE)MmGetMdlVirtualAddress(_pMDL);
    pPortsPart = (PIP_HEADER_PORT_PART)&pByteBuffer[nIpHeaderOffset];
    nProtocol = pPortsPart->_Protocol;
    if((IPPROTO_TCP != nProtocol) && (IPPROTO_UDP != nProtocol))
    {
        KeReleaseInStackQueuedSpinLock(&stFilterDataLock);
        classifyOut->actionType = FWP_ACTION_PERMIT;
        return;
    }
    if(nDataSize < sizeof(IP_HEADER_PORT_PART))
    {
        KeReleaseInStackQueuedSpinLock(&stFilterDataLock);
        classifyOut->actionType = FWP_ACTION_PERMIT;
        return;
    }
    wPortNumber = RtlUshortByteSwap(pPortsPart->destPort);
    if(NULL != g_pDisabledSinglePortArray)
    {
        for(i=0;i<g_pFilterInitStruct->_nSinglePortCount;i++)
        {
            if(wPortNumber == g_pDisabledSinglePortArray[i])
            {
                bPortDisabled = TRUE;
                break;
            }
        }
    }
    if((!bPortDisabled) && (NULL != g_pPortRangeArray))
    {
        for(i=0;i<g_pFilterInitStruct->_nRangePortCount;i++)
        {
            if((wPortNumber >= g_pPortRangeArray[i]._usLowValue)
                && (wPortNumber <= g_pPortRangeArray[i]._usHighValue)
                )
            {
                bPortDisabled = TRUE;
                break;
            }
        }
    }
    if(bPortDisabled)
    {
        classifyOut->actionType = FWP_ACTION_BLOCK;
        classifyOut->rights &= ~FWPS_RIGHT_ACTION_WRITE;
    }
    else
    {
        classifyOut->actionType = FWP_ACTION_PERMIT;
    }
    KeReleaseInStackQueuedSpinLock(&stFilterDataLock);
}

void FilterDataCleanup()
{
    if(NULL != g_pFilterInitStruct)
    {
        ExFreePoolWithTag(
            g_pFilterInitStruct,
            POOL_TAG
            );
    }
    g_pFilterInitStruct        = NULL;
    g_pPublicSingleIpArray     = NULL;
    g_pPrivateSingleIpArray    = NULL;
    g_pPublicRangeIpArray      = NULL;
    g_pPrivateRangeIpArray     = NULL;
    g_pPublicSubnetIpArray     = NULL;
    g_pPrivateSubnetIpArray    = NULL;
    g_pDisabledSinglePortArray = NULL;
    g_pPortRangeArray          = NULL;
}

void NTAPI onIndoundTransportClassify(
    __in const FWPS_INCOMING_VALUES0* inFixedValues,
    __in const FWPS_INCOMING_METADATA_VALUES0* inMetaValues,
    __inout_opt void* layerData,
    __in const FWPS_FILTER0* filter,
    __in UINT64 flowContext,
    __out FWPS_CLASSIFY_OUT0* classifyOut
    )
{
    int     i;
    KLOCK_QUEUE_HANDLE      stFilterDataLock;
    UINT32                  ulRemoteAddress;
    UINT8                   nProtocol;
    UINT16                  wPortNumber;
    BOOLEAN                 bPortDisabled = FALSE;
    UNREFERENCED_PARAMETER(inMetaValues);
    UNREFERENCED_PARAMETER(layerData);
    UNREFERENCED_PARAMETER(filter);
    UNREFERENCED_PARAMETER(flowContext);
    if(!(classifyOut->rights & FWPS_RIGHT_ACTION_WRITE))
    {
        return;
    }
    KeAcquireInStackQueuedSpinLock(
        &g_nFilterDataLock,
        &stFilterDataLock
        );
    if(NULL == g_pFilterInitStruct)
    {
        KeReleaseInStackQueuedSpinLock(&stFilterDataLock);
        classifyOut->actionType = FWP_ACTION_PERMIT;
        return;
    }
    ulRemoteAddress = inFixedValues->incomingValue[FWPS_FIELD_INBOUND_TRANSPORT_V4_IP_REMOTE_ADDRESS].value.uint32;
    if(NULL != g_pPublicSingleIpArray)
    {
        for(i=0;i<g_pFilterInitStruct->_nSinglePublicIpCount;i++)
        {
            if(ulRemoteAddress == g_pPublicSingleIpArray[i])
            {
                KeReleaseInStackQueuedSpinLock(&stFilterDataLock);
                classifyOut->actionType = FWP_ACTION_PERMIT;
                return;
            }
        }
    }
    if(NULL != g_pPrivateSingleIpArray)
    {
        for(i=0;i<g_pFilterInitStruct->_nSinglePrivateIpCount;i++)
        {
            if(ulRemoteAddress == g_pPrivateSingleIpArray[i])
            {
                KeReleaseInStackQueuedSpinLock(&stFilterDataLock);
                classifyOut->actionType = FWP_ACTION_PERMIT;
                return;
            }
        }
    }
    if(NULL != g_pPublicRangeIpArray)
    {
        for(i=0;i<g_pFilterInitStruct->_nPublicRangeIpCount;i++)
        {
            if((ulRemoteAddress >= g_pPublicRangeIpArray[i]._ulLowValue)
                && (ulRemoteAddress <= g_pPublicRangeIpArray[i]._ulHighValue)
                )
            {
                KeReleaseInStackQueuedSpinLock(&stFilterDataLock);
                classifyOut->actionType = FWP_ACTION_PERMIT;
                return;
            }
        }
    }
    if(NULL != g_pPrivateRangeIpArray)
    {
        for(i=0;i<g_pFilterInitStruct->_nPrivateRangeIpCount;i++)
        {
            if((ulRemoteAddress >= g_pPrivateRangeIpArray[i]._ulLowValue)
                && (ulRemoteAddress <= g_pPrivateRangeIpArray[i]._ulHighValue)
                )
            {
                KeReleaseInStackQueuedSpinLock(&stFilterDataLock);
                classifyOut->actionType = FWP_ACTION_PERMIT;
                return;
            }
        }
    }
    if(NULL != g_pPublicSubnetIpArray)
    {
        for(i=0;i<g_pFilterInitStruct->_nPublicSubnetIpCount;i++)
        {
            UINT32  ulI4AddrPart = (ulRemoteAddress & g_pPublicSubnetIpArray[i]._ulIpMask);
            if(ulI4AddrPart == g_pPublicSubnetIpArray[i]._ulIpAddr)
            {
                KeReleaseInStackQueuedSpinLock(&stFilterDataLock);
                classifyOut->actionType = FWP_ACTION_PERMIT;
                return;
            }
        }
    }
    if(NULL != g_pPrivateSubnetIpArray)
    {
        for(i=0;i<g_pFilterInitStruct->_nPrivateSubnetIpCount;i++)
        {
            UINT32  ulI4AddrPart = (ulRemoteAddress & g_pPrivateSubnetIpArray[i]._ulIpMask);
            if(ulI4AddrPart == g_pPrivateSubnetIpArray[i]._ulIpAddr)
            {
                KeReleaseInStackQueuedSpinLock(&stFilterDataLock);
                classifyOut->actionType = FWP_ACTION_PERMIT;
                return;
            }
        }
    }
    nProtocol = inFixedValues->incomingValue[FWPS_FIELD_INBOUND_TRANSPORT_V4_IP_PROTOCOL].value.uint8;
    if((IPPROTO_TCP != nProtocol) && (IPPROTO_UDP != nProtocol))
    {
        KeReleaseInStackQueuedSpinLock(&stFilterDataLock);
        classifyOut->actionType = FWP_ACTION_PERMIT;
        return;
    }
    wPortNumber = inFixedValues->incomingValue[FWPS_FIELD_INBOUND_TRANSPORT_V4_IP_LOCAL_PORT].value.uint16;
    if(NULL != g_pDisabledSinglePortArray)
    {
        for(i=0;i<g_pFilterInitStruct->_nSinglePortCount;i++)
        {
            if(wPortNumber == g_pDisabledSinglePortArray[i])
            {
                bPortDisabled = TRUE;
                break;
            }
        }
    }
    if((!bPortDisabled) && (NULL != g_pPortRangeArray))
    {
        for(i=0;i<g_pFilterInitStruct->_nRangePortCount;i++)
        {
            if((wPortNumber >= g_pPortRangeArray[i]._usLowValue)
                && (wPortNumber <= g_pPortRangeArray[i]._usHighValue)
                )
            {
                bPortDisabled = TRUE;
                break;
            }
        }
    }
    if(bPortDisabled)
    {
        classifyOut->actionType = FWP_ACTION_BLOCK;
        classifyOut->rights &= ~FWPS_RIGHT_ACTION_WRITE;
    }
    else
    {
        classifyOut->actionType = FWP_ACTION_PERMIT;
    }
    KeReleaseInStackQueuedSpinLock(&stFilterDataLock);
}
