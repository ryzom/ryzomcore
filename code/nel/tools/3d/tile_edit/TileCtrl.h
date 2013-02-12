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

#if !defined(AFX_TILECTRL_H__4E768057_293D_4D27_8193_06423D1D7380__INCLUDED_)
#define AFX_TILECTRL_H__4E768057_293D_4D27_8193_06423D1D7380__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TileCtrl.h : header file

#include <list>

#define TILE_BPP 24
#define NB_MAX_TILE 1500

#define SIZE_SMALL 32
#define SIZE_NORMAL 64
#define SIZE_BIG 128
#define SPACING_SMALL_X 2
#define SPACING_SMALL_Y 2
#define SPACING_NORMAL_X 4
#define SPACING_NORMAL_Y 4
#define SPACING_BIG_X 6
#define SPACING_BIG_Y 6

#define INS_DEBUT 1
#define INS_FIN 2
#define INS_CURSEUR 3

#define BUFFERSIZE 200 //le buffer ne peut pas contenir plus de 100 items
/////////////////////////////////////////////////////////////////////////////
// TileCtrl window
class TileInfo
{
public:
	//constructeurs
	TileInfo();

	//data
	HBITMAP DibSection; BITMAPINFO BmpInfo; void *Bits;
	char *path;
	int Loaded; //tells if the tile was already loaded or not
	int Selected; //tells if the tile is selected
	int id; //numero du tile
	int h,b,g,d; //index dans la liste des bordures pour le haut, le bas, la gauche et la droite du tile
};

typedef std::list<TileInfo*> tilelist;

class TileList
{
public:	
	TileList();
	tilelist theList;
	tilelist::iterator i; //pointeur sur le dernier element de la liste
	int Add(const char *path); //ajoute un tile (bmp) dans la liste sans le charger
	void Delete(tilelist::iterator i,int n); 
	void DeleteAll(); //efface provisoirement les sections DIB et les *Bits;
	void Reload(CDC *pDC,tilelist::iterator iFirst,int n); //recharge en memoire une tranche de tiles
	int last_id;
	tilelist::iterator iFirst,iLast; // first and last index loaded in memory
};


class _Edge //permet de stocker toutes les bordures des tiles
{
public:
	_Edge();
	~_Edge();
	_Edge(const _Edge& edge);

	int size;
	char *line;

	void CreateH(TileInfo *tile);
	void CreateB(TileInfo *tile);
	void CreateG(TileInfo *tile);
	void CreateD(TileInfo *tile);
	int operator==(const _Edge &ed) const;
};

typedef list<_Edge> edgelist;

extern int CALLBACK CompareFunc(LPARAM lParam1,LPARAM lParam2,LPARAM lParamSort);
extern int _LoadBitmap(const std::string& path,LPBITMAPINFO BitmapInfo, std::vector<NLMISC::CBGRA>&Tampon, std::vector<NLMISC::CBGRA>* Alpha, int rot);



class TileCtrl : public CListCtrl
{
// Construction
public:
//owner functions
	TileCtrl();
	void Init();
	void Delete();
	void DrawTile(int i,CDC *pDC);
	void UpdateBuffer(); //permet de charger et d'effacer tous les buffers inutiles des tiles invisible
	int  LoadInListCtrl(tilelist::iterator iFirst,tilelist::iterator iLast);
	void DeleteTile(int i);
	int  IsSelected(int i);
	void CheckTile(TileInfo *theTile); //check si le tile est "tilable" avec un autre et initialise h,b,g,d dans TileInfo
	void ShadeRect( CDC *pDC, CRect& rect ); //permet d'afficher un bitmap selectionne a la "windows style"
	void InsertItemInCtrlList(tilelist::iterator iFirst,tilelist::iterator iLast);
	void GetVisibility(int &First,int &Last);
	int  GetNbTileLine(void);
	int  GetNbTileColumn(void);
	int	 GetIndex(LPPOINT pt);
	POINT GetPos(int i);

//owner data	
	TileList InfoList;
	edgelist EdgeList; //liste de toutes les bordures 
	int sizetile_x,sizetile_y;
	int sizeicon_x,sizeicon_y,spacing_x,spacing_y,spacing_tile_text;
	int scrollpos;
	int Sort; //radio button state (1,2,3)
	int Zoom;
	int Texture;
	int InfoTexte;
	CImageList *pImList;
	void *pipo_buffer;
	CBitmap *bmp;
	int count_;
	int ViewTileMode;
	TileInfo TileCroix;
	CPoint MousePos;
	int iFirst,iLast; //indexes du premier et du dernier item du buffer
	int iFV,iLV; //index du premier et du dernier item visible

// Attributes
public:


// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(TileCtrl)
	protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~TileCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(TileCtrl)
	afx_msg void OnPaint();
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TILECTRL_H__4E768057_293D_4D27_8193_06423D1D7380__INCLUDED_)
