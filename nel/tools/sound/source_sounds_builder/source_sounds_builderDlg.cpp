// source_sounds_builderDlg.cpp : implementation file
//

#include "stdafx.h"
#include "source_sounds_builder.h"
#include "source_sounds_builderDlg.h"

#include <io.h>

#include "nel/misc/file.h"
#include "nel/misc/path.h"
using namespace NLMISC;

#include "file_dialog.h"

#include <string>
#include <fstream>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


struct compare_sound_ptr : public binary_function<CSound*,CSound*,bool>
{
	bool operator()( CSound* a, CSound* b )
	{
		nlassert( a && b );
		return *a < *b;
	}
};


/////////////////////////////////////////////////////////////////////////////
// CSource_sounds_builderDlg dialog

CSource_sounds_builderDlg::CSource_sounds_builderDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSource_sounds_builderDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSource_sounds_builderDlg)
	//}}AFX_DATA_INIT
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	_SoundPage = NULL;
}

void CSource_sounds_builderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSource_sounds_builderDlg)
	DDX_Control(pDX, IDC_TREE1, m_Tree);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSource_sounds_builderDlg, CDialog)
	//{{AFX_MSG_MAP(CSource_sounds_builderDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_AddSound, OnAddSound)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE1, OnSelchangedTree1)
	ON_BN_CLICKED(IDC_Save, OnSave)
	ON_NOTIFY(TVN_DELETEITEM, IDC_TREE1, OnDeleteitemTree1)
	ON_BN_CLICKED(IDC_Load, OnLoad)
	ON_BN_CLICKED(IDC_MoveUp, OnMoveUp)
	ON_BN_CLICKED(IDC_MoveDown, OnMoveDown)
	ON_BN_CLICKED(IDC_Import, OnImport)
	ON_NOTIFY(TVN_BEGINLABELEDIT, IDC_TREE1, OnBeginlabeleditTree1)
	ON_NOTIFY(TVN_ENDLABELEDIT, IDC_TREE1, OnEndlabeleditTree1)
	ON_BN_CLICKED(IDC_ImpDir, OnImpDir)
	ON_NOTIFY(TVN_KEYDOWN, IDC_TREE1, OnKeydownTree1)
	ON_BN_CLICKED(IDC_Sort, OnSortView)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSource_sounds_builderDlg message handlers

BOOL CSource_sounds_builderDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	/*
	 * Init
	 */

	//_Modified = false;
	_EditingName = false;

	ResetTree();

	_SoundPage = new CSoundPage( this );
	_SoundPage->setTree( &m_Tree );
	_SoundPage->Create( IDD_SoundPage );

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CSource_sounds_builderDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

HCURSOR CSource_sounds_builderDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


/*
 *
 */
void CSource_sounds_builderDlg::ResetTree()
{
	m_Tree.DeleteAllItems();

	// Root
	TVINSERTSTRUCT tvInsert;
	tvInsert.hParent = NULL;
	tvInsert.hInsertAfter = NULL;
	tvInsert.item.mask = TVIF_TEXT;
	tvInsert.item.pszText = _T("Sounds");
	m_Tree.InsertItem( &tvInsert );
}


/*
 *
 */
void CSource_sounds_builderDlg::OnAddSound() 
{
	HTREEITEM hitem = AddSound( "<New Sound>" );
	m_Tree.EditLabel( hitem );
}


/*
 *
 */
HTREEITEM CSource_sounds_builderDlg::AddSound( const char *name )
{
	CSound *sound = new CSound();
	sound->setProperties( name, "" );
	_Sounds.push_back( sound );
	HTREEITEM item = m_Tree.InsertItem( name, m_Tree.GetRootItem(), TVI_LAST );
	m_Tree.SetItemData( item, _Sounds.size()-1 );
	m_Tree.Expand( m_Tree.GetRootItem(), TVE_EXPAND );
	return item;
}


/*
 *
 */
