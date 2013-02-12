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

#include "file_tree_view.h"

using namespace std;

CFileTreeCtrl::CFileTreeCtrl()
{
	//{{AFX_DATA_INIT(CFileTreeCtrl)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	SHFILEINFO  sfi;
	_ImageListSmall = (HIMAGELIST)SHGetFileInfo( TEXT("C:\\"),0,&sfi,sizeof(SHFILEINFO),SHGFI_SYSICONINDEX | SHGFI_SMALLICON);
	_ArrangeMode = ByName;
	_HWndNotify = NULL;
}

CFileTreeCtrl::~CFileTreeCtrl()
{
}
 
BEGIN_MESSAGE_MAP(CFileTreeCtrl, CWnd)
	//{{AFX_MSG_MAP(CFileTreeCtrl)
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFileTreeCtrl message handlers

bool CFileTreeCtrl::create( const RECT& rect, CWnd* pParentWnd, UINT nID )
{
	// Register window class
	LPCTSTR className = AfxRegisterWndClass( 0 ); 
	// Create this window

	if (CWnd::Create (className, "empty", WS_CHILD, rect, pParentWnd, nID ))


#if defined(NL_COMP_VC) && NL_COMP_VC_VERSION >= 80
		if (_TreeCtrl.CreateEx (WS_EX_CLIENTEDGE, /*_T("SysTreeView32"), "",*/ TVS_HASLINES|TVS_LINESATROOT|TVS_HASBUTTONS|TVS_EDITLABELS|WS_CHILD|WS_TABSTOP, rect, this, 0))
#else		
		if (_TreeCtrl.CreateEx (WS_EX_CLIENTEDGE, _T("SysTreeView32"), "", TVS_HASLINES|TVS_LINESATROOT|TVS_HASBUTTONS|TVS_EDITLABELS|WS_CHILD|WS_TABSTOP, rect, this, 0))
#endif
		//if (_TreeCtrl.Create( TVS_HASLINES|TVS_LINESATROOT|TVS_HASBUTTONS|TVS_EDITLABELS, rect, this, 0))
			return true;

	// Set the directory
	setRootDirectory (_RootDirectory.c_str());

	return false;
}

struct CTreeItemInfo
{
	CTreeItemInfo() 
	{
		pidlSelf=NULL;
		pidlFullyQual=NULL;
		pParentFolder=NULL;
		dwFlags=NULL;
	}
	ITEMIDLIST*   pidlSelf;
	ITEMIDLIST*   pidlFullyQual;
	IShellFolder* pParentFolder;
	DWORD		  dwFlags;
	std::string	  displayName;
};

bool CFileTreeCtrl::setRootDirectory (const char *dir)
{
	// Save the string
	_RootDirectory = dir;
	if (!_RootDirectory.empty ())
	{
		// Is a window ?
		if (IsWindow (m_hWnd))
		{
			// Clrear the tree
			_TreeCtrl.DeleteAllItems ();

			IShellFolder* pDesktop;
			ITEMIDLIST*   pidl;
			ITEMIDLIST*   pidlDir;
			TV_ITEM tvItem={0};
			TV_INSERTSTRUCT   tvInsert={0};

			// Create tmp image list
			CImageList *imageList = CImageList::FromHandle( _ImageListSmall );

			_TreeCtrl.SetImageList( imageList, TVSIL_NORMAL);

			if(SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOP, &pidl)))
			{
			   if(FAILED(SHGetDesktopFolder(&pDesktop)))
			   {
				   //TODO: free the pidl through IMalloc
				   return false;
			   }

			   CTreeItemInfo* pItemInfo = new CTreeItemInfo;
			   if(pItemInfo == NULL)
			   {
				   //TODO: free the pidl through IMalloc
				   pDesktop->Release();
				   return false;
			   }

				// String null ?
				if (_RootDirectory == "")
				{
					pidlDir = pidl;
				}
				else
				{
					OLECHAR       olePath[MAX_PATH];
					ULONG         chEaten;
					ULONG         dwAttributes;
					HRESULT       hr;

					// IShellFolder::ParseDisplayName requires the file name be in
					// Unicode.
					MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, _RootDirectory.c_str(), -1, olePath, MAX_PATH);

					// Convert the path to an ITEMIDLIST.
					hr = pDesktop->ParseDisplayName(m_hWnd, NULL, olePath, &chEaten, &pidlDir, &dwAttributes);
					if (FAILED(hr))
					{
						return false;
					}
				}

			   pItemInfo->pidlSelf = pidlDir;
			   pItemInfo->pidlFullyQual = pidlDir;
			   tvItem.mask = TVIF_PARAM | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_CHILDREN;
			   tvItem.lParam= (LPARAM)pItemInfo;
			   tvItem.pszText = LPSTR_TEXTCALLBACK;
			   tvItem.iImage = tvItem.iSelectedImage = I_IMAGECALLBACK;
			   tvItem.cChildren = TRUE;
			   tvInsert.item = tvItem;
			   tvInsert.hInsertAfter = TVI_LAST;
			   HTREEITEM hItem = _TreeCtrl.InsertItem(&tvInsert);
			   _TreeCtrl.SelectItem (hItem);
			   _TreeCtrl.Expand (hItem, TVE_EXPAND);
			   pDesktop->Release();
			   return true;
			}
		}
	}
	return false;
}

