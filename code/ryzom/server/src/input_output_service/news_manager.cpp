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
#include "news_manager.h"
#include "input_output_service.h"

//#include "game_share/generic_msg_mngr.h"
#include "game_share/msg_client_server.h"
#include "game_share/news_types.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;

class CNewsEntry
{
public:
	CNewsEntry () : Type(NEWSTYPE::Unknown) { }
	CNewsEntry (NEWSTYPE::TNewsType type, const string &stringId, const vector<uint64> &args) : Type(type), StringId(StringId), Args(args) { }

	NEWSTYPE::TNewsType		Type;
	string			StringId;
	vector<uint64>	Args;
};

deque<CNewsEntry> News;

static void cbAddNews (CMessage& msgin, const std::string &serviceName, TServiceId serviceId )
{
	NEWSTYPE::TNewsType type = NEWSTYPE::Unknown;
	string stringId;
	vector<uint64> args;
	
	msgin.serialEnum (type);
	msgin.serial (stringId);
	msgin.serialCont (args);

	News.push_back(CNewsEntry(type, stringId, args));

	nlinfo ("added news %s for type %d from service %s", stringId.c_str(), type, serviceName.c_str());
}


static TUnifiedCallbackItem CbArray[]=
{
	{ "ADD_NEWS", cbAddNews },
};


void CNewsManager::init()
{
	CUnifiedNetwork::getInstance()->addCallbackArray(CbArray, sizeof (CbArray) / sizeof (CbArray[0]));
}


void CNewsManager::getNews (NEWSTYPE::TNewsType type, CBitMemStream &bms)
{
	nlassert (type != NEWSTYPE::Unknown);

	sint val = (sint)frand ((float)News.size());

	string res;
	//CChatManager::getStaticDB ().getInfos (News[val].stringId, res, bms);
	nlinfo ("sending news '%s' '%s' with %d args", News[val].StringId.c_str(), res.c_str(), News[val].Args.size());

	//bms.serial (News[val].stringId);
	bms.serialCont (News[val].Args);
}
