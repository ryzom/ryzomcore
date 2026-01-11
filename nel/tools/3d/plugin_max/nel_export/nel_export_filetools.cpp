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

#include "std_afx.h"

#include "nel_export.h"

#include "nel/misc/path.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

//--------------------------------------------------------------------------------------------------------------

ULONG CNelExport::SelectFileForSave(HWND Parent, const TCHAR* Title, const TCHAR* Mask, std::string &FileName)
{
	TCHAR curdir[MAX_PATH];
	TCHAR fname[MAX_PATH];

	std::string path, filename;

	if (!FileName[0])
	{
		path = NLMISC::CPath::getCurrentPath();
	}
	else
	{
		path = NLMISC::CFile::getPath(FileName);

		if (path.empty())
		{
			path = NLMISC::CPath::getCurrentPath();
		}

		filename = NLMISC::CFile::getFilename(FileName);
	}

	// copy path and filename to temporary buffers
	_tcscpy_s(curdir, MAX_PATH, MaxTStrFromUtf8(path).data());
	_tcscpy_s(fname, MAX_PATH, MaxTStrFromUtf8(filename).data());

	OPENFILENAME ofn;
	memset(&ofn,0,sizeof(OPENFILENAME));
	ofn.lStructSize	 		=	sizeof ( OPENFILENAME );
	ofn.hwndOwner			=	Parent;
	ofn.hInstance			=	GetModuleHandle(NULL);;
	ofn.lpstrFilter	  		=	Mask;
	ofn.lpstrCustomFilter	=	NULL;
	ofn.nFilterIndex	  	=	0;
	ofn.lpstrFile		    =	fname;
	ofn.nMaxFile		    =	500;
	ofn.lpstrTitle	    	=	Title;
	ofn.Flags			    =	OFN_HIDEREADONLY | OFN_LONGNAMES | OFN_OVERWRITEPROMPT;
	ofn.lpstrDefExt		  	=	_T("*");
	ofn.lpstrInitialDir		=	curdir;
	BOOL r = GetSaveFileName ( &ofn );

	FileName = MCharStrToUtf8(fname);

	return r;
}

//--------------------------------------------------------------------------------------------------------------

ULONG CNelExport::SelectDir(HWND Parent, const TCHAR* Title, std::string &Path)
{
	TCHAR str[MAX_PATH];
	_tcscpy_s(str, MAX_PATH, MaxTStrFromUtf8(Path).data());

	BROWSEINFO	bi;
	bi.hwndOwner=Parent;
	bi.pidlRoot=NULL;
	bi.pszDisplayName=str;
	bi.lpszTitle=Title;
	bi.ulFlags=0;
	bi.lpfn=0;
	bi.lParam=0;
	bi.iImage=0;

	PIDLIST_ABSOLUTE pidl = SHBrowseForFolder(&bi);

	if (!SHGetPathFromIDList(pidl,str) ) 
	{
		return 0;
	}

	Path = MCharStrToUtf8(str);

	return 1;
}