BOOL CFileTreeCtrl::ShowWindow(int nCmdShow)
{
	BOOL res=CWnd::ShowWindow(nCmdShow);
	_TreeCtrl.ShowWindow(nCmdShow);
	return res;
}

int	CFileTreeCtrl::OnCreate  ( LPCREATESTRUCT lpCreateStruct )
{
	int toto=0;
	return CWnd::OnCreate  ( lpCreateStruct );
}

BOOL CFileTreeCtrl::OnNotify ( WPARAM wParam, LPARAM lParam, LRESULT* pResult )
{
	LPNMHDR pnmh = (LPNMHDR) lParam;
	// Tree ?
	if (wParam == 0)
	{
		switch (pnmh->code)
		{
		case TVN_GETDISPINFO:
			{
				  LPNMTVDISPINFO lpdi = (LPNMTVDISPINFO)pnmh;
				  CTreeItemInfo* pItemInfo = (CTreeItemInfo*)lpdi->item.lParam;
				  SHFILEINFO     sfi;
				  if (pItemInfo)
				  {
					  if(lpdi->item.mask & TVIF_TEXT)
						 {
						 if(SHGetFileInfo((LPCTSTR)pItemInfo->pidlFullyQual, 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_DISPLAYNAME))
							lstrcpy(lpdi->item.pszText, sfi.szDisplayName);
						 }
					  
					  if(lpdi->item.mask & TVIF_IMAGE)
						 {
						 if(SHGetFileInfo((LPCTSTR)pItemInfo->pidlFullyQual, 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_LINKOVERLAY))
							lpdi->item.iImage = sfi.iIcon;
						 }
					  if(lpdi->item.mask & TVIF_SELECTEDIMAGE)
						 {
						 if(SHGetFileInfo((LPCTSTR)pItemInfo->pidlFullyQual, 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_OPENICON))
							lpdi->item.iSelectedImage = sfi.iIcon;
						 }
				  }
			}
			break;
		case TVN_ITEMEXPANDING:
			{
				LPNMTREEVIEW   pnmtv = (LPNMTREEVIEW)pnmh;
				IShellFolder *pParentFolder;

				if(pnmtv->action == TVE_COLLAPSE)
					_TreeCtrl.Expand(pnmtv->itemNew.hItem, TVE_COLLAPSE | TVE_COLLAPSERESET);
				else if(pnmtv->action == TVE_EXPAND)
				{
					HCURSOR	 hCursor;
					TVITEM   tvItem = {0};
					tvItem.mask = TVIF_PARAM;
					tvItem.hItem = pnmtv->itemNew.hItem;
					if(!_TreeCtrl.GetItem(&tvItem))
						return FALSE;
					CTreeItemInfo* pItemInfo = (CTreeItemInfo*)tvItem.lParam;
					hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
					
					if(pItemInfo->pParentFolder == 0)
					{
						if(FAILED(SHGetDesktopFolder(&pParentFolder)))
							return FALSE;
						if (_RootDirectory != "")
						{
							SHGetDesktopFolder(&pParentFolder);
							if(FAILED(pParentFolder->BindToObject(pItemInfo->pidlSelf, NULL, IID_IShellFolder, (void**)&pParentFolder)))
								return 0;
						}
					}
					else if(FAILED(pItemInfo->pParentFolder->BindToObject(pItemInfo->pidlSelf, NULL, IID_IShellFolder, (void**)&pParentFolder)))
						return 0;
				
					_TreeCtrl.SetRedraw(FALSE);
					enumObjects (pnmtv->itemNew.hItem, pParentFolder, pItemInfo->pidlFullyQual);

					_TreeCtrl.SetRedraw(TRUE);
					pParentFolder->Release();
					SetCursor(hCursor);
				}		
			}
			break;
		case TVN_DELETEITEM:
			{
				LPNMTREEVIEW   pnmtv = (LPNMTREEVIEW)pnmh;
				IMalloc* pMalloc;
				CTreeItemInfo* pItemInfo = (CTreeItemInfo*)pnmtv->itemOld.lParam;
				if (pItemInfo)
				{
					if (SUCCEEDED(SHGetMalloc(&pMalloc)))
					{
						if (pItemInfo->dwFlags == 0)
						{
							pMalloc->Free(pItemInfo->pidlSelf);
							pMalloc->Release();
							if (pItemInfo->pParentFolder)
							{
								pItemInfo->pParentFolder->Release();
								pMalloc->Free(pItemInfo->pidlFullyQual);
							}
						}
					}
					delete pItemInfo;
				}
			}
			break;
		case NM_RCLICK:
			{
				TVHITTESTINFO  tvhti;
				GetCursorPos(&tvhti.pt);
				::ScreenToClient(_TreeCtrl, &tvhti.pt);
				tvhti.flags = LVHT_NOWHERE;
				TreeView_HitTest(_TreeCtrl, &tvhti);

				if(TVHT_ONITEM & tvhti.flags)
				{
					::ClientToScreen(_TreeCtrl, &tvhti.pt);
					doItemMenu(_TreeCtrl, tvhti.hItem , &tvhti.pt);
				}
			}
			break;
		case NM_DBLCLK:
			{
				// Get the item
				TVHITTESTINFO  tvhti;
				GetCursorPos(&tvhti.pt);
				::ScreenToClient(_TreeCtrl, &tvhti.pt);
				tvhti.flags = LVHT_NOWHERE;
				TreeView_HitTest(_TreeCtrl, &tvhti);

				if(TVHT_ONITEM & tvhti.flags)
				{
					if (_TreeCtrl.GetRootItem () != tvhti.hItem)
					{
						::ClientToScreen(_TreeCtrl, &tvhti.pt);
						doClick(_TreeCtrl, tvhti.hItem);
					}
				}
			}
			break;
		}
	}
	return CWnd::OnNotify ( wParam, lParam, pResult );
}

