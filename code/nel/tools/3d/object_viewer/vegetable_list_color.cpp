// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

// vegetable_list_color.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "vegetable_list_color.h"

/////////////////////////////////////////////////////////////////////////////
// CVegetableListColor

CVegetableListColor::CVegetableListColor()
{
}

CVegetableListColor::~CVegetableListColor()
{
}


BEGIN_MESSAGE_MAP(CVegetableListColor, CListBox)
	//{{AFX_MSG_MAP(CVegetableListColor)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVegetableListColor message handlers

// ***************************************************************************
int CVegetableListColor::CompareItem(LPCOMPAREITEMSTRUCT lpCompareItemStruct) 
{
	// TODO: Add your code to determine the sorting order of the specified items
	// return -1 = item 1 sorts before item 2
	// return 0 = item 1 and item 2 sort the same
	// return 1 = item 1 sorts after item 2
	
	return 0;
}

// ***************************************************************************
void CVegetableListColor::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	if(lpDrawItemStruct->itemID!=LB_ERR)
	{
		CDC dc;
		dc.Attach(lpDrawItemStruct->hDC) ;

		// Fill the square with gray first.
		CBrush	backBrush(RGB(200, 200, 200));
		dc.FillRect(&lpDrawItemStruct->rcItem, &backBrush);

		// Fill the square with good color.
		CRect	rect;
		rect.left= lpDrawItemStruct->rcItem.left + (_ItemWidth - _SquareWidth)/2;
		rect.top= lpDrawItemStruct->rcItem.top + (_ItemHeight - _SquareHeight)/2;
		rect.right= rect.left + _SquareWidth;
		rect.bottom= rect.top + _SquareHeight;
		uint	itemId= lpDrawItemStruct->itemID;
		CRGBA	color= getValue(itemId);
		CBrush	fillBrush(RGB(color.R, color.G, color.B));
		dc.FillRect(&rect, &fillBrush);

		// draw the selection.
		CBrush	selectedBrush;
		if(lpDrawItemStruct->itemState & ODS_SELECTED)
			selectedBrush.CreateSolidBrush(RGB(0, 0, 0));
		else
			selectedBrush.CreateSolidBrush(RGB(255, 255, 255));
		dc.FrameRect(&(lpDrawItemStruct->rcItem), &selectedBrush);

		
		dc.Detach();
	}
}

// ***************************************************************************
void CVegetableListColor::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct) 
{
	lpMeasureItemStruct->CtlType= ODT_LISTBOX;
	lpMeasureItemStruct->CtlID= _Id;
	lpMeasureItemStruct->itemWidth= _ItemWidth;
	lpMeasureItemStruct->itemHeight= _ItemHeight;
}


// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void CVegetableListColor::setupItem(uint itemWidth, uint itemHeight, uint squareWidth, uint squareHeight )
{
	_ItemWidth= itemWidth;
	_ItemHeight= itemHeight;
	_SquareWidth= squareWidth;
	_SquareHeight= squareHeight;

	// set the appropriate column size of the listbox
	SetColumnWidth(itemWidth);
	SetItemHeight(0, itemHeight);
}


// ***************************************************************************
void	CVegetableListColor::clear()
{
	// clear the listbox
	ResetContent();

	// append a color to the array.
	_Colors.clear();
}


// ***************************************************************************
void	CVegetableListColor::addValue(CRGBA color)
{
	// Append a dummy string to the list box.
	AddString(" ");

	// append a color to the array.
	_Colors.push_back(color);
}

// ***************************************************************************
void	CVegetableListColor::insertValueBeforeCurSel(CRGBA color)
{
	int	id= GetCurSel();
	if(id == LB_ERR)
		addValue(color);
	else
	{
		// insert a dummy string to the list box.
		InsertString(id, " ");

		// insert a color to the array.
		_Colors.insert(_Colors.begin()+id, color);
	}
}

// ***************************************************************************
void	CVegetableListColor::removeCurSelValue()
{
	int	id= GetCurSel();
	if(id != LB_ERR)
	{
		// insert a dummy string to the list box.
		DeleteString(id);

		// insert a color to the array.
		_Colors.erase(_Colors.begin()+id);

		// set the current selection to the same id, or prec id.
		if(id>=GetCount())
			SetCurSel(id-1);
		else
			SetCurSel(id);
	}
}

// ***************************************************************************
void	CVegetableListColor::changeCurSelValue(CRGBA color)
{
	int	id= GetCurSel();
	if(id != LB_ERR)
	{
		_Colors[id]= color;
		Invalidate();
	}
}

// ***************************************************************************
CRGBA	CVegetableListColor::getValue(uint id)
{
	nlassert(id < _Colors.size());
	return _Colors[id];
}
