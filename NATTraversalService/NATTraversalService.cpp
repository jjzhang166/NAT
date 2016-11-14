
// NATTraversalService.cpp : Определяет поведение классов для приложения.
//

#include "stdafx.h"
#include "NATTraversalService.h"
#include "MainWorkerThread.h"
#include "StartProcCmdLineInfo.h"
#include "commrcfg.h"
#include "MessageTable.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CNATTraversalServiceApp

BEGIN_MESSAGE_MAP(CNATTraversalServiceApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// создание CNATTraversalServiceApp

CNATTraversalServiceApp::CNATTraversalServiceApp()
: m_nRetCode(0)
, m_bParseError(false)
, m_bInstallService(false)
, m_bUninstallService(false)
, m_bUsageOut(false)
, m_bInstallDriver(false)
, m_bUninstallDriver(false)
, m_sModuleFileName(_T(""))
, m_sModuleFolder(_T(""))
, m_sInfFilePath(_T(""))
, m_bRebootNeed(false)
, m_bTestModeInstalled(false)
, m_bRootCertExist(false)
, m_bRootCertInstalled(false)
, m_bTrustedPublisherCertExist(false)
, m_bTrustedPublisherCertInstalled(false)
, m_sCoInstallerPath(_T(""))
, m_sDriversFolder(_T(""))
, m_pWorkerThread(NULL)
, m_dwCheckPoint(0)
, m_sFilterDriverFolder(_T(""))
{
	// TODO: добавьте код создания,
	// Размещает весь важный код инициализации в InitInstance
}


// Единственный объект CNATTraversalServiceApp

CNATTraversalServiceApp theApp;

const TCHAR c_sUsageStr[] = _T(
"\
Синтаксис командной строки:\n\
    NATTraversalService -h    >\n\
                              > получение справки\n\
    NATTraversalService -?    >\n\
\n\
    NATTraversalService -uninstall         > деинсталяция сервиса\n\
\n\
    NATTraversalService -install [ опции ] > инсталяция сервиса\n\
\n\
опции:\n\
    -start_mode=automatic|manual    -- режим запуска сервиса\n\
                                    -- (по-умолчанию = automatic)\n\
\n\
    NATTraversalService -instdriver        > инсталляция драйвера   \"nattraverse_kmdffilter\"\n\
\n\
    NATTraversalService -uninstdriver      > деинсталляция драйвера \"nattraverse_kmdffilter\"\n\
");
const TCHAR c_sRebootNeedStr[]  = _T(
"\
*******************************************************************\n\
* Драйвер \"nattraverse_kmdffilter\" успешно установлен в систему,*\n\
* но для его функционирования необходимо перезагрузить компьютер. *\n\
*******************************************************************\n\
");
const TCHAR c_sUninstallRebootStr[]  = _T(
"\
*****************************************************************\n\
* Драйвер \"nattraverse_kmdffilter\" успешно удалён из системы, *\n\
* но необходимо перезагрузить компьютер.                        *\n\
*****************************************************************\n\
");

const SERVICE_DESCRIPTION c_stServiceDescription =
{
    _T("Служба обхода NAT")
};


// инициализация CNATTraversalServiceApp
#pragma warning( disable : 4238 )
BOOL CNATTraversalServiceApp::InitInstance()
{
    CStartProcCmdLineInfo oCmdLineInfo;
    TCHAR                 szPath[MAX_PATH];
    DWORD                 dwModuleNameLength;
    int                   nCharPos;
    BOOL                  bRet;
    DWORD                 dwErrorCode;
    INT                   nPromptRet;
    _tsetlocale( LC_ALL, _T("Russian") );
	CWinApp::InitInstance();
	// Стандартная инициализация
	// Если эти возможности не используются и необходимо уменьшить размер
	// конечного исполняемого файла, необходимо удалить из следующих
	// конкретных процедур инициализации, которые не требуются
	// Измените раздел реестра, в котором хранятся параметры
	// TODO: следует изменить эту строку на что-нибудь подходящее,
	// например на название организации
	SetRegistryKey(_T("Real Time Systems DD"));
    if(m_pszProfileName != NULL)
        free((void*)m_pszProfileName);
    m_pszProfileName = _tcsdup(_T("NATTraversalConfig"));
    ParseCommandLine(oCmdLineInfo);
    if(m_bParseError) {
        m_nRetCode = 1;
        m_bUsageOut = true;
        _tprintf(
            _T("Ошибка синтаксиса командной строки.\n")
            );
    }
    if(m_bUsageOut)
    {
        _tprintf(c_sUsageStr);
        return FALSE;
    }
    dwModuleNameLength = GetModuleFileName( NULL, szPath, MAX_PATH );
    m_sModuleFileName = szPath;
    nCharPos = m_sModuleFileName.ReverseFind(_T('\\'));
    m_sModuleFolder = m_sModuleFileName.Left(nCharPos);
    m_sFilterDriverFolder = m_sModuleFolder + _T("\\FilterDriver");
    m_sInfFilePath = m_sFilterDriverFolder + _T("\\nattraverse_kmdffilter.inf");
    if(m_bInstallDriver)
    {
        // инсталяция драйвера
        HRESULT hr;
        hr = CoInitialize(NULL);
        {
            // Проверка и установка BCD-элемента "TESTSIGNING" в текущий загрузчик
            bool                    bReturnValue;
            bool                    bTestMode;
            ISWbemLocatorPtr        spLoc;
            ISWbemServicesPtr       spSvc;
            ISWbemObjectPtr         spBcdStoreClass;
            ISWbemMethodSetPtr      spBcdStoreClassMethods;
            ISWbemMethodSetPtr      spObjectMethods;
            ISWbemMethodPtr         spMethodObject;
            ISWbemObjectPtr         spInParamsObject;
            ISWbemPropertySetPtr    spInParams;
            ISWbemPropertyPtr       spParameter;
            ISWbemObjectPtr         spOutParamsObject;
            ISWbemPropertySetPtr    spOutParams;
            ISWbemObjectPtr         spBcdStoreObject;
            ISWbemObjectPtr         spCurrentLoader;
            ISWbemSecurityPtr       spServiceSecurity;
            ISWbemObjectPtr         spElemenObject;
            ISWbemPropertySetPtr    spElementProperties;
            ISWbemPropertyPtr       spElementProperty;
            _variant_t              vElementProperty;
            hr = spLoc.CreateInstance(
                __uuidof(SWbemLocator)
                );
            ASSERT(SUCCEEDED(hr));
            // Connect to the root\wmi namespace with the current user.
            spSvc = spLoc->ConnectServer(
                _bstr_t( ),
                L"root\\wmi",
                _bstr_t( ),
                _bstr_t( ),
                _bstr_t( ),
                _bstr_t( ),
                0,
                (IDispatch*)NULL
                );
            spServiceSecurity = spSvc->Security_;
            spServiceSecurity->ImpersonationLevel = wbemImpersonationLevelImpersonate;
            spBcdStoreClass = spSvc->Get(
                L"BcdStore",
                0,
                (IDispatch*)NULL
                );
            spBcdStoreClassMethods = spBcdStoreClass->Methods_;
            spMethodObject = spBcdStoreClassMethods->Item(
                L"OpenStore",
                0
                );
            spInParamsObject = spMethodObject->InParameters->SpawnInstance_(
                0
                );
            spInParams = spInParamsObject->Properties_;
            spParameter = spInParams->Item(
                L"File",
                0
                );
            spParameter->PutValue(&_variant_t(L""));
            spOutParamsObject = spBcdStoreClass->ExecMethod_(
                L"OpenStore",
                spInParamsObject,
                0,
                NULL
                );
            spOutParams = spOutParamsObject->Properties_;
            spParameter = spOutParams->Item(
                L"ReturnValue",
                0
                );
            bReturnValue = spParameter->GetValue();
            ASSERT(bReturnValue);
            spParameter = spOutParams->Item(
                L"Store",
                0
                );
            spBcdStoreObject = spParameter->GetValue();
            spMethodObject = spBcdStoreClassMethods->Item(
                L"OpenObject",
                0
                );
            spInParamsObject = spMethodObject->InParameters->SpawnInstance_(
                0
                );
            spInParams = spInParamsObject->Properties_;
            spParameter = spInParams->Item(
                L"Id",
                0
                );
            spParameter->PutValue(&_variant_t(L"{fa926493-6f1c-4193-a414-58f0b2456d1e}"));
            spOutParamsObject = spBcdStoreObject->ExecMethod_(
                L"OpenObject",
                spInParamsObject,
                0,
                NULL
                );
            spOutParams = spOutParamsObject->Properties_;
            spParameter = spOutParams->Item(
                L"ReturnValue",
                0
                );
            bReturnValue = spParameter->GetValue();
            ASSERT(bReturnValue);
            spParameter = spOutParams->Item(
                L"Object",
                0
                );
            spCurrentLoader = spParameter->GetValue();
            spObjectMethods = spCurrentLoader->Methods_;
            spMethodObject = spObjectMethods->Item(
                L"GetElement",
                0
                );
            spInParamsObject = spMethodObject->InParameters->SpawnInstance_(
                0
                );
            spInParams = spInParamsObject->Properties_;
            spParameter = spInParams->Item(
                L"Type",
                0
                );
            try
            {
                spParameter->PutValue(&_variant_t((DWORD)BcdLibraryBoolean_AllowPrereleaseSignatures));
                spOutParamsObject = spCurrentLoader->ExecMethod_(
                    L"GetElement",
                    spInParamsObject,
                    0,
                    NULL
                    );
                spOutParams = spOutParamsObject->Properties_;
                spParameter = spOutParams->Item(
                    L"ReturnValue",
                    0
                    );
                bReturnValue = spParameter->GetValue();
                if(!bReturnValue)
                {
                    bTestMode = false;
                }
                else
                {
                    spParameter = spOutParams->Item(
                        L"Element",
                        0
                        );
                    spElemenObject = spParameter->GetValue();
                    spElementProperties = spElemenObject->Properties_;
                    spElementProperty = spElementProperties->Item(
                        L"Boolean",
                        0
                        );
                    vElementProperty = spElementProperty->GetValue();
                    bTestMode = vElementProperty;
                }
            }
            catch(...)
            {
                bTestMode = false;
            }
            if(!bTestMode)
            {
                spMethodObject = spObjectMethods->Item(
                    L"SetBooleanElement",
                    0
                    );
                spInParamsObject = spMethodObject->InParameters->SpawnInstance_(
                    0
                    );
                spInParams = spInParamsObject->Properties_;
                spParameter = spInParams->Item(
                    L"Type",
                    0
                    );
                spParameter->PutValue(&_variant_t((DWORD)BcdLibraryBoolean_AllowPrereleaseSignatures));
                spParameter = spInParams->Item(
                    L"Boolean",
                    0
                    );
                spParameter->PutValue(&_variant_t((bool)true));
                spOutParamsObject = spCurrentLoader->ExecMethod_(
                    L"SetBooleanElement",
                    spInParamsObject,
                    0,
                    NULL
                    );
                spOutParams = spOutParamsObject->Properties_;
                spParameter = spOutParams->Item(
                    L"ReturnValue",
                    0
                    );
                bReturnValue = spParameter->GetValue();
                ASSERT(bReturnValue);
                m_bRebootNeed = true;
            }
            m_bTestModeInstalled = true;
        }
        {
            // установка тестового сертификата в хранилище корневых сертификатов и в хранилище сертификатов доверенных издателей
            HCERTSTORE      hSource;
            HCERTSTORE      hDestination;
            CString         sSourceFileName;
            PCCERT_CONTEXT  pSourceContext;
            bool            bCertExist = false;
            sSourceFileName  = m_sFilterDriverFolder;
            sSourceFileName += _T("\\rtsddWDKTestCert.cer");
            hSource = CertOpenStore(
                CERT_STORE_PROV_FILENAME,
                PKCS_7_ASN_ENCODING,
                NULL,
                CERT_STORE_OPEN_EXISTING_FLAG,
                (LPCTSTR)sSourceFileName
                );
            ASSERT(NULL != hSource);
            pSourceContext = CertEnumCertificatesInStore(
                hSource,
                NULL
                );
            ASSERT(NULL != pSourceContext);
            hDestination = CertOpenStore(
                CERT_STORE_PROV_SYSTEM,
                PKCS_7_ASN_ENCODING,
                NULL,
                CERT_SYSTEM_STORE_LOCAL_MACHINE,
                _T("root")
                );
            ASSERT(NULL != hDestination);
            bRet = CertAddCertificateContextToStore(
                hDestination,
                pSourceContext,
                CERT_STORE_ADD_NEW,
                NULL
                );
            if(!bRet)
            {
                dwErrorCode = ::GetLastError();
                ASSERT(CRYPT_E_EXISTS == dwErrorCode);
                bRet = CertAddCertificateContextToStore(
                    hDestination,
                    pSourceContext,
                    CERT_STORE_ADD_NEWER,
                    NULL
                    );
                bCertExist = true;
            }
            if(bCertExist &&(!bRet))
            {
                dwErrorCode = ::GetLastError();
                ASSERT(CRYPT_E_EXISTS == dwErrorCode);
            }
            if(bCertExist)
            {
                m_bRootCertExist = true;
            }
            else
            {
                m_bRootCertInstalled = true;
            }
            bRet = CertCloseStore(
                hDestination,
                0
                );
            bCertExist = false;
            hDestination = CertOpenStore(
                CERT_STORE_PROV_SYSTEM,
                PKCS_7_ASN_ENCODING,
                NULL,
                CERT_SYSTEM_STORE_LOCAL_MACHINE,
                _T("trustedpublisher")
                );
            ASSERT(NULL != hDestination);
            bRet = CertAddCertificateContextToStore(
                hDestination,
                pSourceContext,
                CERT_STORE_ADD_NEW,
                NULL
                );
            if(!bRet)
            {
                dwErrorCode = ::GetLastError();
                ASSERT(CRYPT_E_EXISTS == dwErrorCode);
                bRet = CertAddCertificateContextToStore(
                    hDestination,
                    pSourceContext,
                    CERT_STORE_ADD_NEWER,
                    NULL
                    );
                bCertExist = true;
            }
            if(bCertExist &&(!bRet))
            {
                dwErrorCode = ::GetLastError();
                ASSERT(CRYPT_E_EXISTS == dwErrorCode);
            }
            if(bCertExist)
            {
                m_bTrustedPublisherCertExist = true;
            }
            else
            {
                m_bTrustedPublisherCertInstalled = true;
            }
            bRet = CertCloseStore(
                hDestination,
                0
                );
            bRet = CertFreeCertificateContext(
                pSourceContext
                );
            pSourceContext = NULL;
            bRet = CertCloseStore(
                hSource,
                0
                );
        }
        {
            // установка драйвера с помощью setup-API и WDF-API
            WDF_COINSTALLER_INSTALL_OPTIONS stClienOptions;
            HINF                    hInfHandle;
            UINT                    nErrorLine, nPathLength;
            INFCONTEXT              stInfContext;
            TCHAR                   szTextBuffer[MAX_PATH];
            DWORD                   dwRequiredSize;
            CString sTextBuffer, sMajorVer, sMinorVer;
            int                     nPos;
            long    nMajorVer, nMinorVer;
            ULONG                   ulRetCode;
            HSPFILEQ                hFileQueue;
            PVOID                   pDefaultCBContext;
            DWORD                   dwInstallRet;
            BOOL                    bNeedReboot = FALSE;
            hInfHandle = SetupOpenInfFile(
                m_sInfFilePath,
                NULL,
                INF_STYLE_WIN4,
                &nErrorLine
                );
            if(INVALID_HANDLE_VALUE == hInfHandle)
            {
                DWORD   dwRetCount;
                DWORD   nRet;
                LPVOID  lpMsgBuf;
                nRet = ::GetLastError();
                dwRetCount = FormatMessage(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                    NULL,
                    nRet,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    (LPTSTR) &lpMsgBuf,
                    0,
                    NULL
                    );
                _tprintf(
                    _T("Ошибка открытия INF-файла!!\n\tКод ошибки = %d (0x%08X): %s\n\tНомер строки INF-файла = %d\n"),
                    nRet,
                    nRet,
                    lpMsgBuf,
                    nErrorLine
                    );
                LocalFree(lpMsgBuf);
			    m_nRetCode = nRet;
                return FALSE;
            }
            bRet = SetupFindFirstLine(
                hInfHandle,
                _T("nattraverse_kmdffilter_wdfsect"),
                _T("KmdfLibraryVersion"),
                &stInfContext
                );
            if(!bRet)
            {
                DWORD   dwRetCount;
                DWORD   nRet;
                LPVOID  lpMsgBuf;
                nRet = ::GetLastError();
                dwRetCount = FormatMessage(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                    NULL,
                    nRet,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    (LPTSTR) &lpMsgBuf,
                    0,
                    NULL
                    );
                _tprintf(
                    _T("Ошибка поиска строки значения \"KmdfLibraryVersion\"!!\n\tКод ошибки = %d (0x%08X): %s\n"),
                    nRet,
                    nRet,
                    lpMsgBuf
                    );
                LocalFree(lpMsgBuf);
                SetupCloseInfFile(hInfHandle);
			    m_nRetCode = nRet;
                return FALSE;
            }
            bRet = SetupGetStringField(
                &stInfContext,
                1,
                szTextBuffer,
                MAX_PATH,
                &dwRequiredSize
                );
            if(!bRet)
            {
                DWORD   dwRetCount;
                DWORD   nRet;
                LPVOID  lpMsgBuf;
                nRet = ::GetLastError();
                dwRetCount = FormatMessage(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                    NULL,
                    nRet,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    (LPTSTR) &lpMsgBuf,
                    0,
                    NULL
                    );
                _tprintf(
                    _T("Ошибка получения строки значения \"KmdfLibraryVersion\"!!\n\tКод ошибки = %d (0x%08X): %s\n"),
                    nRet,
                    nRet,
                    lpMsgBuf
                    );
                LocalFree(lpMsgBuf);
                SetupCloseInfFile(hInfHandle);
			    m_nRetCode = nRet;
                return FALSE;
            }
            sTextBuffer = szTextBuffer;
            nPos = sTextBuffer.Find(_T('.'));
            sMajorVer = sTextBuffer.Left(nPos);
            sMinorVer = sTextBuffer.Mid(nPos + 1);
            nMajorVer = _tstol(sMajorVer);
            nMinorVer = _tstol(sMinorVer);
            sTextBuffer.Format(
                _T("%02d%03d"),
                nMajorVer,
                nMinorVer
                );
            m_sCoInstallerPath  = m_sFilterDriverFolder;
            m_sCoInstallerPath += _T("\\WdfCoInstaller");
            m_sCoInstallerPath += sTextBuffer;
            m_sCoInstallerPath += _T(".dll");
            // прединсталляция драйвера в хранилище драйверов
            bRet = DiInstallDriver(
                NULL,
                m_sInfFilePath,
                DIIRFLAG_FORCE_INF, // DIIRFLAG_FORCE_INF
                &bNeedReboot
                );
            if(!bRet)
            {
                DWORD   dwRetCount;
                DWORD   nRet;
                LPVOID  lpMsgBuf;
                nRet = ::GetLastError();
                dwRetCount = FormatMessage(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                    NULL,
                    nRet,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    (LPTSTR) &lpMsgBuf,
                    0,
                    NULL
                    );
                _tprintf(
                    _T("Ошибка вызова функции \"DiInstallDriver\"!!\n\tКод ошибки = %d (0x%08X): %s\n"),
                    nRet,
                    nRet,
                    lpMsgBuf
                    );
                LocalFree(lpMsgBuf);
                if(!bNeedReboot)
                {
			        m_nRetCode = nRet;
                    return FALSE;
                }
            }
            m_bRebootNeed = m_bRebootNeed || bNeedReboot;
            nPathLength = GetWindowsDirectory(
                szTextBuffer,
                MAX_PATH
                );
            ASSERT(nPathLength > 1);
            m_sDriversFolder  = szTextBuffer;
            m_sDriversFolder += _T("\\System32\\drivers");
            hFileQueue = SetupOpenFileQueue();
            if(INVALID_HANDLE_VALUE == hFileQueue)
            {
                DWORD   dwRetCount;
                DWORD   nRet;
                LPVOID  lpMsgBuf;
                nRet = ::GetLastError();
                dwRetCount = FormatMessage(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                    NULL,
                    nRet,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    (LPTSTR) &lpMsgBuf,
                    0,
                    NULL
                    );
                _tprintf(
                    _T("Ошибка открытия очереди устанавливаемых файлов!!\n\tКод ошибки = %d (0x%08X): %s\n"),
                    nRet,
                    nRet,
                    lpMsgBuf
                    );
                LocalFree(lpMsgBuf);
                SetupCloseInfFile(hInfHandle);
			    m_nRetCode = nRet;
                return FALSE;
            }
            bRet = SetupQueueDefaultCopy(
                hFileQueue,
                hInfHandle,
                m_sFilterDriverFolder,
                _T("nattraverse_kmdffilter.sys"),
                _T("nattraverse_kmdffilter.sys"),
                SP_COPY_SOURCEPATH_ABSOLUTE
                );
            if(!bRet)
            {
                DWORD   dwRetCount;
                DWORD   nRet;
                LPVOID  lpMsgBuf;
                nRet = ::GetLastError();
                dwRetCount = FormatMessage(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                    NULL,
                    nRet,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    (LPTSTR) &lpMsgBuf,
                    0,
                    NULL
                    );
                _tprintf(
                    _T("Ошибка копирования файла драйвера!!\n\tКод ошибки = %d (0x%08X): %s\n"),
                    nRet,
                    nRet,
                    lpMsgBuf
                    );
                LocalFree(lpMsgBuf);
                SetupCloseFileQueue(hFileQueue);
                SetupCloseInfFile(hInfHandle);
			    m_nRetCode = nRet;
                return FALSE;
            }
            pDefaultCBContext = SetupInitDefaultQueueCallback(NULL);
            ASSERT(NULL != pDefaultCBContext);
            bRet = SetupCommitFileQueue(
                NULL,
                hFileQueue,
                SetupDefaultQueueCallback,
                pDefaultCBContext
                );
            if(!bRet)
            {
                DWORD   dwRetCount;
                DWORD   nRet;
                LPVOID  lpMsgBuf;
                nRet = ::GetLastError();
                SetupTermDefaultQueueCallback(pDefaultCBContext);
                dwRetCount = FormatMessage(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                    NULL,
                    nRet,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    (LPTSTR) &lpMsgBuf,
                    0,
                    NULL
                    );
                _tprintf(
                    _T("Ошибка подтверждения очереди файлов!!\n\tКод ошибки = %d (0x%08X): %s\n"),
                    nRet,
                    nRet,
                    lpMsgBuf
                    );
                LocalFree(lpMsgBuf);
                nPromptRet = SetupPromptReboot(
                    hFileQueue,
                    NULL,
                    FALSE
                    );
                SetupCloseFileQueue(hFileQueue);
                SetupCloseInfFile(hInfHandle);
			    m_nRetCode = nRet;
                return FALSE;
            }
            SetupTermDefaultQueueCallback(pDefaultCBContext);
            SetupCloseFileQueue(hFileQueue);
            // загрузка КоИнсталлера
            m_hCoInatallerModule = LoadLibrary(m_sCoInstallerPath);
            if(NULL == m_hCoInatallerModule)
            {
                DWORD   dwRetCount;
                DWORD   nRet;
                LPVOID  lpMsgBuf;
                nRet = ::GetLastError();
                dwRetCount = FormatMessage(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                    NULL,
                    nRet,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    (LPTSTR) &lpMsgBuf,
                    0,
                    NULL
                    );
                _tprintf(
                    _T("Ошибка загрузки \"КоИнсталлера\"!!\n\tКод ошибки = %d (0x%08X): %s\n"),
                    nRet,
                    nRet,
                    lpMsgBuf
                    );
                LocalFree(lpMsgBuf);
                SetupCloseInfFile(hInfHandle);
			    m_nRetCode = nRet;
                return FALSE;
            }
            m_pfnWdfPreDeviceInstall = (PFN_WDFPREDEVICEINSTALL)GetProcAddress(
                m_hCoInatallerModule,
                "WdfPreDeviceInstall"
                );
            ASSERT(NULL != m_pfnWdfPreDeviceInstall);
            m_pfnWdfPostDeviceInstall = (PFN_WDFPOSTDEVICEINSTALL)GetProcAddress(
                m_hCoInatallerModule,
                "WdfPostDeviceInstall"
                );
            ASSERT(NULL != m_pfnWdfPostDeviceInstall);
            m_pfnWdfPreDeviceInstallEx = (PFN_WDFPREDEVICEINSTALLEX)GetProcAddress(
                m_hCoInatallerModule,
                "WdfPreDeviceInstallEx"
                );
            ASSERT(NULL != m_pfnWdfPreDeviceInstallEx);
            m_pfnWdfPreDeviceRemove = (PFN_WDFPREDEVICEREMOVE)GetProcAddress(
                m_hCoInatallerModule,
                "WdfPreDeviceRemove"
                );
            ASSERT(NULL != m_pfnWdfPreDeviceRemove);
            m_pfnWdfPostDeviceRemove = (PFN_WDFPOSTDEVICEREMOVE)GetProcAddress(
                m_hCoInatallerModule,
                "WdfPostDeviceRemove"
                );
            ASSERT(NULL != m_pfnWdfPostDeviceRemove);
            // прединсталляция драйвера
            WDF_COINSTALLER_INSTALL_OPTIONS_INIT(&stClienOptions);
            stClienOptions.ShowRebootPrompt = FALSE;
            ulRetCode = m_pfnWdfPreDeviceInstallEx(
                m_sInfFilePath,
                WDF_SECTION_NAME,
                &stClienOptions
                );
            if(ERROR_SUCCESS != ulRetCode)
            {
                if(ERROR_SUCCESS_REBOOT_REQUIRED == ulRetCode)
                {
                    CString sApplicationName;
                    STARTUPINFO         stStartupInfo;
                    PROCESS_INFORMATION stProcessInformation;
                    _tprintf(
                        _T("Ошибка: перед установкой необходимо перезагрузить компьютер!!\n")
                        );
                    RtlZeroMemory(
                        &stStartupInfo,
                        sizeof(stStartupInfo)
                        );
                    stStartupInfo.cb = sizeof(stStartupInfo);
                    RtlZeroMemory(
                        &stProcessInformation,
                        sizeof(stProcessInformation)
                        );
                    sApplicationName  = m_sModuleFolder;
                    sApplicationName += _T("\\reboot_prompt.exe");
                    bRet = CreateProcess(
                        sApplicationName,
                        NULL,
                        NULL,
                        NULL,
                        FALSE,
                        0,
                        NULL,
                        NULL,
                        &stStartupInfo,
                        &stProcessInformation
                        );
                    ASSERT(bRet);
                    bRet = CloseHandle(stProcessInformation.hProcess);
                    bRet = CloseHandle(stProcessInformation.hThread);
                    m_nRetCode = ERROR_SUCCESS_REBOOT_REQUIRED;
                }
                else
                {
                    DWORD   dwRetCount;
                    LPVOID  lpMsgBuf;
                    CString sErrorMessage;
                    dwRetCount = FormatMessage(
                        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_FROM_HMODULE,
                        m_hCoInatallerModule,
                        ulRetCode,
                        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                        (LPTSTR) &lpMsgBuf,
                        0,
                        NULL
                        );
                    if(dwRetCount == 0)
                    {
                        DWORD   dwRetCode = ::GetLastError();
                        sErrorMessage.Format(
                            _T("Ошибка вызова функции \"FormatMessage\"! Код ошибки = %u."),
                            dwRetCode
                            );
                    }
                    else
                    {
                        sErrorMessage = (LPCTSTR)lpMsgBuf;
                        ::LocalFree((HLOCAL)lpMsgBuf);
                    }
                    _tprintf(
                        _T("Ошибка вызова функции \"WdfPreDeviceInstallEx\"!!\n\tКод ошибки = 0x%08X: %s\n"),
                        ulRetCode,
                        (LPCTSTR)sErrorMessage
                        );
                    m_nRetCode = ulRetCode;
                }
                bRet = FreeLibrary(m_hCoInatallerModule);
                SetupCloseInfFile(hInfHandle);
                return FALSE;
            }
            // инсталяция сервиса драйвера
            bRet = SetupInstallServicesFromInfSection(
                hInfHandle,
                SERVICE_SECTION_NAME,
                0
                );
            dwInstallRet = GetLastError();
            if(ERROR_SUCCESS_REBOOT_REQUIRED == dwInstallRet)
            {
                m_bRebootNeed = true;
            }
            else
            if(ERROR_SUCCESS != dwInstallRet)
            {
                DWORD   dwRetCount;
                LPVOID  lpMsgBuf;
                dwRetCount = FormatMessage(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                    NULL,
                    dwInstallRet,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    (LPTSTR) &lpMsgBuf,
                    0,
                    NULL
                    );
                _tprintf(
                    _T("Ошибка инсталяции сервиса драйвера!!\n\tКод ошибки = %d (0x%08X): %s\n"),
                    dwInstallRet,
                    dwInstallRet,
                    lpMsgBuf
                    );
                LocalFree(lpMsgBuf);
                SetupCloseInfFile(hInfHandle);
			    m_nRetCode = dwInstallRet;
                return FALSE;
            }
            SetupCloseInfFile(hInfHandle);
            //
            // POST-INSTALL for WDF support
            //
            ulRetCode = m_pfnWdfPostDeviceInstall(
                m_sInfFilePath,
                WDF_SECTION_NAME
                );
            if(ERROR_SUCCESS != ulRetCode)
            {
                DWORD   dwRetCount;
                LPVOID  lpMsgBuf;
                CString sErrorMessage;
                dwRetCount = FormatMessage(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_FROM_HMODULE,
                    m_hCoInatallerModule,
                    ulRetCode,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    (LPTSTR) &lpMsgBuf,
                    0,
                    NULL
                    );
                if(dwRetCount == 0)
                {
                    DWORD   dwRetCode = ::GetLastError();
                    sErrorMessage.Format(
                        _T("Ошибка вызова функции \"FormatMessage\"! Код ошибки = %u."),
                        dwRetCode
                        );
                }
                else
                {
                    sErrorMessage = (LPCTSTR)lpMsgBuf;
                    ::LocalFree((HLOCAL)lpMsgBuf);
                }
                _tprintf(
                    _T("Ошибка вызова функции \"WdfPostDeviceInstall\"!!\n\tКод ошибки = %d (0x%08X): %s\n"),
                    ulRetCode,
                    ulRetCode,
                    (LPCTSTR)sErrorMessage
                    );
                m_nRetCode = ulRetCode;
                return FALSE;
            }
        }
        if(m_bRebootNeed)
        {
            CString sApplicationName;
            STARTUPINFO         stStartupInfo;
            PROCESS_INFORMATION stProcessInformation;
            _tprintf(
                c_sRebootNeedStr
                );
            RtlZeroMemory(
                &stStartupInfo,
                sizeof(stStartupInfo)
                );
            stStartupInfo.cb = sizeof(stStartupInfo);
            RtlZeroMemory(
                &stProcessInformation,
                sizeof(stProcessInformation)
                );
            sApplicationName  = m_sModuleFolder;
            sApplicationName += _T("\\reboot_prompt.exe");
            bRet = CreateProcess(
                sApplicationName,
                NULL,
                NULL,
                NULL,
                FALSE,
                0,
                NULL,
                NULL,
                &stStartupInfo,
                &stProcessInformation
                );
            ASSERT(bRet);
            bRet = CloseHandle(stProcessInformation.hProcess);
            bRet = CloseHandle(stProcessInformation.hThread);
        }
        else
        {
            _tprintf(
                _T("Драйвер \"nattraverse_kmdffilter\" успешно установлен в систему.\n")
                );
        }
        return FALSE;
    }
    if(m_bInstallService)
    {
        // инсталяция сервиса
        SC_HANDLE schSCManager;
        SC_HANDLE schService;
        DWORD nErrorCode;
        LONG nResult;
        DWORD dwData, dwDisp;
        HKEY hk;
        DWORD dwStartType;
        BOOL  bRet;
        schSCManager = OpenSCManager(
            NULL,                 // local machine
            NULL,                 // ServicesActive database
            SC_MANAGER_ALL_ACCESS // full access rights
            );
        if(NULL == schSCManager) {
            DWORD   dwRetCount;
            LPVOID  lpMsgBuf;
            nErrorCode = ::GetLastError();
            CString sErrorMessage;
            dwRetCount = FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_FROM_HMODULE,
                NULL,
                nErrorCode,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR) &lpMsgBuf,
                0,
                NULL
                );
            if(dwRetCount == 0)
            {
                DWORD   dwRetCode = ::GetLastError();
                sErrorMessage.Format(
                    _T("Ошибка вызова функции \"FormatMessage\"! Код ошибки = %u."),
                    dwRetCode
                    );
            }
            else
            {
                sErrorMessage = (LPCTSTR)lpMsgBuf;
                ::LocalFree((HLOCAL)lpMsgBuf);
            }
            _tprintf(
                _T("Ошибка вызова функции \"OpenSCManager\"!!\n\tКод ошибки = 0x%08X: %s\n"),
                nErrorCode,
                (LPCTSTR)sErrorMessage
                );
            m_nRetCode = 23;
            return FALSE;
        }
        schService = OpenService(
            schSCManager,
            _T("NATTraversalService"),
            SERVICE_QUERY_CONFIG
            );
        if(NULL != schService) {
            CloseServiceHandle(schService);
            CloseServiceHandle(schSCManager);
            m_nRetCode = 3;
            _tprintf(
                _T("Ошибка: сервис \"NATTraversalService\" уже инсталирован.\r\n")
                );
            return FALSE;
        }
        nErrorCode = ::GetLastError();
        if(ERROR_SERVICE_DOES_NOT_EXIST != nErrorCode) {
            m_nRetCode = nErrorCode;
            CloseServiceHandle(schSCManager);
            _tprintf(
                _T("\"OpenService\" failed. Error code = %d\r\n"),
                nErrorCode
                );
            return FALSE;
        }
        dwStartType = (m_enStartMode == enAutomatic) ?
            SERVICE_AUTO_START : SERVICE_DEMAND_START;
        schService = CreateService(
            schSCManager,                    // SCManager database
            _T("NATTraversalService"),       // name of service
            _T("NATTraversalService"),       // service name to display
            SERVICE_ALL_ACCESS,              // desired access
            SERVICE_WIN32_OWN_PROCESS,       // service type
            dwStartType,                     // start type
            SERVICE_ERROR_NORMAL,            // error control type
            szPath,                          // path to service's binary
            NULL,                            // no load ordering group
            NULL,                            // no tag identifier
            _T("nattraverse_kmdffilter\0"),  // завсимость от драйвера "nattraverse_kmdffilter"
            NULL,                            // LocalSystem account
            NULL                             // no password
            );
        if(NULL == schService) {
            DWORD   dwRetCount;
            LPVOID  lpMsgBuf;
            nErrorCode = ::GetLastError();
            CString sErrorMessage;
            dwRetCount = FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_FROM_HMODULE,
                NULL,
                nErrorCode,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR) &lpMsgBuf,
                0,
                NULL
                );
            if(dwRetCount == 0)
            {
                DWORD   dwRetCode = ::GetLastError();
                sErrorMessage.Format(
                    _T("Ошибка вызова функции \"FormatMessage\"! Код ошибки = %u."),
                    dwRetCode
                    );
            }
            else
            {
                sErrorMessage = (LPCTSTR)lpMsgBuf;
                ::LocalFree((HLOCAL)lpMsgBuf);
            }
            _tprintf(
                _T("Ошибка вызова функции \"CreateService\"!!\n\tКод ошибки = 0x%08X: %s\n"),
                nErrorCode,
                (LPCTSTR)sErrorMessage
                );
            m_nRetCode = 24;
            return FALSE;
        }
        // регистрация источника сообщений
        nResult = RegCreateKeyEx(
            HKEY_LOCAL_MACHINE,
            _T("SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\NATTraversalService"),
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_WRITE,
            NULL,
            &hk,
            &dwDisp
            );
        nResult = RegSetValueEx(
            hk,
            _T("EventMessageFile"),
            0,
            REG_EXPAND_SZ,
            (LPBYTE)szPath,
            (DWORD)(lstrlen(szPath)+1)*sizeof(TCHAR)
            );
        dwData = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE;
        nResult = RegSetValueEx(
            hk,
            _T("TypesSupported"),
            0,
            REG_DWORD,
            (LPBYTE)&dwData,
            sizeof(DWORD)
            );
        nResult = RegCloseKey(hk);
        // занесение параметров сервиса в реестр
        bRet = ChangeServiceConfig2(
            schService,
            SERVICE_CONFIG_DESCRIPTION,
            (LPVOID)&c_stServiceDescription
            );
        _tprintf(
            _T("Сервис \"NATTraversalService\" успешно инсталирован.\n")
            );
        if(m_enStartMode == enAutomatic)
        {
            bRet = StartService(
                schService,
                0,
                NULL
                );
            if(!bRet)
            {
                m_nRetCode = ::GetLastError();
                _tprintf(
                    _T("Неудачная попытка запуска сервиса.\n\tКод ошибки = %u (0x%08lX)\n"),
                    m_nRetCode,
                    m_nRetCode
                    );
            }
        }
        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);
        return FALSE;
    }
    if(m_bUninstallService)
    {
        SC_HANDLE schSCManager;
        SC_HANDLE schService;
        DWORD nErrorCode;
        LONG nResult;
        SERVICE_STATUS st;
        BOOL bRet;
        // удаление сервиса "NATTraversalService"
        schSCManager = OpenSCManager(
            NULL,                 // local machine
            NULL,                 // ServicesActive database
            SC_MANAGER_ALL_ACCESS // full access rights
            );
        if(NULL == schSCManager) {
            nErrorCode = ::GetLastError();
            m_nRetCode = nErrorCode;
            _tprintf(
                _T("\"OpenSCManager\" failed. Error code = %d\r\n"),
                nErrorCode
                );
            return FALSE;
        }
        schService = OpenService(
            schSCManager,
            _T("NATTraversalService"),
            SERVICE_ALL_ACCESS
            );
        if(NULL == schService) {
            nErrorCode = ::GetLastError();
            CloseServiceHandle(schSCManager);
            if(ERROR_SERVICE_DOES_NOT_EXIST == nErrorCode) {
                _tprintf(
                    _T("Информация: сервис \"NATTraversalService\" не был инсталирован.\r\n")
                    );
                if(!m_bUninstallDriver)
                {
                    return FALSE;
                }
                else
                {
                    goto lUninstallDriver;
                }
            }
            _tprintf(
                _T("\"OpenService\" failed. Error code = %d\r\n"),
                nErrorCode
                );
            m_nRetCode = nErrorCode;
            return FALSE;
        }
        memset( &st, 0, sizeof st );
        QueryServiceStatus( schService, &st );
        if(SERVICE_RUNNING == st.dwCurrentState)
        {
            bRet = ControlService(
                schService,
                SERVICE_CONTROL_STOP,
                &st
                );
        }
        if(!DeleteService(schService)) {
            nErrorCode = ::GetLastError();
            m_nRetCode = nErrorCode;
            CloseServiceHandle(schService);
            CloseServiceHandle(schSCManager);
            _tprintf(
                _T("\"DeleteService\" failed. Error code = %d\r\n"),
                nErrorCode
                );
            return FALSE;
        }
        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);
        // удаление регистрации источника сообщений
        nResult = RegDeleteKey(
            HKEY_LOCAL_MACHINE,
            _T("SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\NATTraversalService")
            );
        // удаление ключа сервиса в реестре
        nResult = RegDeleteKey(
            HKEY_LOCAL_MACHINE,
            _T("SYSTEM\\CurrentControlSet\\Services\\NATTraversalService")
            );
        _tprintf(
            _T("Сервис \"NATTraversalService\" успешно удалён.\n")
            );
        if(!m_bUninstallDriver)
        {
            return FALSE;
        }
    }
