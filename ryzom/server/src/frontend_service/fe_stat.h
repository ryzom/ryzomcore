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



#ifndef NL_FE_STAT_H
#define NL_FE_STAT_H


//#define MEASURE_FRONTEND_TABLES
#ifdef MEASURE_FRONTEND_TABLES


#include <nel/misc/types_nl.h>
#include <nel/misc/time_nl.h>
#include <vector>


/*
 * TEventPerSeenEntityCounterFrame
 */
struct TEventPerSeenEntityCounterFrame
{
	NLMISC::TGameCycle	Tick;
	sint32				SeenEntities [MAX_SEEN_ENTITIES_PER_CLIENT];
	float				Value; // optional value

	/// Constructor
	TEventPerSeenEntityCounterFrame() : Tick(0)
	{
		reset();
	}

	/// reset
	void				reset( sint32 def=0 )
	{
		sint i;
		for ( i=0; i!=MAX_SEEN_ENTITIES_PER_CLIENT; ++i )
			SeenEntities[i] = def;
	}

	/// setGameTick
	void				setGameTick();
	
	/// commit
	void				commit( std::vector<TEventPerSeenEntityCounterFrame>& vect )
	{
		vect.push_back( *this );
	}

	/// display
	void				display( bool withvalue=false );

	/// displayAll
	static void			displayAll( const char *name, std::vector<TEventPerSeenEntityCounterFrame>& vect, bool withvalue=false );
};



extern TEventPerSeenEntityCounterFrame				PropRecvrCntFrame1, PropRecvrCntFrame2,
													DistCntFrame,
													DeltaCntFrame,
													PrioCntFrame,
													PosSentCntFrame;

extern std::vector<TEventPerSeenEntityCounterFrame>	PropRecvrCntClt1,
													DistCntClt1,
													DeltaCntClt1,
													PrioCntClt1,
													PosSentCntClt1;

#endif

#endif // NL_FE_STAT_H

/* End of fe_stat.h */
