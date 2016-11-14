
// MainWorkerThread.cpp : implementation file
//

#include "stdafx.h"
#include "NATTraversalService.h"
#include "MainWorkerThread.h"
#include "commrcfg.h"
#include "MessageTable.h"
#include "IpForwardDistinationFilters.h"
#include "IpForwardSourceFilters.h"
#include "InboundIPPacketFilters.h"
#include "DriverInitialize.h"
#include "InboundTransportFilters.h"


// CMainWorkerThread

IMPLEMENT_DYNCREATE(CMainWorkerThread, CWinThread)

CMainWorkerThread::CMainWorkerThread()
: m_nRetCode(0)
, m_hEngineHandle(NULL)
, m_nIpForwardCalloutId(0)
, m_nInboundIpPacketCalloutId(0)
, m_nIPForwardDestinationFilterCount(0)
, m_pIPForwardDestinationFilterArray(NULL)
, m_nIPForwardSourceFilterCount(0)
, m_pIPForwardSourceFilterArray(NULL)
, m_nInboundIPPacketFilterCount(0)
, m_pInboundIPPacketFilterArray(NULL)
, m_nOutboundIPPacketFilterCount(0)
, m_pOutboundIPPacketFilterArray(NULL)
, m_hFilterDriver(INVALID_HANDLE_VALUE)
, m_oThreadDoneEvent(FALSE,TRUE)
, m_bOnDelete(false)
, m_nInboundTransportFilterCount(0)
, m_pInboundTransportFilterArray(NULL)
, m_nInboundTransportCalloutId(0)
{
    m_bAutoDelete = TRUE;
}

CMainWorkerThread::~CMainWorkerThread()
{
    BOOL    bRet;
    m_bOnDelete = true;
    bRet = theApp.m_oWorkerThreadLock.Lock();
    if(NULL != theApp.m_pWorkerThread)
    {
        bRet = m_oThreadDoneEvent.SetEvent();
        theApp.m_pWorkerThread = NULL;
    }
    bRet = theApp.m_oWorkerThreadLock.Unlock();
}

