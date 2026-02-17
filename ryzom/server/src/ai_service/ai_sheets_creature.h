// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2014  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef RYAI_SHEETS_CREATURE_H
#define RYAI_SHEETS_CREATURE_H

#include "sheets.h"

namespace AISHEETS {

/**
 * AI-service-specific creature sheet class.
 * Extends CCreature with script comp processing that depends on
 * ai_script_comp.cpp (CFightScriptCompReader, CFightSelectFilter).
 */
class CCreatureAI
: public CCreature
{
protected:
	void onScriptComp(const std::string &scriptCompStr);

public:
	void registerScriptComp(CFightScriptComp* scriptComp);
};
typedef NLMISC::CSmartPtr<CCreatureAI> CCreatureAIPtr;

/**
 * AI-service-specific sheets singleton.
 * Overrides packSheets to load CCreatureAI instead of CCreature,
 * enabling script comp processing without preprocessor hacks.
 */
class CSheetsAI
: public CSheets
{
public:
	void packSheets(const std::string &writeFilesDirectoryName);
	
	/// Register CSheetsAI as the singleton. Call before getInstance().
	static void initInstance();
};

}

#endif
