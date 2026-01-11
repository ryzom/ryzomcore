/**
 * \file class_data.cpp
 * \brief CClassData
 * \date 2012-08-18 19:24GMT
 * \author Jan Boon (Kaetemi)
 * CClassData
 */

/*
 * Copyright (C) 2012  by authors
 *
 * This file is part of RYZOM CORE PIPELINE.
 * RYZOM CORE PIPELINE is free software: you can redistribute it
 * and/or modify it under the terms of the GNU Affero General Public
 * License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * RYZOM CORE PIPELINE is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with RYZOM CORE PIPELINE.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <nel/misc/types_nl.h>
#include "class_data.h"

// STL includes

// NeL includes
// #include <nel/misc/debug.h>

// Project includes

using namespace std;
// using namespace NLMISC;

namespace PIPELINE {
namespace MAX {

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

CClassData::CClassData()
{

}

CClassData::~CClassData()
{

}

std::string CClassData::className() const
{
	return "ClassData";
}

void CClassData::toString(std::ostream &ostream, const std::string &pad) const
{
	CStorageContainer::toString(ostream, pad);
}

void CClassData::parse(uint16 version, uint filter)
{
	CStorageContainer::parse(version);
}

void CClassData::clean()
{
	CStorageContainer::clean();
}

void CClassData::build(uint16 version, uint filter)
{
	CStorageContainer::build(version);
}

void CClassData::disown()
{
	CStorageContainer::disown();
}

IStorageObject *CClassData::createChunkById(uint16 id, bool container)
{
	if (container)
	{
		switch (id)
		{
		case 0x2100: // ClassDataEntry
			return new CClassDataEntry();
		}
	}
	return CStorageContainer::createChunkById(id, container);
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

CClassDataEntry::CClassDataEntry()
{

}

CClassDataEntry::~CClassDataEntry()
{

}

std::string CClassDataEntry::className() const
{
	return "ClassDataEntry";
}

void CClassDataEntry::toString(std::ostream &ostream, const std::string &pad) const
{
	CStorageContainer::toString(ostream, pad);
}

void CClassDataEntry::parse(uint16 version, uint filter)
{
	CStorageContainer::parse(version);
}

void CClassDataEntry::clean()
{
	CStorageContainer::clean();
}

void CClassDataEntry::build(uint16 version, uint filter)
{
	CStorageContainer::build(version);
}

void CClassDataEntry::disown()
{
	CStorageContainer::disown();
}

IStorageObject *CClassDataEntry::createChunkById(uint16 id, bool container)
{
	if (!container)
	{
		switch (id)
		{
		case 0x2110: // ClassDataHeader
			return new CClassDataHeader();
		}
	}
	switch (id)
	{
	case 0x2120: // ClassDataBody: Depends on the ClassDataHeader: TODO
		return CStorageContainer::createChunkById(id, container);
	}
	return CStorageContainer::createChunkById(id, container);
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

CClassDataHeader::CClassDataHeader()
{

}

CClassDataHeader::~CClassDataHeader()
{

}

std::string CClassDataHeader::className() const
{
	return "ClassDataHeader";
}

void CClassDataHeader::serial(NLMISC::IStream &stream)
{
	stream.serial(ClassID);
	stream.serial(SuperClassID);
}

void CClassDataHeader::toString(std::ostream &ostream, const std::string &pad) const
{
	ostream << "(" << className() << ") { ";
	ostream << "\n" << pad << "ClassID: " << NLMISC::toString(ClassID);
	ostream << "\n" << pad << "SuperClassID: " << SuperClassID;
	ostream << " } ";
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
