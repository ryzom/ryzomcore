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

#ifndef __FILE_TREE_VIEW_H
#define __FILE_TREE_VIEW_H

#include <string>

/**
  *  Send to notify window (registered with setNotifyWindow() ).
  *  wParam is TRUE if the item is a directory
  */
#define WM_FILE_TREE_VIEW_LDBLCLICK (WM_APP+0x40)

class CFileTreeCtrl : public CWnd
{
public:
	friend int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFileTreeCtrl)
	protected:
	virtual int		OnCreate  ( LPCREATESTRUCT lpCreateStruct );
	virtual BOOL	OnNotify ( WPARAM wParam, LPARAM lParam, LRESULT* pResult );
	//}}AFX_VIRTUAL

public:
	CFileTreeCtrl ();
	
	enum TArrange
	{
		None = 0,
		ByName,
		ByType
	};

	bool create( const RECT& rect, CWnd* pParentWnd, UINT nID );
	bool setRootDirectory (const char *dir);
	void setArrangeMode (TArrange arrangeMode);
	bool enumObjects (HTREEITEM hParentItem,IShellFolder* pParentFolder, ITEMIDLIST* pidlParent);
	void doItemMenu (HWND hwndTreeView, HTREEITEM hItem, LPPOINT pptScreen);
	void doClick (HWND hwndTreeView, HTREEITEM hItem);
	void addExclusiveExtFilter (const char *ext);
	void addNegativeExtFilter (const char *ext);
	void clearExtFilters ();
	void setNotifyWindow (HWND hWnd, uint32 param);
	bool getCurrentFilename (std::string &result);
	bool haveFocus ();

	virtual ~CFileTreeCtrl();

	// Overrided
	BOOL	ShowWindow ( int nCmdShow );

private:
	HWND			_HWndNotify;
	uint32			_NotifyParam;
	CTreeCtrl		_TreeCtrl;
	std::string		_RootDirectory;
	HIMAGELIST		_ImageListSmall;
	TArrange		_ArrangeMode;

	// Filter
	std::vector<std::string>	_ExclusiveExtFilter;
	std::vector<std::string>	_NegativeExtFilter;

	// Generated message map functions
	//{{AFX_MSG(CFileTreeCtrl)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // __FILE_TREE_VIEW_H
