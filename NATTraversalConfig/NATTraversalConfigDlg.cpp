
// NATTraversalConfigDlg.cpp : файл реализации
//

#include "stdafx.h"
#include "NATTraversalConfig.h"
#include "NATTraversalConfigDlg.h"
#include "AboutBox.h"
#include "commrcfg.h"
#include "RTSDDMessageBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define SERVICE_FILE_NAME _T("NATTraversalService.exe")

const SERVICE_DESCRIPTION c_stServiceDescription =
{
    _T("Служба обхода NAT")
};

// диалоговое окно CNATTraversalConfigDlg

BEGIN_DHTML_EVENT_MAP(CNATTraversalConfigDlg)
	DHTML_EVENT_ONCLICK(_T("ButtonOK"), OnButtonOK)
	DHTML_EVENT_ONCLICK(_T("ButtonCancel"), OnButtonCancel)
    DHTML_EVENT_ONCLICK(_T("setup_button"), OnSetupClick)
    DHTML_EVENT_ONCLICK(_T("exit_button"), OnExitClick)
    DHTML_EVENT_ONCLICK(_T("about_icon"), DoAboutBox)
    DHTML_EVENT_ONCLICK(_T("executable_path_button"), SelectServicePath)
    DHTML_EVENT_ONCLICK(_T("test_public_router_ip_button"), TestRouteIP)
    DHTML_EVENT_ONCLICK(_T("insert_public_ip"), InsertPublicIP)
    DHTML_EVENT_ONCLICK(_T("delete_public_ip"), DeletePublicIP)
    DHTML_EVENT_ONCLICK(_T("test_public_ip_button"), TestPublicIP)
    DHTML_EVENT_ONCLICK(_T("insert_public_range"), InsertPublicRange)
    DHTML_EVENT_ONCLICK(_T("delete_public_range"), DeletePublicRange)
    DHTML_EVENT_ONCLICK(_T("test_public_range_button"), TestPublicRange)
    DHTML_EVENT_ONCLICK(_T("insert_public_subnet"), InsertPublicSubnet)
    DHTML_EVENT_ONCLICK(_T("delete_public_subnet"), DeletePublicSubnet)
    DHTML_EVENT_ONCLICK(_T("test_public_subnet_button"), TestPublicSubnet)
    DHTML_EVENT_ONCLICK(_T("insert_private_ip"), InsertPrivateIP)
    DHTML_EVENT_ONCLICK(_T("delete_private_ip"), DeletePrivateIP)
    DHTML_EVENT_ONCLICK(_T("test_private_ip_button"), TestPrivateIP)
    DHTML_EVENT_ONCLICK(_T("insert_private_range"), InsertPrivateRange)
    DHTML_EVENT_ONCLICK(_T("delete_private_range"), DeletePrivateRange)
    DHTML_EVENT_ONCLICK(_T("test_private_range_button"), TestPrivateRange)
    DHTML_EVENT_ONCLICK(_T("insert_private_subnet"), InsertPrivateSubnet)
    DHTML_EVENT_ONCLICK(_T("delete_private_subnet"), DeletePrivateSubnet)
    DHTML_EVENT_ONCLICK(_T("test_private_subnet_button"), TestPrivateSubnet)
    DHTML_EVENT_ONCLICK(_T("insert_ip_port"), InsertIpPort)
    DHTML_EVENT_ONCLICK(_T("delete_ip_port"), DeleteIpPort)
    DHTML_EVENT_ONCLICK(_T("test_ip_port_button"), TesIpPort)
    DHTML_EVENT_ONCLICK(_T("insert_ip_port_range"), InsertIpPortRange)
    DHTML_EVENT_ONCLICK(_T("delete_ip_port_range"), DeleteIpPortRange)
    DHTML_EVENT_ONCLICK(_T("test_ip_port_range_button"), TestIpPortRange)
    DHTML_EVENT_ONCLICK(_T("save_button"), SaveParameters)
    DHTML_EVENT_ONCLICK(_T("load_button"), ReloadParameters)
    DHTML_EVENT_ONCLICK(_T("uninstall_button"), OnUninstallClick)
    DHTML_EVENT_ONCLICK(_T("restart_button"), OnRestartClick)
END_DHTML_EVENT_MAP()



CNATTraversalConfigDlg::CNATTraversalConfigDlg(CWnd* pParent /*=NULL*/)
	: CDHtmlDialog(_T("NATTRAVERSALCONFIG_DIALOG"), _T("NATTRAVERSALCONFIG.HTM"), pParent)
    , m_bNoBreakStage(false)
    , m_sFileVersion(theApp.m_sVersionHTMLString)
    , m_sStatusStr(_T("Готово"))
    , m_sServicesFolder(_T(""))
    , m_sPublicRouterIP(_T(""))
    , m_sSetupImageSRC(_T("SETUP.GIF"))
    , m_bHTMLInit(false)
    , m_bServiceInstalled(false)
    , m_sUninstallImageSRC(_T("UNINSTALL32.PNG"))
    , m_sRestartImageSRC(_T("RESTARTHS.PNG"))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    m_bUseHtmlTitle = TRUE;
	m_sServicesFolder = GetConfigString(
        _T("service_settings"),
        _T("service_folder"),
        theApp.m_sModuleFolder
        );
	m_sPublicRouterIP = GetConfigString(
        _T("service_settings"),
        _T("public_router_ip"),
        _T("")
        );
}

void CNATTraversalConfigDlg::DoDataExchange(CDataExchange* pDX)
{
    DDX_DHtml_ElementInnerHtml(
        pDX,
        _T("version_str"),
        m_sFileVersion
        );
    DDX_DHtml_ElementInnerText(
        pDX,
        _T("status_text"),
        m_sStatusStr
        );
    DDX_DHtml_ElementValue(
        pDX,
        _T("executable_path"),
        m_sServicesFolder
        );
    DDX_DHtml_ElementValue(
        pDX,
        _T("public_router_ip"),
        m_sPublicRouterIP
        );
    DDX_DHtml_Img_Src(
        pDX,
        _T("setup_image"),
        m_sSetupImageSRC
        );
    DDX_DHtml_Img_Src(
        pDX,
        _T("uninstall_image"),
        m_sUninstallImageSRC
        );
    DDX_DHtml_Img_Src(
        pDX,
        _T("restart_image"),
        m_sRestartImageSRC
        );
	CDHtmlDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CNATTraversalConfigDlg, CDHtmlDialog)
	ON_WM_SYSCOMMAND()
    ON_WM_SIZE()
    ON_WM_CLOSE()
    ON_WM_DESTROY()
END_MESSAGE_MAP()


// обработчики сообщений CNATTraversalConfigDlg

BOOL CNATTraversalConfigDlg::OnInitDialog()
{
    SetHostFlags(
        DOCHOSTUIFLAG_NO3DBORDER | DOCHOSTUIFLAG_DIALOG
        );
	CDHtmlDialog::OnInitDialog();

	// Добавление пункта "О программе..." в системное меню.

	// IDM_ABOUTBOX должен быть в пределах системной команды.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Задает значок для этого диалогового окна. Среда делает это автоматически,
	//  если главное окно приложения не является диалоговым
	SetIcon(m_hIcon, TRUE);			// Крупный значок
	SetIcon(m_hIcon, FALSE);		// Мелкий значок

	return TRUE;  // возврат значения TRUE, если фокус не передан элементу управления
}

void CNATTraversalConfigDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutBox dlgAbout(this);
		dlgAbout.DoModal();
	}
	else
	{
		CDHtmlDialog::OnSysCommand(nID, lParam);
	}
}

// При добавлении кнопки свертывания в диалоговое окно нужно воспользоваться приведенным ниже кодом,
//  чтобы нарисовать значок. Для приложений MFC, использующих модель документов или представлений,
//  это автоматически выполняется рабочей областью.

void CNATTraversalConfigDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // контекст устройства для рисования

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Выравнивание значка по центру клиентского прямоугольника
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Нарисуйте значок
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDHtmlDialog::OnPaint();
	}
}

// Система вызывает эту функцию для получения отображения курсора при перемещении
//  свернутого окна.
HCURSOR CNATTraversalConfigDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

HRESULT CNATTraversalConfigDlg::OnButtonOK(IHTMLElement* /*pElement*/)
{
	OnOK();
	return S_OK;
}

HRESULT CNATTraversalConfigDlg::OnButtonCancel(IHTMLElement* /*pElement*/)
{
	OnCancel();
	return S_OK;
}


void CNATTraversalConfigDlg::OnSize(UINT nType, int cx, int cy)
{
    CDHtmlDialog::OnSize(nType, cx, cy);
    if(nType == SIZE_RESTORED) {
        UpdateWindow();
    }
}


void CNATTraversalConfigDlg::OnClose()
{
    if(!m_bNoBreakStage) {
        DestroyModeless( );
    }
}


HRESULT CNATTraversalConfigDlg::OnSetupClick(IHTMLElement* pElement)
{
    // инсталяция сервиса
    SC_HANDLE schSCManager;
    SC_HANDLE schService;
    DWORD nErrorCode;
    LONG nResult;
    DWORD dwData, dwDisp;
    HKEY hk;
    DWORD dwStartType;
    BOOL bRet = UpdateData(TRUE);
    CString sFullFilePath(m_sServicesFolder);
    HRESULT hr;
    MSHTML::IHTMLElementPtr spIHTMLElement;
    MSHTML::IHTMLInputTextElementPtr    spInputText;
    MSHTML::IHTMLElement2Ptr    spHTMLElement2;
    CFileStatus stFileStatus;
    MSHTML::IHTMLButtonElementPtr  spHTMLButtonElement;
    UNREFERENCED_PARAMETER(pElement);
    sFullFilePath += _T('\\');
    sFullFilePath += SERVICE_FILE_NAME;
    hr = GetElement(
        _T("executable_path"),
        (IHTMLElement**)&spIHTMLElement
        );
    hr = spIHTMLElement->QueryInterface(&spInputText);
    hr = spIHTMLElement->QueryInterface(&spHTMLElement2);
    bRet = CFile::GetStatus(
        sFullFilePath,
        stFileStatus
        );
    if(!bRet)
    {
        INT_PTR nDialogRet;
        CRTSDDMessageBox    oRTSDDMessageBox(this);
        oRTSDDMessageBox.m_sMessageText = _T("Исполняемый файл сервиса не найден в указанном каталоге!!");
        nDialogRet = oRTSDDMessageBox.DoModal();
        hr = spHTMLElement2->focus();
        hr = spInputText->select();
        return S_OK;
    }
    schSCManager = OpenSCManager(
        NULL,                 // local machine
        NULL,                 // ServicesActive database
        SC_MANAGER_ALL_ACCESS // full access rights
        );
    if(NULL == schSCManager) {
        nErrorCode = ::GetLastError();
        INT_PTR nDialogRet;
        CRTSDDMessageBox    oRTSDDMessageBox(this);
        oRTSDDMessageBox.m_sMessageText.Format(
            _T("\"OpenSCManager\" failed!! Error code = %d"),
            nErrorCode
            );
        nDialogRet = oRTSDDMessageBox.DoModal();
        return S_OK;
    }
    schService = OpenService(
        schSCManager,
        _T("NATTraversalService"),
        SERVICE_QUERY_CONFIG
        );
    if(NULL != schService) {
        INT_PTR nDialogRet;
        CRTSDDMessageBox    oRTSDDMessageBox(this);
        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);
        oRTSDDMessageBox.m_sMessageText = _T("Сервис \"NATTraversalService\" уже инсталирован!");
        oRTSDDMessageBox.m_sMessageTypeImageSRC = _T("WARNING_ICON.PNG");
        nDialogRet = oRTSDDMessageBox.DoModal();
        hr = GetElement(
            _T("setup_button"),
            (IHTMLElement**)&spIHTMLElement
            );
        hr = spIHTMLElement->QueryInterface(&spHTMLButtonElement);
        spHTMLButtonElement->disabled = VARIANT_TRUE;
        m_sSetupImageSRC = _T("SETUP_HIDEN.GIF");
        hr = GetElement(
            _T("uninstall_button"),
            (IHTMLElement**)&spIHTMLElement
            );
        hr = spIHTMLElement->QueryInterface(&spHTMLButtonElement);
        spHTMLButtonElement->disabled = VARIANT_FALSE;
        m_sUninstallImageSRC = _T("UNINSTALL32.PNG");
        hr = GetElement(
            _T("restart_button"),
            (IHTMLElement**)&spIHTMLElement
            );
        hr = spIHTMLElement->QueryInterface(&spHTMLButtonElement);
        spHTMLButtonElement->disabled = VARIANT_FALSE;
        m_sRestartImageSRC = _T("RESTARTHS.PNG");
        bRet = UpdateData(FALSE);
        return S_OK;
    }
    nErrorCode = ::GetLastError();
    if(ERROR_SERVICE_DOES_NOT_EXIST != nErrorCode) {
        INT_PTR nDialogRet;
        CRTSDDMessageBox    oRTSDDMessageBox(this);
        CloseServiceHandle(schSCManager);
        oRTSDDMessageBox.m_sMessageText.Format(
            _T("\"OpenService\" failed!! Error code = %d"),
            nErrorCode
            );
        nDialogRet = oRTSDDMessageBox.DoModal();
        return S_OK;
    }
    dwStartType = SERVICE_AUTO_START;
    schService = CreateService(
        schSCManager,                    // SCManager database
        _T("NATTraversalService"),       // name of service
        _T("NATTraversalService"),       // service name to display
        SERVICE_ALL_ACCESS,              // desired access
        SERVICE_WIN32_OWN_PROCESS,       // service type
        dwStartType,                     // start type
        SERVICE_ERROR_NORMAL,            // error control type
        sFullFilePath,                   // path to service's binary
        NULL,                            // no load ordering group
        NULL,                            // no tag identifier
        _T("nattraverse_kmdffilter\0"),  // завсимость от драйвера "nattraverse_kmdffilter"
        NULL,                            // LocalSystem account
        NULL                             // no password
        );
    if(NULL == schService) {
        nErrorCode = ::GetLastError();
        INT_PTR nDialogRet;
        CRTSDDMessageBox    oRTSDDMessageBox(this);
        CloseServiceHandle(schSCManager);
        oRTSDDMessageBox.m_sMessageText.Format(
            _T("\"CreateService\" failed!! Error code = %d"),
            nErrorCode
            );
        nDialogRet = oRTSDDMessageBox.DoModal();
        return S_OK;
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
        (LPBYTE)((LPCTSTR)sFullFilePath),
        (DWORD)(lstrlen((LPCTSTR)sFullFilePath)+1)*sizeof(TCHAR)
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
    {
        INT_PTR nDialogRet;
        CRTSDDMessageBox    oRTSDDMessageBox(this);
        oRTSDDMessageBox.m_sMessageText = _T("Сервис \"NATTraversalService\" успешно инсталирован.");
        oRTSDDMessageBox.m_sMessageTypeImageSRC = _T("INFORMATION_ICON.PNG");
        nDialogRet = oRTSDDMessageBox.DoModal();
        hr = GetElement(
            _T("setup_button"),
            (IHTMLElement**)&spIHTMLElement
            );
        hr = spIHTMLElement->QueryInterface(&spHTMLButtonElement);
        spHTMLButtonElement->disabled = VARIANT_TRUE;
        m_sSetupImageSRC = _T("SETUP_HIDEN.GIF");
        hr = GetElement(
            _T("uninstall_button"),
            (IHTMLElement**)&spIHTMLElement
            );
        hr = spIHTMLElement->QueryInterface(&spHTMLButtonElement);
        spHTMLButtonElement->disabled = VARIANT_FALSE;
        m_sUninstallImageSRC = _T("UNINSTALL32.PNG");
        hr = GetElement(
            _T("restart_button"),
            (IHTMLElement**)&spIHTMLElement
            );
        hr = spIHTMLElement->QueryInterface(&spHTMLButtonElement);
        spHTMLButtonElement->disabled = VARIANT_FALSE;
        m_sRestartImageSRC = _T("RESTARTHS.PNG");
        bRet = UpdateData(FALSE);
    }
    bRet = StartService(
        schService,
        0,
        NULL
        );
    if(!bRet)
    {
        nErrorCode = ::GetLastError();
        INT_PTR nDialogRet;
        CRTSDDMessageBox    oRTSDDMessageBox(this);
        oRTSDDMessageBox.m_sMessageText.Format(
            _T("Неудачная попытка запуска сервиса! Код ошибки = %u (0x%08lX)"),
            nErrorCode,
            nErrorCode
            );
        oRTSDDMessageBox.m_sMessageTypeImageSRC = _T("WARNING_ICON.PNG");
        nDialogRet = oRTSDDMessageBox.DoModal();
    }
    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
    return S_OK;
}


HRESULT CNATTraversalConfigDlg::OnExitClick(IHTMLElement* pElement)
{
    UNREFERENCED_PARAMETER(pElement);
    if(!m_bNoBreakStage) {
        DestroyModeless( );
    }
    return S_OK;
}


HRESULT CNATTraversalConfigDlg::DoAboutBox(IHTMLElement* pElement)
{
    CAboutBox oAboutBox(this);
    INT_PTR nRet;
    UNREFERENCED_PARAMETER(pElement);
    nRet = oAboutBox.DoModal();
    return S_OK;
}


void CNATTraversalConfigDlg::OnDestroy()
{
    WINDOWPLACEMENT strcWndPl;
    if(!GetWindowPlacement( &strcWndPl )) {
        CDHtmlDialog::OnDestroy( );
        return;
    }
    if(strcWndPl.showCmd == SW_SHOWMAXIMIZED) {
        theApp.WriteProfileInt(
            _T("WindowPlacement"),
            _T("showMaximized"),
            1
            );
    } else {
        theApp.WriteProfileInt(
            _T("WindowPlacement"),
            _T("showMaximized"),
            0
            );
    }
    theApp.WriteProfileInt(
        _T("WindowPlacement"),
        _T("leftNormal"),
        strcWndPl.rcNormalPosition.left
        );
    theApp.WriteProfileInt(
        _T("WindowPlacement"),
        _T("topNormal"),
        strcWndPl.rcNormalPosition.top
        );
    theApp.WriteProfileInt(
        _T("WindowPlacement"),
        _T("rightNormal"),
        strcWndPl.rcNormalPosition.right
        );
    theApp.WriteProfileInt(
        _T("WindowPlacement"),
        _T("bottomNormal"),
        strcWndPl.rcNormalPosition.bottom
        );
    CDHtmlDialog::OnDestroy();
}

STDMETHODIMP CNATTraversalConfigDlg::ShowContextMenu(
    DWORD dwID,
    POINT* ppt,
    IUnknown* pcmdtReserved,
    IDispatch* pdispReserved
    )
{
    UNREFERENCED_PARAMETER(ppt);
    UNREFERENCED_PARAMETER(pcmdtReserved);
    UNREFERENCED_PARAMETER(pdispReserved);
    if(dwID == CONTEXT_MENU_CONTROL) {
        return S_FALSE;
    }
    return S_OK;
}

STDMETHODIMP CNATTraversalConfigDlg::GetHostInfo(
    DOCHOSTUIINFO *pInfo
    )
{
	pInfo->dwFlags = DOCHOSTUIFLAG_NO3DBORDER | DOCHOSTUIFLAG_DIALOG;
    pInfo->dwDoubleClick = DOCHOSTUIDBLCLK_DEFAULT;
    return S_OK;
}


HRESULT CNATTraversalConfigDlg::SelectServicePath(IHTMLElement* pElement)
{
    BOOL bRet = UpdateData(TRUE);
    INT_PTR nDialogRet;
    UNREFERENCED_PARAMETER(pElement);
    CFolderPickerDialog oFolderDialog(
        NULL,
        0,
        this,
        0
        );
    oFolderDialog.GetOFN().lpstrTitle = _T("Выбор папки, содержащий исполняемый файл сервиса");
    oFolderDialog.GetOFN().lpstrInitialDir = m_sServicesFolder;
    nDialogRet = oFolderDialog.DoModal();
    if(nDialogRet == IDOK)
    {
        m_sServicesFolder = oFolderDialog.GetPathName();
        bRet = UpdateData(FALSE);
    }
    return S_OK;
}


HRESULT CNATTraversalConfigDlg::TestRouteIP(IHTMLElement* pElement)
{
    IN_ADDR stIP4Address;
    LONG    nRet;
    HRESULT hr;
    LPCTSTR pTerminator;
    BOOL    bRet = FALSE;
    bRet = UpdateData(TRUE);
    MSHTML::IHTMLElementPtr     spPublicRouterIP;
    MSHTML::IHTMLElement2Ptr    spPublicRouterIPElement2;
    MSHTML::IHTMLInputElementPtr    spPublicRouterIPInput;
    bRet = UpdateData(TRUE);
    PMIB_IPADDRTABLE pIPAddrTable;
    DWORD dwSize = sizeof(MIB_IPADDRTABLE);
    DWORD dwRetVal = 0;
    PBYTE   pByteBuffer = NULL;
    int i;
    bool    bIPExist = false;
    UNREFERENCED_PARAMETER(pElement);
    hr = GetElement(
        _T("public_router_ip"),
        (IHTMLElement**)&spPublicRouterIP
        );
    hr = spPublicRouterIP->QueryInterface(&spPublicRouterIPElement2);
    hr = spPublicRouterIP->QueryInterface(&spPublicRouterIPInput);
    nRet = RtlIpv4StringToAddress(
        m_sPublicRouterIP,
        TRUE,
        &pTerminator,
        &stIP4Address
        );
    if(NO_ERROR != nRet)
    {
        INT_PTR nDialogRet;
        CRTSDDMessageBox    oRTSDDMessageBox(this);
        oRTSDDMessageBox.m_sMessageText = _T("Ошибка формата IP4-адреса!");
        nDialogRet = oRTSDDMessageBox.DoModal();
        hr = spPublicRouterIPElement2->focus();
        hr = spPublicRouterIPInput->select();
        return S_OK;
    }
    pIPAddrTable = new MIB_IPADDRTABLE;
    dwRetVal = GetIpAddrTable(
        pIPAddrTable,
        &dwSize,
        FALSE
        );
    if(ERROR_INSUFFICIENT_BUFFER == dwRetVal)
    {
        delete pIPAddrTable;
        pByteBuffer = new BYTE[dwSize];
        pIPAddrTable = (MIB_IPADDRTABLE *)pByteBuffer;
    }
    dwRetVal = GetIpAddrTable(
        pIPAddrTable,
        &dwSize,
        FALSE
        );
    for(i=0;i<(int)pIPAddrTable->dwNumEntries;i++)
    {
        if(pIPAddrTable->table[i].dwAddr == (DWORD)stIP4Address.S_un.S_addr)
        {
            bIPExist = true;
            break;
        }
    }
    if(NULL == pByteBuffer)
    {
        delete pIPAddrTable;
        pIPAddrTable = NULL;
    }
    else
    {
        delete [] pByteBuffer;
        pIPAddrTable = NULL;
        pByteBuffer = NULL;
    }
    if(!bIPExist)
    {
        INT_PTR nDialogRet;
        CRTSDDMessageBox    oRTSDDMessageBox(this);
        oRTSDDMessageBox.m_sMessageText = _T("Публичный интерфейс маршрутизатора с введенным IP-адресом не существет или отключен!");
        oRTSDDMessageBox.m_sMessageTypeImageSRC = _T("WARNING_ICON.PNG");
        nDialogRet = oRTSDDMessageBox.DoModal();
        hr = spPublicRouterIPElement2->focus();
        hr = spPublicRouterIPInput->select();
    }
    else
    {
        INT_PTR nDialogRet;
        CRTSDDMessageBox    oRTSDDMessageBox(this);
        oRTSDDMessageBox.m_sMessageText = _T("Публичный интерфейс маршрутизатора с введенным IP-адресом существет и подключён.");
        oRTSDDMessageBox.m_sMessageTypeImageSRC = _T("INFORMATION_ICON.PNG");
        nDialogRet = oRTSDDMessageBox.DoModal();
    }
    return S_OK;
}