lUninstallDriver:
    if(m_bUninstallDriver)
    {
        // удаление пакета драйвера из системы
        HINF                    hInfHandle;
        UINT                    nErrorLine, nPathLength;
        INFCONTEXT              stInfContext;
        TCHAR                   szTextBuffer[MAX_PATH];
        CString sTextBuffer, sMajorVer, sMinorVer;
        int                     nPos;
        long    nMajorVer, nMinorVer;
        ULONG                   ulRetCode;
        HSPFILEQ                hFileQueue;
        PVOID                   pDefaultCBContext;
        SC_HANDLE               schSCManager;
        SC_HANDLE               schService;
        DWORD                   cbBytesNeeded = 0;
        SERVICE_STATUS          stServiceStatus;
        SERVICE_STATUS_PROCESS  stServiceProcessStatus;
        HDEVINFO                hDevsInfoSet;
        SP_DEVINFO_DATA         stDevInfoData;
        SP_DRVINFO_DATA         stDrvInfoData;
        CONFIGRET               cr;
        bool                    bDeviceNotInstalled = false;
        PBYTE                   pDrvinfoDetailDataBuffer = NULL;
        PSP_DRVINFO_DETAIL_DATA pDrvinfoDetailData = NULL;
        DWORD                   dwDriverInfoDetailDataSize = sizeof(SP_DRVINFO_DETAIL_DATA);
        DWORD                   dwRequiredSize = 0;
        CString sOemInfPath, sOemInfName, sMessageText;
        TCHAR                   szInfFilePath[MAX_PATH];
        schSCManager = OpenSCManager(
            NULL,                 // local machine
            NULL,                 // ServicesActive database
            SC_MANAGER_ALL_ACCESS // full access rights
            );
        ASSERT(NULL != schSCManager);
        schService = OpenService(
            schSCManager,
            DRIVER_SERVICE_NAME,
            SERVICE_ALL_ACCESS
            );
        if(NULL == schService)
        {
            bRet = CloseServiceHandle(schSCManager);
            _tprintf(
                _T("Драйвер \"nattraverse_kmdffilter\" не установлен! Нет необходимости деинсталлировать его.\n")
                );
            return FALSE;
        }
        hDevsInfoSet = SetupDiCreateDeviceInfoList(
            NULL,
            NULL
            );
        ASSERT(INVALID_HANDLE_VALUE != hDevsInfoSet);
        stDevInfoData.cbSize = sizeof(stDevInfoData);
        bRet = SetupDiOpenDeviceInfo(
            hDevsInfoSet,
            _T("Root\\LEGACY_NATTRAVERSE_KMDFFILTER\\0000"),
            NULL,
            0,
            &stDevInfoData
            );
        if(!bRet)
        {
            bDeviceNotInstalled = true;
            bRet = SetupDiCreateDeviceInfo(
                hDevsInfoSet,
                _T("Root\\LEGACY_NATTRAVERSE_KMDFFILTER\\0000"),
                &GUID_NULL,
                NULL,
                NULL,
                0,
                &stDevInfoData
                );
            ASSERT(bRet);
        }
        cr = CM_Add_ID(
            stDevInfoData.DevInst,
            _T("Root\\nattraverse_kmdffilter"),
            CM_ADD_ID_HARDWARE
            );
        ASSERT(CR_SUCCESS == cr);
        bRet = SetupDiBuildDriverInfoList(
            hDevsInfoSet,
            &stDevInfoData,
            SPDIT_COMPATDRIVER
            );
        ASSERT(bRet);
        RtlZeroMemory(
            &stDrvInfoData,
            sizeof(stDrvInfoData)
            );
        stDrvInfoData.cbSize = sizeof(stDrvInfoData);
        bRet = SetupDiEnumDriverInfo(
            hDevsInfoSet,
            &stDevInfoData,
            SPDIT_COMPATDRIVER,
            0,
            &stDrvInfoData
            );
        ASSERT(bRet);
        pDrvinfoDetailDataBuffer = new BYTE[dwDriverInfoDetailDataSize];
        pDrvinfoDetailData = (PSP_DRVINFO_DETAIL_DATA)pDrvinfoDetailDataBuffer;
        pDrvinfoDetailData->cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);
        bRet = SetupDiGetDriverInfoDetail(
            hDevsInfoSet,
            &stDevInfoData,
            &stDrvInfoData,
            pDrvinfoDetailData,
            dwDriverInfoDetailDataSize,
            &dwRequiredSize
            );
        if(!bRet)
        {
            dwErrorCode = ::GetLastError();
            if(ERROR_INSUFFICIENT_BUFFER == dwErrorCode)
            {
                delete pDrvinfoDetailDataBuffer;
                pDrvinfoDetailDataBuffer = new BYTE[dwRequiredSize];
                pDrvinfoDetailData = (PSP_DRVINFO_DETAIL_DATA)pDrvinfoDetailDataBuffer;
                pDrvinfoDetailData->cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);
                bRet = SetupDiGetDriverInfoDetail(
                    hDevsInfoSet,
                    &stDevInfoData,
                    &stDrvInfoData,
                    pDrvinfoDetailData,
                    dwRequiredSize,
                    NULL
                    );
                if(!bRet)
                {
                    dwErrorCode = ::GetLastError();
                }
            }
        }
        ASSERT(bRet);
        sOemInfPath = pDrvinfoDetailData->InfFileName;
        delete pDrvinfoDetailDataBuffer;
        nPos = sOemInfPath.ReverseFind(_T('\\'));
        sOemInfName = sOemInfPath.Mid(nPos + 1);
        bRet = SetupDiDestroyDriverInfoList(
            hDevsInfoSet,
            &stDevInfoData,
            SPDIT_COMPATDRIVER
            );
        bRet = SetupDiDeleteDeviceInfo(
            hDevsInfoSet,
            &stDevInfoData
            );
        if(!bRet)
        {
            dwErrorCode = ::GetLastError();
            sMessageText.Format(
                _T("0x%08X"),
                dwErrorCode
                );
        }
        bRet = SetupDiDestroyDeviceInfoList(hDevsInfoSet);
        bRet = SetupGetInfDriverStoreLocation(
            (LPCTSTR)sOemInfPath,
            NULL,
            NULL,
            szInfFilePath,
            MAX_PATH,
            &dwRequiredSize
            );
        ASSERT(bRet);
        m_sInfFilePath = szInfFilePath;
        nPos = m_sInfFilePath.ReverseFind(_T('\\'));
        m_sFilterDriverFolder = m_sInfFilePath.Left(nPos);
        hInfHandle = SetupOpenInfFile(
            m_sInfFilePath,
            NULL,
            INF_STYLE_WIN4,
            &nErrorLine
            );
        if(INVALID_HANDLE_VALUE == hInfHandle)
        {
            DWORD   dwRetCount;
            DWORD   nRet;
            LPVOID  lpMsgBuf;
            nRet = ::GetLastError();
            dwRetCount = FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                NULL,
                nRet,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR) &lpMsgBuf,
                0,
                NULL
                );
            _tprintf(
                _T("Ошибка открытия INF-файла!!\n\tКод ошибки = %d (0x%08X): %s\n\tНомер строки INF-файла = %d\n"),
                nRet,
                nRet,
                lpMsgBuf,
                nErrorLine
                );
            LocalFree(lpMsgBuf);
			m_nRetCode = 15;
            bRet = CloseServiceHandle(schService);
            bRet = CloseServiceHandle(schSCManager);
            return FALSE;
        }
        bRet = SetupFindFirstLine(
            hInfHandle,
            _T("nattraverse_kmdffilter_wdfsect"),
            _T("KmdfLibraryVersion"),
            &stInfContext
            );
        if(!bRet)
        {
            DWORD   dwRetCount;
            DWORD   nRet;
            LPVOID  lpMsgBuf;
            nRet = ::GetLastError();
            dwRetCount = FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                NULL,
                nRet,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR) &lpMsgBuf,
                0,
                NULL
                );
            _tprintf(
                _T("Ошибка поиска строки значения \"KmdfLibraryVersion\"!!\n\tКод ошибки = %d (0x%08X): %s\n"),
                nRet,
                nRet,
                lpMsgBuf
                );
            LocalFree(lpMsgBuf);
            SetupCloseInfFile(hInfHandle);
			m_nRetCode = 16;
            bRet = CloseServiceHandle(schService);
            bRet = CloseServiceHandle(schSCManager);
            return FALSE;
        }
        bRet = SetupGetStringField(
            &stInfContext,
            1,
            szTextBuffer,
            MAX_PATH,
            &dwRequiredSize
            );
        if(!bRet)
        {
            DWORD   dwRetCount;
            DWORD   nRet;
            LPVOID  lpMsgBuf;
            nRet = ::GetLastError();
            dwRetCount = FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                NULL,
                nRet,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR) &lpMsgBuf,
                0,
                NULL
                );
            _tprintf(
                _T("Ошибка получения строки значения \"KmdfLibraryVersion\"!!\n\tКод ошибки = %d (0x%08X): %s\n"),
                nRet,
                nRet,
                lpMsgBuf
                );
            LocalFree(lpMsgBuf);
            SetupCloseInfFile(hInfHandle);
			m_nRetCode = 17;
            bRet = CloseServiceHandle(schService);
            bRet = CloseServiceHandle(schSCManager);
            return FALSE;
        }
        sTextBuffer = szTextBuffer;
        nPos = sTextBuffer.Find(_T('.'));
        sMajorVer = sTextBuffer.Left(nPos);
        sMinorVer = sTextBuffer.Mid(nPos + 1);
        nMajorVer = _tstol(sMajorVer);
        nMinorVer = _tstol(sMinorVer);
        sTextBuffer.Format(
            _T("%02d%03d"),
            nMajorVer,
            nMinorVer
            );
        m_sCoInstallerPath  = m_sFilterDriverFolder;
        m_sCoInstallerPath += _T("\\WdfCoInstaller");
        m_sCoInstallerPath += sTextBuffer;
        m_sCoInstallerPath += _T(".dll");
        nPathLength = GetWindowsDirectory(
            szTextBuffer,
            MAX_PATH
            );
        ASSERT(nPathLength > 1);
        m_sDriversFolder  = szTextBuffer;
        m_sDriversFolder += _T("\\System32\\drivers");
        // загрузка КоИнсталлера
        m_hCoInatallerModule = LoadLibrary(m_sCoInstallerPath);
        if(NULL == m_hCoInatallerModule)
        {
            DWORD   dwRetCount;
            DWORD   nRet;
            LPVOID  lpMsgBuf;
            nRet = ::GetLastError();
            dwRetCount = FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                NULL,
                nRet,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR) &lpMsgBuf,
                0,
                NULL
                );
            _tprintf(
                _T("Ошибка загрузки \"КоИнсталлера\"!!\n\tКод ошибки = %d (0x%08X): %s\n"),
                nRet,
                nRet,
                lpMsgBuf
                );
            LocalFree(lpMsgBuf);
            SetupCloseInfFile(hInfHandle);
			m_nRetCode = 18;
            bRet = CloseServiceHandle(schService);
            bRet = CloseServiceHandle(schSCManager);
            return FALSE;
        }
        m_pfnWdfPreDeviceInstall = (PFN_WDFPREDEVICEINSTALL)GetProcAddress(
            m_hCoInatallerModule,
            "WdfPreDeviceInstall"
            );
        ASSERT(NULL != m_pfnWdfPreDeviceInstall);
        m_pfnWdfPostDeviceInstall = (PFN_WDFPOSTDEVICEINSTALL)GetProcAddress(
            m_hCoInatallerModule,
            "WdfPostDeviceInstall"
            );
        ASSERT(NULL != m_pfnWdfPostDeviceInstall);
        m_pfnWdfPreDeviceInstallEx = (PFN_WDFPREDEVICEINSTALLEX)GetProcAddress(
            m_hCoInatallerModule,
            "WdfPreDeviceInstallEx"
            );
        ASSERT(NULL != m_pfnWdfPreDeviceInstallEx);
        m_pfnWdfPreDeviceRemove = (PFN_WDFPREDEVICEREMOVE)GetProcAddress(
            m_hCoInatallerModule,
            "WdfPreDeviceRemove"
            );
        ASSERT(NULL != m_pfnWdfPreDeviceRemove);
        m_pfnWdfPostDeviceRemove = (PFN_WDFPOSTDEVICEREMOVE)GetProcAddress(
            m_hCoInatallerModule,
            "WdfPostDeviceRemove"
            );
        ASSERT(NULL != m_pfnWdfPostDeviceRemove);
        //
        // PRE-REMOVE of WDF support
        //
        ulRetCode = m_pfnWdfPreDeviceRemove(
            m_sInfFilePath,
            WDF_SECTION_NAME
            );
        if(ERROR_SUCCESS != ulRetCode)
        {
            DWORD   dwRetCount;
            LPVOID  lpMsgBuf;
            CString sErrorMessage;
            dwRetCount = FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_FROM_HMODULE,
                m_hCoInatallerModule,
                ulRetCode,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR) &lpMsgBuf,
                0,
                NULL
                );
            if(dwRetCount == 0)
            {
                DWORD   dwRetCode = ::GetLastError();
                sErrorMessage.Format(
                    _T("Ошибка вызова функции \"FormatMessage\"! Код ошибки = %u."),
                    dwRetCode
                    );
            }
            else
            {
                sErrorMessage = (LPCTSTR)lpMsgBuf;
                ::LocalFree((HLOCAL)lpMsgBuf);
            }
            _tprintf(
                _T("Ошибка вызова функции \"WdfPreDeviceRemove\"!!\n\tКод ошибки = 0x%08X: %s\n"),
                ulRetCode,
                (LPCTSTR)sErrorMessage
                );
            bRet = FreeLibrary(m_hCoInatallerModule);
            SetupCloseInfFile(hInfHandle);
            m_nRetCode = 19;
            bRet = CloseServiceHandle(schService);
            bRet = CloseServiceHandle(schSCManager);
            return FALSE;
        }
        // деинсталляция сервиса драйвера
        RtlZeroMemory(
            &stServiceProcessStatus,
            sizeof(stServiceProcessStatus)
            );
        RtlZeroMemory(
            &stServiceStatus,
            sizeof(stServiceStatus)
            );
        bRet = QueryServiceStatusEx(
            schService,
            SC_STATUS_PROCESS_INFO,
            (LPBYTE)&stServiceProcessStatus,
            sizeof(stServiceProcessStatus),
            &cbBytesNeeded
            );
        if(SERVICE_RUNNING == stServiceProcessStatus.dwCurrentState)
        {
            bRet = ControlService(
                schService,
                SERVICE_CONTROL_STOP,
                &stServiceStatus
                );
            if(!bRet)
            {
                DWORD   dwRetCount;
                LPVOID  lpMsgBuf;
                DWORD dwErrorCode = GetLastError();
                dwRetCount = FormatMessage(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                    NULL,
                    dwErrorCode,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    (LPTSTR) &lpMsgBuf,
                    0,
                    NULL
                    );
                _tprintf(
                    _T("Ошибка остановки работающего сервиса драйвера!!\n\tКод ошибки = %d (0x%08X): %s\n"),
                    dwErrorCode,
                    dwErrorCode,
                    lpMsgBuf
                    );
                LocalFree(lpMsgBuf);
                bRet = CloseServiceHandle(schService);
                bRet = CloseServiceHandle(schSCManager);
                bRet = FreeLibrary(m_hCoInatallerModule);
                SetupCloseInfFile(hInfHandle);
			    m_nRetCode = 21;
                return FALSE;
            }
        }
        bRet = DeleteService(schService);
        if(!bRet)
        {
            dwErrorCode = ::GetLastError();
            ASSERT(ERROR_SERVICE_MARKED_FOR_DELETE == dwErrorCode);
            m_bRebootNeed = true;
        }
        bRet = CloseServiceHandle(schService);
        bRet = CloseServiceHandle(schSCManager);
        //
        // POST-REMOVE of WDF support
        //
        ulRetCode = m_pfnWdfPostDeviceRemove(
            m_sInfFilePath,
            WDF_SECTION_NAME
            );
        if(ERROR_SUCCESS != ulRetCode)
        {
            DWORD   dwRetCount;
            LPVOID  lpMsgBuf;
            CString sErrorMessage;
            dwRetCount = FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_FROM_HMODULE,
                m_hCoInatallerModule,
                ulRetCode,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR) &lpMsgBuf,
                0,
                NULL
                );
            if(dwRetCount == 0)
            {
                DWORD   dwRetCode = ::GetLastError();
                sErrorMessage.Format(
                    _T("Ошибка вызова функции \"FormatMessage\"! Код ошибки = %u."),
                    dwRetCode
                    );
            }
            else
            {
                sErrorMessage = (LPCTSTR)lpMsgBuf;
                ::LocalFree((HLOCAL)lpMsgBuf);
            }
            _tprintf(
                _T("Ошибка вызова функции \"WdfPostDeviceRemove\"!!\n\tКод ошибки = 0x%08X: %s\n"),
                ulRetCode,
                (LPCTSTR)sErrorMessage
                );
            bRet = FreeLibrary(m_hCoInatallerModule);
            SetupCloseInfFile(hInfHandle);
            m_nRetCode = 22;
            return FALSE;
        }
        bRet = FreeLibrary(m_hCoInatallerModule);
        // удаление файла драйвера
        hFileQueue = SetupOpenFileQueue();
        if(INVALID_HANDLE_VALUE == hFileQueue)
        {
            DWORD   dwRetCount;
            DWORD   nRet;
            LPVOID  lpMsgBuf;
            nRet = ::GetLastError();
            dwRetCount = FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                NULL,
                nRet,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR) &lpMsgBuf,
                0,
                NULL
                );
            _tprintf(
                _T("Ошибка открытия очереди устанавливаемых файлов!!\n\tКод ошибки = %d (0x%08X): %s\n"),
                nRet,
                nRet,
                lpMsgBuf
                );
            LocalFree(lpMsgBuf);
            SetupCloseInfFile(hInfHandle);
            _tprintf(
                _T("Предупреждение: файл драйвера не удалён из папки драйверов!\nМожете удалить его вручную позже.\n")
                );
			m_nRetCode = 0;
            return FALSE;
        }
        bRet = SetupQueueDeleteSection(
            hFileQueue,
            hInfHandle,
            NULL,
            _T("Drivers_Dir")
            );
        if(!bRet)
        {
            DWORD   dwRetCount;
            DWORD   nRet;
            LPVOID  lpMsgBuf;
            nRet = ::GetLastError();
            dwRetCount = FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                NULL,
                nRet,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR) &lpMsgBuf,
                0,
                NULL
                );
            _tprintf(
                _T("Ошибка удаления секции [Drivers_Dir]!!\n\tКод ошибки = %d (0x%08X): %s\n"),
                nRet,
                nRet,
                lpMsgBuf
                );
            LocalFree(lpMsgBuf);
            SetupCloseFileQueue(hFileQueue);
            SetupCloseInfFile(hInfHandle);
            _tprintf(
                _T("Предупреждение: файл драйвера не удалён из папки драйверов!\nМожете удалить его вручную позже.\n")
                );
			m_nRetCode = 0;
            return FALSE;
        }
        pDefaultCBContext = SetupInitDefaultQueueCallback(NULL);
        ASSERT(NULL != pDefaultCBContext);
        bRet = SetupCommitFileQueue(
            NULL,
            hFileQueue,
            SetupDefaultQueueCallback,
            pDefaultCBContext
            );
        if(!bRet)
        {
            DWORD   dwRetCount;
            DWORD   nRet;
            LPVOID  lpMsgBuf;
            nRet = ::GetLastError();
            SetupTermDefaultQueueCallback(pDefaultCBContext);
            dwRetCount = FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                NULL,
                nRet,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR) &lpMsgBuf,
                0,
                NULL
                );
            _tprintf(
                _T("Ошибка подтверждения очереди файлов!!\n\tКод ошибки = %d (0x%08X): %s\n"),
                nRet,
                nRet,
                lpMsgBuf
                );
            LocalFree(lpMsgBuf);
            _tprintf(
                _T("Предупреждение: файл драйвера не удалён из папки драйверов!\nМожете удалить его вручную позже.\n")
                );
            nPromptRet = SetupPromptReboot(
                hFileQueue,
                NULL,
                FALSE
                );
            SetupCloseFileQueue(hFileQueue);
            SetupCloseInfFile(hInfHandle);
			m_nRetCode = 0;
            return FALSE;
        }
        SetupTermDefaultQueueCallback(pDefaultCBContext);
        SetupCloseFileQueue(hFileQueue);
        SetupCloseInfFile(hInfHandle);
        // удаление пакета драйвера из хранилища драйверов
        bRet = SetupUninstallOEMInf(
            sOemInfName,
            SUOI_FORCEDELETE,
            NULL
            );
        if(!bRet)
        {
            dwErrorCode = ::GetLastError();
            sMessageText.Format(
                _T("0x%08X"),
                dwErrorCode
                );
        }
        if(m_bRebootNeed)
        {
            _tprintf(
                c_sUninstallRebootStr
                );
            free((void*)m_pszProfileName);
            m_pszProfileName = _tcsdup(_T("rtsdd_parameters"));
            bRet = WriteConfigInt(
                _T("rtsdd_setup"),
                _T("reboot_need"),
                1
                );
        }
        else
        {
            _tprintf(
                _T("Драйвер \"nattraverse_kmdffilter\" успешно удалён из системы.\n")
                );
        }
        return FALSE;
    }
    return TRUE;
}



