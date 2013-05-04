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
#include "item_fx_sheet.h"
#include "client_sheets.h"

#include "nel/georges/u_form_elm.h"

// *******************************************************************************************
CItemFXSheet::CItemFXSheet()
{
	_Trail = 0;
	TrailMinSliceTime = 0.05f;
	TrailMaxSliceTime = 0.05f;
	ImpactFXDelay = 0.f;
	_AdvantageFX = 0;
	_AttackFX = 0;
	AttackFXRot.set(0.f, 0.f, 0.f);
}

// *******************************************************************************************
void CItemFXSheet::build(const NLGEORGES::UFormElm &item, const std::string &prefix)
{
	std::string trail;
	item.getValueByName(trail, (prefix + "Trail").c_str());
	_Trail = ClientSheetsStrings.add(trail);
	item.getValueByName(TrailMinSliceTime, (prefix + "TrailMinSliceTime").c_str());
	item.getValueByName(TrailMaxSliceTime, (prefix + "TrailMaxSliceTime").c_str());
	std::string advantageFX;
	item.getValueByName(advantageFX, (prefix + "AdvantageFX").c_str());
	_AdvantageFX = ClientSheetsStrings.add(advantageFX);
	std::string attackFX;
	item.getValueByName(attackFX, (prefix + "AttackFX").c_str());
	_AttackFX = ClientSheetsStrings.add(attackFX);
	item.getValueByName(AttackFXOffset.x, (prefix + "AttackFXOffset.X").c_str());
	item.getValueByName(AttackFXOffset.y, (prefix + "AttackFXOffset.Y").c_str());
	item.getValueByName(AttackFXOffset.z, (prefix + "AttackFXOffset.Z").c_str());
	item.getValueByName(AttackFXRot.x,    (prefix + "AttackFXRot.X").c_str());
	item.getValueByName(AttackFXRot.y,    (prefix + "AttackFXRot.Y").c_str());
	item.getValueByName(AttackFXRot.z,    (prefix + "AttackFXRot.Z").c_str());
	item.getValueByName(ImpactFXDelay,    (prefix + "ImpactFXDelay").c_str());
	const NLGEORGES::UFormElm *array = NULL;
	if (item.getNodeByName(&array, (prefix + "StaticFXs").c_str()) && array)
	{
		uint count;
		nlverify(array->getArraySize(count));
		_StaticFXs.reserve(count);
		for(uint k = 0; k < count; ++k)
		{
			const NLGEORGES::UFormElm *node;
			if (array->getArrayNode(&node, k))
			{
				CStaticFX fx;
				fx.build(*node);
				_StaticFXs.push_back(fx);
			}
		}
	}
}

// *******************************************************************************************
void CItemFXSheet::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serial(TrailMinSliceTime);
	f.serial(TrailMaxSliceTime);
	f.serial(AttackFXOffset);
	ClientSheetsStrings.serial(f, _Trail);
	ClientSheetsStrings.serial(f, _AdvantageFX);
	ClientSheetsStrings.serial(f, _AttackFX);
	f.serial(AttackFXRot);
	f.serial(ImpactFXDelay);
	f.serialCont(_StaticFXs);
}

// *******************************************************************************************
const char *CItemFXSheet::getTrail() const
{
	return _Trail ? ClientSheetsStrings.get(_Trail) : "";
}

// *******************************************************************************************
const char *CItemFXSheet::getAdvantageFX() const
{
	return _AdvantageFX ? ClientSheetsStrings.get(_AdvantageFX) : "";
}

// *******************************************************************************************
const char *CItemFXSheet::getAttackFX() const
{
	return _AttackFX ? ClientSheetsStrings.get(_AttackFX) : "";
}

// *******************************************************************************************
void CItemFXSheet::CStaticFX::build(const NLGEORGES::UFormElm &item)
{
	std::string name;
	std::string bone;
	item.getValueByName(name, "Name");
	item.getValueByName(bone, "Bone");
	Name = ClientSheetsStrings.add(name);
	Bone = ClientSheetsStrings.add(bone);
	item.getValueByName(Offset.x,    "OffsetX");
	item.getValueByName(Offset.y,    "OffsetY");
	item.getValueByName(Offset.z,    "OffsetZ");
}

// *******************************************************************************************
void CItemFXSheet::CStaticFX::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	ClientSheetsStrings.serial(f, Name);
	ClientSheetsStrings.serial(f, Bone);
	f.serial(Offset);
}

// *******************************************************************************************
const char *CItemFXSheet::getStaticFXName(uint index) const
{
	nlassert(index < _StaticFXs.size());
	return _StaticFXs[index].Name ? ClientSheetsStrings.get(_StaticFXs[index].Name) : "";
}

// *******************************************************************************************
const char *CItemFXSheet::getStaticFXBone(uint index) const
{
	nlassert(index < _StaticFXs.size());
	return _StaticFXs[index].Bone ? ClientSheetsStrings.get(_StaticFXs[index].Bone) : "";
}

// *******************************************************************************************
const NLMISC::CVector &CItemFXSheet::getStaticFXOffset(uint index) const
{
	nlassert(index < _StaticFXs.size());
	return _StaticFXs[index].Offset;
}




