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
#include "TileCtrl.h"
#include "browse.h"



BEGIN_MESSAGE_MAP(TileCtrl, CListCtrl)
	//{{AFX_MSG_MAP(TileCtrl)
	ON_WM_PAINT()
	ON_WM_DROPFILES()
	ON_WM_RBUTTONDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_VSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TileCtrl message handlers



//gloabal
int CALLBACK CompareFunc(LPARAM lParam1,LPARAM lParam2,LPARAM lParamSort)
{
	TileCtrl *theCtrl = (TileCtrl*)lParamSort;
	int i1=((TileInfo*)lParam1)->id;
	int i2=((TileInfo*)lParam2)->id;
	if (i1==i2) return 0;
	if (i1<i2) return -1;
	else return 1;
}





//TileInfo
TileInfo::TileInfo()
{
	Loaded=h=g=b=d=0; Bits = NULL;
	Selected = 0;
}








//Edge
_Edge::_Edge()
{
	line = 0;
}

_Edge::~_Edge()
{
	if (line) delete line;
}

_Edge::_Edge(const _Edge& edge)
{
	line = new char[edge.size*3];
	memcpy(line,edge.line,edge.size*3);
	size = edge.size;
}

int _Edge::operator == (const _Edge & ed) const
{
	if (ed.size!=size) return 0;
	for (int i=0;i<size*3;i++)
	{
		if (ed.line[i]!=line[i]) return 0;
	}
	return 1;
}

void _Edge::CreateH(TileInfo *tile)
{
	size = tile->BmpInfo.bmiHeader.biWidth-2; //on n'inclut pas les coins dans les bordures
	line = new char[3*size];
	for (int i=0;i<size;i++)
	{
		line[i*3]=((char*)tile->Bits)[(i+1)*3+(tile->BmpInfo.bmiHeader.biWidth)*(tile->BmpInfo.bmiHeader.biHeight-1)*3];
		line[i*3+1]=((char*)tile->Bits)[(i+1)*3+1+(tile->BmpInfo.bmiHeader.biWidth)*(tile->BmpInfo.bmiHeader.biHeight-1)*3];
		line[i*3+2]=((char*)tile->Bits)[(i+1)*3+2+(tile->BmpInfo.bmiHeader.biWidth)*(tile->BmpInfo.bmiHeader.biHeight-1)*3];
	}
}

void _Edge::CreateB(TileInfo *tile)
{
	size = tile->BmpInfo.bmiHeader.biWidth-2; //on n'inclut pas les coins dans les bordures
	line = new char[3*size];
	for (int i=0;i<size;i++)
	{
		line[i*3]=((char*)tile->Bits)[(i+1)*3];
		line[i*3+1]=((char*)tile->Bits)[(i+1)*3+1];
		line[i*3+2]=((char*)tile->Bits)[(i+1)*3+2];
	}
}

void _Edge::CreateG(TileInfo *tile)
{
	size = tile->BmpInfo.bmiHeader.biHeight-2; //on n'inclut pas les coins dans les bordures
	line = new char[3*size];
	for (int i=0;i<size;i++)
	{
		line[i*3]=((char*)tile->Bits)[(i+1)*3*tile->BmpInfo.bmiHeader.biWidth];
		line[i*3+1]=((char*)tile->Bits)[(i+1)*3*tile->BmpInfo.bmiHeader.biWidth+1];
		line[i*3+2]=((char*)tile->Bits)[(i+1)*3*tile->BmpInfo.bmiHeader.biWidth+2];
	}
}

void _Edge::CreateD(TileInfo *tile)
{
	size = tile->BmpInfo.bmiHeader.biHeight-2; //on n'inclut pas les coins dans les bordures
	line = new char[3*size];
	for (int i=0;i<size;i++)
	{
		line[i*3]=((char*)tile->Bits)[(i+1)*3*tile->BmpInfo.bmiHeader.biWidth+(tile->BmpInfo.bmiHeader.biWidth-1)*3];
		line[i*3+1]=((char*)tile->Bits)[(i+1)*3*tile->BmpInfo.bmiHeader.biWidth+1+(tile->BmpInfo.bmiHeader.biWidth-1)*3];
		line[i*3+2]=((char*)tile->Bits)[(i+1)*3*tile->BmpInfo.bmiHeader.biWidth+2+(tile->BmpInfo.bmiHeader.biWidth-1)*3];
	}
}








