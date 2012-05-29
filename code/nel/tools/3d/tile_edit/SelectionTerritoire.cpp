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

#include "stdafx.h"
#include "resource.h"
#include "DialogEditList.h"
#include "resource.h"
#include "SelectionTerritoire.h"
#include "GetVal.h"
#include "Browse.h"
#include "choose_veget_set.h"
#include <shlobj.h>

using namespace NL3D;
using namespace NLMISC;

#define REGKEY_MAINFILE "MAINFILE"

CTileBank tileBank;
CTileBank tileBank2;

void Start(void) //main function
{
//	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	SelectionTerritoire Sel;

	Sel.DoModal();
}	

/////////////////////////////////////////////////////////////////////////////
// SelectionTerritoire dialog


SelectionTerritoire::SelectionTerritoire(CWnd* pParent /*=NULL*/)
	: CDialog(SelectionTerritoire::IDD, pParent)
{
	//{{AFX_DATA_INIT(SelectionTerritoire)
	SurfaceData = 0;
	//}}AFX_DATA_INIT
}


void SelectionTerritoire::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(SelectionTerritoire)
	DDX_Control(pDX, IDC_SURFACE_DATA, SurfaceDataCtrl);
	DDX_Text(pDX, IDC_SURFACE_DATA, SurfaceData);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(SelectionTerritoire, CDialog)
	//{{AFX_MSG_MAP(SelectionTerritoire)
	ON_BN_CLICKED(IDC_ADD_TERRITOIRE, OnAddTerritoire)
	ON_BN_CLICKED(IDC_EDIT_TERRITOIRE, OnEditTerritoire)
	ON_BN_CLICKED(IDC_REMOVE_TERRITOIRE, OnRemoveTerritoire)
	ON_BN_CLICKED(IDC_ADD_TILESET, OnAddTileSet)
	ON_BN_CLICKED(IDC_EDIT_TILESET, OnEditTileSet)
	ON_BN_CLICKED(IDC_REMOVE_TILESET, OnRemoveTileSet)
	ON_BN_CLICKED(IDC_EDIT_MONTER, OnMonter)
	ON_BN_CLICKED(IDC_EDIT_DESCENDRE, OnDescendre)
	ON_BN_CLICKED(ID_SELECT, OnSelect)
	ON_BN_CLICKED(ID_SAVE, OnSave)
	ON_BN_CLICKED(ID_SAVE_AS, OnSaveAs)
	ON_BN_CLICKED(IDC_PATH, OnPath)
	ON_BN_CLICKED(ID_EXPORT, OnExport)
	ON_BN_CLICKED(IDC_CHOOSE_VEGET, OnChooseVeget)
	ON_EN_CHANGE(IDC_SURFACE_DATA, OnChangeSurfaceData)
	ON_LBN_SELCHANGE(IDC_TILE_SET, OnSelchangeTileSet)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// SelectionTerritoire message handlers

void SelectionTerritoire::OnAddTerritoire() 
{
	// TODO: Add your control notification handler code here
	GetVal GetStr;
	GetStr.DoModal();
	if (GetStr.NameOk && GetStr.name[0])
	{
		CListBox *list=(CListBox*)GetDlgItem(IDC_LIST_TERRITOIRE);
		if (list->FindStringExact(0,GetStr.name)!=LB_ERR)
		{
			MessageBox("Ce nom existe deja","Error",MB_ICONERROR);
		}
		else
		{
			list->InsertString(-1, GetStr.name);
			tileBank.addLand (GetStr.name);
		}
	}
}

