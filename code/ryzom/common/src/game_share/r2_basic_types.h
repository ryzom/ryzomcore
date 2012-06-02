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


#ifndef R2_BASIC_TYPES_H
#define R2_BASIC_TYPES_H

#include "nel/misc/types_nl.h"


/// a very simple type to replace the uint32 session ids used evrywhere in order to reduce bugs
class CSessionId
{
public:
	CSessionId(uint32 id=0): _Id(id) {}
	operator uint32() const { return _Id; }
//	explicit CSessionId(uint32 id=0): _Id(id) {}

	bool operator<(const CSessionId& other) const { return _Id < other._Id; }
	bool operator==(const CSessionId& other) const { return _Id == other._Id; }
	bool operator!=(const CSessionId& other) const { return _Id != other._Id; }
	void serial(NLMISC::IStream& stream) { stream.serial(_Id); }
	uint32 asInt() const { return _Id; }
	std::string toString() const { return NLMISC::toString(_Id); }
private:
	uint32 _Id;
};

// Useful because if not defined the compiler do not chose between
// lh.operator uint32() < rh or  lh < CSessionId(rh)

inline bool operator<(const CSessionId& lh, uint32 rh)  { return lh.asInt() < rh; }
inline bool operator==(const CSessionId& lh, uint32 rh)  { return lh.asInt() == rh; }
inline bool operator!=(const CSessionId& lh, uint32 rh)  { return lh.asInt() != rh; }
// useful for lh != 0
inline bool operator!=(const CSessionId& lh, int rh)  { return lh.asInt() != uint32(rh); }

typedef CSessionId TSessionId;


typedef uint32 TShardId;
typedef uint32 TCharId;


#endif

