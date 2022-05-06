// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2014  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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
#include "browse_faq.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

using namespace NLMISC;

void browseFAQ(NLMISC::CConfigFile &cf)
{
	std::string url;
	std::string languageCode = "wk";
	CConfigFile::CVar *languageCodeVarPtr = cf.getVarPtr("LanguageCode");

	if (languageCodeVarPtr)
	{
		languageCode = languageCodeVarPtr->asString();
	}

	CConfigFile::CVar *helpPages = cf.getVarPtr("HelpPages");

	if (helpPages)
	{
		for (uint i = 0; i < helpPages->size(); ++i)
		{
			std::string entry = helpPages->asString(i);

			if (entry.size() >= languageCode.size())
			{
				if (nlstricmp(entry.substr(0, languageCode.size()), languageCode) == 0)
				{
					std::string::size_type pos = entry.find("=");

					if (pos != std::string::npos)
					{
						url = entry.substr(pos + 1);
					}
				}
			}
		}
	}

	if (url.empty())
	{
		// not found
		nlwarning("No FAQ url");
	}
	else
	{
		openURL(url);
	}
}