inline ITEMIDLIST* Pidl_GetNextItem(LPCITEMIDLIST pidl)
{
	if(pidl)
	{
	   return (ITEMIDLIST*)(BYTE*)(((BYTE*)pidl) + pidl->mkid.cb);
	}
	else
	   return NULL;
}

LPITEMIDLIST Pidl_Create(UINT cbSize)
{
	LPITEMIDLIST pidl = NULL;

	IMalloc* pMalloc;
	if(FAILED(SHGetMalloc(&pMalloc)))
		return false;

	pidl = (LPITEMIDLIST) pMalloc->Alloc(cbSize);
	if(pidl)
	   ZeroMemory(pidl, cbSize);

	pMalloc->Release();
return pidl;
}

UINT Pidl_GetSize(LPCITEMIDLIST pidl)
{
	UINT           cbTotal = 0;
	ITEMIDLIST*   pidlTemp = (ITEMIDLIST*) pidl;

	if(pidlTemp)
	{
		while(pidlTemp->mkid.cb)
		{
			cbTotal += pidlTemp->mkid.cb;
			pidlTemp = Pidl_GetNextItem(pidlTemp);
		}  

		// Requires a 16 bit zero value for the NULL terminator
		cbTotal += 2 * sizeof(BYTE);
	}

	return cbTotal;
}

LPITEMIDLIST Pidl_Concatenate(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
	LPITEMIDLIST   pidlNew;
	UINT           cb1, 
	cb2 = 0;
	if(pidl1)
		cb1 = Pidl_GetSize(pidl1) - (2 * sizeof(BYTE));

	cb2 = Pidl_GetSize(pidl2);
	pidlNew = Pidl_Create(cb1 + cb2);
	if(pidlNew)
	{
		if(pidl1)   
			CopyMemory(pidlNew, pidl1, cb1);

		CopyMemory(((LPBYTE)pidlNew) + cb1, pidl2, cb2);
	}
	return pidlNew;
}