class EditTerr : public CDialogEditList
{
public:
	EditTerr (int land)
	{
		_land=land;
	}
private:
	virtual void OnInit ()
	{
		UpdateData ();
		char sTitle[512];
		sprintf (sTitle, "Tile sets use by %s", tileBank.getLand(_land)->getName().c_str());
		SetWindowText (sTitle);
		for (int i=0; i<tileBank.getTileSetCount(); i++)
		{
			m_ctrlCombo.InsertString (-1, tileBank.getTileSet(i)->getName().c_str());
			if (tileBank.getLand(_land)->isTileSet (tileBank.getTileSet(i)->getName()))
				m_ctrlList.InsertString (-1, tileBank.getTileSet(i)->getName().c_str());
		}
		UpdateData (FALSE);
	}
	virtual void OnOk ()
	{
		UpdateData ();
		int i;
		for (i=0; i<tileBank.getTileSetCount(); i++)
		{
			// remove tile set
			tileBank.getLand(_land)->removeTileSet (tileBank.getTileSet(i)->getName());
		}
		for (i=0; i<m_ctrlList.GetCount(); i++)
		{
			CString rString;
			m_ctrlList.GetText(i, rString);
			tileBank.getLand(_land)->addTileSet ((const char*)rString);
		}
		UpdateData (FALSE);
	}
	int _land;
};

void SelectionTerritoire::OnEditTerritoire() 
{
	// TODO: Add your control notification handler code here

	CListBox *list=(CListBox*)GetDlgItem(IDC_LIST_TERRITOIRE);
	int index=list->GetCurSel();
	if (index!=LB_ERR) 
	{
		EditTerr edit(index);
		edit.DoModal();
	}
	else
	{
		MessageBox("No tilesset selected","Error",MB_ICONERROR);
	}
}

void SelectionTerritoire::OnRemoveTerritoire() 
{
	// TODO: Add your control notification handler code here
	CListBox *list=(CListBox*)GetDlgItem(IDC_LIST_TERRITOIRE);	
	int nindex=list->GetCurSel();
	if (nindex!=LB_ERR) 
	{
		tileBank.removeLand (nindex);
		list->DeleteString(nindex);
	}
	else
	{
		MessageBox("No tilesset selected","Chcrois kca va pas etreuu possibleuuu",MB_ICONERROR);
	}
}





void SelectionTerritoire::OnAddTileSet() 
{
	// TODO: Add your control notification handler code here
	GetVal GetStr;
	GetStr.DoModal();
	if (GetStr.NameOk && GetStr.name[0])
	{
		CListBox *list=(CListBox*)GetDlgItem(IDC_TILE_SET);
		if (list->FindStringExact(0,GetStr.name)!=LB_ERR)
		{
			MessageBox("Ce nom existe deja","Error",MB_ICONERROR);
		}
		else
		{
			list->InsertString(-1, GetStr.name);
			tileBank.addTileSet (GetStr.name);
		}
	}
}

class EditTileSet : public CDialogEditList
{
public:
	EditTileSet (int tileSet)
	{
		_tileSet=tileSet;
	}
private:
	virtual void OnInit ()
	{
		UpdateData ();
		char sTitle[512];
		sprintf (sTitle, "Children of the tile set %s", tileBank.getTileSet(_tileSet)->getName().c_str());
		SetWindowText (sTitle);
		for (int i=0; i<tileBank.getTileSetCount(); i++)
		{
			if (i!=_tileSet)
				m_ctrlCombo.InsertString (-1, tileBank.getTileSet(i)->getName().c_str());
			if (tileBank.getTileSet(_tileSet)->isChild (tileBank.getTileSet(i)->getName()))
				m_ctrlList.InsertString (-1, tileBank.getTileSet(i)->getName().c_str());
		}
		UpdateData (FALSE);
	}
	virtual void OnOk ()
	{
		UpdateData ();
		int i;
		for (i=0; i<tileBank.getTileSetCount(); i++)
		{
			// remove tile set
			tileBank.getTileSet(_tileSet)->removeChild (tileBank.getTileSet(i)->getName());
		}
		for (i=0; i<m_ctrlList.GetCount(); i++)
		{
			CString rString;
			m_ctrlList.GetText(i, rString);
			tileBank.getTileSet(_tileSet)->addChild ((const char*)rString);
		}
		UpdateData (FALSE);
	}
	int _tileSet;
};