//TileList
TileList::TileList()
{
	last_id = 0;
	i=NULL;	
}

int TileList::Add(const char *path)
{
	TileInfo *info = new TileInfo;
	info->id = last_id++;
	if (path)
	{
		info->path = new char[strlen(path)];
		strcpy(info->path,path);
	}
	theList.insert(theList.end(),info);
	if (i==NULL) i=theList.begin();
	else i++;
	return 1;
}

void TileList::Delete(tilelist::iterator i,int n)
{
	for (int k=0;k<n;k++)
	{
		if ((*i)->Loaded)
		{
			delete (*i)->Bits; 
			(*i)->Loaded=0;
		// TODO : delete DIB section
		}		
		i++;
	}
}

void TileList::DeleteAll()
{
	int size = theList.size();
	if (size==0) return;
	Delete(theList.begin()++,size);
}

char buffer_bidon[SIZE_BIG*SIZE_BIG*3];
void TileList::Reload(CDC *pDC,tilelist::iterator iFirst,int n) //recharge en memoire une tranche de tiles
{
	/*tilelist::iterator p = iFirst;
	for (int i=0;i<n;i++)
	{
		if (!(*p)->Loaded && _LoadBitmap((*p)->path,&(*p)->BmpInfo,&(*p)->Bits))
		{
			(*p)->Loaded=1;
			(*p)->DibSection = CreateDIBSection(pDC->m_hDC,&(*p)->BmpInfo,DIB_RGB_COLORS,(void**)&buffer_bidon,0,0);
		}
		p++;
	}*/
}







//TileCtrl
TileCtrl::TileCtrl()
{
	sizetile_x = SIZE_SMALL; sizetile_y = SIZE_SMALL;
	spacing_x = SPACING_SMALL_X; spacing_y = SPACING_SMALL_Y; 
	Zoom=1; Texture = 1; Sort = 1; InfoTexte = 1;
	count_ = 0; ViewTileMode = 0;
	iFirst = iLast = 0;
	scrollpos = 0;
}

TileCtrl::~TileCtrl()
{
}

void TileCtrl::Init()
{	
	spacing_x = 10; spacing_y = 10; spacing_tile_text = 5;
	sizeicon_x = spacing_tile_text + sizetile_x;
	sizeicon_y = spacing_tile_text + sizetile_y;
	pipo_buffer = (void *)new char[sizetile_x * sizetile_y * 3];
	bmp = new CBitmap;
	bmp->CreateBitmap(sizetile_x,sizetile_y,1,24,pipo_buffer);
	pImList = new CImageList;
	pImList->Create(sizetile_x,sizetile_y,ILC_COLOR24,0,1);
	pImList->Add(bmp,(CBitmap*)NULL);
	SetImageList(pImList,0);
	char name[256];
	char *defautpath = ((SelectionTerritoire*)GetParent()->GetParent())->DefautPath.GetBuffer(256);
	/*sprintf(name,"%s%s",defautpath,"croix.bmp");
	if (_LoadBitmap(name,&TileCroix.BmpInfo,&TileCroix.Bits,false,false))
	{
		int size=TileCroix.BmpInfo.bmiHeader.biHeight*TileCroix.BmpInfo.bmiHeader.biWidth*TileCroix.BmpInfo.bmiHeader.biBitCount/8;
		char *temp = new char[size];
		TileCroix.DibSection = CreateDIBSection(GetDC()->m_hDC,&TileCroix.BmpInfo,DIB_RGB_COLORS,(void**)&temp,0,0);
	}*/
	count_=1;
}

void TileCtrl::Delete()
{
	count_=0; pImList = 0;
}

int TileCtrl::GetNbTileLine(void)
{
	RECT rect; GetClientRect(&rect);
	return ((rect.right - rect.left - spacing_x)/(sizeicon_x + spacing_x));
}

int TileCtrl::GetNbTileColumn(void)
{
	RECT rect; GetClientRect(&rect);
	return ((rect.bottom - rect.top - spacing_y)/(sizeicon_y + spacing_y));
}

