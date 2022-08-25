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

//Nel georges
#include "nel/georges/u_form.h"
#include "game_share/sp_type.h"


namespace GUILD_OPTIONS
{
	enum TType
	{
		MainBuilding,
		RmFight,
		RmMagic,
		RmHarvest,
		RmCraft,
		Unknown,
	};
}

/**
 * A guild option form
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2003
 */
class CStaticGuildOption
{
public:
	

	EGSPD::CSPType::TSPType getSpType()const
	{
		switch(Type)
		{
		case GUILD_OPTIONS::RmFight:
			return EGSPD::CSPType::Fight;
		case GUILD_OPTIONS::RmMagic:
			return EGSPD::CSPType::Magic;
		case GUILD_OPTIONS::RmHarvest:
			return EGSPD::CSPType::Harvest;
		case GUILD_OPTIONS::RmCraft:
			return EGSPD::CSPType::Craft;
		default:
			return EGSPD::CSPType::EndSPType;
		}
	}

	/// Read georges sheet
	void readGeorges (const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId);

	/// Serial
	void serial(class NLMISC::IStream &f);

	/// Return the version of this class, increments this value when the content of this class has changed
	inline static uint getVersion () { return 1; }

	/// Removed
	void removed() {}

	uint32 Price;
	GUILD_OPTIONS::TType Type;

};


