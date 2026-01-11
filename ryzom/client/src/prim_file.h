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



#ifndef CL_PRIM_FILE_H
#define CL_PRIM_FILE_H

/////////////
// INCLUDE //
/////////////
// Ligo
#include "nel/ligo/primitive.h"

/// Prim file manager
class CPrimFileMgr
{
public:

	/// Cstor
	CPrimFileMgr ();

	/// Release the prim file manager
	void	release (NL3D::UDriver &driver);

	/// Load next prim file
	void	loadNext ()
	{
		if (_ShowPrim)
			load (_PrimFile+1);
	}

	/// Load previous prim file
	void	loadPrevious ()
	{
		if (_ShowPrim)
			load (_PrimFile-1);
	}

	/// Display the prim file
	void	display (NL3D::UDriver &driver);

	/// Toggle show / hide
	void	toggleShowHide ()
	{
		_ShowPrim ^= true;
	}

	/// Change color
	void	changeColor ();

	/// Current primitive name, "" if no primitive selected
	std::string	getCurrentPrimitive () const;

private:
	// Load a new prim file
	void	load (sint primFileIndex);

	// Change material color
	void	changeColor (uint &currentColor);

	// Draw a 3d text
	static void draw3dText (const NLMISC::CVector &pos, NL3D::UCamera cam, const char *text);

	// Adjuste point height
	static void adjustPosition (NLMISC::CVector &pos);


	// Are we in Show prim mode?
	bool						_ShowPrim;

	// Loaded ?
	bool						_Loaded;

	// The selected prim file
	sint						_PrimFile;

	// The current prim file pointer
	NLLIGO::CPrimRegion			_PrimRegion;

	// The current material
	NL3D::UMaterial				_Material;
};

extern CPrimFileMgr				PrimFiles;

#endif // CL_PRIM_FILE_H