void SelectionTerritoire::OnEditTileSet() 
{
	// TODO: Add your control notification handler code here

	CListBox *list=(CListBox*)GetDlgItem(IDC_TILE_SET);
	int index=list->GetCurSel();
	if (index!=LB_ERR) 
	{
		tileBank2=tileBank;
		Browse plop(index, this);
		list->GetText (index,CurrentTerritory);
		if (plop.DoModal()==IDOK)
		{
			tileBank=tileBank2;
		}
	}
	else
	{
		MessageBox("No tilesset selected","Error",MB_ICONERROR);
	}
}

void SelectionTerritoire::OnRemoveTileSet() 
{
	// TODO: Add your control notification handler code here
	CListBox *list=(CListBox*)GetDlgItem(IDC_TILE_SET);	
	int nindex=list->GetCurSel();
	if (nindex!=LB_ERR) 
	{
		tileBank.removeTileSet(nindex);
		list->DeleteString(nindex);
	}
	else
	{
		MessageBox("No tilesset selected","Chcrois kca va pas etreuu possibleuuu",MB_ICONERROR);
	}
}

void SelectionTerritoire::OnMonter()
{
	// TODO: Add your control notification handler code here
	CListBox *list=(CListBox*)GetDlgItem(IDC_TILE_SET);	
	int nindex=list->GetCurSel();
	if (nindex!=LB_ERR) 
	{
		if (nindex>0)
		{
			tileBank.xchgTileset (nindex, nindex-1);

			// xchg the name
			CString tmp1, tmp2;
			list->GetText(nindex-1, tmp1);
			list->GetText(nindex, tmp2);
			
			list->DeleteString (nindex-1);
			list->DeleteString (nindex-1);

			list->InsertString (nindex-1, tmp1);
			list->InsertString (nindex-1, tmp2);

			list->SetCurSel(nindex-1);
		}
	}
	else
	{
		MessageBox("No tilesset selected","Chcrois kca va pas etreuu possibleuuu",MB_ICONERROR);
	}
}

void SelectionTerritoire::OnDescendre()
{
	// TODO: Add your control notification handler code here
	CListBox *list=(CListBox*)GetDlgItem(IDC_TILE_SET);	
	int nindex=list->GetCurSel();
	if (nindex!=LB_ERR) 
	{
		if (nindex<(list->GetCount()-1))
		{
			tileBank.xchgTileset (nindex, nindex+1);
		
			// xchg the name
			CString tmp1, tmp2;
			list->GetText(nindex, tmp1);
			list->GetText(nindex+1, tmp2);

			list->DeleteString (nindex);
			list->DeleteString (nindex);

			list->InsertString (nindex, tmp1);
			list->InsertString (nindex, tmp2);

			list->SetCurSel(nindex+1);
		}
	}
	else
	{
		MessageBox("No tilesset selected","Chcrois kca va pas etreuu possibleuuu",MB_ICONERROR);
	}
}

