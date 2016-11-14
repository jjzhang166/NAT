#pragma once

#ifdef _WIN32_WCE
#error "CDHtmlDialog is not supported for Windows CE."
#endif 

// CRTSDDMessageBox dialog

class CRTSDDMessageBox : public CDHtmlDialog
{
	DECLARE_DYNCREATE(CRTSDDMessageBox)

public:
	CRTSDDMessageBox(CWnd* pParent = NULL);   // standard constructor
	virtual ~CRTSDDMessageBox();
// Overrides
	HRESULT OnButtonOK(IHTMLElement *pElement);
	HRESULT OnButtonCancel(IHTMLElement *pElement);

// Dialog Data
	enum { IDD = IDD_RTSDDMESSAGEBOX, IDH = IDR_HTML_RTSDDMESSAGEBOX };

	DECLARE_MESSAGE_MAP()
	DECLARE_DHTML_EVENT_MAP()
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

public:
    CString m_sFileVersion;
    CString m_sMessageText;
    CString m_sMessageTypeImageSRC;
};
