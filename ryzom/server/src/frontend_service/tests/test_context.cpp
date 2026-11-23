// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2026  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

// The tests of this program use NeL classes, which want a NeL context.

#include <gtest/gtest.h>

#include "nel/misc/types_nl.h"
#include "nel/misc/app_context.h"
#include "nel/misc/debug.h"

namespace /* anonymous */ {

class CNelContextEnvironment : public ::testing::Environment
{
public:
	void SetUp() override
	{
		if (!NLMISC::INelContext::isContextInitialised())
			m_Context = new NLMISC::CApplicationContext();
		NLMISC::createDebug(nullptr);
	}

private:
	NLMISC::CApplicationContext *m_Context = nullptr;
};

::testing::Environment *const s_NelContextEnvironment = ::testing::AddGlobalTestEnvironment(new CNelContextEnvironment());

} /* anonymous namespace */

/* end of file */
