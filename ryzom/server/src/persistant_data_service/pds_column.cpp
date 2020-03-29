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

#include "../pd_lib/pds_common.h"

#include "pds_column.h"
#include "pds_attribute.h"
#include "pds_type.h"
#include "pds_database.h"
#include "pds_table.h"

#include <nel/misc/debug.h>

using namespace std;
using namespace NLMISC;

// Destructor
CColumn::~CColumn()
{
	//PDS_DEBUG("delete()");
}

/*
 * Initialize column
 */
bool	CColumn::init(CAttribute* parent, CDatabase *root)
{
	_Parent = parent;
	_Root = root;
	_Init = true;

	// set parent logger
	setParentLogger(_Parent->getParent());

	return true;
}