int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CFileTreeCtrl* pfileTreeCtrl = (CFileTreeCtrl*)lParamSort;
	CTreeItemInfo* pItemInfo1 = (CTreeItemInfo*)lParam1;
	CTreeItemInfo* pItemInfo2 = (CTreeItemInfo*)lParam2;

	if (pItemInfo1 && pItemInfo2)
	{
		// File or directory ?
		if ( ( (pItemInfo1->dwFlags&SFGAO_FOLDER)!=0) && ( (pItemInfo2->dwFlags&SFGAO_FOLDER)==0) )
			return -1;
		if ( ( (pItemInfo1->dwFlags&SFGAO_FOLDER)==0) && ( (pItemInfo2->dwFlags&SFGAO_FOLDER)!=0) )
			return 1;

		// By file name ?
		if (pfileTreeCtrl->_ArrangeMode == CFileTreeCtrl::ByName)
			return stricmp (pItemInfo1->displayName.c_str(), pItemInfo2->displayName.c_str());
		if (pfileTreeCtrl->_ArrangeMode == CFileTreeCtrl::ByType)
		{
			char ext1[_MAX_EXT];
			_splitpath (pItemInfo1->displayName.c_str(), NULL, NULL, NULL, ext1);
			char ext2[_MAX_EXT];
			_splitpath (pItemInfo2->displayName.c_str(), NULL, NULL, NULL, ext2);
			int res = stricmp (ext1, ext2);
			if ( res == 0)
				return stricmp (pItemInfo1->displayName.c_str(), pItemInfo2->displayName.c_str());
			else
				return res;
		}
	}
	return 0;
}