int CNATTraversalServiceApp::ExitInstance()
{
    CWinApp::ExitInstance();
    return m_nRetCode;
}


void CNATTraversalServiceApp::ReportEventInvoke(
    WORD wType,
    DWORD dwEventID,
    WORD cInserts,
    LPCTSTR* szMsgArray
    )
{
    HANDLE hEventLog;
    hEventLog = RegisterEventSource(
        NULL,
        _T("NATTraversalService")
        );
    ReportEvent(
        hEventLog,
        wType,
        0,
        dwEventID,
        NULL,
        cInserts,
        0,
        szMsgArray,
        NULL
        );
    DeregisterEventSource(hEventLog);
}


int CNATTraversalServiceApp::Run()
{
    BOOL bRet;
    CString sMsg;
    SERVICE_TABLE_ENTRY arDispatchTable[] =
    {
        { _T("NATTraversalService"), ServiceMain },
        { NULL,            NULL        }
    };
    bRet = StartServiceCtrlDispatcher(
        arDispatchTable
        );
    if(!bRet) {
        m_nRetCode = ::GetLastError();
        sMsg.Format(
            _T("\"StartServiceCtrlDispatcher\" failed.\r\n\tnErrorCode = %u (0x%08lX)\r\n"),
            m_nRetCode
            );
        LPCTSTR pStrings[] = { (LPCTSTR)sMsg };
        ReportEventInvoke(
            EVENTLOG_ERROR_TYPE,
            MSG_ERROR_EVENT_TEXT,
            1,
            pStrings
            );
    }
    return ExitInstance();
}


