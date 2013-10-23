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

//#include "tile_edit_dll.h"
#include "resource.h"
#include "Browse.h"
#include "custom.h"
#include "getval.h"

using namespace NL3D;
using namespace NLMISC;

extern CTileBank tileBank2;

/////////////////////////////////////////////////////////////////////////////
// Browse dialog

Browse::Browse(int nland, CWnd* pParent /*=NULL*/)
	: CDialog(Browse::IDD, pParent)
{	
	//{{AFX_DATA_INIT(Browse)
	SubGroup0 = FALSE;
	SubGroup1 = FALSE;
	SubGroup2 = FALSE;
	SubGroup3 = FALSE;
	SubGroup4 = FALSE;
	SubGroup5 = FALSE;
	SubGroup6 = FALSE;
	SubGroup7 = FALSE;
	SubGroup10 = FALSE;
	SubGroup11 = FALSE;
	SubGroup8 = FALSE;
	SubGroup9 = FALSE;
	Oriented = FALSE;
	//}}AFX_DATA_INIT
	land=nland;
	m_128x128=0;
}


void Browse::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(Browse)
	DDX_Radio(pDX, IDC_128X128, m_128x128);
	DDX_Control(pDX, IDC_VIEW, m_ctrl);
	DDX_Control(pDX, IDC_INFONUM, m_infotexte);
	DDX_Control(pDX, IDC_JOUR, m_rb_jour);
	DDX_Check(pDX, IDC_SUBGROUP0, SubGroup0);
	DDX_Check(pDX, IDC_SUBGROUP1, SubGroup1);
	DDX_Check(pDX, IDC_SUBGROUP2, SubGroup2);
	DDX_Check(pDX, IDC_SUBGROUP3, SubGroup3);
	DDX_Check(pDX, IDC_SUBGROUP4, SubGroup4);
	DDX_Check(pDX, IDC_SUBGROUP5, SubGroup5);
	DDX_Check(pDX, IDC_SUBGROUP6, SubGroup6);
	DDX_Check(pDX, IDC_SUBGROUP7, SubGroup7);
	DDX_Check(pDX, IDC_SUBGROUP10, SubGroup10);
	DDX_Check(pDX, IDC_SUBGROUP11, SubGroup11);
	DDX_Check(pDX, IDC_SUBGROUP8, SubGroup8);
	DDX_Check(pDX, IDC_SUBGROUP9, SubGroup9);
	DDX_Check(pDX, IDC_ORIENTED, Oriented);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(Browse, CDialog)
	//{{AFX_MSG_MAP(Browse)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_ALPHA, OnAlpha)
	ON_BN_CLICKED(IDC_128X128, OnChangeVariety)
	ON_BN_CLICKED(IDC_JOUR, OnJour)
	ON_BN_CLICKED(IDC_NUIT, OnNuit)
	ON_BN_CLICKED(IDC_OK, OnOk)
	ON_WM_RBUTTONDOWN()
	ON_BN_CLICKED(IDC_OK2, OnUpdateTiles)
	ON_BN_CLICKED(IDC_BATCH_LOAD, OnBatchLoad)
	ON_BN_CLICKED(IDC_SUBGROUP0, OnSubgroup0)
	ON_BN_CLICKED(IDC_SUBGROUP1, OnSubgroup1)
	ON_BN_CLICKED(IDC_SUBGROUP2, OnSubgroup2)
	ON_BN_CLICKED(IDC_SUBGROUP3, OnSubgroup3)
	ON_BN_CLICKED(IDC_SUBGROUP4, OnSubgroup4)
	ON_BN_CLICKED(IDC_SUBGROUP5, OnSubgroup5)
	ON_BN_CLICKED(IDC_SUBGROUP6, OnSubgroup6)
	ON_BN_CLICKED(IDC_SUBGROUP7, OnSubgroup7)
	ON_BN_CLICKED(IDC_SUBGROUP8, OnSubgroup8)
	ON_BN_CLICKED(IDC_SUBGROUP9, OnSubgroup9)
	ON_BN_CLICKED(IDC_SUBGROUP10, OnSubgroup10)
	ON_BN_CLICKED(IDC_SUBGROUP11, OnSubgroup11)
	ON_BN_CLICKED(IDC_EXPORT_BORDER, OnExportBorder)
	ON_BN_CLICKED(IDC_ZOOM5, OnChangeVariety)
	ON_BN_CLICKED(IDC_ZOOM6, OnChangeVariety)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_CANCEL, OnCancel)
	ON_BN_CLICKED(IDC_DISPLACE, OnChangeVariety)
	ON_BN_CLICKED(IDC_IMPORT_BORDER2, OnImportBorder)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Browse message handlers

BOOL Browse::PreCreateWindow(CREATESTRUCT& cs) 
{
	// TODO: Add your specialized code here and/or call the base class

	return CDialog::PreCreateWindow(cs);
}

DWORD thread_id;
int thread_actif = 0;
Browse *pDialog;
int ccount=0;