HRESULT CNATTraversalConfigDlg::InsertPublicIP(IHTMLElement* pElement)
{
    HRESULT hr;
    MSHTML::IHTMLElementPtr     spIHTMLElement;
    MSHTML::IHTMLElement2Ptr    spIHTMLElement2;
    MSHTML::IHTMLTableRowPtr    spPublicIPTemplateRow;
    long    nRowIndex;
    IDispatchPtr    spDispatch;
    MSHTML::IHTMLTablePtr       spSettingsTable;
    _bstr_t sCellHTML;
    MSHTML::IHTMLTableRowPtr    spNewRow;
    MSHTML::IHTMLElementCollectionPtr   spHTMLElementCollection;
    MSHTML::IHTMLStylePtr       spStyle;
    MSHTML::IHTMLTableCellPtr   spNewCell;
    UNREFERENCED_PARAMETER(pElement);
    hr = GetElement(
        _T("public_ip_row_template"),
        (IHTMLElement**)&spIHTMLElement
        );
    hr = spIHTMLElement->QueryInterface(&spPublicIPTemplateRow);
    nRowIndex = spPublicIPTemplateRow->rowIndex;
    hr = GetElement(
        _T("settings_table"),
        (IHTMLElement**)&spIHTMLElement
        );
    hr = spIHTMLElement->QueryInterface(&spSettingsTable);
    spDispatch = spSettingsTable->insertRow(nRowIndex);
    hr = spDispatch->QueryInterface(&spNewRow);
    hr = GetElement(
        _T("public_ip_description"),
        (IHTMLElement**)&spIHTMLElement
        );
    sCellHTML = spIHTMLElement->innerHTML;
    spDispatch = spNewRow->insertCell(-1);
    hr = spDispatch->QueryInterface(&spNewCell);
    spNewCell->align = _bstr_t(L"right");
    spNewCell->width = _variant_t(L"50%");
    hr = spDispatch->QueryInterface(&spIHTMLElement);
    spIHTMLElement->innerHTML = sCellHTML;
    hr = GetElement(
        _T("public_ip_cell"),
        (IHTMLElement**)&spIHTMLElement
        );
    sCellHTML = spIHTMLElement->innerHTML;
    spDispatch = spNewRow->insertCell(-1);
    hr = spDispatch->QueryInterface(&spIHTMLElement);
    spIHTMLElement->innerHTML = sCellHTML;
    spDispatch = spIHTMLElement->children;
    hr = spDispatch->QueryInterface(&spHTMLElementCollection);
    spDispatch = spHTMLElementCollection->item(_variant_t(L"insert_public_ip"));
    hr = spDispatch->QueryInterface(&spIHTMLElement);
    spStyle = spIHTMLElement->style;
    spStyle->visibility = _bstr_t(L"hidden");
    spDispatch = spHTMLElementCollection->item(_variant_t(L"delete_public_ip"));
    hr = spDispatch->QueryInterface(&spIHTMLElement);
    spStyle = spIHTMLElement->style;
    spStyle->visibility = _bstr_t(L"visible");
    spDispatch = spHTMLElementCollection->item(_variant_t(L"public_ip"));
    hr = spDispatch->QueryInterface(&spIHTMLElement);
    spStyle = spIHTMLElement->style;
    spStyle->visibility = _bstr_t(L"visible");
    hr = spDispatch->QueryInterface(&spIHTMLElement2);
    hr = spIHTMLElement2->focus();
    spDispatch = spHTMLElementCollection->item(_variant_t(L"test_public_ip_button"));
    hr = spDispatch->QueryInterface(&spIHTMLElement);
    spStyle = spIHTMLElement->style;
    spStyle->visibility = _bstr_t(L"visible");
    return S_OK;
}

HRESULT CNATTraversalConfigDlg::DeletePublicIP(IHTMLElement* pElement)
{
    HRESULT hr;
    MSHTML::IHTMLElementPtr     spIHTMLElement;
    MSHTML::IHTMLTableRowPtr    spTableRow;
    long    nRowIndex;
    MSHTML::IHTMLTablePtr       spSettingsTable;
    spIHTMLElement = pElement;
    spIHTMLElement = spIHTMLElement->parentElement;
    spIHTMLElement = spIHTMLElement->parentElement;
    hr = spIHTMLElement->QueryInterface(&spTableRow);
    nRowIndex = spTableRow->rowIndex;
    hr = GetElement(
        _T("settings_table"),
        (IHTMLElement**)&spIHTMLElement
        );
    hr = spIHTMLElement->QueryInterface(&spSettingsTable);
    hr = spSettingsTable->deleteRow(nRowIndex);
    return S_OK;
}

HRESULT CNATTraversalConfigDlg::TestPublicIP(IHTMLElement* pElement)
{
    HRESULT hr;
    MSHTML::IHTMLElementPtr spIHTMLElement;
    MSHTML::IHTMLTablePtr   spSettingsTable;
    CString sIPAddress;
    IDispatchPtr    spDispatch;
    MSHTML::IHTMLElementCollectionPtr   spHTMLElementCollection;
    MSHTML::IHTMLInputTextElementPtr    spInputText;
    LONG    nRet;
    LPCTSTR pTerminator;
    IN_ADDR stIP4Address;
    MSHTML::IHTMLElement2Ptr    spPublicIPElement2;
    spIHTMLElement = pElement;
    spIHTMLElement = spIHTMLElement->parentElement;
    spDispatch = spIHTMLElement->children;
    hr = spDispatch->QueryInterface(&spHTMLElementCollection);
    spDispatch = spHTMLElementCollection->item(_variant_t(L"public_ip"));
    hr = spDispatch->QueryInterface(&spInputText);
    hr = spDispatch->QueryInterface(&spPublicIPElement2);
    sIPAddress = (LPCTSTR)spInputText->value;
    nRet = RtlIpv4StringToAddress(
        sIPAddress,
        TRUE,
        &pTerminator,
        &stIP4Address
        );
    if(NO_ERROR != nRet)
    {
        INT_PTR nDialogRet;
        CRTSDDMessageBox    oRTSDDMessageBox(this);
        oRTSDDMessageBox.m_sMessageText = _T("Ошибка формата IP-адреса!!");
        nDialogRet = oRTSDDMessageBox.DoModal();
        hr = spPublicIPElement2->focus();
        hr = spInputText->select();
    }
    else
    {
        INT_PTR nDialogRet;
        CRTSDDMessageBox    oRTSDDMessageBox(this);
        oRTSDDMessageBox.m_sMessageText = _T("Формат публичного IP-адреса верен.");
        oRTSDDMessageBox.m_sMessageTypeImageSRC = _T("INFORMATION_ICON.PNG");
        nDialogRet = oRTSDDMessageBox.DoModal();
    }
    return S_OK;
}


HRESULT CNATTraversalConfigDlg::InsertPublicRange(IHTMLElement* pElement)
{
    HRESULT hr;
    MSHTML::IHTMLElementPtr     spIHTMLElement;
    MSHTML::IHTMLElement2Ptr    spIHTMLElement2;
    MSHTML::IHTMLTableRowPtr    spPublicIPTemplateRow;
    long    nRowIndex;
    IDispatchPtr    spDispatch;
    MSHTML::IHTMLTablePtr       spSettingsTable;
    _bstr_t sCellHTML;
    MSHTML::IHTMLTableRowPtr    spNewRow;
    MSHTML::IHTMLElementCollectionPtr   spHTMLElementCollection;
    MSHTML::IHTMLStylePtr       spStyle;
    MSHTML::IHTMLTableCellPtr   spNewCell;
    UNREFERENCED_PARAMETER(pElement);
    hr = GetElement(
        _T("public_range_row_template"),
        (IHTMLElement**)&spIHTMLElement
        );
    hr = spIHTMLElement->QueryInterface(&spPublicIPTemplateRow);
    nRowIndex = spPublicIPTemplateRow->rowIndex;
    hr = GetElement(
        _T("settings_table"),
        (IHTMLElement**)&spIHTMLElement
        );
    hr = spIHTMLElement->QueryInterface(&spSettingsTable);
    spDispatch = spSettingsTable->insertRow(nRowIndex);
    hr = spDispatch->QueryInterface(&spNewRow);
    hr = GetElement(
        _T("public_range_description"),
        (IHTMLElement**)&spIHTMLElement
        );
    sCellHTML = spIHTMLElement->innerHTML;
    spDispatch = spNewRow->insertCell(-1);
    hr = spDispatch->QueryInterface(&spNewCell);
    spNewCell->align = _bstr_t(L"right");
    spNewCell->width = _variant_t(L"50%");
    hr = spDispatch->QueryInterface(&spIHTMLElement);
    spIHTMLElement->innerHTML = sCellHTML;
    hr = GetElement(
        _T("public_range_cell"),
        (IHTMLElement**)&spIHTMLElement
        );
    sCellHTML = spIHTMLElement->innerHTML;
    spDispatch = spNewRow->insertCell(-1);
    hr = spDispatch->QueryInterface(&spIHTMLElement);
    spIHTMLElement->innerHTML = sCellHTML;
    spDispatch = spIHTMLElement->children;
    hr = spDispatch->QueryInterface(&spHTMLElementCollection);
    spDispatch = spHTMLElementCollection->item(_variant_t(L"insert_public_range"));
    hr = spDispatch->QueryInterface(&spIHTMLElement);
    spStyle = spIHTMLElement->style;
    spStyle->visibility = _bstr_t(L"hidden");
    spDispatch = spHTMLElementCollection->item(_variant_t(L"delete_public_range"));
    hr = spDispatch->QueryInterface(&spIHTMLElement);
    spStyle = spIHTMLElement->style;
    spStyle->visibility = _bstr_t(L"visible");
    spDispatch = spHTMLElementCollection->item(_variant_t(L"range_begin"));
    hr = spDispatch->QueryInterface(&spIHTMLElement);
    spStyle = spIHTMLElement->style;
    spStyle->visibility = _bstr_t(L"visible");
    hr = spDispatch->QueryInterface(&spIHTMLElement2);
    hr = spIHTMLElement2->focus();
    spDispatch = spHTMLElementCollection->item(_variant_t(L"range_symbol"));
    hr = spDispatch->QueryInterface(&spIHTMLElement);
    spStyle = spIHTMLElement->style;
    spStyle->visibility = _bstr_t(L"visible");
    spDispatch = spHTMLElementCollection->item(_variant_t(L"range_end"));
    hr = spDispatch->QueryInterface(&spIHTMLElement);
    spStyle = spIHTMLElement->style;
    spStyle->visibility = _bstr_t(L"visible");
    spDispatch = spHTMLElementCollection->item(_variant_t(L"test_public_range_button"));
    hr = spDispatch->QueryInterface(&spIHTMLElement);
    spStyle = spIHTMLElement->style;
    spStyle->visibility = _bstr_t(L"visible");
    return S_OK;
}

HRESULT CNATTraversalConfigDlg::DeletePublicRange(IHTMLElement* pElement)
{
    HRESULT hr;
    MSHTML::IHTMLElementPtr spIHTMLElement;
    MSHTML::IHTMLTableRowPtr    spTableRow;
    long    nRowIndex;
    MSHTML::IHTMLTablePtr   spSettingsTable;
    spIHTMLElement = pElement;
    spIHTMLElement = spIHTMLElement->parentElement;
    spIHTMLElement = spIHTMLElement->parentElement;
    hr = spIHTMLElement->QueryInterface(&spTableRow);
    nRowIndex = spTableRow->rowIndex;
    hr = GetElement(
        _T("settings_table"),
        (IHTMLElement**)&spIHTMLElement
        );
    hr = spIHTMLElement->QueryInterface(&spSettingsTable);
    hr = spSettingsTable->deleteRow(nRowIndex);
    return S_OK;
}


HRESULT CNATTraversalConfigDlg::TestPublicRange(IHTMLElement* pElement)
{
    HRESULT hr;
    MSHTML::IHTMLElementPtr spIHTMLElement;
    MSHTML::IHTMLTablePtr   spSettingsTable;
    CString sIPAddress;
    IDispatchPtr    spDispatch;
    MSHTML::IHTMLElementCollectionPtr   spHTMLElementCollection;
    MSHTML::IHTMLInputTextElementPtr    spInputText;
    LONG    nRet;
    LPCTSTR pTerminator;
    IN_ADDR stIP4Address;
    MSHTML::IHTMLElement2Ptr    spPublicIPElement2;
    u_long  nhBeginAddress, nhEndAddress;
    spIHTMLElement = pElement;
    spIHTMLElement = spIHTMLElement->parentElement;
    spDispatch = spIHTMLElement->children;
    hr = spDispatch->QueryInterface(&spHTMLElementCollection);
    spDispatch = spHTMLElementCollection->item(_variant_t(L"range_begin"));
    hr = spDispatch->QueryInterface(&spInputText);
    hr = spDispatch->QueryInterface(&spPublicIPElement2);
    sIPAddress = (LPCTSTR)spInputText->value;
    nRet = RtlIpv4StringToAddress(
        sIPAddress,
        TRUE,
        &pTerminator,
        &stIP4Address
        );
    if(NO_ERROR != nRet)
    {
        INT_PTR nDialogRet;
        CRTSDDMessageBox    oRTSDDMessageBox(this);
        oRTSDDMessageBox.m_sMessageText = _T("Ошибка формата начального значения диапазона публичных IP-адресов!!");
        nDialogRet = oRTSDDMessageBox.DoModal();
        hr = spPublicIPElement2->focus();
        hr = spInputText->select();
        return S_OK;
    }
    nhBeginAddress = ntohl(stIP4Address.S_un.S_addr);
    spDispatch = spHTMLElementCollection->item(_variant_t(L"range_end"));
    hr = spDispatch->QueryInterface(&spInputText);
    hr = spDispatch->QueryInterface(&spPublicIPElement2);
    sIPAddress = (LPCTSTR)spInputText->value;
    nRet = RtlIpv4StringToAddress(
        sIPAddress,
        TRUE,
        &pTerminator,
        &stIP4Address
        );
    if(NO_ERROR != nRet)
    {
        INT_PTR nDialogRet;
        CRTSDDMessageBox    oRTSDDMessageBox(this);
        oRTSDDMessageBox.m_sMessageText = _T("Ошибка формата конечного значения диапазона публичных IP-адресов!!");
        nDialogRet = oRTSDDMessageBox.DoModal();
        hr = spPublicIPElement2->focus();
        hr = spInputText->select();
        return S_OK;
    }
    nhEndAddress = ntohl(stIP4Address.S_un.S_addr);
    if(nhBeginAddress > nhEndAddress)
    {
        INT_PTR nDialogRet;
        CRTSDDMessageBox    oRTSDDMessageBox(this);
        oRTSDDMessageBox.m_sMessageText = _T("Значение начального адреса диапазона больше конечного!!");
        nDialogRet = oRTSDDMessageBox.DoModal();
        hr = spPublicIPElement2->focus();
        hr = spInputText->select();
        return S_OK;
    }
    if(nhBeginAddress == nhEndAddress)
    {
        INT_PTR nDialogRet;
        CRTSDDMessageBox    oRTSDDMessageBox(this);
        oRTSDDMessageBox.m_sMessageText = _T("Значение начального адреса диапазона равно конечному, т.е задан единственный IP-адрес!");
        oRTSDDMessageBox.m_sMessageTypeImageSRC = _T("WARNING_ICON.PNG");
        nDialogRet = oRTSDDMessageBox.DoModal();
        hr = spPublicIPElement2->focus();
        hr = spInputText->select();
    }
    else
    {
        INT_PTR nDialogRet;
        CRTSDDMessageBox    oRTSDDMessageBox(this);
        oRTSDDMessageBox.m_sMessageText = _T("Формат диапазона публичных IP-адресов верен.");
        oRTSDDMessageBox.m_sMessageTypeImageSRC = _T("INFORMATION_ICON.PNG");
        nDialogRet = oRTSDDMessageBox.DoModal();
    }
    return S_OK;
}


HRESULT CNATTraversalConfigDlg::InsertPublicSubnet(IHTMLElement* pElement)
{
    HRESULT hr;
    MSHTML::IHTMLElementPtr     spIHTMLElement;
    MSHTML::IHTMLElement2Ptr    spIHTMLElement2;
    MSHTML::IHTMLTableRowPtr    spPublicIPTemplateRow;
    long    nRowIndex;
    IDispatchPtr    spDispatch;
    MSHTML::IHTMLTablePtr       spSettingsTable;
    _bstr_t sCellHTML;
    MSHTML::IHTMLTableRowPtr    spNewRow;
    MSHTML::IHTMLElementCollectionPtr   spHTMLElementCollection;
    MSHTML::IHTMLStylePtr       spStyle;
    MSHTML::IHTMLTableCellPtr   spNewCell;
    UNREFERENCED_PARAMETER(pElement);
    hr = GetElement(
        _T("public_subnet_row_template"),
        (IHTMLElement**)&spIHTMLElement
        );
    hr = spIHTMLElement->QueryInterface(&spPublicIPTemplateRow);
    nRowIndex = spPublicIPTemplateRow->rowIndex;
    hr = GetElement(
        _T("settings_table"),
        (IHTMLElement**)&spIHTMLElement
        );
    hr = spIHTMLElement->QueryInterface(&spSettingsTable);
    spDispatch = spSettingsTable->insertRow(nRowIndex);
    hr = spDispatch->QueryInterface(&spNewRow);
    hr = GetElement(
        _T("public_subnet_description"),
        (IHTMLElement**)&spIHTMLElement
        );
    sCellHTML = spIHTMLElement->innerHTML;
    spDispatch = spNewRow->insertCell(-1);
    hr = spDispatch->QueryInterface(&spNewCell);
    spNewCell->align = _bstr_t(L"right");
    spNewCell->width = _variant_t(L"50%");
    hr = spDispatch->QueryInterface(&spIHTMLElement);
    spIHTMLElement->innerHTML = sCellHTML;
    hr = GetElement(
        _T("public_subnet_cell"),
        (IHTMLElement**)&spIHTMLElement
        );
    sCellHTML = spIHTMLElement->innerHTML;
    spDispatch = spNewRow->insertCell(-1);
    hr = spDispatch->QueryInterface(&spIHTMLElement);
    spIHTMLElement->innerHTML = sCellHTML;
    spDispatch = spIHTMLElement->children;
    hr = spDispatch->QueryInterface(&spHTMLElementCollection);
    spDispatch = spHTMLElementCollection->item(_variant_t(L"insert_public_subnet"));
    hr = spDispatch->QueryInterface(&spIHTMLElement);
    spStyle = spIHTMLElement->style;
    spStyle->visibility = _bstr_t(L"hidden");
    spDispatch = spHTMLElementCollection->item(_variant_t(L"delete_public_subnet"));
    hr = spDispatch->QueryInterface(&spIHTMLElement);
    spStyle = spIHTMLElement->style;
    spStyle->visibility = _bstr_t(L"visible");
    spDispatch = spHTMLElementCollection->item(_variant_t(L"subnet_address"));
    hr = spDispatch->QueryInterface(&spIHTMLElement);
    spStyle = spIHTMLElement->style;
    spStyle->visibility = _bstr_t(L"visible");
    hr = spDispatch->QueryInterface(&spIHTMLElement2);
    hr = spIHTMLElement2->focus();
    spDispatch = spHTMLElementCollection->item(_variant_t(L"subnet_symbol"));
    hr = spDispatch->QueryInterface(&spIHTMLElement);
    spStyle = spIHTMLElement->style;
    spStyle->visibility = _bstr_t(L"visible");
    spDispatch = spHTMLElementCollection->item(_variant_t(L"subnet_mask"));
    hr = spDispatch->QueryInterface(&spIHTMLElement);
    spStyle = spIHTMLElement->style;
    spStyle->visibility = _bstr_t(L"visible");
    spDispatch = spHTMLElementCollection->item(_variant_t(L"test_public_subnet_button"));
    hr = spDispatch->QueryInterface(&spIHTMLElement);
    spStyle = spIHTMLElement->style;
    spStyle->visibility = _bstr_t(L"visible");
    return S_OK;
}


HRESULT CNATTraversalConfigDlg::DeletePublicSubnet(IHTMLElement* pElement)
{
    HRESULT hr;
    MSHTML::IHTMLElementPtr spIHTMLElement;
    MSHTML::IHTMLTableRowPtr    spTableRow;
    long    nRowIndex;
    MSHTML::IHTMLTablePtr   spSettingsTable;
    spIHTMLElement = pElement;
    spIHTMLElement = spIHTMLElement->parentElement;
    spIHTMLElement = spIHTMLElement->parentElement;
    hr = spIHTMLElement->QueryInterface(&spTableRow);
    nRowIndex = spTableRow->rowIndex;
    hr = GetElement(
        _T("settings_table"),
        (IHTMLElement**)&spIHTMLElement
        );
    hr = spIHTMLElement->QueryInterface(&spSettingsTable);
    hr = spSettingsTable->deleteRow(nRowIndex);
    return S_OK;
}


HRESULT CNATTraversalConfigDlg::TestPublicSubnet(IHTMLElement* pElement)
{
    HRESULT hr;
    MSHTML::IHTMLElementPtr spIHTMLElement;
    MSHTML::IHTMLTablePtr   spSettingsTable;
    CString sIPAddress;
    IDispatchPtr    spDispatch;
    MSHTML::IHTMLElementCollectionPtr   spHTMLElementCollection;
    MSHTML::IHTMLInputTextElementPtr    spInputText;
    LONG    nRet;
    LPCTSTR pTerminator;
    IN_ADDR stIP4Address;
    MSHTML::IHTMLElement2Ptr    spPublicIPElement2;
    spIHTMLElement = pElement;
    spIHTMLElement = spIHTMLElement->parentElement;
    spDispatch = spIHTMLElement->children;
    hr = spDispatch->QueryInterface(&spHTMLElementCollection);
    spDispatch = spHTMLElementCollection->item(_variant_t(L"subnet_address"));
    hr = spDispatch->QueryInterface(&spInputText);
    hr = spDispatch->QueryInterface(&spPublicIPElement2);
    sIPAddress = (LPCTSTR)spInputText->value;
    nRet = RtlIpv4StringToAddress(
        sIPAddress,
        TRUE,
        &pTerminator,
        &stIP4Address
        );
    if(NO_ERROR != nRet)
    {
        INT_PTR nDialogRet;
        CRTSDDMessageBox    oRTSDDMessageBox(this);
        oRTSDDMessageBox.m_sMessageText = _T("Ошибка формата адреса публичной IP-подсети!!");
        nDialogRet = oRTSDDMessageBox.DoModal();
        hr = spPublicIPElement2->focus();
        hr = spInputText->select();
        return S_OK;
    }
    spDispatch = spHTMLElementCollection->item(_variant_t(L"subnet_mask"));
    hr = spDispatch->QueryInterface(&spInputText);
    hr = spDispatch->QueryInterface(&spPublicIPElement2);
    sIPAddress = (LPCTSTR)spInputText->value;
    nRet = RtlIpv4StringToAddress(
        sIPAddress,
        TRUE,
        &pTerminator,
        &stIP4Address
        );
    if(NO_ERROR != nRet)
    {
        INT_PTR nDialogRet;
        CRTSDDMessageBox    oRTSDDMessageBox(this);
        oRTSDDMessageBox.m_sMessageText = _T("Ошибка формата маски публичной IP-подсети!!");
        nDialogRet = oRTSDDMessageBox.DoModal();
        hr = spPublicIPElement2->focus();
        hr = spInputText->select();
    }
    else
    {
        INT_PTR nDialogRet;
        CRTSDDMessageBox    oRTSDDMessageBox(this);
        oRTSDDMessageBox.m_sMessageText = _T("Формат публичной IP-подсети верен.");
        oRTSDDMessageBox.m_sMessageTypeImageSRC = _T("INFORMATION_ICON.PNG");
        nDialogRet = oRTSDDMessageBox.DoModal();
    }
    return S_OK;
}


