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

#if !defined(AFX_LIST_BOX_COLOR_H__C4882C3B_0E50_4BC6_A51A_EAA1B4C2E479__INCLUDED_)
#define AFX_LIST_BOX_COLOR_H__C4882C3B_0E50_4BC6_A51A_EAA1B4C2E479__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// list_box_color.h : header file
//


namespace NLMISC
{
	class CBitmap;
}


/////////////////////////////////////////////////////////////////////////////
// CListBoxColor window

// A list box with color buttons.
// Additionnaly, check box can be added
// TODO nico : rename this CListBoxEntity or something like that
class CListBoxColor : public CCheckListBox
{
// Construction
public:
	CListBoxColor();	
	

	void		setColor(uint index, COLORREF col);
	COLORREF	getColor(uint index) const;
	void		enableCheckBoxes(bool enabled) { _CheckBoxEnabled = enabled; Invalidate(); }
	// enable icons
	void		enableIcons(bool enabled) { _IconsEnabled = enabled; Invalidate(); }

	/** Preload a bitmap where packed icons are taken from
	  * NB : bitmap must be in TGA format
	  */
	void		setIconBitmap(const NLMISC::CBitmap &bm, uint iconSize);

	/** Set an icon from the big bitmap for the given index
      * Set -1 for x or y to disable icons for that item
	  */
	void		setIcon(uint index, sint srcX, sint srcY);
	void		getIcon(uint index, sint &x, sint &y);


// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CListBoxColor)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CListBoxColor();

	// Generated message map functions
protected:	
	bool				  _CheckBoxEnabled;
	bool				  _IconsEnabled;
	std::vector<COLORREF> _Colors;	
	uint				  _IconSize;	
	::CBitmap			  _PackedIcons;
	class CIcon // an icon taken from the big bitmap
	{
	public:
		sint SrcX;
		sint SrcY;
		CIcon() : SrcX(-1), SrcY(-1) {}
	};
	std::vector<CIcon>	  _Icons;
	bool				  _IconsBitmapLoaded;




	//{{AFX_MSG(CListBoxColor)
	afx_msg void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	//}}AFX_MSG

	virtual CRect OnGetCheckPosition( CRect rectItem, CRect rectCheckBox );
	BOOL OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LIST_BOX_COLOR_H__C4882C3B_0E50_4BC6_A51A_EAA1B4C2E479__INCLUDED_)
