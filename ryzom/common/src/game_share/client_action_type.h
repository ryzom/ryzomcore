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



#ifndef NL_CLIENT_ACTION_TYPE_H
#define NL_CLIENT_ACTION_TYPE_H

#include "nel/misc/types_nl.h"


namespace CLIENT_ACTION_TYPE
{
	enum TClientActionType
	{
		None= 0,	// display nothing

		Combat,
		Spell,
		Faber,
		Repair,
		Refine,
		Memorize,
		Forage,
		Harvest,
		Training,
		Tame,
		Teleport,
		Disconnect,
		Mount,
		Unmount,
		ConsumeItem,

		NumClientActionType
	};

	const std::string	&toString(TClientActionType e);
	TClientActionType	fromString(const std::string &s);
};

#endif // NL_CLIENT_ACTION_TYPE_H

/* End of client_action_type.h */
