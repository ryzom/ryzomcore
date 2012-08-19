/**
 * \file class_directory_3.cpp
 * \brief CClassDirectory3
 * \date 2012-08-18 18:01GMT
 * \author Jan Boon (Kaetemi)
 * CClassDirectory3
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
#include "class_directory_3.h"

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

CClassDirectory3::CClassDirectory3()
{

}

CClassDirectory3::~CClassDirectory3()
{

}

std::string CClassDirectory3::getClassName()
{
	return "ClassDirectory3";
}

void CClassDirectory3::toString(std::ostream &ostream, const std::string &pad)
{
	CStorageContainer::toString(ostream, pad);
}

void CClassDirectory3::parse(uint16 version, TParseLevel level)
{
	CStorageContainer::parse(version, level);
}

void CClassDirectory3::build(uint16 version)
{
	CStorageContainer::build(version);
}

IStorageObject *CClassDirectory3::createChunkById(uint16 id, bool container)
{
	if (container)
	{
		switch (id)
		{
		case 0x2040: // ClassEntry
			return new CClassEntry();
		}
	}
	return CStorageContainer::createChunkById(id, container);
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

CClassEntry::CClassEntry()
{

}

CClassEntry::~CClassEntry()
{

}

std::string CClassEntry::getClassName()
{
	return "ClassEntry";
}

void CClassEntry::toString(std::ostream &ostream, const std::string &pad)
{
	CStorageContainer::toString(ostream, pad);
}

void CClassEntry::parse(uint16 version, TParseLevel level)
{
	CStorageContainer::parse(version, level);
}

void CClassEntry::build(uint16 version)
{
	CStorageContainer::build(version);
}

IStorageObject *CClassEntry::createChunkById(uint16 id, bool container)
{
	if (!container)
	{
		switch (id)
		{
		case 0x2060: // ClassDirectoryHeader
			return new CClassDirectoryHeader();
		case 0x2042: // ClassName
			return new CStorageValue<ucstring>();
		}
	}
	return CStorageContainer::createChunkById(id, container);
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

CClassDirectoryHeader::CClassDirectoryHeader()
{

}

CClassDirectoryHeader::~CClassDirectoryHeader()
{

}

std::string CClassDirectoryHeader::getClassName()
{
	return "ClassDirectoryHeader";
}

void CClassDirectoryHeader::serial(NLMISC::IStream &stream)
{
	stream.serial(DllIndex);
	stream.serial(ClassID);
	stream.serial(SuperClassID);
}

void CClassDirectoryHeader::toString(std::ostream &ostream, const std::string &pad)
{
	ostream << "(" << getClassName() << ") { ";
	ostream << "\n" << pad << "DllIndex: " << DllIndex;
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
