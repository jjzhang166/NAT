// NATTraversalUninstaller.cpp : implementation file
//

#include "stdafx.h"
#include "NATTraversalUninstaller.h"
#include "MessageTable.h"
#include "commrcfg.h"

CNATTraversalUninstallerApp theApp;

// CNATTraversalUninstallerApp

CNATTraversalUninstallerApp::CNATTraversalUninstallerApp()
: m_nRetCode(0)
, m_sModuleFileName(_T(""))
, m_sModuleFolder(_T(""))
, m_sInfFilePath(_T(""))
, m_sFilterDriverFolder(_T(""))
, m_sCoInstallerPath(_T(""))
, m_sDriversFolder(_T(""))
, m_hCoInatallerModule(NULL)
, m_pfnWdfPreDeviceInstall(NULL)
, m_pfnWdfPostDeviceInstall(NULL)
, m_pfnWdfPreDeviceInstallEx(NULL)
, m_pfnWdfPreDeviceRemove(NULL)
, m_pfnWdfPostDeviceRemove(NULL)
, m_bRebootNeed(false)
{
}

CNATTraversalUninstallerApp::~CNATTraversalUninstallerApp()
{
}

BOOL CNATTraversalUninstallerApp::InitInstance()
{
    SC_HANDLE       schSCManager;
    SC_HANDLE       schService;
    DWORD           nErrorCode;
    LONG            nResult;
    SERVICE_STATUS  st;
    BOOL            bRet;
    DWORD dwData, dwDisp;
    HKEY            hk;
    TCHAR           szPath[MAX_PATH];
    DWORD           dwModuleNameLength;
    int             nCharPos;
	SetRegistryKey(_T("Real Time Systems DD"));
    if(m_pszProfileName != NULL)
        free((void*)m_pszProfileName);
    m_pszProfileName = _tcsdup(_T("rtsdd_parameters"));
    dwModuleNameLength = GetModuleFileName( NULL, szPath, MAX_PATH );
    m_sModuleFileName = szPath;
    nCharPos = m_sModuleFileName.ReverseFind(_T('\\'));
    m_sModuleFolder = m_sModuleFileName.Left(nCharPos);
    // регистрация источника сообщений
    nResult = RegCreateKeyEx(
        HKEY_LOCAL_MACHINE,
        _T("SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\NATTraversalUninstaller"),
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_WRITE,
        NULL,
        &hk,
        &dwDisp
        );
    nResult = RegSetValueEx(
        hk,
        _T("EventMessageFile"),
        0,
        REG_EXPAND_SZ,
        (LPBYTE)szPath,
        (DWORD)(lstrlen(szPath)+1)*sizeof(TCHAR)
        );
    dwData = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE;
    nResult = RegSetValueEx(
        hk,
        _T("TypesSupported"),
        0,
        REG_DWORD,
        (LPBYTE)&dwData,
        sizeof(DWORD)
        );
    nResult = RegCloseKey(hk);
    // удаление сервиса "NATTraversalService"
    schSCManager = OpenSCManager(
        NULL,                 // local machine
        NULL,                 // ServicesActive database
        SC_MANAGER_ALL_ACCESS // full access rights
        );
    if(NULL == schSCManager) {
        nErrorCode = ::GetLastError();
        DWORD dwRetCount;
        LPVOID lpMsgBuf;
        CString sMsg, sErrorMessage;
        dwRetCount = FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            nErrorCode,
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
            _T("Ошибка вызова функции \"OpenSCManager\"!!\r\n\tКод ошибки = %d (0x%08X): %s\r\n"),
            nErrorCode,
            nErrorCode,
            (LPCTSTR)sErrorMessage
            );
        m_nRetCode = nErrorCode;
        LPCTSTR pStrings[] = { (LPCTSTR)sMsg };
        ReportEventInvoke(
            EVENTLOG_ERROR_TYPE,
            MSG_ERROR_EVENT_TEXT,
            1,
            pStrings
            );
        return FALSE;
    }
    schService = OpenService(
        schSCManager,
        _T("NATTraversalService"),
        SERVICE_ALL_ACCESS
        );
    if(NULL == schService) {
        nErrorCode = ::GetLastError();
        CloseServiceHandle(schSCManager);
        if(ERROR_SERVICE_DOES_NOT_EXIST == nErrorCode)
        {
            LPCTSTR pStrings[] = { _T("Сервис \"NATTraversalService\" не был инсталирован и не требует деинсталляции.") };
            ReportEventInvoke(
                EVENTLOG_INFORMATION_TYPE,
                MSG_INFORMATIONAL_EVENT_TEXT,
                1,
                pStrings
                );
            goto lUninstallDriver;
        }
        DWORD dwRetCount;
        LPVOID lpMsgBuf;
        CString sMsg, sErrorMessage;
        dwRetCount = FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            nErrorCode,
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
            _T("Ошибка вызова функции \"OpenService\"!!\r\n\tКод ошибки = %d (0x%08X): %s\r\n"),
            nErrorCode,
            nErrorCode,
            (LPCTSTR)sErrorMessage
            );
        m_nRetCode = nErrorCode;
        LPCTSTR pStrings[] = { (LPCTSTR)sMsg };
        ReportEventInvoke(
            EVENTLOG_ERROR_TYPE,
            MSG_ERROR_EVENT_TEXT,
            1,
            pStrings
            );
        return FALSE;
    }
    RtlZeroMemory(
        &st,
        sizeof(st)
        );
    QueryServiceStatus( schService, &st );
    if(SERVICE_RUNNING == st.dwCurrentState)
    {
        bRet = ControlService(
            schService,
            SERVICE_CONTROL_STOP,
            &st
            );
    }
    bRet = DeleteService(schService);
    if(!bRet)
    {
        nErrorCode = ::GetLastError();
        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);
        DWORD dwRetCount;
        LPVOID lpMsgBuf;
        CString sMsg, sErrorMessage;
        dwRetCount = FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            nErrorCode,
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
            _T("Ошибка вызова функции \"DeleteService\"!!\r\n\tКод ошибки = %d (0x%08X): %s\r\n"),
            nErrorCode,
            nErrorCode,
            (LPCTSTR)sErrorMessage
            );
        m_nRetCode = nErrorCode;
        LPCTSTR pStrings[] = { (LPCTSTR)sMsg };
        ReportEventInvoke(
            EVENTLOG_ERROR_TYPE,
            MSG_ERROR_EVENT_TEXT,
            1,
            pStrings
            );
        return FALSE;
    }
    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
    // удаление регистрации источника сообщений
    nResult = RegDeleteKey(
        HKEY_LOCAL_MACHINE,
        _T("SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\NATTraversalService")
        );
    // удаление ключа сервиса в реестре
    nResult = RegDeleteKey(
        HKEY_LOCAL_MACHINE,
        _T("SYSTEM\\CurrentControlSet\\Services\\NATTraversalService")
        );
    {
        LPCTSTR pStrings[] = { _T("Сервис \"NATTraversalService\" успешно удалён.") };
        ReportEventInvoke(
            EVENTLOG_INFORMATION_TYPE,
            MSG_INFORMATIONAL_EVENT_TEXT,
            1,
            pStrings
            );
    }