void TileCtrl::GetVisibility(int &First,int &Last) //retourne l'indice du premier et du dernier item visible dans la fenetre
{
	RECT rect; 
	int i = GetNbTileLine();
	int j = GetNbTileColumn();
	GetClientRect(&rect);
	First = (rect.top + scrollpos - spacing_y)/(sizeicon_y + spacing_y);
	if (First<0) First = 0;
	else First *= i;
	Last = First + i*(j+1);	
	if (InfoList.theList.size()>0 && Last>=InfoList.theList.size()) Last = InfoList.theList.size()-1;
}

int TileCtrl::GetIndex(LPPOINT pt) //retourne l'index d'un incone a partir de sa position dans le fenetre
//si le curseur n'est pas sur un icon, retourne -1
{
	int i = GetNbTileLine();
	if ((pt->y+scrollpos)<spacing_y) return -1;
	int y = (pt->y + scrollpos - spacing_y)%(spacing_y + sizeicon_y);
	if (y>sizeicon_y) return -1;
	int il = (pt->y + scrollpos - spacing_y)/(spacing_y + sizeicon_y);
	int x = (pt->x - spacing_x)%(spacing_x + sizeicon_x);
	if (x>sizeicon_x) return -1;
	int ic = (pt->x - spacing_x)/(spacing_x + sizeicon_x);
	int ret = ic + il*i;
	if (i<InfoList.theList.size()) return i;
	else return -1;
}

POINT TileCtrl::GetPos(int i) //fonction inverse de GetIndex
{
	POINT ret;
	int nl = GetNbTileLine();
	ret.x = (i%nl)*(spacing_x + sizeicon_x) + spacing_x;
	ret.y = (i/nl)*(spacing_y + sizeicon_y) + spacing_y + scrollpos;	
	return ret;
}

void TileCtrl::UpdateBuffer() //gestion du cache
{
	int c = InfoList.theList.size();
	if (!c) return;

	if (iFirst==-1) //le buffer est vide
	{
		int NbItem =c-iFV; //nbitem : nb d'item a charger
		if (NbItem>BUFFERSIZE) NbItem = BUFFERSIZE;
		tilelist::iterator p=InfoList.theList.begin();
		for (int i=0;i<iFV;i++) p++;
		ASSERT(p!=InfoList.theList.end());
		InfoList.Reload(GetDC(),p,NbItem);
		iFirst = iFV; 
		iLast = iFV + NbItem-1;
	}
	else
	{
		if (iFV<iFirst) iFirst = iFV;
		else if (iLV>iLast) iLast = iLV;
		
		tilelist::iterator p=InfoList.theList.begin();
		for (int i=0;i<iFirst;i++) p++;
		InfoList.Reload(GetDC(),p,iLast-iFirst+1);
	}
}
		
void TileCtrl::CheckTile(TileInfo *theTile)
{
	_Edge h,b,g,d;
	b.CreateB(theTile);
	h.CreateH(theTile);
	g.CreateG(theTile);
	d.CreateD(theTile);

	_Edge test = b;

	int found=0;
	int i=0;
	for (edgelist::iterator p=EdgeList.begin();p!=EdgeList.end();++p)
	{
		if (b==*p)
		{
			found=1;
			theTile->b=i;
			break;
		}
		i++;
	}
	if (!found) 
	{
		EdgeList.insert(EdgeList.end(),b);
		theTile->b=i;
	}

	found=0;
	i=0;
	for (p=EdgeList.begin();p!=EdgeList.end();++p)
	{
		if (h==*p)
		{
			found=1;
			theTile->h=i;
			break;
		}
		i++;
	}
	if (!found) 
	{
		EdgeList.insert(EdgeList.end(),h);
		theTile->h=i;
	}

	found=0;
	i=0;
	for (p=EdgeList.begin();p!=EdgeList.end();++p)
	{
		if (g==*p)
		{
			found=1;
			theTile->g=i;
			break;
		}
		i++;
	}
	if (!found) 
	{
		EdgeList.insert(EdgeList.end(),g);
		theTile->g=i;
	}

	found=0;
	i=0;
	for (p=EdgeList.begin();p!=EdgeList.end();++p)
	{
		if (d==*p)
		{
			found=1;
			theTile->d=i;
			break;
		}
		i++;
	}
	if (!found) 
	{
		EdgeList.insert(EdgeList.end(),d);
		theTile->d=i;
	}
}


