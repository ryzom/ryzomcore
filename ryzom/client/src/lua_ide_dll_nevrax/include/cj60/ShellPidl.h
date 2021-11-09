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

#ifndef __SHELLPIDL_H__
#define __SHELLPIDL_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ShellPidl.h : header file
//

#include <shlobj.h>

/////////////////////////////////////////////////////////////////////////////
// CShellPidl window
//
// This source is part of CShellTree - Selom Ofori
// 
// Version: 1.02 (any previously unversioned copies are older/inferior
//
// This code is free for all to use. Mutatilate it as much as you want
// See MFCENUM sample from microsoft

class AFX_EXT_CLASS CShellPidl
{
public:
	// STRUCTURES
	typedef struct tagLVID
	{
		LPSHELLFOLDER lpsfParent;
		LPITEMIDLIST  lpi;
		ULONG         ulAttribs;
	} LVITEMDATA, *LPLVITEMDATA;

	typedef struct tagID
	{
		LPSHELLFOLDER lpsfParent;
		LPITEMIDLIST  lpi;
		LPITEMIDLIST  lpifq;
	} TVITEMDATA, *LPTVITEMDATA;

public:

    // Functions that deal with PIDLs
    LPITEMIDLIST	ConcatPidls(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
    LPITEMIDLIST	GetFullyQualPidl(LPSHELLFOLDER lpsf, LPITEMIDLIST lpi);
    LPITEMIDLIST	CopyITEMID(LPMALLOC lpMalloc, LPITEMIDLIST lpi);
    BOOL			GetName(LPSHELLFOLDER lpsf, LPITEMIDLIST  lpi, DWORD dwFlags, LPSTR lpFriendlyName);
    LPITEMIDLIST	CreatePidl(UINT cbSize);
    UINT			GetSize(LPCITEMIDLIST pidl);
    LPITEMIDLIST	Next(LPCITEMIDLIST pidl);

	// Utility Functions
	BOOL	DoTheMenuThing(HWND hwnd, LPSHELLFOLDER lpsfParent, LPITEMIDLIST  lpi, LPPOINT lppt);
	int		GetItemIcon(LPITEMIDLIST lpi, UINT uFlags);
};

#endif // __SHELLTREE_H__
