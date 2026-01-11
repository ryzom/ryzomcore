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



#ifndef RY_ROLEMASTER_FLAGS_H
#define RY_ROLEMASTER_FLAGS_H

#include "nel/misc/types_nl.h"

namespace ROLEMASTER_FLAGS
{
	enum TRolemasterFlag
	{
		FightActions = 0,
		MagicActions = 1,
		ForageActions = 2,
		CraftActions = 3,

		CaracteristicUpgrades = 4,
		SpecialPowers = 5,

		NbFlags = 6,
	};

	/// convert a TRolemasterFlag to a string
	const std::string &toString(TRolemasterFlag type);

	/// convert a string to a TRolemasterFlag
	TRolemasterFlag fromString( const std::string &str);

	/// from a .sphrase sheet name, test if it can be sold by rolemaster Flags
	bool	canSellPhrase(uint32 rmfBitField, const std::string &sheetName);

}; // ROLEMASTER_FLAGS

#endif // RY_ROLEMASTER_FLAGS_H
/* End of rolemaster_flags.h */