void TileCtrl::DeleteTile(int i)
{
/*	for (tilelist::iterator p=InfoList.theList.begin();p!=InfoList.theList.end();++p)
	{
		if (*p==(TileInfo*)GetItemData(i)) 
		{
			InfoList.Delete(p,1);
		}
	}*/
	
	//this->DeleteItem(i); //temp, should first delete the bitmap file and the DIB section
	/*TileInfo *info = (TileInfo*)GetItemData(i);
	if (info && info->Loaded)
	{
		//delete info->DibSection;
		delete info->Bits;
		if (info->id==last_id-1) {
			do {
				DeleteItem(i--);
				last_id--;			
			} while(i>=0 && !((TileInfo*)GetItemData(i))->Loaded);
		}
		else
		{
			info->Loaded=0;
			LVITEM item;
			item.mask=LVIF_STATE;
			item.stateMask=LVIS_SELECTED;
			item.state=0;
			SetItemState(i,&item);
		}
	}
	SortItems(CompareFunc,(DWORD)this);*/
}

int TileCtrl::IsSelected(int i)
{
	POSITION pos = this->GetFirstSelectedItemPosition();
	for (int k=0;k<GetSelectedCount();k++)
	{
		if (GetNextSelectedItem(pos)==i) return 1;
	}
	return 0;
}

void TileCtrl::DrawTile(int i,CDC *pDC)
{
	TileInfo *bmp;// = (TileInfo*)this->GetItemData(i);	
	tilelist::iterator p=InfoList.theList.begin();
	for (int k=0;k<i && p!=InfoList.theList.end();k++) p++;
	if (p==InfoList.theList.end()) return;
	bmp = *p;
	if (bmp)
	{
		POINT pt;
		pt = GetPos(i);
		if (!bmp->Loaded)
		{
			StretchDIBits(pDC->m_hDC,pt.x,pt.y,
				sizeicon_x,sizeicon_y,0,0,
				TileCroix.BmpInfo.bmiHeader.biWidth,TileCroix.BmpInfo.bmiHeader.biHeight,
				TileCroix.Bits,&TileCroix.BmpInfo,DIB_RGB_COLORS,SRCCOPY);
		}
		else
		{
			RECT rect;
			GetClientRect(&rect);
			StretchDIBits(pDC->m_hDC,pt.x,pt.y,
				rect.right - rect.left,rect.bottom - rect.top,0,0,
				bmp->BmpInfo.bmiHeader.biWidth,bmp->BmpInfo.bmiHeader.biHeight,
				bmp->Bits,&bmp->BmpInfo,DIB_RGB_COLORS,SRCCOPY);
		}
				
		CString str= GetItemText(i,0);
		char temp[100];
		char Name[256];
		if (InfoTexte==2)
		{
			_splitpath(str.GetBuffer(256),temp,temp,Name,temp);		
		}
		else
		{
			sprintf(Name,"%d",bmp->id);
		}
	}
}

void TileCtrl::ShadeRect( CDC *pDC, CRect& rect )
                     {
                             // Bit pattern for a monochrome brush with every
                             // other pixel turned off
                             WORD Bits[8] = { 0x0055, 0x00aa, 0x0055, 0x00aa,
                                              0x0055, 0x00aa, 0x0055, 0x00aa };

                             CBitmap bmBrush;
                             CBrush brush;

                             // Need a monochrome pattern bitmap
                             bmBrush.CreateBitmap( 8, 8, 1, 1, &Bits );

                             // Create the pattern brush
                             brush.CreatePatternBrush( &bmBrush );

                             CBrush *pOldBrush = pDC->SelectObject( &brush );

                             // Turn every other pixel to black
                             COLORREF clrBk = pDC->SetBkColor( RGB(255,255,255) );
                             COLORREF clrText = pDC->SetTextColor( RGB(0,0,0) );
                             // 0x00A000C9 is the ROP code to AND the brush with the destination
                             pDC->PatBlt(rect.left, rect.top, rect.Width(), rect.Height(), 
                                     (DWORD)0x00A000C9);                     //DPa - raster code

                             pDC->SetBkColor( RGB(0,0,0) );
                             pDC->SetTextColor( GetSysColor(COLOR_HIGHLIGHT) );
                             // 0x00FA0089 is the ROP code to OR the brush with the destination
                             pDC->PatBlt(rect.left, rect.top, rect.Width(), rect.Height(), 
                                     (DWORD)0x00FA0089);                     //DPo - raster code

                             // Restore the device context
                             pDC->SelectObject( pOldBrush );
                             pDC->SetBkColor( clrBk );
                             pDC->SetTextColor( clrText );
                     }

