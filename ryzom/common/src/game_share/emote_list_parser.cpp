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
#include "emote_list_parser.h"
#include "nel/georges/u_form_loader.h"
#include "nel/georges/u_form_elm.h"
#include "nel/georges/u_form.h"
#include "nel/misc/smart_ptr.h"

using namespace	std;
using namespace	NLMISC;
using namespace	NLGEORGES;

namespace EMOTE_LIST_PARSER
{

bool initEmoteList( std::map<std::string, uint32>& 	emoteContainer )
{
	// load the emots names config files
	UFormLoader *formLoader = UFormLoader::createLoader();
	if (formLoader)
	{
		CSmartPtr<UForm> emotList = formLoader->loadForm("list.emot");
		if(emotList)
		{
			const UFormElm *list = 0;
			emotList->getRootNode().getNodeByName(&list, "emot_list");
			if(list)
			{
				// Get the array size.
				uint size;
				list->getArraySize(size);
				emoteContainer.clear();
				// Get emots name.
				for(uint i=0; i<size; ++i)
				{
					string result;
					list->getArrayValue(result, i);
					emoteContainer.insert(make_pair(result, i));
				}
			}
			else
			{
				nlwarning("<initEmoteList>Cannot find the key 'emot_list'.");
				delete formLoader;
				return false;
			}
		}
		else
		{
			nlwarning("<initEmoteList>Cannot load the form 'list.emot'.");
			delete formLoader;
			return false;
		}
	}
	else
	{
		nlwarning("<initEmoteList>Can't create a form loader ! emots will not be available.");
		return false;
	}
	UFormLoader::releaseLoader(formLoader);
	return true;
}

}
