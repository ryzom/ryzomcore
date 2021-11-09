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
#include "action_generic.h"

//
// Using
//

using namespace std;

using namespace NLMISC;
using namespace NLNET;

namespace CLFECOMMON {

bool	CActionGeneric::ServerSide = false;

//
// Classes
//

void CActionGeneric::unpack (NLMISC::CBitMemStream &message)
{
	// Prepare _Message for output
	_Message.clear();
	if (!_Message.isReading())
		_Message.invert();

	// Read size from message, and check to	avoid hacking!
	uint32 size;
	message.serial (size);
	if ( size > 512 && ServerSide)
	{
		throw EInvalidDataStream();
	}

	// Write the data from message to _Message
	uint8 *ptr = _Message.bufferToFill(size);
	message.serialBuffer(ptr, size);

	//message.serial (_Message);
}

void CActionGeneric::pack (NLMISC::CBitMemStream &message)
{
	message.serialBufferWithSize((uint8*)_Message.buffer(), _Message.length());
//	message.serial (_Message);
}

void CActionGeneric::serial (NLMISC::IStream &/* f */)
{
//	f.serial (Position);
}

uint32 CActionGeneric::size ()
{
	// If you change this size, please update IMPULSE_ACTION_HEADER_SIZE in the front-end

	// in bits!!! (the message size and after the message itself)
	return (4 + _Message.length()) * 8;
}

void CActionGeneric::set (CBitMemStream &message)
{
	_Message = message;

	if (!_Message.isReading())
		_Message.invert ();
}

// Avoid to alloc a bitmemstream if not needed
void CActionGeneric::setFromMessage (CMemStream &message, uint32 bytelen)
{
	if (!_Message.isReading())
		_Message.invert ();

	nlassert( message.getPos() + bytelen <= message.length() );
	_Message.fill(message.buffer() + message.getPos(), bytelen);
}

CBitMemStream &CActionGeneric::get ()
{
	// when we get a the message, it s that you want to read it, so change the flux if needed
	if (!_Message.isReading())
		_Message.invert ();

	// reset the flux to the start
	_Message.resetBufPos();

	return _Message;
}

}