void TileCtrl::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	Browse *parent = (Browse*)this->GetParent();
	GetVisibility(iFV,iLV);
	UpdateBuffer();
	int olds = sizetile_x;
	if (Zoom==1) {sizetile_x = sizetile_y = SIZE_SMALL; spacing_x = SPACING_SMALL_X; spacing_y = SPACING_SMALL_Y;}
	if (Zoom==2) {sizetile_x = sizetile_y = SIZE_NORMAL; spacing_x = SPACING_NORMAL_X; spacing_y = SPACING_NORMAL_Y;}
	if (Zoom==3) {sizetile_x = sizetile_y = SIZE_BIG; spacing_x = SPACING_BIG_X; spacing_y = SPACING_BIG_Y;}
	if (olds!=sizetile_x && InfoList.theList.size())
	{
		bmp->DeleteObject();
		bmp->CreateBitmap(sizetile_x,sizetile_y,1,24,pipo_buffer);
		pImList->DeleteImageList();

		pImList->Create(sizetile_x,sizetile_y,ILC_COLOR24,0,1);
		pImList->Add(bmp,(CBitmap *)0);
		//SetIconSpacing(spacing_x,spacing_y);
		SetImageList(pImList,0);
	}
	CFont font;
	font.CreateFont(-10,0,0,0,FW_THIN,false,false,false,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,FIXED_PITCH,NULL);
	CFont* oldFont=dc.SelectObject(&font);
	for (int i=iFV;i<=iLV;i++) DrawTile(i,&dc);
	::ReleaseDC (*this, dc);
	dc.SelectObject(oldFont);
	// Do not call CListCtrl::OnPaint() for painting messages
}

void TileCtrl::OnDropFiles(HDROP hDropInfo) 
{
	// TODO: Add your message handler code here and/or call default
	//first : on verifie s'il les tiles doivent etre inseres
	char FileName[256];
	int count=DragQueryFile(hDropInfo,0xffffffff,FileName,256); //count = nombre de fichiers dans le drop
	

	POINT pos;
	DragQueryPoint(hDropInfo,&pos); //retrieve cursor position
	for (int i=0;i<count;i++) 
	{
		DragQueryFile(hDropInfo,i,FileName,256);
		if (!InfoList.Add(FileName)) return;
//		InfoList.Reload(GetDC(),InfoList.i,1);
		int iItem = InfoList.theList.size();
		char num[10];
		sprintf(num,"%d",iItem);
		InsertItem(iItem,num,0);
		SetItemData(iItem,(DWORD)*InfoList.i);
	}
	
	i = InfoList.theList.size();
//	UpdateBuffer();
	CListCtrl::OnDropFiles(hDropInfo);
}

void TileCtrl::InsertItemInCtrlList(tilelist::iterator iFirst,tilelist::iterator iLast)
{
	int iItem = InfoList.theList.size();
	for (tilelist::iterator i=iFirst;i!=iLast;++i)
	{
		char num[10];
		sprintf(num,"%d",iItem);
		InsertItem(iItem,num,0);
		SetItemData(iItem++,(DWORD)(*i));
	}
}

