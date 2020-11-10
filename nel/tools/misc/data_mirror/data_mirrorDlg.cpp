// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2019  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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
#include "data_mirror.h"
#include "data_mirrorDlg.h"
#include "progress_dialog.h"
#include <sys/timeb.h>
#include "nel/misc/file.h"
#include "nel/misc/common.h"

using namespace std;
using namespace NLMISC;

#define PATH_WIDTH 500
#define SIZE_WIDTH 60
#define DATE_WIDTH 100
#define TYPE_WIDTH 80

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

#ifndef ListView_SetCheckState
   #define ListView_SetCheckState(hwndLV, i, fCheck) \
      ListView_SetItemState(hwndLV, i, \
      INDEXTOSTATEIMAGEMASK((fCheck)+1), LVIS_STATEIMAGEMASK)
#endif

CString GetString (uint res)
{
	CString str;
	str.LoadString (res);
	return str;
}

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CData_mirrorDlg dialog

CData_mirrorDlg::CData_mirrorDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CData_mirrorDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CData_mirrorDlg)
	ModifiedFilter = 0;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	ButtonXModifiedFromRight = 0;
	SortOrder = true;
	SortedColumn = 0;
}

void CData_mirrorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CData_mirrorDlg)
	DDX_Control(pDX, IDIGNORE, IgnoreCtrl);
	DDX_Control(pDX, IDC_MODIFIED_FILTERS, ModifiedFilterCtrl);
	DDX_Control(pDX, IDC_ADDED_FILTERS, AddedFilterCtrl);
	DDX_Control(pDX, IDC_REMOVED_FILTERS, RemovedFilterCtrl);
	DDX_Control(pDX, IDC_LIST, List);
	DDX_Radio(pDX, IDC_MODIFIED_FILTERS, ModifiedFilter);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CData_mirrorDlg, CDialog)
	//{{AFX_MSG_MAP(CData_mirrorDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDIGNORE, OnIgnore)
	ON_WM_SIZE()
	ON_NOTIFY(NM_CLICK, IDC_LIST, OnClickList)
	ON_BN_CLICKED(IDUPDATE, OnUpdate)
	ON_BN_CLICKED(IDC_ADDED_FILTERS, OnAddedFilters)
	ON_BN_CLICKED(IDC_MODIFIED_FILTERS, OnModifiedFilters)
	ON_BN_CLICKED(IDC_REMOVED_FILTERS, OnRemovedFilters)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST, OnClickList)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST, OnColumnclickList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CData_mirrorDlg message handlers

