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



#ifndef NL_SYSTEM_MESSAGE_H
#define NL_SYSTEM_MESSAGE_H

#include "nel/misc/types_nl.h"

namespace CLFECOMMON {

const uint8				SYSTEM_LOGIN_CODE = 0;				// From client
const uint8				SYSTEM_SYNC_CODE = 1;				// From server
const uint8				SYSTEM_ACK_SYNC_CODE = 2;			// From client
const uint8				SYSTEM_PROBE_CODE = 3;				// From server
const uint8				SYSTEM_ACK_PROBE_CODE = 4;			// From client
const uint8				SYSTEM_DISCONNECTION_CODE = 5;		// From client
const uint8				SYSTEM_STALLED_CODE = 6;			// From server
const uint8				SYSTEM_SERVER_DOWN_CODE = 7;		// From server
const uint8				SYSTEM_QUIT_CODE = 8;				// From client
const uint8				SYSTEM_ACK_QUIT_CODE = 9;			// From server

const uint				NumBitsInLongAck = 512;

const static char*		SystemMessagesNames[32] = {
	"LOGIN",
	"SYNC",
	"ACK_SYNC",
	"PROBE",
	"ACK_PROBE",
	"DISCONNECTION",
	"STALLED",
	"SERVER_DOWN",

	"QUIT",
	"ACK_QUIT",
	"",
	"",
	"",
	"",
	"",
	"",

	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",

	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
};

}

#endif // NL_SYSTEM_MESSAGE_H

/* End of system_message.h */
