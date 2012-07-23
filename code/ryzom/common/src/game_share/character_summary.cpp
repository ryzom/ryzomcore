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

#include "stdpch.h"
#include "character_summary.h"

void CCharacterSummary::serial(class NLMISC::IStream &f) throw (NLMISC::EStream)
{
	f.serialVersion(0);
	f.serial		(Mainland);
	f.serial		(Name);
	f.serialEnum	(People);
	f.serial		(Location);
	f.serial		(VisualPropA);
	f.serial		(VisualPropB);
	f.serial		(VisualPropC);
	f.serial		(SheetId);
	f.serialEnum	(Title);
	f.serial		(CharacterSlot);
	f.serial		(InRingSession);
	f.serial		(HasEditSession);

	static volatile bool serialNB = true; // TMP TMP, for test
	if (serialNB)
	{
		f.serial		(InNewbieland);
	}
}



