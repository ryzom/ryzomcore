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

#include <nelns/login_service/connection_web.h>
#include <nelns/login_service/functions.h>
#include <nelns/login_service/variables.h>
#include <nelns/login_service/mysql_persistence.h>

#include <utility>

//
// Namespaces
//

using std::string;
using NLMISC::CLog;
using NLNET::IService;


CLoginService::CLoginService () :
	CLoginService(std::make_shared<CMysqlPersistence>())
{ }

CLoginService::CLoginService (std::shared_ptr<IPersistence>&& persistence) :
	UseDirectClient(false),
	m_persistence(std::move(persistence))
{ }

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
	m_persistence->init();

	m_connection_ws = std::make_unique<ConnectionWS>();

	if(UseDirectClient)
	{
		m_connection_client = std::make_unique<ConnectionClient>(*m_persistence);
	}
	else
	{
		connectionWebInit ();
	}

	Output->displayNL ("Login Service initialized");
}

bool CLoginService::update ()
{
	m_connection_ws->update ();
	if(UseDirectClient)
		m_connection_client->update ();
	else
		connectionWebUpdate ();
	return true;
}

/// release the service, save the universal time
void CLoginService::release ()
{
	m_connection_ws = nullptr;
	if(UseDirectClient)
		m_connection_client = nullptr;
	else
		connectionWebRelease ();

	Output->displayNL ("Login Service released");
}