void SelectionTerritoire::OnSelect() 
{
	// TODO: Add your control notification handler code here
	CListBox *list=(CListBox*)GetDlgItem(IDC_LIST_TERRITOIRE);
	CListBox *list2=(CListBox*)GetDlgItem(IDC_TILE_SET);
	static char BASED_CODE szFilter[] = 
		"NeL tile bank files (*.bank)|*.bank|All Files (*.*)|*.*||";
 	CFileDialog sFile(true, "bank", "main.bank",0, szFilter, this);
	if (sFile.DoModal()==IDOK)
	{
		POSITION p = sFile.GetStartPosition();
		CString str = sFile.GetNextPathName(p);
		char *temp = str.GetBuffer(256);
		if (temp)
		{
			CIFile stream;
			if (stream.open ((const char*)str))
			{
				list->ResetContent ();
				list2->ResetContent ();
				tileBank.clear();
				tileBank.serial (stream);
			}
			
			int i;
			for (i=0; i<tileBank.getLandCount(); i++)
			{
				// Add to the list
				list->AddString(tileBank.getLand(i)->getName().c_str());
			}

			for (i=0; i<tileBank.getTileSetCount(); i++)
			{
				// Add to the list
				list2->AddString(tileBank.getTileSet(i)->getName().c_str());
			}

			char drive[256],name[256],path[256],ext[256];
			_splitpath(temp,drive,path,name,ext);
			MainFileName = name;
			MainFileName += ext;
			DefautPath = drive;
			DefautPath += path;
			
			MainFileOk = 1;
			CButton *button = (CButton*)GetDlgItem(IDC_ADD_TERRITOIRE);
			button->EnableWindow(true);
			button = (CButton*)GetDlgItem(IDC_REMOVE_TERRITOIRE);
			button->EnableWindow(true);
			button = (CButton*)GetDlgItem(IDC_EDIT_TERRITOIRE);
			button->EnableWindow(true);
			button = (CButton*)GetDlgItem(ID_SAVE);
			button->EnableWindow(true);

			// Change the bouton text path
			GetDlgItem (IDC_PATH)->SetWindowText (tileBank.getAbsPath().c_str());
		}
	}

	OnSelchangeTileSet ();
}

LRESULT SelectionTerritoire::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (message==WM_INITDIALOG)
	{
	}

	return CDialog::WindowProc(message, wParam, lParam);
}

void SelectionTerritoire::OnOK() 
{
	// TODO: Add extra validation here
	if (::MessageBox (NULL, "Are you sure you want to quit?", "Quit", MB_OK|MB_ICONQUESTION|MB_YESNO)==IDYES)
	{
			CDialog::OnOK();
	}
}

void SelectionTerritoire::OnSave()
{
	// TODO: Add extra validation here
	CString str = DefautPath + MainFileName;
	Save (str, tileBank);
}

void SelectionTerritoire::Save(const char* path, CTileBank &toSave)
{
	// TODO: Add extra validation here
	{
		COFile stream;
		if (stream.open ((const char*)path))
		{
			toSave.serial (stream);
		}
	}
}

void SelectionTerritoire::OnSaveAs()
{
	// TODO: Add your control notification handler code here
	static char BASED_CODE szFilter[] = 
		"NeL tile bank files (*.bank)|*.bank|All Files (*.*)|*.*||";
 	CFileDialog sFile(false, "bank", DefautPath+MainFileName, 0, szFilter, this);
	if (sFile.DoModal()==IDOK)
	{
		Save (sFile.GetPathName(), tileBank);
		MainFileOk = 1;
		CButton *button = (CButton*)GetDlgItem(IDC_ADD_TERRITOIRE);
		button->EnableWindow(true);
		button = (CButton*)GetDlgItem(IDC_REMOVE_TERRITOIRE);
		button->EnableWindow(true);
		button = (CButton*)GetDlgItem(IDC_EDIT_TERRITOIRE);
		button->EnableWindow(true);
		button = (CButton*)GetDlgItem(ID_SAVE);
		button->EnableWindow(true);

		// Create a file name
		char drive[256],name[256],path[256],ext[256];
		_splitpath(sFile.GetPathName(), drive, path, name, ext);
		MainFileName = name;
		MainFileName += ext;
		DefautPath = drive;
		DefautPath += path;
	}
}

void SelectionTerritoire::OnCancel() 
{
	// TODO: Add extra cleanup here
	if (::MessageBox (NULL, "Are you sure you want to quit?", "Quit", MB_OK|MB_ICONQUESTION|MB_YESNO)==IDYES)
	{
		CDialog::OnCancel();
	}
}

bool CheckPath (const std::string& path, const char* absolutePathToRemplace)
{
	// Look for absolute path in path
	if (strnicmp (path.c_str(), absolutePathToRemplace, strlen (absolutePathToRemplace))==0)
		return true;
	else
		return false;
}

