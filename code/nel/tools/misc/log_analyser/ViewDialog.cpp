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


// ViewDialog.cpp : implementation file
//

#include "stdafx.h"
#include "log_analyser.h"
#include "ViewDialog.h"
#include "log_analyserDlg.h"

#include <fstream>
#include <algorithm>
using namespace std;


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


extern CString						LogDateString;
static UINT WM_FINDREPLACE = ::RegisterWindowMessage(FINDMSGSTRING);
time_t CurrentTime;


void CListCtrlEx::initIt()
{
	DWORD dwStyle = GetWindowLong(m_hWnd, GWL_STYLE); 
	SetWindowLong( m_hWnd, GWL_STYLE, dwStyle | LVS_OWNERDRAWFIXED );
}


/*
 * Keyboard handler in list box
 */
afx_msg void CListCtrlEx::OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags )
{
	// Transmit to List Ctrl AND to main window
	CListCtrl::OnKeyDown( nChar, nRepCnt, nFlags );
	((CLog_analyserDlg*)(_ViewDialog->GetParent()))->OnKeyDown( nChar, nRepCnt, nFlags );
}


// Adapted from http://zeus.eed.usv.ro/misc/doc/prog/c/mfc/listview/sel_row.html
void CListCtrlEx::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	CRect rcItem(lpDrawItemStruct->rcItem);
	int nItem = lpDrawItemStruct->itemID;
	CImageList* pImageList;

	// Save dc state
	int nSavedDC = pDC->SaveDC();

	// Get item image and state info
	LV_ITEM lvi;
	lvi.mask = LVIF_IMAGE | LVIF_STATE;
	lvi.iItem = nItem;
	lvi.iSubItem = 0;
	lvi.stateMask = 0xFFFF;		// get all state flags
	GetItem(&lvi);

	BOOL bHighlight =((lvi.state & LVIS_DROPHILITED)
					|| ( (lvi.state & LVIS_SELECTED)
						&& ((GetFocus() == this)
							|| (GetStyle() & LVS_SHOWSELALWAYS)
							)
						)
					);

	// Get rectangles for drawing
	CRect rcBounds, rcLabel, rcIcon;
	GetItemRect(nItem, rcBounds, LVIR_BOUNDS);
	GetItemRect(nItem, rcLabel, LVIR_LABEL);
	GetItemRect(nItem, rcIcon, LVIR_ICON);
	CRect rcCol( rcBounds ); 


	CString sLabel = GetItemText( nItem, 0 );

	// Labels are offset by a certain amount  
	// This offset is related to the width of a space character
	int offset = pDC->GetTextExtent(_T(" "), 1 ).cx*2;

	CRect rcHighlight;
	CRect rcWnd;
	GetClientRect(&rcWnd);
	rcHighlight = rcBounds;
	rcHighlight.left = rcLabel.left;
	rcHighlight.right = rcWnd.right;

	// Draw the background color
	if( bHighlight )
		pDC->SetBkColor(::GetSysColor(COLOR_HIGHLIGHT));
	pDC->FillRect( rcHighlight, &CBrush(_ViewDialog->getBkColorForLine( nItem, bHighlight!=0 )) );

	// Set clip region
	rcCol.right = rcCol.left + GetColumnWidth(0);
	CRgn rgn;
	rgn.CreateRectRgnIndirect(&rcCol);
	pDC->SelectClipRgn(&rgn);
	rgn.DeleteObject();

	// Draw state icon
	if (lvi.state & LVIS_STATEIMAGEMASK)
	{
		int nImage = ((lvi.state & LVIS_STATEIMAGEMASK)>>12) - 1;
		pImageList = GetImageList(LVSIL_STATE);
		if (pImageList)
		{
			pImageList->Draw(pDC, nImage,
				CPoint(rcCol.left, rcCol.top), ILD_TRANSPARENT);
		}
	}
	
	// Draw normal and overlay icon
	pImageList = GetImageList(LVSIL_SMALL);
	if (pImageList)
	{
		UINT nOvlImageMask=lvi.state & LVIS_OVERLAYMASK;
		pImageList->Draw(pDC, lvi.iImage, 
			CPoint(rcIcon.left, rcIcon.top),
			ILD_TRANSPARENT | nOvlImageMask );
	}

	
	
	// Draw item label - Column 0
	rcLabel.left += offset/2;
	rcLabel.right -= offset;

	pDC->SetTextColor( _ViewDialog->getTextColorForLine( nItem, bHighlight!=0 ) );
	pDC->DrawText(sLabel,-1,rcLabel,DT_LEFT | DT_SINGLELINE | DT_NOPREFIX | DT_NOCLIP 
				| DT_VCENTER /*| DT_END_ELLIPSIS*/);


	// Draw labels for remaining columns
	LV_COLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_WIDTH;

	rcBounds.right = rcHighlight.right > rcBounds.right ? rcHighlight.right :
							rcBounds.right;
	rgn.CreateRectRgnIndirect(&rcBounds);
	pDC->SelectClipRgn(&rgn);
				   
	for(int nColumn = 1; GetColumn(nColumn, &lvc); nColumn++)
	{
		rcCol.left = rcCol.right;
		rcCol.right += lvc.cx;

		sLabel = GetItemText(nItem, nColumn);
		if (sLabel.GetLength() == 0)
			continue;

		// Get the text justification
		UINT nJustify = DT_LEFT;
		switch(lvc.fmt & LVCFMT_JUSTIFYMASK)
		{
		case LVCFMT_RIGHT:
			nJustify = DT_RIGHT;
			break;
		case LVCFMT_CENTER:
			nJustify = DT_CENTER;
			break;
		default:
			break;
		}

		rcLabel = rcCol;
		rcLabel.left += offset;
		rcLabel.right -= offset;

		pDC->DrawText(sLabel, -1, rcLabel, nJustify | DT_SINGLELINE | 
					DT_NOPREFIX | DT_VCENTER | DT_END_ELLIPSIS);
	}

	// Draw focus rectangle if item has focus
	if (lvi.state & LVIS_FOCUSED && (GetFocus() == this))
	{
		pDC->DrawFocusRect(rcHighlight);
	}

	
	// Restore dc
	pDC->RestoreDC( nSavedDC );
}


