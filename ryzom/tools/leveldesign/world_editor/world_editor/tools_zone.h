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

#ifndef TOOLSZONE_H
#define TOOLSZONE_H

// ***************************************************************************

#include <afxcview.h>
#include "resource.h"

#include <string>
#include <vector>

// ***************************************************************************

class CMainFrame;
class CToolsZone;

// ***************************************************************************

class CToolsZoneList : public CListBox
{
	bool						_MouseLDown;
	CToolsZone					*_Tools;
	std::vector<std::string>	_ItemNames;
	std::vector<CBitmap*>		_BitmapList;

public:

	CToolsZoneList ();

	void setTool (CToolsZone *pTool);
	void setImages (std::vector<CBitmap*> &vBitmaps);
	void addItem (const std::string &itemName);
	const std::string &getItem (uint32 nIndex);
	void reset ();

	void DrawItem (LPDRAWITEMSTRUCT);
	void MeasureItem (LPMEASUREITEMSTRUCT);
	int	 CompareItem (LPCOMPAREITEMSTRUCT);

	// For some obscur reason on subclassing windows system we cant use the message map
	// mechanism so we have to notify the parent by hand

	void notifyParent();

	afx_msg void OnLButtonDown	(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp	(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove	(UINT nFlags, CPoint point);
	

	DECLARE_MESSAGE_MAP()

};

// ***************************************************************************

class CToolsZone : public CFormView
{
	DECLARE_DYNCREATE(CToolsZone)

	CToolsZoneList	_List;
	bool			_ListCreated;

	CMainFrame		*_MainFrame;
	
private:

	void addToAllCatTypeCB (const std::string &Name);
	void updateComboPairAndFilter (int CatTypeId, int CatValueId, std::string *pFilterType);

public:
	
	CToolsZone();

	CToolsZoneList *getListCtrl();
	
	void init (CMainFrame *pMF);
	void uninit ();

	// Event handlers
	afx_msg int OnCreate (LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize (UINT nType, int cx, int cy);
	afx_msg void OnPaint ();

	afx_msg void OnSelectCatType1 ();
	afx_msg void OnSelectCatType2 ();
	afx_msg void OnSelectCatType3 ();
	afx_msg void OnSelectCatType4 ();
	afx_msg void OnSelectCatValue1 ();
	afx_msg void OnSelectCatValue2 ();
	afx_msg void OnSelectCatValue3 ();
	afx_msg void OnSelectCatValue4 ();
	afx_msg void OnSelectAnd2 ();
	afx_msg void OnSelectOr2 ();
	afx_msg void OnSelectAnd3 ();
	afx_msg void OnSelectOr3 ();
	afx_msg void OnSelectAnd4 ();
	afx_msg void OnSelectOr4 ();
	afx_msg void OnSelectRandom ();
	afx_msg void OnSelectCycle ();
	afx_msg void OnSelectNotPropagate ();
	afx_msg void OnSelectForce ();

	afx_msg void OnSelectRot0 ();
	afx_msg void OnSelectRot90 ();
	afx_msg void OnSelectRot180 ();
	afx_msg void OnSelectRot270 ();
	afx_msg void OnSelectRotRan ();
	afx_msg void OnSelectRotCycle ();
	afx_msg void OnSelectFlipNo ();
	afx_msg void OnSelectFlipYes ();
	afx_msg void OnSelectFlipRan ();
	afx_msg void OnSelectFlipCycle ();
	
	void OnSelChange (); // Notified by hand

	DECLARE_MESSAGE_MAP()
};

// ***************************************************************************

#endif // TOOLSZONE_H