void CSource_sounds_builderDlg::OnBeginlabeleditTree1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;

	if ( pTVDispInfo->item.hItem != m_Tree.GetRootItem() )
	{
		CString name = SoundName( pTVDispInfo->item.hItem );
		m_Tree.GetEditControl()->SetWindowText( name );
		m_Tree.SelectItem( pTVDispInfo->item.hItem );
		GetDlgItem( IDC_Import )->EnableWindow( false );
		GetDlgItem( IDC_ImpDir )->EnableWindow( false );
		GetDlgItem( IDC_AddSound )->EnableWindow( false );
		*pResult = 0;
		_EditingName = true;
	}
	else
	{
		*pResult = 1;
	}
}


/*
 *
 */
void CSource_sounds_builderDlg::OnEndlabeleditTree1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;

	if ( (pTVDispInfo->item.pszText != NULL) && (pTVDispInfo->item.pszText[0] != '\0') )
	{
		// Changed
		uint32 index = m_Tree.GetItemData( pTVDispInfo->item.hItem );
		nlassert( index < _Sounds.size() );
		if ( _Sounds[index]->getFilename() == "" )
		{
			CString s;
			s.Format( "%s*", pTVDispInfo->item.pszText );
			m_Tree.SetItemText( pTVDispInfo->item.hItem, s );
		}
		else
		{
			_SoundPage->setCurrentSound( _Sounds[index], pTVDispInfo->item.hItem );
			_SoundPage->rename( pTVDispInfo->item.pszText );
			_SoundPage->apply();
		}
		//_Modified = true;
	}
	else
	{
		// Cancelled
		//m_Tree.SetItemText( pTVDispInfo->item.hItem, "<New Sound>*" );
	}

	GetDlgItem( IDC_Import )->EnableWindow( true );
	GetDlgItem( IDC_ImpDir )->EnableWindow( true );
	GetDlgItem( IDC_AddSound )->EnableWindow( true );
	_EditingName = false;
	*pResult = 0;
}


/*
 *
 */
void CSource_sounds_builderDlg::OnSelchangedTree1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

	_SoundPage->apply();

	if ( (pNMTreeView->itemNew.hItem != NULL) && (pNMTreeView->itemNew.hItem != m_Tree.GetRootItem()) )
	{
		uint32 index = m_Tree.GetItemData( pNMTreeView->itemNew.hItem );
		nlassert( index < _Sounds.size() );
		_SoundPage->setCurrentSound( _Sounds[index], pNMTreeView->itemNew.hItem );
		_SoundPage->ShowWindow( SW_SHOW );
		_SoundPage->SetFocus();
		_SoundPage->getPropertiesFromSound();
	}
	else
	{
		_SoundPage->ShowWindow( SW_HIDE );
		_SoundPage->setCurrentSound( NULL, NULL );
	}

	*pResult = 0;
}


/*
 *
 */
void CSource_sounds_builderDlg::OnDeleteitemTree1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

	_SoundPage->cancel();
	
	nlassert( pNMTreeView );
	if ( pNMTreeView->itemOld.hItem != m_Tree.GetRootItem() )
	{
		uint32 index = m_Tree.GetItemData( pNMTreeView->itemOld.hItem );
		if ( index < _Sounds.size() )
		{
			_Sounds.erase( _Sounds.begin() + index );
		}

		// Renumber
		HTREEITEM hitem = m_Tree.GetChildItem( m_Tree.GetRootItem() );
		for ( index=0; index!=_Sounds.size(); index++ )
		{
			if ( hitem == pNMTreeView->itemOld.hItem )
			{
				hitem = m_Tree.GetNextItem( hitem, TVGN_NEXT );
			}
			m_Tree.SetItemData( hitem, index );
			hitem = m_Tree.GetNextItem( hitem, TVGN_NEXT );
		}

		//_Modified = true;
	}

	*pResult = 0;
}



/*
 *
 */
