// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#ifndef NL_TDS_H
#define NL_TDS_H

#include "types_nl.h"


namespace NLMISC
{


/**
 * Thread dependant storage class
 *
 * This class provides a thread specific (void*).
 * It is initialized at NULL.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2002
 */
class CTDS
{
public:

	/// Constructor. The pointer is initialized with NULL.
	CTDS ();

	/// Destructor
	~CTDS ();

	/// Get the thread specific pointer
	void	*getPointer () const;

	/// Set the thread specific pointer
	void	setPointer (void* pointer);

private:
#ifdef NL_OS_WINDOWS
	uint32			_Handle;
#else // NL_OS_WINDOWS
	pthread_key_t	_Key;
#endif // NL_OS_WINDOWS

};


} // NLMISC


#endif // NL_TDS_H

/* End of tds.h */
