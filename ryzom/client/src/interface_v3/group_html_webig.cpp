// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010-2019  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2012  Matt RAYKOWSKI (sfb) <matt.raykowski@gmail.com>
// Copyright (C) 2013  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
// Copyright (C) 2019-2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include "group_html_webig.h"
#include "nel/misc/xml_auto_ptr.h"
#include "nel/gui/lua_manager.h"
#include "../client_cfg.h"
#include "../user_entity.h"
#include "../entities.h"
#include "interface_manager.h"
#include "user_agent.h"

// used for login cookie to be sent to the web server
#include "../net_manager.h"
#include "../connection.h"

#include <curl/curl.h>
#include "nel/web/curl_certificates.h"

using namespace std;
using namespace NLMISC;

// ***************************************************************************
class CHandlerBrowseHome : public IActionHandler
{
public:
	void execute (CCtrlBase *pCaller, const std::string &sParams)
	{
		CInterfaceManager	*pIM= CInterfaceManager::getInstance();
		string container = getParam (sParams, "name");
		CGroupHTML *groupHtml = dynamic_cast<CGroupHTML*>(CWidgetManager::getInstance()->getElementFromId(container));
		if (groupHtml)
		{
			groupHtml->browse(groupHtml->Home.c_str());
		}
	};
};
REGISTER_ACTION_HANDLER( CHandlerBrowseHome, "browse_home");

// ***************************************************************************

static string getWebAuthKey()
{
	if(!UserEntity || !NetMngr.getLoginCookie().isValid()) return "";

	// authkey = <sharid><name><cid><cookie>
	uint32 cid = NetMngr.getLoginCookie().getUserId() * 16 + PlayerSelectedSlot;
	string rawKey = toString(CharacterHomeSessionId) +
		UserEntity->getLoginName() +
		toString(cid) +
		NetMngr.getLoginCookie().toString();
	string key = getMD5((const uint8*)rawKey.c_str(), (uint32)rawKey.size()).toString();
	//nlinfo("rawkey = '%s'", rawKey.c_str());
	//nlinfo("authkey = %s", key.c_str());
	return key;
}

void addWebIGParams (string &url, bool trustedDomain)
{
	// no extras parameters added to url if not in trusted domains list
	if (!trustedDomain) return;

	if(!UserEntity || !NetMngr.getLoginCookie().isValid()) return;

	uint32 cid = NetMngr.getLoginCookie().getUserId() * 16 + PlayerSelectedSlot;
	url += ((url.find('?') != string::npos) ? "&" : "?") +
		string("shardid=") + toString(CharacterHomeSessionId) +
		string("&name=") + UserEntity->getLoginName() + // FIXME: UrlEncode
		string("&lang=") + CI18N::getCurrentLanguageCode() +
		string("&datasetid=") + toString(UserEntity->dataSetId()) +
		string("&ig=1");
	if (trustedDomain)
	{
		url += string("&cid=") + toString(cid) +
		string("&authkey=") + getWebAuthKey();

		if (url.find('$') != string::npos)
		{
			strFindReplace(url, "$gender$", GSGENDER::toString(UserEntity->getGender()));
			strFindReplace(url, "$displayName$", UserEntity->getDisplayName()); // FIXME: UrlEncode...
			strFindReplace(url, "$posx$", toString(UserEntity->pos().x));
			strFindReplace(url, "$posy$", toString(UserEntity->pos().y));
			strFindReplace(url, "$posz$", toString(UserEntity->pos().z));
			strFindReplace(url, "$post$", toString(atan2(UserEntity->front().y, UserEntity->front().x)));
			
			// Target fields
			const char *dbPath = "UI:VARIABLES:TARGET:SLOT";
			CInterfaceManager *im = CInterfaceManager::getInstance();
			CCDBNodeLeaf *node = NLGUI::CDBManager::getInstance()->getDbProp(dbPath, false);
			if (node && (uint8)node->getValue32() != (uint8) CLFECOMMON::INVALID_SLOT)
			{
				CEntityCL *target = EntitiesMngr.entity((uint) node->getValue32());
				if (target)
				{
					strFindReplace(url, "$tdatasetid$", toString(target->dataSetId()));
					strFindReplace(url, "$tdisplayName$", target->getDisplayName()); // FIXME: UrlEncode...
					strFindReplace(url, "$tposx$", toString(target->pos().x));
					strFindReplace(url, "$tposy$", toString(target->pos().y));
					strFindReplace(url, "$tposz$", toString(target->pos().z));
					strFindReplace(url, "$tpost$", toString(atan2(target->front().y, target->front().x)));
					strFindReplace(url, "$tsheet$", target->sheetId().toString());
					string type;
					if (target->isFauna())
						type = "fauna";
					else if (target->isNPC())
						type = "npc";
					else if (target->isPlayer())
						type = "player";
					else if (target->isUser())
						type = "user";
					strFindReplace(url, "$ttype$", target->sheetId().toString());
				}
				else
				{
					strFindReplace(url, "$tdatasetid$", "");
					strFindReplace(url, "$tdisplayName$", "");
					strFindReplace(url, "$tposx$", "");
					strFindReplace(url, "$tposy$", "");
					strFindReplace(url, "$tposz$", "");
					strFindReplace(url, "$tpost$", "");
					strFindReplace(url, "$tsheet$", "");
					strFindReplace(url, "$ttype$", "");
				}
			}
		}
	}
}

