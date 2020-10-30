// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#include <string>
#include <iostream>

// contains all i18n features
#include "nel/misc/i18n.h"

// contains the path features
#include "nel/misc/path.h"

#ifndef NL_LANG_DATA
#define NL_LANG_DATA "."
#endif // NL_LANG_DATA

using namespace NLMISC;

int main (int argc, char **argv)
{
	createDebug();

	// Add the language data path to the search path.
	CPath::addSearchPath(NL_LANG_DATA);

	InfoLog->displayRawNL("Please, choose 'en', 'fr' or 'de' and press <return>");

	std::string langName;
	std::getline(std::cin, langName);

	// load the language
	CI18N::load(langName);

	InfoLog->displayRawNL(CI18N::get("Hi").c_str());
	InfoLog->displayRawNL(CI18N::get("PresentI18N").c_str(), "Nevrax");
	InfoLog->displayRawNL(CI18N::get("ExitStr").c_str());
	getchar();

	return EXIT_SUCCESS;
}
