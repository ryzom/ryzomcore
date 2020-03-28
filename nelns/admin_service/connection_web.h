// NeLNS - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#ifndef NL_CONNECTION_WEB_H
#define NL_CONNECTION_WEB_H

#include "nel/misc/types_nl.h"

#include "nel/net/buf_server.h"

void sendString (NLNET::TSockId from, const std::string &str);

void connectionWebInit ();
void connectionWebUpdate ();
void connectionWebRelease ();

#endif // NL_CONNECTION_WEB_H

/* End of connection_web.h */
