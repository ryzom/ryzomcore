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

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "stdpch.h"
#include "outpost_version_adapter.h"

//-----------------------------------------------------------------------------
// namespaces
//-----------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;


//-----------------------------------------------------------------------------
// static members COutpostVersionAdapter
//-----------------------------------------------------------------------------

NL_INSTANCE_COUNTER_IMPL(COutpostVersionAdapter);
COutpostVersionAdapter * COutpostVersionAdapter::_Instance = NULL;


//-----------------------------------------------------------------------------
// methods COutpostVersionAdapter
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
uint32 COutpostVersionAdapter::currentVersionNumber() const
{
	////////////////////////////////////
	// VERSION History
	// 0 : (18/05/2005) initial version
	////////////////////////////////////
	return 0;
}

//-----------------------------------------------------------------------------
void COutpostVersionAdapter::adaptOutpostFromVersion(COutpost & outpost, uint32 version) const
{
	// Do NOT break between case labels
	//switch (version)
	//{
	//case 0: adaptToVersion1(outpost);
	//}
}

//-----------------------------------------------------------------------------
//void COutpostVersionAdapter::adaptToVersion1(COutpost & outpost) const
//{
//}