BOOL CData_mirrorDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// Init position remainder
	RECT client;
	RECT childClient;
	GetClientRect (&client);
	ModifiedFilterCtrl.GetWindowRect (&childClient);
	ScreenToClient (&childClient);
	ButtonXModifiedFromRight = client.right - childClient.left;
	AddedFilterCtrl.GetWindowRect (&childClient);
	ScreenToClient (&childClient);
	ButtonXAddedFromRight = client.right - childClient.left;
	RemovedFilterCtrl.GetWindowRect (&childClient);
	ScreenToClient (&childClient);
	ButtonXRemovedFromRight = client.right - childClient.left;
	List.GetWindowRect (&childClient);
	ScreenToClient (&childClient);
	ListBottomFromBottom = client.bottom - childClient.bottom;
	ListRightFromRight = client.right - childClient.right;
	
	// Init list
	ListView_SetExtendedListViewStyle (List, LVS_EX_CHECKBOXES);
	SHFILEINFO  sfi;
	HIMAGELIST imageListSmall = (HIMAGELIST)SHGetFileInfo( TEXT("C:\\"), 0, &sfi, sizeof(SHFILEINFO), SHGFI_SYSICONINDEX | SHGFI_SMALLICON);
	CImageList *imageList = CImageList::FromHandle( imageListSmall );
	List.SetImageList( imageList, LVSIL_SMALL);

	resize ();
	buildSourceFiles ();
	updateList ();
	updateSort ();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CData_mirrorDlg::updateList ()
{
	UpdateData ();

	// Checks
	nlassert ((ModifiedFilter>=Modified) && (ModifiedFilter<=Removed));

	// Delete item data
	uint count = List.GetItemCount ();
	uint i;
	for (i=0; i<count; i++)
		delete (std::list<CEntryFile>::iterator*)List.GetItemData(i);

	List.DeleteAllItems ();
	List.DeleteColumn (5);
	List.DeleteColumn (4);
	List.DeleteColumn (3);
	List.DeleteColumn (2);
	List.DeleteColumn (1);
	List.DeleteColumn (0);
	switch (ModifiedFilter)
	{
	case Modified:
		List.InsertColumn (0, GetString (IDS_NAME));
		List.InsertColumn (1, GetString (IDS_NEW_SIZE));
		List.InsertColumn (2, GetString (IDS_OLD_SIZE));
		List.InsertColumn (3, GetString (IDS_NEW_DATE));
		List.InsertColumn (4, GetString (IDS_OLD_DATE));
		List.InsertColumn (5, GetString (IDS_TYPE));
		break;
	case Added:
		List.InsertColumn (0, GetString (IDS_NAME));
		List.InsertColumn (1, GetString (IDS_NEW_SIZE));
		List.InsertColumn (2, GetString (IDS_NEW_DATE));
		List.InsertColumn (3, GetString (IDS_TYPE));
		break;
	case Removed:
		List.InsertColumn (0, GetString (IDS_NAME));
		List.InsertColumn (1, GetString (IDS_OLD_SIZE));
		List.InsertColumn (2, GetString (IDS_OLD_DATE));
		List.InsertColumn (3, GetString (IDS_TYPE));
		break;
	}

	// Add the items
	List.SetColumnWidth (0, PATH_WIDTH);

	// Sub string
	uint subString = 1;

	// Add the sizes
	if (ModifiedFilter != Added)
	{
		List.SetColumnWidth (subString++, SIZE_WIDTH);
	}
	if (ModifiedFilter != Removed)
	{
		List.SetColumnWidth (subString++, SIZE_WIDTH);
	}

	// Add the dates
	if (ModifiedFilter != Added)
	{
		List.SetColumnWidth (subString++, DATE_WIDTH);
	}
	if (ModifiedFilter != Removed)
	{
		List.SetColumnWidth (subString++, DATE_WIDTH);
	}

	// Add the type
	List.SetColumnWidth (subString++, TYPE_WIDTH);

	// Next item


	// Add items
	std::list<CEntryFile> &entries = Files[ModifiedFilter];
	std::list<CEntryFile>::iterator ite = entries.begin ();
	while (ite != entries.end ())
	{
		// Add the items
		const CEntryFile &entry = *ite;
		uint nItem = List.InsertItem(0, nlUtf8ToTStr(entry.Strings[CEntryFile::Path]), entry.Image);
		List.SetItemData (nItem, DWORD_PTR(new std::list<CEntryFile>::iterator (ite)));

		// Sub string
		subString = 1;

		// Add the sizes
		if (ModifiedFilter != Removed)
		{
			List.SetItemText(nItem, subString++, nlUtf8ToTStr(entry.Strings[CEntryFile::NewSize]));
		}
		if (ModifiedFilter != Added)
		{
			List.SetItemText(nItem, subString++, nlUtf8ToTStr(entry.Strings[CEntryFile::OldSize]));
		}

		// Add the dates
		if (ModifiedFilter != Removed)
		{
			List.SetItemText(nItem, subString++, nlUtf8ToTStr(entry.Strings[CEntryFile::NewDate]));
		}
		if (ModifiedFilter != Added)
		{
			List.SetItemText(nItem, subString++, nlUtf8ToTStr(entry.Strings[CEntryFile::OldDate]));
		}

		// Add the type
		List.SetItemText(nItem, subString++, nlUtf8ToTStr(entry.Strings[CEntryFile::Type]));

		// Next item
		ite++;
	}

	UpdateData (FALSE);
}