LRESULT Browse::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{

	// TODO: Add your specialized code here and/or call the base class
	if (ccount==0 && message==WM_PAINT)
	{	
		Init();
	}
	if (message==WM_KEYUP || message==WM_KEYDOWN)
	{
		RECT parent,client; 
		GetWindowRect(&parent); m_ctrl.GetWindowRect(&client);
		if (m_ctrl.MousePos.x<(client.right - parent.left) && 
			m_ctrl.MousePos.x>0 &&
			m_ctrl.MousePos.y<(client.bottom - client.top) &&
			m_ctrl.MousePos.y>0)
		{
			m_ctrl.SendMessage(message,wParam,lParam);
		}
	}

	if (message==WM_MOUSEMOVE)
	{
		m_ctrl.MousePos.x = LOWORD(lParam);
		m_ctrl.MousePos.y = HIWORD(lParam);

		RECT client, parent; 
		
		ClientToScreen (&m_ctrl.MousePos);
		m_ctrl.ScreenToClient (&m_ctrl.MousePos);

		m_ctrl.GetWindowRect(&client);
		GetWindowRect(&parent);
		if (m_ctrl.MousePos.x<0)
			m_ctrl.MousePos.x=0;
		if (m_ctrl.MousePos.x>client.right-client.left)
			m_ctrl.MousePos.x=client.right-client.left;
		if (m_ctrl.MousePos.y<0)
			m_ctrl.MousePos.y=0;
		if (m_ctrl.MousePos.y>client.bottom-client.top)
			m_ctrl.MousePos.y=client.bottom-client.top;

		m_ctrl.ClientToScreen (&m_ctrl.MousePos);
		ScreenToClient (&m_ctrl.MousePos);
		
		if (lbutton) //on dessine le carre de selection
		{ 
			selection = 1;
			RECT current;
			SIZE size; size.cx = size.cy = 1;
			current.left = OriginalPos.x;
			current.top = OriginalPos.y;
			current.right = m_ctrl.MousePos.x;
			current.bottom = m_ctrl.MousePos.y;
			if (current.left>current.right) {int temp = current.left; current.left = current.right; current.right = temp;}
			if (current.top>current.bottom) {int temp = current.bottom; current.bottom = current.top; current.top = temp;}
			
			CDC *pDC = GetDC();
			m_ctrl.DrawDragRect(pDC,NULL,size,&last_sel,size);			//on efface l'ancien carre
			
			m_ctrl.UpdateSelection(&current, (int)wParam, m_128x128);						//on affiche les modifes
			
			m_ctrl.DrawDragRect(pDC,&current,size,NULL,size);			//on affiche le nouveau carre
			::ReleaseDC(*this,*pDC);			
			
			last_sel = current;
		}
	}
	if (message==WM_DROPFILES)
	{
		m_ctrl.PostMessage(WM_DROPFILES,wParam,lParam);
	}
	if (message==WM_COMMAND && !thread_actif)
	{
		int button = LOWORD(wParam);
		if (button==IDC_ZOOM1 || button==IDC_ZOOM2 || button==IDC_ZOOM3)
		{
			m_ctrl.Zoom = button - IDC_ZOOM1 +1;
			m_ctrl.UpdateSize(m_128x128);
			m_ctrl.scrollpos = 0;
			SetScrollPos(SB_VERT,0,true);
			m_ctrl.RedrawWindow();
		}
		else if (button==IDC_INFONUM)
		{
			m_ctrl.InfoTexte = 1;
			m_ctrl.RedrawWindow();
		}			
		else if (button==IDC_FILENAME)
		{
			m_ctrl.InfoTexte =2;
			m_ctrl.RedrawWindow();
		}
		else if (button==IDC_GROUP)
		{
			m_ctrl.InfoTexte = 3;
			m_ctrl.RedrawWindow();
		}
		else if (button>=10 && button<=15) 
			m_ctrl.PostMessage(WM_COMMAND,wParam,lParam);
	}
	if (message==WM_LBUTTONDOWN)
	{
		int xPos = LOWORD(lParam);  // horizontal position of cursor 
		int yPos = HIWORD(lParam);  // vertical position of cursor 

		if (lbutton) //on dessine le carre de selection
		{ 
			selection = 1;
			RECT current;
			SIZE size; size.cx = size.cy = 1;
			current.left = OriginalPos.x;
			current.top = OriginalPos.y;
			current.right = m_ctrl.MousePos.x;
			current.bottom = m_ctrl.MousePos.y;
			if (current.left>current.right) {int temp = current.left; current.left = current.right; current.right = temp;}
			if (current.top>current.bottom) {int temp = current.bottom; current.bottom = current.top; current.top = temp;}
			
			CDC *pDC = GetDC();
			m_ctrl.DrawDragRect(pDC,NULL,size,&last_sel,size);			//on efface l'ancien carre
			
			m_ctrl.UpdateSelection(&current,(int)wParam, m_128x128);						//on affiche les modifes
			
			::ReleaseDC(*this,*pDC);			
			
			last_sel = current;
		}
 
		RECT p,rect; p.left = m_ctrl.MousePos.x; p.top = m_ctrl.MousePos.y;
		ClientToScreen(&p); 
		m_ctrl.GetClientRect(&rect);
		POINT pt; pt.x = p.left; pt.y = p.top;
		m_ctrl.ScreenToClient(&pt);
		if (pt.x>=rect.left && pt.x<rect.right && pt.y>=rect.top && pt.y<rect.bottom) 
		{
			m_ctrl.SetFocus();
			int index = m_ctrl.GetIndex(&pt, m_128x128);
			if (index!=-1 && !(wParam&MK_SHIFT)/* && !(wParam&MK_CONTROL)*/)
			{
				tilelist::iterator p = m_ctrl.InfoList.Get(index, m_128x128);
				if (p!=m_ctrl.InfoList.GetLast(m_128x128))
				{
					tilelist::iterator pp = p;
					if (wParam&MK_CONTROL)
						p->Selected = p->Selected?0:7;
					else 
						p->Selected = 1;
					CDC *pDC = NULL;
					int indexx=0;
					for (p=m_ctrl.InfoList.GetFirst(m_128x128);p!=m_ctrl.InfoList.GetLast(m_128x128);++p, indexx++)
					{
						if (p!=pp && p->Selected)
						{
							if (!(wParam&MK_CONTROL))
							{
								p->Selected = 0;
								if (pDC==NULL) pDC = m_ctrl.GetDC();
								m_ctrl.DrawTile(p,pDC,1,m_128x128);
							}
						}
						else
						{
							if (p==pp)
							{
								if (pDC==NULL) pDC = m_ctrl.GetDC();
								m_ctrl.DrawTile(p,pDC,1,m_128x128);
							}
						}
					}
					if (pDC) ::ReleaseDC(*this,*pDC);
				}
			}
			else
			{
				if (!(wParam&MK_CONTROL) && !(wParam&MK_SHIFT))
				{
					tilelist::iterator p = m_ctrl.InfoList.GetFirst(m_128x128);
					CDC *pDC = NULL;
					for (int i = 0; i<m_ctrl.InfoList.GetSize(m_128x128); i++)
					{
						if (p->Selected)
						{
							if (pDC==NULL) pDC = m_ctrl.GetDC();
							//m_ctrl.InfoList.setSelection (i, 0);
							p->Selected = 0;
							m_ctrl.DrawTile(p,pDC,1,m_128x128);
						}
						p++;
					}
					if (pDC) ::ReleaseDC(*this,*pDC);			
				}
			}
			lbutton = 1;

			SIZE size; size.cx = size.cy = 1;

			last_sel.top = xPos;
			last_sel.left = yPos;
			last_sel.bottom = xPos;
			last_sel.right = yPos;
			OriginalPos.x=xPos;
			OriginalPos.y=yPos;
		}
	}
	if (message==WM_LBUTTONUP || message==WM_NCLBUTTONUP)
	{
		RECT p; p.left = m_ctrl.MousePos.x; p.top = m_ctrl.MousePos.y;
		ClientToScreen(&p);
		POINT pt; pt.x = p.left; pt.y = p.top;
		m_ctrl.ScreenToClient(&pt);
		int index = m_ctrl.GetIndex(&pt, m_128x128);
		if (!selection && index!=-1)
		{
			int i = 0;
		}
		else if (selection)
		{
			CDC *pDC = GetDC();
			CSize size; size.cx = size.cy = 1;
			m_ctrl.DrawDragRect(pDC,NULL,size,&last_sel,size);
			::ReleaseDC(*this,*pDC);
			int index=0;
			for (tilelist::iterator p = m_ctrl.InfoList.GetFirst(m_128x128);p!=m_ctrl.InfoList.GetLast(m_128x128);++p, index++)
			{
				if (p->Selected&3)
				{
					p->Selected=2;
				}
				else 
					p->Selected = 0;
			}
		}
		selection =0;
		lbutton = 0;
	}
	if (message==WM_KEYDOWN)
	{
		int toto = 0;
	}
	if (message==WM_SIZE && m_ctrl.count_ )
	{
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);
		
		int i = std::max (1, m_ctrl.GetNbTileLine()); 
		int j = std::max (1, m_ctrl.GetNbTileColumn());
		int pos = m_ctrl.GetScrollPos(SB_VERT);
		int hview = (m_ctrl.InfoList.GetSize(m_128x128)/i + 1)*(m_ctrl.sizeicon_y + m_ctrl.spacing_y) + m_ctrl.spacing_y;
		m_ctrl.scrollpos = (pos*hview)/SCROLL_MAX;

		RECT clientrect,rect;
		m_ctrl.GetWindowRect(&clientrect);
		InvalidateRect(NULL,false);
		GetWindowRect(&rect);
		m_ctrl.SetWindowPos(NULL, 0, 0, std::max (100, x - 120), y - 20, SWP_NOMOVE);
		int iFirst,iLast; 
		m_ctrl.GetVisibility(iFirst, iLast, m_128x128);
		m_ctrl.UpdateBar(iFirst, iLast, m_128x128);
		return 0;
	}
	if (message==WM_VSCROLL || message==WM_MOUSEWHEEL)
	{
		SCROLLINFO inf;
		RECT rect_scroll,rect_clip;
		int scrollcode,pos;
		inf.fMask = SIF_ALL;
		GetScrollInfo(SB_VERT,&inf);
		m_ctrl.GetClientRect(&rect_scroll);
		int i = m_ctrl.GetNbTileLine();
		int hview = (m_ctrl.InfoList.GetSize(m_128x128)/i + 2)*(m_ctrl.sizeicon_y + m_ctrl.spacing_y) + m_ctrl.spacing_y;

		if (message==WM_MOUSEWHEEL)
		{
			int inc = ((int)(short)HIWORD(wParam))/WHEEL_DELTA;
			pos = inf.nPos - inc*(((m_ctrl.sizeicon_y+m_ctrl.spacing_y)*SCROLL_MAX)/(hview - m_ctrl.spacing_y));
		}
		else 
		{
			scrollcode = LOWORD(wParam);
			pos = inf.nTrackPos;
			switch (scrollcode)
			{					
				case SB_BOTTOM:
					pos = SCROLL_MAX - inf.nPage;
					break;
				case SB_PAGEDOWN:
					pos = inf.nPos + inf.nPage;
					break;
				case SB_PAGEUP:
					pos = inf.nPos - inf.nPage;
					break;
				case SB_LINEUP:
					pos = inf.nPos - (((m_ctrl.sizeicon_y+m_ctrl.spacing_y)*SCROLL_MAX)/(hview - m_ctrl.spacing_y));
					break;
				case SB_LINEDOWN:
					pos = inf.nPos + (((m_ctrl.sizeicon_y+m_ctrl.spacing_y)*SCROLL_MAX)/(hview - m_ctrl.spacing_y));
					break;
				case SB_TOP:
					pos = 0;
					break;
				case SB_THUMBPOSITION:
				case SB_ENDSCROLL:
					pos = inf.nPos;
					break;
			}
		}

		if (pos<0) pos = 0;
		if (pos>(SCROLL_MAX - (int)inf.nPage)) 
			pos = SCROLL_MAX - inf.nPage;
		
		SetScrollPos(SB_VERT,pos,1);
		rect_scroll.bottom -= rect_scroll.top;
		rect_scroll.top = 0;
		rect_clip = rect_scroll;
		int scroll_pixel = m_ctrl.scrollpos;
		int old_iFV,old_iLV;
		m_ctrl.GetVisibility(old_iFV, old_iLV, m_128x128);
		m_ctrl.scrollpos = (pos*hview)/(SCROLL_MAX);
		int iFV,iLV;
		m_ctrl.GetVisibility(iFV, iLV, m_128x128);
		
		if (iFV>old_iLV || iLV<old_iFV || scrollcode==SB_PAGEDOWN || scrollcode==SB_PAGEUP)
		{
			m_ctrl.RedrawWindow();
		}
		else
		{
			scroll_pixel -= m_ctrl.scrollpos;
			if (scroll_pixel)
			{
				CDC *pDC = m_ctrl.GetDC();
				if (abs(scroll_pixel)>(rect_clip.bottom - rect_clip.top)) scroll_pixel = 0;
				else pDC->ScrollDC(0,scroll_pixel,&rect_scroll,&rect_clip,NULL,NULL);

				tilelist::iterator p = m_ctrl.InfoList.GetFirst(m_128x128);		
				CBrush brush (GetSysColor(COLOR_3DFACE));
				if (scroll_pixel<0)
				{
					rect_scroll.top = rect_scroll.bottom + scroll_pixel;
					pDC->FillRect(&rect_scroll,&brush);
					if ((iLV-i)<iFV) i = iLV - iFV;
					int k;
					for (k = 0;k<old_iLV-i;k++) p++;
					for (k=old_iLV - i;k<=iLV;k++) 
					{
						m_ctrl.DrawTile(p,pDC,0,m_128x128);
						p++;
					}
				}
				else
				{
					rect_scroll.bottom = rect_scroll.top + scroll_pixel;
					pDC->FillRect(&rect_scroll,&brush);
					int k;
					for (k = 0;k<iFV;k++) p++;
					for (k = iFV;k<(old_iFV+i);k++)
					{
						m_ctrl.DrawTile(p,pDC,0,m_128x128);
						p++;
					}
				}
				::ReleaseDC(m_ctrl,*pDC);
			}
		}
		m_ctrl.lastVBarPos = pos;
	}
	if (message==WM_CLOSE) 
	{
		ccount=0; 
		this->m_ctrl.count_=0;
		OnDestroy();
	}
	if (message==WM_LBUTTONDBLCLK)
	{
		m_ctrl.SendMessage(WM_LBUTTONDBLCLK,wParam,lParam);
	}
	pDialog=this;
	return CDialog::WindowProc(message, wParam, lParam);
}	


