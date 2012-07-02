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




#ifndef RY_SABRINA_PHRASE_MODEL_CRAFT_H
#define RY_SABRINA_PHRASE_MODEL_CRAFT_H

#include "sabrina_phrase_model.h"
 
/**
 * Base virtual class for all Sabrina phrases
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
class CSabrinaPhraseModelCraft : public ISabrinaPhraseModel
{
public:
	//--------------------------------------------------------------------
	// specialisation of virtual methods from ISabrinaPhraseModel

	virtual bool addBrick( const CStaticBrick* brick ) 
	{
		return true;	// TODO
	}

	virtual bool isValid( ) 
	{
		return true;	// TODO
	}

	virtual SABRINA::TEventCode evaluate(const CSabrinaPhraseInstance* phraseInstance, CEvalReturnInfos* msg = NULL) 
	{
		return SABRINA::SuccessNormal;	// TODO
	}

	virtual bool requiresTarget() 
	{
		return false;
	}

	virtual SABRINA::TEventCode validate(const CSabrinaPhraseInstance* phraseInstance) 
	{
		return SABRINA::SuccessNormal;	// TODO
	}

	virtual uint32 calculatePreExecutionDelay(const CSabrinaPhraseInstance* phraseInstance) 
	{
		return 10;		// TODO
	}

	virtual SABRINA::TEventCode executeAndApplyResults(const CSabrinaPhraseInstance* phraseInstance) 
	{
		return SABRINA::SuccessNormal;	// TODO
	}

	virtual uint32 calculatePostExecutionDelay(const CSabrinaPhraseInstance* phraseInstance) 
	{
		return 0;		// TODO
	}
};

#endif