void CData_mirrorDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CData_mirrorDlg::OnPaint() 
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

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CData_mirrorDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CData_mirrorDlg::OnIgnore() 
{
	UpdateData ();

	// Update files
	uint count = List.GetItemCount ();
	uint i;
	std::list<CEntryFile> &entries = Files[ModifiedFilter];
	for (i=0; i<count; i++)
	{
		// Checked ?
		if (ListView_GetCheckState (List, i))
		{
			// Get the file
			CString itemText = List.GetItemText (i, 0);

			// Add to ignore list
			IgnoreFiles.insert (tStrToUtf8(itemText));

			// Remove from the file list
			std::list<CEntryFile>::iterator *ite = (std::list<CEntryFile>::iterator *)List.GetItemData (i);
			nlassert (ite);
			entries.erase (*ite);
			delete ite;

			// Remove the item
			List.DeleteItem (i);
			i--;
			count--;
		}
	}
}

void createDirectory (const string &dir)
{
	string temp = dir;
	temp = temp.substr (0, temp.size ()-1);
	temp = NLMISC::CFile::getPath (temp);
	if (!temp.empty ())
		createDirectory (temp);
	NLMISC::CFile::createDirectory (dir);
}

bool setFileTime(const std::string &filename, const FILETIME &result)
{
	HANDLE handle = CreateFile(nlUtf8ToTStr(filename), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (handle)
	{
		SetFileTime (handle, NULL, NULL, &result);
		CloseHandle (handle);
		return true;
	}
	return false;
}

void CData_mirrorDlg::OnOK() 
{
	// Update first
	OnUpdate ();
	
	bool success = true;

	// Update window
	CProgressDialog progress;
	progress.Create (CProgressDialog::IDD, this);
	progress.ShowWindow (SW_SHOW);
	progress.progress (0);

	uint totalCount = (uint)FilesToUpdate[Modified].size () + (uint)FilesToUpdate[Added].size () + (uint)FilesToUpdate[Removed].size ();
	uint currentFile = 0;

	// System time
	SYSTEMTIME systemTime;
	GetSystemTime(&systemTime);
	FILETIME fileTime;
	nlverify (SystemTimeToFileTime (&systemTime, &fileTime));

	// Update files
	std::vector<string> &modifiedList = FilesToUpdate[Modified];
	uint count = (uint)modifiedList.size ();
	uint i;
	for (i=0; i<count; i++)
	{
		// Copy it
		const string &itemText = modifiedList[i];
		string source = MainDirectory + itemText;
		string dest = MirrorDirectory + itemText;

		progress.DisplayString = "Copy \"" + source + "\" to \"" + dest + "\"";
		progress.progress(float(currentFile++) / (float(totalCount)));

		string directory = NLMISC::CFile::getPath(dest);
		createDirectory(directory.c_str());
		if (!CopyFile(nlUtf8ToTStr(source), nlUtf8ToTStr(dest), FALSE))
		{
			MessageBox(nlUtf8ToTStr("Can't copy file " + source + " in file " + dest), _T("NeL Data Mirror"),
			    MB_OK | MB_ICONEXCLAMATION);
			success = false;
		}
		else
		{
			if (!LogDirectory.empty())
			{
				string sTmp = LogDirectory + "data_mirror.txt";
				FILE *f = nlfopen(sTmp ,"at");
				
				if (f)
				{
					fprintf(f,"Modified file : %s\n", dest.c_str());
					fclose(f);
				}
			}
		}

		// Touch 
		setFileTime (dest.c_str(), fileTime);
	}

	std::vector<string> &addedList = FilesToUpdate[Added];
	count = (uint)addedList.size ();
	for (i=0; i<count; i++)
	{
		// Copy it
		const string &itemText = addedList[i];
		string source = MainDirectory+itemText;
		string dest = MirrorDirectory+itemText;
		string directory = NLMISC::CFile::getPath (dest);
			
		progress.DisplayString = "Copy \"" + source + "\" to \"" + dest + "\"";
		progress.progress (float(currentFile++)/(float(totalCount)));
		
		createDirectory (directory.c_str ());
		if (!CopyFile(nlUtf8ToTStr(source), nlUtf8ToTStr(dest), FALSE))
		{
			MessageBox(nlUtf8ToTStr("Can't copy file " + source + " in file " + dest), _T("NeL Data Mirror"),
				MB_OK|MB_ICONEXCLAMATION);
			success = false;
		}
		else
		{
			if (!LogDirectory.empty())
			{
				string sTmp = LogDirectory + "data_mirror.txt";
				FILE *f = nlfopen(sTmp, "at");
				if (f)
				{
					fprintf(f,"Added file : %s\n", dest.c_str());
					fclose(f);
				}
			}
		}

		// Touch 
		setFileTime (dest.c_str(), fileTime);
	}

	std::vector<string> &removedList = FilesToUpdate[Removed];
	count = (uint)removedList.size ();
	for (i=0; i<count; i++)
	{
		// Copy it
		const string &itemText = removedList[i];
		string dest = MirrorDirectory+itemText;
			
		progress.DisplayString = "Delete \"" + dest + "\"";
		progress.progress (float(currentFile++)/(float(totalCount)));

		if (!DeleteFile(nlUtf8ToTStr(dest)))
		{
			MessageBox(nlUtf8ToTStr("Can't delete the file " + dest), _T("NeL Data Mirror"),
				MB_OK|MB_ICONEXCLAMATION);
			success = false;
		}
		else
		{
			if (!LogDirectory.empty())
			{
				string sTmp = LogDirectory + "data_mirror.txt";
				FILE *f = nlfopen(sTmp, "at");
				
				if (f)
				{
					fprintf(f,"Removed file : %s\n", dest.c_str());
					fclose(f);
				}
			}
		}
	}

	FILE *file = fopen ((IgnoreDirectory+"ignore_list.txt").c_str (), "w");
	if (file)
	{
		// Save ignore list
		std::set<string>::iterator ite = IgnoreFiles.begin ();
		while (ite != IgnoreFiles.end ())
		{
			fputs ((*ite + "\n").c_str (), file);		

			ite++;
		}
	}

	CDialog::OnOK();
}

void CData_mirrorDlg::resize () 
{
	// Initialised ?
	if (ButtonXModifiedFromRight)
	{
		RECT client;
		RECT child;
		GetClientRect (&client);
		ModifiedFilterCtrl.GetClientRect (&child);
		ModifiedFilterCtrl.SetWindowPos (NULL, client.right - ButtonXModifiedFromRight, child.top, 0, 0, SWP_NOSIZE|SWP_NOZORDER);
		AddedFilterCtrl.SetWindowPos (NULL, client.right - ButtonXAddedFromRight, child.top, 0, 0, SWP_NOSIZE|SWP_NOZORDER);
		RemovedFilterCtrl.SetWindowPos (NULL, client.right - ButtonXRemovedFromRight, child.top, 0, 0, SWP_NOSIZE|SWP_NOZORDER);
		List.SetWindowPos (NULL, 0, 0, client.right - ListRightFromRight, client.bottom - ListBottomFromBottom, SWP_NOMOVE|SWP_NOZORDER);

		Invalidate ();
	}
}

void CData_mirrorDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	resize ();
}