unsigned long Browse::MyControllingFunction( void* pParam )
{
	thread_actif = 1;
	Browse *br = (Browse*)pParam;
	br->m_ctrl.lockInsertion = 1;
	int iFV,iLV;
	br->m_ctrl.GetVisibility(iFV, iLV, br->m_128x128);
	br->m_ctrl.UpdateBar(iFV, iLV, br->m_128x128);
	tilelist::iterator p = br->m_ctrl.InfoList.GetFirst(br->m_128x128);
	tilelist::iterator plast = p;
	for (int i=0;i<br->m_ctrl.InfoList.GetSize(br->m_128x128);i++)
	{
		int *ld; 
		int rot=0;
		std::string path;
		LPBITMAPINFO pBmp; 
		std::vector<NLMISC::CBGRA>* bits;

		switch (br->m_128x128)
		{
		case 0:
			path = tileBank2.getTile (tileBank2.getTileSet (br->m_ctrl.InfoList._tileSet)->getTile128 (i))->
				getRelativeFileName ((CTile::TBitmap)(br->m_ctrl.Texture-1));
			break;
		case 1:
			path = tileBank2.getTile (tileBank2.getTileSet (br->m_ctrl.InfoList._tileSet)->getTile256 (i))->
				getRelativeFileName ((CTile::TBitmap)(br->m_ctrl.Texture-1));
			break;
		case 2:
			{
				int index=tileBank2.getTileSet (br->m_ctrl.InfoList._tileSet)->getTransition (i)->getTile();
				if (index!=-1)
				{
					path = tileBank2.getTile (index)->getRelativeFileName ((CTile::TBitmap)(br->m_ctrl.Texture-1));
					if (br->m_ctrl.Texture==3)
						rot = tileBank2.getTile (index)->getRotAlpha ();
				}
				else
					path = "";
			}
			break;
		case 3:
			// Get diplacement filename
			path = tileBank2.getDisplacementMap (tileBank2.getTileSet (br->m_ctrl.InfoList._tileSet)->getDisplacementTile ((CTileSet::TDisplacement)i));
			break;
		}
		std::vector<NLMISC::CBGRA>* pAlpha=NULL;
		switch (br->m_ctrl.Texture)
		{
		case 1:
			ld = &p->loaded;
			pBmp = &p->BmpInfo;
			bits = &p->Bits;
			pAlpha = &p->alphaBits;
			break;
		case 2:
			ld = &p->nightLoaded;
			pBmp = &p->nightBmpInfo;
			bits = &p->nightBits;
			pAlpha = &p->alphaBits;
			break;
		case 3:
			ld = &p->alphaLoaded;
			pBmp = &p->alphaBmpInfo;
			bits = &p->alphaBits;
			break;
		}

		if ((path!="") && _LoadBitmap(tileBank2.getAbsPath() + path, pBmp, *bits, pAlpha, rot))
		{			
			*ld=1;
			int iFV,iLV; br->m_ctrl.GetVisibility(iFV, iLV, br->m_128x128);
			if (i<=iLV && i>=iFV) 
			{
				CDC *pDC = br->m_ctrl.GetDC();
				br->m_ctrl.DrawTile(p,pDC,1,br->m_128x128);
				::ReleaseDC(*br,*pDC);
			}
		}
		p++;
	}
	br->m_ctrl.lockInsertion = 0;
	thread_actif = 0;
	return 1;
}