void CSource_sounds_builderDlg::OnMoveUp() 
{
	HTREEITEM hitem = m_Tree.GetSelectedItem();
	if ( (hitem != NULL) && (hitem != m_Tree.GetRootItem()) )
	{
		uint32 oldindex = m_Tree.GetItemData( hitem );
		uint32 newindex = oldindex - 1;
		if ( oldindex > 0 )
		{
			CSound *snd = _Sounds[oldindex];
			_Sounds[oldindex] = _Sounds[newindex];
			_Sounds[newindex] = snd;
			m_Tree.SetItemText( hitem, _Sounds[oldindex]->getFilename().c_str() );
			hitem = m_Tree.GetPrevSiblingItem( hitem );
			m_Tree.SetItemText( hitem, _Sounds[newindex]->getFilename().c_str() );
			m_Tree.SelectItem( hitem );
			//_Modified = true;
		}
	}
}


/*
 *
 */
void CSource_sounds_builderDlg::OnMoveDown() 
{
	HTREEITEM hitem = m_Tree.GetSelectedItem();
	if ( (hitem != NULL) && (hitem != m_Tree.GetRootItem()) )
	{
		uint32 oldindex = m_Tree.GetItemData( hitem );
		uint32 newindex = oldindex + 1;
		if ( oldindex < _Sounds.size()-1 )
		{
			CSound *snd = _Sounds[oldindex];
			_Sounds[oldindex] = _Sounds[newindex];
			_Sounds[newindex] = snd;
			m_Tree.SetItemText( hitem, _Sounds[oldindex]->getFilename().c_str() );
			hitem = m_Tree.GetNextSiblingItem( hitem );
			m_Tree.SetItemText( hitem, _Sounds[newindex]->getFilename().c_str() );
			m_Tree.SelectItem( hitem );
			//_Modified = true;
		}
	}
}


/*
 *
 */
CString CSource_sounds_builderDlg::SoundName( HTREEITEM hitem )
{
	CString s = m_Tree.GetItemText( hitem );
	sint last;
	if ( !s.empty() )
	{
		if ( s[s.GetLength()-1] == '*' )
		{
			last = s.GetLength()-2;
		}
		else
		{
			last = s.ReverseFind( '(' )-2;
		}
		if ( last >= 0 )
		{
			return s.Left( last+1 );
		}
		else
		{
			return s;
		}
	}
	else
	{
		return s;
	}
}


/*
 *
 */
void CSource_sounds_builderDlg::OnSave() 
{
	_SoundPage->apply();
	if ( _Filename == "" )
	{
		_Filename = "sounds.nss";
	}

	// Prompt filename
	CFileDialog savedlg( false, "nss", _Filename, OFN_OVERWRITEPROMPT, "NeL Source Sounds (*.nss)|*.nss||", this );
	if ( savedlg.DoModal()==IDOK )
	{
		CWaitCursor waitcursor;

		try
		{
			// Check for blank waves and duplicates
			set<string> nameset;
			string blanksounds, duplicates;
			vector<CSound*>::iterator ips;
			for ( ips=_Sounds.begin(); ips!=_Sounds.end(); ++ips )
			{
				if ( (*ips)->getFilename() == "" )
				{
					blanksounds += (*ips)->getName() + " ";
				}
				if ( nameset.find( (*ips)->getName() ) != nameset.end() )
				{
					duplicates += (*ips)->getName() + " ";
				}
				nameset.insert( (*ips)->getName() );
			}
			if ( !duplicates.empty() )
			{
				CString s;
				s.Format( "Warning: the following names are duplicates. The first occurence of each one was not written in the output file. Correct the names and save again:\n\n%s", duplicates.c_str() );
				AfxMessageBox( s, MB_ICONWARNING );
			}
			if ( !blanksounds.empty() )
			{
				CString s;
				s.Format( "Warning: the following sounds have no wave file specified:\n\n%s", blanksounds.c_str() );
				AfxMessageBox( s, MB_ICONWARNING );
			}

			// Save
			COFile file;
			file.open( string( savedlg.GetPathName() ), false );
			CSound::save( _Sounds, file );
			file.close();
		}
		catch ( Exception& e )
		{
			CString s;
			s.Format( "Error saving %s: %s", savedlg.GetPathName(), e.what() );
			AfxMessageBox( s );
		}

		//_Modified = false;
		_Filename = savedlg.GetFileName();

		waitcursor.Restore();
	}
}


