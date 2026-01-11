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

#include <vector>
#include <string>

#if !defined(AFX_VIEWDIALOG_H__FDD49815_5955_4204_8D1C_2839AE39DDB3__INCLUDED_)
#define AFX_VIEWDIALOG_H__FDD49815_5955_4204_8D1C_2839AE39DDB3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ViewDialog.h : header file
//


class CViewDialog;

/*
 *
 */
class CListCtrlEx : public CListCtrl
{
public:
	void	initIt();
	void	DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	void	RepaintSelectedItems();
	//void	OnKillFocus(CWnd* pNewWnd);
	//void	OnSetFocus(CWnd* pOldWnd);
	
	void	setViewDialog( CViewDialog *pt ) { _ViewDialog = pt; }

protected:

	//{{AFX_MSG(CListCtrlEx)
	afx_msg void OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags );
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	CViewDialog	*_ViewDialog;
};

/////////////////////////////////////////////////////////////////////////////
// CViewDialog dialog

class CViewDialog : public CDialog
{
// Construction
public:
	CViewDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CViewDialog)
	enum { IDD = IDD_View };
	CListCtrlEx	m_ListCtrl;
	CString	m_Caption;
	//}}AFX_DATA

	///	 Load, using the current filters
	void		reload();

	/// Load trace
	void		reloadTrace();

	/// Set the filters (and backup the previous ones for bookmark translation)
	void		setFilters( const std::vector<CString>& posFilter, const std::vector<CString>& negFilter );
	
	/// Returns true if the string must be logged, according to the current filters
	bool		passFilter( const char *text, const std::vector<CString>& posFilter, const std::vector<CString>& negFilter ) const;

	/// Resize
	void		resizeView( int nbViews, int top, int left );

	/// Clear
	void		clear();

	/// Return the nb of lines
	int			getNbLines() const;

	/// Return the nb of visible lines
	int			getNbVisibleLines() const;

	/// Set redraw state
	void		setRedraw( bool redraw ) { m_ListCtrl.SetRedraw( redraw ); }

	/// Fill from getNbLines() to maxNbLines with blank lines
	void		fillGaps( int maxNbLines );

	/// Load a log file or series
	void		loadFileOrSeries( const std::vector<int>& bookmarksAbsoluteLines );

	/// Add one line
	void		addLine( const CString& line ) { Buffer.push_back( line ); }

	/// Add several lines
	void		addText( const CString& lines );

	/// Commit the lines previously added
	void		commitAddedLines();

	/// Scroll
	void		scrollTo( int index );

	/// Select
	void		select( int index );

	/// Get selected index
	int			getSelectionIndex() { return m_ListCtrl.GetSelectionMark(); }
	
	/// Return the index of the top of the listbox
	int			getScrollIndex() const;

	/// Add the current scroll index to the bookmark list, or delete it if already inside the list
	void		addBookmark();

	/// Scroll the listbox to the next found stored bookmkark (downwards from the current scroll index)
	void		recallNextBookmark();

	/// Display string
	void		displayString();

	/// Return the textcolor
	COLORREF	getTextColorForLine( int index, bool selected );

	/// Return the background color
	COLORREF	getBkColorForLine( int index, bool selected );

	///
	std::string	corruptedLinesString( const std::vector<unsigned int>& corruptedLines );

	/// Reload with old filter to get bookmarks absolute line numbers (not called if there's no bookmark)
	void		getBookmarksAbsoluteLines( std::vector<int>& bookmarksAbsoluteLines );

	//{{AFX_MSG(CViewDialog)
	afx_msg void OnButtonFind();
	//}}AFX_MSG

	int						Index;
	CString					Seriesname;
	std::vector<CString>	Filenames;
	std::vector<CString>	PosFilter, NegFilter, PreviousPosFilter, PreviousNegFilter;
	CString					LogSessionStartDate;
	bool					SessionDatePassed;
	std::vector<CString>	Buffer;
	int						BeginFindIndex;
	CFindReplaceDialog		*FindDialog;
	CString					FindStr;
	bool					FindMatchCase, FindDownwards;
	float					WidthR; // ratio to the app's client window
	std::vector<int>		Bookmarks;
	int						CurrentBookmark;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CViewDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	afx_msg LRESULT OnFindReplace(WPARAM wParam, LPARAM lParam);

	// Generated message map functions
	//{{AFX_MSG(CViewDialog)
	afx_msg void OnButtonFilter();
	virtual BOOL OnInitDialog();
	afx_msg void OnGetdispinfoList1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemchangedList1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSetfocusList1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VIEWDIALOG_H__FDD49815_5955_4204_8D1C_2839AE39DDB3__INCLUDED_)
