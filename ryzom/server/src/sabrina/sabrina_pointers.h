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




#ifndef RY_SABRINA_POINTERS_H
#define RY_SABRINA_POINTERS_H

#include "nel/misc/smart_ptr.h"
 
/**
 * ISabrinaPhraseModel safe pointer
 * \author Sadge
 * \author Nevrax France
 * \date 2003
 */

class ISabrinaPhraseModel;
typedef NLMISC::CSmartPtr<ISabrinaPhraseModel> ISabrinaPhraseModelPtr;


/**
 * CSabrinaPhraseInstance safe pointer
 * \author Sadge
 * \author Nevrax France
 * \date 2003
 */

class CSabrinaPhraseInstance;
typedef NLMISC::CSmartPtr<CSabrinaPhraseInstance> CSabrinaPhraseInstancePtr;


/**
 * Sabrina actor base class safe pointer
 * \author Sadge
 * \author Nevrax France
 * \date 2003
 */

//class ISabrinaActor;
//typedef NLMISC::CSmartPtr<ISabrinaActor> ISabrinaActorPtr;


/**
 * ISabrinaPhraseDescription safe pointer
 * \author Sadge
 * \author Nevrax France
 * \date 2003
 */

class ISabrinaPhraseDescription;
typedef NLMISC::CSmartPtr<ISabrinaPhraseDescription> ISabrinaPhraseDescriptionPtr;


#endif