BOOL CMainWorkerThread::InitInstance()
{
    DWORD           dwResult;
    FWPM_SESSION0   stSession;
    CString         sSectionName;
    CString         sIPAddress;
    IN_ADDR         stIP4Address;
    LONG            nRet;
    LPCTSTR         pTerminator;
    FWPM_CALLOUT0   stIpForwardCallOut, stInboundIpPacketCallOut, stInboundTransportCallOut;
    DWORD           dwNetIoStatus;
    CString         sRouterGUID;
    GUID            stRouterGUID;
    NET_LUID        qwRouterLuid;
    HRESULT         hr;
    NET_IFINDEX     ulRouterIndex;
    BOOL            bRet;
    int nSingleIP = 0, nRangeIP = 0, nSubnetIP = 0;
    int nSinglePrivateIP = 0, nRangePrivateIP = 0, nSubnetPrivateIP = 0;
    int nDisabledSinglePorts, nDisabledRangePorts;
    u_long  nhRouterIPAddress;
    CIpForwardDistinationFilters    oIpForwardDistinationFilters(this);
    CIpForwardSourceFilters         oIpForwardSourceFilters(this);
    CInboundIPPacketFilters         oInboundIPPacketFilters(this);
    CInboundTransportFilters        oInboundTransportFilters(this);
    CDriverInitialize               oDriverInitialize(this);
    _tsetlocale( LC_ALL, _T("Russian") );
    bRet = CWinThread::InitInstance();
    hr = CoInitialize(NULL);
    sIPAddress = GetConfigString(
        _T("service_settings"),
        _T("public_router_ip"),
        _T("")
        );
    nRet = RtlIpv4StringToAddress(
        sIPAddress,
        TRUE,
        &pTerminator,
        &stIP4Address
        );
    if(NO_ERROR != nRet)
    {
        DWORD dwRetCount;
        LPVOID lpMsgBuf;
        CString sMsg, sErrorMessage;
        dwRetCount = FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            nRet,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR) &lpMsgBuf,
            0,
            NULL
            );
            if(dwRetCount == 0)
            {
                DWORD   dwRetCode = ::GetLastError();
                sErrorMessage.Format(
                    _T("Ошибка вызова функции \"FormatMessage\"! Код ошибки = %u."),
                    dwRetCode
                    );
            }
            else
            {
                sErrorMessage = (LPCTSTR)lpMsgBuf;
                ::LocalFree((HLOCAL)lpMsgBuf);
            }
        sMsg.Format(
            _T("Ошибка формата IP-адреса маршрутизатора.\n\tКод ошибки = %d (0x%08X): %s\n"),
            nRet,
            nRet,
            (LPCTSTR)sErrorMessage
            );
        m_nRetCode = nRet;
        LPCTSTR pStrings[] = { (LPCTSTR)sMsg };
        theApp.ReportEventInvoke(
            EVENTLOG_ERROR_TYPE,
            MSG_ERROR_EVENT_TEXT,
            1,
            pStrings
            );
        return FALSE;
    }
    nhRouterIPAddress = RtlUlongByteSwap(stIP4Address.S_un.S_addr);
    sRouterGUID = GetConfigString(
        _T("service_settings"),
        _T("public_router_guid"),
        _T("")
        );
    hr = IIDFromString(
        sRouterGUID,
        &stRouterGUID
        );
    if(S_OK != hr)
    {
        DWORD   dwRetCount;
        LPVOID  lpMsgBuf;
        CString sMsg, sErrorMessage;
        dwRetCount = FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            hr,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR) &lpMsgBuf,
            0,
            NULL
            );
            if(dwRetCount == 0)
            {
                DWORD   dwRetCode = ::GetLastError();
                sErrorMessage.Format(
                    _T("Ошибка вызова функции \"FormatMessage\"! Код ошибки = %u."),
                    dwRetCode
                    );
            }
            else
            {
                sErrorMessage = (LPCTSTR)lpMsgBuf;
                ::LocalFree((HLOCAL)lpMsgBuf);
            }
        sMsg.Format(
            _T("Ошибка вызова \"IIDFromString\"!!\n\tКод ошибки = %d (0x%08X): %s\n"),
            hr,
            hr,
            (LPCTSTR)sErrorMessage
            );
        m_nRetCode = hr;
        LPCTSTR pStrings[] = { (LPCTSTR)sMsg };
        theApp.ReportEventInvoke(
            EVENTLOG_ERROR_TYPE,
            MSG_ERROR_EVENT_TEXT,
            1,
            pStrings
            );
        return FALSE;
    }
    dwNetIoStatus = ConvertInterfaceGuidToLuid(
        &stRouterGUID,
        &qwRouterLuid
        );
    if(STATUS_SUCCESS != dwNetIoStatus)
    {
        DWORD   dwRetCount;
        LPVOID  lpMsgBuf;
        CString sMsg, sErrorMessage;
        dwRetCount = FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            dwNetIoStatus,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR) &lpMsgBuf,
            0,
            NULL
            );
            if(dwRetCount == 0)
            {
                DWORD   dwRetCode = ::GetLastError();
                sErrorMessage.Format(
                    _T("Ошибка вызова функции \"FormatMessage\"! Код ошибки = %u."),
                    dwRetCode
                    );
            }
            else
            {
                sErrorMessage = (LPCTSTR)lpMsgBuf;
                ::LocalFree((HLOCAL)lpMsgBuf);
            }
        sMsg.Format(
            _T("Ошибка вызова \"ConvertInterfaceGuidToLuid\"!!\n\tКод ошибки = %d (0x%08X): %s\n"),
            dwNetIoStatus,
            dwNetIoStatus,
            (LPCTSTR)sErrorMessage
            );
        m_nRetCode = dwNetIoStatus;
        LPCTSTR pStrings[] = { (LPCTSTR)sMsg };
        theApp.ReportEventInvoke(
            EVENTLOG_ERROR_TYPE,
            MSG_ERROR_EVENT_TEXT,
            1,
            pStrings
            );
        return FALSE;
    }
    dwNetIoStatus = ConvertInterfaceLuidToIndex(
        &qwRouterLuid,
        &ulRouterIndex
        );
    if(STATUS_SUCCESS != dwNetIoStatus)
    {
        DWORD   dwRetCount;
        LPVOID  lpMsgBuf;
        CString sMsg, sErrorMessage;
        dwRetCount = FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            dwNetIoStatus,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR) &lpMsgBuf,
            0,
            NULL
            );
        if(dwRetCount == 0)
        {
            DWORD   dwRetCode = ::GetLastError();
            sErrorMessage.Format(
                _T("Ошибка вызова функции \"FormatMessage\"! Код ошибки = %u."),
                dwRetCode
                );
        }
        else
        {
            sErrorMessage = (LPCTSTR)lpMsgBuf;
            ::LocalFree((HLOCAL)lpMsgBuf);
        }
        sMsg.Format(
            _T("Ошибка вызова \"ConvertInterfaceLuidToIndex\"!!\n\tКод ошибки = %d (0x%08X): %s\n"),
            dwNetIoStatus,
            dwNetIoStatus,
            (LPCTSTR)sErrorMessage
            );
        m_nRetCode = dwNetIoStatus;
        LPCTSTR pStrings[] = { (LPCTSTR)sMsg };
        theApp.ReportEventInvoke(
            EVENTLOG_ERROR_TYPE,
            MSG_ERROR_EVENT_TEXT,
            1,
            pStrings
            );
        return FALSE;
    }
	nSingleIP = GetConfigInt(
        _T("service_settings"),
        _T("public_ip_count"),
        0
        );
	nRangeIP = GetConfigInt(
        _T("service_settings"),
        _T("public_range_count"),
        0
        );
	nSubnetIP = GetConfigInt(
        _T("service_settings"),
        _T("public_subnet_count"),
        0
        );
	nSinglePrivateIP = GetConfigInt(
        _T("service_settings"),
        _T("private_ip_count"),
        0
        );
	nRangePrivateIP = GetConfigInt(
        _T("service_settings"),
        _T("private_range_count"),
        0
        );
	nSubnetPrivateIP = GetConfigInt(
        _T("service_settings"),
        _T("private_subnet_count"),
        0
        );
    RtlZeroMemory(
        &stSession,
        sizeof(stSession)
        );
    stSession.displayData.name = L"NATTraversalService WFP objects session";
    stSession.displayData.description = L"NATTraversalService WFP objects session";
    stSession.flags = FWPM_SESSION_FLAG_DYNAMIC;
    dwResult = FwpmEngineOpen0(
        NULL,
        RPC_C_AUTHN_WINNT,
        NULL,
        &stSession,
        &m_hEngineHandle
        );
    if(STATUS_SUCCESS != dwResult)
    {
        DWORD   dwRetCount;
        LPVOID  lpMsgBuf;
        CString sMsg, sErrorMessage;
        dwRetCount = FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            dwResult,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR) &lpMsgBuf,
            0,
            NULL
            );
        if(dwRetCount == 0)
        {
            DWORD   dwRetCode = ::GetLastError();
            sErrorMessage.Format(
                _T("Ошибка вызова функции \"FormatMessage\"! Код ошибки = %u."),
                dwRetCode
                );
        }
        else
        {
            sErrorMessage = (LPCTSTR)lpMsgBuf;
            ::LocalFree((HLOCAL)lpMsgBuf);
        }
        sMsg.Format(
            _T("Ошибка вызова \"FwpmEngineOpen0\"!!\n\tКод ошибки = %d (0x%08X): %s\n"),
            dwResult,
            dwResult,
            (LPCTSTR)sErrorMessage
            );
        m_nRetCode = dwResult;
        LPCTSTR pStrings[] = { (LPCTSTR)sMsg };
        theApp.ReportEventInvoke(
            EVENTLOG_ERROR_TYPE,
            MSG_ERROR_EVENT_TEXT,
            1,
            pStrings
            );
        return FALSE;
    }
    dwResult = FwpmTransactionBegin0(
        m_hEngineHandle,
        0
        );
    if(STATUS_SUCCESS != dwResult)
    {
        DWORD   dwRetCount;
        LPVOID  lpMsgBuf;
        CString sMsg, sErrorMessage;
        dwRetCount = FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            dwResult,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR) &lpMsgBuf,
            0,
            NULL
            );
        if(dwRetCount == 0)
        {
            DWORD   dwRetCode = ::GetLastError();
            sErrorMessage.Format(
                _T("Ошибка вызова функции \"FormatMessage\"! Код ошибки = %u."),
                dwRetCode
                );
        }
        else
        {
            sErrorMessage = (LPCTSTR)lpMsgBuf;
            ::LocalFree((HLOCAL)lpMsgBuf);
        }
        sMsg.Format(
            _T("Ошибка запуска транзакции создания набора WFP-фильтров!!\n\tКод ошибки = %d (0x%08X): %s\n"),
            dwResult,
            dwResult,
            (LPCTSTR)sErrorMessage
            );
        m_nRetCode = 28;
        LPCTSTR pStrings[] = { (LPCTSTR)sMsg };
        theApp.ReportEventInvoke(
            EVENTLOG_ERROR_TYPE,
            MSG_ERROR_EVENT_TEXT,
            1,
            pStrings
            );
        return FALSE;
    }
    RtlZeroMemory(
        &stIpForwardCallOut,
        sizeof(stIpForwardCallOut)
        );
    stIpForwardCallOut.calloutKey = NATTRAVERSE_IPFORWARD_CALLOUT_V4;
    stIpForwardCallOut.displayData.name        = L"NAT Traversal filter IP Forward CALLOUT";
    stIpForwardCallOut.displayData.description = L"NAT Traversal filter IP Forward CALLOUT";
    stIpForwardCallOut.applicableLayer = FWPM_LAYER_IPFORWARD_V4;
    dwResult = FwpmCalloutAdd0(
        m_hEngineHandle,
        &stIpForwardCallOut,
        NULL,
        &m_nIpForwardCalloutId
        );
    if(STATUS_SUCCESS != dwResult)
    {
        DWORD   dwRetCount;
        LPVOID  lpMsgBuf;
        CString sMsg, sErrorMessage;
        dwRetCount = FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            dwResult,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR) &lpMsgBuf,
            0,
            NULL
            );
        if(dwRetCount == 0)
        {
            DWORD   dwRetCode = ::GetLastError();
            sErrorMessage.Format(
                _T("Ошибка вызова функции \"FormatMessage\"! Код ошибки = %u."),
                dwRetCode
                );
        }
        else
        {
            sErrorMessage = (LPCTSTR)lpMsgBuf;
            ::LocalFree((HLOCAL)lpMsgBuf);
        }
        sMsg.Format(
            _T("Ошибка добавления обработчика IP-форвандинга!!\n\tКод ошибки = %d (0x%08X): %s\n"),
            dwResult,
            dwResult,
            (LPCTSTR)sErrorMessage
            );
        m_nRetCode = dwResult;
        LPCTSTR pStrings[] = { (LPCTSTR)sMsg };
        theApp.ReportEventInvoke(
            EVENTLOG_ERROR_TYPE,
            MSG_ERROR_EVENT_TEXT,
            1,
            pStrings
            );
        m_nIpForwardCalloutId = 0;
        goto abortWfpTransaction;
    }
    RtlZeroMemory(
        &stInboundIpPacketCallOut,
        sizeof(stInboundIpPacketCallOut)
        );
    stInboundIpPacketCallOut.calloutKey = NATTRAVERSE_INBOUND_IPPACKET_CALLOUT_V4;
    stInboundIpPacketCallOut.displayData.name        = L"NAT Traversal filter Inbound IP Packet CALLOUT";
    stInboundIpPacketCallOut.displayData.description = L"NAT Traversal filter Inbound IP Packet CALLOUT";
    stInboundIpPacketCallOut.applicableLayer = FWPM_LAYER_INBOUND_IPPACKET_V4;
    dwResult = FwpmCalloutAdd0(
        m_hEngineHandle,
        &stInboundIpPacketCallOut,
        NULL,
        &m_nInboundIpPacketCalloutId
        );
    if(STATUS_SUCCESS != dwResult)
    {
        DWORD   dwRetCount;
        LPVOID  lpMsgBuf;
        CString sMsg, sErrorMessage;
        dwRetCount = FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            dwResult,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR) &lpMsgBuf,
            0,
            NULL
            );
        if(dwRetCount == 0)
        {
            DWORD   dwRetCode = ::GetLastError();
            sErrorMessage.Format(
                _T("Ошибка вызова функции \"FormatMessage\"! Код ошибки = %u."),
                dwRetCode
                );
        }
        else
        {
            sErrorMessage = (LPCTSTR)lpMsgBuf;
            ::LocalFree((HLOCAL)lpMsgBuf);
        }
        sMsg.Format(
            _T("Ошибка добавления обработчика входящих IP-пакетов!!\n\tКод ошибки = %d (0x%08X): %s\n"),
            dwResult,
            dwResult,
            (LPCTSTR)sErrorMessage
            );
        m_nRetCode = dwResult;
        LPCTSTR pStrings[] = { (LPCTSTR)sMsg };
        theApp.ReportEventInvoke(
            EVENTLOG_ERROR_TYPE,
            MSG_ERROR_EVENT_TEXT,
            1,
            pStrings
            );
        m_nInboundIpPacketCalloutId = 0;
        goto abortWfpTransaction;
    }
    RtlZeroMemory(
        &stInboundTransportCallOut,
        sizeof(stInboundTransportCallOut)
        );
    stInboundTransportCallOut.calloutKey = NATTRAVERSE_INBOUND_TRANSPORT_CALLOUT_V4;
    stInboundTransportCallOut.displayData.name        = L"NAT Traversal filter Inbound Transport CALLOUT";
    stInboundTransportCallOut.displayData.description = L"NAT Traversal filter Inbound Transport CALLOUT";
    stInboundTransportCallOut.applicableLayer = FWPM_LAYER_INBOUND_TRANSPORT_V4;
    dwResult = FwpmCalloutAdd0(
        m_hEngineHandle,
        &stInboundTransportCallOut,
        NULL,
        &m_nInboundTransportCalloutId
        );
    if(STATUS_SUCCESS != dwResult)
    {
        DWORD   dwRetCount;
        LPVOID  lpMsgBuf;
        CString sMsg, sErrorMessage;
        dwRetCount = FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            dwResult,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR) &lpMsgBuf,
            0,
            NULL
            );
        if(dwRetCount == 0)
        {
            DWORD   dwRetCode = ::GetLastError();
            sErrorMessage.Format(
                _T("Ошибка вызова функции \"FormatMessage\"! Код ошибки = %u."),
                dwRetCode
                );
        }
        else
        {
            sErrorMessage = (LPCTSTR)lpMsgBuf;
            ::LocalFree((HLOCAL)lpMsgBuf);
        }
        sMsg.Format(
            _T("Ошибка добавления обработчика входящего IP-транспорта!!\n\tКод ошибки = %d (0x%08X): %s\n"),
            dwResult,
            dwResult,
            (LPCTSTR)sErrorMessage
            );
        m_nRetCode = dwResult;
        LPCTSTR pStrings[] = { (LPCTSTR)sMsg };
        theApp.ReportEventInvoke(
            EVENTLOG_ERROR_TYPE,
            MSG_ERROR_EVENT_TEXT,
            1,
            pStrings
            );
        m_nInboundTransportCalloutId = 0;
        goto abortWfpTransaction;
    }
    // setup for FWPM_LAYER_IPFORWARD_V4 FWPM_CONDITION_IP_DESTINATION_ADDRESS
    try
    {
        oIpForwardDistinationFilters.CreateFilters(
            nSingleIP,
            nRangeIP,
            nSubnetIP,
            ulRouterIndex // nhRouterIPAddress
            );
    }
    catch(int nE)
    {
        UNREFERENCED_PARAMETER(nE);
        goto abortWfpTransaction;
    }
    // setup for FWPM_LAYER_IPFORWARD_V4 FWPM_CONDITION_IP_SOURCE_ADDRESS
    try
    {
        oIpForwardSourceFilters.CreateFilters(
            nSingleIP,
            nRangeIP,
            nSubnetIP,
            ulRouterIndex
            );
    }
    catch(int nE)
    {
        UNREFERENCED_PARAMETER(nE);
        goto abortWfpTransaction;
    }
	nDisabledSinglePorts = GetConfigInt(
        _T("service_settings"),
        _T("ip_port_count"),
        0
        );
	nDisabledRangePorts = GetConfigInt(
        _T("service_settings"),
        _T("ip_port_range_count"),
        0
        );
    //// setup for FWPM_LAYER_INBOUND_IPPACKET_V4
    //try
    //{
    //    oInboundIPPacketFilters.CreateFilters(
    //        nhRouterIPAddress,
    //        ulRouterIndex
    //        );
    //}
    //catch(int nE)
    //{
    //    UNREFERENCED_PARAMETER(nE);
    //    goto abortWfpTransaction;
    //}
    // setup for FWPM_LAYER_INBOUND_TRANSPORT_V4
    try
    {
        oInboundTransportFilters.CreateFilters(
            nhRouterIPAddress
            );
    }
    catch(int nE)
    {
        UNREFERENCED_PARAMETER(nE);
        goto abortWfpTransaction;
    }
    m_hFilterDriver = CreateFileW(
        NATTRAVERSE_DOS_NAME,
        0, // GENERIC_READ | GENERIC_WRITE
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
        );
    if(INVALID_HANDLE_VALUE == m_hFilterDriver)
    {
        DWORD   dwRetCount;
        LPVOID  lpMsgBuf;
        dwResult = GetLastError();
        CString sMsg, sErrorMessage;
        dwRetCount = FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            dwResult,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR) &lpMsgBuf,
            0,
            NULL
            );
        if(dwRetCount == 0)
        {
            DWORD   dwRetCode = ::GetLastError();
            sErrorMessage.Format(
                _T("Ошибка вызова функции \"FormatMessage\"! Код ошибки = %u."),
                dwRetCode
                );
        }
        else
        {
            sErrorMessage = (LPCTSTR)lpMsgBuf;
            ::LocalFree((HLOCAL)lpMsgBuf);
        }
        sMsg.Format(
            _T("Ошибка выполнения функции \"CreateFileW\" для драйвера CALLOUT-фильтра!!\n\tКод ошибки = %d (0x%08X): %s\n"),
            dwResult,
            dwResult,
            (LPCTSTR)sErrorMessage
            );
        m_nRetCode = dwResult;
        LPCTSTR pStrings[] = { (LPCTSTR)sMsg };
        theApp.ReportEventInvoke(
            EVENTLOG_ERROR_TYPE,
            MSG_ERROR_EVENT_TEXT,
            1,
            pStrings
            );
        goto abortWfpTransaction;
    }
    try
    {
        oDriverInitialize.InitFilterDriver(
            nhRouterIPAddress,
            nSingleIP,
            nRangeIP,
            nSubnetIP,
            nSinglePrivateIP,
            nRangePrivateIP,
            nSubnetPrivateIP,
            nDisabledSinglePorts,
            nDisabledRangePorts
            );
    }
    catch(int nE)
    {
        UNREFERENCED_PARAMETER(nE);
        goto abortWfpTransaction;
    }
    dwResult = FwpmTransactionCommit0(m_hEngineHandle);
    ASSERT(SUCCEEDED(dwResult));
    bRet = theApp.m_oInitWorkerThreadDone.SetEvent();
    return TRUE;