void Browse::LoadInThread(void)
{
	if (!thread_actif)
	MyControllingFunction (this);
}


void Browse::Init()
{		
	UpdateData ();
	lbutton = 0;
	selection = 0;
	control = 0;
	m_ctrl.lockInsertion = 0; oldsel = -1;
	HKEY regkey; 
	unsigned long value; 
	unsigned long type; 
	int cx=-1,cy=-1,x=-1,y=-1;
	char sWindowpl[256];

	if (RegOpenKey(HKEY_CURRENT_USER,REGKEY_TILEDIT,&regkey)==ERROR_SUCCESS)
	{		
		value=256;
		type=REG_SZ;
		if (RegQueryValueEx(regkey,REGKEY_WNDPL,0,&type,(unsigned char *)&sWindowpl,&value)==ERROR_SUCCESS)
		{
			WINDOWPLACEMENT wndpl;
			sscanf(sWindowpl,"%d %d %d %d %d %d %d %d %d %d",
						&wndpl.flags,
						&wndpl.ptMaxPosition.x,&wndpl.ptMaxPosition.y,
						&wndpl.ptMinPosition.x,&wndpl.ptMinPosition.y,
						&wndpl.rcNormalPosition.bottom,&wndpl.rcNormalPosition.left,&wndpl.rcNormalPosition.right,&wndpl.rcNormalPosition.top,
						&wndpl.showCmd);
			wndpl.length = sizeof(WINDOWPLACEMENT);
			this->SetWindowPlacement(&wndpl);
		}
		value=256;
		type=REG_SZ;
		if (RegQueryValueEx(regkey,REGKEY_LASTPATH,0,&type,(unsigned char *)&sWindowpl,&value)!=ERROR_SUCCESS)
			m_ctrl.LastPath="";
		else
			m_ctrl.LastPath=(const char*)sWindowpl;
		value=4;
		type=REG_DWORD;
		if (RegQueryValueEx(regkey,REGKEY_BUTTONZOOM,0,&type,(unsigned char *)&m_ctrl.Zoom,&value)!=ERROR_SUCCESS) 
			m_ctrl.Zoom = 3;
		value=4;
		type=REG_DWORD;
		if (RegQueryValueEx(regkey,REGKEY_BUTTONVARIETY,0,&type,(unsigned char *)&m_128x128,&value)!=ERROR_SUCCESS) 
			m_128x128 = 0;
		value=4;
		type=REG_DWORD;
		if (RegQueryValueEx(regkey,REGKEY_BUTTONTEXTURE,0,&type,(unsigned char *)&m_ctrl.Texture,&value)!=ERROR_SUCCESS) 
			m_ctrl.Texture = 1;
		value=4;
		type=REG_DWORD;
		if (RegQueryValueEx(regkey,REGKEY_BUTTONTEXTINFO,0,&type,(unsigned char *)&m_ctrl.InfoTexte,&value)!=ERROR_SUCCESS) 
			m_ctrl.InfoTexte = 1;
		RegCloseKey(regkey);
	}		
	CButton *button = (CButton*)GetDlgItem(IDC_ZOOM1 + m_ctrl.Zoom -1);
	button->SetCheck(1);
	button = (CButton*)GetDlgItem(IDC_JOUR + m_ctrl.Texture -1);
	button->SetCheck(1);
	button = (CButton*)GetDlgItem(IDC_INFONUM + m_ctrl.InfoTexte -1);
	button->SetCheck(1);	
	if (cx!=-1 && cy!=-1 && x!=-1 && y!=-1) SetWindowPos(0,x,y,cx,cy,0);

	m_ctrl.Init(land, m_128x128);
	SelectionTerritoire *slt = (SelectionTerritoire*)GetParent();
	ccount=1;
	
	RECT rect;
	this->GetWindowRect(&rect);
	SendMessage(WM_SIZE,rect.right - rect.left,rect.bottom - rect.top); //force resize

	SelectionTerritoire *parent = (SelectionTerritoire*)GetParent();

	// The land	
	CTileSet *tileSet=tileBank2.getTileSet (land);

	if (tileSet->getOriented())
		Oriented = 1;
	else
		Oriented = 0;
	
	// 128
	m_ctrl.InfoList.theList128.resize (tileSet->getNumTile128 ());
	int i;
	for (i=0; i<tileSet->getNumTile128 (); i++)
	{
		m_ctrl.InfoList.theList128[i].id=i;
		m_ctrl.InfoList.theList128[i].Selected=0;
		m_ctrl.InfoList.theList128[i].loaded=0;
		m_ctrl.InfoList.theList128[i].nightLoaded=0;
		m_ctrl.InfoList.theList128[i].alphaLoaded=0;
	}
	m_ctrl.InfoList.Reload (0, tileSet->getNumTile128 (), 0);

	// 256
	m_ctrl.InfoList.theList256.resize (tileSet->getNumTile256 ());
	for (i=0; i<tileSet->getNumTile256 (); i++)
	{
		m_ctrl.InfoList.theList256[i].id=i;
		m_ctrl.InfoList.theList256[i].Selected=0;
		m_ctrl.InfoList.theList256[i].loaded=0;
		m_ctrl.InfoList.theList256[i].nightLoaded=0;
		m_ctrl.InfoList.theList256[i].alphaLoaded=0;
	}
	m_ctrl.InfoList.Reload (0, tileSet->getNumTile256 (), 1);

	// Transition
	for (i=0; i<CTileSet::count; i++)
	{
		m_ctrl.InfoList.theListTransition[i].id=i;
		m_ctrl.InfoList.theListTransition[i].Selected=0;
		m_ctrl.InfoList.theListTransition[i].loaded=0;
		m_ctrl.InfoList.theListTransition[i].nightLoaded=0;
		m_ctrl.InfoList.theListTransition[i].alphaLoaded=0;
	}
	m_ctrl.InfoList.Reload (0, CTileSet::count, 2);

	// Displacement
	for (i=0; i<CTileSet::CountDisplace; i++)
	{
		m_ctrl.InfoList.theListDisplacement[i].id=i;
		m_ctrl.InfoList.theListDisplacement[i].Selected=0;
		m_ctrl.InfoList.theListDisplacement[i].loaded=0;
		m_ctrl.InfoList.theListDisplacement[i].nightLoaded=0;
		m_ctrl.InfoList.theListDisplacement[i].alphaLoaded=0;
	}
	m_ctrl.InfoList.Reload (0, CTileSet::CountDisplace, 3);

	CString fullpath = parent->DefautPath + parent->CurrentTerritory;
	
	LoadInThread();
	UpdateData (FALSE);
	
	OnChangeVariety();
}



