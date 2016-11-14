#include "stdafx.h"
#include "NATTraversalService.h"
#include "MainWorkerThread.h"
#include "InboundIPPacketFilters.h"
#include "commrcfg.h"
#include "MessageTable.h"


CInboundIPPacketFilters::CInboundIPPacketFilters(
    CMainWorkerThread* pOwner
    )
: m_pOwner(pOwner)
, m_pConditionsArray(NULL)
, m_pFiltersArray(NULL)
{
}


CInboundIPPacketFilters::~CInboundIPPacketFilters(void)
{
    if(NULL != m_pConditionsArray)
    {
        delete[] m_pConditionsArray;
        m_pConditionsArray = NULL;
    }
    if(NULL != m_pFiltersArray)
    {
        delete[] m_pFiltersArray;
        m_pFiltersArray = NULL;
    }
}


void CInboundIPPacketFilters::CreateFilters(
    UINT32 nhRouterIPAddress,
    UINT32 ulRouterIndex
    )
{
    int     nFilterCount = 1;
    UINT64* pFilterIDsArray = new UINT64[nFilterCount];
    int     ulConditionsCount = 2;
    DWORD   dwResult;
    static UINT64   nFilterWeight = 0xFFFFFFFFFFFFFFFE;
    // setup for FWPM_LAYER_INBOUND_IPPACKET_V4
    m_pOwner->m_nInboundIPPacketFilterCount = nFilterCount;
    m_pOwner->m_pInboundIPPacketFilterArray = pFilterIDsArray;
    RtlZeroMemory(
        pFilterIDsArray,
        sizeof(UINT64)*nFilterCount
        );
    m_pFiltersArray = new FWPM_FILTER0[nFilterCount];
    RtlZeroMemory(
        m_pFiltersArray,
        sizeof(FWPM_FILTER0)*nFilterCount
        );
    m_pConditionsArray = new FWPM_FILTER_CONDITION0[ulConditionsCount];
    RtlZeroMemory(
        m_pConditionsArray,
        sizeof(FWPM_FILTER_CONDITION0)*ulConditionsCount
        );
    m_pConditionsArray[0].fieldKey = FWPM_CONDITION_INTERFACE_INDEX;
    m_pConditionsArray[0].matchType = FWP_MATCH_EQUAL;
    m_pConditionsArray[0].conditionValue.type = FWP_UINT32;
    m_pConditionsArray[0].conditionValue.uint32 = ulRouterIndex;
    m_pConditionsArray[1].fieldKey = FWPM_CONDITION_IP_LOCAL_ADDRESS;
    m_pConditionsArray[1].matchType = FWP_MATCH_EQUAL;
    m_pConditionsArray[1].conditionValue.type = FWP_UINT32;
    m_pConditionsArray[1].conditionValue.uint32 = nhRouterIPAddress;
    m_pFiltersArray[0].displayData.name = L"NAT Travelsal IP-router INBOUND_IPPACKET Filter";
    m_pFiltersArray[0].flags = FWPM_FILTER_FLAG_NONE;
    m_pFiltersArray[0].layerKey = FWPM_LAYER_INBOUND_IPPACKET_V4;
    m_pFiltersArray[0].subLayerKey = FWPM_SUBLAYER_UNIVERSAL;
    m_pFiltersArray[0].weight.type = FWP_UINT64;
    m_pFiltersArray[0].weight.uint64 = &nFilterWeight;
    m_pFiltersArray[0].numFilterConditions = 0; // ulConditionsCount
    m_pFiltersArray[0].filterCondition = NULL; // m_pConditionsArray
    m_pFiltersArray[0].action.type = FWP_ACTION_CALLOUT_TERMINATING;
    m_pFiltersArray[0].action.calloutKey = NATTRAVERSE_INBOUND_IPPACKET_CALLOUT_V4;
    dwResult = FwpmFilterAdd0(
        m_pOwner->m_hEngineHandle,
        &m_pFiltersArray[0],
        NULL,
        &pFilterIDsArray[0]
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
                _T("������ ������ ������� \"FormatMessage\"! ��� ������ = %u."),
                dwRetCode
                );
        }
        else
        {
            sErrorMessage = (LPCTSTR)lpMsgBuf;
            ::LocalFree((HLOCAL)lpMsgBuf);
        }
        sMsg.Format(
            _T("������ ���������� ������� \"FwpmFilterAdd0\"!!\n\t��� ������ = %d (0x%08X): %s\n"),
            dwResult,
            dwResult,
            (LPCTSTR)sErrorMessage
            );
        m_pOwner->m_nRetCode = dwResult;
        LPCTSTR pStrings[] = { (LPCTSTR)sMsg };
        theApp.ReportEventInvoke(
            EVENTLOG_ERROR_TYPE,
            MSG_ERROR_EVENT_TEXT,
            1,
            pStrings
            );
        throw (int)0;
    }
}
