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
	ucstring user_name = UserEntity->getLoginName ();
	url += ((url.find('?') != string::npos) ? "&" : "?") +
		string("shard=") + toString(CharacterHomeSessionId) +
		string("&user_login=") + user_name.toString() +
		string("&session_cookie=") + NetMngr.getLoginCookie().toString() + 
		string("&lang=") + CI18N::getCurrentLanguageCode();
}

// ***************************************************************************

void CGroupHTMLMail::addHTTPPostParams (HTAssocList *formfields, bool /*trustedDomain*/)
{
	ucstring user_name = UserEntity->getLoginName ();
	HTParseFormInput(formfields, ("shard="+toString(CharacterHomeSessionId)).c_str());
	HTParseFormInput(formfields, ("user_login="+user_name.toString()).c_str());
	HTParseFormInput(formfields, ("session_cookie="+NetMngr.getLoginCookie().toString()).c_str());
	HTParseFormInput(formfields, ("lang="+CI18N::getCurrentLanguageCode()).c_str());
}

// ***************************************************************************

string	CGroupHTMLMail::home ()
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:MAIL_WAITING")->setValue32(0);
	return Home;
}

// ***************************************************************************

void CGroupHTMLMail::handle ()
{
	// Do nothing if WebServer is not initialized
/*	if (!WebServer.empty())
	{
		Home = WebServer+"mailbox.php";
		CGroupHTML::handle ();
	}
*/
}

// ***************************************************************************
