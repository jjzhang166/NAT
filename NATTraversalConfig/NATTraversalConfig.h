
// NATTraversalConfig.h : ������� ���� ��������� ��� ���������� PROJECT_NAME
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�������� stdafx.h �� ��������� ����� ����� � PCH"
#endif

#include "resource.h"		// �������� �������


// CNATTraversalConfigApp:
// � ���������� ������� ������ ��. NATTraversalConfig.cpp
//

class CNATTraversalConfigDlg;
class CNATTraversalConfigApp : public CWinApp
{
public:
	CNATTraversalConfigApp();

// ���������������
public:
	virtual BOOL InitInstance();

// ����������

	DECLARE_MESSAGE_MAP()
public:
    CString m_sModuleFileName;
    CString m_sVersionHTMLString;
    ULONGLONG m_qwFileVersion;
    CNATTraversalConfigDlg* m_pMainDialog;
    CString m_sModuleFolder;
    virtual int ExitInstance();
};

extern CNATTraversalConfigApp theApp;