void CListCtrlEx::RepaintSelectedItems()
{
	CRect rcBounds, rcLabel;

	// Invalidate focused item so it can repaint 
	int nItem = GetNextItem(-1, LVNI_FOCUSED);

	if(nItem != -1)
	{
		GetItemRect(nItem, rcBounds, LVIR_BOUNDS);
		GetItemRect(nItem, rcLabel, LVIR_LABEL);
		rcBounds.left = rcLabel.left;

		InvalidateRect(rcBounds, FALSE);
	}

	// Invalidate selected items depending on LVS_SHOWSELALWAYS
	if(!(GetStyle() & LVS_SHOWSELALWAYS))
	{
		for(nItem = GetNextItem(-1, LVNI_SELECTED);
			nItem != -1; nItem = GetNextItem(nItem, LVNI_SELECTED))
		{
			GetItemRect(nItem, rcBounds, LVIR_BOUNDS);
			GetItemRect(nItem, rcLabel, LVIR_LABEL);
			rcBounds.left = rcLabel.left;

			InvalidateRect(rcBounds, FALSE);
		}
	}

	UpdateWindow();
}


/*void CListCtrlEx::OnKillFocus(CWnd* pNewWnd) 
{
	CListCtrl::OnKillFocus(pNewWnd);

	// check if we are losing focus to label edit box
	if(pNewWnd != NULL && pNewWnd->GetParent() == this)
		return;

	// repaint items that should change appearance
	if((GetStyle() & LVS_TYPEMASK) == LVS_REPORT)
		RepaintSelectedItems();
}

void CListCtrlEx::OnSetFocus(CWnd* pOldWnd) 
{
	CListCtrl::OnSetFocus(pOldWnd);
	
	// check if we are getting focus from label edit box
	if(pOldWnd!=NULL && pOldWnd->GetParent()==this)
		return;

	// repaint items that should change appearance
	if((GetStyle() & LVS_TYPEMASK)==LVS_REPORT)
		RepaintSelectedItems();
}*/


BEGIN_MESSAGE_MAP(CListCtrlEx, CListCtrl)
	//{{AFX_MSG_MAP(CListCtrlEx)
	ON_WM_KEYDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CViewDialog dialog


CViewDialog::CViewDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CViewDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CViewDialog)
	m_Caption = _T("");
	//}}AFX_DATA_INIT
}


void CViewDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CViewDialog)
	DDX_Control(pDX, IDC_LIST1, m_ListCtrl);
	DDX_Text(pDX, IDC_Service, m_Caption);
	//}}AFX_DATA_MAP
}


/*
 * Load, using the current filters
 */
