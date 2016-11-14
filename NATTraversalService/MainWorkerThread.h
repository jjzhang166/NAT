
#pragma once
#include "afxmt.h"



// CMainWorkerThread

class CMainWorkerThread : public CWinThread
{
	DECLARE_DYNCREATE(CMainWorkerThread)

public:
	CMainWorkerThread();
	virtual ~CMainWorkerThread();

public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

protected:
	DECLARE_MESSAGE_MAP()
public:
    int m_nRetCode;
    HANDLE m_hEngineHandle;
    UINT32 m_nIpForwardCalloutId;
    UINT32 m_nInboundIpPacketCalloutId;
    int m_nIPForwardDestinationFilterCount;
    UINT64* m_pIPForwardDestinationFilterArray;
    int m_nIPForwardSourceFilterCount;
    UINT64* m_pIPForwardSourceFilterArray;
    int m_nInboundIPPacketFilterCount;
    UINT64* m_pInboundIPPacketFilterArray;
    int m_nOutboundIPPacketFilterCount;
    UINT64* m_pOutboundIPPacketFilterArray;
    HANDLE m_hFilterDriver;
    CEvent m_oThreadDoneEvent;
    bool m_bOnDelete;
    int m_nInboundTransportFilterCount;
    UINT64* m_pInboundTransportFilterArray;
    UINT32 m_nInboundTransportCalloutId;
};
