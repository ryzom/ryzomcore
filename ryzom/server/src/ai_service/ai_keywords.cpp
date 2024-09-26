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
#include "ai_keywords.h"
#include "nel/net/service.h"

using namespace NLMISC;
using namespace NLNET;

// singleton data instantiation

CKeywordSet CAIKeywords::_botKeywords;
CKeywordSet CAIKeywords::_groupKeywords;
CKeywordSet CAIKeywords::_stateKeywords;

void CAIKeywords::init()
{
	updateKeywordsFromCfg();
}

static void loadKeywordsFromCfg(const char *varName,CKeywordSet &wordSet)
{
	// get hold of the config file variable
	CConfigFile::CVar *varPtr;
	varPtr=IService::getInstance()->ConfigFile.getVarPtr(std::string(varName));
	if	(!varPtr)
	{
		nlwarning("WARNING: Config file variable: %s not found",varName);
		return;
	}

	// add config file variable contents to keyword set
	for (uint32 i=0;i<varPtr->size();++i)
		wordSet.addKeywords(varPtr->asString(i));
}

void CAIKeywords::updateKeywordsFromCfg()
{
	loadKeywordsFromCfg("KeywordsGroupNpc",_groupKeywords);
	loadKeywordsFromCfg("KeywordsBotNpc",_botKeywords);
	loadKeywordsFromCfg("KeywordsStateNpc",_stateKeywords);
}

void CAIKeywords::display()
{
	nlinfo ("bot keywords: %s",_botKeywords.toString().c_str());
	nlinfo ("group keywords: %s",_groupKeywords.toString().c_str());
	nlinfo ("state keywords: %s",_stateKeywords.toString().c_str());
}

