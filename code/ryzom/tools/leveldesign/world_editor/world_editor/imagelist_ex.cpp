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
#include "nel/misc/types_nl.h"
#include "nel/misc/common.h"
#include "resource.h"
#include "imagelist_ex.h"

#include "set"

using namespace std;
using namespace NLMISC;

#pragma warning (disable : 4786)

BOOL CALLBACK EnumResLangProc(HMODULE hModule, LPCSTR lpszType, LPCSTR lpszName, WORD wIDLanguage, 
							  LONG_PTR lParam)
{
	set<HRSRC> *iconNames = (set<HRSRC>*)lParam;
	
	HRSRC hResInfo = FindResourceEx(hModule, lpszType, lpszName, wIDLanguage); 
 
	iconNames->insert (hResInfo);

	return FALSE;
}

BOOL CALLBACK EnumResNameProc (HINSTANCE hModule, LPCTSTR lpszType, LPTSTR lpszName, LONG lParam)
{

	EnumResourceLanguages (hModule, lpszType, lpszName, EnumResLangProc, lParam);

	return TRUE;
}

void CImageListEx::addResourceIcon (HINSTANCE hinst, int resource)
{
	// Image index
	int index = ImageList.GetImageCount ();

	// Resize the list
	ImageList.SetImageCount (index+1);

	// Get the size
	IMAGEINFO imageInfo;
	if (ImageList.GetImageInfo( index, &imageInfo))
	{
		// Size
		int width = imageInfo.rcImage.right - imageInfo.rcImage.left;
		int height = imageInfo.rcImage.bottom - imageInfo.rcImage.top;
		
		// Load the icon
		HICON handle = (HICON) LoadImage (hinst, MAKEINTRESOURCE(resource), IMAGE_ICON, width, height, LR_COLOR);
		if (handle)
		{
			// Copy the icon
			index = ImageList.Replace( index, handle);
		
			// Add in the map
			_IconMapInt.insert (std::map<int, int>::value_type (resource, index));

			// Release the icon
			DestroyIcon (handle);
		}
	}
}

void CImageListEx::addResourceIcon (const char *filename)
{
	// Image index
	int index = ImageList.GetImageCount ();

	// Resize the list
	ImageList.SetImageCount (index+1);

	// Get the size
	IMAGEINFO imageInfo;
	if (ImageList.GetImageInfo( index, &imageInfo))
	{
		// Size
		int width = imageInfo.rcImage.right - imageInfo.rcImage.left;
		int height = imageInfo.rcImage.bottom - imageInfo.rcImage.top;
		
		// Load the icon
		HICON handle = (HICON) LoadImage (NULL, filename, IMAGE_ICON, width, height, LR_COLOR|LR_LOADFROMFILE);
		if (handle)
		{
			// Copy the icon
			index = ImageList.Replace( index, handle);
		
			// Add in the map
			char name[MAX_PATH];
			_splitpath (filename, NULL, NULL, name, NULL);
			string llwr = strlwr (string (name));
			_IconMapString.insert (std::map<string, int>::value_type (llwr, index));

			// Release the icon
			DestroyIcon (handle);
		}
	}
}

void CImageListEx::create (int width, int height)
{
	ImageList.Create (16, 16, TRUE, 6, 10);	
	ImageList.SetBkColor( CLR_NONE );
}

int CImageListEx::getImage (int resource) const
{
	std::map<int, int>::const_iterator ite = _IconMapInt.find (resource);
	if (ite == _IconMapInt.end())
		return -1;
	else
		return ite->second;
}

int CImageListEx::getImage (const char *filename) const
{
	char name[MAX_PATH];
	_splitpath (filename, NULL, NULL, name, NULL);
	string llwr = strlwr (string (name));
	std::map<string, int>::const_iterator ite = _IconMapString.find (llwr);
	if (ite == _IconMapString.end())
		return -1;
	else
		return ite->second;
}