lUninstallDriver:
    {
        // удаление пакета драйвера из системы
        HINF                    hInfHandle;
        UINT                    nErrorLine, nPathLength;
        INFCONTEXT              stInfContext;
        TCHAR                   szTextBuffer[MAX_PATH];
        CString sTextBuffer, sMajorVer, sMinorVer;
        int                     nPos;
        long    nMajorVer, nMinorVer;
        ULONG                   ulRetCode;
        HSPFILEQ                hFileQueue;
        PVOID                   pDefaultCBContext;
        DWORD                   cbBytesNeeded = 0;
        SERVICE_STATUS          stServiceStatus;
        SERVICE_STATUS_PROCESS  stServiceProcessStatus;
        HDEVINFO                hDevsInfoSet;
        SP_DEVINFO_DATA         stDevInfoData;
        SP_DRVINFO_DATA         stDrvInfoData;
        CONFIGRET               cr;
        bool                    bDeviceNotInstalled = false;
        PBYTE                   pDrvinfoDetailDataBuffer = NULL;
        PSP_DRVINFO_DETAIL_DATA pDrvinfoDetailData = NULL;
        DWORD                   dwDriverInfoDetailDataSize = sizeof(SP_DRVINFO_DETAIL_DATA);
        DWORD                   dwRequiredSize = 0;
        CString sOemInfPath, sOemInfName, sMessageText;
        TCHAR                   szInfFilePath[MAX_PATH];
        DWORD                   dwErrorCode;
        schSCManager = OpenSCManager(
            NULL,                 // local machine
            NULL,                 // ServicesActive database
            SC_MANAGER_ALL_ACCESS // full access rights
            );
        ASSERT(NULL != schSCManager);
        schService = OpenService(
            schSCManager,
            DRIVER_SERVICE_NAME,
            SERVICE_ALL_ACCESS
            );
        if(NULL == schService)
        {
            nErrorCode = ::GetLastError();
            CloseServiceHandle(schSCManager);
            if(ERROR_SERVICE_DOES_NOT_EXIST == nErrorCode)
            {
                LPCTSTR pStrings[] = { _T("Драйвер \"nattraverse_kmdffilter\" не установлен! Нет необходимости деинсталлировать его.") };
                ReportEventInvoke(
                    EVENTLOG_INFORMATION_TYPE,
                    MSG_INFORMATIONAL_EVENT_TEXT,
                    1,
                    pStrings
                    );
                return FALSE;
            }
            DWORD dwRetCount;
            LPVOID lpMsgBuf;
            CString sMsg, sErrorMessage;
            dwRetCount = FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                NULL,
                nErrorCode,
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
                _T("Ошибка вызова функции \"OpenService\" для сервиса драйвера!!\r\n\tКод ошибки = %d (0x%08X): %s\r\n"),
                nErrorCode,
                nErrorCode,
                (LPCTSTR)sErrorMessage
                );
            m_nRetCode = nErrorCode;
            LPCTSTR pStrings[] = { (LPCTSTR)sMsg };
            ReportEventInvoke(
                EVENTLOG_ERROR_TYPE,
                MSG_ERROR_EVENT_TEXT,
                1,
                pStrings
                );
            return FALSE;
        }
        hDevsInfoSet = SetupDiCreateDeviceInfoList(
            NULL,
            NULL
            );
        ASSERT(INVALID_HANDLE_VALUE != hDevsInfoSet);
        stDevInfoData.cbSize = sizeof(stDevInfoData);
        bRet = SetupDiOpenDeviceInfo(
            hDevsInfoSet,
            _T("Root\\LEGACY_NATTRAVERSE_KMDFFILTER\\0000"),
            NULL,
            0,
            &stDevInfoData
            );
        if(!bRet)
        {
            bDeviceNotInstalled = true;
            bRet = SetupDiCreateDeviceInfo(
                hDevsInfoSet,
                _T("Root\\LEGACY_NATTRAVERSE_KMDFFILTER\\0000"),
                &GUID_NULL,
                NULL,
                NULL,
                0,
                &stDevInfoData
                );
            ASSERT(bRet);
        }
        cr = CM_Add_ID(
            stDevInfoData.DevInst,
            _T("Root\\nattraverse_kmdffilter"),
            CM_ADD_ID_HARDWARE
            );
        ASSERT(CR_SUCCESS == cr);
        bRet = SetupDiBuildDriverInfoList(
            hDevsInfoSet,
            &stDevInfoData,
            SPDIT_COMPATDRIVER
            );
        ASSERT(bRet);
        RtlZeroMemory(
            &stDrvInfoData,
            sizeof(stDrvInfoData)
            );
        stDrvInfoData.cbSize = sizeof(stDrvInfoData);
        bRet = SetupDiEnumDriverInfo(
            hDevsInfoSet,
            &stDevInfoData,
            SPDIT_COMPATDRIVER,
            0,
            &stDrvInfoData
            );
        ASSERT(bRet);
        pDrvinfoDetailDataBuffer = new BYTE[dwDriverInfoDetailDataSize];
        pDrvinfoDetailData = (PSP_DRVINFO_DETAIL_DATA)pDrvinfoDetailDataBuffer;
        pDrvinfoDetailData->cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);
        bRet = SetupDiGetDriverInfoDetail(
            hDevsInfoSet,
            &stDevInfoData,
            &stDrvInfoData,
            pDrvinfoDetailData,
            dwDriverInfoDetailDataSize,
            &dwRequiredSize
            );
        if(!bRet)
        {
            dwErrorCode = ::GetLastError();
            if(ERROR_INSUFFICIENT_BUFFER == dwErrorCode)
            {
                delete pDrvinfoDetailDataBuffer;
                pDrvinfoDetailDataBuffer = new BYTE[dwRequiredSize];
                pDrvinfoDetailData = (PSP_DRVINFO_DETAIL_DATA)pDrvinfoDetailDataBuffer;
                pDrvinfoDetailData->cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);
                bRet = SetupDiGetDriverInfoDetail(
                    hDevsInfoSet,
                    &stDevInfoData,
                    &stDrvInfoData,
                    pDrvinfoDetailData,
                    dwRequiredSize,
                    NULL
                    );
                if(!bRet)
                {
                    dwErrorCode = ::GetLastError();
                }
            }
        }
        ASSERT(bRet);
        sOemInfPath = pDrvinfoDetailData->InfFileName;
        delete pDrvinfoDetailDataBuffer;
        nPos = sOemInfPath.ReverseFind(_T('\\'));
        sOemInfName = sOemInfPath.Mid(nPos + 1);
        bRet = SetupDiDestroyDriverInfoList(
            hDevsInfoSet,
            &stDevInfoData,
            SPDIT_COMPATDRIVER
            );
        bRet = SetupDiDeleteDeviceInfo(
            hDevsInfoSet,
            &stDevInfoData
            );
        if(!bRet)
        {
            dwErrorCode = ::GetLastError();
            sMessageText.Format(
                _T("0x%08X"),
                dwErrorCode
                );
        }
        bRet = SetupDiDestroyDeviceInfoList(hDevsInfoSet);
        bRet = SetupGetInfDriverStoreLocation(
            (LPCTSTR)sOemInfPath,
            NULL,
            NULL,
            szInfFilePath,
            MAX_PATH,
            &dwRequiredSize
            );
        ASSERT(bRet);
        m_sInfFilePath = szInfFilePath;
        nPos = m_sInfFilePath.ReverseFind(_T('\\'));
        m_sFilterDriverFolder = m_sInfFilePath.Left(nPos);
        hInfHandle = SetupOpenInfFile(
            m_sInfFilePath,
            NULL,
            INF_STYLE_WIN4,
            &nErrorLine
            );
        if(INVALID_HANDLE_VALUE == hInfHandle)
        {
            bRet = CloseServiceHandle(schService);
            bRet = CloseServiceHandle(schSCManager);
            DWORD dwRetCount;
            LPVOID lpMsgBuf;
            DWORD   nRet;
            nRet = ::GetLastError();
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
                _T("Ошибка открытия INF-файла!!\n\tКод ошибки = %d (0x%08X): %s\n\tНомер строки INF-файла = %d\n"),
                nRet,
                nRet,
                (LPCTSTR)sErrorMessage,
                nErrorLine
                );
            m_nRetCode = nRet;
            LPCTSTR pStrings[] = { (LPCTSTR)sMsg };
            ReportEventInvoke(
                EVENTLOG_ERROR_TYPE,
                MSG_ERROR_EVENT_TEXT,
                1,
                pStrings
                );
            return FALSE;
        }
        bRet = SetupFindFirstLine(
            hInfHandle,
            _T("nattraverse_kmdffilter_wdfsect"),
            _T("KmdfLibraryVersion"),
            &stInfContext
            );
        if(!bRet)
        {
            SetupCloseInfFile(hInfHandle);
            bRet = CloseServiceHandle(schService);
            bRet = CloseServiceHandle(schSCManager);
            DWORD dwRetCount;
            LPVOID lpMsgBuf;
            DWORD   nRet;
            nRet = ::GetLastError();
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
                _T("Ошибка поиска строки значения \"KmdfLibraryVersion\"!!\n\tКод ошибки = %d (0x%08X): %s\n"),
                nRet,
                nRet,
                (LPCTSTR)sErrorMessage
                );
            m_nRetCode = nRet;
            LPCTSTR pStrings[] = { (LPCTSTR)sMsg };
            ReportEventInvoke(
                EVENTLOG_ERROR_TYPE,
                MSG_ERROR_EVENT_TEXT,
                1,
                pStrings
                );
            return FALSE;
        }
        bRet = SetupGetStringField(
            &stInfContext,
            1,
            szTextBuffer,
            MAX_PATH,
            &dwRequiredSize
            );
        if(!bRet)
        {
            SetupCloseInfFile(hInfHandle);
            bRet = CloseServiceHandle(schService);
            bRet = CloseServiceHandle(schSCManager);
            DWORD dwRetCount;
            LPVOID lpMsgBuf;
            DWORD   nRet;
            nRet = ::GetLastError();
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
                _T("Ошибка получения строки значения \"KmdfLibraryVersion\"!!\n\tКод ошибки = %d (0x%08X): %s\n"),
                nRet,
                nRet,
                (LPCTSTR)sErrorMessage
                );
            m_nRetCode = nRet;
            LPCTSTR pStrings[] = { (LPCTSTR)sMsg };
            ReportEventInvoke(
                EVENTLOG_ERROR_TYPE,
                MSG_ERROR_EVENT_TEXT,
                1,
                pStrings
                );
            return FALSE;
        }
        sTextBuffer = szTextBuffer;
        nPos = sTextBuffer.Find(_T('.'));
        sMajorVer = sTextBuffer.Left(nPos);
        sMinorVer = sTextBuffer.Mid(nPos + 1);
        nMajorVer = _tstol(sMajorVer);
        nMinorVer = _tstol(sMinorVer);
        sTextBuffer.Format(
            _T("%02d%03d"),
            nMajorVer,
            nMinorVer
            );
        m_sCoInstallerPath  = m_sFilterDriverFolder;
        m_sCoInstallerPath += _T("\\WdfCoInstaller");
        m_sCoInstallerPath += sTextBuffer;
        m_sCoInstallerPath += _T(".dll");
        nPathLength = GetWindowsDirectory(
            szTextBuffer,
            MAX_PATH
            );
        ASSERT(nPathLength > 1);
        m_sDriversFolder  = szTextBuffer;
        m_sDriversFolder += _T("\\System32\\drivers");
        // загрузка КоИнсталлера
        m_hCoInatallerModule = LoadLibrary(m_sCoInstallerPath);
        if(NULL == m_hCoInatallerModule)
        {
            SetupCloseInfFile(hInfHandle);
            bRet = CloseServiceHandle(schService);
            bRet = CloseServiceHandle(schSCManager);
            DWORD dwRetCount;
            LPVOID lpMsgBuf;
            DWORD   nRet;
            nRet = ::GetLastError();
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
                _T("Ошибка загрузки \"КоИнсталлера\"!!\n\tКод ошибки = %d (0x%08X): %s\n"),
                nRet,
                nRet,
                (LPCTSTR)sErrorMessage
                );
            m_nRetCode = nRet;
            LPCTSTR pStrings[] = { (LPCTSTR)sMsg };
            ReportEventInvoke(
                EVENTLOG_ERROR_TYPE,
                MSG_ERROR_EVENT_TEXT,
                1,
                pStrings
                );
            return FALSE;
        }
        m_pfnWdfPreDeviceInstall = (PFN_WDFPREDEVICEINSTALL)GetProcAddress(
            m_hCoInatallerModule,
            "WdfPreDeviceInstall"
            );
        ASSERT(NULL != m_pfnWdfPreDeviceInstall);
        m_pfnWdfPostDeviceInstall = (PFN_WDFPOSTDEVICEINSTALL)GetProcAddress(
            m_hCoInatallerModule,
            "WdfPostDeviceInstall"
            );
        ASSERT(NULL != m_pfnWdfPostDeviceInstall);
        m_pfnWdfPreDeviceInstallEx = (PFN_WDFPREDEVICEINSTALLEX)GetProcAddress(
            m_hCoInatallerModule,
            "WdfPreDeviceInstallEx"
            );
        ASSERT(NULL != m_pfnWdfPreDeviceInstallEx);
        m_pfnWdfPreDeviceRemove = (PFN_WDFPREDEVICEREMOVE)GetProcAddress(
            m_hCoInatallerModule,
            "WdfPreDeviceRemove"
            );
        ASSERT(NULL != m_pfnWdfPreDeviceRemove);
        m_pfnWdfPostDeviceRemove = (PFN_WDFPOSTDEVICEREMOVE)GetProcAddress(
            m_hCoInatallerModule,
            "WdfPostDeviceRemove"
            );
        ASSERT(NULL != m_pfnWdfPostDeviceRemove);
        //
        // PRE-REMOVE of WDF support
        //
        ulRetCode = m_pfnWdfPreDeviceRemove(
            m_sInfFilePath,
            WDF_SECTION_NAME
            );
        if(ERROR_SUCCESS != ulRetCode)
        {
            DWORD dwRetCount;
            LPVOID lpMsgBuf;
            CString sMsg, sErrorMessage;
            dwRetCount = FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_FROM_HMODULE,
                m_hCoInatallerModule,
                ulRetCode,
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
            bRet = FreeLibrary(m_hCoInatallerModule);
            SetupCloseInfFile(hInfHandle);
            bRet = CloseServiceHandle(schService);
            bRet = CloseServiceHandle(schSCManager);
            sMsg.Format(
                _T("Ошибка вызова функции \"WdfPreDeviceRemove\"!!\n\tКод ошибки = %d (0x%08X): %s\n"),
                ulRetCode,
                ulRetCode,
                (LPCTSTR)sErrorMessage
                );
            m_nRetCode = ulRetCode;
            LPCTSTR pStrings[] = { (LPCTSTR)sMsg };
            ReportEventInvoke(
                EVENTLOG_ERROR_TYPE,
                MSG_ERROR_EVENT_TEXT,
                1,
                pStrings
                );
            return FALSE;
        }
        // деинсталляция сервиса драйвера
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
            if(!bRet)
            {
                DWORD dwErrorCode = GetLastError();
                bRet = FreeLibrary(m_hCoInatallerModule);
                SetupCloseInfFile(hInfHandle);
                bRet = CloseServiceHandle(schService);
                bRet = CloseServiceHandle(schSCManager);
                DWORD dwRetCount;
                LPVOID lpMsgBuf;
                CString sMsg, sErrorMessage;
                dwRetCount = FormatMessage(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                    NULL,
                    dwErrorCode,
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
                    _T("Ошибка остановки работающего сервиса драйвера!!\n\tКод ошибки = %d (0x%08X): %s\n"),
                    dwErrorCode,
                    dwErrorCode,
                    (LPCTSTR)sErrorMessage
                    );
                m_nRetCode = ulRetCode;
                LPCTSTR pStrings[] = { (LPCTSTR)sMsg };
                ReportEventInvoke(
                    EVENTLOG_ERROR_TYPE,
                    MSG_ERROR_EVENT_TEXT,
                    1,
                    pStrings
                    );
                return FALSE;
            }
        }
        bRet = DeleteService(schService);
        if(!bRet)
        {
            dwErrorCode = ::GetLastError();
            ASSERT(ERROR_SERVICE_MARKED_FOR_DELETE == dwErrorCode);
            m_bRebootNeed = true;
        }
        bRet = CloseServiceHandle(schService);
        bRet = CloseServiceHandle(schSCManager);
        //
        // POST-REMOVE of WDF support
        //
        ulRetCode = m_pfnWdfPostDeviceRemove(
            m_sInfFilePath,
            WDF_SECTION_NAME
            );
        if(ERROR_SUCCESS != ulRetCode)
        {
            DWORD dwRetCount;
            LPVOID lpMsgBuf;
            CString sMsg, sErrorMessage;
            dwRetCount = FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_FROM_HMODULE,
                m_hCoInatallerModule,
                ulRetCode,
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
            bRet = FreeLibrary(m_hCoInatallerModule);
            SetupCloseInfFile(hInfHandle);
            sMsg.Format(
                _T("Ошибка вызова функции \"WdfPostDeviceRemove\"!!\n\tКод ошибки = %d (0x%08X): %s\n"),
                ulRetCode,
                ulRetCode,
                (LPCTSTR)sErrorMessage
                );
            m_nRetCode = ulRetCode;
            LPCTSTR pStrings[] = { (LPCTSTR)sMsg };
            ReportEventInvoke(
                EVENTLOG_ERROR_TYPE,
                MSG_ERROR_EVENT_TEXT,
                1,
                pStrings
                );
            return FALSE;
        }
        bRet = FreeLibrary(m_hCoInatallerModule);
        // удаление файла драйвера
        hFileQueue = SetupOpenFileQueue();
        if(INVALID_HANDLE_VALUE == hFileQueue)
        {
            DWORD dwErrorCode = GetLastError();
            SetupCloseInfFile(hInfHandle);
            DWORD dwRetCount;
            LPVOID lpMsgBuf;
            CString sMsg, sErrorMessage;
            dwRetCount = FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                NULL,
                dwErrorCode,
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
                _T("Ошибка открытия очереди устанавливаемых файлов!!\n\tКод ошибки = %d (0x%08X): %s\n"),
                dwErrorCode,
                dwErrorCode,
                (LPCTSTR)sErrorMessage
                );
            m_nRetCode = 0;
            {
                LPCTSTR pStrings[] = { _T("Файл драйвера не удалён из папки драйверов!\nМожете удалить его вручную позже.\n") };
                ReportEventInvoke(
                    EVENTLOG_WARNING_TYPE,
                    MSG_WARNING_EVENT_TEXT,
                    1,
                    pStrings
                    );
            }
            m_bRebootNeed = true;
            goto lRemoveDriverPackage;
        }
        bRet = SetupQueueDeleteSection(
            hFileQueue,
            hInfHandle,
            NULL,
            _T("Drivers_Dir")
            );
        if(!bRet)
        {
            DWORD dwErrorCode = GetLastError();
            SetupCloseInfFile(hInfHandle);
            DWORD dwRetCount;
            LPVOID lpMsgBuf;
            CString sMsg, sErrorMessage;
            dwRetCount = FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                NULL,
                dwErrorCode,
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
                _T("Ошибка удаления секции [Drivers_Dir]!!\n\tКод ошибки = %d (0x%08X): %s\n"),
                dwErrorCode,
                dwErrorCode,
                (LPCTSTR)sErrorMessage
                );
            m_nRetCode = 0;
            {
                LPCTSTR pStrings[] = { _T("Файл драйвера не удалён из папки драйверов!\nМожете удалить его вручную позже.\n") };
                ReportEventInvoke(
                    EVENTLOG_WARNING_TYPE,
                    MSG_WARNING_EVENT_TEXT,
                    1,
                    pStrings
                    );
            }
            m_bRebootNeed = true;
            goto lRemoveDriverPackage;
        }
        pDefaultCBContext = SetupInitDefaultQueueCallback(NULL);
        ASSERT(NULL != pDefaultCBContext);
        bRet = SetupCommitFileQueue(
            NULL,
            hFileQueue,
            SetupDefaultQueueCallback,
            pDefaultCBContext
            );
        if(!bRet)
        {
            DWORD dwErrorCode = GetLastError();
            SetupCloseFileQueue(hFileQueue);
            SetupCloseInfFile(hInfHandle);
            DWORD dwRetCount;
            LPVOID lpMsgBuf;
            CString sMsg, sErrorMessage;
            dwRetCount = FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                NULL,
                dwErrorCode,
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
                _T("Ошибка подтверждения очереди файлов!!\n\tКод ошибки = %d (0x%08X): %s\n"),
                dwErrorCode,
                dwErrorCode,
                (LPCTSTR)sErrorMessage
                );
            m_nRetCode = 0;
            {
                LPCTSTR pStrings[] = { _T("Файл драйвера не удалён из папки драйверов!\nМожете удалить его вручную позже.\n") };
                ReportEventInvoke(
                    EVENTLOG_WARNING_TYPE,
                    MSG_WARNING_EVENT_TEXT,
                    1,
                    pStrings
                    );
            }
            m_bRebootNeed = true;
            goto lRemoveDriverPackage;
        }
        SetupTermDefaultQueueCallback(pDefaultCBContext);
        SetupCloseFileQueue(hFileQueue);
        SetupCloseInfFile(hInfHandle);
