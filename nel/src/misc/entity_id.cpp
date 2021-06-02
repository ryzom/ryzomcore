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

#include "stdmisc.h"

#include "nel/misc/entity_id.h"

using namespace std;

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC {

	const uint64 CEntityId::MaxEntityId = ((uint64)1 << (CEntityId::ID_SIZE + 1)) - (uint64)1;

	CEntityId CEntityId::_NextEntityId;

	uint8 CEntityId::_ServerId = 0;

	const CEntityId CEntityId::Unknown (CEntityId::UNKNOWN_TYPE, 0, 0, 0);

} // NLMISC
