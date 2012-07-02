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

#include "stdafx.h"
#include "resource.h"
#include "ViewColumn.h"
#include "Browse.h"

char *WndRegKeys[4][5] = {{"POPUP BOTTOM RX","POPUP BOTTOM RY","POPUP BOTTOM CX","POPUP BOTTOM CY","POPUP BOTTOM N"},
						{"POPUP TOP RX","POPUP TOP RY","POPUP TOP CX","POPUP TOP CY","POPUP TOP N"},
						{"POPUP LEFT RX","POPUP LEFT RY","POPUP LEFT CX","POPUP LEFT CY","POPUP LEFT N"},
						{"POPUP RIGHT RX","POPUP RIGHT RY","POPUP RIGHT CX","POPUP RIGHT CY","POPUP RIGHT N"}};

/////////////////////////////////////////////////////////////////////////////
// ViewColumn dialog


ViewColumn::ViewColumn(CWnd* pParent /*=NULL*/)
	: CDialog(ViewColumn::IDD, pParent)
{
	//{{AFX_DATA_INIT(ViewColumn)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	list = NULL;
	nTiles = sizetile_y = 0;
	nTileInWnd = 1;
	parent = 0;
	MousePos.x = MousePos.y = 0;
}


void ViewColumn::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ViewColumn)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(ViewColumn, CDialog)
	//{{AFX_MSG_MAP(ViewColumn)
	ON_WM_PAINT()
	ON_WM_VSCROLL()
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_WM_RBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ViewColumn message handlers

void ViewColumn::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	RECT rect,wndrect; SCROLLINFO inf;
	int first,scroll,nInWnd,cx,cy;
	GetClientRect(&wndrect);	
	rect = wndrect;
	inf.fMask = SIF_ALL;
	GetScrollInfo(SB_VERT,&inf);	

	if ((nTiles==0)||(nTiles==1))
	{
		dc.FillSolidRect (&wndrect, 0xffffffff);
	}
	if (inf.nPos>(10000 - (int)inf.nPage)) inf.nPos = 10000 - inf.nPage - 1;
	if (inf.nPage>=10000) inf.nPage = 10000 -1;
	if (nTileInWnd==4)
	{		
		nInWnd = (((rect.bottom - rect.top)/(sizetile_y>>1))<<1)+2;
		if (nTiles>nInWnd) first = ((inf.nPos*((nTiles-nInWnd)>>1))/(10000 - inf.nPage))<<1;
		else first = 0;		
		int n = (nTiles+1)>>1;
		scroll = ((inf.nPos*((sizetile_y>>1)*n - (rect.bottom - rect.top)))/(10000 - inf.nPage));
		if (scroll>=0) scroll-= (first>>1)*(sizetile_y>>1);
		else scroll = 0;
		rect.top -= scroll; rect.bottom -= scroll;
		cy = sizetile_y>>1;
		cx = (rect.right - rect.left)>>1;
	}
	else
	{
		if (nTiles==1)
		{
			first = 0;
			cy = sizetile_y;
			cx = (rect.right - rect.left);
		}
		else
		{
			nInWnd = ((rect.bottom - rect.top)/sizetile_y)+1;
			if (nTiles>nInWnd) first = ((inf.nPos*(nTiles-nInWnd))/(10000 - inf.nPage));
			else first = 0;		
			scroll = ((inf.nPos*(sizetile_y*nTiles - (rect.bottom - rect.top)))/(10000 - inf.nPage));
			if (scroll<0) scroll = 0;
			else scroll-=first*sizetile_y;
			rect.top -= scroll; rect.bottom -= scroll;
			cy = sizetile_y;
			cx = (rect.right - rect.left);
		}
	}
	int i = first;
	while (rect.top<wndrect.bottom)
	{
		if (i<nTiles)
		{
			StretchDIBits(dc,rect.left,rect.top,cx,cy,
				0,0,list[i]->BmpInfo.bmiHeader.biWidth,list[i]->BmpInfo.bmiHeader.biHeight,
				&*list[i]->Bits.begin(),&list[i]->BmpInfo,DIB_RGB_COLORS,SRCCOPY);
		}
		else
		{
			StretchDIBits(dc,rect.left,rect.top,cx,cy,
				0,0,0,0,
				0,0,DIB_RGB_COLORS,WHITENESS);
		}
		if (nTileInWnd==1)
		{
			rect.top += sizetile_y;
			rect.bottom += sizetile_y;
		}
		else if (nTileInWnd==4)
		{
			if ((i&1)==0)
			{
				rect.left += cx;
				rect.right += cx;
			}
			else
			{
				rect.left -= cx;
				rect.right -= cx;
				rect.top += cy;
				rect.bottom += cy;
			}
		}				
		i++;
	}
	// Do not call CDialog::OnPaint() for painting messages
}

