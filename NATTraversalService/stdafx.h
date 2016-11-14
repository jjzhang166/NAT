
#pragma once

#include <tchar.h>
#include <locale.h>
#include <stdio.h>
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit
#define _AFX_NO_MFC_CONTROLS_IN_DIALOGS         // remove support for MFC controls in dialogs

#include <afx.h>
#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>       // MFC support for Internet Explorer 4 Common Controls
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>                     // MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT
#include <afxsock.h>        // расширения сокетов MFC
#include <afxmt.h>
#include <afxtempl.h>

#define STATUS_SUCCESS                          ((NTSTATUS)0x00000000L) // ntsubauth
#define INITGUID
#include <guiddef.h>
#include <Fwpmu.h>
#include <comutil.h>
#include "ntddkaddon.h"
#include <netioapi.h>
#include <mstcpip.h>
#include <SetupAPI.h>
#include <DriverSpecs.h>
_Analysis_mode_(_Analysis_code_type_user_code_)
#include <wdfinstaller.h>
#include <newdev.h>
#include <cfgmgr32.h>

#ifdef _M_X64
#   import "I:\Windows\System32\wbem\wbemdisp.tlb" no_namespace
#else
#   import "I:\Windows\SysWOW64\wbem\wbemdisp.tlb" no_namespace
#endif

#ifdef _M_X64
#   import "x64\bcdsrv.dll" no_namespace
#else
#   import "x86\bcdsrv.dll" no_namespace
#endif
