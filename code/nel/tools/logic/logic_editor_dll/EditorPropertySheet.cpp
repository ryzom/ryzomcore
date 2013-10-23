// EditorPropertySheet.cpp : implementation file
//

#include "stdafx.h"
#include "logic_editor.h"
#include "EditorPropertySheet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEditorPropertySheet

IMPLEMENT_DYNAMIC(CEditorPropertySheet, CPropertySheet)

CEditorPropertySheet::CEditorPropertySheet(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
	AddPage(&m_variablePage);
	AddPage(&m_counterPage);
	AddPage(&m_conditionPage);	
	AddPage(&m_statePage);
}

CEditorPropertySheet::CEditorPropertySheet(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
	AddPage(&m_variablePage);
	AddPage(&m_counterPage);
	AddPage(&m_conditionPage);
	AddPage(&m_statePage);
}

CEditorPropertySheet::~CEditorPropertySheet()
{
}


BEGIN_MESSAGE_MAP(CEditorPropertySheet, CPropertySheet)
	//{{AFX_MSG_MAP(CEditorPropertySheet)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CEditorPropertySheet message handlers
