#pragma once

#ifdef _WIN32_WCE
#error "CDHtmlDialog is not supported for Windows CE."
#endif 

// CAboutBox dialog

class CAboutBox : public CDHtmlDialog
{
	DECLARE_DYNCREATE(CAboutBox)

public:
	CAboutBox(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAboutBox();
// Overrides
	HRESULT OnButtonOK(IHTMLElement *pElement);
	HRESULT OnButtonCancel(IHTMLElement *pElement);

// Dialog Data
	enum { IDD = IDD_ABOUTBOX, IDH = IDR_HTML_ABOUTBOX };

	DECLARE_MESSAGE_MAP()
	DECLARE_DHTML_EVENT_MAP()
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

public:
    CString m_sFileVersion;
};
