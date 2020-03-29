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


#ifndef R2_AIPRIMITIVEPARSER_H
#define R2_AIPRIMITIVEPARSER_H

//nel 
#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"
#include "nel/misc/singleton.h"
#include "nel/ligo/ligo_config.h"

//game_share
class CPersistentDataRecord;

namespace R2
{
	
class CAIPrimitiveParser : public NLMISC::CSingleton<CAIPrimitiveParser>
{

public:
	static void init(CPersistentDataRecord* pdr);	
	static void release();	

	void clear();
	void readFile(const std::string &fileName);
	void writeFile(const std::string &fileName);
	void display();
	void serial(NLMISC::IStream &stream);

};

}
#endif

