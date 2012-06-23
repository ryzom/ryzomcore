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

#ifndef LD_FORMFLORA_H
#define LD_FORMFLORA_H

// ---------------------------------------------------------------------------

#include "nel/misc/types_nl.h"

#include <vector>
#include <string>

namespace NLGEORGES
{
	class UFormElm;
}

// ---------------------------------------------------------------------------
struct SPlantInstance
{
	std::string		Name;
	float			Density;
	float			Falloff;
};

// ---------------------------------------------------------------------------
struct SFormFlora
{
	std::vector<std::string>	IncludePatats;
	std::vector<std::string>	ExcludePatats;
	std::vector<SPlantInstance>	PlantInstances;
	float	JitterPos;
	float	ScaleMin;
	float	ScaleMax;
	bool	PutOnWater;
	float	WaterHeight;
	uint32	RandomSeed;

	void build (NLGEORGES::UFormElm &item);
};

#endif // LD_FORMFLORA_H