void		CViewDialog::reload()
{
	SessionDatePassed = false;
	CWaitCursor wc;
	if ( LogSessionStartDate.IsEmpty() || (LogSessionStartDate == _T("Beginning")) )
	{
		SessionDatePassed = true;
	}

	((CButton*)GetDlgItem( IDC_BUTTON1 ))->ShowWindow( SW_SHOW );
	((CButton*)GetDlgItem( IDC_BUTTON2 ))->ShowWindow( SW_SHOW );
	m_Caption.Format( _T("%s (%u file%s) %u+ %u- (%s)"), Seriesname, Filenames.size(), Filenames.size()>1?"s":"", PosFilter.size(), NegFilter.size(), LogSessionStartDate.IsEmpty()?"all":CString("session ")+LogSessionStartDate );
	UpdateData( false );
	clear();
	setRedraw( false );

	time( &CurrentTime );

	// Translate bookmarks (phase 1)
	CurrentBookmark = -1;
	vector<int> bookmarksAbsoluteLines;
	if ( ! Bookmarks.empty() )
	{
		getBookmarksAbsoluteLines( bookmarksAbsoluteLines );
		Bookmarks.clear();
	}

	loadFileOrSeries( bookmarksAbsoluteLines );
	commitAddedLines();

	setRedraw( true );
}


/*
 * Code from IDisplayer::dateToHumanString() (NeL misc)
 */
string timeToStr( const time_t& date )
{
	static char cstime[25];
	struct tm *tms = localtime(&date);
	if (tms)
		strftime (cstime, 25, "%Y/%m/%d %H:%M:%S", tms);
	else
		smprintf(cstime, 25, "bad date %d", (unsigned int)date);
	return cstime;
}


/*
 * Reload with old filter to get bookmarks absolute line numbers (not called if there's no bookmark)
 */
void		CViewDialog::getBookmarksAbsoluteLines( vector<int>& bookmarksAbsoluteLines )
{
	// Read the files
	unsigned int currentAbsoluteLineNum = 0, currentLineNum = 0;
	for ( unsigned int i=0; i!=Filenames.size(); ++i )
	{
		CString& filename = Filenames[i];

		NLMISC::CIFile ifs;

		if (ifs.open(NLMISC::tStrToUtf8(filename)))
		{
			char line [1024];

			while (!ifs.eof())
			{
				ifs.getline(line, 1024);

				if ( SessionDatePassed )
				{
					// Stop if the session is finished
					if ( (! LogSessionStartDate.IsEmpty()) && (strstr( line, nlTStrToUtf8(LogDateString))) )
					{
						return;
					}

					// Test the filters
					if ( passFilter( line, PreviousPosFilter, PreviousNegFilter ) )
					{
						// Translate bookmarks (phase 1: filtered -> absolute)
						vector<int>::iterator ibl;
						ibl = find( Bookmarks.begin(), Bookmarks.end(), currentLineNum );
						if ( ibl != Bookmarks.end() )
						{
							bookmarksAbsoluteLines.push_back( currentAbsoluteLineNum );
						}

						++currentLineNum;
					}

					++currentAbsoluteLineNum;
				}
				else
				{
					// Look for the session beginning
					if ( strstr( line, nlTStrToUtf8(LogSessionStartDate)) != NULL )
					{
						SessionDatePassed = true;
					}
				}
			}
		}
	}
}


/*
 * Auto-detect file corruption (version for any NeL log file)
 */
bool		detectLineCorruption( const char *cLine )
{
	string line = string(cLine);
	if ( ! line.empty() )
	{
		bool corrupted = false;
		string::size_type p;

		// Some line may begin with no date, this is normal, we don't state them as corruption

		// Search for year not at beginning. Ex: "2003/" (it does not work when the year is different from the current one!)
		p = line.substr( 1 ).find( timeToStr( CurrentTime ).substr( 0, 5 ) );
		if ( p != string::npos )
		{
			++p; // because searched from pos 1

			// Search for date/time
			if ( (line.size()>p+20) && (line[p+10]==' ') && (line[p+13]==':')
				 && (line[p+16]==':') && (line[p+19]==' ') )
			{
				// Search for the five next blank characters. The fifth is followed by ": ".
				// (Date Time LogType ThreadId Machine/Service SourceFile Line : User-defined log line)
				unsigned int nbBlank = 0;
				string::size_type sp;
				for ( sp=p+20; sp!=line.size(); ++sp )
				{
					if ( line[sp]==' ')
						++nbBlank;
					if ( nbBlank==5 )
						break;
				}
				if ( (nbBlank==5) && (line[sp+1]==':') && (line[sp+2]==' ') )
				{
					return true;
				}
			}
		}
	}
	return false;
}


bool HasCorruptedLines;


/*
 *
 */
