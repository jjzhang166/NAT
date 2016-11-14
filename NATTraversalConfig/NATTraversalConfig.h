
// NATTraversalConfig.h : главный файл заголовка для приложения PROJECT_NAME
//

#pragma once

#ifndef __AFXWIN_H__
	#error "включить stdafx.h до включения этого файла в PCH"
#endif

#include "resource.h"		// основные символы


// CNATTraversalConfigApp:
// О реализации данного класса см. NATTraversalConfig.cpp
//

class CNATTraversalConfigDlg;
class CNATTraversalConfigApp : public CWinApp
{
public:
	CNATTraversalConfigApp();

// Переопределение
public:
	virtual BOOL InitInstance();

// Реализация

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