bool RemovePath (std::string& path, const char* absolutePathToRemplace)
{
	// Look for absolute path in path
	if (strnicmp (path.c_str(), absolutePathToRemplace, strlen (absolutePathToRemplace))==0)
	{
		// New path
		std::string toto=path;
		path=toto.c_str()+strlen (absolutePathToRemplace);
		return true;
	}
	else
		return false;
}

void SelectionTerritoire::OnPath() 
{
	// TODO: Add your control notification handler code here
	
	// Select a directory.
	char path[MAX_PATH];

	// Build the struct
	BROWSEINFO info;
	memset (&info, 0, sizeof (BROWSEINFO));
	info.lpszTitle="Select the absolute base path of the bank";
	info.ulFlags=BIF_RETURNONLYFSDIRS;

	// Select the path
	LPITEMIDLIST list;
	if (list=SHBrowseForFolder (&info))
	{
		// Convert item into path string
		BOOL bRet=SHGetPathFromIDList(list, path);
		nlassert (bRet);

		// Add a final back slash
		if (strcmp (path, "")!=0)
		{
			// Add a '\' at the end
			if (path[strlen (path)-1]!='\\')
				strcat (path, "\\");
		}

		// Last check
		char msg[512];
		sprintf (msg, "Do you really want to set %s as base path of the bank ?", path);
		if (MessageBox (msg, "TileEdit", MB_YESNO|MB_ICONQUESTION)==IDYES)
		{
			// Set as default path..

			// Old path
			const char* oldPath=tileBank.getAbsPath ().c_str();

			// Path are good
			bool goodPath=true;

			// If no absolute path, must check before use it
			if ((*oldPath)==0)
			{
				// Compute xref
				tileBank.computeXRef();

				// For all tiles, check we can change the path
				for (int tiles=0; tiles<tileBank.getTileCount(); tiles++)
				{
					// Get tile xref
					int tileSet;
					int number;
					CTileBank::TTileType type;
					tileBank.getTileXRef (tiles, tileSet, number, type);

					// Is tile used ?
					if (tileSet!=-1)
					{
						// 3 types of bitmaps
						int type;
						for (type=CTile::diffuse; type<CTile::bitmapCount; type++)
						{
							// Bitmap string
							const std::string& bitmapPath=tileBank.getTile(tiles)->getRelativeFileName ((CTile::TBitmap)type);

							// not empty ?
							if (bitmapPath!="")
							{
								// Check the path
								if (CheckPath (bitmapPath, path)==false)
								{
									// Bad path
									goodPath=false;

									// Make a message
									sprintf (msg, "Path '%s' can't be found in bitmap '%s'. Continue ?", path, bitmapPath.c_str());

									// Message
									if (MessageBox (msg, "TileEdit", MB_YESNO|MB_ICONQUESTION)==IDNO)
										break;
								}
							}
						}
						if (type!=CTile::bitmapCount)
							break;
					}
				}

				// For all tiles, check we can change the path
				for (uint noise=1; noise<tileBank.getDisplacementMapCount (); noise++)
				{
					// Bitmap string
					const char *bitmapPath=tileBank.getDisplacementMap (noise);

					// not empty ?
					if (strcmp (bitmapPath, "")!=0)
					{
						// Check the path
						if (CheckPath (bitmapPath, path)==false)
						{
							// Bad path
							goodPath=false;

							// Make a message
							sprintf (msg, "Path '%s' can't be found in bitmap '%s'. Continue ?", path, bitmapPath);

							// Message
							if (MessageBox (msg, "TileEdit", MB_YESNO|MB_ICONQUESTION)==IDNO)
								break;
						}
					}
				}

				// Good path ?
				if (goodPath)
				{
					// Ok change the path

					// For all tiles, check we can change the path
					for (int tiles=0; tiles<tileBank.getTileCount(); tiles++)
					{
						// Get tile xref
						int tileSet;
						int number;
						CTileBank::TTileType type;
						tileBank.getTileXRef (tiles, tileSet, number, type);

						// Is tile used ?
						if (tileSet!=-1)
						{
							// 3 types of bitmaps
							for (int type=CTile::diffuse; type<CTile::bitmapCount; type++)
							{
								// Bitmap string
								std::string bitmapPath=tileBank.getTile(tiles)->getRelativeFileName ((CTile::TBitmap)type);

								// not empty ?
								if (bitmapPath!="")
								{
									// Remove the absolute path
									bool res=RemovePath (bitmapPath, path);
									nlassert (res);

									// Set the bitmap
									tileBank.getTile(tiles)->setFileName ((CTile::TBitmap)type, bitmapPath);
								}
							}
						}	
					}

					// For all tiles, check we can change the path
					for (uint noise=1; noise<tileBank.getDisplacementMapCount (); noise++)
					{
						// Bitmap string
						std::string bitmapPath=tileBank.getDisplacementMap (noise);

						// not empty ?
						if (bitmapPath!="")
						{
							// Remove the absolute path
							bool res=RemovePath (bitmapPath, path);
							nlassert (res);

							// Set the bitmap
							tileBank.setDisplacementMap (noise, bitmapPath.c_str());
						}
					}
				}
				else
					// Info message
					MessageBox ("Can't set the path.", "TileEdit", MB_OK|MB_ICONINFORMATION);
			}


			// Good path ?
			if (goodPath)
			{
				// Change the abs path of the bank
				tileBank.setAbsPath (path);

				// Change the bouton text
				GetDlgItem (IDC_PATH)->SetWindowText (path);
			}
		}

		// Remove path from all tiles
		//tileBank
	}
}

