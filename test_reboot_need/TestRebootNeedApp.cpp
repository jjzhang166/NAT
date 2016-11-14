// TestRebootNeedApp.cpp : implementation file
//

#include "stdafx.h"
#include "TestRebootNeedApp.h"
#include "commrcfg.h"

CTestRebootNeedApp theApp;
// CTestRebootNeedApp

CTestRebootNeedApp::CTestRebootNeedApp()
: m_sModuleFileName(_T(""))
, m_sModuleFolder(_T(""))
{
}

CTestRebootNeedApp::~CTestRebootNeedApp()
{
}

BOOL CTestRebootNeedApp::InitInstance()
{
    BOOL    bRebootNeed;
    BOOL    bRet;
    TCHAR   szPath[MAX_PATH];
    DWORD   dwModuleNameLength;
    int     nCharPos;
	SetRegistryKey(_T("Real Time Systems DD"));
    if(NULL != m_pszProfileName)
    {
        free((void*)m_pszProfileName);
    }
    m_pszProfileName = _tcsdup(_T("rtsdd_parameters"));
    dwModuleNameLength = GetModuleFileName( NULL, szPath, MAX_PATH );
    m_sModuleFileName = szPath;
    nCharPos = m_sModuleFileName.ReverseFind(_T('\\'));
    m_sModuleFolder = m_sModuleFileName.Left(nCharPos);
    bRebootNeed = GetConfigInt(
        _T("rtsdd_setup"),
        _T("reboot_need"),
        FALSE
        );
    bRet = WriteConfigInt(
        _T("rtsdd_setup"),
        _T("reboot_need"),
        FALSE
        );
    if(bRebootNeed)
    {
        INT     nPromptRet;
        nPromptRet = SetupPromptReboot(
            NULL,
            NULL,
            FALSE
            );
    }
	return FALSE;
}

int CTestRebootNeedApp::ExitInstance()
{
	// TODO:  perform any per-thread cleanup here
	return CWinApp::ExitInstance();
}

BEGIN_MESSAGE_MAP(CTestRebootNeedApp, CWinApp)
END_MESSAGE_MAP()


// CTestRebootNeedApp message handlers
