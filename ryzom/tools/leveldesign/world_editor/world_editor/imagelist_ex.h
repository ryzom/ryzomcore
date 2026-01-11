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

#ifndef IMAGELIST_H_INCLUDED
#define IMAGELIST_H_INCLUDED

// Super Imagelist class
class CImageListEx
{
public:

	// Interface

	// Create the image list
	void			create (int width, int height);

	// Add resource icon 
	void			addResourceIcon (HINSTANCE hinst, int resource);

	// Add resource icon 
	void			addResourceIcon (const char *filename);

	// Get an icon resource image by icon name
	int				getImage (int resource) const;

	// Get an icon resource image by icon name
	int				getImage (const char *name) const;

public:
	// The image list
	CImageList		ImageList;

private:
	// The second icon map
	std::map<int, int>			_IconMapInt;
	std::map<std::string, int>	_IconMapString;
};

#endif // IMAGELIST_H_INCLUDED