void CNATTraversalServiceApp::ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv)
{
    theApp.mfServiceMain(dwArgc, lpszArgv);
}


void CNATTraversalServiceApp::mfServiceMain(DWORD dwArgc, LPTSTR* lpszArgv)
{
    DWORD status;
    DWORD specificError;
    m_stServiceStatus.dwServiceType             = SERVICE_WIN32_OWN_PROCESS;
    m_stServiceStatus.dwCurrentState            = SERVICE_START_PENDING;
    m_stServiceStatus.dwControlsAccepted        = SERVICE_ACCEPT_STOP;
    m_stServiceStatus.dwWin32ExitCode           = 0;
    m_stServiceStatus.dwServiceSpecificExitCode = 0;
    m_stServiceStatus.dwCheckPoint              = 0;
    m_stServiceStatus.dwWaitHint                = 0;
    m_hServiceStatusHandle = RegisterServiceCtrlHandler(
        _T("NATTraversalService"),
        ServiceCtrlHandler
        );
    SetServiceStatus(
        m_hServiceStatusHandle,
        &m_stServiceStatus
        );
    // Initialization code goes here.
    status = ServiceInitialization(
        dwArgc,
        lpszArgv,
        &specificError
        );
    // Handle error condition
    if(status != NO_ERROR) {
        m_stServiceStatus.dwCurrentState            = SERVICE_STOPPED;
        m_stServiceStatus.dwCheckPoint              = 0;
        m_stServiceStatus.dwWaitHint                = 0;
        m_stServiceStatus.dwWin32ExitCode           = status;
        m_stServiceStatus.dwServiceSpecificExitCode = specificError;
        m_nRetCode = status;
        SetServiceStatus(
            m_hServiceStatusHandle,
            &m_stServiceStatus
            );
        return;
    }
    // Initialization complete - report running status.
    m_stServiceStatus.dwCurrentState            = SERVICE_RUNNING;
    m_stServiceStatus.dwCheckPoint              = 0;
    m_stServiceStatus.dwWaitHint                = 0;
    SetServiceStatus(
        m_hServiceStatusHandle,
        &m_stServiceStatus
        );
}


