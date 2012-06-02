// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// world_editor_graph_plugin_dlg.cpp : implementation file
//

#include "stdafx.h"
#include "nel/misc/types_nl.h"
#include <fstream>
#include <iostream>
#include <objidl.h>
#include <string.h>
#include <vector>
#include "nel/misc/common.h"
#include "graph_plugin.h"
#include "world_editor_graph_plugin_dlg.h"

using namespace NLMISC;
using namespace NLLIGO;
using namespace std;

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnPaint();
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}
void CAboutDlg::OnPaint()/*:CDialog::OnPaint()*/
{
	CDialog::OnPaint();		
}
void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWorldEditorGraphPluginDlg dialog

CWorldEditorGraphPluginDlg::CWorldEditorGraphPluginDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CWorldEditorGraphPluginDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CWorldEditorGraphPluginDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CWorldEditorGraphPluginDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWorldEditorGraphPluginDlg)
	DDX_Control(pDX, IDC_BORDER, m_border);
	DDX_Control(pDX, IDC_IMAGE_FRAME, m_mainFrame);
	DDX_Control(pDX, IDC_SCROLLBAR_VERT, m_scroll_vert);
	DDX_Control(pDX, IDC_SCROLLBAR_HORZ, m_scroll_horz);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CWorldEditorGraphPluginDlg, CDialog)
	//{{AFX_MSG_MAP(CWorldEditorGraphPluginDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_Z_50, OnZ50)
	ON_BN_CLICKED(IDC_Z_100, OnZ100)
	ON_BN_CLICKED(IDC_Z_150, OnZ150)
	ON_BN_CLICKED(IDC_Z_200, OnZ200)
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_BN_CLICKED(IDREFRESH, OnRefresh)
	ON_WM_CLOSE()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWorldEditorGraphPluginDlg message handlers

BOOL CWorldEditorGraphPluginDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	RECT rect;
	m_mainFrame.GetClientRect(&rect);
	m_mainFrame.ClientToScreen(&rect);
	ScreenToClient(&rect);
	aff_size.left = rect.left;
	aff_size.top = rect.top;
	aff_size.right = rect.right;
	aff_size.bottom = rect.bottom;

	magnifier=1;
	#undef  new
	_DibBits=NULL;

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CWorldEditorGraphPluginDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CWorldEditorGraphPluginDlg::OnPaint() 
{
	CDialog::OnPaint();

	HDC hdc(m_mainFrame.GetDC()->GetSafeHdc());
	RECT rect;
	RECT test;
	GetClientRect(&test);

	int maxX,minX;
	m_scroll_horz.GetScrollRange(&minX,&maxX);
//////////////////////////////////////
	m_mainFrame.GetClientRect(&rect);
	ClientToScreen(&rect);
	ScreenToClient(&rect);

	aff_size.left = rect.left;
	aff_size.top = rect.top;
	aff_size.right = (LONG)(rect.left+_DibBitmapInfo.bmiHeader.biWidth*(float)magnifier);
	aff_size.bottom = (LONG)(rect.top+_DibBitmapInfo.bmiHeader.biHeight*(float)magnifier);

	::CRect img_src_size;

	img_src_size.left = 0;
	img_src_size.top = 0;
	img_src_size.right = _DibBitmapInfo.bmiHeader.biWidth;
	img_src_size.bottom = _DibBitmapInfo.bmiHeader.biHeight;
	
	int decX=m_scroll_horz.GetScrollPos(),decY=m_scroll_vert.GetScrollPos();

	uint32 imgWidLd=uint32(_DibBitmapInfo.bmiHeader.biWidth*(float)magnifier),imgHgtLd=uint32(_DibBitmapInfo.bmiHeader.biHeight*(float)magnifier);

	if(rect.right-rect.left<_DibBitmapInfo.bmiHeader.biWidth*(float)magnifier)
	{
		imgWidLd=rect.right-rect.left;
		img_src_size.left=LONG(decX/(float)magnifier);
		img_src_size.right=LONG(img_src_size.left+imgWidLd/(float)magnifier);
		aff_size.right = rect.right;
	}

	if(rect.bottom-rect.top<_DibBitmapInfo.bmiHeader.biHeight*(float)magnifier)
	{
		imgHgtLd=rect.bottom-rect.top;
		img_src_size.top=LONG(_DibBitmapInfo.bmiHeader.biHeight-imgHgtLd/(float)magnifier-decY/(float)magnifier);
		img_src_size.bottom=LONG(_DibBitmapInfo.bmiHeader.biHeight-decY/(float)magnifier);
		aff_size.bottom=rect.bottom;
	}


	StretchDIBits(
					  hdc,							// handle to DC
					  aff_size.left,                   // x-coord of destination upper-left corner
					  aff_size.top,                   // y-coord of destination upper-left corner
					  aff_size.Width(),               // width of destination rectangle
					  aff_size.Height(),              // height of destination rectangle
					  img_src_size.left,                   // x-coord of source upper-left corner
					  img_src_size.top,                   // y-coord of source upper-left corner
					  img_src_size.Width(),						// width of source rectangle
					  img_src_size.Height(),						// height of source rectangle
					  _DibBits,						// bitmap bits
					  &_DibBitmapInfo,				// bitmap data
					  DIB_RGB_COLORS,							// usage options
					  SRCCOPY								// raster operation code
					);

	
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CWorldEditorGraphPluginDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CWorldEditorGraphPluginDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// TODO: Add your message handler code here and/or call default
	int CurPos = pScrollBar->GetScrollPos();
	int maxpos=0,minpos=0;
//	INT imax, imin;
	m_scroll_horz.GetScrollRange(&minpos, &maxpos);

	// Determine the new position of scroll box.
	switch (nSBCode)
	{
	case SB_LEFT:      // Scroll to far left.
		CurPos = 0;
		break;

	case SB_RIGHT:      // Scroll to far right.
		CurPos = maxpos;
		break;

	case SB_ENDSCROLL:   // End scroll.
		break;

	case SB_LINELEFT:      // Scroll left.
		if (CurPos > 0)
			CurPos--;
		break;

	case SB_LINERIGHT:   // Scroll right.
		if (CurPos < maxpos)
			CurPos++;
		break;

	case SB_PAGELEFT:    // Scroll one page left.
		{
			// Get the page size. 
			SCROLLINFO   info;
			pScrollBar->GetScrollInfo(&info, SIF_ALL);
   
			if (CurPos > 0)
				CurPos = std::max(0, CurPos - (int) info.nPage);
		}
		break;

	case SB_PAGERIGHT:      // Scroll one page right
		{
			// Get the page size. 
			SCROLLINFO   info;
			pScrollBar->GetScrollInfo(&info, SIF_ALL);

			if (CurPos < maxpos)
				CurPos = std::min(maxpos, CurPos + (int) info.nPage);
		}
		break;

	case SB_THUMBPOSITION: // Scroll to absolute position. nPos is the position
		CurPos = nPos;      // of the scroll box at the end of the drag operation.
		break;

	case SB_THUMBTRACK:   // Drag scroll box to specified position. nPos is the
		CurPos = nPos;     // position that the scroll box has been dragged to.
		break;
	}

	// Set the new position of the thumb (scroll box).
	pScrollBar->SetScrollPos(CurPos);
	
	CDialog::OnVScroll(nSBCode, nPos, pScrollBar);
	RECT dlgRect;
	m_mainFrame.GetClientRect(&dlgRect);
	m_mainFrame.ClientToScreen(&dlgRect);
	ScreenToClient(&dlgRect);
	InvalidateRect(&dlgRect);
}


void CWorldEditorGraphPluginDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// TODO: Add your message handler code here and/or call default
	int CurPos = pScrollBar->GetScrollPos();
	int maxpos=0,minpos=0;