abortWfpTransaction:
    if(NULL != m_pInboundTransportFilterArray)
    {
        delete[] m_pInboundTransportFilterArray;
        m_pInboundTransportFilterArray = NULL;
        m_nInboundTransportFilterCount = 0;
    }
    if(NULL != m_pOutboundIPPacketFilterArray)
    {
        delete[] m_pOutboundIPPacketFilterArray;
        m_pOutboundIPPacketFilterArray = NULL;
        m_nOutboundIPPacketFilterCount = 0;
    }
    if(NULL != m_pInboundIPPacketFilterArray)
    {
        delete[] m_pInboundIPPacketFilterArray;
        m_pInboundIPPacketFilterArray = NULL;
        m_nInboundIPPacketFilterCount = 0;
    }
    if(NULL != m_pIPForwardSourceFilterArray)
    {
        delete[] m_pIPForwardSourceFilterArray;
        m_pIPForwardSourceFilterArray = NULL;
        m_nIPForwardSourceFilterCount = 0;
    }
    if(NULL != m_pIPForwardDestinationFilterArray)
    {
        delete[] m_pIPForwardDestinationFilterArray;
        m_pIPForwardDestinationFilterArray = NULL;
        m_nIPForwardDestinationFilterCount = 0;
    }
    m_nInboundIpPacketCalloutId = 0;
    m_nIpForwardCalloutId = 0;
    dwResult = FwpmTransactionAbort0(m_hEngineHandle);
	return FALSE;
}