BOOL ViewColumn::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	HKEY regkey; int rx=-1,ry=-1,cx=-1,cy=-1;
	if (RegOpenKey(HKEY_CURRENT_USER,REGKEY_TILEDIT,&regkey)==ERROR_SUCCESS)
	{
		unsigned long value; 
		RegQueryValueEx(regkey,WndRegKeys[pos&3][0],0,&value,(unsigned char *)&rx,&value);
		RegQueryValueEx(regkey,WndRegKeys[pos&3][1],0,&value,(unsigned char *)&ry,&value);
		RegQueryValueEx(regkey,WndRegKeys[pos&3][2],0,&value,(unsigned char *)&cx,&value);
		RegQueryValueEx(regkey,WndRegKeys[pos&3][3],0,&value,(unsigned char *)&cy,&value);		
		RegQueryValueEx(regkey,WndRegKeys[pos&3][4],0,&value,(unsigned char *)&nTileInWnd,&value);
		RegCloseKey(regkey);
	}
	EnableScrollBar(SB_VERT);
	RECT rect;
	parent->GetWindowRect(&rect);	
	if (rx==-1 || ry==-1 || cx==-1 || cy==-1)
	{
		rx = rect.left;
		ry = rect.top;
		cx = rect.right - rect.left;
		cy = rect.bottom - rect.top;
		switch (pos)
		{
		case 0:
			ry=rect.bottom;
			break;
		case 1:
			ry-=rect.bottom - rect.top;
			break;
		case 2:
			rx-=rect.right - rect.left;
			break;
		case 3:
			rx+=rect.right - rect.left;
			break;
		}
	}
	else
	{
		rx += rect.left;
		ry += rect.top;
	}
	SetWindowPos(0,rx,ry,cx,cy,0);

	SendMessage(WM_VSCROLL,0,0);
	// TODO: Add extra initialization here
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void ViewColumn::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// TODO: Add your message handler code here and/or call default	
	SCROLLINFO scroll; RECT rect;
	scroll.fMask = SIF_TRACKPOS | SIF_POS;	
	GetScrollInfo(SB_VERT,&scroll);
	scroll.cbSize = sizeof(SCROLLINFO);
	scroll.fMask = SIF_ALL;
	//scroll.nPos = scroll.nTrackPos;
	scroll.nMax = 10000;
	scroll.nMin = 0;	
	GetClientRect(&rect);
	if (nTiles) 
	{
		if (nTileInWnd==4)
		{
			scroll.nPage = ((rect.bottom - rect.top)*10000)/(( (nTiles>>1)+(nTiles&1) )*(sizetile_y>>1));
		}
		else
		{
			scroll.nPage = ((rect.bottom - rect.top)*10000)/(nTiles*sizetile_y);
		}
	}
	else scroll.nPage = 10000;
	
	switch(nSBCode)
	{
	case SB_BOTTOM:
		scroll.nPos = 10000 - scroll.nPage;
		break;
	case SB_LINEDOWN:
		scroll.nPos += scroll.nPage/4;
		break;
	case SB_LINEUP:
		scroll.nPos -= scroll.nPage/4;
		break;
	case SB_PAGEDOWN:
		scroll.nPos += scroll.nPage;
		break;
	case SB_PAGEUP:
		scroll.nPos -= scroll.nPage;
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		scroll.nPos = scroll.nTrackPos;
		break;
	case SB_TOP:
		scroll.nPos = 0;
		break;
	default:
		scroll.fMask^=SIF_POS;
		break;
	}		
	if (scroll.nPos<0) 
		scroll.nPos = 0;
	else if (scroll.nPos > (10000 - (int)scroll.nPage)) 
		scroll.nPos = 10000 - scroll.nPage - 1;
	this->SetScrollInfo(SB_VERT,&scroll);
	RedrawWindow();
	CDialog::OnVScroll(nSBCode, nPos, pScrollBar);
}

