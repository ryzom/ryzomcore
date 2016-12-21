// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#ifndef UT_NET
#define UT_NET

#include <nel/net/message.h>

#include "ut_net_layer3.h"
#include "ut_net_message.h"
#include "ut_net_module.h"
// Add a line here when adding a new test CLASS

struct CUTNet : public Test::Suite
{
	CUTNet()
	{
		add(std::auto_ptr<Test::Suite>(new CUTNetLayer3));
		add(std::auto_ptr<Test::Suite>(new CUTNetMessage));
		add(std::auto_ptr<Test::Suite>(new CUTNetModule));
		// Add a line here when adding a new test CLASS
	}
};

#endif
