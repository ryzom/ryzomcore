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

#include <nelns/login_service/login_service.h>


#include <nel/misc/types_nl.h>
#include <nel/misc/window_displayer.h>
#include <nel/net/service.h>
#include <nel/net/login_cookie.h>

#include <nelns/login_service/connection_client.h>
#include <nelns/login_service/connection_ws.h>
#include <nelns/login_service/connection_web.h>
#include <nelns/login_service/functions.h>
#include <nelns/login_service/variables.h>
#include <nelns/login_service/mysql_helper.h>

//
// Namespaces
//

using std::string;
using NLMISC::CLog;
using NLNET::IService;


CLoginService::CLoginService () : UseDirectClient(false) { }

/// Init the service, load the universal time.
void CLoginService::init ()
{
	beep ();

	Output = new CLog;

	if(ConfigFile.exists("UseDirectClient"))
		UseDirectClient = ConfigFile.getVar("UseDirectClient").asBool();

	string fn = IService::getInstance()->SaveFilesDirectory;
	fn += "login_service.stat";
	nlinfo("Login stat in directory '%s'", fn.c_str());
	Fd = new NLMISC::CFileDisplayer(fn);
	Output->addDisplayer (Fd);
	if (WindowDisplayer) Output->addDisplayer (WindowDisplayer);

	// Initialize the database access
	sqlInit();

	connectionWSInit ();

	if(UseDirectClient)
		connectionClientInit ();
	else
		connectionWebInit ();

	Output->displayNL ("Login Service initialized");
}

bool CLoginService::update ()
{
	connectionWSUpdate ();
	if(UseDirectClient)
		connectionClientUpdate ();
	else
		connectionWebUpdate ();
	return true;
}

/// release the service, save the universal time
void CLoginService::release ()
{
	connectionWSRelease ();
	if(UseDirectClient)
		connectionClientRelease ();
	else
		connectionWebRelease ();

	Output->displayNL ("Login Service released");
}
