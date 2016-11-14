
#pragma once

// CRebootPrompt

class CRebootPrompt : public CWinApp
{
public:
	CRebootPrompt();
	virtual ~CRebootPrompt();

public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

protected:
	DECLARE_MESSAGE_MAP()
};

extern CRebootPrompt theApp;
