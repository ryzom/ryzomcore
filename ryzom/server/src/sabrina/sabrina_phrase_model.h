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




#ifndef RY_SABRINA_PHRASE_MODEL_H
#define RY_SABRINA_PHRASE_MODEL_H

#include "nel/misc/types_nl.h"
#include "game_share/egs_sheets/egs_static_brick.h"

#include "sabrina_messages.h"
#include "sabrina_pointers.h"
#include "sabrina_enum.h"
 
// advanced ref to actor base class 
class ISabrinaActor;

/**
 * Base virtual class for all Sabrina phrases
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
class ISabrinaPhraseModel : public NLMISC::CRefCount
{
public:
	/**
	 * add a brick to the phrase model
	 * \param brick : the brick to add
	 * \return true on success (if the phrase is valid)
	 */
	virtual bool addBrick( const CStaticBrick* brick ) = 0;

	/**
	 * test whether the phrase model is valid - should depend on validity of bricks added
	 * \return true if the phrase is valid
	 */
	virtual bool isValid( ) = 0;

	/**
	 * evaluate phrase for a given actor
	 * \param evalReturnInfos struct that will receive evaluation results
	 * \return true if eval has been made without errors
	 */
	virtual SABRINA::TEventCode evaluate(const CSabrinaPhraseInstance* phraseInstance, CEvalReturnInfos* msg = NULL) = 0;

	/**
	 * signal whether or not a target is required for the phrase
	 * \return true if the phrase requires a target
	 */
	virtual bool requiresTarget() = 0;

	/**
	 * validate phrase for a given actor on a given target
	 * \return true of the phrase is valid
	 */
	virtual SABRINA::TEventCode validate(const CSabrinaPhraseInstance* phraseInstance) = 0;

	/**
	 * execute this phrase (apply results to actors, etc)
	 */
	virtual uint32 calculatePreExecutionDelay(const CSabrinaPhraseInstance* phraseInstance) = 0;

	/**
	 * execute this phrase (apply results to actors, etc)
	 * \return true of the phrase was executed ok (phrase was still valid, etc)
	 */
	virtual SABRINA::TEventCode executeAndApplyResults(const CSabrinaPhraseInstance* phraseInstance) = 0;

	/**
	 * execute this phrase (apply results to actors, etc)
	 */
	virtual uint32 calculatePostExecutionDelay(const CSabrinaPhraseInstance* phraseInstance) = 0;

	// name property accessors
	std::string& name() { return _Name; }
	void setName(std::string& name) { _Name=name; }

private:
	// private data
	std::string _Name;
};

#endif
