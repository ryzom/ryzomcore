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



#ifndef NL_ACTION_GENERIC_MULTI_PART_H
#define NL_ACTION_GENERIC_MULTI_PART_H

#include "nel/misc/types_nl.h"

#include <vector>

#include "action.h"


namespace CLFECOMMON {

/**
 * ActionGenericMultiPart: spreading a block over several sends
 * \author Nevrax France
 * \date 2001
 */
class CActionGenericMultiPart: public CActionImpulsion
{
public:

	virtual ~CActionGenericMultiPart() {}

	/** This function creates initializes its fields using the buffer.
	 * \param buffer pointer to the buffer where the data are
	 * \size size of the buffer
	 */
	virtual void unpack (NLMISC::CBitMemStream &message)
	{
		message.serial (Number);
		message.serial (Part);
		message.serial (NbBlock);

		uint32	size;
		message.serial(size);
// The following test removed by Sadge because it appears to be pointless and prevents ^2 testing to continue as required
//		if ( size > 512 )
//		{
//			throw NLMISC::EInvalidDataStream();
//		}

		PartCont.resize(size);
		if (size > 0)
			message.serialBuffer(&(PartCont[0]), size);
	}

	/** Returns the size of this action when it will be send to the UDP connection:
	 * the size is IN BITS, not in bytes (the actual size is this one plus the header size)
	 */
	virtual uint32	size ()
	{
		uint32	bytesize = 1 + 2 + 2 + 4;	// header
		bytesize += (uint32)PartCont.size();
		return bytesize*8;
	}

	static CAction *create () { return new CActionGenericMultiPart(); }

	// set(): vector version (size are in BYTES)
	void set (uint8 number, uint16 part, std::vector<uint8> &v, uint32 size, uint16 nbBlock)
	{
		reset ();
		uint32 start = part*size;
		uint32 end = start + size;
		if (end > v.size())
			end = (uint32)v.size();
		PartCont.resize (end-start);
		std::copy (v.begin()+start, v.begin()+end, PartCont.begin());

		Number = number;
		Part = part;
		NbBlock = nbBlock;
	}

	/**
	 * set(): uint8* version (to match with sendImpulsion() optimisation) (size are in BYTES)
	 *
	 * Preconditions:
	 * - size != 0
	 */
	void set (uint8 number, uint16 part, const uint8 *buffer, uint32 bytelen, uint32 size, uint16 nbBlock)
	{
		//nlassert( size != 0 ); // => PartCont won't be resized to 0
		reset ();
		uint32 start = part*size;
		uint32 end = start + size;
		if (end > bytelen)
			end = bytelen;
		PartCont.resize (end-start);
		memcpy( &PartCont[0], buffer + start, end - start );

		Number = number;
		Part = part;
		NbBlock = nbBlock;
	}

	std::vector<uint8> PartCont;
	uint8	Number;
	uint16	Part, NbBlock;

protected:

	/** This function transform the internal field and transform them into a buffer for the UDP connection.
	 * \param buffer pointer to the buffer where the data will be written
	 * \size size of the buffer
	 */
	virtual void pack (NLMISC::CBitMemStream &message)
	{
		message.serial (Number);
		message.serial (Part);
		message.serial (NbBlock);
		message.serialCont (PartCont);
	}

	virtual void	reset()
	{
		PartCont.clear ();
		AllowExceedingMaxSize = false;
	}

	friend class CActionFactory;

};

}

#endif // NL_ACTION_GENERIC_MULTI_PART_H

/* End of action_generic_multi_part.h */