// ***************************************************************************
// ***************************************************************************
static string curlresult;

size_t writeDataFromCurl(void *buffer, size_t size, size_t nmemb, void *pcl)
{
	//CWebigNotificationThread *cl = static_cast<CWebigNotificationThread*>(pcl);
	string str;
	str.assign((char*)buffer, size*nmemb);
	curlresult += str;
	return size*nmemb;
}

// ***************************************************************************
NLMISC_REGISTER_OBJECT(CViewBase, CGroupHTMLAuth, std::string, "auth_html");

CGroupHTMLAuth::CGroupHTMLAuth(const TCtorParam &param)
: CGroupHTML(param)
{
}

// ***************************************************************************

CGroupHTMLAuth::~CGroupHTMLAuth()
{
}

void CGroupHTMLAuth::addHTTPGetParams (string &url, bool trustedDomain)
{
	if(!UserEntity || !NetMngr.getLoginCookie().isValid()) return;

	addWebIGParams(url, trustedDomain);
}

// ***************************************************************************

void CGroupHTMLAuth::addHTTPPostParams (SFormFields &formfields, bool trustedDomain)
{
	// no extras parameters added to url if not in trusted domains list
	if (!trustedDomain) return;

	if(!UserEntity || !NetMngr.getLoginCookie().isValid()) return;

	uint32 cid = NetMngr.getLoginCookie().getUserId() * 16 + PlayerSelectedSlot;
	formfields.add("shardid", toString(CharacterHomeSessionId));
	formfields.add("name", UserEntity->getLoginName());
	formfields.add("lang", CI18N::getCurrentLanguageCode());
	formfields.add("ig", "1");
	if (trustedDomain)
	{
		formfields.add("cid", toString(cid));
		formfields.add("authkey", getWebAuthKey());
	}
}

// ***************************************************************************

string CGroupHTMLAuth::home () const
{
	return Home;
}

// ***************************************************************************

void CGroupHTMLAuth::handle ()
{
	CGroupHTML::handle ();
}

// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
NLMISC_REGISTER_OBJECT(CViewBase, CGroupHTMLWebIG, std::string, "webig_html");

CGroupHTMLWebIG::CGroupHTMLWebIG(const TCtorParam &param)
: CGroupHTMLAuth(param)
{
}

// ***************************************************************************

CGroupHTMLWebIG::~CGroupHTMLWebIG()
{
}

// ***************************************************************************

void CGroupHTMLWebIG::addHTTPGetParams (string &url, bool trustedDomain)
{
	CGroupHTMLAuth::addHTTPGetParams(url, trustedDomain);
}

// ***************************************************************************

void CGroupHTMLWebIG::addHTTPPostParams (SFormFields &formfields, bool trustedDomain)
{
	CGroupHTMLAuth::addHTTPPostParams(formfields, trustedDomain);
}

// ***************************************************************************

string CGroupHTMLWebIG::home () const
{
	return Home;
}

// ***************************************************************************

void CGroupHTMLWebIG::handle ()
{
	CGroupHTMLAuth::handle ();
}

