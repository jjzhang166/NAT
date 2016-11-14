
#pragma once

#define NATTRAVERSE_DOS_NAME  L"\\\\.\\nattraverse_kmdffilter"

#define	NATTRAVERSE_IOCTL_INIT_FILTERS      CTL_CODE(FILE_DEVICE_NETWORK, 0x1, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define	NATTRAVERSE_IOCTL_RELEASE_RESORCE   CTL_CODE(FILE_DEVICE_NETWORK, 0x2, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct _IP4ADDR_RANGE_STRUCT
{
    UINT32 _ulLowValue;
    UINT32 _ulHighValue;
}
IP4ADDR_RANGE_STRUCT, *PIP4ADDR_RANGE_STRUCT;

typedef struct _IP4SUBNET_STRUCT
{
    UINT32 _ulIpAddr;
    UINT32 _ulIpMask;
}
IP4SUBNET_STRUCT, *PIP4SUBNET_STRUCT;

typedef struct _IP_PORT_RANGE_STRUCT
{
    UINT16 _usLowValue;
    UINT16 _usHighValue;
}
IP_PORT_RANGE_STRUCT, *PIP_PORT_RANGE_STRUCT;

typedef struct _FILTER_INIT_STRUCT
{
    int     _nSinglePublicIpCount;
    int      _nSinglePrivateIpCount;
    int     _nPublicRangeIpCount;
    int     _nPrivateRangeIpCount;
    int     _nPublicSubnetIpCount;
    int     _nPrivateSubnetIpCount;
    int     _nSinglePortCount;
    int     _nRangePortCount;
    UINT32  _ulRouterAddress;
}
FILTER_INIT_STRUCT, *PFILTER_INIT_STRUCT;

// {DF26945F-A33E-496A-B0B8-46255F756DEA}
DEFINE_GUID(NATTRAVERSE_IPFORWARD_CALLOUT_V4, 
0xdf26945f, 0xa33e, 0x496a, 0xb0, 0xb8, 0x46, 0x25, 0x5f, 0x75, 0x6d, 0xea);
// {426520D8-D21E-4186-83B8-F5B602A6D7B5}
DEFINE_GUID(NATTRAVERSE_INBOUND_IPPACKET_CALLOUT_V4,
0x426520d8, 0xd21e, 0x4186, 0x83, 0xb8, 0xf5, 0xb6, 0x2, 0xa6, 0xd7, 0xb5);
// {89099FB6-7D0B-486D-8563-FEF1753996C6}
DEFINE_GUID(NATTRAVERSE_INBOUND_TRANSPORT_CALLOUT_V4,
0x89099fb6, 0x7d0b, 0x486d, 0x85, 0x63, 0xfe, 0xf1, 0x75, 0x39, 0x96, 0xc6);
