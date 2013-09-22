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

#include "stdpacs.h"
#include "nel/misc/i_xml.h"
#include "nel/pacs/primitive_block.h"


namespace NLPACS
{

// ***************************************************************************

CPrimitiveDesc::CPrimitiveDesc ()
{
	Attenuation = 1;
	Type = UMovePrimitive::_2DOrientedBox;
	Reaction = UMovePrimitive::DoNothing;
	Trigger = UMovePrimitive::NotATrigger;
	Obstacle = true;
	OcclusionMask = 0xffffffff;
	CollisionMask = 0xffffffff;
}

// ***************************************************************************

void CPrimitiveDesc::serial (NLMISC::IStream &s)
{
	// Serial the version
	sint ver = s.serialVersion (1);

	s.xmlPush ("LENGTH");
	s.serial (Length[0]);
	s.serial (Length[1]);
	s.xmlPop ();

	s.xmlSerial (Height, "HEIGHT");
	s.xmlSerial (Attenuation, "ATTENUATION");

	s.xmlPush ("TYPE");
	s.serialEnum (Type);
	s.xmlPop ();

	s.xmlPush ("REACTION");
	s.serialEnum (Reaction);
	s.xmlPop ();

	s.xmlPush ("TRIGGER");
	s.serialEnum (Trigger);
	s.xmlPop ();

	s.xmlSerial (Obstacle, "OBSTACLE");
	s.xmlSerial (OcclusionMask, "OCCLUSION_MASK");
	s.xmlSerial (CollisionMask, "COLLISION_MASK");
	s.xmlSerial (Position, "POSITION");
	s.xmlSerial (Orientation, "ORIENTATION");

	if (ver>=1)
	{
		s.xmlSerial (UserData, "USER_DATA");
	}
	else
	{
		UserData = 0;
	}
}

// ***************************************************************************

void CPrimitiveBlock::serial (NLMISC::IStream &s)
{
	s.xmlPush ("PRIMITIVE_BLOCK");

	// Serial checks
	s.serialCheck (NELID("KBRP"));

	// Serial the version
	(void)s.serialVersion (0);

	s.xmlPush ("PRIMITIVES");
	s.serialCont (Primitives);
	s.xmlPop ();

	s.xmlPop ();
}

// ***************************************************************************
UPrimitiveBlock *UPrimitiveBlock::createPrimitiveBlock(NLMISC::IStream &src)
{

	nlassert(src.isReading());
	std::auto_ptr<CPrimitiveBlock> pb(new CPrimitiveBlock);
	pb->serial(src);
	return pb.release();
}

// ***************************************************************************
UPrimitiveBlock *UPrimitiveBlock::createPrimitiveBlockFromFile(const std::string &fileName)
{

	NLMISC::CIFile input;
	if (input.open(fileName))
	{
		NLMISC::CIXml xmlInput;
		// Init
		if (xmlInput.init (input))
		{
			return createPrimitiveBlock(xmlInput);
		}
		else
		{
			throw NLMISC::Exception(std::string("Unable to init an xml input file from ") + fileName);
		}
	}
	return NULL;
}




} // NLPACS
