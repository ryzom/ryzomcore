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

#include "std_afx.h"
#include "object_viewer.h"
#include "animation_set_dlg.h"

#include "nel/misc/file.h"
#include "nel/3d/track_keyframer.h"


using namespace NLMISC;
using namespace NL3D;

/////////////////////////////////////////////////////////////////////////////
// CAnimationSetDlg dialog


CAnimationSetDlg::CAnimationSetDlg(CObjectViewer* objView, CWnd* pParent /*=NULL*/)
	: CDialog(CAnimationSetDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAnimationSetDlg)
	UseMixer = 0;
	//}}AFX_DATA_INIT

	_ObjView=objView;
}


void CAnimationSetDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAnimationSetDlg)
	DDX_Control(pDX, IDC_EDITED_OBJECT, EditedObject);
	DDX_Control(pDX, IDC_PLAYLIST, PlayList);
	DDX_Control(pDX, IDC_TREE2, SkelTree);
	DDX_Control(pDX, IDC_TREE, Tree);
	DDX_Radio(pDX, IDC_USE_LIST, UseMixer);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAnimationSetDlg, CDialog)
	//{{AFX_MSG_MAP(CAnimationSetDlg)
	ON_BN_CLICKED(IDC_ADD_ANIMATION, OnAddAnimation)
	ON_BN_CLICKED(IDC_RESET, OnReset)
	ON_BN_CLICKED(IDC_ADD_SKEL_WT, OnAddSkelWt)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_LIST_INSERT, OnListInsert)
	ON_BN_CLICKED(IDC_LIST_UP, OnListUp)
	ON_BN_CLICKED(IDC_LIST_DOWN, OnListDown)
	ON_BN_CLICKED(IDC_LIST_DELETE, OnListDelete)
	ON_BN_CLICKED(IDC_SET_ANIM_LENGTH, OnSetAnimLength)
	ON_BN_CLICKED(IDC_USE_LIST, OnUseList)
	ON_BN_CLICKED(IDC_USE_MIXER, OnUseMixer)
	ON_CBN_SELCHANGE(IDC_EDITED_OBJECT, OnSelchangeEditedObject)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAnimationSetDlg message handlers

// ***************************************************************************

void CAnimationSetDlg::OnAddAnimation () 
{
	uint instance = _ObjView->getEditedObject ();
	if (instance != 0xffffffff)
	{
		// Create a dialog
		static char BASED_CODE szFilter[] = 
			"NeL Animation Files (*.anim)\0*.anim\0"
			"All Files (*.*)\0*.*\0\0";

		// Filename buffer
		char buffer[65535];
		buffer[0]=0;

		OPENFILENAME openFile;
		memset (&openFile, 0, sizeof (OPENFILENAME));
		openFile.lStructSize = sizeof (OPENFILENAME);
		openFile.hwndOwner = this->m_hWnd;
		openFile.lpstrFilter = szFilter;
		openFile.nFilterIndex = 0;
		openFile.lpstrFile = buffer;
		openFile.nMaxFile = 65535;
		openFile.Flags = OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT|OFN_ALLOWMULTISELECT|OFN_ENABLESIZING|OFN_EXPLORER;
		openFile.lpstrDefExt = "*.anim";
		

		if (GetOpenFileName(&openFile))
		{
			// Open the file
			try
			{
				// Filename pointer
				char *c=buffer;

				// Read the path
				CString path = buffer;
				if (path.GetLength()>openFile.nFileOffset)
				{
					// Double zero at the end
					c[path.GetLength()+1]=0;

					// Path is empty
					path = "";
				}
				else
				{
					// Adda slash
					path += "\\";

					// Look for the next string
					while (*(c++)) {}
				}

				// For each file selected
				while (*c)
				{
					// File name
					char filename[256];
					char *ptr=filename;

					// Read a file name
					while (*c)
					{
						*(ptr++)=*(c++);
					}
					*ptr=0;
					c++;

					// File name
					CString name = path + filename;

					// Load the animation
					_ObjView->loadAnimation (name, instance);

					// Touch the channel mixer
					_ObjView->reinitChannels ();

					// Update 
					refresh (TRUE);
				}
			}
			catch (Exception& e)
			{
				MessageBox (e.what(), "NeL object viewer", MB_OK|MB_ICONEXCLAMATION);
			}
		}
	}
}

// ***************************************************************************

void CAnimationSetDlg::OnAddSkelWt() 
{
	// Instance number
	int instance = _ObjView->getEditedObject ();
	if (instance != CB_ERR)
	{
		// TODO: Add your control notification handler code here
		static char BASED_CODE szFilter[] = "NeL Skeleton Weight Template Files (*.swt)|*.swt|All Files (*.*)|*.*||";
		CFileDialog fileDlg( TRUE, ".swt", "*.swt", OFN_ALLOWMULTISELECT|OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter);
		if (fileDlg.DoModal()==IDOK)
		{
			// Open the file
			try
			{
				// Get first file
				POSITION pos=fileDlg.GetStartPosition( );
				while (pos)
				{
					// Get the name
					CString filename=fileDlg.GetNextPathName(pos);

					// Load the animation
					_ObjView->loadSWT (filename, instance);

					// Touch the channel mixer
					_ObjView->reinitChannels  ();

					// Update 
					refresh (TRUE);
				}
			}
			catch (Exception& e)
			{
				MessageBox (e.what(), "NeL object viewer", MB_OK|MB_ICONEXCLAMATION);
			}
		}
	}
}

