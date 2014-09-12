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



#ifndef RY_KNOWN_BRICK_INFO_H
#define RY_KNOWN_BRICK_INFO_H
/*
// ---------------------------------------------------------------------------
class CStaticGameBrick;

// ---------------------------------------------------------------------------
struct CKnownBrickInfo
{
	const CStaticGameBrick*	Form;
	uint32					LatencyEndDate;
	bool					OldLatentState;
	
	CKnownBrickInfo( const CStaticGameBrick *form = NULL ) : Form(form)
	{		
		LatencyEndDate = 0;
		OldLatentState = false;
	}
	
	/// Serialisation
	void serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
	{
		//		f.serial( LatencyEndDate );
		//		f.serial( OldLatentState );
		// nothing to serial here, the date are no longer meaningful, all bricks are available when the character connects to the game
		// and the Form is set while setting the databse in the setDatabase() method
	}
};
*/
#endif // RY_KNOWN_BRICK_INFO_H
/* known_brick_info.h */
