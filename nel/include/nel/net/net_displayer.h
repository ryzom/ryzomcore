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

#ifndef NL_NET_DISPLAYER_H
#define NL_NET_DISPLAYER_H

#include "nel/misc/log.h"
#include "nel/misc/displayer.h"

#include "callback_client.h"

namespace NLNET {


/**
 * Net Displayer. Sends the strings to a logger server (LOGS).
 * \ref log_howto
 * \bug When nlerror is called in a catch block, a connected NetDisplayer becomes an IDisplayer => pure virtual call
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2000
 */
class CNetDisplayer : public NLMISC::IDisplayer
{
public:

	/// Constructor
	CNetDisplayer(bool autoConnect = true);

	/** Sets logging server address. Call this method from outside only if you want to use a LOGS not registered within the NS.
	 * It does nothing if the displayer is already connected to a server.
	 */
	void setLogServer( const CInetAddress& logServerAddr );

	/** Sets logging server with an already connected server.
	 */
	void setLogServer( CCallbackClient *server );

	/// Returns true if the displayer is connected to a Logging Service.
	bool connected () { return _Server->connected(); }

	/// Destructor
	virtual ~CNetDisplayer();

protected:

	/** Sends the string to the logging server
	 * \warning If not connected, tries to connect to the logging server each call. It can slow down your program a lot.
	 */
	virtual void doDisplay ( const NLMISC::CLog::TDisplayInfo& args, const char *message);

	 /// Find the server (using the NS) and connect
	void findAndConnect();

private:

	CInetAddress	_ServerAddr;
//	CCallbackClient	_Server;
	CCallbackClient	*_Server;
	bool			_ServerAllocated;
//	uint32			_ServerNumber;
};


} // NLNET


#endif // NL_NET_DISPLAYER_H

/* End of net_displayer.h */
