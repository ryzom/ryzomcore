/**
 * \file config.cpp
 * \brief CConfig
 * \date 2012-08-18 19:25GMT
 * \author Jan Boon (Kaetemi)
 * CConfig
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
#include "config.h"

// STL includes

// NeL includes
// #include <nel/misc/debug.h>
#include <nel/misc/vector.h>

// Project includes

using namespace std;
// using namespace NLMISC;

namespace PIPELINE {
namespace MAX {

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

CConfig::CConfig()
{

}

CConfig::~CConfig()
{

}

std::string CConfig::className() const
{
	return "Config";
}

void CConfig::toString(std::ostream &ostream, const std::string &pad) const
{
	CStorageContainer::toString(ostream, pad);
}

void CConfig::parse(uint16 version, uint filter)
{
	CStorageContainer::parse(version);
}

void CConfig::clean()
{
	CStorageContainer::clean();
}

void CConfig::build(uint16 version, uint filter)
{
	CStorageContainer::build(version);
}

void CConfig::disown()
{
	CStorageContainer::disown();
}

IStorageObject *CConfig::createChunkById(uint16 id, bool container)
{
	if (container)
	{
		switch (id)
		{
		case 0x20a0: // unknown
			return new CConfig20a0();
		case 0x2180: // CConfigScript
			return new CConfigScript();
		}
	}
	else
	{
		switch (id)
		{
		case 0x2090: // unknown; known values: 1 (m3), 0 (m9, m2008, m2010)
			return new CStorageValue<sint32>();
		}
	}
	return CStorageContainer::createChunkById(id, container);
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

CConfig20a0::CConfig20a0()
{

}

CConfig20a0::~CConfig20a0()
{

}

std::string CConfig20a0::className() const
{
	return "Config20a0";
}

void CConfig20a0::toString(std::ostream &ostream, const std::string &pad) const
{
	CStorageContainer::toString(ostream, pad);
}

void CConfig20a0::parse(uint16 version, uint filter)
{
	CStorageContainer::parse(version);
}

void CConfig20a0::clean()
{
	CStorageContainer::clean();
}

void CConfig20a0::build(uint16 version, uint filter)
{
	CStorageContainer::build(version);
}

void CConfig20a0::disown()
{
	CStorageContainer::disown();
}

IStorageObject *CConfig20a0::createChunkById(uint16 id, bool container)
{
	if (container)
	{
		switch (id)
		{
		case 0x0110: // CConfig20a0Entry
			return new CConfig20a0Entry();
		}
	}
	else
	{
		switch (id)
		{
		case 0x0100: // CConfig20a0Header: Number of entries
			return new CStorageValue<sint32>();
		}
	}
	return CStorageContainer::createChunkById(id, container);
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

CConfig20a0Entry::CConfig20a0Entry()
{

}

CConfig20a0Entry::~CConfig20a0Entry()
{

}

std::string CConfig20a0Entry::className() const
{
	return "Config20a0Entry";
}

void CConfig20a0Entry::toString(std::ostream &ostream, const std::string &pad) const
{
	CStorageContainer::toString(ostream, pad);
}

void CConfig20a0Entry::parse(uint16 version, uint filter)
{
	CStorageContainer::parse(version);
}

void CConfig20a0Entry::clean()
{
	CStorageContainer::clean();
}

void CConfig20a0Entry::build(uint16 version, uint filter)
{
	CStorageContainer::build(version);
}

void CConfig20a0Entry::disown()
{
	CStorageContainer::disown();
}

IStorageObject *CConfig20a0Entry::createChunkById(uint16 id, bool container)
{
	if (container)
	{
		switch (id)
		{
		case 0x0300: // unknown, contains 0x0100 with 4 bytes int value
			return CStorageContainer::createChunkById(id, container);
		}
	}
	else
	{
		switch (id)
		{
		case 0x0100: // unknown
		case 0x0110: // unknown
		case 0x0120: // unknown
		case 0x0130: // unknown
		case 0x0140: // unknown
		case 0x0150: // unknown
		case 0x0160: // unknown
		case 0x0161: // unknown
		case 0x0170: // unknown
		case 0x0180: // unknown
		case 0x0190: // unknown
		case 0x0200: // unknown
		case 0x0210: // unknown
		case 0x0220: // unknown
		case 0x0230: // unknown
		case 0x0240: // unknown
		case 0x0250: // unknown
		case 0x0270: // unknown
		case 0x0280: // unknown
		case 0x0310: // unknown
		case 0x0320: // unknown
			return new CStorageValue<sint32>();
		case 0x0260: // unknown
			return new CStorageValue<sint64>();
		case 0x0330: // unknown, 16 bytes
			return CStorageContainer::createChunkById(id, container);
		case 0x0290: // unknown
		case 0x0390: // unknown
			return new CStorageValue<ucstring>();
		}
	}
	return CStorageContainer::createChunkById(id, container);
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

CConfigScript::CConfigScript()
{

}

CConfigScript::~CConfigScript()
{

}

std::string CConfigScript::className() const
{
	return "ConfigScript";
}

void CConfigScript::toString(std::ostream &ostream, const std::string &pad) const
{
	CStorageContainer::toString(ostream, pad);
}

void CConfigScript::parse(uint16 version, uint filter)
{
	CStorageContainer::parse(version);
}

void CConfigScript::clean()
{
	CStorageContainer::clean();
}

void CConfigScript::build(uint16 version, uint filter)
{
	CStorageContainer::build(version);
}

void CConfigScript::disown()
{
	CStorageContainer::disown();
}

IStorageObject *CConfigScript::createChunkById(uint16 id, bool container)
{
	if (container)
	{
		switch (id)
		{
		case 0x0040: // ConfigScriptEntry
			return new CConfigScriptEntry();
		}
	}
	return CStorageContainer::createChunkById(id, container);
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

CConfigScriptEntry::CConfigScriptEntry()
{

}

CConfigScriptEntry::~CConfigScriptEntry()
{

}

std::string CConfigScriptEntry::className() const
{
	return "ConfigScriptEntry";
}

void CConfigScriptEntry::toString(std::ostream &ostream, const std::string &pad) const
{
	CStorageContainer::toString(ostream, pad);
}

void CConfigScriptEntry::parse(uint16 version, uint filter)
{
	CStorageContainer::parse(version);
}

void CConfigScriptEntry::clean()
{
	CStorageContainer::clean();
}

void CConfigScriptEntry::build(uint16 version, uint filter)
{
	CStorageContainer::build(version);
}

void CConfigScriptEntry::disown()
{
	CStorageContainer::disown();
}

IStorageObject *CConfigScriptEntry::createChunkById(uint16 id, bool container)
{
	if (container)
	{
		switch (id)
		{
		case 0x0007: // ConfigScriptMetaContainer
			return new CConfigScriptMetaContainer();
		}
	}
	else
	{
		switch (id)
		{
		case 0x0050: // ConfigScriptHeader
			return new CConfigScriptHeader();
		}
	}
	return CStorageContainer::createChunkById(id, container);
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

CConfigScriptHeader::CConfigScriptHeader()
{

}

CConfigScriptHeader::~CConfigScriptHeader()
{

}

std::string CConfigScriptHeader::className() const
{
	return "ConfigScriptHeader";
}

void CConfigScriptHeader::serial(NLMISC::IStream &stream)
{
	stream.serial(SuperClassID);
	stream.serial(ClassID);
}

void CConfigScriptHeader::toString(std::ostream &ostream, const std::string &pad) const
{
	ostream << "(" << className() << ") { ";
	ostream << "\n" << pad << "SuperClassID: " << SuperClassID;
	ostream << "\n" << pad << "ClassID: " << NLMISC::toString(ClassID);
	ostream << " } ";
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

CConfigScriptMetaContainer::CConfigScriptMetaContainer()
{

}

CConfigScriptMetaContainer::~CConfigScriptMetaContainer()
{

}

std::string CConfigScriptMetaContainer::className() const
{
	return "ConfigScriptMetaContainer";
}

void CConfigScriptMetaContainer::toString(std::ostream &ostream, const std::string &pad) const
{
	CStorageContainer::toString(ostream, pad);
}

void CConfigScriptMetaContainer::parse(uint16 version, uint filter)
{
	CStorageContainer::parse(version);
}

void CConfigScriptMetaContainer::clean()
{
	CStorageContainer::clean();
}

void CConfigScriptMetaContainer::build(uint16 version, uint filter)
{
	CStorageContainer::build(version);
}

void CConfigScriptMetaContainer::disown()
{
	CStorageContainer::disown();
}

IStorageObject *CConfigScriptMetaContainer::createChunkById(uint16 id, bool container)
{
	if (container)
	{
		switch (id)
		{
		case 0x0007: // ConfigScriptMetaContainer
			return new CConfigScriptMetaContainer();
		}
	}
	else
	{
		switch (id)
		{
		case 0x0001: // type: boolean (stored in four bytes, yes)
			return new CStorageValue<uint32>();
		case 0x0002: // ???
			nlerror("0x0002 found, please implement");
			break;
		case 0x0003: // type: integer (not sure if signed)
			return new CStorageValue<sint32>();
		case 0x0004: // type: float
			return new CStorageValue<float>();
		case 0x0005: // type: string (same format as the meta string)
		case 0x0006: // ConfigScriptMetaString
			return new CConfigScriptMetaString();
		case 0x0008: // type: color (stored as 3 float vector)
			return new CStorageValue<NLMISC::CVector>;
		case 0x0060: // ConfigScriptMetaContainerHeader (contains the number of entries, amusingly)
			return new CStorageValue<uint32>();
		}
	}
	return CStorageContainer::createChunkById(id, container);
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

CConfigScriptMetaString::CConfigScriptMetaString()
{

}

CConfigScriptMetaString::~CConfigScriptMetaString()
{

}

std::string CConfigScriptMetaString::className() const
{
	return "ConfigScriptMetaString";
}

void CConfigScriptMetaString::serial(NLMISC::IStream &stream)
{
	if (stream.isReading())
	{
		uint32 size;
		stream.serial(size);
		Value.resize(size - 1);
	}
	else
	{
		uint32 size = Value.size() + 1;
		stream.serial(size);
	}
	stream.serialBuffer(static_cast<uint8 *>(static_cast<void *>(&Value[0])), Value.size());
	uint8 endByte = 0;
	stream.serial(endByte);
	nlassert(endByte == 0);
}

void CConfigScriptMetaString::toString(std::ostream &ostream, const std::string &pad) const
{
	ostream << "(" << className() << ") { " << Value << " } ";
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
