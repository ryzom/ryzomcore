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


#include "stdnet.h"
#include "nel/net/module_message.h"

// a stupid function to remove some more stupid visual warnings
void foo_module_message() {}

namespace NLNET
{
//	CModuleMessage::CModuleMessage(const CMessage &messageBody)
//		: MessageType(mt_invalid),
//		SenderModuleId(INVALID_MODULE_ID),
//		AddresseeModuleId(INVALID_MODULE_ID),
//		MessageBody(const_cast<CMessage&>(messageBody))
//	{
//
//	}
//
//
//	void CModuleMessage::serial(NLMISC::IStream &s)
//	{
//		nlassert(mt_num_types < 0xFF);
//		nlassert(MessageType != mt_invalid);
//
//		s.serialBitField8(reinterpret_cast<uint8&>(MessageType));
//		s.serial(SenderModuleId);
//		s.serial(AddresseeModuleId);
//		MessageBody.serialMessage()
//	}


} // namespace NLNET
