#include "stdafx.h"
#include "DriverInitialize.h"
#include "NATTraversalService.h"
#include "MainWorkerThread.h"
#include "commrcfg.h"
#include "MessageTable.h"


CDriverInitialize::CDriverInitialize(
    CMainWorkerThread* pOwner
    )
: m_pOwner(pOwner)
, m_pInitByteBuffer(NULL)
{
}


CDriverInitialize::~CDriverInitialize(void)
{
    if(NULL != m_pInitByteBuffer)
    {
        delete m_pInitByteBuffer;
        m_pInitByteBuffer = NULL;
    }
}


void CDriverInitialize::InitFilterDriver(
    UINT32 nhRouterIPAddress,
    int nSingleIP,
    int nRangeIP,
    int nSubnetIP,
    int nSinglePrivateIP,
    int nRangePrivateIP,
    int nSubnetPrivateIP,
    int nDisabledSinglePorts,
    int nDisabledRangePorts
    )
{
    int     nRequestedSize, j = -1, i;
    PBYTE   pCurrentPosition;
    PUINT32 pSingleIP4Array = NULL;
    CString sSectionName, sIPAddress;
    LONG    nRet;
    LPCTSTR pTerminator;
    IN_ADDR stIP4Address;
    u_long  nhIPAddress, nhBeginAddress, nhEndAddress;
    ULONG   ulBPort, ulEPort;
    BOOL    bRet;
    DWORD   dwResult = 0;
    PUINT16 pSinglePortArray = NULL;
    PFILTER_INIT_STRUCT     pFilterInitStructure;
    PIP4ADDR_RANGE_STRUCT   pIP4RangeArray = NULL;
    PIP4SUBNET_STRUCT       pIP4SubnetArray = NULL;
    PIP_PORT_RANGE_STRUCT   pPortRangeArray = NULL;
    if(INVALID_HANDLE_VALUE == m_pOwner->m_hFilterDriver)
    {
        throw j;
    }
    nRequestedSize = sizeof(FILTER_INIT_STRUCT)
        + nSingleIP*sizeof(UINT32)
        + nSinglePrivateIP*sizeof(UINT32)
        + nRangeIP*sizeof(IP4ADDR_RANGE_STRUCT)
        + nRangePrivateIP*sizeof(IP4ADDR_RANGE_STRUCT)
        + nSubnetIP*sizeof(IP4SUBNET_STRUCT)
        + nSubnetPrivateIP*sizeof(IP4SUBNET_STRUCT)
        + nDisabledSinglePorts*sizeof(UINT16)
        + nDisabledRangePorts*sizeof(IP_PORT_RANGE_STRUCT);
    j = -2;
    try
    {
        m_pInitByteBuffer = new BYTE[nRequestedSize];
    }
    catch(...)
    {
        m_pInitByteBuffer = NULL;
        throw j;
    }
    j = 0;
    RtlZeroMemory(
        m_pInitByteBuffer,
        nRequestedSize
        );
    pFilterInitStructure = (PFILTER_INIT_STRUCT)m_pInitByteBuffer;
    pCurrentPosition = m_pInitByteBuffer + sizeof(FILTER_INIT_STRUCT);
    pFilterInitStructure->_ulRouterAddress       = nhRouterIPAddress;
    pFilterInitStructure->_nSinglePublicIpCount  = nSingleIP;
    pFilterInitStructure->_nSinglePrivateIpCount = nSinglePrivateIP;
    pFilterInitStructure->_nPublicRangeIpCount   = nRangeIP;
    pFilterInitStructure->_nPrivateRangeIpCount  = nRangePrivateIP;
    pFilterInitStructure->_nPublicSubnetIpCount  = nSubnetIP;
    pFilterInitStructure->_nPrivateSubnetIpCount = nSubnetPrivateIP;
    pFilterInitStructure->_nSinglePortCount      = nDisabledSinglePorts;
    pFilterInitStructure->_nRangePortCount       = nDisabledRangePorts;
    if(nSingleIP > 0)
    {
        pSingleIP4Array = (PUINT32)pCurrentPosition;
        pCurrentPosition += nSingleIP*sizeof(UINT32);
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
            pSingleIP4Array[i] = nhIPAddress;
            j++;
        }
    }
    if(nSinglePrivateIP > 0)
    {
        pSingleIP4Array = (PUINT32)pCurrentPosition;
        pCurrentPosition += nSinglePrivateIP*sizeof(UINT32);
        for(i=0;i<nSinglePrivateIP;i++)
        {
            sSectionName.Format(
                _T("private_single_ip%d"),
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
                    _T("Ошибка формата приватного IP-адреса.\n\tКод ошибки = %d (0x%08X): %s\n"),
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
            pSingleIP4Array[i] = nhIPAddress;
            j++;
        }
    }
    if(nRangeIP > 0)
    {
        pIP4RangeArray = (PIP4ADDR_RANGE_STRUCT)pCurrentPosition;
        pCurrentPosition += nRangeIP*sizeof(IP4ADDR_RANGE_STRUCT);
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
            pIP4RangeArray[i]._ulLowValue  = nhBeginAddress;
            pIP4RangeArray[i]._ulHighValue = nhEndAddress;
            j++;
        }
    }
    if(nRangePrivateIP > 0)
    {
        pIP4RangeArray = (PIP4ADDR_RANGE_STRUCT)pCurrentPosition;
        pCurrentPosition += nRangePrivateIP*sizeof(IP4ADDR_RANGE_STRUCT);
        for(i=0;i<nRangePrivateIP;i++)
        {
            sSectionName.Format(
                _T("private_range_begin_ip%d"),
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
                    _T("Ошибка формата начального значения диапазона приватных IP-адресов!!\n\tКод ошибки = %d (0x%08X): %s\n"),
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
            nhBeginAddress = ntohl(stIP4Address.S_un.S_addr);
            sSectionName.Format(
                _T("private_range_end_ip%d"),
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
            nhEndAddress = ntohl(stIP4Address.S_un.S_addr);
            pIP4RangeArray[i]._ulLowValue  = nhBeginAddress;
            pIP4RangeArray[i]._ulHighValue = nhEndAddress;
            j++;
        }
    }
    if(nSubnetIP > 0)
    {
        pIP4SubnetArray = (PIP4SUBNET_STRUCT)pCurrentPosition;
        pCurrentPosition += nSubnetIP*sizeof(IP4SUBNET_STRUCT);
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
            pIP4SubnetArray[i]._ulIpAddr = RtlUlongByteSwap(stIP4Address.S_un.S_addr);
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
            pIP4SubnetArray[i]._ulIpMask = RtlUlongByteSwap(stIP4Address.S_un.S_addr);
            j++;
        }
    }
    if(nSubnetPrivateIP > 0)
    {
        pIP4SubnetArray = (PIP4SUBNET_STRUCT)pCurrentPosition;
        pCurrentPosition += nSubnetPrivateIP*sizeof(IP4SUBNET_STRUCT);
        for(i=0;i<nSubnetPrivateIP;i++)
        {
            sSectionName.Format(
                _T("private_subnet_address_ip%d"),
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
                    _T("Ошибка формата адреса приватной IP-подсети!!\n\tКод ошибки = %d (0x%08X): %s\n"),
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
            pIP4SubnetArray[i]._ulIpAddr = RtlUlongByteSwap(stIP4Address.S_un.S_addr);
            sSectionName.Format(
                _T("private_subnet_mask_ip%d"),
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
                    _T("Ошибка формата маски приватной IP-подсети!!\n\tКод ошибки = %d (0x%08X): %s\n"),
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
            pIP4SubnetArray[i]._ulIpMask = RtlUlongByteSwap(stIP4Address.S_un.S_addr);
            j++;
        }
    }
    if(nDisabledSinglePorts > 0)
    {
        pSinglePortArray = (PUINT16)pCurrentPosition;
        pCurrentPosition += nDisabledSinglePorts*sizeof(UINT16);
        for(i=0;i<nDisabledSinglePorts;i++)
        {
            sSectionName.Format(
                _T("single_ip_port%d"),
                i
                );
	        ulBPort = GetConfigInt(
                _T("service_settings"),
                sSectionName,
                0
                );
            ASSERT(ulBPort > 0);
            pSinglePortArray[i]  = (UINT16)ulBPort;
            j++;
        }
    }
    if(nDisabledRangePorts > 0)
    {
        pPortRangeArray = (PIP_PORT_RANGE_STRUCT)pCurrentPosition;
        pCurrentPosition += nDisabledRangePorts*sizeof(IP_PORT_RANGE_STRUCT);
        for(i=0;i<nDisabledRangePorts;i++)
        {
            sSectionName.Format(
                _T("range_begin_ip_port%d"),
                i
                );
	        ulBPort = GetConfigInt(
                _T("service_settings"),
                sSectionName,
                0
                );
            ASSERT(ulBPort > 0);
            sSectionName.Format(
                _T("range_end_ip_port%d"),
                i
                );
	        ulEPort = GetConfigInt(
                _T("service_settings"),
                sSectionName,
                0
                );
            ASSERT(ulEPort > 0);
            pPortRangeArray[i]._usLowValue  = (UINT16)ulBPort;
            pPortRangeArray[i]._usHighValue = (UINT16)ulEPort;
            j++;
        }
    }
    j = -3;
    bRet = DeviceIoControl(
        m_pOwner->m_hFilterDriver,
        NATTRAVERSE_IOCTL_INIT_FILTERS,
        m_pInitByteBuffer,
        (DWORD)nRequestedSize,
        NULL,
        0,
        &dwResult,
        NULL
        );
    if(!bRet)
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
            _T("Ошибка выполнения запроса ициализации драйвера CALLOUT-фильтра!!\n\tКод ошибки = %d (0x%08X): %s\n"),
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
}
