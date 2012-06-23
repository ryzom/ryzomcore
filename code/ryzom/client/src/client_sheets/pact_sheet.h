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



#ifndef RY_PACT_SHEET_H
#define RY_PACT_SHEET_H

#include "entity_sheet.h"

/// A pact, read from a .death_impact sheet
class CPactSheet : public CEntitySheet
{
public:
	struct SPact
	{
		uint16	    LoseHitPointsLevel;
		uint16	    LoseStaminaLevel;
		uint16	    LoseSapLevel;
		uint16	    LoseSkillsLevel;
		float	    Duration;
		std::string Name;

		void serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
		{
			f.serial( LoseHitPointsLevel);
			f.serial( LoseStaminaLevel );
			f.serial( LoseSapLevel );
			f.serial( LoseSkillsLevel );
			f.serial( Duration );
			f.serial( Name );
		}
	};
public:
	std::vector< SPact > PactLose;
	std::string			 Icon;
	std::string			 IconBackground;
public:
	///\name Object
	//@{
		/// Build the sheet from an external script.
		virtual void build(const NLGEORGES::UFormElm &item);
		/// Serialize plant sheet into binary data file.
		virtual void serial(class NLMISC::IStream &f) throw(NLMISC::EStream);
	//@}
};

#endif