void Browse::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
}

void Browse::OnAlpha ()
{
	// TODO: Add your control notification handler code here
	m_ctrl.Texture = 3;
	LoadInThread();
	m_ctrl.RedrawWindow();
}

void Browse::OnJour ()
{
	// TODO: Add your control notification handler code here
	m_ctrl.Texture = 1;
	LoadInThread();
	m_ctrl.RedrawWindow();
}

void Browse::OnNuit ()
{
	// TODO: Add your control notification handler code here
	m_ctrl.Texture = 2;
	LoadInThread();
	m_ctrl.RedrawWindow();
}

void Browse::OnNum() 
{
	// TODO: Add your control notification handler code here
	m_ctrl.Sort = 1;	
	m_ctrl.SendMessage(WM_PAINT);
}

void Browse::OnCancel() 
{
	// TODO: Add your control notification handler code here
	if (thread_actif) return;

	if (::MessageBox (NULL, "Are you sure you want to cancel?", "Cancel", MB_OK|MB_ICONQUESTION|MB_YESNO)==IDYES)
	{
		this->SendMessage(WM_CLOSE);
		CDialog::OnCancel();
		/*
		EndDialog(0);*/
	}
}

void Browse::UpdateAll(void)
{
	
}

void Browse::OnDestroy()
{
	// TODO: Add your control notification handler code here
	HKEY regkey;
	WINDOWPLACEMENT wndpl;
	this->GetWindowPlacement(&wndpl);
	char sWindowpl[256];
	sprintf(sWindowpl,"%d %d %d %d %d %d %d %d %d %d",
						wndpl.flags,
						wndpl.ptMaxPosition.x,wndpl.ptMaxPosition.y,
						wndpl.ptMinPosition.x,wndpl.ptMinPosition.y,
						wndpl.rcNormalPosition.bottom,wndpl.rcNormalPosition.left,wndpl.rcNormalPosition.right,wndpl.rcNormalPosition.top,
						wndpl.showCmd);
	if (RegCreateKey(HKEY_CURRENT_USER,REGKEY_TILEDIT,&regkey)==ERROR_SUCCESS)
	{	
		//int sel = ((CComboBox*)GetDlgItem(IDC_LISTTYPE))->GetCurSel();
		RegSetValueEx(regkey,REGKEY_WNDPL,0,REG_SZ,(const unsigned char*)sWindowpl,(DWORD)strlen(sWindowpl));
		RegSetValueEx(regkey,REGKEY_LASTPATH,0,REG_SZ,(const unsigned char*)m_ctrl.LastPath.c_str(),(DWORD)strlen(m_ctrl.LastPath.c_str()));
		RegSetValueEx(regkey,REGKEY_BUTTONZOOM,0,REG_DWORD,(const unsigned char*)&m_ctrl.Zoom,4);
		RegSetValueEx(regkey,REGKEY_BUTTONVARIETY,0,REG_DWORD,(const unsigned char*)&m_128x128,4);
		RegSetValueEx(regkey,REGKEY_BUTTONTEXTURE,0,REG_DWORD,(const unsigned char*)&m_ctrl.Texture,4);
		RegSetValueEx(regkey,REGKEY_BUTTONSORT,0,REG_DWORD,(const unsigned char*)&m_ctrl.Sort,4);
		RegSetValueEx(regkey,REGKEY_BUTTONTEXTINFO,0,REG_DWORD,(const unsigned char*)&m_ctrl.InfoTexte,4);
		RegCloseKey(regkey);
	}
}

