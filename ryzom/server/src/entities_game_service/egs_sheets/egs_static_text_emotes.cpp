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
#include "egs_sheets/egs_static_text_emotes.h"
#include "nel/georges/u_form_elm.h"
#include "nel/georges/load_form.h"

#include "egs_sheets/egs_sheets.h"


using namespace NLMISC;
using namespace NLGEORGES;
using namespace std;



void CStaticTextEmotes::readGeorges (const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId)
{
	UFormElm &root = form->getRootNode();
	const UFormElm *pElt;
	nlverify (root.getNodeByName (&pElt, "Array"));
	uint size;
	nlverify (pElt->getArraySize (size));
	string value;
	_Phrases.reserve(size);
	for (uint32 i = 0; i < size; ++i)
	{
		const UFormElm *pEltOfList;
		// Get the continent
		if (pElt->getArrayNode (&pEltOfList, i) && pEltOfList)
		{
			_Phrases.push_back( CTextEmotePhrases() );
			nlverify( pEltOfList->getValueByName( _Phrases.back().EmoteId, "EmoteId" ) );
			nlverify( pEltOfList->getValueByName( _Phrases.back().TargetCrowd, "Phrase With Target To The Crowd" ) );
			nlverify( pEltOfList->getValueByName( _Phrases.back().TargetActor, "Phrase With Target To The Actor" ) );
			nlverify( pEltOfList->getValueByName( _Phrases.back().TargetTarget, "Phrase With Target To The Target" ) );
			nlverify( pEltOfList->getValueByName( _Phrases.back().NoTargetCrowd, "Phrase Without Target To The Crowd" ) );
			nlverify( pEltOfList->getValueByName( _Phrases.back().NoTargetActor, "Phrase Without Target To The Actor" ) );
			nlverify( pEltOfList->getValueByName( _Phrases.back().SelfActor, "Phrase Self Target To The Actor" ) );
			nlverify( pEltOfList->getValueByName( _Phrases.back().SelfCrowd, "Phrase Self Target To The Crowd" ) );
			nlverify( pEltOfList->getValueByName( _Phrases.back().UsableFromClientUI, "Usable From Client UI" ) );
			nlverify( pEltOfList->getValueByName( _Phrases.back().Animation, "Animation" ) );
			_Phrases.back().OnlyForFBT = (_Phrases.back().Animation == "FBT"); // hard coded
		}
	}
	buildEmoteIdMap();
}

void CStaticTextEmotes::buildEmoteIdMap()
{
	size_t size = _Phrases.size();
	for (size_t i=0; i<size; ++i)
		_EmoteIdMap.insert(make_pair(_Phrases[i].EmoteId, i));
}

MBEHAV::EBehaviour CStaticTextEmotes::getEmoteBehav(const std::string& emoteId) const
{
	std::map<std::string, size_t>::const_iterator it = _EmoteIdMap.find(emoteId);
	if (it!=_EmoteIdMap.end() && it->second<_Phrases.size())
		return (MBEHAV::EBehaviour)(MBEHAV::EMOTE_BEGIN + CSheets::getEmoteList().getAnimIndex(_Phrases[it->second].Animation));
	return MBEHAV::UNKNOWN_BEHAVIOUR;
}