bool CFileTreeCtrl::enumObjects(HTREEITEM hParentItem,IShellFolder* pParentFolder, ITEMIDLIST* pidlParent)
{
	IEnumIDList* pEnum;
	if(SUCCEEDED(pParentFolder->EnumObjects(NULL, SHCONTF_NONFOLDERS |SHCONTF_FOLDERS|SHCONTF_INCLUDEHIDDEN, &pEnum)))
	{
		ITEMIDLIST* pidl;
		DWORD  dwFetched = 1;
		TV_ITEM tvItem={0};
		TV_INSERTSTRUCT   tvInsert={0};
		bool inserted = false;
		while(SUCCEEDED(pEnum->Next(1, &pidl, &dwFetched)) && dwFetched)
		{
			CTreeItemInfo* pItemInfo = new CTreeItemInfo;
			pItemInfo->pidlSelf = pidl;
			pItemInfo->pidlFullyQual = Pidl_Concatenate(pidlParent,pidl);
			pParentFolder->AddRef();
			pItemInfo->pParentFolder = pParentFolder;
			ZeroMemory(&tvItem, sizeof(tvItem));
			tvItem.mask = TVIF_PARAM | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_CHILDREN;
			tvItem.pszText = LPSTR_TEXTCALLBACK;
			tvItem.iImage = tvItem.iSelectedImage = I_IMAGECALLBACK;
			tvItem.lParam= (LPARAM)pItemInfo;
			pItemInfo->dwFlags = SFGAO_LINK | SFGAO_HASSUBFOLDER | SFGAO_FOLDER | SFGAO_DISPLAYATTRMASK | SFGAO_CANRENAME;
			pParentFolder->GetAttributesOf(1, (LPCITEMIDLIST*)&pidl, &pItemInfo->dwFlags);

			// Convert display name in file system path
			char name[MAX_PATH];
			nlverify ( SHGetPathFromIDList ( pidl, name ) );

			// Save it
			pItemInfo->displayName = name;

			// Is a folder ?
			bool folder = (pItemInfo->dwFlags&SFGAO_FOLDER) !=0;

			// No CVS directory
			uint displayNameSize = pItemInfo->displayName.size ();
			string ext3 = pItemInfo->displayName.substr(displayNameSize-3);
			string ext4 = pItemInfo->displayName.substr(displayNameSize-4);
			string ext5 = pItemInfo->displayName.substr(displayNameSize-5);

			bool cvs = ext3 == "CVS" || ext4 == "CVS\\" || ext4 == "CVS/" ||
				ext4 == ".svn" || ext5 == ".svn\\" || ext5 == ".svn/";

/*			bool cvs = ( pItemInfo->displayName[displayNameSize-3] == 'C') &&
				(pItemInfo->displayName[displayNameSize-2] == 'V') &&
				(pItemInfo->displayName[displayNameSize-1] == 'S');
			if (!cvs)
			{
				cvs = ( pItemInfo->displayName[displayNameSize-4] == 'C') &&
					(pItemInfo->displayName[displayNameSize-3] == 'V') &&
					(pItemInfo->displayName[displayNameSize-2] == 'S') &&
					( (pItemInfo->displayName[displayNameSize-1] == '\\') || (pItemInfo->displayName[displayNameSize-1] == '/') );
			}*/

			// Continue ?
			if (!folder || !cvs)
			{
				// Filter
				uint filter;
				uint filterCount = _ExclusiveExtFilter.size ();

				// Get the extension
				uint pos = pItemInfo->displayName.rfind ('.');
				const char *extName = NULL;
				if (pos != string::npos)
					extName = pItemInfo->displayName.c_str ()+pos;

				if (!folder)
				for (filter=0; filter<filterCount; filter++)
				{
					if (extName)
					{
						if (stricmp (extName, _ExclusiveExtFilter[filter].c_str ()) == 0)
							break;
					}
				}

				// All exclusive filters ok ?
				if (folder || (filter < filterCount) || (filterCount == 0))
				{
					filterCount = _NegativeExtFilter.size ();

					if (!folder)
					for (filter=0; filter<filterCount; filter++)
					{
						if (extName)
						{
							if (stricmp (extName, _NegativeExtFilter[filter].c_str ()) == 0)
								break;
						}
					}

					// All negative filters ok ?
					if (folder || (filter == filterCount))
					{
						tvItem.cChildren = (pItemInfo->dwFlags & SFGAO_FOLDER) && ((pItemInfo->dwFlags & SFGAO_LINK) ==0);
						if(pItemInfo->dwFlags & SFGAO_SHARE)
						{
							tvItem.mask |= TVIF_STATE;
							tvItem.stateMask |= TVIS_OVERLAYMASK;
							tvItem.state |= INDEXTOOVERLAYMASK(1);
						}
						tvInsert.item = tvItem;
						tvInsert.hInsertAfter = TVI_LAST;
						tvInsert.hParent = hParentItem;
						_TreeCtrl.InsertItem(&tvInsert);
						dwFetched = 0;
						inserted = true;
					}
				}
			}
		}
		if (!inserted)
		{
			/*tvInsert.item.mask = LVIF_IMAGE;
			tvInsert.item.stateMask = 0;
			tvInsert.item.state = 0;
			tvInsert.item.mask = 0;
			tvInsert.item.pszText = 0;
			tvInsert.item.iImage = tvInsert.item.iSelectedImage = 10;
			tvInsert.item.lParam = NULL;
			tvInsert.item.cChildren = 0;
			tvInsert.hInsertAfter = TVI_LAST;
			tvInsert.hParent = hParentItem;
			_TreeCtrl.InsertItem(&tvInsert);*/
			/*HTREEITEM hItem = _TreeCtrl.InsertItem ("", hParentItem );
			_TreeCtrl.SetItemImage( hItem, 0xffffffff, 0xffffffff );*/
			/*memset (&tvItem, 0, sizeof (TVITEM));
			tvItem.hItem = hParentItem;
			nlverify (_TreeCtrl.GetItem( &tvItem ));
			tvItem.cChildren = 0;
			nlverify (_TreeCtrl.SetItem( &tvItem ));*/
		}

		// Sort the list
		if (_ArrangeMode != None)
		{
			TVSORTCB tvSort;
			tvSort.hParent = hParentItem;
			tvSort.lpfnCompare = CompareFunc;
			tvSort.lParam = (LPARAM)this;
			_TreeCtrl.SortChildrenCB( &tvSort );
		}

		pEnum->Release();
		return true;
	}
	return false;
}

