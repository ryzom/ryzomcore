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

using namespace NLMISC;
using namespace NLNET;
using namespace std;

//////////////////////////////////////////////////////////////////////////////
// MULTI_LINE_FORMATER                                                      //
//////////////////////////////////////////////////////////////////////////////

static int const MULTI_LINE_FORMATER_maxn = 78;
void MULTI_LINE_FORMATER::pushTitle(std::vector<std::string>& container, std::string const& text)
{
	const sint maxn = MULTI_LINE_FORMATER_maxn;
	sint n = maxn - (sint)text.length() - 4;
	container.push_back(" _/");
	container.back() += text;
	container.back() += "\\" + std::string(n, '_');
	container.push_back("/");
	container.back() += std::string(maxn - 1, ' ');
}

void MULTI_LINE_FORMATER::pushEntry(std::vector<std::string>& container, std::string const& text)
{
	container.push_back("| ");
	container.back() += text;
}

void MULTI_LINE_FORMATER::pushFooter(std::vector<std::string>& container)
{
	int const maxn = MULTI_LINE_FORMATER_maxn;
	container.push_back("\\");
		container.back() += std::string(maxn - 1, '_');
}
