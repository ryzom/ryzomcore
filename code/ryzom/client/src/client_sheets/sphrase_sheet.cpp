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
#include "sphrase_sheet.h"
#include "nel/georges/u_form_elm.h"
#include "nel/misc/common.h"

using namespace std;
using namespace NLMISC;
using namespace NLGEORGES;


// Easy macro to translate value from georges
#define TRANSLATE_VAL( _Var_, _key_ )						\
	if(!root.getValueByName(_Var_, _key_))					\
		debug( string("key '") + string(_key_) + string("' not found.") );


// ***************************************************************************
void CSPhraseSheet::build (const NLGEORGES::UFormElm &root)
{
	string sTmp, sTmp2;

	uint i;
	for (i = 0; i < SPHRASE_MAX_BRICK; ++i)
	{
		sTmp2 = string("brick ") + toString(i);
		root.getValueByName (sTmp, sTmp2.c_str());
		if (!sTmp.empty())
		{
			CSheetId id(sTmp);
			Bricks.push_back(id);
		}
	}

	// read castable
	TRANSLATE_VAL(Castable, "castable");

	// read ShowInActionProgression
	TRANSLATE_VAL(ShowInActionProgression, "ShowInActionProgression");

	// read ShowInAPOnlyIfLearnt
	TRANSLATE_VAL(ShowInAPOnlyIfLearnt, "ShowInAPOnlyIfLearnt");

}

// ***************************************************************************
bool	CSPhraseSheet::isValid() const
{
	if(Bricks.empty())
		return false;
	for(uint i=0;i<Bricks.size();i++)
	{
		if(Bricks[i]==CSheetId::Unknown)
			return false;
	}

	return true;
}

