/**
 * \file dll_directory.cpp
 * \brief CDllDirectory
 * \date 2012-08-18 09:01GMT
 * \author Jan Boon (Kaetemi)
 * CDllDirectory
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
#include "dll_directory.h"

// STL includes

// NeL includes
// #include <nel/misc/debug.h>

// Project includes

using namespace std;
// using namespace NLMISC;

namespace PIPELINE {
namespace MAX {

CDllDirectory::CDllDirectory()
{

}

CDllDirectory::~CDllDirectory()
{

}

std::string CDllDirectory::getClassName()
{
	return "DllDirectory";
}

void CDllDirectory::toString(std::ostream &ostream, const std::string &pad)
{
	CStorageContainer::toString(ostream, pad);
}

void CDllDirectory::parse(uint16 version, TParseLevel level)
{
	CStorageContainer::parse(version, level);
}

void CDllDirectory::build(uint16 version)
{
	CStorageContainer::build(version);
}

IStorageObject *CDllDirectory::createChunkById(uint16 id, bool container)
{
	if (container)
	{
		switch (id)
		{
		case 0x2038: // DllEntry
			return new CDllEntry();
		}
	}
	else
	{
		switch (id)
		{
		case 0x21C0: // DllDirectoryHeader
			return new CStorageValue<uint32>();
		}
	}
	return CStorageContainer::createChunkById(id, container);
}

CDllEntry::CDllEntry()
{

}

CDllEntry::~CDllEntry()
{

}

std::string CDllEntry::getClassName()
{
	return "DllEntry";
}

void CDllEntry::toString(std::ostream &ostream, const std::string &pad)
{
	CStorageContainer::toString(ostream, pad);
}

void CDllEntry::parse(uint16 version, TParseLevel level)
{
	CStorageContainer::parse(version, level);
}

void CDllEntry::build(uint16 version)
{
	CStorageContainer::build(version);
}

IStorageObject *CDllEntry::createChunkById(uint16 id, bool container)
{
	if (!container)
	{
		switch (id)
		{
		case 0x2039: // DllDescription
		case 0x2037: // DllFilename
			return new CStorageValue<ucstring>();
		}
	}
	return CStorageContainer::createChunkById(id, container);
}

} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