int CMainWorkerThread::ExitInstance()
{
    DWORD   dwResult;
    int     i;
    BOOL    bRet;
    if(NULL != m_pOutboundIPPacketFilterArray)
    {
        for(i=0;i<m_nOutboundIPPacketFilterCount;i++)
        {
            if(m_pOutboundIPPacketFilterArray[i] != 0)
            {
                dwResult = FwpmFilterDeleteById0(
                    m_hEngineHandle,
                    m_pOutboundIPPacketFilterArray[i]
                    );
                m_pOutboundIPPacketFilterArray[i] = 0;
            }
        }
        delete [] m_pOutboundIPPacketFilterArray;
        m_pOutboundIPPacketFilterArray = NULL;
    }
    if(NULL != m_pInboundTransportFilterArray)
    {
        for(i=0;i<m_nInboundTransportFilterCount;i++)
        {
            if(m_pInboundTransportFilterArray[i] != 0)
            {
                dwResult = FwpmFilterDeleteById0(
                    m_hEngineHandle,
                    m_pInboundTransportFilterArray[i]
                    );
                m_pInboundTransportFilterArray[i] = 0;
            }
        }
        delete [] m_pInboundTransportFilterArray;
        m_pInboundTransportFilterArray = NULL;
    }
    if(NULL != m_pInboundIPPacketFilterArray)
    {
        for(i=0;i<m_nInboundIPPacketFilterCount;i++)
        {
            if(m_pInboundIPPacketFilterArray[i] != 0)
            {
                dwResult = FwpmFilterDeleteById0(
                    m_hEngineHandle,
                    m_pInboundIPPacketFilterArray[i]
                    );
                m_pInboundIPPacketFilterArray[i] = 0;
            }
        }
        delete [] m_pInboundIPPacketFilterArray;
        m_pInboundIPPacketFilterArray = NULL;
    }
    if(NULL != m_pIPForwardSourceFilterArray)
    {
        for(i=0;i<m_nIPForwardSourceFilterCount;i++)
        {
            if(m_pIPForwardSourceFilterArray[i] != 0)
            {
                dwResult = FwpmFilterDeleteById0(
                    m_hEngineHandle,
                    m_pIPForwardSourceFilterArray[i]
                    );
                m_pIPForwardSourceFilterArray[i] = 0;
            }
        }
        delete [] m_pIPForwardSourceFilterArray;
        m_pIPForwardSourceFilterArray = NULL;
    }
    if(NULL != m_pIPForwardDestinationFilterArray)
    {
        for(i=0;i<m_nIPForwardDestinationFilterCount;i++)
        {
            if(m_pIPForwardDestinationFilterArray[i] != 0)
            {
                dwResult = FwpmFilterDeleteById0(
                    m_hEngineHandle,
                    m_pIPForwardDestinationFilterArray[i]
                    );
                m_pIPForwardDestinationFilterArray[i] = 0;
            }
        }
        delete [] m_pIPForwardDestinationFilterArray;
        m_pIPForwardDestinationFilterArray = NULL;
    }
    if(0 != m_nInboundTransportCalloutId)
    {
        dwResult = FwpmCalloutDeleteById0(
            m_hEngineHandle,
            m_nInboundTransportCalloutId
            );
        m_nInboundTransportCalloutId = 0;
    }
    if(0 != m_nInboundIpPacketCalloutId)
    {
        dwResult = FwpmCalloutDeleteById0(
            m_hEngineHandle,
            m_nInboundIpPacketCalloutId
            );
        m_nInboundIpPacketCalloutId = 0;
    }
    if(0 != m_nIpForwardCalloutId)
    {
        dwResult = FwpmCalloutDeleteById0(
            m_hEngineHandle,
            m_nIpForwardCalloutId
            );
        m_nIpForwardCalloutId = 0;
    }
    if(NULL != m_hEngineHandle)
    {
        dwResult = FwpmEngineClose0(m_hEngineHandle);
        m_hEngineHandle = NULL;
    }
    if(INVALID_HANDLE_VALUE != m_hFilterDriver)
    {
        bRet = DeviceIoControl(
            m_hFilterDriver,
            NATTRAVERSE_IOCTL_RELEASE_RESORCE,
            NULL,
            0,
            NULL,
            0,
            &dwResult,
            NULL
            );
        bRet = CloseHandle(m_hFilterDriver);
        m_hFilterDriver = INVALID_HANDLE_VALUE;
    }
	//CWinThread::ExitInstance();
    {
        SC_HANDLE               schSCManager;
        SC_HANDLE               schService;
        DWORD                   cbBytesNeeded;
        SERVICE_STATUS          stServiceStatus;
        SERVICE_STATUS_PROCESS  stServiceProcessStatus;
        schSCManager = OpenSCManager(
            NULL,                 // local machine
            NULL,                 // ServicesActive database
            SC_MANAGER_ALL_ACCESS // full access rights
            );
        if(NULL == schSCManager)
        {
            return m_nRetCode;
        }
        schService = OpenService(
            schSCManager,
            _T("NATTraversalService"),
            SERVICE_ALL_ACCESS
            );
        if(NULL == schService)
        {
            bRet = CloseServiceHandle(schSCManager);
            return m_nRetCode;
        }
        RtlZeroMemory(
            &stServiceProcessStatus,
            sizeof(stServiceProcessStatus)
            );
        RtlZeroMemory(
            &stServiceStatus,
            sizeof(stServiceStatus)
            );
        bRet = QueryServiceStatusEx(
            schService,
            SC_STATUS_PROCESS_INFO,
            (LPBYTE)&stServiceProcessStatus,
            sizeof(stServiceProcessStatus),
            &cbBytesNeeded
            );
        if(SERVICE_RUNNING == stServiceProcessStatus.dwCurrentState)
        {
            bRet = ControlService(
                schService,
                SERVICE_CONTROL_STOP,
                &stServiceStatus
                );
            m_bOnDelete = true;
            bRet = theApp.m_oWorkerThreadLock.Lock();
            theApp.m_pWorkerThread = NULL;
            bRet = theApp.m_oWorkerThreadLock.Unlock();
        }
        bRet = CloseServiceHandle(schService);
        bRet = CloseServiceHandle(schSCManager);
    }
	return m_nRetCode;
}

BEGIN_MESSAGE_MAP(CMainWorkerThread, CWinThread)
END_MESSAGE_MAP()


// CMainWorkerThread message handlers
