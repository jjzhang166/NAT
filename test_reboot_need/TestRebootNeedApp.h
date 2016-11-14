#pragma once



// CTestRebootNeedApp

class CTestRebootNeedApp : public CWinApp
{
public:
	CTestRebootNeedApp();
	virtual ~CTestRebootNeedApp();

public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

protected:
	DECLARE_MESSAGE_MAP()
public:
    CString m_sModuleFileName;
    CString m_sModuleFolder;
};

extern CTestRebootNeedApp theApp;