void Browse::OnOk() 
{
	// TODO: Add your control notification handler code here
	if (thread_actif) return;

	// trap - Don't know if this is the right place to do this
	UpdateData ();
	CTileSet *tileSet=tileBank2.getTileSet (land);
	tileSet->setOriented(Oriented?true:false);


	this->SendMessage(WM_CLOSE);
	EndDialog(1);
}

void Browse::OnRButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	m_ctrl.PostMessage(WM_RBUTTONDOWN,point.x,point.y);
	CDialog::OnRButtonDown(nFlags, point);
}

void Browse::OnSelchangeListtype() 
{
}

void Browse::OnUpdateTiles() 
{
	// TODO: Add your control notification handler code here
	LoadInThread();
}


void Browse::OnChangeVariety()
{
	UpdateData();
	m_ctrl.UpdateSize (m_128x128);

	// Enable window
	GetDlgItem (IDC_JOUR)->EnableWindow (m_128x128!=3);
	GetDlgItem (IDC_NUIT)->EnableWindow (m_128x128!=3);
	GetDlgItem (IDC_ALPHA)->EnableWindow (m_128x128==2);
	GetDlgItem (IDC_BATCH_LOAD)->EnableWindow (m_128x128==2);

	if ((m_ctrl.Texture==3)&&(m_128x128!=2))
	{
		m_ctrl.Texture=2;
		((CButton*)GetDlgItem (IDC_ALPHA))->SetCheck (0);
		((CButton*)GetDlgItem (IDC_NUIT))->SetCheck (1);
	}

	if ((m_ctrl.Texture!=1)&&(m_128x128==3))
	{
		m_ctrl.Texture=1;
		((CButton*)GetDlgItem (IDC_ALPHA))->SetCheck (0);
		((CButton*)GetDlgItem (IDC_NUIT))->SetCheck (0);
		((CButton*)GetDlgItem (IDC_JOUR))->SetCheck (1);
	}

	m_ctrl.Invalidate ();
	UpdateData(FALSE);
}

void Browse::OnBatchLoad ()
{
	CFileDialog sFile (true, NULL, NULL, OFN_ENABLESIZING,
		"Targa bitmap (*.tga)|*.tga|All files (*.*)|*.*||",NULL);

	if (sFile.DoModal()==IDOK)
	{
		char sDrive[256];
		char sPath[256];
		char sName[256];
		char sExt[256];
		_splitpath (sFile.GetPathName(), sDrive, sPath, sName, sExt);

		// look for some numbers..
		char *sNumber=sName+strlen(sName)-1;
		while ((sNumber>sName)&&(*sNumber>='0')&&(*sNumber<='9'))
		{
			sNumber--;
		}
		sNumber[1]=0;

		bool rotate=false;
		if (::MessageBox (NULL, "Do you want to use rotation to reuse alpha tiles ?", "Import rotated tiles", MB_OK|MB_ICONQUESTION|MB_YESNO)==IDYES)
			rotate=true;

		for (int i=0; i<CTileSet::count; i++)
		{
			if (m_ctrl.Texture==3)
			{
				// Current transition
				CTileSet::TTransition transition=(CTileSet::TTransition)i;

				// Transition to patch
				CTileSetTransition* trans=tileBank2.getTileSet (land)->getTransition (transition);
				if (tileBank2.getTile (trans->getTile())->getRelativeFileName (CTile::alpha)=="")
				{
					// Continue ?
					int ok;

					// Try to load transition with rotation
					for (int rot=0; rot<4; rot++)
					{
						// Try to load a tile with a file name like /tiletransition0.tga
						char sName2[256];
						char sFinal[256];
						sprintf (sName2, "%s%02d", sName, (int)transition);
						_makepath (sFinal, sDrive, sPath, sName2, sExt);
						FILE *pFile=fopen (sFinal, "rb");

						// Close the file and add the tile if opened
						if (pFile)
						{
							fclose (pFile);
							ok=m_ctrl.InfoList.setTileTransitionAlpha (i, sFinal, (4-rot)%4);

							// End
							break;
						}

						// Rotate the transition
						transition=CTileSet::rotateTransition (transition);

						if (!rotate)
							break;
					}
					if (!ok)
						break;
				}
			}
			else
			{
				// Current transition
				CTileSet::TTransition transition=(CTileSet::TTransition)i;

				// Transition to patch
				CTileSetTransition* trans=tileBank2.getTileSet (land)->getTransition (transition);
				if (tileBank2.getTile (trans->getTile())->getRelativeFileName (m_ctrl.Texture==1?CTile::diffuse:CTile::additive)=="")
				{
					// Try to load a tile with a file name like /tiletransition0.tga
					char sName2[256];
					char sFinal[256];
					sprintf (sName2, "%s%02d", sName, (int)transition);
					_makepath (sFinal, sDrive, sPath, sName2, sExt);
					FILE *pFile=fopen (sFinal, "rb");

					// Close the file and add the tile if opened
					if (pFile)
					{
						fclose (pFile);
						if (!m_ctrl.InfoList.setTileTransition (i, sFinal, m_ctrl.Texture==1?CTile::diffuse:CTile::additive))
							break;
					}
				}
			}
		}
		m_ctrl.Invalidate ();

	}
}

