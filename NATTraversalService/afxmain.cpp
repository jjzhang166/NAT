/********************************************************************\
| main( ) function for MFC Console Applications                      |
|   Ðàçðàáîò÷èê Äþêîâ Ñ. Í. \   ÍÍÖ  ÕÔÒÈ    \   1996 - 2004 ãã.     |
\********************************************************************/
#include "StdAfx.h"

int __cdecl _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
    HINSTANCE hInstance     = HINSTANCE(GetModuleHandle( NULL ));
    HINSTANCE hPrevInstance = NULL;
    LPTSTR  lpCmdLine = ::GetCommandLine( );
    int     nCmdShow = 0;
	int nReturnCode = -1;
	CWinThread* pThread = AfxGetThread();
	CWinApp* pApp = AfxGetApp();
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);
    UNREFERENCED_PARAMETER(envp);

	// AFX internal initialization
	if (!AfxWinInit(hInstance, hPrevInstance, lpCmdLine, nCmdShow))
		goto InitFailure;

	// App global initializations (rare)
	if (pApp != NULL && !pApp->InitApplication())
		goto InitFailure;

	// Perform specific initializations
	if (!pThread->InitInstance())
	{
		if (pThread->m_pMainWnd != NULL)
		{
			TRACE(traceAppMsg, 0, "Warning: Destroying non-NULL m_pMainWnd\n");
			pThread->m_pMainWnd->DestroyWindow();
		}
		nReturnCode = pThread->ExitInstance();
		goto InitFailure;
	}
	nReturnCode = pThread->Run();

InitFailure:
#ifdef _DEBUG
	// Check for missing AfxLockTempMap calls
	if (AfxGetModuleThreadState()->m_nTempMapLock != 0)
	{
		TRACE(traceAppMsg, 0, "Warning: Temp map lock count non-zero (%ld).\n",
			AfxGetModuleThreadState()->m_nTempMapLock);
	}
	AfxLockTempMaps();
	AfxUnlockTempMaps(-1);
#endif

	AfxWinTerm();
	return nReturnCode;
}
