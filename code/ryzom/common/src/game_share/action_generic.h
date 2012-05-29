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



#ifndef NL_ACTION_GENERIC_H
#define NL_ACTION_GENERIC_H

#include "nel/misc/types_nl.h"

#include "action.h"


namespace CLFECOMMON {

/**
 * This action is used to forward client message to the back end via front end (and vice et versa)
 * \author Vianney Lecoart
 * \author Nevrax France
 * \date 2002
 */
class CActionGeneric : public CActionImpulsion
{
public:

	virtual ~CActionGeneric() {}

	/** This function creates initializes its fields using the buffer.
	 * \param buffer pointer to the buffer where the data are
	 * \size size of the buffer
	 */
	virtual void unpack (NLMISC::CBitMemStream &message);

	/// This functions is used when you want to transform an action into an IStream.
	virtual void serial (NLMISC::IStream &f);

	/// Returns the size of this action when will be send to the UDP connection (in number of bits.)
	virtual uint32 size ();

	virtual void set (NLMISC::CBitMemStream &message);

	/** Same as set(), avoiding the need to alloc a bitmemstream: copy from message.getPos()
	 *
	 * Preconditions:
	 * - message.getPos() + bytelen <= message.length()
	 */
	void setFromMessage (NLMISC::CMemStream &message, uint32 bytelen);

	virtual NLMISC::CBitMemStream &get ();

	static CAction *create () { return new CActionGeneric(); }

	static bool		ServerSide;

protected:

	/** This function transform the internal field and transform them into a buffer for the UDP connection.
	 * \param buffer pointer to the buffer where the data will be written
	 * \size size of the buffer
	 */
	virtual void pack (NLMISC::CBitMemStream &message);

	/// This method intialises the action with a default state
	virtual void reset()
	{
		AllowExceedingMaxSize = false;
	}

	friend class CActionFactory;

	NLMISC::CBitMemStream _Message;
};

}

#endif // NL_ACTION_GENERIC_H

/* End of action_generic.h */