void CFileTreeCtrl::doItemMenu (HWND hwndTreeView, HTREEITEM hItem, LPPOINT pptScreen)
{
	TVITEM   tvItem;

	ZeroMemory(&tvItem, sizeof(tvItem));
	tvItem.mask = TVIF_PARAM;
	tvItem.hItem = hItem;

	if(TreeView_GetItem(hwndTreeView, &tvItem))
	{
		HWND           hwndParent = ::GetParent(hwndTreeView);
	    HRESULT        hr;
	    CTreeItemInfo*     pInfo = (CTreeItemInfo*)tvItem.lParam;
	    IContextMenu   *pcm;
	    IShellFolder   *psfFolder = pInfo->pParentFolder;

		if(!psfFolder)
		{
			SHGetDesktopFolder(&psfFolder);
		}
		else
		{
			psfFolder->AddRef();
		}

		if(psfFolder)
		{
		hr = psfFolder->GetUIObjectOf(   hwndParent, 
                                       1, 
                                       (LPCITEMIDLIST*)&pInfo->pidlSelf,
                                       IID_IContextMenu, 
                                       NULL, 
                                       (LPVOID*)&pcm);

		if(SUCCEEDED(hr))
        {
			HMENU hPopup;

			hPopup = CreatePopupMenu();
			if(hPopup)
			{
	            hr = pcm->QueryContextMenu(hPopup, 0, 1, 0x7fff, CMF_NORMAL | CMF_EXPLORE);

				if(SUCCEEDED(hr))
				{
					IContextMenu2* pcm2;
					pcm->QueryInterface(IID_IContextMenu2, (LPVOID*)&pcm2);

					UINT  idCmd;

					idCmd = TrackPopupMenu( hPopup, 
                                       TPM_LEFTALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON, 
                                       pptScreen->x,
                                       pptScreen->y,
                                       0,
                                       hwndParent,
                                       NULL);
            
					if(pcm2)
					{
						pcm2->Release();
						pcm2 = NULL;
					}

					if(idCmd)
					{
						CMINVOKECOMMANDINFO  cmi;
					  cmi.cbSize = sizeof(CMINVOKECOMMANDINFO);
					  cmi.fMask = 0;
					  cmi.hwnd = hwndParent;
					  cmi.lpVerb = (LPCSTR)(INT_PTR)(idCmd - 1);
					  cmi.lpParameters = NULL;
					  cmi.lpDirectory = NULL;
					  cmi.nShow = SW_SHOWNORMAL;
					  cmi.dwHotKey = 0;
					  cmi.hIcon = NULL;
					  hr = pcm->InvokeCommand(&cmi);
                  }
               }
            }
	         pcm->Release();
		}
		psfFolder->Release();
      }
   }
}

void CFileTreeCtrl::doClick (HWND hwndTreeView, HTREEITEM hItem)
{
	TVITEM   tvItem;

	ZeroMemory(&tvItem, sizeof(tvItem));
	tvItem.mask = TVIF_PARAM;
	tvItem.hItem = hItem;

	if(TreeView_GetItem(hwndTreeView, &tvItem))
	{
		HWND           hwndParent = ::GetParent(hwndTreeView);
	    CTreeItemInfo*     pItemInfo = (CTreeItemInfo*)tvItem.lParam;

		// Notify parent
		if (pItemInfo)
			if (_HWndNotify)
				::SendMessage (_HWndNotify, WM_FILE_TREE_VIEW_LDBLCLICK, (pItemInfo->dwFlags&SFGAO_FOLDER)!=0, _NotifyParam);
	}
}

void CFileTreeCtrl::setArrangeMode (TArrange arrangeMode)
{
	_ArrangeMode = arrangeMode;

	// Set the directory
	setRootDirectory (_RootDirectory.c_str());
}

void CFileTreeCtrl::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);
	
	CRect sz;
	GetClientRect(sz);
	//sz.DeflateRect(5,0);
	if (IsWindow (_TreeCtrl))
		_TreeCtrl.MoveWindow(sz);
}

void CFileTreeCtrl::addExclusiveExtFilter (const char *ext)
{
	_ExclusiveExtFilter.push_back (ext);
}

void CFileTreeCtrl::addNegativeExtFilter (const char *ext)
{
	_NegativeExtFilter.push_back (ext);
}

void CFileTreeCtrl::clearExtFilters ()
{
	_ExclusiveExtFilter.clear ();
	_NegativeExtFilter.clear ();
}

void CFileTreeCtrl::setNotifyWindow (HWND hWnd, uint32 param)
{
	_HWndNotify = hWnd;
	_NotifyParam = param;
}

bool CFileTreeCtrl::getCurrentFilename (std::string &result)
{
	HTREEITEM curSel = _TreeCtrl.GetSelectedItem ();
	if (curSel)
	{
		CString str = _TreeCtrl.GetItemText (curSel);
		result = str;
		return true;
	}
	return false;
}

void CFileTreeCtrl::OnSetFocus(CWnd* pOldWnd) 
{
	CWnd::OnSetFocus(pOldWnd);
	
	_TreeCtrl.SetFocus ();
}

bool CFileTreeCtrl::haveFocus ()
{
	return _TreeCtrl.GetFocus () == &_TreeCtrl;
}
