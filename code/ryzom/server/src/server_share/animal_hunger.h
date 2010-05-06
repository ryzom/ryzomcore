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

/* Copyright, 2000-2004 Nevrax Ltd.
 *
 * This file is part of NEVRAX RYZOM.
 * NEVRAX RYZOM is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX RYZOM is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX RYZOM; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#ifndef NL_ANIMAL_HUNGER_H
#define NL_ANIMAL_HUNGER_H

#include "nel/misc/types_nl.h"
#include "game_share/ryzom_mirror_properties.h"
#include "game_share/mirror_prop_value.h"
#include "game_share/synchronised_message.h"

/**
 * CSpeedLimit
 * \author Olivier Cado
 * \author Nevrax France
 * All speeds are in meter per game cycle (m/gc).
 * \date 2004
 */
class CSpeedLimit
{
public:

	/// Constructor. The specified row need be accessible.
	CSpeedLimit( CMirroredDataSet& dataset, const TDataSetRow& row ) : _FreeSpeedValue( dataset, row, DSPropertyFUEL ) {}

	/// Return the current max allowable speed (one of the params)
	float		getSpeedLimit( float walkSpeed, float maxSpeed ) const
	{
		return (_FreeSpeedValue() ? maxSpeed : walkSpeed);
	}

private:
	
	CMirrorPropValueRO<TYPE_FUEL>	_FreeSpeedValue;

};


#endif // NL_ANIMAL_HUNGER_H

/* End of animal_hunger.h */