void ViewColumn::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	
	CDialog::OnClose();
}

void ViewColumn::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	SendMessage(WM_VSCROLL,0,0);
}

LRESULT ViewColumn::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (message==WM_CLOSE || message==WM_DESTROY)
	{
		HKEY regkey;
		if (RegCreateKey(HKEY_CURRENT_USER,REGKEY_TILEDIT,&regkey)==ERROR_SUCCESS)
		{

			RECT rect; RECT rpopup;
			GetWindowRect(&rect);
			parent->GetWindowRect(&rpopup);
			int rx,ry,cx,cy; //on sauvegarde les coordonnes de la wnd relativement a la fenetre popup principale
			rx = rect.left - rpopup.left;
			ry = rect.top - rpopup.top;
			cx = rect.right - rect.left;
			cy = rect.bottom - rect.top;
			RegSetValueEx(regkey,WndRegKeys[pos][0],0,REG_DWORD,(unsigned char*)&rx,4);
			RegSetValueEx(regkey,WndRegKeys[pos][1],0,REG_DWORD,(unsigned char*)&ry,4);
			RegSetValueEx(regkey,WndRegKeys[pos][2],0,REG_DWORD,(unsigned char*)&cx,4);
			RegSetValueEx(regkey,WndRegKeys[pos][3],0,REG_DWORD,(unsigned char*)&cy,4);
			RegSetValueEx(regkey,WndRegKeys[pos][4],0,REG_DWORD,(unsigned char*)&nTileInWnd,4);
			RegCloseKey(regkey);
		}		
	}
	else if (message==WM_MOUSEMOVE)
	{
		MousePos.x = LOWORD(lParam);
		MousePos.y = HIWORD(lParam);
	}
	else if (message==WM_ERASEBKGND)
	{
		return 0;
	}
	else if (message==WM_MOUSEWHEEL)
	{
		if ((short)(HIWORD(wParam))<0)
			SendMessage(WM_VSCROLL,SB_LINEDOWN,0);
		else
			SendMessage(WM_VSCROLL,SB_LINEUP,0);
	}
	else if (message==WM_COMMAND)
	{
		switch(LOWORD(wParam))
		{
		case 10:
			if (nTileInWnd!=1)
			{
				nTileInWnd = 1;
				SendMessage(WM_VSCROLL,0,0);
			}
			break;
		case 12:
			if (nTileInWnd!=4)
			{
				nTileInWnd = 4;
				SendMessage(WM_VSCROLL,0,0);
			}
			break;
		case 13:
			RECT rect;
			parent->GetWindowRect(&rect);
			int rx,ry,cx,cy;
			rx = rect.left;
			ry = rect.top;
			cx = rect.right - rect.left;
			cy = rect.bottom - rect.top;
			switch (pos)
			{
			case 0:
				ry=rect.bottom;
				break;
			case 1:
				ry-=rect.bottom - rect.top;
				break;
			case 2:
				rx-=rect.right - rect.left;
				break;
			case 3:
				rx+=rect.right - rect.left;
				break;
			}
			SetWindowPos(0,rx,ry,cx,cy,0);
			break;
		}
	}
	return CDialog::WindowProc(message, wParam, lParam);
}

void ViewColumn::OnRButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	CMenu popup;
	popup.CreatePopupMenu();
	popup.AppendMenu(MF_STRING | (nTileInWnd==1?MF_CHECKED:0),10,"*1");
	popup.AppendMenu(MF_STRING | (nTileInWnd==4?MF_CHECKED:0),12,"*4");
	popup.AppendMenu(MF_STRING,13,"Replace window");
	
	RECT rect; GetWindowRect(&rect);
	popup.TrackPopupMenu(TPM_LEFTALIGN,MousePos.x+rect.left,MousePos.y+rect.top,this,NULL);
	CDialog::OnRButtonDown(nFlags, point);
}