lRemoveDriverPackage:
        // удаление пакета драйвера из хранилища драйверов
        bRet = SetupUninstallOEMInf(
            sOemInfName,
            SUOI_FORCEDELETE,
            NULL
            );
        if(!bRet)
        {
            dwErrorCode = ::GetLastError();
            sMessageText.Format(
                _T("0x%08X"),
                dwErrorCode
                );
        }
        if(m_bRebootNeed)
        {
            LPCTSTR pStrings[] = { _T("Драйвер фильтра и системная служба успешно удалены из системы, но небходимо перезагрузить компьютер.") };
            ReportEventInvoke(
                EVENTLOG_WARNING_TYPE,
                MSG_WARNING_EVENT_TEXT,
                1,
                pStrings
                );
            RestartPromptShedule();
        }
        else
        {
            LPCTSTR pStrings[] = { _T("Драйвер фильтра и системная служба успешно удалены из системы.") };
            ReportEventInvoke(
                EVENTLOG_INFORMATION_TYPE,
                MSG_INFORMATIONAL_EVENT_TEXT,
                1,
                pStrings
                );
        }
    }
	return FALSE;
}

int CNATTraversalUninstallerApp::ExitInstance()
{
    LONG    nResult;
    // удаление регистрации источника сообщений
    nResult = RegDeleteKey(
        HKEY_LOCAL_MACHINE,
        _T("SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\NATTraversalUninstaller")
        );
    CWinApp::ExitInstance();
	return m_nRetCode;
}

