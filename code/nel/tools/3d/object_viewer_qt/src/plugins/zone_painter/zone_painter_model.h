// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
// Copyright (C) 2011  Dzmitry Kamiahin <dnk-88@tut.by>
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

#ifndef ZONE_PAINTER_MODEL_H
#define ZONE_PAINTER_MODEL_H

// NeL includes
#include <nel/misc/types_nl.h>
#include <nel/misc/rgba.h>
#include <nel/misc/event_emitter.h>

// Qt includes

namespace Plugin
{

class CZonePainterModel
{
public:
	CZonePainterModel();
	virtual ~CZonePainterModel();

}; /* class CZonePainterModel */

} /* namespace Plugin */


#endif // ZONE_PAINTER_MODEL_H
