// RebootPrompt.cpp : implementation file
//

#include "stdafx.h"
#include "RebootPrompt.h"

CRebootPrompt theApp;

// CRebootPrompt

CRebootPrompt::CRebootPrompt()
{
}

CRebootPrompt::~CRebootPrompt()
{
}

BOOL CRebootPrompt::InitInstance()
{
    INT     nPromptRet;
    nPromptRet = SetupPromptReboot(
        NULL,
        NULL,
        FALSE
        );
	return FALSE;
}

int CRebootPrompt::ExitInstance()
{
	// TODO:  perform any per-thread cleanup here
	return CWinApp::ExitInstance();
}

BEGIN_MESSAGE_MAP(CRebootPrompt, CWinApp)
END_MESSAGE_MAP()


// CRebootPrompt message handlers
