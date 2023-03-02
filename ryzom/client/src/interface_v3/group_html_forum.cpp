// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010-2015  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2012  Matt RAYKOWSKI (sfb) <matt.raykowski@gmail.com>
// Copyright (C) 2013  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
// Copyright (C) 2019-2021  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include "group_html_forum.h"
#include "nel/misc/xml_auto_ptr.h"
#include "../client_cfg.h"
#include "../user_entity.h"
#include "guild_manager.h"
#include "interface_manager.h"

// used for login cookie to be sent to the web server
#include "../net_manager.h"

using namespace std;
using namespace NLMISC;


// ***************************************************************************
NLMISC_REGISTER_OBJECT(CViewBase, CGroupHTMLForum, std::string, "forum_html");

CGroupHTMLForum::CGroupHTMLForum(const TCtorParam &param)
: CGroupHTML(param)
{
}

// ***************************************************************************

CGroupHTMLForum::~CGroupHTMLForum()
{
}

// ***************************************************************************

void CGroupHTMLForum::addHTTPGetParams (string &url, bool /*trustedDomain*/)
{
 	string user_name = UserEntity->getLoginName ();
	const SGuild &guild = CGuildManager::getInstance()->getGuild();
	string	gname = guild.Name;

	if (!gname.empty())
	{
		string temp;
		for (uint i=0; i<gname.size(); ++i)
		{
			if (gname[i] != ' ')
				temp.push_back(gname[i]);
			else
				temp += "%20";
		}
		gname.swap(temp);

		url += ((url.find('?') != string::npos) ? "&" : "?") +
		string("shard=") + toString(CharacterHomeSessionId) +
		string("&user_login=") + user_name +
		string("&forum=") + gname +
		string("&session_cookie=") + NetMngr.getLoginCookie().toString();
	}
	else
	{
		nlwarning("WEB: guild name is empty, unable to GET guild forum pages.");
	}
}

// ***************************************************************************

void CGroupHTMLForum::addHTTPPostParams (SFormFields &formfields, bool /*trustedDomain*/)
{
	string user_name = UserEntity->getLoginName ();
	const SGuild &guild = CGuildManager::getInstance()->getGuild();
	string	gname = guild.Name;

	if (!gname.empty())
	{
		formfields.add("shard", toString(CharacterHomeSessionId));
		formfields.add("user_login", user_name);
		formfields.add("forum", gname);
		formfields.add("session_cookie", NetMngr.getLoginCookie().toString());
	}
	else
	{
		nlwarning("WEB: guild name is empty, unable to POST to guild forum pages.");
	}
}

// ***************************************************************************

string	CGroupHTMLForum::home () const
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:FORUM_UPDATED")->setValue32(0); // FIXME: How is this const?!
	return Home;
}

// ***************************************************************************

void CGroupHTMLForum::handle ()
{
	// Do nothing if WebServer is not initialized
	if (!WebServer.empty())
	{
		Home = "/webig/forum.php";
		CGroupHTML::handle ();
	}
}

// ***************************************************************************
