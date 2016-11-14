// StartProcCmdLineInfo.cpp : implementation file
//

#include "stdafx.h"
#include "NATTraversalService.h"
#include "StartProcCmdLineInfo.h"


// CStartProcCmdLineInfo

CStartProcCmdLineInfo::CStartProcCmdLineInfo()
{
}

CStartProcCmdLineInfo::~CStartProcCmdLineInfo()
{
}

// CStartProcCmdLineInfo member functions

void CStartProcCmdLineInfo::ParseParam(
    const TCHAR* pszParam,
    BOOL bFlag,
    BOOL bLast
    )
{
    CString sParam(pszParam);
    UNREFERENCED_PARAMETER(bLast);
    if(bFlag&&(sParam == _T("install")))
    {
        theApp.m_bInstallService = true;
    }
    else if(bFlag&&(sParam == _T("uninstall")))
    {
        theApp.m_bUninstallService = true;
    }
    else if(bFlag&&(sParam.Find(_T("start_mode=")) == 0))
    {
        int nPos = sParam.Find(_T('=')) + 1;
        sParam = sParam.Mid(nPos);
        if((sParam != _T("automatic"))&&(sParam != _T("manual")))
        {
            theApp.m_bParseError = true;
            return;
        }
        if(sParam == _T("automatic"))
        {
            theApp.m_enStartMode = enAutomatic;
        }
        else
        {
            theApp.m_enStartMode = enManual;
        }
    }
    else if(bFlag&&(sParam == _T("instdriver")))
    {
        theApp.m_bInstallDriver = true;
    }
    else if(bFlag&&(sParam == _T("uninstdriver")))
    {
        theApp.m_bUninstallDriver = true;
    }
    else if(bFlag&&((sParam == _T("h")) || (sParam == _T("?"))))
    {
        theApp.m_bUsageOut = true;
    }
    else
    {
        theApp.m_bParseError = true;
        return;
    }
    if(((theApp.m_bInstallService || theApp.m_bInstallDriver)
        && (theApp.m_bUninstallService || theApp.m_bUninstallDriver))
        || (theApp.m_bInstallService && theApp.m_bInstallDriver))
    {
        theApp.m_bParseError = true;
    }
}
