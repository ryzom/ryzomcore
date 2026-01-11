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

#include "formPlant.h"

#include "nel/misc/common.h"

#include "nel/georges/u_form_elm.h"

using namespace std;
using namespace NLMISC;
using namespace NLGEORGES;

// ---------------------------------------------------------------------------
void SFormPlant::build (UFormElm &item)
{
	item.getValueByName (Name, "Plant Name");

	item.getValueByName (Shape, "3D.Shape");

	item.getValueByName (Shadow, "3D.Shadow Shape");

	if (!item.getValueByName (CollisionRadius, "3D.Collision Radius"))
		CollisionRadius = 0.0f;

	if (!item.getValueByName (BoundingRadius, "3D.Bounding Radius"))
		BoundingRadius = 0.0f;
}