/*
 *
 */
void CSource_sounds_builderDlg::OnLoad() 
{
	// Prompt filename
	CFileDialog opendlg( true, "nss", "", OFN_HIDEREADONLY, "NeL Source Sounds (*.nss)|*.nss||", this );
	if ( opendlg.DoModal()==IDOK )
	{
		CWaitCursor waitcursor;

		ResetTree();
		_SoundPage->ShowWindow( SW_HIDE );
		_Sounds.clear();

		// Load
		CIFile file;
		file.open( string( opendlg.GetPathName() ), false );
		TSoundMap soundmap;
		try
		{
			// Loading works even if the wave file are missing because we have called allowMissingWave() before
			CSound::load( soundmap, file );
			
			TSoundMap::iterator ipsnds;
			for ( ipsnds=soundmap.begin(); ipsnds!=soundmap.end(); ++ipsnds )
			{
				_Sounds.push_back( (*ipsnds).second );
			}

			// Sort
			sort( _Sounds.begin(), _Sounds.end(), compare_sound_ptr() );
			
		}
		catch( EStream& )
		{
			AfxMessageBox( "Cannot load: the file does not match the current format or version !", MB_ICONSTOP );
		}

		file.close();

		// Update tree
		UpdateTree();

		//_Modified = false;
		_Filename = opendlg.GetFileName();

		waitcursor.Restore();
	}
}


/*
 *
 */
void CSource_sounds_builderDlg::UpdateTree()
{
	uint32 i;
	for ( i=0; i!=_Sounds.size(); i++ )
	{
		CString s;
		s.Format( "%s (%s)", _Sounds[i]->getName().c_str(), _Sounds[i]->getFilename().c_str() );
		HTREEITEM item = m_Tree.InsertItem( s, m_Tree.GetRootItem(), TVI_LAST );
		m_Tree.SetItemData( item, i );
	}
	m_Tree.Expand( m_Tree.GetRootItem(), TVE_EXPAND );
}


/*
 *
 */
HTREEITEM CSource_sounds_builderDlg::FindInTree( const char *name )
{
	HTREEITEM hitem = m_Tree.GetChildItem( m_Tree.GetRootItem() );
	while ( hitem != NULL )
	{
		if ( SoundName( hitem ) == CString(name) )
		{
			return hitem;
		}
		hitem = m_Tree.GetNextItem( hitem, TVGN_NEXT );
	}
	return NULL;
}


/*
 *
 */
void CSource_sounds_builderDlg::OnImport() 
{
	// Prompt filename
	CFileDialog opendlg( true, "nsn", "", OFN_HIDEREADONLY, "NeL Sounds Names (*.nsn; *.txt)|*.nsn; *.txt||", this );
	if ( opendlg.DoModal()==IDOK )
	{
		CWaitCursor waitcursor;

		char name [80];
		ifstream fs;
		fs.open( opendlg.GetPathName() );
		while ( ! fs.eof() )
		{
			fs.getline( name, 40 );

			// Add new name if not already existing (useful for new versions of the names file)
			HTREEITEM hitem = FindInTree( name );
			if ( hitem == NULL )
			{
				string sname = string(name);
				if ( !sname.empty() ) // prevent from taking blank names
				{
					AddSound( sname.c_str() );
				}
			}

			// Note1: does not check if some names have been removed
			// Note2: does not check if there is twice the same name
		}
		fs.close();

		waitcursor.Restore();
	}
}


/*
 *
 */
void CSource_sounds_builderDlg::OnOK()
{
	// (Disable dialog closure by Enter)

	_SoundPage->apply();
	//_SoundPage->SetFocus(); // to Exit from label editing if one label in the tree control is in editing mode (in apply())
}


/*
 *
 */
