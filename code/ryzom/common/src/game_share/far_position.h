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

#include "nel/misc/types_nl.h"
#include "game_share/r2_basic_types.h"
//#include "game_share/r2_types.h"
#include "persistent_data.h"


#ifndef NL_FAR_POSITION_H
#define NL_FAR_POSITION_H


class CEntityState;

/** Define the online state of a character */
enum TCharConnectionState
{
	/// the character is offline
	ccs_offline =0,
	/// the character is online on the same shard
	ccs_online =1,
	/// the character is online, but on another shard in the domain.
	ccs_online_abroad =2
};

/**
 * COfflineEntityState
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2002
 */
class COfflineEntityState
{
public:
	// Start by declaring methods for persistent load/ save operations
	// The following macro is defined in persistent_data.h
	// At time of writing it evaluated to:
	//	void store(CPersistentDataRecord &pdr) const;
	//	void apply(CPersistentDataRecord &pdr);

	DECLARE_PERSISTENCE_METHODS

	sint32 X;
	sint32 Y;
	sint32 Z;

	float Heading;

	/**
	 * Constructor
	 */
	COfflineEntityState()
	{
		clear();
	}

	void clear()
	{
		X = 0;
		Y = 0;
		Z = 0;
		Heading = 0;
	}

	COfflineEntityState(const CEntityState& state);

	// Comparison
	bool operator== (const COfflineEntityState &other ) const;

	// Assignment
	COfflineEntityState& operator= (const COfflineEntityState &src)
	{
		X = src.X;
		Y = src.Y;
		Z = src.Z;
		Heading = src.Heading;
		return *this;
	}

	/**
	 * Serial
	 */
	void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
	{
		f.serial(X);
		f.serial(Y);
		f.serial(Z);
		f.serial(Heading);
	}

	// Return as string
	std::string toString() const;
};


/*
 * Session identifier.
 * On a mainland shard, there is a main fixed TSessionId for the whole EGS.
 * On a ring shard (adventures, outlands), the DSS manages several sessions.
 * Each character is in a session.
 */
//typedef uint32 TSessionId;

// Special value to force saving the position stack without updating it from the outside anymore
const TSessionId SessionLockPositionStack(~0u);


/**
 * Session Id + Entity State (X,Y,Z,Theta)
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2006
 */
class CFarPosition
{
public:

	DECLARE_PERSISTENCE_METHODS

	TSessionId			SessionId;
	COfflineEntityState	PosState;

	CFarPosition();

	// Comparison
	bool operator== (const CFarPosition &other) const;

	// Assignment
	CFarPosition& operator= (const CFarPosition &src)
	{
		SessionId = src.SessionId;
		PosState = src.PosState;
		return *this;
	}

	/**
	 * Serial
	 */
	void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
	{
		f.serial(SessionId);
		f.serial(PosState);
	}


};


#endif // NL_FAR_POSITION_H

/* End of far_position.h */
