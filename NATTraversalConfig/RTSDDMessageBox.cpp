// RTSDDMassageBox.cpp : implementation file
//

#include "stdafx.h"
#include "NATTraversalConfig.h"
#include "RTSDDMessageBox.h"


// CRTSDDMessageBox dialog

IMPLEMENT_DYNCREATE(CRTSDDMessageBox, CDHtmlDialog)

CRTSDDMessageBox::CRTSDDMessageBox(CWnd* pParent /*=NULL*/)
	: CDHtmlDialog(_T("RTSDDMESSAGEBOX"), _T("RTSDDMESSAGEBOX.HTM"), pParent)
    , m_sFileVersion(theApp.m_sVersionHTMLString)
    , m_sMessageText(_T(""))
    , m_sMessageTypeImageSRC(_T("ERROR_ICON.PNG"))
{
    m_bUseHtmlTitle = TRUE;
}

CRTSDDMessageBox::~CRTSDDMessageBox()
{
}

void CRTSDDMessageBox::DoDataExchange(CDataExchange* pDX)
{
    DDX_DHtml_ElementInnerHtml(
        pDX,
        _T("version_str"),
        m_sFileVersion
        );
    DDX_DHtml_ElementInnerText(
        pDX,
        _T("message_text"),
        m_sMessageText
        );
    DDX_DHtml_Img_Src(
        pDX,
        _T("message_type_img"),
        m_sMessageTypeImageSRC
        );
	CDHtmlDialog::DoDataExchange(pDX);
}

BOOL CRTSDDMessageBox::OnInitDialog()
{
	CDHtmlDialog::OnInitDialog();
	return TRUE;  // return TRUE  unless you set the focus to a control
}

BEGIN_MESSAGE_MAP(CRTSDDMessageBox, CDHtmlDialog)
END_MESSAGE_MAP()

BEGIN_DHTML_EVENT_MAP(CRTSDDMessageBox)
	DHTML_EVENT_ONCLICK(_T("ButtonOK"), OnButtonOK)
	//DHTML_EVENT_ONCLICK(_T("ButtonCancel"), OnButtonCancel)
END_DHTML_EVENT_MAP()



// CRTSDDMessageBox message handlers

HRESULT CRTSDDMessageBox::OnButtonOK(IHTMLElement* /*pElement*/)
{
	OnOK();
	return S_OK;
}

HRESULT CRTSDDMessageBox::OnButtonCancel(IHTMLElement* /*pElement*/)
{
	OnCancel();
	return S_OK;
}