bool getFileTime (const std::string &filename, FILETIME &result)
{
	HANDLE handle = CreateFile(nlUtf8ToTStr(filename), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (handle)
	{
		FILETIME res0;
		FILETIME res1;
		GetFileTime (handle, &res0, &res1, &result);
		CloseHandle (handle);
		return true;
	}
	return false;
}

void CData_mirrorDlg::buildSourceFiles ()
{
	UpdateData (FALSE);

	Files[Modified].clear ();
	Files[Added].clear ();
	Files[Removed].clear ();
	
	// List all files in database
	vector<string> fileSource;
	CPath::getPathContent (MainDirectory+CurrentDir, true, false, true, fileSource);
	uint i;
	uint count = (uint)fileSource.size ();
	for (i=0; i<count; i++)
	{
		// Get the filename
		string &str = fileSource[i];
		str = toLowerAscii(str.substr (MainDirectory.size (), str.size ()));

		// In the ignore list ?
		if (IgnoreFiles.find (str) == IgnoreFiles.end () && (str != "ignore_list.txt"))
		{
			// Does the destination exist ?
			string mirrorFile = MirrorDirectory+str;
			string mainFile = MainDirectory+str;
			if (NLMISC::CFile::fileExists (mirrorFile))
			{
				FILETIME time0;
				FILETIME time1;
				getFileTime (mirrorFile, time0);
				getFileTime (mainFile, time1);
				sint64 deltaInt = (((uint64)time1.dwHighDateTime)<<32|((uint64)time1.dwLowDateTime)) - (((uint64)time0.dwHighDateTime)<<32|((uint64)time0.dwLowDateTime));
				double deltaInSec = (double)deltaInt;
				deltaInSec /= 10000000.0;
				deltaInSec = fabs(deltaInSec);

				if (deltaInSec > 2.0)
				{
					if (BinaryCompare)
					{
						bool bDiff = false;
						
						// Check Files sizes
						if (NLMISC::CFile::getFileSize(mainFile) != NLMISC::CFile::getFileSize(mirrorFile))
							bDiff = true;
						
						// Check Files (Binary check)
						if (!bDiff)
						{
							uint32 nLength = NLMISC::CFile::getFileSize(mainFile);
							CIFile inMain, inMirror;
							if (inMain.open(mainFile) && inMirror.open(mirrorFile))
							{
								uint8 bufferMain[1024];
								uint8 bufferMirror[1024];
								while (nLength > 0)
								{
									uint32 r = min(nLength, (uint32)1024);
									inMain.serialBuffer(&bufferMain[0], r);
									inMirror.serialBuffer(&bufferMirror[0], r);
									if (memcmp(bufferMirror,bufferMain,r) != 0)
									{
										bDiff = true;
										break;
									}
									nLength -= r;
								}
							}
							else
								bDiff = true;
						}

						if (bDiff)
						{
							addEntry (Modified, str.c_str (), time1, time0);
						}
						else
						{
							// Update time stamp
							FILETIME fileTime;
							getFileTime (mainFile, fileTime);
							setFileTime (mirrorFile, fileTime);
						}
					}
					else
						addEntry (Modified, str.c_str (), time1, time0);
				}
			}
			else
			{
				FILETIME time;
				getFileTime (mainFile, time);
				addEntry (Added, str.c_str(), time, time);
			}
		}
	}

	// List all files in mirror
	fileSource.clear ();
	CPath::getPathContent (MirrorDirectory+CurrentDir, true, false, true, fileSource);
	uint count2 = (uint)fileSource.size ();
	for (i=0; i<count2; i++)
	{
		// Get the filename
		string &str = fileSource[i];
		str = toLowerAscii(str.substr (MirrorDirectory.size (), str.size ()));

		// In the ignore list ?
		if (IgnoreFiles.find (str) == IgnoreFiles.end () && (str != "ignore_list.txt"))
		{
			// Does the destination exist ?
			string mirrorFile = MirrorDirectory+str;
			string mainFile = MainDirectory+str;
			if (!NLMISC::CFile::fileExists (mainFile))
			{
				FILETIME time;
				getFileTime (mainFile.c_str (), time);
				addEntry (Removed, str.c_str (), time, time);
			}
		}
	}

	// Sort
	/*ModifiedList.SortItems ();
	AddedList.SortItems ();
	RemovedList.SortItems ();*/

	UpdateData (TRUE);
}

void timeToString (string &dest, FILETIME time)
{
	FILETIME localFileTime;
	SYSTEMTIME systemTime;
	if (FileTimeToLocalFileTime (&time, &localFileTime))
	{
		if (FileTimeToSystemTime (&localFileTime, &systemTime))
		{
			char date[512];
			smprintf (date, 512, "%d/%d/%d %d:%02d", systemTime.wDay, systemTime.wMonth, systemTime.wYear, systemTime.wHour, systemTime.wMinute);
			dest = date;
		}
	}
}

void sizeToString (string &dest, uint size)
{
	char sizeText[512];
	smprintf (sizeText, 512, "%d KB", (size / 1024) + ((size%1024) ? 1 : 0));
	dest = sizeText;
}

class CExtension
{
public:
	string	Description;
	uint	Icon;
};

std::map<string, CExtension> MapExtensions;

void CData_mirrorDlg::addEntry (uint where, const char *filename, FILETIME &newDate, FILETIME &oldDate)
{
	// Add an entry first
	Files[where].push_back (CEntryFile ());
	CEntryFile &file = Files[where].back ();
	file.Strings[CEntryFile::Path] = filename;
	file.NewDateST = newDate;
	file.OldDateST = oldDate;
	std::string aFilename;

	string mirrorFile = MirrorDirectory+filename;
	string mainFile = MainDirectory+filename;

	if (where != Added)
	{
		// Get file size
		file.OldSizeUI = NLMISC::CFile::getFileSize (mirrorFile);
		sizeToString (file.Strings[CEntryFile::OldSize], file.OldSizeUI);
		
		// Date
		timeToString (file.Strings[CEntryFile::OldDate], oldDate);
		aFilename = mirrorFile;
	}

	if (where != Removed)
	{
		// Get file size
		file.NewSizeUI = NLMISC::CFile::getFileSize (mainFile);
		sizeToString (file.Strings[CEntryFile::NewSize], file.NewSizeUI);
		
		// Date
		timeToString (file.Strings[CEntryFile::NewDate], newDate);
		aFilename = mainFile;
	}

	// Get the extension
	string ext = NLMISC::CFile::getExtension (aFilename);
	std::map<string, CExtension>::iterator ite = MapExtensions.find (ext);
	if (ite == MapExtensions.end ())
	{
		// Get the image
		SHFILEINFO     sfi;
		TCHAR winName[512];
		_tcscpy(winName, nlUtf8ToTStr(aFilename));
		TCHAR *ptr = winName;
		while (*ptr)
		{
			if (*ptr==_T('/'))
				*ptr = _T('\\');
			ptr++;
		}
		SHGetFileInfo (winName, 0, &sfi, sizeof (SHFILEINFO), SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_LINKOVERLAY | SHGFI_TYPENAME );

		CExtension extension;
		extension.Description = tStrToUtf8(sfi.szTypeName);
		extension.Icon = sfi.iIcon;
		ite = MapExtensions.insert (std::map<string, CExtension>::value_type (ext, extension)).first;
	}

	file.Strings[CEntryFile::Type] = ite->second.Description;
	file.Image = ite->second.Icon;
}

void CData_mirrorDlg::OnClickList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LPNMLISTVIEW lpnmlv = (LPNMLISTVIEW) pNMHDR;

	UpdateData (FALSE);
	UINT flags;
	List.HitTest (lpnmlv->ptAction, &flags);

	// Get check button state
	BOOL state = ListView_GetCheckState (List, lpnmlv->iItem);
	if (List.GetItemState (lpnmlv->iItem, LVIS_SELECTED) == LVIS_SELECTED)
	if (flags & LVHT_ONITEMSTATEICON)
	{
		POSITION pos = List.GetFirstSelectedItemPosition();
		while (pos)
		{
			int nItem = List.GetNextSelectedItem(pos);
			if (nItem != lpnmlv->iItem)
			{
				ListView_SetCheckState (List, nItem, !state);
			}
		}
	}
	UpdateData (TRUE);
	
	*pResult = 0;
}

