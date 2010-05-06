// custom.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "custom.h"


/////////////////////////////////////////////////////////////////////////////
// Custom dialog


Custom::Custom(CWnd* pParent /*=NULL*/)
	: CDialog(Custom::IDD, pParent)
{
	//{{AFX_DATA_INIT(Custom)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void Custom::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(Custom)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(Custom, CDialog)
	//{{AFX_MSG_MAP(Custom)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Custom message handlers

void Custom::Free(void)
{
//	for (int i = 0;i<nButton;i++) delete buttonList[i];
}

void Custom::OnOK() 
{
	// TODO: Add extra validation here
	Free();	
	bOk = 1;
	CButton *b = (CButton*)GetDlgItem(IDC_OR);
	if (b->GetCheck()) mode = 0;
	else mode = 1;
	CDialog::OnOK();	
}

void Custom::OnCancel() 
{
	// TODO: Add extra cleanup here
	Free();
	CDialog::OnCancel();
	bOk = 0;
}

LRESULT Custom::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (message==WM_INITDIALOG)
	{
		CButton *b = (CButton*)GetDlgItem(IDC_OR);
		b->SetCheck(1);
		
		flag = 0;
		nButton = clist->GetCount()-2;
		buttonList = new CButton[nButton];
		staticList = new CStatic[nButton];
		RECT client,button;
		GetClientRect(&client);
		button.top = client.top + 15;
		button.bottom = button.top + 15;
		button.left = client.left + 20;
		button.right = client.right - 90; //button.left + 15;
		if (nButton>4) SetWindowPos(0,0,0,client.right - client.left,15*2 + (button.bottom - button.top + 10)*nButton + 10,SWP_NOMOVE);
		font.CreateFont(-10,0,0,0,FW_THIN,false,false,false,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,FIXED_PITCH,NULL);
		for (int i = 0;i<nButton;i++)
		{
			CString str;
			clist->GetLBText(i+2,str);
			buttonList[i].Create((LPCSTR)str,BS_CHECKBOX,button,this,i+10);
			buttonList[i].SetFont(&font,1);
			buttonList[i].ModifyStyle(0,WS_VISIBLE);
/*			RECT st = button; st.left+=20; st.right = client.right - 90; st.top -= 3;
			staticList[i].Create(str,0,st,this,i+10);
			staticList[i].ModifyStyle(0,WS_VISIBLE);*/
			button.top += 10 + 10;
			button.bottom += 10 + 10;
		}
	}
	else if (message==WM_COMMAND)
	{
		int button = LOWORD(wParam)-10;
		if (button>=0 && button<=(nButton)) 
		{
			buttonList[button].SetCheck(buttonList[button].GetCheck()==0?1:0);
			__int64 add2flag=1;
			for (int i = 0;i<button;i++) add2flag<<=1;
			if (buttonList[button].GetCheck()) flag|=add2flag;
			else flag^=add2flag;
		}
	}
	return CDialog::WindowProc(message, wParam, lParam);
}