std::string		CViewDialog::corruptedLinesString( const std::vector<unsigned int>& corruptedLines )
{
	string res;
	if ( ! corruptedLines.empty() )
	{
		res = NLMISC::toString(" -> %u corrupted lines:", (uint)corruptedLines.size());

		vector<unsigned int>::const_iterator ivc;
		for ( ivc=corruptedLines.begin(); ivc!=corruptedLines.end(); ++ivc )
		{
			res += NLMISC::toString("\r\n   line %u : %s...", *ivc, NLMISC::tStrToUtf8(Buffer[*ivc].Left(20)).c_str());
		}
		HasCorruptedLines = true;
	}
	return res;
}


/*
 * Load a log file or series
 */
void		CViewDialog::loadFileOrSeries( const vector<int>& bookmarksAbsoluteLines )
{
	// Header for 'files loaded' display
	string actualFilenames = "Files loaded";
	if ( LogSessionStartDate.IsEmpty() )
		actualFilenames += ":\r\n";
	else
		actualFilenames += " for Session of " + NLMISC::tStrToUtf8(LogSessionStartDate) + ":\r\n";
	bool corruptionDetectionEnabled = (((CButton*)(((CLog_analyserDlg*)GetParent())->GetDlgItem( IDC_DetectCorruptedLines )))->GetCheck() == 1);
	HasCorruptedLines = false;
	vector<unsigned int> corruptedLines;

	// Read the files
	unsigned int currentAbsoluteLineNum = 0, currentLineNum = 0;
	for ( unsigned int i=0; i!=Filenames.size(); ++i )
	{
		CString& filename = Filenames[i];
		ifstream ifs( filename );
		if ( ! ifs.fail() )
		{
			char line [1024];
			while ( ! ifs.eof() )
			{
				ifs.getline( line, 1024 );
				line[1023] = '\0'; // force valid end of line
				if ( SessionDatePassed )
				{
					// Stop if the session is finished
					if ( (! LogSessionStartDate.IsEmpty()) && (strstr( line, nlTStrToUtf8(LogDateString) )) )
					{
						actualFilenames += NLMISC::tStrToUtf8(filename) + corruptedLinesString(corruptedLines) + "\r\n";
						corruptedLines.clear();
						goto endOfLoading;
					}

					// Test the filters
					if ( passFilter( line, PosFilter, NegFilter ) )
					{
						// Auto-detect line corruption
						if ( corruptionDetectionEnabled && detectLineCorruption( line ) )
							corruptedLines.push_back( currentLineNum );

						// Translate bookmarks (phase 2: absolute -> filtered)
						if ( ! bookmarksAbsoluteLines.empty() )
						{
							vector<int>::const_iterator ibal;
							ibal = find( bookmarksAbsoluteLines.begin(), bookmarksAbsoluteLines.end(), currentAbsoluteLineNum );
							if ( ibal != bookmarksAbsoluteLines.end() )
							{
								Bookmarks.push_back( currentLineNum );
							}
						}

						// Add line to list box
						addLine( line );
						++currentLineNum;
					}

					++currentAbsoluteLineNum;
				}
				else
				{
					// Look for the session beginning
					if ( strstr( line, nlTStrToUtf8(LogSessionStartDate) ) != NULL )
					{
						SessionDatePassed = true;
					}
				}
			}

			if ( SessionDatePassed )
			{
				actualFilenames += NLMISC::tStrToUtf8(filename) + corruptedLinesString(corruptedLines) + "\r\n";
				corruptedLines.clear();
			}
		}
		else
		{
			CString s;
			s.Format(_T( "<Cannot open file %s>\r\n"), filename);
			actualFilenames += NLMISC::tStrToUtf8(s);
		}
	}

endOfLoading:
	if ( HasCorruptedLines )
	{
		actualFilenames += "(When corrupted lines are present, it is possible that some other lines are missing)\r\n\
Select the line number and press Ctrl-G to scroll to the corrupted line.\r\n\
At any time, press Ctrl-L in this edit box to return to this list of files and corrupted lines.";
	}
	((CLog_analyserDlg*)GetParent())->displayCurrentLine( actualFilenames.c_str() );
	((CLog_analyserDlg*)GetParent())->memorizeFileList( actualFilenames.c_str() );
}


/*
 * Set the filters (and backup the previous ones for bookmark translation)
 */
void		CViewDialog::setFilters( const std::vector<CString>& posFilter, const std::vector<CString>& negFilter )
{
	PreviousPosFilter = PosFilter;
	PreviousNegFilter = NegFilter;
	PosFilter = posFilter;
	NegFilter = negFilter;
}


/*
 * Returns true if the string must be logged, according to the current filters
 */
