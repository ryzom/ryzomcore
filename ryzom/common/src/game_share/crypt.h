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

#ifndef NL_CRYPT_H
#define NL_CRYPT_H

#include "nel/misc/types_nl.h"

#include <string>

/**
 * Encapsulation of the classic Unix crypt function
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2003
 */
class CCrypt
{
public:

	/// Crypts password using salt
	static std::string crypt(const std::string& password, const std::string& salt);

};


#endif // NL_CRYPT_H

/* End of crypt.h */
