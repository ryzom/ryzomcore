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
#include "tile_edit_exe.h"
#include "TileList.h"
#include "browse.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define SEPARE_X 5
#define SEPARE_TILE_TEXT_Y 5
#define SEPARE_TEXT_TILE_Y 5

/////////////////////////////////////////////////////////////////////////////
// TileList

TileList::TileList()
{
}

TileList::~TileList()
{
}


BEGIN_MESSAGE_MAP(TileList, CListBox)
	//{{AFX_MSG_MAP(TileList)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TileList message handlers

BOOL TileList::PreCreateWindow(CREATESTRUCT& cs) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	return CListBox::PreCreateWindow(cs);
}

void TileList::DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct )
{
	int x,y; //compute (x,y) coord of the item
	if (lpDrawItemStruct->itemID<0) return;
	int nb_tile_per_line = (lpDrawItemStruct->rcItem.right-SEPARE_X)/(SEPARE_X+sizetile_x);
	int nb_tile_per_column = (lpDrawItemStruct->rcItem.bottom-SEPARE_TEXT_TILE_Y)/(SEPARE_TEXT_TILE_Y+SEPARE_TILE_TEXT_Y+SIZETILE_Y);
	

}