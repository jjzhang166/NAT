
#pragma once

#include "../include/nattraverse_public.h"

extern BOOLEAN                  g_bCleanupState;
extern PFILTER_INIT_STRUCT      g_pFilterInitStruct;
extern PUINT32                  g_pPublicSingleIpArray;
extern PUINT32                  g_pPrivateSingleIpArray;
extern PIP4ADDR_RANGE_STRUCT    g_pPublicRangeIpArray;
extern PIP4ADDR_RANGE_STRUCT    g_pPrivateRangeIpArray;
extern PIP4SUBNET_STRUCT        g_pPublicSubnetIpArray;
extern PIP4SUBNET_STRUCT        g_pPrivateSubnetIpArray;
extern PUINT16                  g_pDisabledSinglePortArray;
extern PIP_PORT_RANGE_STRUCT    g_pPortRangeArray;
extern UINT32                   g_nIndoundIpPacketId;
extern UINT32                   g_nIpForwardId;
extern KSPIN_LOCK               g_nFilterDataLock;

#pragma pack(push, 1)
#pragma warning(push)
#pragma warning(disable : 4214)
typedef struct _IP_HEADER
{
    // -- 0
    BYTE        _IHL                : 4;
    BYTE        _Version            : 4;
    BYTE        _Type_of_Service;
    WORD        _Total_Length;
    // -- 1
    WORD        _Identification;
    WORD        _Fragment_Offset    : 13;
    WORD        _Flags              : 3;
    // -- 2
    BYTE        _Time_to_Live;
    BYTE        _Protocol;
    WORD        _Header_Checksum;
    // -- 3
    DWORD       _Source_Address;
    // -- 4
    DWORD       _Destination_Address;
    // -- 5     -- minimal
    //DWORD       _Options[variable length];
} IP_HEADER, *PIP_HEADER;

typedef struct _IP_HEADER_PORT_PART
{
    // -- 0
    BYTE        _IHL                : 4;
    BYTE        _Version            : 4;
    BYTE        _Type_of_Service;
    WORD        _Total_Length;
    // -- 1
    WORD        _Identification;
    WORD        _Fragment_Offset    : 13;
    WORD        _Flags              : 3;
    // -- 2
    BYTE        _Time_to_Live;
    BYTE        _Protocol;
    WORD        _Header_Checksum;
    // -- 3
    DWORD       _Source_Address;
    // -- 4
    DWORD       _Destination_Address;
    // -- 5     -- minimal
    //DWORD       _Options[variable length];
    // -- UDP or TCP ports header part
    UINT16 srcPort;
    UINT16 destPort;
} IP_HEADER_PORT_PART, *PIP_HEADER_PORT_PART;

typedef struct _UDP_HEADER
{
    UINT16 srcPort;
    UINT16 destPort;
    UINT16 length;
    UINT16 checksum;
} UDP_HEADER, *PUDP_HEADER;

typedef struct _UDP_PSEVDO_PREFIX
{
    DWORD   _source_address;
    DWORD   _destination_address;
    BYTE    _zero;
    BYTE    _protocol;
    WORD    _UDP_length;
} UDP_PSEVDO_PREFIX, *PUDP_PSEVDO_PREFIX;
#pragma warning(pop)
#pragma pack(pop)

NTSTATUS
nattravfilterCoInitialize(PDEVICE_OBJECT deviceObject);

NTSTATUS 
nattravfilterCoRegisterCallouts(PDEVICE_OBJECT deviceObject);

NTSTATUS
nattravfilterCoRegisterCallout(
   IN OUT PDEVICE_OBJECT deviceObject,
   IN  FWPS_CALLOUT_CLASSIFY_FN0 ClassifyFunction,
   IN  FWPS_CALLOUT_NOTIFY_FN0   NotifyFunction,
   IN  FWPS_CALLOUT_FLOW_DELETE_NOTIFY_FN0 FlowDeleteFunction,
   IN  GUID const* calloutKey,
   IN  UINT32 flags,
   OUT UINT32* calloutId
   );

void NTAPI onIndoundTransportClassify(
    __in const FWPS_INCOMING_VALUES0* inFixedValues,
    __in const FWPS_INCOMING_METADATA_VALUES0* inMetaValues,
    __inout_opt void* layerData,
    __in const FWPS_FILTER0* filter,
    __in UINT64 flowContext,
    __out FWPS_CLASSIFY_OUT0* classifyOut
    );

void NTAPI onIndoundIpPacketClassify(
    __in const FWPS_INCOMING_VALUES0* inFixedValues,
    __in const FWPS_INCOMING_METADATA_VALUES0* inMetaValues,
    __inout_opt void* layerData,
    __in const FWPS_FILTER0* filter,
    __in UINT64 flowContext,
    __out FWPS_CLASSIFY_OUT0* classifyOut
    );

void NTAPI onIpForwardClassify(
    __in const FWPS_INCOMING_VALUES0* inFixedValues,
    __in const FWPS_INCOMING_METADATA_VALUES0* inMetaValues,
    __inout_opt void* layerData,
    __in const FWPS_FILTER0* filter,
    __in UINT64 flowContext,
    __out FWPS_CLASSIFY_OUT0* classifyOut
    );

NTSTATUS NTAPI onFiltersChangeNotify(
    __in FWPS_CALLOUT_NOTIFY_TYPE notifyType,
    __in const GUID* filterKey,
    __inout FWPS_FILTER0* filter
    );

void FilterDataCleanup();