void Browse::UpdateFlags ()
{
	SubGroup0=0;
	SubGroup1=0;
	SubGroup2=0;
	SubGroup3=0;
	SubGroup4=0;
	SubGroup5=0;
	SubGroup6=0;
	SubGroup7=0;
	SubGroup8=0;
	SubGroup9=0;
	SubGroup10=0;
	SubGroup11=0;

	// Flags
	uint or=0, and=0xffffffff;
	bool find=false;

	// For each 
	for (int i=0;i<m_ctrl.InfoList.GetSize(m_128x128);i++)
	{
		// Selected ?
		if (m_ctrl.InfoList.theList[m_128x128][i].Selected)
		{
			// Tile index
			sint index;

			// get flags
			switch (m_128x128)
			{
			case 0:
				// Tile index
				index=tileBank2.getTileSet (land)->getTile128 (i);
				break;
			case 1:
				// Tile index
				index=tileBank2.getTileSet (land)->getTile256 (i);
				break;
			case 2:
				// Tile index
				index=tileBank2.getTileSet (land)->getTransition (i)->getTile ();
				break;
			case 3:
				// not found
				index=-1;
				break;
			default:
				nlassert (0);	// no!
			}

			// valid flags
			if (index!=-1)
			{
				// Get flags
				or|=tileBank2.getTile (index)->getGroupFlags ();
				and&=tileBank2.getTile (index)->getGroupFlags ();

				// Find one
				find=true;
			}
		}
	}

	// Valid ctrl
	GetDlgItem (IDC_SUBGROUP0)->EnableWindow (find?TRUE:FALSE);
	GetDlgItem (IDC_SUBGROUP1)->EnableWindow (find?TRUE:FALSE);
	GetDlgItem (IDC_SUBGROUP2)->EnableWindow (find?TRUE:FALSE);
	GetDlgItem (IDC_SUBGROUP3)->EnableWindow (find?TRUE:FALSE);
	GetDlgItem (IDC_SUBGROUP4)->EnableWindow (find?TRUE:FALSE);
	GetDlgItem (IDC_SUBGROUP5)->EnableWindow (find?TRUE:FALSE);
	GetDlgItem (IDC_SUBGROUP6)->EnableWindow (find?TRUE:FALSE);
	GetDlgItem (IDC_SUBGROUP7)->EnableWindow (find?TRUE:FALSE);
	GetDlgItem (IDC_SUBGROUP8)->EnableWindow (find?TRUE:FALSE);
	GetDlgItem (IDC_SUBGROUP9)->EnableWindow (find?TRUE:FALSE);
	GetDlgItem (IDC_SUBGROUP10)->EnableWindow (find?TRUE:FALSE);
	GetDlgItem (IDC_SUBGROUP11)->EnableWindow (find?TRUE:FALSE);

	// Find at least one tile ?
	if (find)
	{
		// Set UI
		SubGroup0=(and&0x1)?1:(or&0x1)?2:0;
		SubGroup1=(and&0x2)?1:(or&0x2)?2:0;
		SubGroup2=(and&0x4)?1:(or&0x4)?2:0;
		SubGroup3=(and&0x8)?1:(or&0x8)?2:0;
		SubGroup4=(and&0x10)?1:(or&0x10)?2:0;
		SubGroup5=(and&0x20)?1:(or&0x20)?2:0;
		SubGroup6=(and&0x40)?1:(or&0x40)?2:0;
		SubGroup7=(and&0x80)?1:(or&0x80)?2:0;
		SubGroup8=(and&0x100)?1:(or&0x100)?2:0;
		SubGroup9=(and&0x200)?1:(or&0x200)?2:0;
		SubGroup10=(and&0x400)?1:(or&0x400)?2:0;
		SubGroup11=(and&0x800)?1:(or&0x800)?2:0;
	}

	// Update UI data
	UpdateData (FALSE);
}

void Browse::Flags (int flagNumber, bool go)
{
	// For each 
	for (int i=0;i<m_ctrl.InfoList.GetSize(m_128x128);i++)
	{
		// Selected ?
		if (m_ctrl.InfoList.theList[m_128x128][i].Selected)
		{
			// Tile index
			sint index;

			// get flags
			switch (m_128x128)
			{
			case 0:
				// Tile index
				index=tileBank2.getTileSet (land)->getTile128 (i);
				break;
			case 1:
				// Tile index
				index=tileBank2.getTileSet (land)->getTile256 (i);
				break;
			case 2:
				// Tile index
				index=tileBank2.getTileSet (land)->getTransition (i)->getTile ();
				break;
			default:
				nlassert (0);	// no!
			}

			// valid flags
			if (index!=-1)
			{
				// Get flags
				uint value=tileBank2.getTile (index)->getGroupFlags ();

				// Clear flag
				value&=~(1<<flagNumber);

				// Set the flag
				if (go)
					value|=(1<<flagNumber);

				// Setup
				tileBank2.getTile (index)->setGroupFlags (value);
			}
		}
	}
}


void Browse::OnSubgroup0() 
{
	// TODO: Add your control notification handler code here
	
	// Check if clicked
	UpdateData ();
	if (SubGroup0==2)
	{
		SubGroup0=0;
		UpdateData (FALSE);
	}

	nlassert (SubGroup0!=2);
	if (SubGroup0==0)
		Flags (0, false);
	if (SubGroup0==1)
		Flags (0, true);
}

void Browse::OnSubgroup1() 
{
	// TODO: Add your control notification handler code here
	
	// Check if clicked
	UpdateData ();
	if (SubGroup1==2)
	{
		SubGroup1=0;
		UpdateData (FALSE);
	}

	nlassert (SubGroup1!=2);
	if (SubGroup1==0)
		Flags (1, false);
	if (SubGroup1==1)
		Flags (1, true);
}

void Browse::OnSubgroup2() 
{
	// TODO: Add your control notification handler code here
	
	// Check if clicked
	UpdateData ();
	if (SubGroup2==2)
	{
		SubGroup2=0;
		UpdateData (FALSE);
	}

	nlassert (SubGroup2!=2);
	if (SubGroup2==0)
		Flags (2, false);
	if (SubGroup2==1)
		Flags (2, true);
}

void Browse::OnSubgroup3() 
{
	// TODO: Add your control notification handler code here
	
	// Check if clicked
	UpdateData ();
	if (SubGroup3==2)
	{
		SubGroup3=0;
		UpdateData (FALSE);
	}

	nlassert (SubGroup3!=2);
	if (SubGroup3==0)
		Flags (3, false);
	if (SubGroup3==1)
		Flags (3, true);
}

void Browse::OnSubgroup4() 
{
	// TODO: Add your control notification handler code here
	
	// Check if clicked
	UpdateData ();
	if (SubGroup4==2)
	{
		SubGroup4=0;
		UpdateData (FALSE);
	}

	nlassert (SubGroup4!=2);
	if (SubGroup4==0)
		Flags (4, false);
	if (SubGroup4==1)
		Flags (4, true);
}

void Browse::OnSubgroup5() 
{
	// TODO: Add your control notification handler code here
	
	// Check if clicked
	UpdateData ();
	if (SubGroup5==2)
	{
		SubGroup5=0;
		UpdateData (FALSE);
	}

	nlassert (SubGroup5!=2);
	if (SubGroup5==0)
		Flags (5, false);
	if (SubGroup5==1)
		Flags (5, true);
}

