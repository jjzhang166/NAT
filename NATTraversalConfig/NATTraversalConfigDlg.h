
// NATTraversalConfigDlg.h : файл заголовка
//

#pragma once


// диалоговое окно CNATTraversalConfigDlg
class CNATTraversalConfigDlg : public CDHtmlDialog
{
// Создание
public:
	CNATTraversalConfigDlg(CWnd* pParent = NULL);	// стандартный конструктор

// Данные диалогового окна
	enum { IDD = IDD_NATTRAVERSALCONFIG_DIALOG, IDH = IDR_HTML_NATTRAVERSALCONFIG_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// поддержка DDX/DDV

	HRESULT OnButtonOK(IHTMLElement *pElement);
	HRESULT OnButtonCancel(IHTMLElement *pElement);

// Реализация
protected:
	HICON m_hIcon;

	// Созданные функции схемы сообщений
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	DECLARE_DHTML_EVENT_MAP()
public:
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnClose();
    bool m_bNoBreakStage;
    CString m_sFileVersion;
    HRESULT OnSetupClick(IHTMLElement* pElement);
    HRESULT OnExitClick(IHTMLElement* pElement);
    HRESULT DoAboutBox(IHTMLElement* pElement);
    afx_msg void OnDestroy();
    STDMETHOD(ShowContextMenu)(
        DWORD dwID,
        POINT* ppt,
        IUnknown* pcmdtReserved,
        IDispatch* pdispReserved
        );
    STDMETHOD(GetHostInfo)(
       DOCHOSTUIINFO *pInfo
    );
    CString m_sStatusStr;
    CString m_sServicesFolder;
    HRESULT SelectServicePath(IHTMLElement* pElement);
    CString m_sPublicRouterIP;
    HRESULT TestRouteIP(IHTMLElement* pElement);
    HRESULT InsertPublicIP(IHTMLElement* pElement);
    HRESULT DeletePublicIP(IHTMLElement* pElement);
    HRESULT TestPublicIP(IHTMLElement* pElement);
    HRESULT InsertPublicRange(IHTMLElement* pElement);
    HRESULT DeletePublicRange(IHTMLElement* pElement);
    HRESULT TestPublicRange(IHTMLElement* pElement);
    HRESULT InsertPublicSubnet(IHTMLElement* pElement);
    HRESULT DeletePublicSubnet(IHTMLElement* pElement);
    HRESULT TestPublicSubnet(IHTMLElement* pElement);
    CString m_sSetupImageSRC;
    HRESULT SaveParameters(IHTMLElement* pElement);
    virtual void OnDocumentComplete(LPDISPATCH pDisp, LPCTSTR szUrl);
    bool m_bHTMLInit;
    HRESULT ReloadParameters(IHTMLElement* pElement);
    bool m_bServiceInstalled;
    CString m_sUninstallImageSRC;
    CString m_sRestartImageSRC;
    HRESULT OnUninstallClick(IHTMLElement* pElement);
    HRESULT OnRestartClick(IHTMLElement* pElement);
    HRESULT InsertIpPort(IHTMLElement* pElement);
    HRESULT DeleteIpPort(IHTMLElement* pElement);
    HRESULT TesIpPort(IHTMLElement* pElement);
    HRESULT InsertIpPortRange(IHTMLElement* pElement);
    HRESULT DeleteIpPortRange(IHTMLElement* pElement);
    HRESULT TestIpPortRange(IHTMLElement* pElement);
    HRESULT InsertPrivateIP(IHTMLElement* pElement);
    HRESULT DeletePrivateIP(IHTMLElement* pElement);
    HRESULT TestPrivateIP(IHTMLElement* pElement);
    HRESULT InsertPrivateRange(IHTMLElement* pElement);
    HRESULT DeletePrivateRange(IHTMLElement* pElement);
    HRESULT TestPrivateRange(IHTMLElement* pElement);
    HRESULT InsertPrivateSubnet(IHTMLElement* pElement);
    HRESULT DeletePrivateSubnet(IHTMLElement* pElement);
    HRESULT TestPrivateSubnet(IHTMLElement* pElement);
    virtual void OnCancel();
    virtual void OnOK();
};