HRESULT CNATTraversalConfigDlg::SaveParameters(IHTMLElement* pElement)
{
    CFileStatus stFileStatus;
    BOOL bRet = UpdateData(TRUE);
    CString sFullFilePath(m_sServicesFolder);
    HRESULT hr;
    MSHTML::IHTMLElementPtr     spIHTMLElement;
    MSHTML::IHTMLTableRowPtr    spIHTMLTableRow;
    MSHTML::IHTMLTablePtr       spSettingsTable;
    MSHTML::IHTMLElement2Ptr    spPublicIPElement2;
    MSHTML::IHTMLElementCollectionPtr   spHTMLElementCollection;
    MSHTML::IHTMLElementCollectionPtr   spCellChildrens;
    MSHTML::IHTMLInputTextElementPtr    spInputText;
    CString sIPAddress;
    IDispatchPtr    spDispatch;
    LONG    nRet;
    LPCTSTR pTerminator;
    IN_ADDR stIP4Address;
    PMIB_IPADDRTABLE pIPAddrTable;
    DWORD dwRetVal = 0;
    DWORD dwSize = sizeof(MIB_IPADDRTABLE);
    PBYTE   pByteBuffer = NULL;
    int i, nSingleIP = 0, nRangeIP = 0, nSubnetIP = 0, nSingleIpPort = 0, nRangeIpPort = 0;
    int nSinglePrivateIP = 0, nRangePrivateIP = 0, nSubnetPrivateIP = 0;
    bool    bIPExist = false;
    BOOL    bCollection = FALSE;
    u_long  nhBeginAddress, nhEndAddress;
    CString sSectionName;
    CString sIpPort;
    long    nIpPort, nIpPortBegin, nIpPortEnd;
    bool    bIpPortValid, bIpPortBeginValid, bIpPortEndValid;
    DWORD   dwInterfaceIndex = 0;
    NET_LUID        qwInterfaceLuid;
    NETIO_STATUS    dwNetIoStatus;
    GUID            stInterfaceGuid;
    UNREFERENCED_PARAMETER(pElement);
    sFullFilePath += _T('\\');
    sFullFilePath += SERVICE_FILE_NAME;
    hr = GetElement(
        _T("executable_path"),
        (IHTMLElement**)&spIHTMLElement
        );
    hr = spIHTMLElement->QueryInterface(&spInputText);
    hr = spIHTMLElement->QueryInterface(&spPublicIPElement2);
    bRet = CFile::GetStatus(
        sFullFilePath,
        stFileStatus
        );
    if(!bRet)
    {
        INT_PTR nDialogRet;
        CRTSDDMessageBox    oRTSDDMessageBox(this);
        oRTSDDMessageBox.m_sMessageText = _T("Исполняемый файл сервиса не найден в указанном каталоге!!");
        nDialogRet = oRTSDDMessageBox.DoModal();
        hr = spPublicIPElement2->focus();
        hr = spInputText->select();
        return S_OK;
    }
    hr = GetElement(
        _T("public_router_ip"),
        (IHTMLElement**)&spIHTMLElement
        );
    hr = spIHTMLElement->QueryInterface(&spPublicIPElement2);
    hr = spIHTMLElement->QueryInterface(&spInputText);
    nRet = RtlIpv4StringToAddress(
        m_sPublicRouterIP,
        TRUE,
        &pTerminator,
        &stIP4Address
        );
    if(NO_ERROR != nRet)
    {
        INT_PTR nDialogRet;
        CRTSDDMessageBox    oRTSDDMessageBox(this);
        oRTSDDMessageBox.m_sMessageText = _T("Ошибка формата публичного IP-адреса маршрутизатора!!");
        nDialogRet = oRTSDDMessageBox.DoModal();
        hr = spPublicIPElement2->focus();
        hr = spInputText->select();
        return S_OK;
    }
    pIPAddrTable = new MIB_IPADDRTABLE;
    dwRetVal = GetIpAddrTable(
        pIPAddrTable,
        &dwSize,
        FALSE
        );
    if(ERROR_INSUFFICIENT_BUFFER == dwRetVal)
    {
        delete pIPAddrTable;
        pByteBuffer = new BYTE[dwSize];
        pIPAddrTable = (MIB_IPADDRTABLE *)pByteBuffer;
    }
    dwRetVal = GetIpAddrTable(
        pIPAddrTable,
        &dwSize,
        FALSE
        );
    for(i=0;i<(int)pIPAddrTable->dwNumEntries;i++)
    {
        if(pIPAddrTable->table[i].dwAddr == (DWORD)stIP4Address.S_un.S_addr)
        {
            bIPExist = true;
            dwInterfaceIndex = pIPAddrTable->table[i].dwIndex;
            break;
        }
    }
    if(NULL == pByteBuffer)
    {
        delete pIPAddrTable;
        pIPAddrTable = NULL;
    }
    else
    {
        delete [] pByteBuffer;
        pIPAddrTable = NULL;
        pByteBuffer = NULL;
    }
    if(!bIPExist)
    {
        INT_PTR nDialogRet;
        CRTSDDMessageBox    oRTSDDMessageBox(this);
        oRTSDDMessageBox.m_sMessageText = _T("Публичный интерфейс маршрутизатора с введенным IP-адресом не существет или отключен!");
        oRTSDDMessageBox.m_sMessageTypeImageSRC = _T("WARNING_ICON.PNG");
        nDialogRet = oRTSDDMessageBox.DoModal();
        hr = spPublicIPElement2->focus();
        hr = spInputText->select();
    }
    else
    {
        dwNetIoStatus = ConvertInterfaceIndexToLuid(
            dwInterfaceIndex,
            &qwInterfaceLuid
            );
        ASSERT(STATUS_SUCCESS == dwNetIoStatus);
        dwNetIoStatus = ConvertInterfaceLuidToGuid(
            &qwInterfaceLuid,
            &stInterfaceGuid
            );
        ASSERT(STATUS_SUCCESS == dwNetIoStatus);
    }
    hr = GetElement(
        _T("public_ip"),
        &spDispatch,
        &bCollection
        );
    if(bCollection)
    {
        hr = spDispatch->QueryInterface(&spHTMLElementCollection);
        for(i=0;i<spHTMLElementCollection->length;i++)
        {
            spDispatch = spHTMLElementCollection->item(_variant_t(i));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spIHTMLElement = spIHTMLElement->parentElement;
            if(spIHTMLElement->id == _bstr_t(L"public_ip_cell"))
            {
                continue;
            }
            hr = spDispatch->QueryInterface(&spInputText);
            hr = spDispatch->QueryInterface(&spPublicIPElement2);
            sIPAddress = (LPCTSTR)spInputText->value;
            nRet = RtlIpv4StringToAddress(
                sIPAddress,
                TRUE,
                &pTerminator,
                &stIP4Address
                );
            if(NO_ERROR != nRet)
            {
                INT_PTR nDialogRet;
                CRTSDDMessageBox    oRTSDDMessageBox(this);
                oRTSDDMessageBox.m_sMessageText = _T("Ошибка формата IP-адреса!!");
                nDialogRet = oRTSDDMessageBox.DoModal();
                hr = spPublicIPElement2->focus();
                hr = spInputText->select();
                return S_OK;
            }
            nSingleIP++;
        }
    }
    hr = GetElement(
        _T("range_begin"),
        &spDispatch,
        &bCollection
        );
    if(bCollection)
    {
        hr = spDispatch->QueryInterface(&spHTMLElementCollection);
        for(i=0;i<spHTMLElementCollection->length;i++)
        {
            spDispatch = spHTMLElementCollection->item(_variant_t(i));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spIHTMLElement = spIHTMLElement->parentElement;
            if(spIHTMLElement->id == _bstr_t(L"public_range_cell"))
            {
                continue;
            }
            hr = spDispatch->QueryInterface(&spInputText);
            hr = spDispatch->QueryInterface(&spPublicIPElement2);
            spDispatch = spIHTMLElement->children;
            hr = spDispatch->QueryInterface(&spCellChildrens);
            sIPAddress = (LPCTSTR)spInputText->value;
            nRet = RtlIpv4StringToAddress(
                sIPAddress,
                TRUE,
                &pTerminator,
                &stIP4Address
                );
            if(NO_ERROR != nRet)
            {
                INT_PTR nDialogRet;
                CRTSDDMessageBox    oRTSDDMessageBox(this);
                oRTSDDMessageBox.m_sMessageText = _T("Ошибка формата начального значения диапазона публичных IP-адресов!!");
                nDialogRet = oRTSDDMessageBox.DoModal();
                hr = spPublicIPElement2->focus();
                hr = spInputText->select();
                return S_OK;
            }
            nhBeginAddress = ntohl(stIP4Address.S_un.S_addr);
            spDispatch = spCellChildrens->item(_variant_t(L"range_end"));
            hr = spDispatch->QueryInterface(&spInputText);
            hr = spDispatch->QueryInterface(&spPublicIPElement2);
            sIPAddress = (LPCTSTR)spInputText->value;
            nRet = RtlIpv4StringToAddress(
                sIPAddress,
                TRUE,
                &pTerminator,
                &stIP4Address
                );
            if(NO_ERROR != nRet)
            {
                INT_PTR nDialogRet;
                CRTSDDMessageBox    oRTSDDMessageBox(this);
                oRTSDDMessageBox.m_sMessageText = _T("Ошибка формата конечного значения диапазона публичных IP-адресов!!");
                nDialogRet = oRTSDDMessageBox.DoModal();
                hr = spPublicIPElement2->focus();
                hr = spInputText->select();
                return S_OK;
            }
            nhEndAddress = ntohl(stIP4Address.S_un.S_addr);
            if(nhBeginAddress > nhEndAddress)
            {
                INT_PTR nDialogRet;
                CRTSDDMessageBox    oRTSDDMessageBox(this);
                oRTSDDMessageBox.m_sMessageText = _T("Значение начального адреса диапазона больше конечного!!");
                nDialogRet = oRTSDDMessageBox.DoModal();
                hr = spPublicIPElement2->focus();
                hr = spInputText->select();
                return S_OK;
            }
            if(nhBeginAddress == nhEndAddress)
            {
                INT_PTR nDialogRet;
                CRTSDDMessageBox    oRTSDDMessageBox(this);
                oRTSDDMessageBox.m_sMessageText = _T("Значение начального адреса диапазона равно конечному, т.е задан единственный IP-адрес!");
                oRTSDDMessageBox.m_sMessageTypeImageSRC = _T("WARNING_ICON.PNG");
                nDialogRet = oRTSDDMessageBox.DoModal();
                hr = spPublicIPElement2->focus();
                hr = spInputText->select();
            }
            nRangeIP++;
        }
    }
    hr = GetElement(
        _T("subnet_address"),
        &spDispatch,
        &bCollection
        );
    if(bCollection)
    {
        hr = spDispatch->QueryInterface(&spHTMLElementCollection);
        for(i=0;i<spHTMLElementCollection->length;i++)
        {
            spDispatch = spHTMLElementCollection->item(_variant_t(i));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spIHTMLElement = spIHTMLElement->parentElement;
            if(spIHTMLElement->id == _bstr_t(L"public_subnet_cell"))
            {
                continue;
            }
            hr = spDispatch->QueryInterface(&spInputText);
            hr = spDispatch->QueryInterface(&spPublicIPElement2);
            spDispatch = spIHTMLElement->children;
            hr = spDispatch->QueryInterface(&spCellChildrens);
            sIPAddress = (LPCTSTR)spInputText->value;
            nRet = RtlIpv4StringToAddress(
                sIPAddress,
                TRUE,
                &pTerminator,
                &stIP4Address
                );
            if(NO_ERROR != nRet)
            {
                INT_PTR nDialogRet;
                CRTSDDMessageBox    oRTSDDMessageBox(this);
                oRTSDDMessageBox.m_sMessageText = _T("Ошибка формата адреса публичной IP-подсети!!");
                nDialogRet = oRTSDDMessageBox.DoModal();
                hr = spPublicIPElement2->focus();
                hr = spInputText->select();
                return S_OK;
            }
            spDispatch = spCellChildrens->item(_variant_t(L"subnet_mask"));
            hr = spDispatch->QueryInterface(&spInputText);
            hr = spDispatch->QueryInterface(&spPublicIPElement2);
            sIPAddress = (LPCTSTR)spInputText->value;
            nRet = RtlIpv4StringToAddress(
                sIPAddress,
                TRUE,
                &pTerminator,
                &stIP4Address
                );
            if(NO_ERROR != nRet)
            {
                INT_PTR nDialogRet;
                CRTSDDMessageBox    oRTSDDMessageBox(this);
                oRTSDDMessageBox.m_sMessageText = _T("Ошибка формата маски публичной IP-подсети!!");
                nDialogRet = oRTSDDMessageBox.DoModal();
                hr = spPublicIPElement2->focus();
                hr = spInputText->select();
                return S_OK;
            }
            nSubnetIP++;
        }
    }
    hr = GetElement(
        _T("private_ip"),
        &spDispatch,
        &bCollection
        );
    if(bCollection)
    {
        hr = spDispatch->QueryInterface(&spHTMLElementCollection);
        for(i=0;i<spHTMLElementCollection->length;i++)
        {
            spDispatch = spHTMLElementCollection->item(_variant_t(i));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spIHTMLElement = spIHTMLElement->parentElement;
            if(spIHTMLElement->id == _bstr_t(L"private_ip_cell"))
            {
                continue;
            }
            hr = spDispatch->QueryInterface(&spInputText);
            hr = spDispatch->QueryInterface(&spPublicIPElement2);
            sIPAddress = (LPCTSTR)spInputText->value;
            nRet = RtlIpv4StringToAddress(
                sIPAddress,
                TRUE,
                &pTerminator,
                &stIP4Address
                );
            if(NO_ERROR != nRet)
            {
                INT_PTR nDialogRet;
                CRTSDDMessageBox    oRTSDDMessageBox(this);
                oRTSDDMessageBox.m_sMessageText = _T("Ошибка формата IP-адреса!!");
                nDialogRet = oRTSDDMessageBox.DoModal();
                hr = spPublicIPElement2->focus();
                hr = spInputText->select();
                return S_OK;
            }
            nSinglePrivateIP++;
        }
    }
    hr = GetElement(
        _T("private_range_begin"),
        &spDispatch,
        &bCollection
        );
    if(bCollection)
    {
        hr = spDispatch->QueryInterface(&spHTMLElementCollection);
        for(i=0;i<spHTMLElementCollection->length;i++)
        {
            spDispatch = spHTMLElementCollection->item(_variant_t(i));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spIHTMLElement = spIHTMLElement->parentElement;
            if(spIHTMLElement->id == _bstr_t(L"private_range_cell"))
            {
                continue;
            }
            hr = spDispatch->QueryInterface(&spInputText);
            hr = spDispatch->QueryInterface(&spPublicIPElement2);
            spDispatch = spIHTMLElement->children;
            hr = spDispatch->QueryInterface(&spCellChildrens);
            sIPAddress = (LPCTSTR)spInputText->value;
            nRet = RtlIpv4StringToAddress(
                sIPAddress,
                TRUE,
                &pTerminator,
                &stIP4Address
                );
            if(NO_ERROR != nRet)
            {
                INT_PTR nDialogRet;
                CRTSDDMessageBox    oRTSDDMessageBox(this);
                oRTSDDMessageBox.m_sMessageText = _T("Ошибка формата начального значения диапазона приватных IP-адресов!!");
                nDialogRet = oRTSDDMessageBox.DoModal();
                hr = spPublicIPElement2->focus();
                hr = spInputText->select();
                return S_OK;
            }
            nhBeginAddress = ntohl(stIP4Address.S_un.S_addr);
            spDispatch = spCellChildrens->item(_variant_t(L"private_range_end"));
            hr = spDispatch->QueryInterface(&spInputText);
            hr = spDispatch->QueryInterface(&spPublicIPElement2);
            sIPAddress = (LPCTSTR)spInputText->value;
            nRet = RtlIpv4StringToAddress(
                sIPAddress,
                TRUE,
                &pTerminator,
                &stIP4Address
                );
            if(NO_ERROR != nRet)
            {
                INT_PTR nDialogRet;
                CRTSDDMessageBox    oRTSDDMessageBox(this);
                oRTSDDMessageBox.m_sMessageText = _T("Ошибка формата конечного значения диапазона приватных IP-адресов!!");
                nDialogRet = oRTSDDMessageBox.DoModal();
                hr = spPublicIPElement2->focus();
                hr = spInputText->select();
                return S_OK;
            }
            nhEndAddress = ntohl(stIP4Address.S_un.S_addr);
            if(nhBeginAddress > nhEndAddress)
            {
                INT_PTR nDialogRet;
                CRTSDDMessageBox    oRTSDDMessageBox(this);
                oRTSDDMessageBox.m_sMessageText = _T("Значение начального адреса диапазона больше конечного!!");
                nDialogRet = oRTSDDMessageBox.DoModal();
                hr = spPublicIPElement2->focus();
                hr = spInputText->select();
                return S_OK;
            }
            if(nhBeginAddress == nhEndAddress)
            {
                INT_PTR nDialogRet;
                CRTSDDMessageBox    oRTSDDMessageBox(this);
                oRTSDDMessageBox.m_sMessageText = _T("Значение начального адреса диапазона равно конечному, т.е задан единственный IP-адрес!");
                oRTSDDMessageBox.m_sMessageTypeImageSRC = _T("WARNING_ICON.PNG");
                nDialogRet = oRTSDDMessageBox.DoModal();
                hr = spPublicIPElement2->focus();
                hr = spInputText->select();
            }
            nRangePrivateIP++;
        }
    }
    hr = GetElement(
        _T("private_subnet_address"),
        &spDispatch,
        &bCollection
        );
    if(bCollection)
    {
        hr = spDispatch->QueryInterface(&spHTMLElementCollection);
        for(i=0;i<spHTMLElementCollection->length;i++)
        {
            spDispatch = spHTMLElementCollection->item(_variant_t(i));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spIHTMLElement = spIHTMLElement->parentElement;
            if(spIHTMLElement->id == _bstr_t(L"private_subnet_cell"))
            {
                continue;
            }
            hr = spDispatch->QueryInterface(&spInputText);
            hr = spDispatch->QueryInterface(&spPublicIPElement2);
            spDispatch = spIHTMLElement->children;
            hr = spDispatch->QueryInterface(&spCellChildrens);
            sIPAddress = (LPCTSTR)spInputText->value;
            nRet = RtlIpv4StringToAddress(
                sIPAddress,
                TRUE,
                &pTerminator,
                &stIP4Address
                );
            if(NO_ERROR != nRet)
            {
                INT_PTR nDialogRet;
                CRTSDDMessageBox    oRTSDDMessageBox(this);
                oRTSDDMessageBox.m_sMessageText = _T("Ошибка формата адреса приватной IP-подсети!!");
                nDialogRet = oRTSDDMessageBox.DoModal();
                hr = spPublicIPElement2->focus();
                hr = spInputText->select();
                return S_OK;
            }
            spDispatch = spCellChildrens->item(_variant_t(L"private_subnet_mask"));
            hr = spDispatch->QueryInterface(&spInputText);
            hr = spDispatch->QueryInterface(&spPublicIPElement2);
            sIPAddress = (LPCTSTR)spInputText->value;
            nRet = RtlIpv4StringToAddress(
                sIPAddress,
                TRUE,
                &pTerminator,
                &stIP4Address
                );
            if(NO_ERROR != nRet)
            {
                INT_PTR nDialogRet;
                CRTSDDMessageBox    oRTSDDMessageBox(this);
                oRTSDDMessageBox.m_sMessageText = _T("Ошибка формата маски приватной IP-подсети!!");
                nDialogRet = oRTSDDMessageBox.DoModal();
                hr = spPublicIPElement2->focus();
                hr = spInputText->select();
                return S_OK;
            }
            nSubnetPrivateIP++;
        }
    }
    hr = GetElement(
        _T("ip_port"),
        &spDispatch,
        &bCollection
        );
    if(bCollection)
    {
        hr = spDispatch->QueryInterface(&spHTMLElementCollection);
        for(i=0;i<spHTMLElementCollection->length;i++)
        {
            spDispatch = spHTMLElementCollection->item(_variant_t(i));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spIHTMLElement = spIHTMLElement->parentElement;
            if(spIHTMLElement->id == _bstr_t(L"ip_port_cell"))
            {
                continue;
            }
            hr = spDispatch->QueryInterface(&spInputText);
            hr = spDispatch->QueryInterface(&spPublicIPElement2);
            sIpPort = (LPCTSTR)spInputText->value;
            nIpPort = _tstol(sIpPort);
            bIpPortValid = ((nIpPort > 0) && (nIpPort < 0x10000));
            if(!bIpPortValid)
            {
                INT_PTR nDialogRet;
                CRTSDDMessageBox    oRTSDDMessageBox(this);
                oRTSDDMessageBox.m_sMessageText = _T("Ошибка формата значения запрещённого входящего IP-порта!!");
                nDialogRet = oRTSDDMessageBox.DoModal();
                hr = spPublicIPElement2->focus();
                hr = spInputText->select();
                return S_OK;
            }
            nSingleIpPort++;
        }
    }
    hr = GetElement(
        _T("ip_port_range_begin"),
        &spDispatch,
        &bCollection
        );
    if(bCollection)
    {
        hr = spDispatch->QueryInterface(&spHTMLElementCollection);
        for(i=0;i<spHTMLElementCollection->length;i++)
        {
            spDispatch = spHTMLElementCollection->item(_variant_t(i));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spIHTMLElement = spIHTMLElement->parentElement;
            if(spIHTMLElement->id == _bstr_t(L"range_ip_port_cell"))
            {
                continue;
            }
            hr = spDispatch->QueryInterface(&spInputText);
            hr = spDispatch->QueryInterface(&spPublicIPElement2);
            spDispatch = spIHTMLElement->children;
            hr = spDispatch->QueryInterface(&spCellChildrens);
            sIpPort = (LPCTSTR)spInputText->value;
            nIpPortBegin = _tstol(sIpPort);
            bIpPortBeginValid = ((nIpPortBegin > 0) && (nIpPortBegin < 0x10000));
            if(!bIpPortBeginValid)
            {
                INT_PTR nDialogRet;
                CRTSDDMessageBox    oRTSDDMessageBox(this);
                oRTSDDMessageBox.m_sMessageText = _T("Ошибка формата начального значения диапазона запрещённых входящих IP-портов!!");
                nDialogRet = oRTSDDMessageBox.DoModal();
                hr = spPublicIPElement2->focus();
                hr = spInputText->select();
                return S_OK;
            }
            spDispatch = spCellChildrens->item(_variant_t(L"ip_port_range_end"));
            hr = spDispatch->QueryInterface(&spInputText);
            hr = spDispatch->QueryInterface(&spPublicIPElement2);
            sIpPort = (LPCTSTR)spInputText->value;
            nIpPortEnd = _tstol(sIpPort);
            bIpPortEndValid = ((nIpPortEnd > 0) && (nIpPortEnd < 0x10000));
            if(!bIpPortEndValid)
            {
                INT_PTR nDialogRet;
                CRTSDDMessageBox    oRTSDDMessageBox(this);
                oRTSDDMessageBox.m_sMessageText = _T("Ошибка формата конечного значения диапазона запрещённых входящих IP-портов!!");
                nDialogRet = oRTSDDMessageBox.DoModal();
                hr = spPublicIPElement2->focus();
                hr = spInputText->select();
                return S_OK;
            }
            if(nIpPortBegin > nIpPortEnd)
            {
                INT_PTR nDialogRet;
                CRTSDDMessageBox    oRTSDDMessageBox(this);
                oRTSDDMessageBox.m_sMessageText = _T("Начальное значение диапазона больше конечного!!");
                nDialogRet = oRTSDDMessageBox.DoModal();
                hr = spPublicIPElement2->focus();
                hr = spInputText->select();
                return S_OK;
            }
            if(nIpPortBegin == nIpPortEnd)
            {
                INT_PTR nDialogRet;
                CRTSDDMessageBox    oRTSDDMessageBox(this);
                oRTSDDMessageBox.m_sMessageText = _T("Начальное значение диапазона равно конечному, т.е задано единственное значение запрещённого входящего IP-порта!");
                oRTSDDMessageBox.m_sMessageTypeImageSRC = _T("WARNING_ICON.PNG");
                nDialogRet = oRTSDDMessageBox.DoModal();
                hr = spPublicIPElement2->focus();
                hr = spInputText->select();
            }
            nRangeIpPort++;
        }
    }
    bRet = WriteConfigString(
        _T("service_settings"),
        _T("service_folder"),
        m_sServicesFolder
        );
    bRet = WriteConfigString(
        _T("service_settings"),
        _T("public_router_ip"),
        m_sPublicRouterIP
        );
    if(bIPExist)
    {
        HRESULT     hr;
        LPOLESTR    pszGuid;
        CString     sGuid;
        hr = StringFromIID(
            stInterfaceGuid,
            &pszGuid
            );
        ASSERT(SUCCEEDED(hr));
        sGuid = pszGuid;
        CoTaskMemFree(pszGuid);
        bRet = WriteConfigString(
            _T("service_settings"),
            _T("public_router_guid"),
            sGuid
            );
        ASSERT(bRet);
        bRet = WriteConfigInt(
            _T("service_settings"),
            _T("public_router_exist"),
            TRUE
            );
        ASSERT(bRet);
    }
    else
    {
        bRet = WriteConfigInt(
            _T("service_settings"),
            _T("public_router_exist"),
            0
            );
    }
    bRet = WriteConfigInt(
        _T("service_settings"),
        _T("public_ip_count"),
        nSingleIP
        );
    bRet = WriteConfigInt(
        _T("service_settings"),
        _T("public_range_count"),
        nRangeIP
        );
    bRet = WriteConfigInt(
        _T("service_settings"),
        _T("public_subnet_count"),
        nSubnetIP
        );
    bRet = WriteConfigInt(
        _T("service_settings"),
        _T("private_ip_count"),
        nSinglePrivateIP
        );
    bRet = WriteConfigInt(
        _T("service_settings"),
        _T("private_range_count"),
        nRangePrivateIP
        );
    bRet = WriteConfigInt(
        _T("service_settings"),
        _T("private_subnet_count"),
        nSubnetPrivateIP
        );
    bRet = WriteConfigInt(
        _T("service_settings"),
        _T("ip_port_count"),
        nSingleIpPort
        );
    bRet = WriteConfigInt(
        _T("service_settings"),
        _T("ip_port_range_count"),
        nRangeIpPort
        );
    if(nSingleIP > 0)
    {
        nSingleIP = 0;
        hr = GetElement(
            _T("public_ip"),
            &spDispatch,
            &bCollection
            );
        hr = spDispatch->QueryInterface(&spHTMLElementCollection);
        for(i=0;i<spHTMLElementCollection->length;i++)
        {
            spDispatch = spHTMLElementCollection->item(_variant_t(i));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spIHTMLElement = spIHTMLElement->parentElement;
            if(spIHTMLElement->id == _bstr_t(L"public_ip_cell"))
            {
                continue;
            }
            hr = spDispatch->QueryInterface(&spInputText);
            sIPAddress = (LPCTSTR)spInputText->value;
            sSectionName.Format(
                _T("single_ip%d"),
                nSingleIP
                );
            bRet = WriteConfigString(
                _T("service_settings"),
                sSectionName,
                sIPAddress
                );
            nSingleIP++;
        }
    }
    if(nRangeIP > 0)
    {
        nRangeIP = 0;
        hr = GetElement(
            _T("range_begin"),
            &spDispatch,
            &bCollection
            );
        hr = spDispatch->QueryInterface(&spHTMLElementCollection);
        for(i=0;i<spHTMLElementCollection->length;i++)
        {
            spDispatch = spHTMLElementCollection->item(_variant_t(i));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spIHTMLElement = spIHTMLElement->parentElement;
            if(spIHTMLElement->id == _bstr_t(L"public_range_cell"))
            {
                continue;
            }
            hr = spDispatch->QueryInterface(&spInputText);
            spDispatch = spIHTMLElement->children;
            hr = spDispatch->QueryInterface(&spCellChildrens);
            sIPAddress = (LPCTSTR)spInputText->value;
            sSectionName.Format(
                _T("range_begin_ip%d"),
                nRangeIP
                );
            bRet = WriteConfigString(
                _T("service_settings"),
                sSectionName,
                sIPAddress
                );
            spDispatch = spCellChildrens->item(_variant_t(L"range_end"));
            hr = spDispatch->QueryInterface(&spInputText);
            sIPAddress = (LPCTSTR)spInputText->value;
            sSectionName.Format(
                _T("range_end_ip%d"),
                nRangeIP
                );
            bRet = WriteConfigString(
                _T("service_settings"),
                sSectionName,
                sIPAddress
                );
            nRangeIP++;
        }
    }
    if(nSubnetIP > 0)
    {
        nSubnetIP = 0;
        hr = GetElement(
            _T("subnet_address"),
            &spDispatch,
            &bCollection
            );
        hr = spDispatch->QueryInterface(&spHTMLElementCollection);
        for(i=0;i<spHTMLElementCollection->length;i++)
        {
            spDispatch = spHTMLElementCollection->item(_variant_t(i));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spIHTMLElement = spIHTMLElement->parentElement;
            if(spIHTMLElement->id == _bstr_t(L"public_subnet_cell"))
            {
                continue;
            }
            hr = spDispatch->QueryInterface(&spInputText);
            spDispatch = spIHTMLElement->children;
            hr = spDispatch->QueryInterface(&spCellChildrens);
            sIPAddress = (LPCTSTR)spInputText->value;
            sSectionName.Format(
                _T("subnet_address_ip%d"),
                nSubnetIP
                );
            bRet = WriteConfigString(
                _T("service_settings"),
                sSectionName,
                sIPAddress
                );
            spDispatch = spCellChildrens->item(_variant_t(L"subnet_mask"));
            hr = spDispatch->QueryInterface(&spInputText);
            sIPAddress = (LPCTSTR)spInputText->value;
            sSectionName.Format(
                _T("subnet_mask_ip%d"),
                nSubnetIP
                );
            bRet = WriteConfigString(
                _T("service_settings"),
                sSectionName,
                sIPAddress
                );
            nSubnetIP++;
        }
    }
    if(nSinglePrivateIP > 0)
    {
        nSinglePrivateIP = 0;
        hr = GetElement(
            _T("private_ip"),
            &spDispatch,
            &bCollection
            );
        hr = spDispatch->QueryInterface(&spHTMLElementCollection);
        for(i=0;i<spHTMLElementCollection->length;i++)
        {
            spDispatch = spHTMLElementCollection->item(_variant_t(i));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spIHTMLElement = spIHTMLElement->parentElement;
            if(spIHTMLElement->id == _bstr_t(L"private_ip_cell"))
            {
                continue;
            }
            hr = spDispatch->QueryInterface(&spInputText);
            sIPAddress = (LPCTSTR)spInputText->value;
            sSectionName.Format(
                _T("private_single_ip%d"),
                nSinglePrivateIP
                );
            bRet = WriteConfigString(
                _T("service_settings"),
                sSectionName,
                sIPAddress
                );
            nSinglePrivateIP++;
        }
    }
    if(nRangePrivateIP > 0)
    {
        nRangePrivateIP = 0;
        hr = GetElement(
            _T("private_range_begin"),
            &spDispatch,
            &bCollection
            );
        hr = spDispatch->QueryInterface(&spHTMLElementCollection);
        for(i=0;i<spHTMLElementCollection->length;i++)
        {
            spDispatch = spHTMLElementCollection->item(_variant_t(i));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spIHTMLElement = spIHTMLElement->parentElement;
            if(spIHTMLElement->id == _bstr_t(L"private_range_cell"))
            {
                continue;
            }
            hr = spDispatch->QueryInterface(&spInputText);
            spDispatch = spIHTMLElement->children;
            hr = spDispatch->QueryInterface(&spCellChildrens);
            sIPAddress = (LPCTSTR)spInputText->value;
            sSectionName.Format(
                _T("private_range_begin_ip%d"),
                nRangePrivateIP
                );
            bRet = WriteConfigString(
                _T("service_settings"),
                sSectionName,
                sIPAddress
                );
            spDispatch = spCellChildrens->item(_variant_t(L"private_range_end"));
            hr = spDispatch->QueryInterface(&spInputText);
            sIPAddress = (LPCTSTR)spInputText->value;
            sSectionName.Format(
                _T("private_range_end_ip%d"),
                nRangePrivateIP
                );
            bRet = WriteConfigString(
                _T("service_settings"),
                sSectionName,
                sIPAddress
                );
            nRangePrivateIP++;
        }
    }
    if(nSubnetPrivateIP > 0)
    {
        nSubnetPrivateIP = 0;
        hr = GetElement(
            _T("private_subnet_address"),
            &spDispatch,
            &bCollection
            );
        hr = spDispatch->QueryInterface(&spHTMLElementCollection);
        for(i=0;i<spHTMLElementCollection->length;i++)
        {
            spDispatch = spHTMLElementCollection->item(_variant_t(i));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spIHTMLElement = spIHTMLElement->parentElement;
            if(spIHTMLElement->id == _bstr_t(L"private_subnet_cell"))
            {
                continue;
            }
            hr = spDispatch->QueryInterface(&spInputText);
            spDispatch = spIHTMLElement->children;
            hr = spDispatch->QueryInterface(&spCellChildrens);
            sIPAddress = (LPCTSTR)spInputText->value;
            sSectionName.Format(
                _T("private_subnet_address_ip%d"),
                nSubnetPrivateIP
                );
            bRet = WriteConfigString(
                _T("service_settings"),
                sSectionName,
                sIPAddress
                );
            spDispatch = spCellChildrens->item(_variant_t(L"private_subnet_mask"));
            hr = spDispatch->QueryInterface(&spInputText);
            sIPAddress = (LPCTSTR)spInputText->value;
            sSectionName.Format(
                _T("private_subnet_mask_ip%d"),
                nSubnetPrivateIP
                );
            bRet = WriteConfigString(
                _T("service_settings"),
                sSectionName,
                sIPAddress
                );
            nSubnetPrivateIP++;
        }
    }
    if(nSingleIpPort > 0)
    {
        nSingleIpPort = 0;
        hr = GetElement(
            _T("ip_port"),
            &spDispatch,
            &bCollection
            );
        hr = spDispatch->QueryInterface(&spHTMLElementCollection);
        for(i=0;i<spHTMLElementCollection->length;i++)
        {
            spDispatch = spHTMLElementCollection->item(_variant_t(i));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spIHTMLElement = spIHTMLElement->parentElement;
            if(spIHTMLElement->id == _bstr_t(L"ip_port_cell"))
            {
                continue;
            }
            hr = spDispatch->QueryInterface(&spInputText);
            sIpPort = (LPCTSTR)spInputText->value;
            nIpPort = _tstol(sIpPort);
            sSectionName.Format(
                _T("single_ip_port%d"),
                nSingleIpPort
                );
            bRet = WriteConfigInt(
                _T("service_settings"),
                sSectionName,
                nIpPort
                );
            nSingleIpPort++;
        }
    }
    if(nRangeIpPort > 0)
    {
        nRangeIpPort = 0;
        hr = GetElement(
            _T("ip_port_range_begin"),
            &spDispatch,
            &bCollection
            );
        hr = spDispatch->QueryInterface(&spHTMLElementCollection);
        for(i=0;i<spHTMLElementCollection->length;i++)
        {
            spDispatch = spHTMLElementCollection->item(_variant_t(i));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spIHTMLElement = spIHTMLElement->parentElement;
            if(spIHTMLElement->id == _bstr_t(L"range_ip_port_cell"))
            {
                continue;
            }
            hr = spDispatch->QueryInterface(&spInputText);
            spDispatch = spIHTMLElement->children;
            hr = spDispatch->QueryInterface(&spCellChildrens);
            sIpPort = (LPCTSTR)spInputText->value;
            nIpPort = _tstol(sIpPort);
            sSectionName.Format(
                _T("range_begin_ip_port%d"),
                nRangeIpPort
                );
            bRet = WriteConfigInt(
                _T("service_settings"),
                sSectionName,
                nIpPort
                );
            spDispatch = spCellChildrens->item(_variant_t(L"ip_port_range_end"));
            hr = spDispatch->QueryInterface(&spInputText);
            sIpPort = (LPCTSTR)spInputText->value;
            nIpPort = _tstol(sIpPort);
            sSectionName.Format(
                _T("range_end_ip_port%d"),
                nRangeIpPort
                );
            bRet = WriteConfigInt(
                _T("service_settings"),
                sSectionName,
                nIpPort
                );
            nRangeIpPort++;
        }
    }
    return S_OK;
}