//	LPINT lpmax=NULL,lpmin=NULL;
	m_scroll_horz.GetScrollRange(&minpos, &maxpos);

	// Determine the new position of scroll box.
	switch (nSBCode)
	{
	case SB_TOP:      // Scroll to far left.
		CurPos = 0;
		break;

	case SB_BOTTOM:      // Scroll to far right.
		CurPos = maxpos;
		break;

	case SB_ENDSCROLL:   // End scroll.
		break;

	case SB_LINEUP:      // Scroll left.
		if (CurPos > 0)
			CurPos--;
		break;

	case SB_LINEDOWN:   // Scroll right.
		if (CurPos <maxpos)
			CurPos++;
		break;

	case SB_PAGEUP:    // Scroll one page left.
		{
			// Get the page size. 
			SCROLLINFO   info;
			pScrollBar->GetScrollInfo(&info, SIF_ALL);
   
			if (CurPos > 0)
				CurPos = std::max(0, CurPos - (int) info.nPage);
		}
		break;

	case SB_PAGEDOWN:      // Scroll one page right
		{
			// Get the page size. 
			SCROLLINFO   info;
			pScrollBar->GetScrollInfo(&info, SIF_ALL);

			if (CurPos < maxpos)
				CurPos = std::min(maxpos, CurPos + (int) info.nPage);
		}
		break;

	case SB_THUMBPOSITION: // Scroll to absolute position. nPos is the position
		CurPos = nPos;      // of the scroll box at the end of the drag operation.
		break;

	case SB_THUMBTRACK:   // Drag scroll box to specified position. nPos is the
		CurPos = nPos;     // position that the scroll box has been dragged to.
		break;
	}

	// Set the new position of the thumb (scroll box).
	pScrollBar->SetScrollPos(CurPos);
	
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
	
	RECT dlgRect;
	m_mainFrame.GetClientRect(&dlgRect);
	m_mainFrame.ClientToScreen(&dlgRect);
	ScreenToClient(&dlgRect);
	InvalidateRect(&dlgRect);
//	Invalidate();
}



void CWorldEditorGraphPluginDlg::OnZ50() 
{
	magnifier=0.5;

	rescaleScroll();
	RECT dlgRect;
	m_mainFrame.GetClientRect(&dlgRect);
	m_mainFrame.ClientToScreen(&dlgRect);
	ScreenToClient(&dlgRect);
	InvalidateRect(&dlgRect);
//	Invalidate();
}

void CWorldEditorGraphPluginDlg::OnZ100() 
{
	// TODO: Add your control notification handler code here
	magnifier=1;

	rescaleScroll();
	RECT dlgRect;
	m_mainFrame.GetClientRect(&dlgRect);
	m_mainFrame.ClientToScreen(&dlgRect);
	ScreenToClient(&dlgRect);
	InvalidateRect(&dlgRect);
//	Invalidate();
}

void CWorldEditorGraphPluginDlg::OnZ150() 
{
	// TODO: Add your control notification handler code here
	magnifier=1.5;

	rescaleScroll();
	RECT dlgRect;
	m_mainFrame.GetClientRect(&dlgRect);
	m_mainFrame.ClientToScreen(&dlgRect);
	ScreenToClient(&dlgRect);
	InvalidateRect(&dlgRect);
//	Invalidate();
}

void CWorldEditorGraphPluginDlg::OnZ200() 
{
	// TODO: Add your control notification handler code here
	magnifier=2;

	rescaleScroll();
	RECT dlgRect;
	m_mainFrame.GetClientRect(&dlgRect);
	m_mainFrame.ClientToScreen(&dlgRect);
	ScreenToClient(&dlgRect);
	InvalidateRect(&dlgRect);
//	Invalidate();
}