LRESULT TileCtrl::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (message==WM_MOUSEMOVE)
	{
		MousePos.x = LOWORD(lParam);
		MousePos.y = HIWORD(lParam);
	}
	if (message==WM_DRAWITEM)
	{
		int toto=0;
	}
	if (message==WM_CLOSE) Delete();
	if (message==WM_COMMAND) //The user had selected an item on popup menu
	{
		int id=LOWORD(wParam);
		if (id==3)
		{
			CFileDialog *load = new CFileDialog(true,"bmp",NULL,OFN_ALLOWMULTISELECT | OFN_ENABLESIZING,"*.bmp ||",this->GetParent());
			if (load->DoModal()==IDOK)
			{
				POSITION p = load->GetStartPosition(); //la doc dit que p=NULL quand il n'y a pas de selection : c'est faux, genial les MFC
				while (p)
				{
					CString str = load->GetNextPathName(p);
					if (str!=CString(""))
					{
						const char *pathname = (LPCTSTR)str;
						InfoList.Add(pathname);
						InfoList.Reload(GetDC(),InfoList.i,1);
						CheckTile(*InfoList.i);
					}
				}				
			}
			delete load;
			//UpdateBuffer();
		}
		else if (id==1)
		{
			POSITION p = this->GetFirstSelectedItemPosition();
			while (p)
			{
				int i = this->GetNextSelectedItem(p);
				DeleteTile(i);
			}
			//UpdateBuffer();
		}
		else if (id==0)
		{
			CFileDialog *load = new CFileDialog(true,"bmp",NULL,OFN_ENABLESIZING,"*.bmp ||",this->GetParent());
			if (load->DoModal()==IDOK)
			{
				POSITION p = load->GetStartPosition(); //la doc dit que p=NULL quand il n'y a pas de selection : c'est faux, genial les MFC
				CString str = load->GetNextPathName(p);
				if (str!=CString(""))
				{
					const char *pathname = (LPCTSTR)str;
					POSITION sel = GetFirstSelectedItemPosition();				
					InfoList.Add(pathname);
//					InfoList.Reload(GetDC(),InfoList.i,1);
					int iItem = InfoList.theList.size();
					char num[10];
					sprintf(num,"%d",iItem);
					InsertItem(iItem,num,0);
					SetItemData(iItem,(DWORD)*InfoList.i);
				}
			}
			delete load;
			//UpdateBuffer();
		}
		else if (id==2)
		{
			ViewTileMode = 1;
			Browse *parent = (Browse*) this->GetParent();
			POSITION sel = GetFirstSelectedItemPosition();
			int i = GetNextSelectedItem(sel);
			parent->TileSelected = (TileInfo*)GetItemData(i);
			parent->SendMessage(WM_PAINT,0,0);
			//UpdateBuffer();
		}
		else if (id==4)
		{
			ViewTileMode = 0; int iItem=0;
			((Browse*)GetParent())->TileSelected = NULL;
			GetParent()->SendMessage(WM_PAINT,0,0);
			DeleteAllItems();
			InsertItemInCtrlList(InfoList.theList.begin(),InfoList.theList.end());
			//UpdateBuffer();
		}
		SendMessage(WM_PAINT,0,0);
	}
	return CListCtrl::WindowProc(message, wParam, lParam);
}
	

void TileCtrl::OnRButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	RECT wndpos; CMenu popup;
	GetParent->GetWindowRect(&wndpos);
	popup.CreatePopupMenu();
	if (!ViewTileMode)
	{
		popup.AppendMenu(GetSelectedCount()==1 ? MF_STRING : MF_STRING | MF_GRAYED,0,"Remplacer...");
		popup.AppendMenu(GetSelectedCount()>0 ? MF_STRING : MF_STRING | MF_GRAYED,1,"Supprimer");
		popup.AppendMenu(MF_STRING,3,"Inserer...");
	}
	popup.TrackPopupMenu(TPM_LEFTALIGN,MousePos.x+wndpos.left,MousePos.y+wndpos.top,GetParent(),NULL);
	CListCtrl::OnRButtonDown(nFlags, point);
}

void TileCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	SendMessage(WM_PAINT,0,0);
	CListCtrl::OnLButtonDown(nFlags, point);
}

void TileCtrl::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	if (GetSelectedCount()==1)
	{
		ViewTileMode = 1;
		Browse *parent = (Browse*) this->GetParent();
		POSITION sel = GetFirstSelectedItemPosition();
		int i = GetNextSelectedItem(sel);
		parent->TileSelected = (TileInfo*)GetItemData(i);
		parent->SendMessage(WM_PAINT,0,0);
		CListCtrl::OnLButtonDblClk(nFlags, point);
	}	
}

void TileCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// TODO: Add your message handler code here and/or call default
	CListCtrl::OnVScroll(nSBCode, nPos, pScrollBar);
}
