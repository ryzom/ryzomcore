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



#ifndef ZC_SHARD_COMMON_H
#define ZC_SHARD_COMMON_H

#include "nel/misc/types_nl.h"

namespace ZCSTATE
{
	enum TZcState
	{
		/// tribe is in control of the zc
		Tribe = 0,
		/// A guild declared war to the tribe
		TribeInWar,
		/// A guild declared peace to the tribe
		//TribeInPeace,
		/// A guild won the zc through war
		GuildInWar,
		/// A guild won the zc through peace
		GuildInPeace,
		/// Unknow zc state
		zs_unknown
	};


	TZcState toZcState( const std::string &str );

	const std::string& toString( TZcState type );

	// The ZC Duty Distribution period, in Days.
	enum	{ZCDistribPeriod= 15};

};


#endif