bool		CViewDialog::passFilter( const char *text, const std::vector<CString>& posFilter, const std::vector<CString>& negFilter ) const
{
	bool yes = posFilter.empty();

	bool found;
	vector<CString>::const_iterator ilf;

	// 1. Positive filter
	for ( ilf=posFilter.begin(); ilf!=posFilter.end(); ++ilf )
	{
		found = (strstr(text, nlTStrToUtf8(*ilf)) != NULL);
		if ( found )
		{
			yes = true; // positive filter passed (no need to check another one)
			break;
		}
		// else try the next one
	}
	if ( ! yes )
	{
		return false; // positive filter not passed
	}

	// 2. Negative filter
	for ( ilf=negFilter.begin(); ilf!=negFilter.end(); ++ilf )
	{
		found = (strstr(text, nlTStrToUtf8(*ilf)) != NULL);
		if ( found )
		{
			return false; // negative filter not passed (no need to check another one)
		}
	}
	return true; // negative filter passed
}


/*
 * Load trace
 */
void		CViewDialog::reloadTrace()
{
	CWaitCursor wc;
	((CButton*)GetDlgItem( IDC_BUTTON1 ))->ShowWindow( SW_HIDE );
	((CButton*)GetDlgItem( IDC_BUTTON2 ))->ShowWindow( SW_HIDE );
	if ( LogSessionStartDate.IsEmpty() )
	{
		SessionDatePassed = true;
		if ( PosFilter.empty() )
			m_Caption = "Trace of " + Seriesname + " (all)";
		else
			m_Caption = "Trace of " + PosFilter[0] + " (all)";
	}
	else
	{
		if ( LogSessionStartDate == "Beginning" )
		{
			SessionDatePassed = true;
		}
		if ( PosFilter.empty() )
			m_Caption = "Trace of " + Seriesname + " (session " + LogSessionStartDate + ")" ;
		else
			m_Caption = "Trace of " + PosFilter[0] + " (session " + LogSessionStartDate + ")" ;
	}

	UpdateData( false );
	clear();

	ifstream ifs( Seriesname );
	if ( ! ifs.fail() )
	{
		char line [1024];
		while ( ! ifs.eof() )
		{
			ifs.getline( line, 1024 );
			if ( SessionDatePassed )
			{
				// Stop if the session is finished
				if ((!LogSessionStartDate.IsEmpty()) && (strstr(line, nlTStrToUtf8(LogDateString))))
					break;

				// Read if it's a TRACE
				char *pc = strstr( line, "TRACE" );
				if ( pc != NULL )
				{
					std::string tposFilter0 = NLMISC::tStrToUtf8(PosFilter[0]);
					if (PosFilter.empty() || (strncmp(pc - PosFilter[0].GetLength(), tposFilter0.c_str(), tposFilter0.size()) == 0))
					{
						((CLog_analyserDlg*)GetParent())->insertTraceLine( Index, pc+6 );
					}
				}
			}
			else
			{
				// Look for the session beginning
				if (strstr(line, nlTStrToUtf8(LogSessionStartDate)) != NULL)
				{
					SessionDatePassed = true;
				}
			}
		}

		addLine( "<After adding all the views" );
		addLine( "you need, click Compute Traces" );
		addLine( "to generate all the views>" );
	}
	else
	{
		addLine( "<Cannot open log file>" );
	}

	commitAddedLines();
}


BEGIN_MESSAGE_MAP(CViewDialog, CDialog)
	//{{AFX_MSG_MAP(CViewDialog)
	ON_BN_CLICKED(IDC_BUTTON1, OnButtonFilter)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_LIST1, OnGetdispinfoList1)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST1, OnItemchangedList1)
	ON_NOTIFY(NM_SETFOCUS, IDC_LIST1, OnSetfocusList1)
	ON_BN_CLICKED(IDC_BUTTON2, OnButtonFind)
	ON_WM_LBUTTONDOWN()
	ON_REGISTERED_MESSAGE( WM_FINDREPLACE, OnFindReplace )
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CViewDialog message handlers


/*
 *
 */
void CViewDialog::OnSetfocusList1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// Force to display the current item when the current view changes
	if ( getSelectionIndex() != -1 )
		displayString();

	((CLog_analyserDlg*)GetParent())->setCurrentView( Index );

	m_ListCtrl.RepaintSelectedItems();
	*pResult = 0;
}

/*
 *
 */
void CViewDialog::OnItemchangedList1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	// Display the current item when it changes
	if ( pNMListView->iItem != -1 )
		displayString();

	((CLog_analyserDlg*)GetParent())->setCurrentView( Index );

	m_ListCtrl.RepaintSelectedItems();
	*pResult = 0;
}


/*
 * Resize
 */