// ***************************************************************************

void CAnimationSetDlg::OnReset () 
{
	// Instance
	uint instance = _ObjView->getEditedObject ();
	if (instance!=0xffffffff)
	{
		// Reset the channel mixer slots
		_ObjView->resetSlots (instance);
	}
}

// ***************************************************************************

void CAnimationSetDlg::refresh (BOOL update)
{
	if (update)
	{
		CDialog::UpdateData (update);

		// Clear the combo box
		EditedObject.ResetContent ();

		// Set edited object list
		uint i;
		for (i=0; i<_ObjView->getNumInstance (); i++)
		{
			char name[512];
			_splitpath (_ObjView->getInstance (i)->Saved.ShapeFilename.c_str(), NULL, NULL, name, NULL);
			EditedObject.InsertString (-1, name);
		}

		// Get edited object
		uint instance = _ObjView->getEditedObject ();

		// Selection combo box
		EditedObject.SetCurSel ((instance==0xffffffff)?-1:(int)instance);

		// Clear the tree
		Tree.DeleteAllItems ();

		// Clear the tree
		SkelTree.DeleteAllItems ();

		// Clear the playlist
		PlayList.ResetContent ();

		if (instance != 0xffffffff)
		{
			// Get the instance
			CInstanceInfo *object = _ObjView->getInstance (instance);

			// For all tracks in the animation
			uint i;
			for (i=0; i<object->Saved.AnimationFileName.size(); i++)
			{
				// Get the animation name
				char name[512];
				_splitpath (object->Saved.AnimationFileName[i].c_str(), NULL, NULL, name, NULL);

				// Get the animation pointer
				CAnimation *anim = object->AnimationSet.getAnimation (object->AnimationSet.getAnimationIdByName (name));

				// Insert an intem
				HTREEITEM item=Tree.InsertItem (name);
				Tree.SetItemData (item, i);
				nlassert (item!=NULL);

				// For all tracks in the animation
				std::set<std::string> setString;
				anim->getTrackNames (setString);
				std::set<std::string>::iterator ite=setString.begin();
				while (ite!=setString.end())
				{
					// Add this string
					HTREEITEM newItem = Tree.InsertItem (ite->c_str(), item);
					Tree.SetItemData (newItem, 0xffffffff);

					// Get the track
					ITrack *track=anim->getTrack (anim->getIdTrackByName (*ite));

					// Keyframer ?
					UTrackKeyframer *keyTrack=dynamic_cast<UTrackKeyframer *>(track);
					if (keyTrack)
					{
						// Get number of keys
						std::vector<TAnimationTime> keys;
						keyTrack->getKeysInRange (track->getBeginTime ()-1, track->getEndTime ()+1, keys);

						// Print track info
						char name[512];
						_snprintf (name, 512, "%s (%f - %f) %d keys", typeid(*track).name(), track->getBeginTime (), track->getEndTime (), keys.size());
						HTREEITEM keyItem = Tree.InsertItem (name, newItem);
						Tree.SetItemData (keyItem, 0xffffffff);
					}
					else
					{
						// Print track info
						char name[512];
						_snprintf (name, 512, "%s (%f - %f)", typeid(*track).name(), track->getBeginTime (), track->getEndTime ());
						HTREEITEM keyItem = Tree.InsertItem (name, newItem);
						Tree.SetItemData (keyItem, 0xffffffff);
					}

					ite++;
				}
			}

			// For all tracks in the animation
			for (i=0; i<object->Saved.SWTFileName.size(); i++)
			{
				// Get the animation name
				char name[512];
				_splitpath (object->Saved.SWTFileName[i].c_str(), NULL, NULL, name, NULL);

				// Get the animation pointer
				CSkeletonWeight *swt = object->AnimationSet.getSkeletonWeight (object->AnimationSet.getSkeletonWeightIdByName (name));

				// Insert an intem
				HTREEITEM item=SkelTree.InsertItem (name);
				nlassert (item!=NULL);

				// Get number of node in this skeleton weight
				uint numNode=swt->getNumNode ();

				// Add the nodein the tree
				for (uint n=0; n<numNode; n++)
				{
					char percent[512];
					sprintf (percent, "%s (%f%%)", swt->getNodeName (n).c_str(), swt->getNodeWeight(n)*100);

					// Add this string
					SkelTree.InsertItem (percent, item);
				}
			}

			// For all tracks in the playlist
			for (i=0; i<object->Saved.PlayList.size(); i++)
			{
				// Insert an intem
				int item=PlayList.InsertString (-1, object->Saved.PlayList[i].c_str());
				nlassert (item!=LB_ERR);
			}
		}

		CDialog::UpdateData (FALSE);
	}
	else
	{
		CDialog::UpdateData (TRUE);

		// Get edited object
		uint instance = _ObjView->getEditedObject ();
		if (instance != 0xffffffff)
		{
			// Get the instance
			CInstanceInfo *object = _ObjView->getInstance (instance);

			// Clear the playlist
			object->Saved.PlayList.resize (PlayList.GetCount ());

			// For all tracks in the playlist
			uint i;
			for (i=0; i<object->Saved.PlayList.size(); i++)
			{
				// Insert an intem
				char text[512];
				PlayList.GetText( i, text);
				object->Saved.PlayList[i] = text;
			}

			CDialog::UpdateData (update);
		}
	}

	_ObjView->refreshAnimationListeners();
}

