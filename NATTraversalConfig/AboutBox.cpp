// AboutBox.cpp : implementation file
//

#include "stdafx.h"
#include "NATTraversalConfig.h"
#include "AboutBox.h"


// CAboutBox dialog

IMPLEMENT_DYNCREATE(CAboutBox, CDHtmlDialog)

CAboutBox::CAboutBox(CWnd* pParent /*=NULL*/)
	: CDHtmlDialog(_T("ABOUTBOX"), _T("ABOUTBOX.HTM"), pParent)
    , m_sFileVersion(theApp.m_sVersionHTMLString)
{
    m_bUseHtmlTitle = TRUE;
}

CAboutBox::~CAboutBox()
{
}

void CAboutBox::DoDataExchange(CDataExchange* pDX)
{
    DDX_DHtml_ElementInnerHtml(
        pDX,
        _T("version_str"),
        m_sFileVersion
        );
	CDHtmlDialog::DoDataExchange(pDX);
}

BOOL CAboutBox::OnInitDialog()
{
	CDHtmlDialog::OnInitDialog();
	return TRUE;  // return TRUE  unless you set the focus to a control
}

BEGIN_MESSAGE_MAP(CAboutBox, CDHtmlDialog)
END_MESSAGE_MAP()

BEGIN_DHTML_EVENT_MAP(CAboutBox)
	DHTML_EVENT_ONCLICK(_T("ButtonOK"), OnButtonOK)
	//DHTML_EVENT_ONCLICK(_T("ButtonCancel"), OnButtonCancel)
END_DHTML_EVENT_MAP()



// CAboutBox message handlers

HRESULT CAboutBox::OnButtonOK(IHTMLElement* /*pElement*/)
{
	OnOK();
	return S_OK;
}

HRESULT CAboutBox::OnButtonCancel(IHTMLElement* /*pElement*/)
{
	OnCancel();
	return S_OK;
}
