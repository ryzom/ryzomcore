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




#ifndef RY_SABRINA_PHRASE_MODEL_FACTORY_H
#define RY_SABRINA_PHRASE_MODEL_FACTORY_H

// stl
#include <vector>
// nel
#include "nel/misc/types_nl.h"
#include "nel/misc/sheet_id.h"
// game share
#include "game_share/egs_sheets/egs_static_brick.h"
// sabrina
#include "sabrina_pointers.h"
 

/**
 * Factory class for sabrina phrases
 * \author Sadge
 * \author Nevrax France
 * \date 2003
 */
class CSabrinaPhraseModelFactory
{
public:
	/**
	 * Singleton factory method
	 * \param bricks : bricks composing the phrase
	 * \return smart pointer to new phrase object or NULL on failure
	 */
	static ISabrinaPhraseModelPtr newPhraseModel(const std::vector< const CStaticBrick* >& bricks);

	/**
	 * Singleton factory method
	 * \param bricks : bricks composing the phrase
	 * \return smart pointer to new phrase object or NULL on failure
	 */
	static ISabrinaPhraseModelPtr newPhraseModel(const std::vector< NLMISC::CSheetId >& bricks);

	/**
	 * Singleton factory method
	 * \param bricks : bricks composing the phrase
	 * \return smart pointer to new phrase object or NULL on failure
	 */
	static ISabrinaPhraseModelPtr newPhraseModel(const std::vector< std::string >& bricks);

};

#endif
