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

#ifndef NL_SCISSOR_H
#define NL_SCISSOR_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"


namespace NL3D
{

using NLMISC::CVector;


// ***************************************************************************
/**
 * A scissor, used for IDriver rendering. NB: you can specify negative values for x/y.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
/* *** IMPORTANT ********************
 * *** IF YOU MODIFY THE STRUCTURE OF THIS CLASS, PLEASE INCREMENT IDriver::InterfaceVersion TO INVALIDATE OLD DRIVER DLL
 * **********************************
 */
class CScissor
{
public:
	float	X;
	float	Y;
	float	Width;
	float	Height;

	/// Constructor. fullScreen.
	CScissor()
	{
		initFullScreen();
	}
	/// Constructor.
	CScissor(float x, float y, float width, float height)
	{
		X= x;
		Y= y;
		Width= width;
		Height= height;
	}


	/// init. simple copy.
	void	init (float x, float y, float width, float height)
	{
		X= x;
		Y= y;
		Width= width;
		Height= height;
	}


	/// reset to FullScreen
	void	initFullScreen()
	{
		X= 0;
		Y= 0;
		Width= 1;
		Height=1;
	}

};


} // NL3D


#endif // NL_SCISSOR_H

/* End of scissor.h */