void CWorldEditorGraphPluginDlg::OnRButtonDown(UINT nFlags, CPoint point) 
{

	if(_DibBits!=NULL)
	{
		//get the correct coordinates
		ClientToScreen(&point);
		m_mainFrame.ScreenToClient(&point);
		::CRect rect;
		m_mainFrame.GetClientRect(&rect);

		CPaintDC dc(this);

		sint32 tmpAff;
		if (isWithin(rect,point))
		{
			if (((point.x-getXCenter(rect) +m_scroll_horz.GetScrollPos())<magnifier * _DibBitmapInfo.bmiHeader.biWidth )&&((point.x-getXCenter(rect) +m_scroll_horz.GetScrollPos())>0))
			{
				tmpAff=point.x-getXCenter(rect) +m_scroll_horz.GetScrollPos();
				m_scroll_horz.SetScrollPos(tmpAff);
			}
			else 
			{
				if ((point.x-getXCenter(rect) +m_scroll_horz.GetScrollPos())<0)
				{
					tmpAff=0;
					m_scroll_horz.SetScrollPos(tmpAff);
				}
				else 
				{
					tmpAff = sint32(magnifier*_DibBitmapInfo.bmiHeader.biWidth-getXCenter(rect));
					m_scroll_horz.SetScrollPos(tmpAff);
				}
			}

		
			if (((point.y-getYCenter(rect) +m_scroll_vert.GetScrollPos())>0)&&((point.y-getYCenter(rect) +m_scroll_vert.GetScrollPos())<_DibBitmapInfo.bmiHeader.biHeight*magnifier))
			{
				tmpAff=point.y-getYCenter(rect) +m_scroll_vert.GetScrollPos();
				m_scroll_vert.SetScrollPos(tmpAff);
			}
			else 
			{
				if ((point.y-getYCenter(rect) +m_scroll_vert.GetScrollPos())<0)
				{
					tmpAff=0;
					m_scroll_vert.SetScrollPos(tmpAff);
				}
				else 
				{
					tmpAff=sint32(magnifier*_DibBitmapInfo.bmiHeader.biHeight-getYCenter(rect));
					m_scroll_vert.SetScrollPos(tmpAff);
				}
			}
		}
		CDialog::OnRButtonDown(nFlags, point);
		RECT dlgRect;
		m_mainFrame.GetClientRect(&dlgRect);
		m_mainFrame.ClientToScreen(&dlgRect);
		ScreenToClient(&dlgRect);
		InvalidateRect(&dlgRect);
	}
}

void CWorldEditorGraphPluginDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{

	if(_DibBits!=NULL)
	{	
		// TODO: Add your message handler code here and/or call default
		string tmpPath = string(::getenv("TEMP"));
		ifstream cmap((tmpPath+"/output.imap").c_str());

		if(cmap.is_open())
		{

			//get the correct coordinates
			ClientToScreen(&point);
			m_mainFrame.ScreenToClient(&point);			
			
			char *bufferCH=new char[256];
			cmap.getline(bufferCH,256);	
			std::string strBuff(bufferCH);
			cmap.getline(bufferCH,256);
			strBuff=bufferCH;
			CPoint iRes2,iRes3;
			char *ret=new char[256];
			std::vector<std::string> strRes,strRes2,strRes3;
			strcpy(ret,"Nothing to point to");
			double cmpx = 1/magnifier, cmpy = 1/magnifier;
			
			::CRect rect;
			m_mainFrame.GetClientRect(&rect);
			::CRect tok;
			int x,y;
			
			while(strcmp(bufferCH,"")!=0)
			{
				
				x=point.x;
				y=point.y;
				
				try{
				explode(strBuff,string(" "),strRes,true);
				explode(strRes.at(2),string(","),strRes2,true);
				explode(strRes.at(3),string(","),strRes3,true);
				tok.left=atoi(strRes2.at(0).c_str());
				tok.top=atoi(strRes2.at(1).c_str());
				tok.right=atoi(strRes3.at(0).c_str());
				tok.bottom=atoi(strRes3.at(1).c_str());
				}catch (exception e) {return;}

				
				
				x = int(x*cmpx);
				y = int(y*cmpy);
				x = int(x+(m_scroll_horz.GetScrollPos())*cmpx);
				y = int(y+(m_scroll_vert.GetScrollPos())*cmpy);
			
				

				if(isWithin(tok,CPoint(x,y)))
				{			
					//strcpy(ret,.c_str());


					graph_plug->doSelection(strRes.at(1));

					
					break;
				}

				cmap.getline(bufferCH,256);
				strBuff=bufferCH;
				
			}
			
			cmap.close();
			delete []ret;
			delete []bufferCH;
		}
	}
	
	CDialog::OnLButtonDown(nFlags, point);

}