void CNATTraversalServiceApp::ServiceCtrlHandler(DWORD fdwControl)
{
    theApp.mfServiceCtrlHandler(fdwControl);
}


void CNATTraversalServiceApp::mfServiceCtrlHandler(DWORD fdwControl)
{
    BOOL    bRet;
    HANDLE  hWorkerThread;
    DWORD   dwWaitState;
    switch(fdwControl) {
    case SERVICE_CONTROL_STOP:
        bRet = m_oWorkerThreadLock.Lock();
        if(NULL == m_pWorkerThread)
        {
            bRet = m_oWorkerThreadLock.Unlock();
            m_stServiceStatus.dwCurrentState            = SERVICE_STOPPED;
            m_stServiceStatus.dwCheckPoint              = 0;
            m_stServiceStatus.dwWaitHint                = 0;
            m_stServiceStatus.dwWin32ExitCode           = m_nRetCode;
            m_stServiceStatus.dwServiceSpecificExitCode = 0;
            SetServiceStatus(
                m_hServiceStatusHandle,
                &m_stServiceStatus
                );
            return;
        }
        {
            CEvent* pDoneEvent;
            m_dwCheckPoint++;
            m_stServiceStatus.dwCurrentState            = SERVICE_STOP_PENDING;
            m_stServiceStatus.dwCheckPoint              = m_dwCheckPoint;
            m_stServiceStatus.dwWaitHint                = 60000;
            m_stServiceStatus.dwWin32ExitCode           = 0;
            m_stServiceStatus.dwServiceSpecificExitCode = 0;
            SetServiceStatus(
                m_hServiceStatusHandle,
                &m_stServiceStatus
                );
            pDoneEvent = &m_pWorkerThread->m_oThreadDoneEvent;
            bRet = DuplicateHandle(
                GetCurrentProcess(),
                *m_pWorkerThread,
                GetCurrentProcess(),
                &hWorkerThread,
                0,
                FALSE,
                DUPLICATE_SAME_ACCESS
                );
            ASSERT(bRet);
            m_pWorkerThread->PostThreadMessage(
                WM_QUIT,
                0,
                0
                );
            bRet = m_oWorkerThreadLock.Unlock();
            dwWaitState = WaitForSingleObject(
                *pDoneEvent,
                INFINITE // 20000 
                );
            bRet = m_oWorkerThreadLock.Lock();
            if(NULL != m_pWorkerThread)
            {
                if(m_pWorkerThread->m_bOnDelete)
                {
                    bRet = m_oWorkerThreadLock.Unlock();
                }
                else
                {
                    bRet = TerminateThread(
                        hWorkerThread,
                        (DWORD)(-1)
                        );
                    bRet = m_oWorkerThreadLock.Unlock();
                    try
                    {
                        delete m_pWorkerThread;
                    }
                    catch(...)
                    {
                    }
                }
                m_pWorkerThread = NULL;
            }
            else
            {
                bRet = m_oWorkerThreadLock.Unlock();
            }
            do
            {
                dwWaitState = 0;
                bRet = GetExitCodeThread(
                    hWorkerThread,
                    &dwWaitState
                    );
                if(STILL_ACTIVE != dwWaitState)
                {
                    break;
                }
                Sleep(1000);
                m_dwCheckPoint++;
                m_stServiceStatus.dwCurrentState            = SERVICE_STOP_PENDING;
                m_stServiceStatus.dwCheckPoint              = m_dwCheckPoint;
                m_stServiceStatus.dwWaitHint                = 60000;
                m_stServiceStatus.dwWin32ExitCode           = 0;
                m_stServiceStatus.dwServiceSpecificExitCode = 0;
                SetServiceStatus(
                    m_hServiceStatusHandle,
                    &m_stServiceStatus
                    );
            }
            while(true);
            bRet = CloseHandle(hWorkerThread);
        }
        m_stServiceStatus.dwCurrentState            = SERVICE_STOPPED;
        m_stServiceStatus.dwCheckPoint              = 0;
        m_stServiceStatus.dwWaitHint                = 0;
        m_stServiceStatus.dwWin32ExitCode           = m_nRetCode;
        m_stServiceStatus.dwServiceSpecificExitCode = 0;
        SetServiceStatus(
            m_hServiceStatusHandle,
            &m_stServiceStatus
            );
        return;
    case SERVICE_CONTROL_INTERROGATE:
        break;
    default:
        break;
    }
    // Send current status.
    SetServiceStatus(
        m_hServiceStatusHandle,
        &m_stServiceStatus
        );
}


