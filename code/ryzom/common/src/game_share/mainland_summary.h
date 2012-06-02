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

#ifndef MAINLAND_SUMMARY_H
#define MAINLAND_SUMMARY_H

#include "far_position.h"

#include <string>


/**
 * CMainlandSummary
 *
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2006
 */
struct CMainlandSummary
{
	CMainlandSummary(): Id(0)
	{
		Name = "";
		Description = "";
		LanguageCode = "en";
		Online = false;
	}

	/// mainland Id
	TSessionId Id;

	/// description
	ucstring Name;

	/// description
	ucstring Description;

	/// language code
	std::string LanguageCode;

	/// true if mainland is up
	bool Online;

	/// serialisation coming from a stream (net message)
	void serial(class NLMISC::IStream &f) throw (NLMISC::EStream)
	{
		f.serial( Id );
		f.serial( Name );
		f.serial( Description );
		f.serial( LanguageCode );
		f.serial( Online );
	}
};

#endif