BEGIN_MESSAGE_MAP(CNATTraversalUninstallerApp, CWinApp)
END_MESSAGE_MAP()


// CNATTraversalUninstallerApp message handlers


void CNATTraversalUninstallerApp::ReportEventInvoke(
    WORD wType,
    DWORD dwEventID,
    WORD cInserts,
    LPCTSTR* szMsgArray
    )
{
    HANDLE hEventLog;
    hEventLog = RegisterEventSource(
        NULL,
        _T("NATTraversalUninstaller")
        );
    ReportEvent(
        hEventLog,
        wType,
        0,
        dwEventID,
        NULL,
        cInserts,
        0,
        szMsgArray,
        NULL
        );
    DeregisterEventSource(hEventLog);
}


void CNATTraversalUninstallerApp::RestartPromptShedule(void)
{
    CString sApplicationName, sSourceApplicationName;
    BOOL    bRet;
    TCHAR   szTempDir[MAX_PATH];
    DWORD   dwNameLength;
    STARTUPINFO         stStartupInfo;
    PROCESS_INFORMATION stProcessInformation;
    sSourceApplicationName  = m_sModuleFolder;
    sSourceApplicationName += _T("\\reboot_prompt.exe");
    dwNameLength = GetEnvironmentVariable(
        _T("TEMP"),
        szTempDir,
        MAX_PATH
        );
    ASSERT(dwNameLength > 0);
    sApplicationName  = szTempDir;
    sApplicationName += _T("\\reboot_prompt.exe");
    bRet = CopyFile(
        sSourceApplicationName,
        sApplicationName,
        FALSE
        );
    ASSERT(bRet);
    RtlZeroMemory(
        &stStartupInfo,
        sizeof(stStartupInfo)
        );
    stStartupInfo.cb = sizeof(stStartupInfo);
    RtlZeroMemory(
        &stProcessInformation,
        sizeof(stProcessInformation)
        );
    bRet = CreateProcess(
        sApplicationName,
        NULL,
        NULL,
        NULL,
        FALSE,
        0,
        NULL,
        NULL,
        &stStartupInfo,
        &stProcessInformation
        );
    ASSERT(bRet);
    bRet = CloseHandle(stProcessInformation.hProcess);
    bRet = CloseHandle(stProcessInformation.hThread);
}