DWORD CNATTraversalServiceApp::ServiceInitialization(
    DWORD argc,
    LPTSTR* argv,
    DWORD* specificError
    )
{
    DWORD   dwRetCode = 0;
    BOOL    bRet;
    DWORD   dwWaitState;
    int     nRet;
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);
    *specificError = 0;
    m_pWorkerThread = new CMainWorkerThread;
    ASSERT(NULL != m_pWorkerThread);
    bRet = m_oWorkerThreadLock.Lock();
    bRet = m_pWorkerThread->CreateThread( );
    if(!bRet)
    {
        CString sMsg;
        sMsg.Format(
            _T("Ошибка создания рабочего потока!!\n")
            );
        try
        {
            delete m_pWorkerThread;
        }
        catch(...)
        {
        }
        m_pWorkerThread = NULL;
        bRet = m_oWorkerThreadLock.Unlock();
        dwRetCode = 1;
        LPCTSTR pStrings[] = { (LPCTSTR)sMsg };
        ReportEventInvoke(
            EVENTLOG_ERROR_TYPE,
            MSG_ERROR_EVENT_TEXT,
            1,
            pStrings
            );
        return dwRetCode;
    }
    bRet = m_oWorkerThreadLock.Unlock();
    HANDLE  haWaitedHanles[] = {
        m_oInitWorkerThreadDone,
        *m_pWorkerThread
    };
    dwWaitState = ::WaitForMultipleObjects(
        2,
        haWaitedHanles,
        FALSE,
        INFINITE
        );
    if(WAIT_OBJECT_0 + 1 == dwWaitState)
    {
        CString sMsg;
        Sleep(100);
        bRet = m_oWorkerThreadLock.Lock();
        if(NULL != m_pWorkerThread)
        {
            nRet = -1;
            try
            {
                nRet = m_pWorkerThread->m_nRetCode;
                delete m_pWorkerThread;
            }
            catch(...)
            {
            }
            m_pWorkerThread = NULL;
        }
        bRet = m_oWorkerThreadLock.Unlock();
        sMsg.Format(
            _T("Ошибка инициализации рабочего потока DNS-плагина!!\n")
            );
        dwRetCode = 1;
        LPCTSTR pStrings[] = { (LPCTSTR)sMsg };
        ReportEventInvoke(
            EVENTLOG_ERROR_TYPE,
            MSG_ERROR_EVENT_TEXT,
            1,
            pStrings
            );
        return dwRetCode;
    }
    return dwRetCode;
}