void CViewDialog::resizeView( int nbViews, int top, int left )
{
	RECT parentRect;
	GetParent()->GetClientRect( &parentRect );

	int width = (int)((parentRect.right-32)*WidthR);
	RECT viewRect;
	viewRect.left = left;
	viewRect.top = top;
	viewRect.right = viewRect.left + width;
	viewRect.bottom = parentRect.bottom-10;
	MoveWindow( &viewRect, TRUE );
	
	m_ListCtrl.MoveWindow( 5, 32, width-5, viewRect.bottom-top-42 );
	LVCOLUMN lvc;
	lvc.mask = LVCF_WIDTH;
	lvc.cx = width-24;
	m_ListCtrl.SetColumn( 0, &lvc );
	//m_ListCtrl.SetColumnWidth( 0, LVSCW_AUTOSIZE ); // worse

	GetDlgItem( IDC_DragBar )->MoveWindow( 0, 0, 32, viewRect.bottom-top );
}


/*
 * Return the nb of lines
 */
int CViewDialog::getNbLines() const
{
	return (int)Buffer.size();
}


/*
 * Return the nb of visible lines
 */
int CViewDialog::getNbVisibleLines() const
{
	return m_ListCtrl.GetCountPerPage();
}


/*
 * Fill from getNbLines() to maxNbLines with blank lines
 */
void CViewDialog::fillGaps( int maxNbLines )
{
	int nbLines = getNbLines();
	for ( int i=0; i!=maxNbLines-nbLines; ++i )
	{
		addLine( "" );
	}
}


/*
 * Commit the lines previously added
 */
void CViewDialog::commitAddedLines()
{
	m_ListCtrl.SetItemCount( (int)Buffer.size() );
	m_ListCtrl.SetColumnWidth( 0, LVSCW_AUTOSIZE );
}


/*
 * Scroll
 */
void CViewDialog::scrollTo( int index )
{
	int deltaIndex = index - m_ListCtrl.GetTopIndex();
	RECT rect;
	if ( m_ListCtrl.GetItemRect( 0, &rect, LVIR_BOUNDS ) )
	{
		int itemH = rect.bottom-rect.top;
		m_ListCtrl.Scroll( CSize( 0, deltaIndex*itemH ) );
	}
	
	//m_ListCtrl.EnsureVisible( index, false );
}


/*
 * Select
 */
void CViewDialog::select( int index )
{
	LVITEM itstate;
	itstate.mask = LVIF_STATE;
	itstate.state = 0;
	int sm = getSelectionIndex();
	if ( sm != -1 )
	{
		m_ListCtrl.SetItemState( sm, &itstate );
	}

	if ( index != -1 )
	{
		itstate.state = LVIS_SELECTED | /*LVIS_DROPHILITED |*/ LVIS_FOCUSED;
		m_ListCtrl.SetItemState( index, &itstate );
		m_ListCtrl.SetSelectionMark( index );
	}
}


/*
 * Return the index of the top of the listbox
 */
int CViewDialog::getScrollIndex() const
{
	return m_ListCtrl.GetTopIndex();
}


/*
 * Add the current scroll index to the bookmark list, or delete it if already inside the list
 */
void CViewDialog::addBookmark()
{
	int bkIndex = getSelectionIndex();
	vector<int>::iterator ibk = find( Bookmarks.begin(), Bookmarks.end(), bkIndex );
	if ( ibk == Bookmarks.end() )
	{
		// Add
		Bookmarks.push_back( bkIndex );
		std::sort( Bookmarks.begin(), Bookmarks.end() ); // not very fast but not many items
		((CLog_analyserDlg*)(GetParent()))->displayCurrentLine( "Bookmark set" );
	}
	else
	{
		// Remove
		Bookmarks.erase( ibk );
	}

	// Refresh the listbox view to display the right bookmark color
	((CLog_analyserDlg*)(GetParent()))->SetFocus();
	m_ListCtrl.SetFocus();
}


/*
 * Scroll the listbox to the next stored bookmkark
 */
void CViewDialog::recallNextBookmark()
{
	if ( Bookmarks.empty() )
		return;

	// Precondition: the vector is sorted
	int origIndex = (CurrentBookmark==-1) ? getScrollIndex() : CurrentBookmark, destIndex;
	unsigned int i = 0;
	while ( (i < Bookmarks.size()) && (Bookmarks[i] <= origIndex) )
		++i;
	if ( i == Bookmarks.size() )
	{
		// Origin index > all the bookmarks => go back to the first one
		destIndex = Bookmarks[0];
	}
	else
	{
		// Go to the next bookmark
		destIndex = Bookmarks[i];
	}

	CurrentBookmark = destIndex; // because scrollTo does not scroll if we are at the bottom of the list
	scrollTo( destIndex );
	//select( destIndex );
}