void CData_mirrorDlg::OnUpdate() 
{
	UpdateData ();

	// Checks
	nlassert ((ModifiedFilter>=Modified) && (ModifiedFilter<=Removed));

	// The vector to add into
	std::vector<string> &entriesToUpdate = FilesToUpdate[ModifiedFilter];
	std::list<CEntryFile> &entries = Files[ModifiedFilter];

	// Update files
	uint count = List.GetItemCount ();
	uint i;
	for (i=0; i<count; i++)
	{
		// Checked ?
		if (ListView_GetCheckState (List, i))
		{
			// Get the file
			CString itemText = List.GetItemText (i, 0);

			// Add to ignore good list
			entriesToUpdate.push_back (tStrToUtf8(itemText));

			// Remove from the file list
			std::list<CEntryFile>::iterator *ite = (std::list<CEntryFile>::iterator *)List.GetItemData (i);
			nlassert (ite);
			entries.erase (*ite);
			delete ite;

			// Remove the item
			List.DeleteItem (i);
			i--;
			count--;
		}
	}

	UpdateData (FALSE);
}

void CData_mirrorDlg::OnAddedFilters() 
{
	UpdateData ();
	SortOrder = true;
	SortedColumn = 0;
	updateList ();
	updateSort ();
	IgnoreCtrl.EnableWindow (TRUE);
	UpdateData (FALSE);
}

