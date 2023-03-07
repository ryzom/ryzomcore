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

#include "group_html_mail.h"
#include "nel/misc/xml_auto_ptr.h"
#include "../client_cfg.h"
#include "../user_entity.h"
#include "interface_manager.h"

// used for login cookie to be sent to the web server
#include "../net_manager.h"

using namespace std;
using namespace NLMISC;


// ***************************************************************************
NLMISC_REGISTER_OBJECT(CViewBase, CGroupHTMLMail, std::string, "mail_html");

CGroupHTMLMail::CGroupHTMLMail(const TCtorParam &param)
: CGroupHTML(param)
{
}

// ***************************************************************************

CGroupHTMLMail::~CGroupHTMLMail()
{
}

// ***************************************************************************

void CGroupHTMLMail::addHTTPGetParams (string &url, bool /*trustedDomain*/)
{
	string user_name = UserEntity->getLoginName ();
	url += ((url.find('?') != string::npos) ? "&" : "?") +
		string("shard=") + toString(CharacterHomeSessionId) +
		string("&user_login=") + user_name + // FIXME: UrlEncode
		string("&session_cookie=") + NetMngr.getLoginCookie().toString() + 
		string("&lang=") + CI18N::getCurrentLanguageCode();
}

// ***************************************************************************

void CGroupHTMLMail::addHTTPPostParams (SFormFields &formfields, bool /*trustedDomain*/)
{
	string user_name = UserEntity->getLoginName ();
	formfields.add("shard", toString(CharacterHomeSessionId));
	formfields.add("user_login", user_name); // FIXME: UrlEncode
	formfields.add("session_cookie", NetMngr.getLoginCookie().toString());
	formfields.add("lang", CI18N::getCurrentLanguageCode());
}

// ***************************************************************************

string	CGroupHTMLMail::home () const
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:MAIL_WAITING")->setValue32(0); // FIXME: How is this const?!
	return Home;
}

// ***************************************************************************

void CGroupHTMLMail::handle ()
{
	// Do nothing if WebServer is not initialized
	if (!WebServer.empty())
	{
		Home = "/webig/mailbox.php";
		CGroupHTML::handle ();
	}
}

// ***************************************************************************
