
// stdafx.h: включите файл для добавления стандартных системных файлов
//или конкретных файлов проектов, часто используемых,
// но редко изменяемых

#pragma once

#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Исключите редко используемые компоненты из заголовков Windows
#endif

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // некоторые конструкторы CString будут явными

// отключает функцию скрытия некоторых общих и часто пропускаемых предупреждений MFC
#define _AFX_ALL_WARNINGS

#include <TCHAR.H>
#include <locale.h>

#include <afxwin.h>         // основные и стандартные компоненты MFC
#include <afxext.h>         // расширения MFC


#include <afxdisp.h>        // классы автоматизации MFC



#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // поддержка MFC для типовых элементов управления Internet Explorer 4
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // поддержка MFC для типовых элементов управления Windows
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // поддержка MFC для лент и панелей управления


#include <afxsock.h>            // расширения сокетов MFC
#include <afxdhtml.h>           // Диалоговые окна HTML
#include <afxdialogex.h>

#define STATUS_SUCCESS                          ((NTSTATUS)0x00000000L) // ntsubauth
#include <comutil.h>
#include <commdlg.h>
#include <olectl.h>
#include <iphlpapi.h>
#include <mstcpip.h>
#include <objbase.h>
#include <winsvc.h>

#pragma warning( disable : 4192 )
#ifdef _M_X64
#   import "I:\Windows\System32\mshtml.tlb" rename("TranslateAccelerator", "TranslateAcceleratorMSHTML") \
	rename("min", "minMSHTML") rename("max", "maxMSHTML") rename("operator", "operatorMSHTML")
#else
#   import "I:\Windows\SysWOW64\mshtml.tlb" rename("TranslateAccelerator", "TranslateAcceleratorMSHTML") \
	rename("min", "minMSHTML") rename("max", "maxMSHTML") rename("operator", "operatorMSHTML")
#endif

#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif
