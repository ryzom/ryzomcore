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

#include "ios_pd.h"

namespace IOSPD
{
	
RY_PDS::CPDSLib	PDSLib;
void							init(uint32 overrideDbId)
{
	// Init CreateTeam log message and parameters
	PDSLib.initLog(0);
	PDSLib.initLogParam(0, 0, 8);
	// Init DeleteTeam log message and parameters
	PDSLib.initLog(1);
	PDSLib.initLogParam(1, 0, 8);
	// Init PlayerJoinsTeam log message and parameters
	PDSLib.initLog(2);
	PDSLib.initLogParam(2, 0, 8);
	PDSLib.initLogParam(2, 1, 8);
	// Init PlayerLeavesTeam log message and parameters
	PDSLib.initLog(3);
	PDSLib.initLogParam(3, 0, 8);
	PDSLib.initLogParam(3, 1, 8);
	std::string	xmlDescription;
	xmlDescription += "<?xml version='1.0'?>\n";
	xmlDescription += "<dbdescription version='0.0'>\n";
	xmlDescription += "<db name='IOSPD' types='15' classes='0'>\n";
	xmlDescription += "<typedef name='bool' id='0' size='1' storage='bool' type='type'/>\n";
	xmlDescription += "<typedef name='char' id='1' size='1' storage='char' type='type'/>\n";
	xmlDescription += "<typedef name='ucchar' id='2' size='2' storage='ucchar' type='type'/>\n";
	xmlDescription += "<typedef name='uint8' id='3' size='1' storage='uint8' type='type'/>\n";
	xmlDescription += "<typedef name='sint8' id='4' size='1' storage='sint8' type='type'/>\n";
	xmlDescription += "<typedef name='uint16' id='5' size='2' storage='uint16' type='type'/>\n";
	xmlDescription += "<typedef name='sint16' id='6' size='2' storage='sint16' type='type'/>\n";
	xmlDescription += "<typedef name='uint32' id='7' size='4' storage='uint32' type='type'/>\n";
	xmlDescription += "<typedef name='sint32' id='8' size='4' storage='sint32' type='type'/>\n";
	xmlDescription += "<typedef name='uint64' id='9' size='8' storage='uint64' type='type'/>\n";
	xmlDescription += "<typedef name='sint64' id='10' size='8' storage='sint64' type='type'/>\n";
	xmlDescription += "<typedef name='float' id='11' size='4' storage='float' type='type'/>\n";
	xmlDescription += "<typedef name='double' id='12' size='8' storage='double' type='type'/>\n";
	xmlDescription += "<typedef name='CEntityId' id='13' size='8' storage='CEntityId' type='type'/>\n";
	xmlDescription += "<typedef name='CSheetId' id='14' size='4' storage='CSheetId' type='type'/>\n";
	xmlDescription += "<logmsg id='0' context='false'>\n";
	xmlDescription += "<param id='0' typeid='13'/>\n";
	xmlDescription += "<msg>Player team $0 created</msg>\n";
	xmlDescription += "</logmsg>\n";
	xmlDescription += "<logmsg id='1' context='false'>\n";
	xmlDescription += "<param id='0' typeid='13'/>\n";
	xmlDescription += "<msg>Player team $0 deleted</msg>\n";
	xmlDescription += "</logmsg>\n";
	xmlDescription += "<logmsg id='2' context='false'>\n";
	xmlDescription += "<param id='0' typeid='13'/>\n";
	xmlDescription += "<param id='1' typeid='13'/>\n";
	xmlDescription += "<msg>Player $0 joins team $1</msg>\n";
	xmlDescription += "</logmsg>\n";
	xmlDescription += "<logmsg id='3' context='false'>\n";
	xmlDescription += "<param id='0' typeid='13'/>\n";
	xmlDescription += "<param id='1' typeid='13'/>\n";
	xmlDescription += "<msg>Player $0 leaves team $1</msg>\n";
	xmlDescription += "</logmsg>\n";
	xmlDescription += "</db>\n";
	xmlDescription += "</dbdescription>\n";
	PDSLib.init(xmlDescription, overrideDbId);
}

bool							ready()
{
	return PDSLib.PDSReady();
}

void							update()
{
	PDSLib.update();
}

void							logChat(const ucstring& sentence, const NLMISC::CEntityId& from, const std::vector<NLMISC::CEntityId>& to)
{
	PDSLib.logChat(sentence, from, to);
}

void							logTell(const ucstring& sentence, const NLMISC::CEntityId& from, const NLMISC::CEntityId& to)
{
	std::vector<NLMISC::CEntityId>	ids;
	ids.push_back(to);
	PDSLib.logChat(sentence, from, ids);
}

void							release()
{
	PDSLib.release();
}

	
} // End of IOSPD
