#pragma once

#define WDF_SECTION_NAME L"nattraverse_kmdffilter_Device.NT.Wdf"
#define SERVICE_SECTION_NAME _T("nattraverse_kmdffilter_Device.NT.Services")
#define DRIVER_SERVICE_NAME _T("nattraverse_kmdffilter")

// CNATTraversalUninstallerApp

class CNATTraversalUninstallerApp : public CWinApp
{
public:
	CNATTraversalUninstallerApp();
	virtual ~CNATTraversalUninstallerApp();

public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

protected:
	DECLARE_MESSAGE_MAP()
public:
    int m_nRetCode;
    CString m_sModuleFileName;
    CString m_sModuleFolder;
    static void ReportEventInvoke(
        WORD wType,
        DWORD dwEventID,
        WORD cInserts,
        LPCTSTR* szMsgArray
        );
    CString m_sInfFilePath;
    CString m_sFilterDriverFolder;
    CString m_sCoInstallerPath;
    CString m_sDriversFolder;
    HMODULE m_hCoInatallerModule;
    PFN_WDFPREDEVICEINSTALL m_pfnWdfPreDeviceInstall;
    PFN_WDFPOSTDEVICEINSTALL m_pfnWdfPostDeviceInstall;
    PFN_WDFPREDEVICEINSTALLEX m_pfnWdfPreDeviceInstallEx;
    PFN_WDFPREDEVICEREMOVE m_pfnWdfPreDeviceRemove;
    PFN_WDFPOSTDEVICEREMOVE m_pfnWdfPostDeviceRemove;
    bool m_bRebootNeed;
    void RestartPromptShedule(void);
};

extern CNATTraversalUninstallerApp theApp;
