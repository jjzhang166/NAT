
#pragma once


UINT GetConfigInt(LPCTSTR lpszSection, LPCTSTR lpszEntry,
	int nDefault);

CString GetConfigString(LPCTSTR lpszSection, LPCTSTR lpszEntry,
	LPCTSTR lpszDefault);

BOOL GetConfigBinary( LPCTSTR lpszSection, LPCTSTR lpszEntry,
	BYTE** ppData, UINT* pBytes );

BOOL WriteConfigInt(LPCTSTR lpszSection, LPCTSTR lpszEntry,
	int nValue);

BOOL WriteConfigString(LPCTSTR lpszSection, LPCTSTR lpszEntry,
			LPCTSTR lpszValue);

BOOL WriteConfigBinary(LPCTSTR lpszSection, LPCTSTR lpszEntry,
	LPBYTE pData, UINT nBytes);