void CData_mirrorDlg::OnModifiedFilters() 
{
	UpdateData ();
	SortOrder = true;
	SortedColumn = 0;
	updateList ();
	updateSort ();
	IgnoreCtrl.EnableWindow (TRUE);
	UpdateData (FALSE);
}

void CData_mirrorDlg::OnRemovedFilters() 
{
	UpdateData ();
	SortOrder = true;
	SortedColumn = 0;
	updateList ();
	updateSort ();
	IgnoreCtrl.EnableWindow (FALSE);
	UpdateData (FALSE);
}

void CData_mirrorDlg::OnColumnclickList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NMLISTVIEW *phdr = (NMLISTVIEW*)pNMHDR;

	if (SortedColumn == (uint)phdr->iSubItem)
		SortOrder ^= true;
	else
		SortedColumn = phdr->iSubItem;

	updateSort ();

	*pResult = 0;
}

int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, 
   LPARAM lParamSort)
{
	CData_mirrorDlg *dlg = (CData_mirrorDlg *)lParamSort;

	// Get the value pointers
	CEntryFile &entry1 = **(list<CEntryFile>::iterator*)lParam1;
	CEntryFile &entry2 = **(list<CEntryFile>::iterator*)lParam2;

	int result;
	switch (dlg->ModifiedFilter)
	{
	case CData_mirrorDlg::Modified:
		{
			switch (dlg->SortedColumn)
			{
			case 0:
				result = strcmp (entry1.Strings[CEntryFile::Path].c_str (), entry2.Strings[CEntryFile::Path].c_str ());
				break;
			case 1:
				result = (entry1.NewSizeUI < entry2.NewSizeUI) ? -1 : (entry1.NewSizeUI == entry2.NewSizeUI) ? 0 : 1;
				break;
			case 2:
				result = (entry1.OldSizeUI < entry2.OldSizeUI) ? -1 : (entry1.OldSizeUI == entry2.OldSizeUI) ? 0 : 1;
				break;
			case 3:
				result = CompareFileTime (&entry1.NewDateST, &entry2.NewDateST);
				break;
			case 4:
				result = CompareFileTime (&entry1.OldDateST, &entry2.OldDateST);
				break;
			case 5:
				result = strcmp (entry1.Strings[CEntryFile::Type].c_str (), entry2.Strings[CEntryFile::Type].c_str ());
				break;
			}
			break;
		}
	case CData_mirrorDlg::Added:
		{
			switch (dlg->SortedColumn)
			{
			case 0:
				result = strcmp (entry1.Strings[CEntryFile::Path].c_str (), entry2.Strings[CEntryFile::Path].c_str ());
				break;
			case 1:
				result = (entry1.NewSizeUI < entry2.NewSizeUI) ? -1 : (entry1.NewSizeUI == entry2.NewSizeUI) ? 0 : 1;
				break;
			case 2:
				result = CompareFileTime (&entry1.NewDateST, &entry2.NewDateST);
				break;
			case 3:
				result = strcmp (entry1.Strings[CEntryFile::Type].c_str (), entry2.Strings[CEntryFile::Type].c_str ());
				break;
			}
			break;
		}
	case CData_mirrorDlg::Removed:
		{
			switch (dlg->SortedColumn)
			{
			case 0:
				result = strcmp (entry1.Strings[CEntryFile::Path].c_str (), entry2.Strings[CEntryFile::Path].c_str ());
				break;
			case 1:
				result = (entry1.OldSizeUI < entry2.OldSizeUI) ? -1 : (entry1.OldSizeUI == entry2.OldSizeUI) ? 0 : 1;
				break;
			case 2:
				result = CompareFileTime (&entry1.OldDateST, &entry2.OldDateST);
				break;
			case 3:
				result = strcmp (entry1.Strings[CEntryFile::Type].c_str (), entry2.Strings[CEntryFile::Type].c_str ());
				break;
			}
			break;
		}
	}
	return dlg->SortOrder ? result : -result;
}

void CData_mirrorDlg::updateSort ()
{
	List.SortItems ( CompareFunc, (LPARAM)this );
}
