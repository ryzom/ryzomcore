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

#if !defined(AFX_VEGETABLE_LIST_COLOR_H__4F2D3BE1_B010_4A29_8452_C0E274B69743__INCLUDED_)
#define AFX_VEGETABLE_LIST_COLOR_H__4F2D3BE1_B010_4A29_8452_C0E274B69743__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// vegetable_list_color.h : header file
//

#include "nel/misc/rgba.h"

using NLMISC::CRGBA;

/////////////////////////////////////////////////////////////////////////////
// CVegetableListColor window

class CVegetableListColor : public CListBox
{
// Construction
public:
	CVegetableListColor();

// Attributes
public:

// Operations
public:

	// Setup.
	void setCtrlID(uint id) { _Id = id ; }
	// itemXXX is the total space taken by an item. squareXXX is the size of the colored square
	void setupItem(uint itemWidth, uint itemHeight, uint squareWidth, uint squareHeight );


	// Value mgt
	// clear all values
	void	clear();
	// add a value after all
	void	addValue(CRGBA color);
	// insert a value after the current selected element.
	void	insertValueBeforeCurSel(CRGBA color);
	// remove the selected element
	void	removeCurSelValue();
	// change the color of the selected element
	void	changeCurSelValue(CRGBA color);
	// get the corresponding color of a value
	CRGBA	getValue(uint id);
	// get the array of colors
	const std::vector<CRGBA>	&getColors() const {return _Colors;}


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVegetableListColor)
	public:
	virtual int CompareItem(LPCOMPAREITEMSTRUCT lpCompareItemStruct);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CVegetableListColor();

	// Generated message map functions
protected:
	//{{AFX_MSG(CVegetableListColor)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

	uint _Id ;
	uint _ItemWidth, _ItemHeight;
	uint _SquareWidth, _SquareHeight;

	std::vector<CRGBA>		_Colors;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VEGETABLE_LIST_COLOR_H__4F2D3BE1_B010_4A29_8452_C0E274B69743__INCLUDED_)