void CWorldEditorGraphPluginDlg::OnRefresh() 
{
	// TODO: Add your control notification handler code here

	graph_plug->refreshPrimitives();
	OnZ100();
	rescaleScroll();
	RECT dlgRect;
	m_mainFrame.GetClientRect(&dlgRect);
	m_mainFrame.ClientToScreen(&dlgRect);
	ScreenToClient(&dlgRect);
	InvalidateRect(&dlgRect);
}

void CWorldEditorGraphPluginDlg::init(CGraphPlugin* plugin)
{
	graph_plug=plugin;
}

void CWorldEditorGraphPluginDlg::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	graph_plug->unsetDlgGraph();
	CDialog::OnClose();
}

CWorldEditorGraphPluginDlg::~CWorldEditorGraphPluginDlg()
{
}

void CWorldEditorGraphPluginDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);

	// 1st place the 'border' relative to new dialg size
	RECT dlgRect;
	GetClientRect(&dlgRect);
	ClientToScreen(&dlgRect);
	RECT rect;
	m_border.GetWindowRect(&rect);
	rect.left = dlgRect.left;
	rect.right = dlgRect.right;
	rect.bottom = dlgRect.bottom;
	ScreenToClient(&rect);
	m_border.MoveWindow(&rect, true);

	// 2nd, place other component relative to border client surface
	m_border.GetClientRect(&dlgRect);
	m_border.ClientToScreen(&dlgRect);

	m_scroll_vert.GetWindowRect(&rect);
	rect.top = dlgRect.top;
	rect.left = dlgRect.right-16;
	rect.right = dlgRect.right;
	rect.bottom = dlgRect.bottom-16;
	ScreenToClient(&rect);
	m_scroll_vert.MoveWindow(&rect, true);

	m_scroll_horz.GetWindowRect(&rect);
	rect.left = dlgRect.left;
	rect.right = dlgRect.right-16;
	rect.top = dlgRect.bottom-16;
	rect.bottom = dlgRect.bottom;
	ScreenToClient(&rect);
	m_scroll_horz.MoveWindow(&rect, true);

	m_mainFrame.GetWindowRect(&rect);
	rect.top = dlgRect.top;
	rect.left = dlgRect.left;
	rect.right = dlgRect.right-16;
	rect.bottom = dlgRect.bottom-16;
	ScreenToClient(&rect);
	m_mainFrame.MoveWindow(&rect, true);

	rescaleScroll();
	Invalidate();
	

}

void CWorldEditorGraphPluginDlg::rescaleScroll()
{
	if (_DibBits)
	{
		RECT rect;
		m_mainFrame.GetClientRect(&rect);

		// recompute slider size
		m_scroll_vert.EnableWindow(TRUE);
		m_scroll_horz.EnableWindow(TRUE);
		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_RANGE | SIF_PAGE | SIF_DISABLENOSCROLL;
		si.nMin = 0;
		si.nMax = int(_DibBitmapInfo.bmiHeader.biHeight*magnifier);
		si.nPage = rect.bottom;
		
		m_scroll_vert.SetScrollInfo(&si, true);
		si.nMax = int(_DibBitmapInfo.bmiHeader.biWidth*magnifier);
		si.nPage = rect.right;
		m_scroll_horz.SetScrollInfo(&si, true);
	}
	else
	{
		m_scroll_vert.EnableWindow(FALSE);
		m_scroll_horz.EnableWindow(FALSE);
	}
}


bool CWorldEditorGraphPluginDlg::isWithin(::CRect &rect,CPoint &point)
{
	if (rect.left<point.x)
	{
		if (rect.right>point.x)
		{
			if (rect.bottom>point.y)
			{
				if (rect.top<point.y)
				{
					return true;
				}
			}
		}
	}
	return false;
}

int CWorldEditorGraphPluginDlg::getXCenter(::CRect &rect)
{
	return((sint32)rect.Width()/2);
}
int CWorldEditorGraphPluginDlg::getYCenter(::CRect &rect)
{
	return((sint32)rect.Height()/2);
}

void CWorldEditorGraphPluginDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	graph_plug->unsetDlgGraph();
	CDialog::OnCancel();
}