/*
 * Add several lines
 */
void CViewDialog::addText( const CString& lines )
{
	int pos, lineStartPos=0;
	for ( pos=0; pos<lines.GetLength(); ++pos )
	{
		if ( lines[pos] == '\n' )
		{
			addLine( lines.Mid( lineStartPos, pos-lineStartPos ) );
			++pos; // skip '\n'
			lineStartPos = pos;
		}
	}
	if ( lineStartPos > pos )
		addLine( lines.Mid( lineStartPos, pos-lineStartPos ) );
}


/*
 * Clear
 */
void CViewDialog::clear()
{
	Buffer.clear();
	m_ListCtrl.DeleteAllItems();
}


/*
 *
 */
void CViewDialog::OnButtonFilter() 
{
	if ( ((CLog_analyserDlg*)GetParent())->FilterDialog.DoModal() == IDOK )
	{
		setFilters( ((CLog_analyserDlg*)GetParent())->FilterDialog.getPosFilter(), ((CLog_analyserDlg*)GetParent())->FilterDialog.getNegFilter() );

		if ( ! ((CLog_analyserDlg*)GetParent())->Trace )
		{
			reload();
		}
	}
}

BOOL CViewDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_ListCtrl.GetHeaderCtrl()->ModifyStyle( 0, HDS_HIDDEN );
	m_ListCtrl.InsertColumn(0, _T(""));
	m_ListCtrl.setViewDialog( this );
	m_ListCtrl.initIt();

	Index = -1;
	BeginFindIndex = -1;
	FindDialog = NULL;
	FindMatchCase = false;
	FindDownwards = true;
	WidthR = 0.0f;
	CurrentBookmark = -1;
	return TRUE;
}


/*
 * Return the text color
 */
COLORREF CViewDialog::getTextColorForLine( int index, bool selected )
{
	if ( selected )
		return ::GetSysColor(COLOR_HIGHLIGHTTEXT);
	else
	{
		if ( Buffer[index].Find( _T("DBG") ) != -1 )
			return RGB(0x80,0x80,0x80);
		else if ( Buffer[index].Find( _T("WRN") ) != -1 )
			return RGB(0x80,0,0);
		else if ( (Buffer[index].Find( _T("ERR") ) != -1) || (Buffer[index].Find( _T("AST") ) != -1) )
			return RGB(0xFF,0,0);
		else // INF and others
			return RGB(0,0,0);
	}
}


/*
 * Return the background color
 */
COLORREF CViewDialog::getBkColorForLine( int index, bool selected )
{
	unsigned int i = 0;
	while ( (i < Bookmarks.size()) && (Bookmarks[i]<index) )
		++i;
	if ( (i < Bookmarks.size()) && (index == Bookmarks[i]) )
		if ( selected )
			return (::GetSysColor(COLOR_HIGHLIGHT) + RGB(0xC0,0x90,0x90)) / 2; // selected bookmark
		else
			return RGB(0xC0,0x90,0x90); // bookmark
	else
		if ( selected )
			return ::GetSysColor(COLOR_HIGHLIGHT); // selected
		else
			return GetSysColor(COLOR_WINDOW); // normal
}


/*
 *
 */
void formatLogStr( CString& str, bool displayHeaders )
{
	if ( ! displayHeaders )
	{
		int pos = str.Find( _T(" : ") );
		if ( pos != -1 )
		{
			str.Delete( 0, pos + 3 );
		}
	}
}


/*
 * Process string before displaying it in the view
 */
void CViewDialog::OnGetdispinfoList1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	LV_ITEM* pItem= &(pDispInfo)->item;

	int iItemIndx = pItem->iItem;
	if (pItem->mask & LVIF_TEXT) //valid text buffer?
	{
		CString str = Buffer[iItemIndx];
		formatLogStr( str, ((CButton*)(((CLog_analyserDlg*)GetParent())->GetDlgItem( IDC_DispLineHeaders )))->GetCheck() == 1 );
		lstrcpy( pItem->pszText, str );
	}
	*pResult = 0;
}


/*
 * Display string
 */
void CViewDialog::displayString()
{
	// Build the string
	CString s;
	POSITION pos = m_ListCtrl.GetFirstSelectedItemPosition();
	while ( pos != NULL )
	{
		int index = m_ListCtrl.GetNextSelectedItem( pos );
		CString str = Buffer[index];
		formatLogStr( str, ((CButton*)(((CLog_analyserDlg*)GetParent())->GetDlgItem( IDC_DispLineHeaders )))->GetCheck() == 1 );
		s += str + "\r\n";
	}

	// Display it
	((CLog_analyserDlg*)GetParent())->displayCurrentLine( s );
}


