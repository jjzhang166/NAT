#pragma once

// CStartProcCmdLineInfo command target

class CStartProcCmdLineInfo : public CCommandLineInfo
{
public:
	CStartProcCmdLineInfo();
	virtual ~CStartProcCmdLineInfo();
    virtual void ParseParam(
        const TCHAR* pszParam,
        BOOL bFlag,
        BOOL bLast
        );
};
