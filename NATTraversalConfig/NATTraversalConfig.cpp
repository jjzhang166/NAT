
// NATTraversalConfig.cpp : ���������� ��������� ������� ��� ����������.
//

#include "stdafx.h"
#include "NATTraversalConfig.h"
#include "NATTraversalConfigDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CNATTraversalConfigApp

BEGIN_MESSAGE_MAP(CNATTraversalConfigApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// �������� CNATTraversalConfigApp

CNATTraversalConfigApp::CNATTraversalConfigApp()
    : m_sModuleFileName(_T(""))
    , m_sVersionHTMLString(_T(""))
    , m_qwFileVersion(0)
    , m_pMainDialog(NULL)
    , m_sModuleFolder(_T(""))
{
	// ��������� ���������� ������������
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: �������� ��� ��������,
	// ��������� ���� ������ ��� ������������� � InitInstance
}


// ������������ ������ CNATTraversalConfigApp

CNATTraversalConfigApp theApp;


// ������������� CNATTraversalConfigApp

BOOL CNATTraversalConfigApp::InitInstance()
{
    TCHAR sModuleFileName[260];
    DWORD dwVerInfoLen = 0, dwVerHandle = 1;
    PVOID pVerInfoData;
    TCHAR sBS[2] = { _T('\\'), 0 };
    VS_FIXEDFILEINFO* pFileVerInfo = NULL;
    UINT  nFILen = 0;
    DWORD anFileVersion[4];
    int nCharPos;
	// InitCommonControlsEx() ��������� ��� Windows XP, ���� ��������
	// ���������� ���������� ComCtl32.dll ������ 6 ��� ����� ������� ������ ��� ���������
	// ������ �����������. � ��������� ������ ����� ��������� ���� ��� �������� ������ ����.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// �������� ���� �������� ��� ��������� ���� ����� ������� ����������, ������� ���������� ������������
	// � ����� ����������.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}


	AfxEnableControlContainer();

    _tsetlocale( LC_ALL, _T("Russian") );

	// ������� ��������� ��������, � ������, ���� ���������� ���� ��������
	// ������������� ������ �������� ��� �����-���� ��� �������� ����������.
	//CShellManager *pShellManager = new CShellManager;

	// ����������� �������������
	// ���� ��� ����������� �� ������������ � ���������� ��������� ������
	// ��������� ������������ �����, ���������� ������� �� ���������
	// ���������� �������� �������������, ������� �� ���������
	// �������� ������ �������, � ������� �������� ���������
	// TODO: ������� �������� ��� ������ �� ���-������ ����������,
	// �������� �� �������� �����������
	SetRegistryKey(_T("Real Time Systems DD"));

    if(GetModuleFileName(
        NULL,
        sModuleFileName,
        sizeof sModuleFileName / sizeof TCHAR
        ) == 0 )
    {
        return FALSE;
    }
    m_sModuleFileName = sModuleFileName;
    nCharPos = m_sModuleFileName.ReverseFind(_T('\\'));
    m_sModuleFolder = m_sModuleFileName.Left(nCharPos);
    dwVerInfoLen = GetFileVersionInfoSize(
        sModuleFileName,
        &dwVerHandle
        );
    if(dwVerInfoLen == 0) {
        return FALSE;
    }
    pVerInfoData = malloc(dwVerInfoLen);
    if(!GetFileVersionInfo(
        sModuleFileName,
        dwVerHandle,
        dwVerInfoLen,
        pVerInfoData
        ))
    {
        free(pVerInfoData);
        return FALSE;
    }
    if(!VerQueryValue(
        pVerInfoData,
        sBS,
        (LPVOID*)&pFileVerInfo,
        &nFILen
        ))
    {
        free(pVerInfoData);
        return FALSE;
    }
    m_qwFileVersion = pFileVerInfo->dwFileVersionMS;
    m_qwFileVersion = (m_qwFileVersion << 32) | pFileVerInfo->dwFileVersionLS;
    anFileVersion[0] = pFileVerInfo->dwFileVersionMS >> 16;
    anFileVersion[1] = pFileVerInfo->dwFileVersionMS & 0xFFFF;
    anFileVersion[2] = pFileVerInfo->dwFileVersionLS >> 16;
    anFileVersion[3] = pFileVerInfo->dwFileVersionLS & 0xFFFF;
    free(pVerInfoData);
    m_sVersionHTMLString.Format(
        _T("%u.%u.%.4u.%.4u &beta;"),
        anFileVersion[0],
        anFileVersion[1],
        anFileVersion[2],
        anFileVersion[3]
        );
	m_pMainDialog = new CNATTraversalConfigDlg( );
	m_pMainWnd = m_pMainDialog;
    if(!m_pMainDialog->Create(_T("NATTRAVERSALCONFIG_DIALOG"))) {
        return FALSE;
    }
	// The one and only window has been initialized, so show and update it
    BOOL bShowMaximized = GetProfileInt(
        _T("WindowPlacement"),
        _T("showMaximized"),
        FALSE
        );
    int left, top, right, bottom;
    CRect cNewRect;
    left    = GetProfileInt(
        _T("WindowPlacement"),
        _T("leftNormal"),
        47
        );
    top     = GetProfileInt(
        _T("WindowPlacement"),
        _T("topNormal"),
        21
        );
    right   = GetProfileInt(
        _T("WindowPlacement"),
        _T("rightNormal"),
        780
        );
    bottom  = GetProfileInt(
        _T("WindowPlacement"),
        _T("bottomNormal"),
        600
        );
    cNewRect.left   = left;
    cNewRect.top    = top;
    cNewRect.right  = right;
    cNewRect.bottom = bottom;
    m_pMainWnd->MoveWindow( cNewRect, TRUE );
    m_pMainWnd->ModifyStyleEx(
        WS_EX_CLIENTEDGE,
        0,
        0
        );
    m_nCmdShow = (bShowMaximized) ? SW_SHOWMAXIMIZED : SW_SHOW;
	m_pMainWnd->ShowWindow( m_nCmdShow );
	m_pMainWnd->UpdateWindow( );
    return TRUE;
}



int CNATTraversalConfigApp::ExitInstance()
{
    if(NULL != m_pMainDialog)
    {
        delete m_pMainDialog;
        m_pMainDialog = NULL;
        m_pMainWnd = NULL;
    }

    return CWinApp::ExitInstance();
}
