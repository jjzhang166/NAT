#include "stdafx.h"
#include "NATTraversalService.h"
#include "MainWorkerThread.h"
#include "IpForwardDistinationFilters.h"
#include "commrcfg.h"
#include "MessageTable.h"


CIpForwardDistinationFilters::CIpForwardDistinationFilters(
    CMainWorkerThread* pOwner
    )
: m_pOwner(pOwner)
, m_pConditionsArray(NULL)
, m_pFiltersArray(NULL)
, m_pIp4RangeArray(NULL)
, m_pV4AddMaskArray(NULL)
{
}


CIpForwardDistinationFilters::~CIpForwardDistinationFilters(void)
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
    if(NULL != m_pV4AddMaskArray)
    {
        delete[] m_pV4AddMaskArray;
        m_pV4AddMaskArray = NULL;
    }
    if(NULL != m_pIp4RangeArray)
    {
        delete[] m_pIp4RangeArray;
        m_pIp4RangeArray = NULL;
    }
}


void CIpForwardDistinationFilters::CreateFilters(
    int nSingleIP,
    int nRangeIP,
    int nSubnetIP,
    UINT32 ulRouterIndex
    )
{
    int nFilterCount = nSingleIP + nRangeIP + nSubnetIP;
    int j = 0, i, nRangeIndex = 0, nSubnetIndex = 0, ulConditionsCount = 2;
    UINT64* pFilterIDsArray = new UINT64[nFilterCount];
    CString sSectionName, sIPAddress;
    LONG    nRet;
    LPCTSTR pTerminator;
    IN_ADDR stIP4Address;
    u_long  nhIPAddress, nhBeginAddress, nhEndAddress;
    DWORD   dwResult;
    static UINT64   nFilterWeight = 0xFFFFFFFFFFFFFFFE;
    // setup for FWPM_LAYER_IPFORWARD_V4 FWPM_CONDITION_IP_DESTINATION_ADDRESS
    RtlZeroMemory(
        &stIP4Address,
        sizeof(stIP4Address)
        );
    if(nRangeIP > 0)
    {
        m_pIp4RangeArray = new FWP_RANGE0[nRangeIP];
        RtlZeroMemory(
            m_pIp4RangeArray,
            sizeof(FWP_RANGE0)*nRangeIP
            );
    }
    if(nSubnetIP > 0)
    {
        m_pV4AddMaskArray = new FWP_V4_ADDR_AND_MASK[nSubnetIP];
        RtlZeroMemory(
            m_pV4AddMaskArray,
            sizeof(FWP_V4_ADDR_AND_MASK)*nSubnetIP
            );
    }
    m_pOwner->m_nIPForwardDestinationFilterCount = nFilterCount;
    m_pOwner->m_pIPForwardDestinationFilterArray = pFilterIDsArray;
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
    m_pConditionsArray[0].fieldKey = FWPM_CONDITION_SOURCE_INTERFACE_INDEX;
    m_pConditionsArray[0].matchType = FWP_MATCH_EQUAL;
    m_pConditionsArray[0].conditionValue.type = FWP_UINT32;
    m_pConditionsArray[0].conditionValue.uint32 = ulRouterIndex;
    m_pConditionsArray[1].fieldKey = FWPM_CONDITION_IP_DESTINATION_ADDRESS;
    for(i=0;i<nSingleIP;i++)
    {
        sSectionName.Format(
            _T("single_ip%d"),
            i
            );
	    sIPAddress = GetConfigString(
            _T("service_settings"),
            sSectionName,
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
            DWORD   dwRetCount;
            LPVOID  lpMsgBuf;
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
                _T("Ошибка формата публичного IP-адреса.\n\tКод ошибки = %d (0x%08X): %s\n"),
                nRet,
                nRet,
                (LPCTSTR)sErrorMessage
                );
            m_pOwner->m_nRetCode = nRet;
            LPCTSTR pStrings[] = { (LPCTSTR)sMsg };
            theApp.ReportEventInvoke(
                EVENTLOG_ERROR_TYPE,
                MSG_ERROR_EVENT_TEXT,
                1,
                pStrings
                );
            throw j;
        }
        nhIPAddress = RtlUlongByteSwap(stIP4Address.S_un.S_addr);
        m_pFiltersArray[j].displayData.name = L"NAT Travelsal IPFORWARD DESTINATION Filter";
        m_pFiltersArray[j].flags = FWPM_FILTER_FLAG_NONE;
        m_pFiltersArray[j].layerKey = FWPM_LAYER_IPFORWARD_V4;
        m_pFiltersArray[j].subLayerKey = FWPM_SUBLAYER_UNIVERSAL;
        m_pFiltersArray[j].weight.type = FWP_UINT64;
        m_pFiltersArray[j].weight.uint64 = &nFilterWeight;
        m_pFiltersArray[j].numFilterConditions = ulConditionsCount;
        m_pFiltersArray[j].filterCondition = m_pConditionsArray;
        m_pFiltersArray[j].action.type = FWP_ACTION_CALLOUT_TERMINATING;
        m_pFiltersArray[j].action.calloutKey = NATTRAVERSE_IPFORWARD_CALLOUT_V4;
        m_pConditionsArray[1].matchType = FWP_MATCH_EQUAL;
        m_pConditionsArray[1].conditionValue.type = FWP_UINT32;
        m_pConditionsArray[1].conditionValue.uint32 = nhIPAddress;
        dwResult = FwpmFilterAdd0(
            m_pOwner->m_hEngineHandle,
            &m_pFiltersArray[j],
            NULL,
            &pFilterIDsArray[j]
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
                _T("Ошибка выполнения функции \"FwpmFilterAdd0\".\n\tКод ошибки = %d (0x%08X): %s\n"),
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
            throw j;
        }
        j++;
    }
    for(i=0;i<nRangeIP;i++)
    {
        sSectionName.Format(
            _T("range_begin_ip%d"),
            i
            );
	    sIPAddress = GetConfigString(
            _T("service_settings"),
            sSectionName,
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
            DWORD   dwRetCount;
            LPVOID  lpMsgBuf;
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
                _T("Ошибка формата начального значения диапазона публичных IP-адресов!!\n\tКод ошибки = %d (0x%08X): %s\n"),
                nRet,
                nRet,
                (LPCTSTR)sErrorMessage
                );
            m_pOwner->m_nRetCode = 34;
            LPCTSTR pStrings[] = { (LPCTSTR)sMsg };
            theApp.ReportEventInvoke(
                EVENTLOG_ERROR_TYPE,
                MSG_ERROR_EVENT_TEXT,
                1,
                pStrings
                );
            throw j;
        }
        nhBeginAddress = RtlUlongByteSwap(stIP4Address.S_un.S_addr);
        sSectionName.Format(
            _T("range_end_ip%d"),
            i
            );
	    sIPAddress = GetConfigString(
            _T("service_settings"),
            sSectionName,
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
            DWORD   dwRetCount;
            LPVOID  lpMsgBuf;
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
                _T("Ошибка формата конечного значения диапазона публичных IP-адресов!!\n\tКод ошибки = %d (0x%08X): %s\n"),
                nRet,
                nRet,
                (LPCTSTR)sErrorMessage
                );
            m_pOwner->m_nRetCode = 35;
            LPCTSTR pStrings[] = { (LPCTSTR)sMsg };
            theApp.ReportEventInvoke(
                EVENTLOG_ERROR_TYPE,
                MSG_ERROR_EVENT_TEXT,
                1,
                pStrings
                );
            throw j;
        }
        nhEndAddress = RtlUlongByteSwap(stIP4Address.S_un.S_addr);
        m_pFiltersArray[j].displayData.name = L"NAT Travelsal IPFORWARD DESTINATION Filter";
        m_pFiltersArray[j].flags = FWPM_FILTER_FLAG_NONE;
        m_pFiltersArray[j].layerKey = FWPM_LAYER_IPFORWARD_V4;
        m_pFiltersArray[j].subLayerKey = FWPM_SUBLAYER_UNIVERSAL;
        m_pFiltersArray[j].weight.type = FWP_UINT64;
        m_pFiltersArray[j].weight.uint64 = &nFilterWeight;
        m_pFiltersArray[j].numFilterConditions = ulConditionsCount;
        m_pFiltersArray[j].filterCondition = m_pConditionsArray;
        m_pFiltersArray[j].action.type = FWP_ACTION_CALLOUT_TERMINATING;
        m_pFiltersArray[j].action.calloutKey = NATTRAVERSE_IPFORWARD_CALLOUT_V4;
        m_pConditionsArray[1].matchType = FWP_MATCH_RANGE;
        m_pConditionsArray[1].conditionValue.type = FWP_RANGE_TYPE;
        m_pConditionsArray[1].conditionValue.rangeValue = &m_pIp4RangeArray[nRangeIndex];
        m_pIp4RangeArray[nRangeIndex].valueLow.type  = FWP_UINT32;
        m_pIp4RangeArray[nRangeIndex].valueHigh.type = FWP_UINT32;
        m_pIp4RangeArray[nRangeIndex].valueLow.uint32  = nhBeginAddress;
        m_pIp4RangeArray[nRangeIndex].valueHigh.uint32 = nhEndAddress;
        dwResult = FwpmFilterAdd0(
            m_pOwner->m_hEngineHandle,
            &m_pFiltersArray[j],
            NULL,
            &pFilterIDsArray[j]
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
                _T("Ошибка выполнения функции \"FwpmFilterAdd0\".\n\tКод ошибки = %d (0x%08X): %s\n"),
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
            throw j;
        }
        nRangeIndex++;
        j++;
    }
    for(i=0;i<nSubnetIP;i++)
    {
        sSectionName.Format(
            _T("subnet_address_ip%d"),
            i
            );
	    sIPAddress = GetConfigString(
            _T("service_settings"),
            sSectionName,
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
            DWORD   dwRetCount;
            LPVOID  lpMsgBuf;
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
                _T("Ошибка формата адреса публичной IP-подсети!!\n\tКод ошибки = %d (0x%08X): %s\n"),
                nRet,
                nRet,
                (LPCTSTR)sErrorMessage
                );
            m_pOwner->m_nRetCode = nRet;
            LPCTSTR pStrings[] = { (LPCTSTR)sMsg };
            theApp.ReportEventInvoke(
                EVENTLOG_ERROR_TYPE,
                MSG_ERROR_EVENT_TEXT,
                1,
                pStrings
                );
            throw j;
        }
        m_pV4AddMaskArray[nSubnetIndex].addr = RtlUlongByteSwap(stIP4Address.S_un.S_addr);
        sSectionName.Format(
            _T("subnet_mask_ip%d"),
            i
            );
	    sIPAddress = GetConfigString(
            _T("service_settings"),
            sSectionName,
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
            DWORD   dwRetCount;
            LPVOID  lpMsgBuf;
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
                _T("Ошибка формата маски публичной IP-подсети!!\n\tКод ошибки = %d (0x%08X): %s\n"),
                nRet,
                nRet,
                (LPCTSTR)sErrorMessage
                );
            m_pOwner->m_nRetCode = nRet;
            LPCTSTR pStrings[] = { (LPCTSTR)sMsg };
            theApp.ReportEventInvoke(
                EVENTLOG_ERROR_TYPE,
                MSG_ERROR_EVENT_TEXT,
                1,
                pStrings
                );
            throw j;
        }
        m_pV4AddMaskArray[nSubnetIndex].mask = RtlUlongByteSwap(stIP4Address.S_un.S_addr);
        m_pFiltersArray[j].displayData.name = L"NAT Travelsal IPFORWARD DESTINATION Filter";
        m_pFiltersArray[j].flags = FWPM_FILTER_FLAG_NONE;
        m_pFiltersArray[j].layerKey = FWPM_LAYER_IPFORWARD_V4;
        m_pFiltersArray[j].subLayerKey = FWPM_SUBLAYER_UNIVERSAL;
        m_pFiltersArray[j].weight.type = FWP_UINT64;
        m_pFiltersArray[j].weight.uint64 = &nFilterWeight;
        m_pFiltersArray[j].numFilterConditions = ulConditionsCount;
        m_pFiltersArray[j].filterCondition = m_pConditionsArray;
        m_pFiltersArray[j].action.type = FWP_ACTION_CALLOUT_TERMINATING;
        m_pFiltersArray[j].action.calloutKey = NATTRAVERSE_IPFORWARD_CALLOUT_V4;
        m_pConditionsArray[1].matchType = FWP_MATCH_EQUAL;
        m_pConditionsArray[1].conditionValue.type = FWP_V4_ADDR_MASK;
        m_pConditionsArray[1].conditionValue.v4AddrMask = &m_pV4AddMaskArray[nSubnetIndex];
        dwResult = FwpmFilterAdd0(
            m_pOwner->m_hEngineHandle,
            &m_pFiltersArray[j],
            NULL,
            &pFilterIDsArray[j]
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
                _T("Ошибка выполнения функции \"FwpmFilterAdd0\".\n\tКод ошибки = %d (0x%08X): %s\n"),
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
            throw j;
        }
        nSubnetIndex++;
        j++;
    }
}
