// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

// FIXME: Lost code

#if 0
#ifndef CL_CANDIDATE_H
#define CL_CANDIDATE_H


/////////////
// INCLUDE //
/////////////
// Misc
#include "nel/misc/types_nl.h"
// Game_share
#include "game_share/ryzom_entity_id.h"

struct Candidate
{
	NLMISC::CEntityId	id;
	std::string			name;
	std::string			surname;
	std::list<std::string>	program;
	uint32				nbVotes;
};


#endif // CL_CANDIDATE_H
#endif