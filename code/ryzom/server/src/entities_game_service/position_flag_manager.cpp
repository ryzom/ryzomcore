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
#include "position_flag_manager.h"

//net
#include "nel/net/service.h"

// misc
#include "nel/misc/hierarchical_timer.h"
#include "nel/misc/file.h"
#include "nel/misc/o_xml.h"
#include "nel/misc/i_xml.h"
// game_share
#include "game_share/utils.h"
#include "game_share/string_manager_sender.h"
// egs
#include "player_manager/character.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "zone_manager.h"


using namespace NLMISC;
using namespace NLNET;
using namespace std;


void CPositionFlagManager::setFlag(const std::string & flagName, const CFlagPosition & flagPos)
{
	_FlagPositions[flagName] = flagPos;
}

void CPositionFlagManager::removeFlag(const std::string & flagName)
{
	_FlagPositions.erase(flagName);
}

const CFlagPosition * CPositionFlagManager::getFlagPosition(const std::string & flagName) const
{
	TFlagPositionMap::const_iterator it = _FlagPositions.find(flagName);
	if (it != _FlagPositions.end())
	{
		return &(*it).second;
	}
	return NULL;
}

bool CPositionFlagManager::flagExists(const std::string & flagName) const
{
	return (_FlagPositions.find(flagName) != _FlagPositions.end());
}

void CPositionFlagManager::serial(NLMISC::IStream & f) throw(NLMISC::EStream)
{
	H_AUTO(CPositionFlagManagerSerial);

	f.xmlPushBegin("PositionFlags");

	f.xmlSetAttrib("size");

	uint32 len;
	if (f.isReading())
	{
		_FlagPositions.clear();
		f.serial(len);

		f.xmlPushEnd();

		for (uint i = 0; i < len; i++)
		{
			string flagName;
			sint32 x, y, z;

			try
			{
				f.xmlPushBegin("Flag");
				{
					f.xmlSetAttrib("name");
					f.serial(flagName);

					f.xmlSetAttrib("x");
					f.serial(x);

					f.xmlSetAttrib("y");
					f.serial(y);

					f.xmlSetAttrib("z");
					f.serial(z);
				}
				f.xmlPushEnd();
				f.xmlPop();
			}
			catch (const EStream &)
			{
				BOMB("<CPositionFlagManager::serial> invalid size or invalid flag", break);
			}

			_FlagPositions[flagName] = CFlagPosition(x, y, z);
		}
	}
	else
	{
		len = (uint32)_FlagPositions.size();
		f.serial(len);

		f.xmlPushEnd();

		TFlagPositionMap::iterator it;
		for (it = _FlagPositions.begin(); it != _FlagPositions.end(); ++it)
		{
			const string & flagName = (*it).first;
			CFlagPosition & flagPos = (*it).second;

			f.xmlPushBegin("Flag");
			{
				f.xmlSetAttrib("name");
				f.serial( const_cast<string &>(flagName) );

				f.xmlSetAttrib("x");
				f.serial(flagPos.X);

				f.xmlSetAttrib("y");
				f.serial(flagPos.Y);

				f.xmlSetAttrib("z");
				f.serial(flagPos.Z);
			}
			f.xmlPushEnd();
			f.xmlPop();
		}
	}

	f.xmlPop ();
}

void CPositionFlagManager::saveToFile(const std::string & fileName)
{
	CMemStream stream;
	try
	{
		COXml output;
		if (!output.init(&stream, "1.0"))
		{
			nlwarning("<CPositionFlagManager::saveToFile> cannot init XML output for file %s", fileName.c_str());
			return;
		}
		serial(output);
		output.flush();
	}
	catch (const Exception & e)
	{
		nlwarning("<CPositionFlagManager::saveToFile> cannot save file %s : %s", fileName.c_str(), e.what());
	}

	nlinfo("CPositionFlagManager::saveToFile: send message to BS");
	CBackupMsgSaveFile msg( fileName, CBackupMsgSaveFile::SaveFile, Bsi );
	msg.DataMsg.serialBuffer((uint8*)stream.buffer(), stream.length());
	Bsi.sendFile( msg );
}

struct CPositionFlagManagerFileLoadCallback: public IBackupFileReceiveCallback
{
	CPositionFlagManager* Parent;
	std::string FileName;

	CPositionFlagManagerFileLoadCallback(CPositionFlagManager* parent,const std::string& fileName): Parent(parent), FileName(fileName)  {}

	virtual void callback(const CFileDescription& fileDescription, NLMISC::IStream& dataStream)
	{
		// if the file isn't found then just give up
		DROP_IF(fileDescription.FileName.empty(),"<CPositionFlagManager::loadFromFile> file not found: "<< FileName, return);

		try
		{
			// setup an xml wrapper round the input stream
			CIXml input;
			DROP_IF(!input.init(dataStream),"<CPositionFlagManager::loadFromFile> cannot init XML input for file: "<< FileName, return);

			// serialise the input data (in xml format)
			Parent->serial(input);
		}
		catch (const Exception & e)
		{
			STOP("<CPositionFlagManager::loadFromFile> cannot parse file: "<< FileName << ": " << e.what());
		}
	}
};

void CPositionFlagManager::loadFromFile(const std::string & fileName)
{
	Bsi.syncLoadFile(fileName, new CPositionFlagManagerFileLoadCallback(this,fileName));
}

void CPositionFlagManager::sendFlagsList(const NLMISC::CEntityId & eid, bool shortFormat, uint32 radius) const
{
	CCharacter * c = PlayerManager.getChar(eid);
	if (c == NULL)
		return;

	SM_STATIC_PARAMS_1(params, STRING_MANAGER::literal);

	TFlagPositionMap::const_iterator it;
	for (it = _FlagPositions.begin(); it != _FlagPositions.end(); ++it)
	{
		const std::string & flagName = (*it).first;
		const CFlagPosition & flagPos = (*it).second;

		// check distance
		if (radius)
		{
			const double dx = flagPos.X - c->getState().X()/1000;
			const double dy = flagPos.Y - c->getState().Y()/1000;
			if ( sqr(dx) + sqr(dy) > sqr((double) radius) )
				continue;
		}

		if (shortFormat)
		{
			params[0].Literal.fromUtf8( NLMISC::toString("&SYS&<CSR> %s", flagName.c_str()) );
		}
		else
		{
			string regionDesc;
			const CContinent * continent = NULL;
			const CRegion * region = NULL;
			if (CZoneManager::getInstance().getRegion(flagPos.X*1000, flagPos.Y*1000, &region, &continent) && continent && region)
			{
				regionDesc = continent->getName() + ", " + region->getName();
			}

			params[0].Literal.fromUtf8( NLMISC::toString("&SYS&<CSR> %s : %d, %d, %d [%s]", flagName.c_str(), flagPos.X, flagPos.Y, flagPos.Z, regionDesc.c_str()) );
		}

		CCharacter::sendDynamicSystemMessage(eid, "LITERAL", params);
	}
}