void Browse::OnSubgroup6() 
{
	// TODO: Add your control notification handler code here
	
	// Check if clicked
	UpdateData ();
	if (SubGroup6==2)
	{
		SubGroup6=0;
		UpdateData (FALSE);
	}

	nlassert (SubGroup6!=2);
	if (SubGroup6==0)
		Flags (6, false);
	if (SubGroup6==1)
		Flags (6, true);
}

void Browse::OnSubgroup7() 
{
	// TODO: Add your control notification handler code here
	
	// Check if clicked
	UpdateData ();
	if (SubGroup7==2)
	{
		SubGroup7=0;
		UpdateData (FALSE);
	}

	nlassert (SubGroup7!=2);
	if (SubGroup7==0)
		Flags (7, false);
	if (SubGroup7==1)
		Flags (7, true);
}

void Browse::OnSubgroup8() 
{
	// TODO: Add your control notification handler code here
	
	// Check if clicked
	UpdateData ();
	if (SubGroup8==2)
	{
		SubGroup8=0;
		UpdateData (FALSE);
	}

	nlassert (SubGroup8!=2);
	if (SubGroup8==0)
		Flags (8, false);
	if (SubGroup8==1)
		Flags (8, true);
}

void Browse::OnSubgroup9() 
{
	// TODO: Add your control notification handler code here
	
	// Check if clicked
	UpdateData ();
	if (SubGroup9==2)
	{
		SubGroup9=0;
		UpdateData (FALSE);
	}

	nlassert (SubGroup9!=2);
	if (SubGroup9==0)
		Flags (9, false);
	if (SubGroup9==1)
		Flags (9, true);
}

void Browse::OnSubgroup10() 
{
	// TODO: Add your control notification handler code here
	
	// Check if clicked
	UpdateData ();
	if (SubGroup10==2)
	{
		SubGroup10=0;
		UpdateData (FALSE);
	}

	nlassert (SubGroup10!=2);
	if (SubGroup10==0)
		Flags (10, false);
	if (SubGroup10==1)
		Flags (10, true);
}

void Browse::OnSubgroup11() 
{
	// TODO: Add your control notification handler code here
	
	// Check if clicked
	UpdateData ();
	if (SubGroup11==2)
	{
		SubGroup11=0;
		UpdateData (FALSE);
	}

	nlassert (SubGroup11!=2);
	if (SubGroup11==0)
		Flags (11, false);
	if (SubGroup11==1)
		Flags (11, true);
}

void Browse::OnExportBorder() 
{
	// Select a file
	CFileDialog sFile (false, NULL, NULL, OFN_ENABLESIZING,
		"Targa bitmap (*.tga)|*.tga|All files (*.*)|*.*||",NULL);
	if (sFile.DoModal()==IDOK)
	{
		// Get the border of the bank
		std::vector<NLMISC::CBGRA> array;

		// 256 or 128 ?
		int width, height;
		tileBank2.getTileSet (land)->getBorder128 (m_ctrl.Texture==1?CTile::diffuse:CTile::additive)->get (width, height, array);

		// Make a bitmap
		if (width&&height)
		{
			NLMISC::CBitmap bitmap;
			bitmap.resize (width, height, NLMISC::CBitmap::RGBA);

			// Get pixel
			CRGBA *pPixel=(CRGBA*)&bitmap.getPixels()[0];

			// Make a copy
			for (int i=0; i<width*height; i++)
			{
				// Copy the pixel
				pPixel->R=array[i].R;
				pPixel->G=array[i].G;
				pPixel->B=array[i].B;
				pPixel->A=array[i].A;
				pPixel++;
			}

			// Write the bitmap
			bool error=false;
			CString pathName=sFile.GetPathName();
			try
			{
				COFile file;
				if (file.open ((const char*)pathName))
				{
					// Export
					bitmap.writeTGA (file, 32);
				}
				else
					error=true;
			}
			catch (Exception& e)
			{
				const char *toto=e.what ();
				error=true;
			}

			// Error during export ?
			if (error)
			{
				// Error message
				char tmp[512];
				sprintf (tmp, "Can't write bitmap %s", (const char*)pathName);
				MessageBox (tmp, "Export border", MB_OK|MB_ICONEXCLAMATION);
			}
		}
	}
}

void Browse::OnImportBorder() 
{
	// Select a file
	CFileDialog sFile (true, NULL, NULL, OFN_ENABLESIZING,
		"Targa bitmap (*.tga)|*.tga|All files (*.*)|*.*||",NULL);
	if (sFile.DoModal()==IDOK)
	{
		// Get the border of the bank
		std::vector<NLMISC::CBGRA> array(128*128);

		// The bitmap
		NLMISC::CBitmap bitmap;

		// Read the bitmap
		bool error=false;
		CString pathName=sFile.GetPathName();
		try
		{
			CIFile file;
			if (file.open ((const char*)pathName))
			{
				// Export
				bitmap.load (file);
			}
			else
				error=true;
		}
		catch (Exception& e)
		{
			const char *toto=e.what ();
			error=true;
		}

		// Error during import ?
		if (error)
		{
			// Error message
			char tmp[512];
			sprintf (tmp, "Can't read bitmap %s", (const char*)pathName);
			MessageBox (tmp, "Import border", MB_OK|MB_ICONEXCLAMATION);
		}

		// Get pixel
		CRGBA *pPixel=(CRGBA*)&bitmap.getPixels()[0];

		// Good size
		if ((bitmap.getWidth()==128)&&(bitmap.getHeight()==128))
		{
			// Make a copy
			for (int i=0; i<128*128; i++)
			{
				// Copy the pixel
				array[i].R=pPixel->R;
				array[i].G=pPixel->G;
				array[i].B=pPixel->B;
				array[i].A=pPixel->A;
				pPixel++;
			}
		}
		else
		{
			// Error message
			char tmp[512];
			sprintf (tmp, "The bitmap must have a size of 128x128 (%s)", (const char*)pathName);
			MessageBox (tmp, "Import border", MB_OK|MB_ICONEXCLAMATION);
		}

		// 256 or 128 ?
		CTileBorder border;
		border.set (128, 128, array);
		tileBank2.getTileSet (land)->setBorder (m_ctrl.Texture==1?CTile::diffuse:CTile::additive, border);

		// Message
		MessageBox ("The border has been changed.", "Import border", MB_OK|MB_ICONINFORMATION);
	}
}
