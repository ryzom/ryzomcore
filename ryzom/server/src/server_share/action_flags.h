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



#ifndef RY_ACTION_FLAGS_H
#define RY_ACTION_FLAGS_H

#include "nel/misc/types_nl.h"

namespace RYZOMACTIONFLAGS
{
	enum TActionFlag
	{
		InWater = 1<<1,
		Attacks = 1<<0,
		Undefined = 1<<16,
	};

	/**
	 * get the action flag size as a string
	 * \param size the TActionFlag size
	 * \return the string associated to this TActionFlag
	 */
	const std::string &toString(TActionFlag flag);

	/**
	 * convert a string to an action flag
	 * \param str the string to convert
	 * \return the action flag associated to the string, Undefined if unknown
	 */
	TActionFlag toActionFlag(const std::string str);

};	// RYZOMACTIONFLAGS

#endif //RY_ACTION_FLAGS_H

/* End of action_flags.h */