void CNATTraversalConfigDlg::OnDocumentComplete(LPDISPATCH pDisp, LPCTSTR szUrl)
{
    HRESULT hr;
    MSHTML::IHTMLTablePtr       spSettingsTable;
    MSHTML::IHTMLElementPtr     spIHTMLElement;
    MSHTML::IHTMLTableRowPtr    spPublicIPTemplateRow;
    MSHTML::IHTMLTableRowPtr    spNewRow;
    MSHTML::IHTMLStylePtr       spStyle;
    MSHTML::IHTMLTableCellPtr   spNewCell;
    MSHTML::IHTMLButtonElementPtr       spHTMLButtonElement;
    MSHTML::IHTMLElementCollectionPtr   spHTMLElementCollection;
    MSHTML::IHTMLInputTextElementPtr    spInputText;
    long    nRowIndex;
    IDispatchPtr    spDispatch;
    _bstr_t sCellHTML;
    CString sSectionName;
    CString sIPAddress;
    SC_HANDLE schSCManager;
    SC_HANDLE schService;
    DWORD nErrorCode;
    BOOL bRet;
    int i, nSingleIP = 0, nRangeIP = 0, nSubnetIP = 0, nSingleIpPort = 0, nRangeIpPort = 0;
    int nSinglePrivateIP = 0, nRangePrivateIP = 0, nSubnetPrivateIP = 0;
    CString sIpPort;
    long    nIpPort;
    CDHtmlDialog::OnDocumentComplete(pDisp, szUrl);
    if(m_bHTMLInit)
    {
        return;
    }
    schSCManager = OpenSCManager(
        NULL,                 // local machine
        NULL,                 // ServicesActive database
        SC_MANAGER_ALL_ACCESS // full access rights
        );
    if(NULL == schSCManager)
    {
        INT_PTR nDialogRet;
        CRTSDDMessageBox    oRTSDDMessageBox(this);
        nErrorCode = ::GetLastError();
        oRTSDDMessageBox.m_sMessageText.Format(
            _T("\"OpenSCManager\" failed. Error code = %d"),
            nErrorCode
            );
        nDialogRet = oRTSDDMessageBox.DoModal();
        return;
    }
    hr = GetElement(
        _T("setup_button"),
        (IHTMLElement**)&spIHTMLElement
        );
    hr = spIHTMLElement->QueryInterface(&spHTMLButtonElement);
    schService = OpenService(
        schSCManager,
        _T("NATTraversalService"),
        SERVICE_ALL_ACCESS
        );
    if(NULL == schService)
    {
        nErrorCode = ::GetLastError();
        CloseServiceHandle(schSCManager);
        if(ERROR_SERVICE_DOES_NOT_EXIST != nErrorCode) {
            INT_PTR nDialogRet;
            CRTSDDMessageBox    oRTSDDMessageBox(this);
            oRTSDDMessageBox.m_sMessageText.Format(
                _T("OpenService failed. Error code = %d"),
                nErrorCode
                );
            nDialogRet = oRTSDDMessageBox.DoModal();
            return;
        }
        spHTMLButtonElement->disabled = VARIANT_FALSE;
        m_sSetupImageSRC = _T("SETUP.GIF");
        hr = GetElement(
            _T("uninstall_button"),
            (IHTMLElement**)&spIHTMLElement
            );
        hr = spIHTMLElement->QueryInterface(&spHTMLButtonElement);
        spHTMLButtonElement->disabled = VARIANT_TRUE;
        m_sUninstallImageSRC = _T("UNINSTALL32_GRAY.PNG");
        hr = GetElement(
            _T("restart_button"),
            (IHTMLElement**)&spIHTMLElement
            );
        hr = spIHTMLElement->QueryInterface(&spHTMLButtonElement);
        spHTMLButtonElement->disabled = VARIANT_TRUE;
        m_sRestartImageSRC = _T("RESTARTHS_GRAY.PNG");
    }
    else
    {
        spHTMLButtonElement->disabled = VARIANT_TRUE;
        m_sSetupImageSRC = _T("SETUP_HIDEN.GIF");
        hr = GetElement(
            _T("uninstall_button"),
            (IHTMLElement**)&spIHTMLElement
            );
        hr = spIHTMLElement->QueryInterface(&spHTMLButtonElement);
        spHTMLButtonElement->disabled = VARIANT_FALSE;
        m_sUninstallImageSRC = _T("UNINSTALL32.PNG");
        hr = GetElement(
            _T("restart_button"),
            (IHTMLElement**)&spIHTMLElement
            );
        hr = spIHTMLElement->QueryInterface(&spHTMLButtonElement);
        spHTMLButtonElement->disabled = VARIANT_FALSE;
        m_sRestartImageSRC = _T("RESTARTHS.PNG");
    }
    bRet = UpdateData(FALSE);
	nSingleIP = GetConfigInt(
        _T("service_settings"),
        _T("public_ip_count"),
        0
        );
	nRangeIP = GetConfigInt(
        _T("service_settings"),
        _T("public_range_count"),
        0
        );
	nSubnetIP = GetConfigInt(
        _T("service_settings"),
        _T("public_subnet_count"),
        0
        );
	nSinglePrivateIP = GetConfigInt(
        _T("service_settings"),
        _T("private_ip_count"),
        0
        );
	nRangePrivateIP = GetConfigInt(
        _T("service_settings"),
        _T("private_range_count"),
        0
        );
	nSubnetPrivateIP = GetConfigInt(
        _T("service_settings"),
        _T("private_subnet_count"),
        0
        );
	nSingleIpPort = GetConfigInt(
        _T("service_settings"),
        _T("ip_port_count"),
        0
        );
	nRangeIpPort = GetConfigInt(
        _T("service_settings"),
        _T("ip_port_range_count"),
        0
        );
    hr = GetElement(
        _T("settings_table"),
        (IHTMLElement**)&spIHTMLElement
        );
    hr = spIHTMLElement->QueryInterface(&spSettingsTable);
    if(nSingleIP > 0)
    {
        for(i=0;i<nSingleIP;i++)
        {
            hr = GetElement(
                _T("public_ip_row_template"),
                (IHTMLElement**)&spIHTMLElement
                );
            hr = spIHTMLElement->QueryInterface(&spPublicIPTemplateRow);
            nRowIndex = spPublicIPTemplateRow->rowIndex;
            spDispatch = spSettingsTable->insertRow(nRowIndex);
            hr = spDispatch->QueryInterface(&spNewRow);
            hr = GetElement(
                _T("public_ip_description"),
                (IHTMLElement**)&spIHTMLElement
                );
            sCellHTML = spIHTMLElement->innerHTML;
            spDispatch = spNewRow->insertCell(-1);
            hr = spDispatch->QueryInterface(&spNewCell);
            spNewCell->align = _bstr_t(L"right");
            spNewCell->width = _variant_t(L"50%");
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spIHTMLElement->innerHTML = sCellHTML;
            hr = GetElement(
                _T("public_ip_cell"),
                (IHTMLElement**)&spIHTMLElement
                );
            sCellHTML = spIHTMLElement->innerHTML;
            spDispatch = spNewRow->insertCell(-1);
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spIHTMLElement->innerHTML = sCellHTML;
            spDispatch = spIHTMLElement->children;
            hr = spDispatch->QueryInterface(&spHTMLElementCollection);
            spDispatch = spHTMLElementCollection->item(_variant_t(L"insert_public_ip"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"hidden");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"delete_public_ip"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"public_ip"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            hr = spDispatch->QueryInterface(&spInputText);
            sSectionName.Format(
                _T("single_ip%d"),
                i
                );
	        sIPAddress = GetConfigString(
                _T("service_settings"),
                sSectionName,
                _T("")
                );
            spInputText->value = _bstr_t((LPCTSTR)sIPAddress);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"test_public_ip_button"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
        }
    }
    if(nRangeIP > 0)
    {
        for(i=0;i<nRangeIP;i++)
        {
            hr = GetElement(
                _T("public_range_row_template"),
                (IHTMLElement**)&spIHTMLElement
                );
            hr = spIHTMLElement->QueryInterface(&spPublicIPTemplateRow);
            nRowIndex = spPublicIPTemplateRow->rowIndex;
            spDispatch = spSettingsTable->insertRow(nRowIndex);
            hr = spDispatch->QueryInterface(&spNewRow);
            hr = GetElement(
                _T("public_range_description"),
                (IHTMLElement**)&spIHTMLElement
                );
            sCellHTML = spIHTMLElement->innerHTML;
            spDispatch = spNewRow->insertCell(-1);
            hr = spDispatch->QueryInterface(&spNewCell);
            spNewCell->align = _bstr_t(L"right");
            spNewCell->width = _variant_t(L"50%");
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spIHTMLElement->innerHTML = sCellHTML;
            hr = GetElement(
                _T("public_range_cell"),
                (IHTMLElement**)&spIHTMLElement
                );
            sCellHTML = spIHTMLElement->innerHTML;
            spDispatch = spNewRow->insertCell(-1);
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spIHTMLElement->innerHTML = sCellHTML;
            spDispatch = spIHTMLElement->children;
            hr = spDispatch->QueryInterface(&spHTMLElementCollection);
            spDispatch = spHTMLElementCollection->item(_variant_t(L"insert_public_range"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"hidden");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"delete_public_range"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"range_begin"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            hr = spDispatch->QueryInterface(&spInputText);
            sSectionName.Format(
                _T("range_begin_ip%d"),
                i
                );
	        sIPAddress = GetConfigString(
                _T("service_settings"),
                sSectionName,
                _T("")
                );
            spInputText->value = _bstr_t((LPCTSTR)sIPAddress);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"range_symbol"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"range_end"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            hr = spDispatch->QueryInterface(&spInputText);
            sSectionName.Format(
                _T("range_end_ip%d"),
                i
                );
	        sIPAddress = GetConfigString(
                _T("service_settings"),
                sSectionName,
                _T("")
                );
            spInputText->value = _bstr_t((LPCTSTR)sIPAddress);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"test_public_range_button"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
        }
    }
    if(nSubnetIP > 0)
    {
        for(i=0;i<nSubnetIP;i++)
        {
            hr = GetElement(
                _T("public_subnet_row_template"),
                (IHTMLElement**)&spIHTMLElement
                );
            hr = spIHTMLElement->QueryInterface(&spPublicIPTemplateRow);
            nRowIndex = spPublicIPTemplateRow->rowIndex;
            spDispatch = spSettingsTable->insertRow(nRowIndex);
            hr = spDispatch->QueryInterface(&spNewRow);
            hr = GetElement(
                _T("public_subnet_description"),
                (IHTMLElement**)&spIHTMLElement
                );
            sCellHTML = spIHTMLElement->innerHTML;
            spDispatch = spNewRow->insertCell(-1);
            hr = spDispatch->QueryInterface(&spNewCell);
            spNewCell->align = _bstr_t(L"right");
            spNewCell->width = _variant_t(L"50%");
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spIHTMLElement->innerHTML = sCellHTML;
            hr = GetElement(
                _T("public_subnet_cell"),
                (IHTMLElement**)&spIHTMLElement
                );
            sCellHTML = spIHTMLElement->innerHTML;
            spDispatch = spNewRow->insertCell(-1);
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spIHTMLElement->innerHTML = sCellHTML;
            spDispatch = spIHTMLElement->children;
            hr = spDispatch->QueryInterface(&spHTMLElementCollection);
            spDispatch = spHTMLElementCollection->item(_variant_t(L"insert_public_subnet"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"hidden");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"delete_public_subnet"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"subnet_address"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            hr = spDispatch->QueryInterface(&spInputText);
            sSectionName.Format(
                _T("subnet_address_ip%d"),
                i
                );
	        sIPAddress = GetConfigString(
                _T("service_settings"),
                sSectionName,
                _T("")
                );
            spInputText->value = _bstr_t((LPCTSTR)sIPAddress);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"subnet_symbol"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"subnet_mask"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            hr = spDispatch->QueryInterface(&spInputText);
            sSectionName.Format(
                _T("subnet_mask_ip%d"),
                i
                );
	        sIPAddress = GetConfigString(
                _T("service_settings"),
                sSectionName,
                _T("")
                );
            spInputText->value = _bstr_t((LPCTSTR)sIPAddress);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"test_public_subnet_button"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
        }
    }
    if(nSinglePrivateIP > 0)
    {
        for(i=0;i<nSinglePrivateIP;i++)
        {
            hr = GetElement(
                _T("private_ip_row_template"),
                (IHTMLElement**)&spIHTMLElement
                );
            hr = spIHTMLElement->QueryInterface(&spPublicIPTemplateRow);
            nRowIndex = spPublicIPTemplateRow->rowIndex;
            spDispatch = spSettingsTable->insertRow(nRowIndex);
            hr = spDispatch->QueryInterface(&spNewRow);
            hr = GetElement(
                _T("private_ip_description"),
                (IHTMLElement**)&spIHTMLElement
                );
            sCellHTML = spIHTMLElement->innerHTML;
            spDispatch = spNewRow->insertCell(-1);
            hr = spDispatch->QueryInterface(&spNewCell);
            spNewCell->align = _bstr_t(L"right");
            spNewCell->width = _variant_t(L"50%");
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spIHTMLElement->innerHTML = sCellHTML;
            hr = GetElement(
                _T("private_ip_cell"),
                (IHTMLElement**)&spIHTMLElement
                );
            sCellHTML = spIHTMLElement->innerHTML;
            spDispatch = spNewRow->insertCell(-1);
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spIHTMLElement->innerHTML = sCellHTML;
            spDispatch = spIHTMLElement->children;
            hr = spDispatch->QueryInterface(&spHTMLElementCollection);
            spDispatch = spHTMLElementCollection->item(_variant_t(L"insert_private_ip"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"hidden");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"delete_private_ip"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"private_ip"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            hr = spDispatch->QueryInterface(&spInputText);
            sSectionName.Format(
                _T("private_single_ip%d"),
                i
                );
	        sIPAddress = GetConfigString(
                _T("service_settings"),
                sSectionName,
                _T("")
                );
            spInputText->value = _bstr_t((LPCTSTR)sIPAddress);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"test_private_ip_button"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
        }
    }
    if(nRangePrivateIP > 0)
    {
        for(i=0;i<nRangePrivateIP;i++)
        {
            hr = GetElement(
                _T("private_range_row_template"),
                (IHTMLElement**)&spIHTMLElement
                );
            hr = spIHTMLElement->QueryInterface(&spPublicIPTemplateRow);
            nRowIndex = spPublicIPTemplateRow->rowIndex;
            spDispatch = spSettingsTable->insertRow(nRowIndex);
            hr = spDispatch->QueryInterface(&spNewRow);
            hr = GetElement(
                _T("private_range_description"),
                (IHTMLElement**)&spIHTMLElement
                );
            sCellHTML = spIHTMLElement->innerHTML;
            spDispatch = spNewRow->insertCell(-1);
            hr = spDispatch->QueryInterface(&spNewCell);
            spNewCell->align = _bstr_t(L"right");
            spNewCell->width = _variant_t(L"50%");
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spIHTMLElement->innerHTML = sCellHTML;
            hr = GetElement(
                _T("private_range_cell"),
                (IHTMLElement**)&spIHTMLElement
                );
            sCellHTML = spIHTMLElement->innerHTML;
            spDispatch = spNewRow->insertCell(-1);
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spIHTMLElement->innerHTML = sCellHTML;
            spDispatch = spIHTMLElement->children;
            hr = spDispatch->QueryInterface(&spHTMLElementCollection);
            spDispatch = spHTMLElementCollection->item(_variant_t(L"insert_private_range"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"hidden");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"delete_private_range"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"private_range_begin"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            hr = spDispatch->QueryInterface(&spInputText);
            sSectionName.Format(
                _T("private_range_begin_ip%d"),
                i
                );
	        sIPAddress = GetConfigString(
                _T("service_settings"),
                sSectionName,
                _T("")
                );
            spInputText->value = _bstr_t((LPCTSTR)sIPAddress);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"private_range_symbol"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"private_range_end"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            hr = spDispatch->QueryInterface(&spInputText);
            sSectionName.Format(
                _T("private_range_end_ip%d"),
                i
                );
	        sIPAddress = GetConfigString(
                _T("service_settings"),
                sSectionName,
                _T("")
                );
            spInputText->value = _bstr_t((LPCTSTR)sIPAddress);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"test_private_range_button"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
        }
    }
    if(nSubnetPrivateIP > 0)
    {
        for(i=0;i<nSubnetPrivateIP;i++)
        {
            hr = GetElement(
                _T("private_subnet_row_template"),
                (IHTMLElement**)&spIHTMLElement
                );
            hr = spIHTMLElement->QueryInterface(&spPublicIPTemplateRow);
            nRowIndex = spPublicIPTemplateRow->rowIndex;
            spDispatch = spSettingsTable->insertRow(nRowIndex);
            hr = spDispatch->QueryInterface(&spNewRow);
            hr = GetElement(
                _T("private_subnet_description"),
                (IHTMLElement**)&spIHTMLElement
                );
            sCellHTML = spIHTMLElement->innerHTML;
            spDispatch = spNewRow->insertCell(-1);
            hr = spDispatch->QueryInterface(&spNewCell);
            spNewCell->align = _bstr_t(L"right");
            spNewCell->width = _variant_t(L"50%");
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spIHTMLElement->innerHTML = sCellHTML;
            hr = GetElement(
                _T("private_subnet_cell"),
                (IHTMLElement**)&spIHTMLElement
                );
            sCellHTML = spIHTMLElement->innerHTML;
            spDispatch = spNewRow->insertCell(-1);
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spIHTMLElement->innerHTML = sCellHTML;
            spDispatch = spIHTMLElement->children;
            hr = spDispatch->QueryInterface(&spHTMLElementCollection);
            spDispatch = spHTMLElementCollection->item(_variant_t(L"insert_private_subnet"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"hidden");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"delete_private_subnet"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"private_subnet_address"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            hr = spDispatch->QueryInterface(&spInputText);
            sSectionName.Format(
                _T("private_subnet_address_ip%d"),
                i
                );
	        sIPAddress = GetConfigString(
                _T("service_settings"),
                sSectionName,
                _T("")
                );
            spInputText->value = _bstr_t((LPCTSTR)sIPAddress);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"private_subnet_symbol"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"private_subnet_mask"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            hr = spDispatch->QueryInterface(&spInputText);
            sSectionName.Format(
                _T("private_subnet_mask_ip%d"),
                i
                );
	        sIPAddress = GetConfigString(
                _T("service_settings"),
                sSectionName,
                _T("")
                );
            spInputText->value = _bstr_t((LPCTSTR)sIPAddress);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"test_private_subnet_button"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
        }
    }
    if(nSingleIpPort > 0)
    {
        for(i=0;i<nSingleIpPort;i++)
        {
            hr = GetElement(
                _T("ip_port_row_template"),
                (IHTMLElement**)&spIHTMLElement
                );
            hr = spIHTMLElement->QueryInterface(&spPublicIPTemplateRow);
            nRowIndex = spPublicIPTemplateRow->rowIndex;
            spDispatch = spSettingsTable->insertRow(nRowIndex);
            hr = spDispatch->QueryInterface(&spNewRow);
            hr = GetElement(
                _T("ip_port_description"),
                (IHTMLElement**)&spIHTMLElement
                );
            sCellHTML = spIHTMLElement->innerHTML;
            spDispatch = spNewRow->insertCell(-1);
            hr = spDispatch->QueryInterface(&spNewCell);
            spNewCell->align = _bstr_t(L"right");
            spNewCell->width = _variant_t(L"50%");
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spIHTMLElement->innerHTML = sCellHTML;
            hr = GetElement(
                _T("ip_port_cell"),
                (IHTMLElement**)&spIHTMLElement
                );
            sCellHTML = spIHTMLElement->innerHTML;
            spDispatch = spNewRow->insertCell(-1);
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spIHTMLElement->innerHTML = sCellHTML;
            spDispatch = spIHTMLElement->children;
            hr = spDispatch->QueryInterface(&spHTMLElementCollection);
            spDispatch = spHTMLElementCollection->item(_variant_t(L"insert_ip_port"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"hidden");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"delete_ip_port"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"ip_port"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            hr = spDispatch->QueryInterface(&spInputText);
            sSectionName.Format(
                _T("single_ip_port%d"),
                i
                );
            nIpPort = GetConfigInt(
                _T("service_settings"),
                sSectionName,
                0
                );
	        sIpPort.Format(_T("%d"), nIpPort);
            spInputText->value = _bstr_t((LPCTSTR)sIpPort);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"test_ip_port_button"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
        }
    }
    if(nRangeIpPort > 0)
    {
        for(i=0;i<nRangeIpPort;i++)
        {
            hr = GetElement(
                _T("range_port_row_template"),
                (IHTMLElement**)&spIHTMLElement
                );
            hr = spIHTMLElement->QueryInterface(&spPublicIPTemplateRow);
            nRowIndex = spPublicIPTemplateRow->rowIndex;
            spDispatch = spSettingsTable->insertRow(nRowIndex);
            hr = spDispatch->QueryInterface(&spNewRow);
            hr = GetElement(
                _T("range_ip_description"),
                (IHTMLElement**)&spIHTMLElement
                );
            sCellHTML = spIHTMLElement->innerHTML;
            spDispatch = spNewRow->insertCell(-1);
            hr = spDispatch->QueryInterface(&spNewCell);
            spNewCell->align = _bstr_t(L"right");
            spNewCell->width = _variant_t(L"50%");
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spIHTMLElement->innerHTML = sCellHTML;
            hr = GetElement(
                _T("range_ip_port_cell"),
                (IHTMLElement**)&spIHTMLElement
                );
            sCellHTML = spIHTMLElement->innerHTML;
            spDispatch = spNewRow->insertCell(-1);
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spIHTMLElement->innerHTML = sCellHTML;
            spDispatch = spIHTMLElement->children;
            hr = spDispatch->QueryInterface(&spHTMLElementCollection);
            spDispatch = spHTMLElementCollection->item(_variant_t(L"insert_ip_port_range"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"hidden");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"delete_ip_port_range"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"ip_port_range_begin"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            hr = spDispatch->QueryInterface(&spInputText);
            sSectionName.Format(
                _T("range_begin_ip_port%d"),
                i
                );
            nIpPort = GetConfigInt(
                _T("service_settings"),
                sSectionName,
                0
                );
	        sIpPort.Format(_T("%d"), nIpPort);
            spInputText->value = _bstr_t((LPCTSTR)sIpPort);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"range_symbol02"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"ip_port_range_end"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            hr = spDispatch->QueryInterface(&spInputText);
            sSectionName.Format(
                _T("range_end_ip_port%d"),
                i
                );
            nIpPort = GetConfigInt(
                _T("service_settings"),
                sSectionName,
                0
                );
	        sIpPort.Format(_T("%d"), nIpPort);
            spInputText->value = _bstr_t((LPCTSTR)sIpPort);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"test_ip_port_range_button"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
        }
    }
    m_bHTMLInit = true;
}


HRESULT CNATTraversalConfigDlg::ReloadParameters(IHTMLElement* pElement)
{
    HRESULT hr;
    MSHTML::IHTMLElementPtr     spIHTMLElement;
    MSHTML::IHTMLTableRowPtr    spPublicIPTemplateRow;
    MSHTML::IHTMLTablePtr       spSettingsTable;
    MSHTML::IHTMLTableRowPtr    spNewRow;
    MSHTML::IHTMLStylePtr       spStyle;
    MSHTML::IHTMLTableCellPtr   spNewCell;
    MSHTML::IHTMLButtonElementPtr       spHTMLButtonElement;
    MSHTML::IHTMLInputTextElementPtr    spInputText;
    MSHTML::IHTMLElementCollectionPtr   spHTMLElementCollection;
    long    nRowIndex;
    IDispatchPtr    spDispatch;
    _bstr_t sCellHTML;
    CString sSectionName;
    CString sIPAddress;
    BOOL    bCollection;
    BOOL    bRet;
    SC_HANDLE schSCManager;
    SC_HANDLE schService;
    DWORD   nErrorCode;
    int i, nSingleIP = 0, nRangeIP = 0, nSubnetIP = 0, nSingleIpPort = 0, nRangeIpPort = 0;
    int nSinglePrivateIP = 0, nRangePrivateIP = 0, nSubnetPrivateIP = 0;
    CString sIpPort;
    long    nIpPort;
    UNREFERENCED_PARAMETER(pElement);
    hr = GetElement(
        _T("settings_table"),
        (IHTMLElement**)&spIHTMLElement
        );
    hr = spIHTMLElement->QueryInterface(&spSettingsTable);
    hr = GetElement(
        _T("public_ip"),
        &spDispatch,
        &bCollection
        );
    if(bCollection)
    {
        hr = spDispatch->QueryInterface(&spHTMLElementCollection);
        for(i=spHTMLElementCollection->length-1;i>=0;i--)
        {
            spDispatch = spHTMLElementCollection->item(_variant_t(i));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spIHTMLElement = spIHTMLElement->parentElement;
            if(spIHTMLElement->id == _bstr_t(L"public_ip_cell"))
            {
                continue;
            }
            spIHTMLElement = spIHTMLElement->parentElement;
            hr = spIHTMLElement->QueryInterface(&spPublicIPTemplateRow);
            nRowIndex = spPublicIPTemplateRow->rowIndex;
            hr = spSettingsTable->deleteRow(nRowIndex);
        }
    }
    hr = GetElement(
        _T("range_begin"),
        &spDispatch,
        &bCollection
        );
    if(bCollection)
    {
        hr = spDispatch->QueryInterface(&spHTMLElementCollection);
        for(i=spHTMLElementCollection->length-1;i>=0;i--)
        {
            spDispatch = spHTMLElementCollection->item(_variant_t(i));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spIHTMLElement = spIHTMLElement->parentElement;
            if(spIHTMLElement->id == _bstr_t(L"public_range_cell"))
            {
                continue;
            }
            spIHTMLElement = spIHTMLElement->parentElement;
            hr = spIHTMLElement->QueryInterface(&spPublicIPTemplateRow);
            nRowIndex = spPublicIPTemplateRow->rowIndex;
            hr = spSettingsTable->deleteRow(nRowIndex);
        }
    }
    hr = GetElement(
        _T("subnet_address"),
        &spDispatch,
        &bCollection
        );
    if(bCollection)
    {
        hr = spDispatch->QueryInterface(&spHTMLElementCollection);
        for(i=spHTMLElementCollection->length-1;i>=0;i--)
        {
            spDispatch = spHTMLElementCollection->item(_variant_t(i));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spIHTMLElement = spIHTMLElement->parentElement;
            if(spIHTMLElement->id == _bstr_t(L"public_subnet_cell"))
            {
                continue;
            }
            spIHTMLElement = spIHTMLElement->parentElement;
            hr = spIHTMLElement->QueryInterface(&spPublicIPTemplateRow);
            nRowIndex = spPublicIPTemplateRow->rowIndex;
            hr = spSettingsTable->deleteRow(nRowIndex);
        }
    }
    hr = GetElement(
        _T("private_ip"),
        &spDispatch,
        &bCollection
        );
    if(bCollection)
    {
        hr = spDispatch->QueryInterface(&spHTMLElementCollection);
        for(i=spHTMLElementCollection->length-1;i>=0;i--)
        {
            spDispatch = spHTMLElementCollection->item(_variant_t(i));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spIHTMLElement = spIHTMLElement->parentElement;
            if(spIHTMLElement->id == _bstr_t(L"private_ip_cell"))
            {
                continue;
            }
            spIHTMLElement = spIHTMLElement->parentElement;
            hr = spIHTMLElement->QueryInterface(&spPublicIPTemplateRow);
            nRowIndex = spPublicIPTemplateRow->rowIndex;
            hr = spSettingsTable->deleteRow(nRowIndex);
        }
    }
    hr = GetElement(
        _T("private_range_begin"),
        &spDispatch,
        &bCollection
        );
    if(bCollection)
    {
        hr = spDispatch->QueryInterface(&spHTMLElementCollection);
        for(i=spHTMLElementCollection->length-1;i>=0;i--)
        {
            spDispatch = spHTMLElementCollection->item(_variant_t(i));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spIHTMLElement = spIHTMLElement->parentElement;
            if(spIHTMLElement->id == _bstr_t(L"private_range_cell"))
            {
                continue;
            }
            spIHTMLElement = spIHTMLElement->parentElement;
            hr = spIHTMLElement->QueryInterface(&spPublicIPTemplateRow);
            nRowIndex = spPublicIPTemplateRow->rowIndex;
            hr = spSettingsTable->deleteRow(nRowIndex);
        }
    }
    hr = GetElement(
        _T("private_subnet_address"),
        &spDispatch,
        &bCollection
        );
    if(bCollection)
    {
        hr = spDispatch->QueryInterface(&spHTMLElementCollection);
        for(i=spHTMLElementCollection->length-1;i>=0;i--)
        {
            spDispatch = spHTMLElementCollection->item(_variant_t(i));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spIHTMLElement = spIHTMLElement->parentElement;
            if(spIHTMLElement->id == _bstr_t(L"private_subnet_cell"))
            {
                continue;
            }
            spIHTMLElement = spIHTMLElement->parentElement;
            hr = spIHTMLElement->QueryInterface(&spPublicIPTemplateRow);
            nRowIndex = spPublicIPTemplateRow->rowIndex;
            hr = spSettingsTable->deleteRow(nRowIndex);
        }
    }
    hr = GetElement(
        _T("ip_port"),
        &spDispatch,
        &bCollection
        );
    if(bCollection)
    {
        hr = spDispatch->QueryInterface(&spHTMLElementCollection);
        for(i=spHTMLElementCollection->length-1;i>=0;i--)
        {
            spDispatch = spHTMLElementCollection->item(_variant_t(i));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spIHTMLElement = spIHTMLElement->parentElement;
            if(spIHTMLElement->id == _bstr_t(L"ip_port_cell"))
            {
                continue;
            }
            spIHTMLElement = spIHTMLElement->parentElement;
            hr = spIHTMLElement->QueryInterface(&spPublicIPTemplateRow);
            nRowIndex = spPublicIPTemplateRow->rowIndex;
            hr = spSettingsTable->deleteRow(nRowIndex);
        }
    }
    hr = GetElement(
        _T("ip_port_range_begin"),
        &spDispatch,
        &bCollection
        );
    if(bCollection)
    {
        hr = spDispatch->QueryInterface(&spHTMLElementCollection);
        for(i=spHTMLElementCollection->length-1;i>=0;i--)
        {
            spDispatch = spHTMLElementCollection->item(_variant_t(i));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spIHTMLElement = spIHTMLElement->parentElement;
            if(spIHTMLElement->id == _bstr_t(L"range_ip_port_cell"))
            {
                continue;
            }
            spIHTMLElement = spIHTMLElement->parentElement;
            hr = spIHTMLElement->QueryInterface(&spPublicIPTemplateRow);
            nRowIndex = spPublicIPTemplateRow->rowIndex;
            hr = spSettingsTable->deleteRow(nRowIndex);
        }
    }
	m_sServicesFolder = GetConfigString(
        _T("service_settings"),
        _T("service_folder"),
        theApp.m_sModuleFolder
        );
	m_sPublicRouterIP = GetConfigString(
        _T("service_settings"),
        _T("public_router_ip"),
        _T("")
        );
    schSCManager = OpenSCManager(
        NULL,                 // local machine
        NULL,                 // ServicesActive database
        SC_MANAGER_ALL_ACCESS // full access rights
        );
    if(NULL == schSCManager)
    {
        INT_PTR nDialogRet;
        CRTSDDMessageBox    oRTSDDMessageBox(this);
        nErrorCode = ::GetLastError();
        oRTSDDMessageBox.m_sMessageText.Format(
            _T("\"OpenSCManager\" failed. Error code = %d"),
            nErrorCode
            );
        nDialogRet = oRTSDDMessageBox.DoModal();
        return S_OK;
    }
    hr = GetElement(
        _T("setup_button"),
        (IHTMLElement**)&spIHTMLElement
        );
    hr = spIHTMLElement->QueryInterface(&spHTMLButtonElement);
    schService = OpenService(
        schSCManager,
        _T("NATTraversalService"),
        SERVICE_ALL_ACCESS
        );
    if(NULL == schService)
    {
        nErrorCode = ::GetLastError();
        CloseServiceHandle(schSCManager);
        if(ERROR_SERVICE_DOES_NOT_EXIST != nErrorCode) {
            INT_PTR nDialogRet;
            CRTSDDMessageBox    oRTSDDMessageBox(this);
            oRTSDDMessageBox.m_sMessageText.Format(
                _T("OpenService failed. Error code = %d"),
                nErrorCode
                );
            nDialogRet = oRTSDDMessageBox.DoModal();
            return S_OK;
        }
        spHTMLButtonElement->disabled = VARIANT_FALSE;
        m_sSetupImageSRC = _T("SETUP.GIF");
        hr = GetElement(
            _T("uninstall_button"),
            (IHTMLElement**)&spIHTMLElement
            );
        hr = spIHTMLElement->QueryInterface(&spHTMLButtonElement);
        spHTMLButtonElement->disabled = VARIANT_TRUE;
        m_sUninstallImageSRC = _T("UNINSTALL32_GRAY.PNG");
        hr = GetElement(
            _T("restart_button"),
            (IHTMLElement**)&spIHTMLElement
            );
        hr = spIHTMLElement->QueryInterface(&spHTMLButtonElement);
        spHTMLButtonElement->disabled = VARIANT_TRUE;
        m_sRestartImageSRC = _T("RESTARTHS_GRAY.PNG");
    }
    else
    {
        spHTMLButtonElement->disabled = VARIANT_TRUE;
        m_sSetupImageSRC = _T("SETUP_HIDEN.GIF");
        hr = GetElement(
            _T("uninstall_button"),
            (IHTMLElement**)&spIHTMLElement
            );
        hr = spIHTMLElement->QueryInterface(&spHTMLButtonElement);
        spHTMLButtonElement->disabled = VARIANT_FALSE;
        m_sUninstallImageSRC = _T("UNINSTALL32.PNG");
        hr = GetElement(
            _T("restart_button"),
            (IHTMLElement**)&spIHTMLElement
            );
        hr = spIHTMLElement->QueryInterface(&spHTMLButtonElement);
        spHTMLButtonElement->disabled = VARIANT_FALSE;
        m_sRestartImageSRC = _T("RESTARTHS.PNG");
    }
    bRet = UpdateData(FALSE);
	nSingleIP = GetConfigInt(
        _T("service_settings"),
        _T("public_ip_count"),
        0
        );
	nRangeIP = GetConfigInt(
        _T("service_settings"),
        _T("public_range_count"),
        0
        );
	nSubnetIP = GetConfigInt(
        _T("service_settings"),
        _T("public_subnet_count"),
        0
        );
	nSinglePrivateIP = GetConfigInt(
        _T("service_settings"),
        _T("private_ip_count"),
        0
        );
	nRangePrivateIP = GetConfigInt(
        _T("service_settings"),
        _T("private_range_count"),
        0
        );
	nSubnetPrivateIP = GetConfigInt(
        _T("service_settings"),
        _T("private_subnet_count"),
        0
        );
	nSingleIpPort = GetConfigInt(
        _T("service_settings"),
        _T("ip_port_count"),
        0
        );
	nRangeIpPort = GetConfigInt(
        _T("service_settings"),
        _T("ip_port_range_count"),
        0
        );
    if(nSingleIP > 0)
    {
        for(i=0;i<nSingleIP;i++)
        {
            hr = GetElement(
                _T("public_ip_row_template"),
                (IHTMLElement**)&spIHTMLElement
                );
            hr = spIHTMLElement->QueryInterface(&spPublicIPTemplateRow);
            nRowIndex = spPublicIPTemplateRow->rowIndex;
            spDispatch = spSettingsTable->insertRow(nRowIndex);
            hr = spDispatch->QueryInterface(&spNewRow);
            hr = GetElement(
                _T("public_ip_description"),
                (IHTMLElement**)&spIHTMLElement
                );
            sCellHTML = spIHTMLElement->innerHTML;
            spDispatch = spNewRow->insertCell(-1);
            hr = spDispatch->QueryInterface(&spNewCell);
            spNewCell->align = _bstr_t(L"right");
            spNewCell->width = _variant_t(L"50%");
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spIHTMLElement->innerHTML = sCellHTML;
            hr = GetElement(
                _T("public_ip_cell"),
                (IHTMLElement**)&spIHTMLElement
                );
            sCellHTML = spIHTMLElement->innerHTML;
            spDispatch = spNewRow->insertCell(-1);
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spIHTMLElement->innerHTML = sCellHTML;
            spDispatch = spIHTMLElement->children;
            hr = spDispatch->QueryInterface(&spHTMLElementCollection);
            spDispatch = spHTMLElementCollection->item(_variant_t(L"insert_public_ip"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"hidden");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"delete_public_ip"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"public_ip"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            hr = spDispatch->QueryInterface(&spInputText);
            sSectionName.Format(
                _T("single_ip%d"),
                i
                );
	        sIPAddress = GetConfigString(
                _T("service_settings"),
                sSectionName,
                _T("")
                );
            spInputText->value = _bstr_t((LPCTSTR)sIPAddress);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"test_public_ip_button"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
        }
    }
    if(nRangeIP > 0)
    {
        for(i=0;i<nRangeIP;i++)
        {
            hr = GetElement(
                _T("public_range_row_template"),
                (IHTMLElement**)&spIHTMLElement
                );
            hr = spIHTMLElement->QueryInterface(&spPublicIPTemplateRow);
            nRowIndex = spPublicIPTemplateRow->rowIndex;
            spDispatch = spSettingsTable->insertRow(nRowIndex);
            hr = spDispatch->QueryInterface(&spNewRow);
            hr = GetElement(
                _T("public_range_description"),
                (IHTMLElement**)&spIHTMLElement
                );
            sCellHTML = spIHTMLElement->innerHTML;
            spDispatch = spNewRow->insertCell(-1);
            hr = spDispatch->QueryInterface(&spNewCell);
            spNewCell->align = _bstr_t(L"right");
            spNewCell->width = _variant_t(L"50%");
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spIHTMLElement->innerHTML = sCellHTML;
            hr = GetElement(
                _T("public_range_cell"),
                (IHTMLElement**)&spIHTMLElement
                );
            sCellHTML = spIHTMLElement->innerHTML;
            spDispatch = spNewRow->insertCell(-1);
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spIHTMLElement->innerHTML = sCellHTML;
            spDispatch = spIHTMLElement->children;
            hr = spDispatch->QueryInterface(&spHTMLElementCollection);
            spDispatch = spHTMLElementCollection->item(_variant_t(L"insert_public_range"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"hidden");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"delete_public_range"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"range_begin"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            hr = spDispatch->QueryInterface(&spInputText);
            sSectionName.Format(
                _T("range_begin_ip%d"),
                i
                );
	        sIPAddress = GetConfigString(
                _T("service_settings"),
                sSectionName,
                _T("")
                );
            spInputText->value = _bstr_t((LPCTSTR)sIPAddress);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"range_symbol"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"range_end"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            hr = spDispatch->QueryInterface(&spInputText);
            sSectionName.Format(
                _T("range_end_ip%d"),
                i
                );
	        sIPAddress = GetConfigString(
                _T("service_settings"),
                sSectionName,
                _T("")
                );
            spInputText->value = _bstr_t((LPCTSTR)sIPAddress);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"test_public_range_button"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
        }
    }
    if(nSubnetIP > 0)
    {
        for(i=0;i<nSubnetIP;i++)
        {
            hr = GetElement(
                _T("public_subnet_row_template"),
                (IHTMLElement**)&spIHTMLElement
                );
            hr = spIHTMLElement->QueryInterface(&spPublicIPTemplateRow);
            nRowIndex = spPublicIPTemplateRow->rowIndex;
            spDispatch = spSettingsTable->insertRow(nRowIndex);
            hr = spDispatch->QueryInterface(&spNewRow);
            hr = GetElement(
                _T("public_subnet_description"),
                (IHTMLElement**)&spIHTMLElement
                );
            sCellHTML = spIHTMLElement->innerHTML;
            spDispatch = spNewRow->insertCell(-1);
            hr = spDispatch->QueryInterface(&spNewCell);
            spNewCell->align = _bstr_t(L"right");
            spNewCell->width = _variant_t(L"50%");
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spIHTMLElement->innerHTML = sCellHTML;
            hr = GetElement(
                _T("public_subnet_cell"),
                (IHTMLElement**)&spIHTMLElement
                );
            sCellHTML = spIHTMLElement->innerHTML;
            spDispatch = spNewRow->insertCell(-1);
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spIHTMLElement->innerHTML = sCellHTML;
            spDispatch = spIHTMLElement->children;
            hr = spDispatch->QueryInterface(&spHTMLElementCollection);
            spDispatch = spHTMLElementCollection->item(_variant_t(L"insert_public_subnet"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"hidden");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"delete_public_subnet"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"subnet_address"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            hr = spDispatch->QueryInterface(&spInputText);
            sSectionName.Format(
                _T("subnet_address_ip%d"),
                i
                );
	        sIPAddress = GetConfigString(
                _T("service_settings"),
                sSectionName,
                _T("")
                );
            spInputText->value = _bstr_t((LPCTSTR)sIPAddress);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"subnet_symbol"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"subnet_mask"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            hr = spDispatch->QueryInterface(&spInputText);
            sSectionName.Format(
                _T("subnet_mask_ip%d"),
                i
                );
	        sIPAddress = GetConfigString(
                _T("service_settings"),
                sSectionName,
                _T("")
                );
            spInputText->value = _bstr_t((LPCTSTR)sIPAddress);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"test_public_subnet_button"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
        }
    }
    if(nSinglePrivateIP > 0)
    {
        for(i=0;i<nSinglePrivateIP;i++)
        {
            hr = GetElement(
                _T("private_ip_row_template"),
                (IHTMLElement**)&spIHTMLElement
                );
            hr = spIHTMLElement->QueryInterface(&spPublicIPTemplateRow);
            nRowIndex = spPublicIPTemplateRow->rowIndex;
            spDispatch = spSettingsTable->insertRow(nRowIndex);
            hr = spDispatch->QueryInterface(&spNewRow);
            hr = GetElement(
                _T("private_ip_description"),
                (IHTMLElement**)&spIHTMLElement
                );
            sCellHTML = spIHTMLElement->innerHTML;
            spDispatch = spNewRow->insertCell(-1);
            hr = spDispatch->QueryInterface(&spNewCell);
            spNewCell->align = _bstr_t(L"right");
            spNewCell->width = _variant_t(L"50%");
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spIHTMLElement->innerHTML = sCellHTML;
            hr = GetElement(
                _T("private_ip_cell"),
                (IHTMLElement**)&spIHTMLElement
                );
            sCellHTML = spIHTMLElement->innerHTML;
            spDispatch = spNewRow->insertCell(-1);
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spIHTMLElement->innerHTML = sCellHTML;
            spDispatch = spIHTMLElement->children;
            hr = spDispatch->QueryInterface(&spHTMLElementCollection);
            spDispatch = spHTMLElementCollection->item(_variant_t(L"insert_private_ip"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"hidden");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"delete_private_ip"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"private_ip"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            hr = spDispatch->QueryInterface(&spInputText);
            sSectionName.Format(
                _T("private_single_ip%d"),
                i
                );
	        sIPAddress = GetConfigString(
                _T("service_settings"),
                sSectionName,
                _T("")
                );
            spInputText->value = _bstr_t((LPCTSTR)sIPAddress);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"test_private_ip_button"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
        }
    }
    if(nRangePrivateIP > 0)
    {
        for(i=0;i<nRangePrivateIP;i++)
        {
            hr = GetElement(
                _T("private_range_row_template"),
                (IHTMLElement**)&spIHTMLElement
                );
            hr = spIHTMLElement->QueryInterface(&spPublicIPTemplateRow);
            nRowIndex = spPublicIPTemplateRow->rowIndex;
            spDispatch = spSettingsTable->insertRow(nRowIndex);
            hr = spDispatch->QueryInterface(&spNewRow);
            hr = GetElement(
                _T("private_range_description"),
                (IHTMLElement**)&spIHTMLElement
                );
            sCellHTML = spIHTMLElement->innerHTML;
            spDispatch = spNewRow->insertCell(-1);
            hr = spDispatch->QueryInterface(&spNewCell);
            spNewCell->align = _bstr_t(L"right");
            spNewCell->width = _variant_t(L"50%");
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spIHTMLElement->innerHTML = sCellHTML;
            hr = GetElement(
                _T("private_range_cell"),
                (IHTMLElement**)&spIHTMLElement
                );
            sCellHTML = spIHTMLElement->innerHTML;
            spDispatch = spNewRow->insertCell(-1);
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spIHTMLElement->innerHTML = sCellHTML;
            spDispatch = spIHTMLElement->children;
            hr = spDispatch->QueryInterface(&spHTMLElementCollection);
            spDispatch = spHTMLElementCollection->item(_variant_t(L"insert_private_range"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"hidden");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"delete_private_range"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"private_range_begin"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            hr = spDispatch->QueryInterface(&spInputText);
            sSectionName.Format(
                _T("private_range_begin_ip%d"),
                i
                );
	        sIPAddress = GetConfigString(
                _T("service_settings"),
                sSectionName,
                _T("")
                );
            spInputText->value = _bstr_t((LPCTSTR)sIPAddress);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"private_range_symbol"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"private_range_end"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            hr = spDispatch->QueryInterface(&spInputText);
            sSectionName.Format(
                _T("private_range_end_ip%d"),
                i
                );
	        sIPAddress = GetConfigString(
                _T("service_settings"),
                sSectionName,
                _T("")
                );
            spInputText->value = _bstr_t((LPCTSTR)sIPAddress);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"test_private_range_button"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
        }
    }
    if(nSubnetPrivateIP > 0)
    {
        for(i=0;i<nSubnetPrivateIP;i++)
        {
            hr = GetElement(
                _T("private_subnet_row_template"),
                (IHTMLElement**)&spIHTMLElement
                );
            hr = spIHTMLElement->QueryInterface(&spPublicIPTemplateRow);
            nRowIndex = spPublicIPTemplateRow->rowIndex;
            spDispatch = spSettingsTable->insertRow(nRowIndex);
            hr = spDispatch->QueryInterface(&spNewRow);
            hr = GetElement(
                _T("private_subnet_description"),
                (IHTMLElement**)&spIHTMLElement
                );
            sCellHTML = spIHTMLElement->innerHTML;
            spDispatch = spNewRow->insertCell(-1);
            hr = spDispatch->QueryInterface(&spNewCell);
            spNewCell->align = _bstr_t(L"right");
            spNewCell->width = _variant_t(L"50%");
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spIHTMLElement->innerHTML = sCellHTML;
            hr = GetElement(
                _T("private_subnet_cell"),
                (IHTMLElement**)&spIHTMLElement
                );
            sCellHTML = spIHTMLElement->innerHTML;
            spDispatch = spNewRow->insertCell(-1);
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spIHTMLElement->innerHTML = sCellHTML;
            spDispatch = spIHTMLElement->children;
            hr = spDispatch->QueryInterface(&spHTMLElementCollection);
            spDispatch = spHTMLElementCollection->item(_variant_t(L"insert_private_subnet"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"hidden");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"delete_private_subnet"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"private_subnet_address"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            hr = spDispatch->QueryInterface(&spInputText);
            sSectionName.Format(
                _T("private_subnet_address_ip%d"),
                i
                );
	        sIPAddress = GetConfigString(
                _T("service_settings"),
                sSectionName,
                _T("")
                );
            spInputText->value = _bstr_t((LPCTSTR)sIPAddress);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"private_subnet_symbol"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"private_subnet_mask"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            hr = spDispatch->QueryInterface(&spInputText);
            sSectionName.Format(
                _T("private_subnet_mask_ip%d"),
                i
                );
	        sIPAddress = GetConfigString(
                _T("service_settings"),
                sSectionName,
                _T("")
                );
            spInputText->value = _bstr_t((LPCTSTR)sIPAddress);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"test_private_subnet_button"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
        }
    }
    if(nSingleIpPort > 0)
    {
        for(i=0;i<nSingleIpPort;i++)
        {
            hr = GetElement(
                _T("ip_port_row_template"),
                (IHTMLElement**)&spIHTMLElement
                );
            hr = spIHTMLElement->QueryInterface(&spPublicIPTemplateRow);
            nRowIndex = spPublicIPTemplateRow->rowIndex;
            spDispatch = spSettingsTable->insertRow(nRowIndex);
            hr = spDispatch->QueryInterface(&spNewRow);
            hr = GetElement(
                _T("ip_port_description"),
                (IHTMLElement**)&spIHTMLElement
                );
            sCellHTML = spIHTMLElement->innerHTML;
            spDispatch = spNewRow->insertCell(-1);
            hr = spDispatch->QueryInterface(&spNewCell);
            spNewCell->align = _bstr_t(L"right");
            spNewCell->width = _variant_t(L"50%");
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spIHTMLElement->innerHTML = sCellHTML;
            hr = GetElement(
                _T("ip_port_cell"),
                (IHTMLElement**)&spIHTMLElement
                );
            sCellHTML = spIHTMLElement->innerHTML;
            spDispatch = spNewRow->insertCell(-1);
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spIHTMLElement->innerHTML = sCellHTML;
            spDispatch = spIHTMLElement->children;
            hr = spDispatch->QueryInterface(&spHTMLElementCollection);
            spDispatch = spHTMLElementCollection->item(_variant_t(L"insert_ip_port"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"hidden");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"delete_ip_port"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"ip_port"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            hr = spDispatch->QueryInterface(&spInputText);
            sSectionName.Format(
                _T("single_ip_port%d"),
                i
                );
            nIpPort = GetConfigInt(
                _T("service_settings"),
                sSectionName,
                0
                );
	        sIpPort.Format(_T("%d"), nIpPort);
            spInputText->value = _bstr_t((LPCTSTR)sIpPort);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"test_ip_port_button"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
        }
    }
    if(nRangeIpPort > 0)
    {
        for(i=0;i<nRangeIpPort;i++)
        {
            hr = GetElement(
                _T("range_port_row_template"),
                (IHTMLElement**)&spIHTMLElement
                );
            hr = spIHTMLElement->QueryInterface(&spPublicIPTemplateRow);
            nRowIndex = spPublicIPTemplateRow->rowIndex;
            spDispatch = spSettingsTable->insertRow(nRowIndex);
            hr = spDispatch->QueryInterface(&spNewRow);
            hr = GetElement(
                _T("range_ip_description"),
                (IHTMLElement**)&spIHTMLElement
                );
            sCellHTML = spIHTMLElement->innerHTML;
            spDispatch = spNewRow->insertCell(-1);
            hr = spDispatch->QueryInterface(&spNewCell);
            spNewCell->align = _bstr_t(L"right");
            spNewCell->width = _variant_t(L"50%");
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spIHTMLElement->innerHTML = sCellHTML;
            hr = GetElement(
                _T("range_ip_port_cell"),
                (IHTMLElement**)&spIHTMLElement
                );
            sCellHTML = spIHTMLElement->innerHTML;
            spDispatch = spNewRow->insertCell(-1);
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spIHTMLElement->innerHTML = sCellHTML;
            spDispatch = spIHTMLElement->children;
            hr = spDispatch->QueryInterface(&spHTMLElementCollection);
            spDispatch = spHTMLElementCollection->item(_variant_t(L"insert_ip_port_range"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"hidden");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"delete_ip_port_range"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"ip_port_range_begin"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            hr = spDispatch->QueryInterface(&spInputText);
            sSectionName.Format(
                _T("range_begin_ip_port%d"),
                i
                );
            nIpPort = GetConfigInt(
                _T("service_settings"),
                sSectionName,
                0
                );
	        sIpPort.Format(_T("%d"), nIpPort);
            spInputText->value = _bstr_t((LPCTSTR)sIpPort);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"range_symbol02"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"ip_port_range_end"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            hr = spDispatch->QueryInterface(&spInputText);
            sSectionName.Format(
                _T("range_end_ip_port%d"),
                i
                );
            nIpPort = GetConfigInt(
                _T("service_settings"),
                sSectionName,
                0
                );
	        sIpPort.Format(_T("%d"), nIpPort);
            spInputText->value = _bstr_t((LPCTSTR)sIpPort);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
            spDispatch = spHTMLElementCollection->item(_variant_t(L"test_ip_port_range_button"));
            hr = spDispatch->QueryInterface(&spIHTMLElement);
            spStyle = spIHTMLElement->style;
            spStyle->visibility = _bstr_t(L"visible");
        }
    }
    return S_OK;
}


HRESULT CNATTraversalConfigDlg::OnUninstallClick(IHTMLElement* pElement)
{
    SC_HANDLE schSCManager;
    SC_HANDLE schService;
    DWORD nErrorCode;
    LONG nResult;
    SERVICE_STATUS st;
    BOOL bRet;
    HRESULT hr;
    MSHTML::IHTMLElementPtr spIHTMLElement;
    MSHTML::IHTMLElement2Ptr    spHTMLElement2;
    MSHTML::IHTMLButtonElementPtr  spHTMLButtonElement;
    UNREFERENCED_PARAMETER(pElement);
    // удаление сервиса "NATTraversalService"
    schSCManager = OpenSCManager(
        NULL,                 // local machine
        NULL,                 // ServicesActive database
        SC_MANAGER_ALL_ACCESS // full access rights
        );
    if(NULL == schSCManager) {
        nErrorCode = ::GetLastError();
        INT_PTR nDialogRet;
        CRTSDDMessageBox    oRTSDDMessageBox(this);
        oRTSDDMessageBox.m_sMessageText.Format(
            _T("\"OpenSCManager\" failed!! Error code = %d"),
            nErrorCode
            );
        nDialogRet = oRTSDDMessageBox.DoModal();
        return S_OK;
    }
    schService = OpenService(
        schSCManager,
        _T("NATTraversalService"),
        SERVICE_ALL_ACCESS
        );
    if(NULL == schService) {
        nErrorCode = ::GetLastError();
        CloseServiceHandle(schSCManager);
        if(ERROR_SERVICE_DOES_NOT_EXIST != nErrorCode) {
            INT_PTR nDialogRet;
            CRTSDDMessageBox    oRTSDDMessageBox(this);
            oRTSDDMessageBox.m_sMessageText.Format(
                _T("\"OpenService\" failed!! Error code = %d"),
                nErrorCode
                );
            nDialogRet = oRTSDDMessageBox.DoModal();
        } else {
            INT_PTR nDialogRet;
            CRTSDDMessageBox    oRTSDDMessageBox(this);
            oRTSDDMessageBox.m_sMessageText = _T("Сервис \"NATTraversalService\" не был инсталирован!");
            oRTSDDMessageBox.m_sMessageTypeImageSRC = _T("WARNING_ICON.PNG");
            nDialogRet = oRTSDDMessageBox.DoModal();
            hr = GetElement(
                _T("setup_button"),
                (IHTMLElement**)&spIHTMLElement
                );
            hr = spIHTMLElement->QueryInterface(&spHTMLButtonElement);
            spHTMLButtonElement->disabled = VARIANT_FALSE;
            m_sSetupImageSRC = _T("SETUP.GIF");
            hr = GetElement(
                _T("uninstall_button"),
                (IHTMLElement**)&spIHTMLElement
                );
            hr = spIHTMLElement->QueryInterface(&spHTMLButtonElement);
            spHTMLButtonElement->disabled = VARIANT_TRUE;
            m_sUninstallImageSRC = _T("UNINSTALL32_GRAY.PNG");
            hr = GetElement(
                _T("restart_button"),
                (IHTMLElement**)&spIHTMLElement
                );
            hr = spIHTMLElement->QueryInterface(&spHTMLButtonElement);
            spHTMLButtonElement->disabled = VARIANT_TRUE;
            m_sRestartImageSRC = _T("RESTARTHS_GRAY.PNG");
            bRet = UpdateData(FALSE);
        }
        return S_OK;
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
        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);
        INT_PTR nDialogRet;
        CRTSDDMessageBox    oRTSDDMessageBox(this);
        oRTSDDMessageBox.m_sMessageText.Format(
            _T("\"DeleteService\" failed. Error code = %d!!"),
            nErrorCode
            );
        nDialogRet = oRTSDDMessageBox.DoModal();
        return S_OK;
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
    {
        INT_PTR nDialogRet;
        CRTSDDMessageBox    oRTSDDMessageBox(this);
        oRTSDDMessageBox.m_sMessageText = _T("Сервис \"NATTraversalService\" успешно удалён.");
        oRTSDDMessageBox.m_sMessageTypeImageSRC = _T("INFORMATION_ICON.PNG");
        nDialogRet = oRTSDDMessageBox.DoModal();
        hr = GetElement(
            _T("setup_button"),
            (IHTMLElement**)&spIHTMLElement
            );
        hr = spIHTMLElement->QueryInterface(&spHTMLButtonElement);
        spHTMLButtonElement->disabled = VARIANT_FALSE;
        m_sSetupImageSRC = _T("SETUP.GIF");
        hr = GetElement(
            _T("uninstall_button"),
            (IHTMLElement**)&spIHTMLElement
            );
        hr = spIHTMLElement->QueryInterface(&spHTMLButtonElement);
        spHTMLButtonElement->disabled = VARIANT_TRUE;
        m_sUninstallImageSRC = _T("UNINSTALL32_GRAY.PNG");
        hr = GetElement(
            _T("restart_button"),
            (IHTMLElement**)&spIHTMLElement
            );
        hr = spIHTMLElement->QueryInterface(&spHTMLButtonElement);
        spHTMLButtonElement->disabled = VARIANT_TRUE;
        m_sRestartImageSRC = _T("RESTARTHS_GRAY.PNG");
        bRet = UpdateData(FALSE);
    }
    return S_OK;
}


HRESULT CNATTraversalConfigDlg::OnRestartClick(IHTMLElement* pElement)
{
    SC_HANDLE schSCManager;
    SC_HANDLE schService;
    DWORD nErrorCode;
    SERVICE_STATUS st;
    BOOL bRet;
    HRESULT hr;
    MSHTML::IHTMLElementPtr spIHTMLElement;
    MSHTML::IHTMLElement2Ptr    spHTMLElement2;
    MSHTML::IHTMLButtonElementPtr  spHTMLButtonElement;
    UNREFERENCED_PARAMETER(pElement);
    // перезапуск сервиса "NATTraversalService"
    schSCManager = OpenSCManager(
        NULL,                 // local machine
        NULL,                 // ServicesActive database
        SC_MANAGER_ALL_ACCESS // full access rights
        );
    if(NULL == schSCManager) {
        nErrorCode = ::GetLastError();
        INT_PTR nDialogRet;
        CRTSDDMessageBox    oRTSDDMessageBox(this);
        oRTSDDMessageBox.m_sMessageText.Format(
            _T("\"OpenSCManager\" failed!! Error code = %d"),
            nErrorCode
            );
        nDialogRet = oRTSDDMessageBox.DoModal();
        return S_OK;
    }
    schService = OpenService(
        schSCManager,
        _T("NATTraversalService"),
        SERVICE_ALL_ACCESS
        );
    if(NULL == schService) {
        nErrorCode = ::GetLastError();
        CloseServiceHandle(schSCManager);
        if(ERROR_SERVICE_DOES_NOT_EXIST != nErrorCode) {
            INT_PTR nDialogRet;
            CRTSDDMessageBox    oRTSDDMessageBox(this);
            oRTSDDMessageBox.m_sMessageText.Format(
                _T("\"OpenService\" failed!! Error code = %d"),
                nErrorCode
                );
            nDialogRet = oRTSDDMessageBox.DoModal();
        } else {
            INT_PTR nDialogRet;
            CRTSDDMessageBox    oRTSDDMessageBox(this);
            oRTSDDMessageBox.m_sMessageText = _T("Сервис \"NATTraversalService\" не был инсталирован!");
            oRTSDDMessageBox.m_sMessageTypeImageSRC = _T("WARNING_ICON.PNG");
            nDialogRet = oRTSDDMessageBox.DoModal();
            hr = GetElement(
                _T("setup_button"),
                (IHTMLElement**)&spIHTMLElement
                );
            hr = spIHTMLElement->QueryInterface(&spHTMLButtonElement);
            spHTMLButtonElement->disabled = VARIANT_FALSE;
            m_sSetupImageSRC = _T("SETUP.GIF");
            hr = GetElement(
                _T("uninstall_button"),
                (IHTMLElement**)&spIHTMLElement
                );
            hr = spIHTMLElement->QueryInterface(&spHTMLButtonElement);
            spHTMLButtonElement->disabled = VARIANT_TRUE;
            m_sUninstallImageSRC = _T("UNINSTALL32_GRAY.PNG");
            hr = GetElement(
                _T("restart_button"),
                (IHTMLElement**)&spIHTMLElement
                );
            hr = spIHTMLElement->QueryInterface(&spHTMLButtonElement);
            spHTMLButtonElement->disabled = VARIANT_TRUE;
            m_sRestartImageSRC = _T("RESTARTHS_GRAY.PNG");
            bRet = UpdateData(FALSE);
        }
        return S_OK;
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
    bRet = StartService(
        schService,
        0,
        NULL
        );
    if(!bRet)
    {
        nErrorCode = ::GetLastError();
        INT_PTR nDialogRet;
        CRTSDDMessageBox    oRTSDDMessageBox(this);
        oRTSDDMessageBox.m_sMessageText.Format(
            _T("Неудачная попытка запуска сервиса! Код ошибки = %u (0x%08lX)"),
            nErrorCode,
            nErrorCode
            );
        oRTSDDMessageBox.m_sMessageTypeImageSRC = _T("WARNING_ICON.PNG");
        nDialogRet = oRTSDDMessageBox.DoModal();
    }
    else
    {
        INT_PTR nDialogRet;
        CRTSDDMessageBox    oRTSDDMessageBox(this);
        oRTSDDMessageBox.m_sMessageText = _T("Сервис \"NATTraversalService\" перезапущен с текущими сохранёнными параметрами.");
        oRTSDDMessageBox.m_sMessageTypeImageSRC = _T("INFORMATION_ICON.PNG");
        nDialogRet = oRTSDDMessageBox.DoModal();
    }
    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
    return S_OK;
}


HRESULT CNATTraversalConfigDlg::InsertIpPort(IHTMLElement* pElement)
{
    HRESULT hr;
    MSHTML::IHTMLElementPtr     spIHTMLElement;
    MSHTML::IHTMLElement2Ptr    spIHTMLElement2;
    MSHTML::IHTMLTableRowPtr    spPublicIPTemplateRow;
    long    nRowIndex;
    IDispatchPtr    spDispatch;
    MSHTML::IHTMLTablePtr       spSettingsTable;
    _bstr_t sCellHTML;
    MSHTML::IHTMLTableRowPtr    spNewRow;
    MSHTML::IHTMLElementCollectionPtr   spHTMLElementCollection;
    MSHTML::IHTMLStylePtr       spStyle;
    MSHTML::IHTMLTableCellPtr   spNewCell;
    UNREFERENCED_PARAMETER(pElement);
    hr = GetElement(
        _T("ip_port_row_template"),
        (IHTMLElement**)&spIHTMLElement
        );
    hr = spIHTMLElement->QueryInterface(&spPublicIPTemplateRow);
    nRowIndex = spPublicIPTemplateRow->rowIndex;
    hr = GetElement(
        _T("settings_table"),
        (IHTMLElement**)&spIHTMLElement
        );
    hr = spIHTMLElement->QueryInterface(&spSettingsTable);
    spDispatch = spSettingsTable->insertRow(nRowIndex);
    hr = spDispatch->QueryInterface(&spNewRow);
    hr = GetElement(
        _T("ip_port_description"),
        (IHTMLElement**)&spIHTMLElement
        );
    sCellHTML = spIHTMLElement->innerHTML;
    spDispatch = spNewRow->insertCell(-1);
    hr = spDispatch->QueryInterface(&spNewCell);
    spNewCell->align = _bstr_t(L"right");
    spNewCell->width = _variant_t(L"50%");
    hr = spDispatch->QueryInterface(&spIHTMLElement);
    spIHTMLElement->innerHTML = sCellHTML;
    hr = GetElement(
        _T("ip_port_cell"),
        (IHTMLElement**)&spIHTMLElement
        );
    sCellHTML = spIHTMLElement->innerHTML;
    spDispatch = spNewRow->insertCell(-1);
    hr = spDispatch->QueryInterface(&spIHTMLElement);
    spIHTMLElement->innerHTML = sCellHTML;
    spDispatch = spIHTMLElement->children;
    hr = spDispatch->QueryInterface(&spHTMLElementCollection);
    spDispatch = spHTMLElementCollection->item(_variant_t(L"insert_ip_port"));
    hr = spDispatch->QueryInterface(&spIHTMLElement);
    spStyle = spIHTMLElement->style;
    spStyle->visibility = _bstr_t(L"hidden");
    spDispatch = spHTMLElementCollection->item(_variant_t(L"delete_ip_port"));
    hr = spDispatch->QueryInterface(&spIHTMLElement);
    spStyle = spIHTMLElement->style;
    spStyle->visibility = _bstr_t(L"visible");
    spDispatch = spHTMLElementCollection->item(_variant_t(L"ip_port"));
    hr = spDispatch->QueryInterface(&spIHTMLElement);
    spStyle = spIHTMLElement->style;
    spStyle->visibility = _bstr_t(L"visible");
    hr = spDispatch->QueryInterface(&spIHTMLElement2);
    hr = spIHTMLElement2->focus();
    spDispatch = spHTMLElementCollection->item(_variant_t(L"test_ip_port_button"));
    hr = spDispatch->QueryInterface(&spIHTMLElement);
    spStyle = spIHTMLElement->style;
    spStyle->visibility = _bstr_t(L"visible");
    return S_OK;
}


HRESULT CNATTraversalConfigDlg::DeleteIpPort(IHTMLElement* pElement)
{
    HRESULT hr;
    MSHTML::IHTMLElementPtr     spIHTMLElement;
    MSHTML::IHTMLTableRowPtr    spTableRow;
    long    nRowIndex;
    MSHTML::IHTMLTablePtr       spSettingsTable;
    spIHTMLElement = pElement;
    spIHTMLElement = spIHTMLElement->parentElement;
    spIHTMLElement = spIHTMLElement->parentElement;
    hr = spIHTMLElement->QueryInterface(&spTableRow);
    nRowIndex = spTableRow->rowIndex;
    hr = GetElement(
        _T("settings_table"),
        (IHTMLElement**)&spIHTMLElement
        );
    hr = spIHTMLElement->QueryInterface(&spSettingsTable);
    hr = spSettingsTable->deleteRow(nRowIndex);
    return S_OK;
}


HRESULT CNATTraversalConfigDlg::TesIpPort(IHTMLElement* pElement)
{
    HRESULT hr;
    MSHTML::IHTMLElementPtr spIHTMLElement;
    MSHTML::IHTMLTablePtr   spSettingsTable;
    CString sIpPort;
    long    nIpPort;
    IDispatchPtr    spDispatch;
    MSHTML::IHTMLElementCollectionPtr   spHTMLElementCollection;
    MSHTML::IHTMLInputTextElementPtr    spInputText;
    MSHTML::IHTMLElement2Ptr            spPublicIPElement2;
    spIHTMLElement = pElement;
    spIHTMLElement = spIHTMLElement->parentElement;
    spDispatch = spIHTMLElement->children;
    hr = spDispatch->QueryInterface(&spHTMLElementCollection);
    spDispatch = spHTMLElementCollection->item(_variant_t(L"ip_port"));
    hr = spDispatch->QueryInterface(&spInputText);
    hr = spDispatch->QueryInterface(&spPublicIPElement2);
    sIpPort = (LPCTSTR)spInputText->value;
    nIpPort = _tstol((LPCTSTR)sIpPort);
    if((nIpPort > 0) && (nIpPort < 0x10000))
    {
        INT_PTR nDialogRet;
        CRTSDDMessageBox    oRTSDDMessageBox(this);
        oRTSDDMessageBox.m_sMessageText = _T("Формат значения запрещённого входящего IP-порта верен.");
        oRTSDDMessageBox.m_sMessageTypeImageSRC = _T("INFORMATION_ICON.PNG");
        nDialogRet = oRTSDDMessageBox.DoModal();
    }
    else
    {
        INT_PTR nDialogRet;
        CRTSDDMessageBox    oRTSDDMessageBox(this);
        oRTSDDMessageBox.m_sMessageText = _T("Ошибка формата значения запрещённого входящего IP-порта!!");
        nDialogRet = oRTSDDMessageBox.DoModal();
        hr = spPublicIPElement2->focus();
        hr = spInputText->select();
    }
    return S_OK;
}


HRESULT CNATTraversalConfigDlg::InsertIpPortRange(IHTMLElement* pElement)
{
    HRESULT hr;
    MSHTML::IHTMLElementPtr     spIHTMLElement;
    MSHTML::IHTMLElement2Ptr    spIHTMLElement2;
    MSHTML::IHTMLTableRowPtr    spPublicIPTemplateRow;
    long    nRowIndex;
    IDispatchPtr    spDispatch;
    MSHTML::IHTMLTablePtr       spSettingsTable;
    _bstr_t sCellHTML;
    MSHTML::IHTMLTableRowPtr    spNewRow;
    MSHTML::IHTMLElementCollectionPtr   spHTMLElementCollection;
    MSHTML::IHTMLStylePtr       spStyle;
    MSHTML::IHTMLTableCellPtr   spNewCell;
    UNREFERENCED_PARAMETER(pElement);
    hr = GetElement(
        _T("range_port_row_template"),
        (IHTMLElement**)&spIHTMLElement
        );
    hr = spIHTMLElement->QueryInterface(&spPublicIPTemplateRow);
    nRowIndex = spPublicIPTemplateRow->rowIndex;
    hr = GetElement(
        _T("settings_table"),
        (IHTMLElement**)&spIHTMLElement
        );
    hr = spIHTMLElement->QueryInterface(&spSettingsTable);
    spDispatch = spSettingsTable->insertRow(nRowIndex);
    hr = spDispatch->QueryInterface(&spNewRow);
    hr = GetElement(
        _T("range_ip_description"),
        (IHTMLElement**)&spIHTMLElement
        );
    sCellHTML = spIHTMLElement->innerHTML;
    spDispatch = spNewRow->insertCell(-1);
    hr = spDispatch->QueryInterface(&spNewCell);
    spNewCell->align = _bstr_t(L"right");
    spNewCell->width = _variant_t(L"50%");
    hr = spDispatch->QueryInterface(&spIHTMLElement);
    spIHTMLElement->innerHTML = sCellHTML;
    hr = GetElement(
        _T("range_ip_port_cell"),
        (IHTMLElement**)&spIHTMLElement
        );
    sCellHTML = spIHTMLElement->innerHTML;
    spDispatch = spNewRow->insertCell(-1);
    hr = spDispatch->QueryInterface(&spIHTMLElement);
    spIHTMLElement->innerHTML = sCellHTML;
    spDispatch = spIHTMLElement->children;
    hr = spDispatch->QueryInterface(&spHTMLElementCollection);
    spDispatch = spHTMLElementCollection->item(_variant_t(L"insert_ip_port_range"));
    hr = spDispatch->QueryInterface(&spIHTMLElement);
    spStyle = spIHTMLElement->style;
    spStyle->visibility = _bstr_t(L"hidden");
    spDispatch = spHTMLElementCollection->item(_variant_t(L"delete_ip_port_range"));
    hr = spDispatch->QueryInterface(&spIHTMLElement);
    spStyle = spIHTMLElement->style;
    spStyle->visibility = _bstr_t(L"visible");
    spDispatch = spHTMLElementCollection->item(_variant_t(L"ip_port_range_begin"));
    hr = spDispatch->QueryInterface(&spIHTMLElement);
    spStyle = spIHTMLElement->style;
    spStyle->visibility = _bstr_t(L"visible");
    hr = spDispatch->QueryInterface(&spIHTMLElement2);
    hr = spIHTMLElement2->focus();
    spDispatch = spHTMLElementCollection->item(_variant_t(L"range_symbol02"));
    hr = spDispatch->QueryInterface(&spIHTMLElement);
    spStyle = spIHTMLElement->style;
    spStyle->visibility = _bstr_t(L"visible");
    spDispatch = spHTMLElementCollection->item(_variant_t(L"ip_port_range_end"));
    hr = spDispatch->QueryInterface(&spIHTMLElement);
    spStyle = spIHTMLElement->style;
    spStyle->visibility = _bstr_t(L"visible");
    spDispatch = spHTMLElementCollection->item(_variant_t(L"test_ip_port_range_button"));
    hr = spDispatch->QueryInterface(&spIHTMLElement);
    spStyle = spIHTMLElement->style;
    spStyle->visibility = _bstr_t(L"visible");
    return S_OK;
}


HRESULT CNATTraversalConfigDlg::DeleteIpPortRange(IHTMLElement* pElement)
{
    HRESULT hr;
    MSHTML::IHTMLElementPtr spIHTMLElement;
    MSHTML::IHTMLTableRowPtr    spTableRow;
    long    nRowIndex;
    MSHTML::IHTMLTablePtr   spSettingsTable;
    spIHTMLElement = pElement;
    spIHTMLElement = spIHTMLElement->parentElement;
    spIHTMLElement = spIHTMLElement->parentElement;
    hr = spIHTMLElement->QueryInterface(&spTableRow);
    nRowIndex = spTableRow->rowIndex;
    hr = GetElement(
        _T("settings_table"),
        (IHTMLElement**)&spIHTMLElement
        );
    hr = spIHTMLElement->QueryInterface(&spSettingsTable);
    hr = spSettingsTable->deleteRow(nRowIndex);
    return S_OK;
}


HRESULT CNATTraversalConfigDlg::TestIpPortRange(IHTMLElement* pElement)
{
    HRESULT hr;
    MSHTML::IHTMLElementPtr     spIHTMLElement;
    MSHTML::IHTMLElement2Ptr    spPublicIPElement2;
    MSHTML::IHTMLTablePtr       spSettingsTable;
    CString sIpPort;
    IDispatchPtr    spDispatch;
    MSHTML::IHTMLElementCollectionPtr   spHTMLElementCollection;
    MSHTML::IHTMLInputTextElementPtr    spInputText;
    long    nBeginIpPort, nEndIpPort;
    bool    bBeginIpPortValid, bEndIpPortValid;
    spIHTMLElement = pElement;
    spIHTMLElement = spIHTMLElement->parentElement;
    spDispatch = spIHTMLElement->children;
    hr = spDispatch->QueryInterface(&spHTMLElementCollection);
    spDispatch = spHTMLElementCollection->item(_variant_t(L"ip_port_range_begin"));
    hr = spDispatch->QueryInterface(&spInputText);
    hr = spDispatch->QueryInterface(&spPublicIPElement2);
    sIpPort = (LPCTSTR)spInputText->value;
    nBeginIpPort = _tstol(sIpPort);
    bBeginIpPortValid = ((nBeginIpPort > 0) && (nBeginIpPort < 0x10000));
    if(!bBeginIpPortValid)
    {
        INT_PTR nDialogRet;
        CRTSDDMessageBox    oRTSDDMessageBox(this);
        oRTSDDMessageBox.m_sMessageText = _T("Ошибка формата начального значения диапазона запрещённых входящих IP-портов!!");
        nDialogRet = oRTSDDMessageBox.DoModal();
        hr = spPublicIPElement2->focus();
        hr = spInputText->select();
        return S_OK;
    }
    spDispatch = spHTMLElementCollection->item(_variant_t(L"ip_port_range_end"));
    hr = spDispatch->QueryInterface(&spInputText);
    hr = spDispatch->QueryInterface(&spPublicIPElement2);
    sIpPort = (LPCTSTR)spInputText->value;
    nEndIpPort = _tstol(sIpPort);
    bEndIpPortValid = ((nEndIpPort > 0) && (nEndIpPort < 0x10000));
    if(!bEndIpPortValid)
    {
        INT_PTR nDialogRet;
        CRTSDDMessageBox    oRTSDDMessageBox(this);
        oRTSDDMessageBox.m_sMessageText = _T("Ошибка формата конечного значения диапазона запрещённых входящих IP-портов!!");
        nDialogRet = oRTSDDMessageBox.DoModal();
        hr = spPublicIPElement2->focus();
        hr = spInputText->select();
        return S_OK;
    }
    if(nBeginIpPort > nEndIpPort)
    {
        INT_PTR nDialogRet;
        CRTSDDMessageBox    oRTSDDMessageBox(this);
        oRTSDDMessageBox.m_sMessageText = _T("Начальное значение диапазона больше конечного!!");
        nDialogRet = oRTSDDMessageBox.DoModal();
        hr = spPublicIPElement2->focus();
        hr = spInputText->select();
        return S_OK;
    }
    if(nBeginIpPort == nEndIpPort)
    {
        INT_PTR nDialogRet;
        CRTSDDMessageBox    oRTSDDMessageBox(this);
        oRTSDDMessageBox.m_sMessageText = _T("Начальное значение диапазона равно конечному, т.е задан единственный запрещённый входящий IP-порт!");
        oRTSDDMessageBox.m_sMessageTypeImageSRC = _T("WARNING_ICON.PNG");
        nDialogRet = oRTSDDMessageBox.DoModal();
        hr = spPublicIPElement2->focus();
        hr = spInputText->select();
    }
    else
    {
        INT_PTR nDialogRet;
        CRTSDDMessageBox    oRTSDDMessageBox(this);
        oRTSDDMessageBox.m_sMessageText = _T("Формат диапазона запрещённых входящих IP-портов верен.");
        oRTSDDMessageBox.m_sMessageTypeImageSRC = _T("INFORMATION_ICON.PNG");
        nDialogRet = oRTSDDMessageBox.DoModal();
    }
    return S_OK;
}


HRESULT CNATTraversalConfigDlg::InsertPrivateIP(IHTMLElement* pElement)
{
    HRESULT hr;
    MSHTML::IHTMLElementPtr     spIHTMLElement;
    MSHTML::IHTMLElement2Ptr    spIHTMLElement2;
    MSHTML::IHTMLTableRowPtr    spPublicIPTemplateRow;
    long    nRowIndex;
    IDispatchPtr    spDispatch;
    MSHTML::IHTMLTablePtr       spSettingsTable;
    _bstr_t sCellHTML;
    MSHTML::IHTMLTableRowPtr    spNewRow;
    MSHTML::IHTMLElementCollectionPtr   spHTMLElementCollection;
    MSHTML::IHTMLStylePtr       spStyle;
    MSHTML::IHTMLTableCellPtr   spNewCell;
    UNREFERENCED_PARAMETER(pElement);
    hr = GetElement(
        _T("private_ip_row_template"),
        (IHTMLElement**)&spIHTMLElement
        );
    hr = spIHTMLElement->QueryInterface(&spPublicIPTemplateRow);
    nRowIndex = spPublicIPTemplateRow->rowIndex;
    hr = GetElement(
        _T("settings_table"),
        (IHTMLElement**)&spIHTMLElement
        );
    hr = spIHTMLElement->QueryInterface(&spSettingsTable);
    spDispatch = spSettingsTable->insertRow(nRowIndex);
    hr = spDispatch->QueryInterface(&spNewRow);
    hr = GetElement(
        _T("private_ip_description"),
        (IHTMLElement**)&spIHTMLElement
        );
    sCellHTML = spIHTMLElement->innerHTML;
    spDispatch = spNewRow->insertCell(-1);
    hr = spDispatch->QueryInterface(&spNewCell);
    spNewCell->align = _bstr_t(L"right");
    spNewCell->width = _variant_t(L"50%");
    hr = spDispatch->QueryInterface(&spIHTMLElement);
    spIHTMLElement->innerHTML = sCellHTML;
    hr = GetElement(
        _T("private_ip_cell"),
        (IHTMLElement**)&spIHTMLElement
        );
    sCellHTML = spIHTMLElement->innerHTML;
    spDispatch = spNewRow->insertCell(-1);
    hr = spDispatch->QueryInterface(&spIHTMLElement);
    spIHTMLElement->innerHTML = sCellHTML;
    spDispatch = spIHTMLElement->children;
    hr = spDispatch->QueryInterface(&spHTMLElementCollection);
    spDispatch = spHTMLElementCollection->item(_variant_t(L"insert_private_ip"));
    hr = spDispatch->QueryInterface(&spIHTMLElement);
    spStyle = spIHTMLElement->style;
    spStyle->visibility = _bstr_t(L"hidden");
    spDispatch = spHTMLElementCollection->item(_variant_t(L"delete_private_ip"));
    hr = spDispatch->QueryInterface(&spIHTMLElement);
    spStyle = spIHTMLElement->style;
    spStyle->visibility = _bstr_t(L"visible");
    spDispatch = spHTMLElementCollection->item(_variant_t(L"private_ip"));
    hr = spDispatch->QueryInterface(&spIHTMLElement);
    spStyle = spIHTMLElement->style;
    spStyle->visibility = _bstr_t(L"visible");
    hr = spDispatch->QueryInterface(&spIHTMLElement2);
    hr = spIHTMLElement2->focus();
    spDispatch = spHTMLElementCollection->item(_variant_t(L"test_private_ip_button"));
    hr = spDispatch->QueryInterface(&spIHTMLElement);
    spStyle = spIHTMLElement->style;
    spStyle->visibility = _bstr_t(L"visible");
    return S_OK;
}


HRESULT CNATTraversalConfigDlg::DeletePrivateIP(IHTMLElement* pElement)
{
    HRESULT hr;
    MSHTML::IHTMLElementPtr     spIHTMLElement;
    MSHTML::IHTMLTableRowPtr    spTableRow;
    long    nRowIndex;
    MSHTML::IHTMLTablePtr       spSettingsTable;
    spIHTMLElement = pElement;
    spIHTMLElement = spIHTMLElement->parentElement;
    spIHTMLElement = spIHTMLElement->parentElement;
    hr = spIHTMLElement->QueryInterface(&spTableRow);
    nRowIndex = spTableRow->rowIndex;
    hr = GetElement(
        _T("settings_table"),
        (IHTMLElement**)&spIHTMLElement
        );
    hr = spIHTMLElement->QueryInterface(&spSettingsTable);
    hr = spSettingsTable->deleteRow(nRowIndex);
    return S_OK;
}


HRESULT CNATTraversalConfigDlg::TestPrivateIP(IHTMLElement* pElement)
{
    HRESULT hr;
    MSHTML::IHTMLElementPtr spIHTMLElement;
    MSHTML::IHTMLTablePtr   spSettingsTable;
    CString sIPAddress;
    IDispatchPtr    spDispatch;
    MSHTML::IHTMLElementCollectionPtr   spHTMLElementCollection;
    MSHTML::IHTMLInputTextElementPtr    spInputText;
    LONG    nRet;
    LPCTSTR pTerminator;
    IN_ADDR stIP4Address;
    MSHTML::IHTMLElement2Ptr    spPublicIPElement2;
    spIHTMLElement = pElement;
    spIHTMLElement = spIHTMLElement->parentElement;
    spDispatch = spIHTMLElement->children;
    hr = spDispatch->QueryInterface(&spHTMLElementCollection);
    spDispatch = spHTMLElementCollection->item(_variant_t(L"private_ip"));
    hr = spDispatch->QueryInterface(&spInputText);
    hr = spDispatch->QueryInterface(&spPublicIPElement2);
    sIPAddress = (LPCTSTR)spInputText->value;
    nRet = RtlIpv4StringToAddress(
        sIPAddress,
        TRUE,
        &pTerminator,
        &stIP4Address
        );
    if(NO_ERROR != nRet)
    {
        INT_PTR nDialogRet;
        CRTSDDMessageBox    oRTSDDMessageBox(this);
        oRTSDDMessageBox.m_sMessageText = _T("Ошибка формата IP-адреса!!");
        nDialogRet = oRTSDDMessageBox.DoModal();
        hr = spPublicIPElement2->focus();
        hr = spInputText->select();
    }
    else
    {
        INT_PTR nDialogRet;
        CRTSDDMessageBox    oRTSDDMessageBox(this);
        oRTSDDMessageBox.m_sMessageText = _T("Формат приватного IP-адреса верен.");
        oRTSDDMessageBox.m_sMessageTypeImageSRC = _T("INFORMATION_ICON.PNG");
        nDialogRet = oRTSDDMessageBox.DoModal();
    }
    return S_OK;
}


HRESULT CNATTraversalConfigDlg::InsertPrivateRange(IHTMLElement* pElement)
{
    HRESULT hr;
    MSHTML::IHTMLElementPtr     spIHTMLElement;
    MSHTML::IHTMLElement2Ptr    spIHTMLElement2;
    MSHTML::IHTMLTableRowPtr    spPublicIPTemplateRow;
    long    nRowIndex;
    IDispatchPtr    spDispatch;
    MSHTML::IHTMLTablePtr       spSettingsTable;
    _bstr_t sCellHTML;
    MSHTML::IHTMLTableRowPtr    spNewRow;
    MSHTML::IHTMLElementCollectionPtr   spHTMLElementCollection;
    MSHTML::IHTMLStylePtr       spStyle;
    MSHTML::IHTMLTableCellPtr   spNewCell;
    UNREFERENCED_PARAMETER(pElement);
    hr = GetElement(
        _T("private_range_row_template"),
        (IHTMLElement**)&spIHTMLElement
        );
    hr = spIHTMLElement->QueryInterface(&spPublicIPTemplateRow);
    nRowIndex = spPublicIPTemplateRow->rowIndex;
    hr = GetElement(
        _T("settings_table"),
        (IHTMLElement**)&spIHTMLElement
        );
    hr = spIHTMLElement->QueryInterface(&spSettingsTable);
    spDispatch = spSettingsTable->insertRow(nRowIndex);
    hr = spDispatch->QueryInterface(&spNewRow);
    hr = GetElement(
        _T("private_range_description"),
        (IHTMLElement**)&spIHTMLElement
        );
    sCellHTML = spIHTMLElement->innerHTML;
    spDispatch = spNewRow->insertCell(-1);
    hr = spDispatch->QueryInterface(&spNewCell);
    spNewCell->align = _bstr_t(L"right");
    spNewCell->width = _variant_t(L"50%");
    hr = spDispatch->QueryInterface(&spIHTMLElement);
    spIHTMLElement->innerHTML = sCellHTML;
    hr = GetElement(
        _T("private_range_cell"),
        (IHTMLElement**)&spIHTMLElement
        );
    sCellHTML = spIHTMLElement->innerHTML;
    spDispatch = spNewRow->insertCell(-1);
    hr = spDispatch->QueryInterface(&spIHTMLElement);
    spIHTMLElement->innerHTML = sCellHTML;
    spDispatch = spIHTMLElement->children;
    hr = spDispatch->QueryInterface(&spHTMLElementCollection);
    spDispatch = spHTMLElementCollection->item(_variant_t(L"insert_private_range"));
    hr = spDispatch->QueryInterface(&spIHTMLElement);
    spStyle = spIHTMLElement->style;
    spStyle->visibility = _bstr_t(L"hidden");
    spDispatch = spHTMLElementCollection->item(_variant_t(L"delete_private_range"));
    hr = spDispatch->QueryInterface(&spIHTMLElement);
    spStyle = spIHTMLElement->style;
    spStyle->visibility = _bstr_t(L"visible");
    spDispatch = spHTMLElementCollection->item(_variant_t(L"private_range_begin"));
    hr = spDispatch->QueryInterface(&spIHTMLElement);
    spStyle = spIHTMLElement->style;
    spStyle->visibility = _bstr_t(L"visible");
    hr = spDispatch->QueryInterface(&spIHTMLElement2);
    hr = spIHTMLElement2->focus();
    spDispatch = spHTMLElementCollection->item(_variant_t(L"private_range_symbol"));
    hr = spDispatch->QueryInterface(&spIHTMLElement);
    spStyle = spIHTMLElement->style;
    spStyle->visibility = _bstr_t(L"visible");
    spDispatch = spHTMLElementCollection->item(_variant_t(L"private_range_end"));
    hr = spDispatch->QueryInterface(&spIHTMLElement);
    spStyle = spIHTMLElement->style;
    spStyle->visibility = _bstr_t(L"visible");
    spDispatch = spHTMLElementCollection->item(_variant_t(L"test_private_range_button"));
    hr = spDispatch->QueryInterface(&spIHTMLElement);
    spStyle = spIHTMLElement->style;
    spStyle->visibility = _bstr_t(L"visible");
    return S_OK;
}


HRESULT CNATTraversalConfigDlg::DeletePrivateRange(IHTMLElement* pElement)
{
    HRESULT hr;
    MSHTML::IHTMLElementPtr     spIHTMLElement;
    MSHTML::IHTMLTableRowPtr    spTableRow;
    long    nRowIndex;
    MSHTML::IHTMLTablePtr       spSettingsTable;
    spIHTMLElement = pElement;
    spIHTMLElement = spIHTMLElement->parentElement;
    spIHTMLElement = spIHTMLElement->parentElement;
    hr = spIHTMLElement->QueryInterface(&spTableRow);
    nRowIndex = spTableRow->rowIndex;
    hr = GetElement(
        _T("settings_table"),
        (IHTMLElement**)&spIHTMLElement
        );
    hr = spIHTMLElement->QueryInterface(&spSettingsTable);
    hr = spSettingsTable->deleteRow(nRowIndex);
    return S_OK;
}


HRESULT CNATTraversalConfigDlg::TestPrivateRange(IHTMLElement* pElement)
{
    HRESULT hr;
    MSHTML::IHTMLElementPtr spIHTMLElement;
    MSHTML::IHTMLTablePtr   spSettingsTable;
    CString sIPAddress;
    IDispatchPtr    spDispatch;
    MSHTML::IHTMLElementCollectionPtr   spHTMLElementCollection;
    MSHTML::IHTMLInputTextElementPtr    spInputText;
    LONG    nRet;
    LPCTSTR pTerminator;
    IN_ADDR stIP4Address;
    MSHTML::IHTMLElement2Ptr    spPublicIPElement2;
    u_long  nhBeginAddress, nhEndAddress;
    spIHTMLElement = pElement;
    spIHTMLElement = spIHTMLElement->parentElement;
    spDispatch = spIHTMLElement->children;
    hr = spDispatch->QueryInterface(&spHTMLElementCollection);
    spDispatch = spHTMLElementCollection->item(_variant_t(L"private_range_begin"));
    hr = spDispatch->QueryInterface(&spInputText);
    hr = spDispatch->QueryInterface(&spPublicIPElement2);
    sIPAddress = (LPCTSTR)spInputText->value;
    nRet = RtlIpv4StringToAddress(
        sIPAddress,
        TRUE,
        &pTerminator,
        &stIP4Address
        );
    if(NO_ERROR != nRet)
    {
        INT_PTR nDialogRet;
        CRTSDDMessageBox    oRTSDDMessageBox(this);
        oRTSDDMessageBox.m_sMessageText = _T("Ошибка формата начального значения диапазона приватных IP-адресов!!");
        nDialogRet = oRTSDDMessageBox.DoModal();
        hr = spPublicIPElement2->focus();
        hr = spInputText->select();
        return S_OK;
    }
    nhBeginAddress = ntohl(stIP4Address.S_un.S_addr);
    spDispatch = spHTMLElementCollection->item(_variant_t(L"private_range_end"));
    hr = spDispatch->QueryInterface(&spInputText);
    hr = spDispatch->QueryInterface(&spPublicIPElement2);
    sIPAddress = (LPCTSTR)spInputText->value;
    nRet = RtlIpv4StringToAddress(
        sIPAddress,
        TRUE,
        &pTerminator,
        &stIP4Address
        );
    if(NO_ERROR != nRet)
    {
        INT_PTR nDialogRet;
        CRTSDDMessageBox    oRTSDDMessageBox(this);
        oRTSDDMessageBox.m_sMessageText = _T("Ошибка формата конечного значения диапазона приватных IP-адресов!!");
        nDialogRet = oRTSDDMessageBox.DoModal();
        hr = spPublicIPElement2->focus();
        hr = spInputText->select();
        return S_OK;
    }
    nhEndAddress = ntohl(stIP4Address.S_un.S_addr);
    if(nhBeginAddress > nhEndAddress)
    {
        INT_PTR nDialogRet;
        CRTSDDMessageBox    oRTSDDMessageBox(this);
        oRTSDDMessageBox.m_sMessageText = _T("Значение начального адреса диапазона больше конечного!!");
        nDialogRet = oRTSDDMessageBox.DoModal();
        hr = spPublicIPElement2->focus();
        hr = spInputText->select();
        return S_OK;
    }
    if(nhBeginAddress == nhEndAddress)
    {
        INT_PTR nDialogRet;
        CRTSDDMessageBox    oRTSDDMessageBox(this);
        oRTSDDMessageBox.m_sMessageText = _T("Значение начального адреса диапазона равно конечному, т.е задан единственный IP-адрес!");
        oRTSDDMessageBox.m_sMessageTypeImageSRC = _T("WARNING_ICON.PNG");
        nDialogRet = oRTSDDMessageBox.DoModal();
        hr = spPublicIPElement2->focus();
        hr = spInputText->select();
    }
    else
    {
        INT_PTR nDialogRet;
        CRTSDDMessageBox    oRTSDDMessageBox(this);
        oRTSDDMessageBox.m_sMessageText = _T("Формат диапазона приватных IP-адресов верен.");
        oRTSDDMessageBox.m_sMessageTypeImageSRC = _T("INFORMATION_ICON.PNG");
        nDialogRet = oRTSDDMessageBox.DoModal();
    }
    return S_OK;
}


HRESULT CNATTraversalConfigDlg::InsertPrivateSubnet(IHTMLElement* pElement)
{
    HRESULT hr;
    MSHTML::IHTMLElementPtr     spIHTMLElement;
    MSHTML::IHTMLElement2Ptr    spIHTMLElement2;
    MSHTML::IHTMLTableRowPtr    spPublicIPTemplateRow;
    long    nRowIndex;
    IDispatchPtr    spDispatch;
    MSHTML::IHTMLTablePtr       spSettingsTable;
    _bstr_t sCellHTML;
    MSHTML::IHTMLTableRowPtr    spNewRow;
    MSHTML::IHTMLElementCollectionPtr   spHTMLElementCollection;
    MSHTML::IHTMLStylePtr       spStyle;
    MSHTML::IHTMLTableCellPtr   spNewCell;
    UNREFERENCED_PARAMETER(pElement);
    hr = GetElement(
        _T("private_subnet_row_template"),
        (IHTMLElement**)&spIHTMLElement
        );
    hr = spIHTMLElement->QueryInterface(&spPublicIPTemplateRow);
    nRowIndex = spPublicIPTemplateRow->rowIndex;
    hr = GetElement(
        _T("settings_table"),
        (IHTMLElement**)&spIHTMLElement
        );
    hr = spIHTMLElement->QueryInterface(&spSettingsTable);
    spDispatch = spSettingsTable->insertRow(nRowIndex);
    hr = spDispatch->QueryInterface(&spNewRow);
    hr = GetElement(
        _T("private_subnet_description"),
        (IHTMLElement**)&spIHTMLElement
        );
    sCellHTML = spIHTMLElement->innerHTML;
    spDispatch = spNewRow->insertCell(-1);
    hr = spDispatch->QueryInterface(&spNewCell);
    spNewCell->align = _bstr_t(L"right");
    spNewCell->width = _variant_t(L"50%");
    hr = spDispatch->QueryInterface(&spIHTMLElement);
    spIHTMLElement->innerHTML = sCellHTML;
    hr = GetElement(
        _T("private_subnet_cell"),
        (IHTMLElement**)&spIHTMLElement
        );
    sCellHTML = spIHTMLElement->innerHTML;
    spDispatch = spNewRow->insertCell(-1);
    hr = spDispatch->QueryInterface(&spIHTMLElement);
    spIHTMLElement->innerHTML = sCellHTML;
    spDispatch = spIHTMLElement->children;
    hr = spDispatch->QueryInterface(&spHTMLElementCollection);
    spDispatch = spHTMLElementCollection->item(_variant_t(L"insert_private_subnet"));
    hr = spDispatch->QueryInterface(&spIHTMLElement);
    spStyle = spIHTMLElement->style;
    spStyle->visibility = _bstr_t(L"hidden");
    spDispatch = spHTMLElementCollection->item(_variant_t(L"delete_private_subnet"));
    hr = spDispatch->QueryInterface(&spIHTMLElement);
    spStyle = spIHTMLElement->style;
    spStyle->visibility = _bstr_t(L"visible");
    spDispatch = spHTMLElementCollection->item(_variant_t(L"private_subnet_address"));
    hr = spDispatch->QueryInterface(&spIHTMLElement);
    spStyle = spIHTMLElement->style;
    spStyle->visibility = _bstr_t(L"visible");
    hr = spDispatch->QueryInterface(&spIHTMLElement2);
    hr = spIHTMLElement2->focus();
    spDispatch = spHTMLElementCollection->item(_variant_t(L"private_subnet_symbol"));
    hr = spDispatch->QueryInterface(&spIHTMLElement);
    spStyle = spIHTMLElement->style;
    spStyle->visibility = _bstr_t(L"visible");
    spDispatch = spHTMLElementCollection->item(_variant_t(L"private_subnet_mask"));
    hr = spDispatch->QueryInterface(&spIHTMLElement);
    spStyle = spIHTMLElement->style;
    spStyle->visibility = _bstr_t(L"visible");
    spDispatch = spHTMLElementCollection->item(_variant_t(L"test_private_subnet_button"));
    hr = spDispatch->QueryInterface(&spIHTMLElement);
    spStyle = spIHTMLElement->style;
    spStyle->visibility = _bstr_t(L"visible");
    return S_OK;
}


HRESULT CNATTraversalConfigDlg::DeletePrivateSubnet(IHTMLElement* pElement)
{
    HRESULT hr;
    MSHTML::IHTMLElementPtr     spIHTMLElement;
    MSHTML::IHTMLTableRowPtr    spTableRow;
    long    nRowIndex;
    MSHTML::IHTMLTablePtr       spSettingsTable;
    spIHTMLElement = pElement;
    spIHTMLElement = spIHTMLElement->parentElement;
    spIHTMLElement = spIHTMLElement->parentElement;
    hr = spIHTMLElement->QueryInterface(&spTableRow);
    nRowIndex = spTableRow->rowIndex;
    hr = GetElement(
        _T("settings_table"),
        (IHTMLElement**)&spIHTMLElement
        );
    hr = spIHTMLElement->QueryInterface(&spSettingsTable);
    hr = spSettingsTable->deleteRow(nRowIndex);
    return S_OK;
}


HRESULT CNATTraversalConfigDlg::TestPrivateSubnet(IHTMLElement* pElement)
{
    HRESULT hr;
    MSHTML::IHTMLElementPtr spIHTMLElement;
    MSHTML::IHTMLTablePtr   spSettingsTable;
    CString sIPAddress;
    IDispatchPtr    spDispatch;
    MSHTML::IHTMLElementCollectionPtr   spHTMLElementCollection;
    MSHTML::IHTMLInputTextElementPtr    spInputText;
    LONG    nRet;
    LPCTSTR pTerminator;
    IN_ADDR stIP4Address;
    MSHTML::IHTMLElement2Ptr    spPublicIPElement2;
    spIHTMLElement = pElement;
    spIHTMLElement = spIHTMLElement->parentElement;
    spDispatch = spIHTMLElement->children;
    hr = spDispatch->QueryInterface(&spHTMLElementCollection);
    spDispatch = spHTMLElementCollection->item(_variant_t(L"private_subnet_address"));
    hr = spDispatch->QueryInterface(&spInputText);
    hr = spDispatch->QueryInterface(&spPublicIPElement2);
    sIPAddress = (LPCTSTR)spInputText->value;
    nRet = RtlIpv4StringToAddress(
        sIPAddress,
        TRUE,
        &pTerminator,
        &stIP4Address
        );
    if(NO_ERROR != nRet)
    {
        INT_PTR nDialogRet;
        CRTSDDMessageBox    oRTSDDMessageBox(this);
        oRTSDDMessageBox.m_sMessageText = _T("Ошибка формата адреса приватной IP-подсети!!");
        nDialogRet = oRTSDDMessageBox.DoModal();
        hr = spPublicIPElement2->focus();
        hr = spInputText->select();
        return S_OK;
    }
    spDispatch = spHTMLElementCollection->item(_variant_t(L"private_subnet_mask"));
    hr = spDispatch->QueryInterface(&spInputText);
    hr = spDispatch->QueryInterface(&spPublicIPElement2);
    sIPAddress = (LPCTSTR)spInputText->value;
    nRet = RtlIpv4StringToAddress(
        sIPAddress,
        TRUE,
        &pTerminator,
        &stIP4Address
        );
    if(NO_ERROR != nRet)
    {
        INT_PTR nDialogRet;
        CRTSDDMessageBox    oRTSDDMessageBox(this);
        oRTSDDMessageBox.m_sMessageText = _T("Ошибка формата маски приватной IP-подсети!!");
        nDialogRet = oRTSDDMessageBox.DoModal();
        hr = spPublicIPElement2->focus();
        hr = spInputText->select();
    }
    else
    {
        INT_PTR nDialogRet;
        CRTSDDMessageBox    oRTSDDMessageBox(this);
        oRTSDDMessageBox.m_sMessageText = _T("Формат приватной IP-подсети верен.");
        oRTSDDMessageBox.m_sMessageTypeImageSRC = _T("INFORMATION_ICON.PNG");
        nDialogRet = oRTSDDMessageBox.DoModal();
    }
    return S_OK;
}


void CNATTraversalConfigDlg::OnCancel()
{
}


void CNATTraversalConfigDlg::OnOK()
{
}
