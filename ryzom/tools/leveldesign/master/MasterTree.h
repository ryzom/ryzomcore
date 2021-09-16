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

#ifndef __MASTERTREE_H__
#define __MASTERTREE_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MasterTree.h : header file
//

#include <string>
#include <map>

/////////////////////////////////////////////////////////////////////////////

#define MT_SORT_BY_NAME_INC	0
#define MT_SORT_BY_NAME_DEC 1
#define MT_SORT_BY_DATE_INC 2
#define MT_SORT_BY_DATE_DEC 3

/////////////////////////////////////////////////////////////////////////////
// CMasterTree Control
/////////////////////////////////////////////////////////////////////////////

class CMasterTree : public CTreeCtrl
{

	bool _LDrag;
	CImageList *_DragImg;
	HTREEITEM _DragItem;
	HTREEITEM _LastItemSelected;
	HTREEITEM _LastActiveContinent;

	UINT    m_nTimerID;
	UINT    m_timerticks;
	
public:

	CMasterTree ();

	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnLBeginDrag (NMHDR* pNMHDR, LRESULT* pResult);

	afx_msg void OnMouseMove (UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown (UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp (UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk (UINT nFlags, CPoint point);

	afx_msg void OnRButtonDown (UINT nFlags, CPoint point);


	// Continent menus (a level under the root)

	afx_msg void OnMenuNewContinent ();
	
	afx_msg void OnMenuContinentOpen ();
	afx_msg void OnMenuContinentProperties ();
	afx_msg void OnMenuContinentNewRegion ();
	afx_msg void OnMenuContinentDelete ();

	// Regions menus (a level under the continent)
	
	afx_msg void OnMenuRegionOpen ();
	afx_msg void OnMenuRegionRename ();
	afx_msg void OnMenuRegionNewSubRegion ();
	afx_msg void OnMenuRegionNewPrim ();
	afx_msg void OnMenuRegionNewGeorges ();
	afx_msg void OnMenuRegionDelete ();

	// File menus (a level under the Region)

	afx_msg void OnMenuFileOpen ();
	afx_msg void OnMenuFileRename ();
	afx_msg void OnMenuFileDelete ();

	std::string getCurrentPath ();

	DECLARE_MESSAGE_MAP()

private:

	// TOOLS
	std::string getPath (HTREEITEM ht);
	HTREEITEM getContinent (HTREEITEM ht);
	HTREEITEM getRegion (HTREEITEM ht);
	bool isFile (HTREEITEM ht);

};

/////////////////////////////////////////////////////////////////////////////
// CMasterTreeDlg Dialog
/////////////////////////////////////////////////////////////////////////////

class CMasterTreeDlg : public CDialog
{
	CMasterTree						*_Tree;
	std::map<HTREEITEM,std::string>	_Files;

	char ContinentSortBy;	// 0/1 - Sort by name increasing(z-a)/decreasing(a-z)
	char TrashSortBy;		// 2/3 - Sort by date increasing(new-old)/decreasing(old-new)
	char BackupSortBy;

private:

	void parseAdd (HTREEITEM itRoot, const std::string &path, char nSortType, int DirDepth = 0);

public:
	
	CMasterTreeDlg();   // standard constructor
	
	HTREEITEM activate (const std::string& EltName, HTREEITEM parent);
	void update (const std::string& ContinentsPath);
	
	void openFile (const std::string &fname);
	void openDir (const std::string &fname);
	
	void continentNew ();
	void sortContinentBy (int mt_sort_type);

	// Continent byPass

	void continentDelete (const std::string &Continent);
	void continentProperties (const std::string &Continent);
	void continentNewRegion (const std::string &Continent);
	void continentOpen (const std::string &Continent);

	// Region byPass

	void regionOpen (const std::string &path);
	void regionRename (const std::string &path);
	void regionNewPrim (const std::string &path);
	void regionNewGeorges (const std::string &path);
	void regionNewSubRegion (const std::string &path);
	void regionDelete (const std::string &path);

	// File byPass

	void fileOpen (const std::string &sFileFullName);
	void fileRename (const std::string &sFileFullName);
	void fileDelete (const std::string &sFileFullName);

	void copy (const std::string &pathSrc, const std::string &pathDst);

	enum { IDD = IDD_MASTERTREE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support


// Implementation
protected:

	BOOL OnInitDialog ();
	void OnCancel ();
	void OnOK ();
	
	afx_msg void OnSize (UINT nType, int cx, int cy);
	afx_msg void OnExpanding (LPNMHDR pnmhdr, LRESULT *pLResult);
	afx_msg void OnLDblClkTree(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()

	// Friends
	
	friend class CMasterTree;
};

#endif // __MASTERTREE_H__