void SelectionTerritoire::OnExport() 
{
	// TODO: Add your control notification handler code here
	static char BASED_CODE szFilter[] = 
		"NeL tile bank files (*.smallbank)|*.smallbank|All Files (*.*)|*.*||";
 	CFileDialog sFile(false, "*.smallbank", DefautPath+"*.smallbank", 0, szFilter, this);
	if (sFile.DoModal()==IDOK)
	{
		// Copy the bank
		CTileBank copy=tileBank;

		// Remove unused data
		copy.cleanUnusedData ();

		// Save it
		Save (sFile.GetPathName(), copy);
	}
}

void SelectionTerritoire::OnChooseVeget() 
{
	// Create a choose veget dialog
	CListBox *list=(CListBox*)GetDlgItem(IDC_TILE_SET);
	int index=list->GetCurSel();
	if (index!=LB_ERR) 
	{
		CChooseVegetSet chooseVeget ( this, tileBank.getTileSet (index)->getTileVegetableDescFileName () );
		if (chooseVeget.DoModal ()==IDOK)
		{
			tileBank.getTileSet (index)->setTileVegetableDescFileName (chooseVeget.FileName);
		}
	}
	else
	{
		MessageBox("No tilesset selected","Error",MB_ICONERROR);
	}
}

void SelectionTerritoire::OnChangeSurfaceData() 
{
	UpdateData ();

	CListBox *list=(CListBox*)GetDlgItem(IDC_TILE_SET);
	int index=list->GetCurSel();
	if (index!=LB_ERR) 
	{
		tileBank.getTileSet (index)->SurfaceData = (uint32)SurfaceData;
	}
}

void SelectionTerritoire::OnSelchangeTileSet() 
{
	UpdateData ();

	CListBox *list=(CListBox*)GetDlgItem(IDC_TILE_SET);
	int index=list->GetCurSel();

	// Enable the surface button
	SurfaceDataCtrl.EnableWindow (index!=LB_ERR);

	if (index!=LB_ERR) 
	{	
		SurfaceData = tileBank.getTileSet (index)->SurfaceData;
	}
	else
	{
		SurfaceDataCtrl.EnableWindow (FALSE);
	}

	UpdateData (FALSE);
}

BOOL SelectionTerritoire::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	OnSelchangeTileSet ();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
