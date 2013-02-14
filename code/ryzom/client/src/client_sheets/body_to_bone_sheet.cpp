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
//
#include "body_to_bone_sheet.h"
#include "client_sheets.h"
//
#include "nel/georges/u_form_elm.h"

static void build(NLMISC::TSStringId &result, const NLGEORGES::UFormElm &item, const std::string &prefix, const std::string &name)
{
	std::string str;
	item.getValueByName(str, (prefix + name).c_str());
	result = ClientSheetsStrings.add(str);
}

// *********************************************************************************************
CBodyToBoneSheet::CBodyToBoneSheet()
{
	Head = 0;
	Chest = 0;
	LeftArm = 0;
	RightArm = 0;
	LeftHand = 0;
	RightHand = 0;
	LeftLeg = 0;
	RightLeg = 0;
	LeftFoot = 0;
	RightFoot = 0;
}

// *********************************************************************************************
void CBodyToBoneSheet::build(const NLGEORGES::UFormElm &item, const std::string &prefix)
{
	::build(Head, item, prefix, "Head");
	::build(Chest, item, prefix, "Chest");
	::build(LeftArm, item, prefix, "LeftArm");
	::build(RightArm, item, prefix, "RightArm");
	::build(LeftHand, item, prefix, "LeftHand");
	::build(RightHand, item, prefix, "RightHand");
	::build(LeftLeg, item, prefix, "LeftLeg");
	::build(RightLeg, item, prefix, "RightLeg");
	::build(LeftFoot, item, prefix, "LeftFoot");
	::build(RightFoot, item, prefix, "RightFoot");
}

// *********************************************************************************************
void CBodyToBoneSheet::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	ClientSheetsStrings.serial(f, Head);
	ClientSheetsStrings.serial(f, Chest);
	ClientSheetsStrings.serial(f, LeftArm);
	ClientSheetsStrings.serial(f, RightArm);
	ClientSheetsStrings.serial(f, LeftHand);
	ClientSheetsStrings.serial(f, RightHand);
	ClientSheetsStrings.serial(f, LeftLeg);
	ClientSheetsStrings.serial(f, RightLeg);
	ClientSheetsStrings.serial(f, LeftFoot);
	ClientSheetsStrings.serial(f, RightFoot);
}

// *********************************************************************************************
const char *CBodyToBoneSheet::getBoneName(BODY::TBodyPart part, BODY::TSide side) const
{
	BODY::TBodyPart hominPart = BODY::getMatchingHominBodyPart(part);
	switch(hominPart)
	{
		case BODY::HHead:	return ClientSheetsStrings.get(Head);
		case BODY::HChest:  return ClientSheetsStrings.get(Chest);
		case BODY::HArms:	return ClientSheetsStrings.get(side == BODY::Left ? LeftArm  : RightArm);
		case BODY::HHands:  return ClientSheetsStrings.get(side == BODY::Left ? LeftHand : RightHand);
		case BODY::HLegs:   return ClientSheetsStrings.get(side == BODY::Left ? LeftLeg  : RightLeg);
		case BODY::HFeet:   return ClientSheetsStrings.get(side == BODY::Left ? LeftFoot : RightFoot);
		default: break;
	}
	return NULL;
}