// ***************************************************************************

void CAnimationSetDlg::OnDestroy() 
{
	setRegisterWindowState (this, REGKEY_OBJ_VIEW_ANIMATION_SET_DLG);

	CDialog::OnDestroy();
}

// ***************************************************************************

void CAnimationSetDlg::OnListInsert() 
{
	// Get selected animation
	HTREEITEM item = Tree.GetSelectedItem ();
	if (item && (Tree.GetItemData (item)!=0xffffffff))
	{
		// Insert the string
		int itemList = PlayList.InsertString (-1, Tree.GetItemText(item));
		PlayList.SetItemData (itemList, Tree.GetItemData (item));

		// Reselect the item
		Tree.SelectItem (item);
	}
	
	// Update back
	refresh (FALSE);
}

// ***************************************************************************

void CAnimationSetDlg::OnListUp() 
{
	// Get selected item
	int sel = PlayList.GetCurSel ();
	if ((sel != LB_ERR) && (sel>0))
	{
		// Backup the string
		CString text;
		PlayList.GetText (sel, text);
		DWORD_PTR data = PlayList.GetItemData (sel);

		// Remove the node
		PlayList.DeleteString (sel);

		// Insert the string
		int pos = PlayList.InsertString (sel-1, text);
		PlayList.SetItemData (pos, data);
		PlayList.SetCurSel (pos);
	}
	
	// Update back
	refresh (FALSE);
}

// ***************************************************************************

void CAnimationSetDlg::OnListDown() 
{
	// Get selected item
	int sel = PlayList.GetCurSel ();
	if ((sel != LB_ERR) && (sel<PlayList.GetCount()-1))
	{
		// Backup the string
		CString text;
		PlayList.GetText (sel, text);
		DWORD_PTR data = PlayList.GetItemData (sel);

		// Remove the node
		PlayList.DeleteString (sel);

		// Insert the string
		int pos = PlayList.InsertString (sel+1, text);
		PlayList.SetItemData (pos, data);
		PlayList.SetCurSel (pos);
	}
	
	// Update back
	refresh (FALSE);
}

// ***************************************************************************

void CAnimationSetDlg::OnListDelete() 
{
	// Get selected item
	int sel = PlayList.GetCurSel ();
	if (sel != LB_ERR)
	{
		// Remove the node
		PlayList.DeleteString (sel);

		if (sel>=PlayList.GetCount ())
			sel--;
		if (sel>=0)
			PlayList.SetCurSel (sel);
	}
	
	// Update back
	refresh (FALSE);
}

// ***************************************************************************

void CAnimationSetDlg::OnSetAnimLength() 
{
	// Longest animation
	float longestLength = 0;

	// For each instance
	for (uint instance=0; instance<_ObjView->_ListInstance.size(); instance++)
	{
		// Ref on the instance
		CInstanceInfo &inst = *_ObjView->_ListInstance[instance];

		// There is some anim ?
		if (inst.Saved.PlayList.size())
		{
			// Calculate the length
			float length = 0;

			// For each animation in the list
			for (uint i=0; i<(uint)inst.Saved.PlayList.size(); i++)
			{
				// Get the animation
				CAnimation *anim = inst.AnimationSet.getAnimation (inst.AnimationSet.getAnimationIdByName (inst.Saved.PlayList[i]));

				// Add the length
				length += anim->getEndTime () - anim->getBeginTime ();
			}

			if (length>longestLength)
				longestLength=length;
		}
	}

	// Adjuste length
	if (longestLength>0)
		_ObjView->setAnimTime (0, longestLength * _ObjView->getFrameRate ());
}

// ***************************************************************************

void CAnimationSetDlg::OnUseList() 
{
	// Disable channels
	_ObjView->enableChannels ();
}

// ***************************************************************************

void CAnimationSetDlg::OnUseMixer() 
{
	// Enable channels
	_ObjView->enableChannels ();
}

// ***************************************************************************


void CAnimationSetDlg::OnSelchangeEditedObject() 
{
	UpdateData ();

	// Selection combo box
	int selection = EditedObject.GetCurSel ();
	_ObjView->setEditedObject ((selection==CB_ERR)?0xffffffff:selection);

	refresh (TRUE);
	_ObjView->getSlotDlg ()->refresh (TRUE);
}
