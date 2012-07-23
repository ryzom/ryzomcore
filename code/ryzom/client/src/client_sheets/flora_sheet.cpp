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
#include "flora_sheet.h"
//
#include "nel/georges/u_form_elm.h"

using namespace NLGEORGES;

// ***************************************************************************************************
CFloraSheet::CFloraSheet()
{
	Type = FLORA;
	_TotalWeight = 0;
}

// ***************************************************************************************************
void CFloraSheet::build(const NLGEORGES::UFormElm &item)
{
	const UFormElm *plantArray = NULL;
	if (item.getNodeByName(&plantArray, "Plants") && plantArray)
	{
		uint numPlants;
		nlverify(plantArray->getArraySize(numPlants));
		_Plants.reserve(numPlants);
		for(uint k = 0; k < numPlants; ++k)
		{
			const UFormElm *subNode = NULL;
			if (plantArray->getArrayNode(&subNode, k) && subNode)
			{
				CPlantInfo pi;
				pi.build(*subNode);
				pi.CumulatedWeight = _TotalWeight;
				_TotalWeight += pi.Weight;
				_Plants.push_back(pi);
			}
		}
	}
	item.getValueByName(MicroLifeThreshold, "MicroLifeThreshold");
}

// ***************************************************************************************************
void CFloraSheet::serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialCont(_Plants);
	f.serial(MicroLifeThreshold);
	f.serial(_TotalWeight);
}

// ***************************************************************************************************
void CPlantInfo::build(const NLGEORGES::UFormElm &item)
{
	item.getValueByName(SheetName, "File name");
	item.getValueByName(Weight, "MicroLifeWeight");
	if (Weight == 0)
	{
		nlwarning("Plant with weight equal to 0");
	}
}

// ***************************************************************************************************
void CPlantInfo::serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serial(SheetName);
	f.serial(CumulatedWeight);
}

// ***************************************************************************************************
const CPlantInfo *CFloraSheet::getPlantInfoFromWeightedIndex(uint64 index) const
{
	if (_TotalWeight == 0) return NULL;
	CPlantInfo comp;
	comp.CumulatedWeight = index;
	std::vector<CPlantInfo>::const_iterator it = std::lower_bound(_Plants.begin(), _Plants.end(), comp);
	if (it == _Plants.end()) return &(_Plants.back());
	if (it == _Plants.begin()) return &(_Plants.front());
	return it->CumulatedWeight == index ? &(*it) : &(*(it - 1));
}
