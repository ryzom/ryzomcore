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



#ifndef RY_ENTITY_STATE_H
#define RY_ENTITY_STATE_H

#include "nel/misc/vector_2d.h"
#include "nel/misc/vector.h"

#include "game_share/far_position.h"
#include "game_share/mirror_prop_value.h"


/**
 * CEntityState
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2002
 */
class CEntityState
{
public:
	// Start by declaring methods for persistent load/ save operations
	// The following macro is defined in persistent_data.h
	// At time of writing it evaluated to:
	//	void store(CPersistentDataRecord &pdr) const;
	//	void apply(CPersistentDataRecord &pdr);

	DECLARE_PERSISTENCE_METHODS

	CMirrorPropValueAlice< sint32, CPropLocationPacked<2> > X;
	CMirrorPropValueAlice< sint32, CPropLocationPacked<2> > Y;
	CMirrorPropValueAlice< sint32, CPropLocationPacked<2> > Z;
	
	CMirrorPropValueAlice< float, CPropLocationPacked<2> > Heading;

	CEntityState()
	{
		clear();
	}

	void clear()
	{
		X=0;
		Y=0;
		Z=0;
		Heading=0;
	}

	const CEntityState &operator = (const COfflineEntityState &s)
	{
		X = s.X;
		Y = s.Y;
		Z = s.Z;
		Heading = s.Heading;
		return *this;
	}

	/**
	 *	operator=
	 */
	void setCOfflineEntityState( COfflineEntityState &s ) const
	{ 
		s.X = X;
		s.Y = Y;
		s.Z = Z;
		s.Heading = Heading;
	}


	/**
	 *	Store temporaryly the state from an COfflineEntityState object (for later mirrorizing)
	 */
	void storeToTemp( const COfflineEntityState& state ) 
	{ 
		X= state.X;
		Y= state.Y;
		Z= state.Z;
		Heading= state.Heading;
	}

	/**
	 * Serial (read to temp storage, or write from mirror)
	 */
	void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
	{
		X.serialRTWM( f );
		Y.serialRTWM( f );
		Z.serialRTWM( f );
		Heading.serialRTWM( f );
	}

	/**
	 * Fill vector in meter corresponding to state
	 */
	void getVector( NLMISC::CVector& v ) const
	{
		v.x = X * 0.001f;
		v.y = Y * 0.001f;
		v.z = Z * 0.001f;
	}

	/**
	 * Fill a CVectorfD in meter corresponding to state
	 */
	void getVector2f( NLMISC::CVector2f& v ) const
	{
		v.x = X * 0.001f;
		v.y = Y * 0.001f;
	}
};


inline COfflineEntityState::COfflineEntityState(const CEntityState& state)
{
	X = state.X;
	Y = state.Y;
	Z = state.Z;
	Heading = state.Heading;
}


#endif // RY_ENTITY_STATE_H
/* entity_state.h */
