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



#ifndef NL_ACTION_DUMMY_H
#define NL_ACTION_DUMMY_H

#include "nel/misc/types_nl.h"
#include "action.h"



namespace CLFECOMMON {

/**
 * Dummy action for test purpose
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2001
 */
class CActionDummy : public CAction
{
public:

	/** This function creates initializes its fields using the buffer.
	 * \param buffer pointer to the buffer where the data are
	 * \size size of the buffer
	 */
	virtual void unpack (NLMISC::CBitMemStream &message) { message.serial( Dummy1 ); message.serial( Dummy2 ); }

	/// This functions is used when you want to transform an action into an IStream.
	virtual void serial (NLMISC::IStream &f) { f.serial( Dummy1 ); f.serial( Dummy2 ); }

	/** Returns the size of this action when it will be send to the UDP connection:
	 * the size is IN BITS, not in bytes (the actual size is this one plus the header size)
	 */
	virtual uint32 size () { return sizeof(Dummy1)*8 + sizeof(Dummy2)*8; }

	static CAction *create () { return new CActionDummy(); }

	uint32 Dummy1;
	uint32 Dummy2;

protected:

	/** This function transform the internal field and transform them into a buffer for the UDP connection.
	 * \param buffer pointer to the buffer where the data will be written
	 * \size size of the buffer
	 */
	virtual void pack (NLMISC::CBitMemStream &message) { message.serial( Dummy1 ); message.serial( Dummy2 ); }

	friend class CActionFactory;
};

}

#endif // NL_ACTION_DUMMY_H

/* End of action_dummy.h */
