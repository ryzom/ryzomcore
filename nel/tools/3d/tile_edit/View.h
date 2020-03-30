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

#if !defined(AFX_CTView_H__72269322_0419_4F61_BAA3_1B1BB2D3E34E__INCLUDED_)
#define AFX_CTView_H__72269322_0419_4F61_BAA3_1B1BB2D3E34E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CTView.h : header file
//
#include <vector>
#include <string>
#include "nel/3d/tile_bank.h"
#include "nel/misc/rgba.h"
#include <list>
#define TILE_BPP 24
#define NB_MAX_TILE 1500

#define SIZE_SMALL 32
#define SIZE_NORMAL 64
#define SIZE_BIG 128
#define SPACING_SMALL_X 15
#define SPACING_SMALL_Y 15
#define SPACING_NORMAL_X 20
#define SPACING_NORMAL_Y 20
#define SPACING_BIG_X 25
#define SPACING_BIG_Y 25

#define ID_MENU_ADD 10
#define ID_MENU_REPLACE 12
#define ID_MENU_SUPR_TILE 11
#define ID_MENU_SUPR_BITMAP 13

#define INS_DEBUT 1
#define INS_FIN 2
#define INS_CURSEUR 3

#define BUFFERSIZE 200 //le buffer ne peut pas contenir plus de 100 items

//extern int SortTile;
//extern int sortMode;
//extern __int64 flagGroupSort;
extern int showNULL;

/////////////////////////////////////////////////////////////////////////////
// CTView window
class TileInfo
{
public:
	//constructeurs
	TileInfo();

	bool Load (int index, std::vector<NLMISC::CBGRA>* Alpha);
	void Delete ();

	//data
	BITMAPINFO BmpInfo; 
	std::vector<NLMISC::CBGRA> Bits;
	BITMAPINFO alphaBmpInfo; 
	std::vector<NLMISC::CBGRA> alphaBits;
	BITMAPINFO nightBmpInfo; 
	std::vector<NLMISC::CBGRA> nightBits;

	const std::string& getRelativeFileName (NL3D::CTile::TBitmap type, int index);

	//int number; //son index dans la liste (different de son id selon les tris !)
	int loaded, nightLoaded, alphaLoaded;	//tells if the tile was already loaded or not
	int Selected;							//tells if the tile is selected
	int id;									//numero du tile
};

typedef std::vector<TileInfo> tilelist;



class TileList
{
public:	
	TileList();
	//ajoute un tile (bmp) dans la liste sans le charger
	//int Add (const char *path,const char *pathNight,const char *pathBump, int hh, int bb, int gg, int dd, unsigned int flags); 
	
	int addTile128 ();
	int addTile256 ();

	int setTile128 (int tile, const std::string& name, NL3D::CTile::TBitmap type);
	int setTile256 (int tile, const std::string& name, NL3D::CTile::TBitmap type);
	int setTileTransition (int tile, const std::string& name, NL3D::CTile::TBitmap type);
	int setTileTransitionAlpha (int tile, const std::string& name, int rot);
	int setDisplacement (int tile, const std::string& name);

	void removeTile128 (int index);
	void removeTile256 (int index);

	void clearTile128 (int index, NL3D::CTile::TBitmap bitmap);
	void clearTile256 (int index, NL3D::CTile::TBitmap bitmap);
	void clearTransition (int index, NL3D::CTile::TBitmap bitmap);
	void clearDisplacement (int index);
	
	void Reload(int first, int count, int n); //recharge en memoire une tranche de tiles
	
	int  GetSize(int n);
	tilelist::iterator GetFirst(int n);
	tilelist::iterator GetLast(int n);
	tilelist::iterator Get(int i, int n);
	int last_id;

public:
	tilelist theList[4];
#define theList128 theList[0]
#define theList256 theList[1]
#define theListTransition theList[2]
#define theListDisplacement theList[3]
	int _tileSet;
};

//charge une image (bmp pour le moment, tga,png,jpg plus tard ?)
extern int _LoadBitmap(const std::string& path,LPBITMAPINFO BitmapInfo,std::vector<NLMISC::CBGRA>& Tampon,std::vector<NLMISC::CBGRA>* Alpha, int rot);

class CTView : public CStatic
{
// Construction
public:
	CTView();
//owner functions
	int TileCtrl();
	void Init(int _land, int n);
	void Delete();
	void DrawTile(tilelist::iterator i,CDC *pDC,int clear,int n);
	int  LoadInListCtrl(tilelist::iterator iFirst,tilelist::iterator iLast);
	void DeleteTile(tilelist::iterator p);
	int  IsSelected(int i);
	void RemoveSelection(int n);
	void ShadeRect( CDC *pDC, CRect& rect ); //permet d'afficher un bitmap selectionne a la "windows style"
	void InsertItemInCtrlList(tilelist::iterator iFirst,tilelist::iterator iLast);
	void GetVisibility(int &First,int &Last, int n);
	int  GetNbTileLine(void);
	int  GetNbTileColumn(void);
	int	 GetIndex(LPPOINT pt, int n);
	tilelist::iterator GetTileSelection(tilelist::iterator i);
	POINT GetPos(int i);
	void UpdateSelection(LPRECT rect_,int mode, int n);
	void DrawDragRect(CDC *pDC,LPCRECT lpRect, SIZE size,LPCRECT lpRectLast, SIZE sizeLast, CBrush* pBrush = NULL, CBrush* pBrushLast = NULL);
	void UpdateSize(int n);
	void UpdateBar(int iFirst,int iLast, int n);

// Attributes
public:
//owner data
	std::string	LastPath;
	int		 smEdgeList; //semaphore
	int		 lockInsertion;
	TileList InfoList;
	//edgelist EdgeList; //liste de toutes les bordures 
	int		 sizetile_x,sizetile_y,sizetext_y;
	int		 sizeicon_x,sizeicon_y,spacing_x,spacing_y,spacing_tile_text;
	int		 scrollpos,lastVBarPos;
	int		 Sort; //radio button state (1,2,3)
	int		 Zoom;
	int		 Texture;
	int		 InfoTexte;
	CImageList *pImList;
	void	 *pipo_buffer;
	CBitmap	 *bmp;
	int		 count_;
	int		 ViewTileMode;
	TileInfo TileCroix;
	CPoint   MousePos;
	int		 iFirst,iLast; //indexes du premier et du dernier item du buffer
	int		 iFV,iLV; //index du premier et du dernier item visible
	int		 bPopup; //permet de savoir s'il le menu popup est active

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTView)
	protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CTView();

	// Generated message map functions
protected:
	//{{AFX_MSG(CTView)
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnPaint();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CTView_H__72269322_0419_4F61_BAA3_1B1BB2D3E34E__INCLUDED_)
