
// NATTraversalService.h : главный файл заголовка для приложения PROJECT_NAME
//

#pragma once

#ifndef __AFXWIN_H__
	#error "включить stdafx.h до включения этого файла в PCH"
#endif

#include "../include/nattraverse_public.h"
#include "resource.h"		// основные символы
#include "afxmt.h"

#define WDF_SECTION_NAME L"nattraverse_kmdffilter_Device.NT.Wdf"
#define SERVICE_SECTION_NAME _T("nattraverse_kmdffilter_Device.NT.Services")
#define DRIVER_SERVICE_NAME _T("nattraverse_kmdffilter")

typedef enum ENStartMode
{
    enAutomatic,
    enManual
};


// CNATTraversalServiceApp:
// О реализации данного класса см. NATTraversalService.cpp
//
class CMainWorkerThread;
class CNATTraversalServiceApp : public CWinApp
{
public:
	CNATTraversalServiceApp();

// Переопределение
public:
	virtual BOOL InitInstance();

// Реализация

	DECLARE_MESSAGE_MAP()
public:
    virtual int ExitInstance();
    int m_nRetCode;
    bool m_bParseError;
    bool m_bInstallService;
    bool m_bUninstallService;
    bool m_bUsageOut;
    ENStartMode m_enStartMode;
    static void ReportEventInvoke(
        WORD wType,
        DWORD dwEventID,
        WORD cInserts,
        LPCTSTR* szMsgArray
        );
    virtual int Run();
    static void WINAPI ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv);
    void mfServiceMain(DWORD dwArgc, LPTSTR* lpszArgv);
    static void WINAPI ServiceCtrlHandler(DWORD fdwControl);
    void mfServiceCtrlHandler(DWORD fdwControl);
    SERVICE_STATUS_HANDLE m_hServiceStatusHandle;
    SERVICE_STATUS m_stServiceStatus;
    DWORD ServiceInitialization(DWORD argc, LPTSTR* argv, DWORD* specificError);
    bool m_bInstallDriver;
    bool m_bUninstallDriver;
    CString m_sModuleFileName;
    CString m_sModuleFolder;
    CString m_sInfFilePath;
    bool m_bRebootNeed;
    bool m_bTestModeInstalled;
    bool m_bRootCertExist;
    bool m_bRootCertInstalled;
    bool m_bTrustedPublisherCertExist;
    bool m_bTrustedPublisherCertInstalled;
    CString m_sCoInstallerPath;
    CString m_sDriversFolder;
    HMODULE m_hCoInatallerModule;
    PFN_WDFPREDEVICEINSTALL m_pfnWdfPreDeviceInstall;
    PFN_WDFPOSTDEVICEINSTALL m_pfnWdfPostDeviceInstall;
    PFN_WDFPREDEVICEINSTALLEX m_pfnWdfPreDeviceInstallEx;
    PFN_WDFPREDEVICEREMOVE m_pfnWdfPreDeviceRemove;
    PFN_WDFPOSTDEVICEREMOVE m_pfnWdfPostDeviceRemove;
    CMainWorkerThread* m_pWorkerThread;
    CCriticalSection m_oWorkerThreadLock;
    CEvent m_oInitWorkerThreadDone;
    DWORD m_dwCheckPoint;
    CString m_sFilterDriverFolder;
};

extern CNATTraversalServiceApp theApp;