void CSource_sounds_builderDlg::OnCancel()
{
	// Called when exiting (Esc, Alt+F4, etc.)

	if ( ! _EditingName )
	{
		/*if ( ! _Modified )
		{
			CDialog::OnCancel();
		}
		else*/
		{
			switch ( AfxMessageBox( "Save before exiting ?", MB_YESNOCANCEL | MB_ICONQUESTION ) )
			{
			// no break;
			case IDYES:
				OnSave();
			case IDNO:
				CDialog::OnCancel();
			}
		}
	}
}


/*
 *
 */
void CSource_sounds_builderDlg::OnImpDir() 
{
	// Prompt filename (CMultiFileDialog is a workaround class derived from CFileDialog)
	CMultiFileDialog opendlg( true, "wav", "", OFN_ALLOWMULTISELECT | OFN_EXPLORER | OFN_HIDEREADONLY, "Wave files (*.wav)|*.wav||", this );

	if ( opendlg.DoModal()==IDOK )
	{
		CWaitCursor waitcursor;

		POSITION pos = opendlg.GetStartPosition();
		CString pathname;
		char drive[_MAX_DRIVE];
		char dir[_MAX_DIR];
		char fname[_MAX_FNAME];
		char ext[_MAX_EXT];
		uint total = 0, err = 0;
		if ( pos != NULL )
		{
			do
			{
				pathname = opendlg.GetNextPathName( pos );
				_splitpath( pathname, drive, dir, fname, ext );
				err += addSoundAndFile( string(fname) );
				total += 1;
			}
			while ( pos != NULL );
		}

		/* // OLD
		_finddata_t fileinfo;
		long hf = _findfirst( "*.wav", &fileinfo );
		if ( hf != -1 )
		{
			addSoundAndFile( fileinfo.name );
			while ( _findnext( hf, &fileinfo ) == 0 )
			{
				addSoundAndFile( fileinfo.name );
			}
			_findclose( hf );
		}*/

		waitcursor.Restore();

		if ( err != 0 )
		{
			CString s;
			s.Format( "%u files on %u could not be loaded. Check the sounds for which 'Mono' or 'Stereo' is not visible", err, total );
			AfxMessageBox( s, MB_ICONWARNING );
		}
	}
	else if ( CommDlgExtendedError() != 0 )
	{
		CString s;
		s.Format( "File dialog error %u", CommDlgExtendedError() );
		MessageBox( s );
	}
}


/*
 * Return 0 if success, 1 if error
 */
uint CSource_sounds_builderDlg::addSoundAndFile( const string& name )
{
	// Add new name if not already existing (useful for new versions of the names file)
	HTREEITEM hitem = FindInTree( name.c_str() );
	if ( hitem == NULL )
	{
		hitem = AddSound( string(name + " (" + name + ".wav)").c_str() );
		uint32 index = m_Tree.GetItemData( hitem );
		nlassert( index < _Sounds.size() );
		_Sounds[index]->setProperties( name, name + ".wav" );
		_SoundPage->setCurrentSound( _Sounds[index], hitem );
		_SoundPage->getPropertiesFromSound();
		try
		{
			_SoundPage->loadSound();
		}
		catch( Exception& )
		{
			return 1;
		}
	}
	return 0;
}


/*
 *
 */
void CSource_sounds_builderDlg::OnKeydownTree1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_KEYDOWN* pTVKeyDown = (TV_KEYDOWN*)pNMHDR;

	if ( pTVKeyDown->wVKey == VK_DELETE )
	{
		// Remove sound
		_SoundPage->removeSound();
	}
	else if (  pTVKeyDown->wVKey == VK_F2 )
	{
		// Rename sound
		if ( m_Tree.GetSelectedItem() != NULL )
		{
			m_Tree.EditLabel( m_Tree.GetSelectedItem() );
		}
	}

	*pResult = 0;
}


/*
 *
 */
void CSource_sounds_builderDlg::OnSortView() 
{
	_SoundPage->ShowWindow( SW_HIDE );

	// Quick
	vector<CSound*> soundscopy = _Sounds;
	sort( soundscopy.begin(), soundscopy.end(), compare_sound_ptr() );
	ResetTree();
	_Sounds = soundscopy;
	UpdateTree();
}