/*
 * Search string
 */
void CViewDialog::OnButtonFind() 
{
	if ( FindDialog ) // spawn only 1 window
		return;

	m_ListCtrl.ModifyStyle( 0, LVS_SHOWSELALWAYS );
	select( -1 );
	DWORD frDown = FindDownwards ? FR_DOWN : 0;
	DWORD frMatchCase = FindMatchCase ? FR_MATCHCASE : 0;
	FindDialog = new CFindReplaceDialog();
	FindDialog->Create( true, FindStr, NULL, frDown | frMatchCase | FR_HIDEWHOLEWORD, this );
}


bool matchString( const CString& str, const CString& substr, bool matchCase, int& matchPos )
{
	if ( matchCase )
	{
		matchPos = str.Find( substr );
		return matchPos != -1;
	}
	else
	{
		CString str2 = str, substr2 = substr;
		str2.MakeUpper();
		substr2.MakeUpper();
		matchPos = str2.Find( substr2 );
		return matchPos != -1;
	}
}


/*
 *
 */
afx_msg LRESULT CViewDialog::OnFindReplace(WPARAM wParam, LPARAM lParam)
{
	// Test 'Cancel'
	if ( FindDialog->IsTerminating() )
    {
		FindDialog = NULL;
		m_ListCtrl.ModifyStyle( LVS_SHOWSELALWAYS, 0 );
		select( -1 );
        return 0;
	}

	// Test 'Find'
	if ( FindDialog->FindNext() )
	{
		FindMatchCase = (FindDialog->MatchCase() != 0);
		FindDownwards = (FindDialog->SearchDown() != 0);
		FindStr = FindDialog->GetFindString();

		int lineIndex, matchPos;
		if ( FindDialog->SearchDown() )
		{
			BeginFindIndex = (getSelectionIndex() == -1) ? 0 : getSelectionIndex() + 1;
			for ( lineIndex=BeginFindIndex; lineIndex!=(int)Buffer.size(); ++lineIndex )
			{
				if ( matchString( Buffer[lineIndex], FindStr, FindDialog->MatchCase()!=0, matchPos ) )
				{
					scrollTo( lineIndex );
					select( lineIndex );
					//BeginFindIndex = getSelectionIndex()+1;
					//displayString();
					CString s;
					s.Format( _T("Found '%s' (downwards from line %d) at line %d:\r\n%s"), FindStr, BeginFindIndex, lineIndex, Buffer[lineIndex] );
					((CLog_analyserDlg*)GetParent())->displayCurrentLine( s );
					((CLog_analyserDlg*)GetParent())->selectText( 1, matchPos, FindStr.GetLength() );
					//BeginFindIndex = lineIndex+1;
					return 1;
				}
			}
		}
		else
		{
			BeginFindIndex = (getSelectionIndex() == -1) ? 0 : getSelectionIndex() - 1;
			for ( lineIndex=BeginFindIndex; lineIndex>=0; --lineIndex )
			{
				if ( matchString( Buffer[lineIndex], FindStr, FindDialog->MatchCase()!=0, matchPos ) )
				{
					scrollTo( lineIndex );
					select( lineIndex );
					//BeginFindIndex = getSelectionIndex()-1;
					//displayString();
					CString s;
					s.Format( _T("Found '%s' (upwards from line %d) at line %d:\r\n%s"), FindStr, BeginFindIndex, lineIndex, Buffer[lineIndex] );
					((CLog_analyserDlg*)GetParent())->displayCurrentLine( s );
					((CLog_analyserDlg*)GetParent())->selectText( 1, matchPos, FindStr.GetLength() );
					//BeginFindIndex = lineIndex-1;
					return 1;
				}
			}
		}
		CString s;
		s.Format( _T("Not found (%s from line %d)"), FindDialog->SearchDown() ? _T("downwards") : _T("upwards"), BeginFindIndex );
		AfxMessageBox( s );
		//BeginFindIndex = 0;
		return 0;
	}

	return 0;
}


void CViewDialog::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if ( (Index > 0) && (ChildWindowFromPoint( point ) == GetDlgItem( IDC_DragBar )) )
	{
		((CLog_analyserDlg*)GetParent())->beginResizeView( Index );
	}
	else
	{
		//PostMessage(WM_NCHITTEST,HTCAPTION,MAKELPARAM(point.x,point.y));	
		CDialog::OnLButtonDown(nFlags, point);
	}
}
