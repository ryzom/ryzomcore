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

#include "group_html_webig.h"
#include "nel/misc/xml_auto_ptr.h"
#include "../client_cfg.h"
#include "../user_entity.h"
#include "../entities.h"
#include "interface_manager.h"

// used for login cookie to be sent to the web server
#include "../net_manager.h"
#include "../connection.h"

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
	if(!UserEntity) return "";

	// authkey = <sharid><name><cid><cookie>
	uint32 cid = NetMngr.getLoginCookie().getUserId() * 16 + PlayerSelectedSlot;
	string rawKey = toString(CharacterHomeSessionId) +
		UserEntity->getLoginName().toString() +
		toString(cid) +
		NetMngr.getLoginCookie().toString();
	string key = getMD5((const uint8*)rawKey.c_str(), (uint32)rawKey.size()).toString();
	//nlinfo("rawkey = '%s'", rawKey.c_str());
	//nlinfo("authkey = %s", key.c_str());
	return key;
}

void addWebIGParams (string &url, bool trustedDomain)
{
	if(!UserEntity || !NetMngr.getLoginCookie().isValid()) return;

	uint32 cid = NetMngr.getLoginCookie().getUserId() * 16 + PlayerSelectedSlot;
	url += ((url.find('?') != string::npos) ? "&" : "?") +
		string("shardid=") + toString(CharacterHomeSessionId) +
		string("&name=") + UserEntity->getLoginName().toUtf8() +
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
			strFindReplace(url, "$displayName$", UserEntity->getDisplayName().toString());
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
					strFindReplace(url, "$tdisplayName$", target->getDisplayName().toString());
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


struct CWebigNotificationThread : public NLMISC::IRunnable
{
	CURL *Curl;

	CWebigNotificationThread()
	{
		Curl = curl_easy_init();
		if(!Curl) return;
		curl_easy_setopt(Curl, CURLOPT_COOKIEFILE, "");
		curl_easy_setopt(Curl, CURLOPT_NOPROGRESS, 1);
		curl_easy_setopt(Curl, CURLOPT_USERAGENT, "Ryzom");
		curl_easy_setopt(Curl, CURLOPT_FOLLOWLOCATION, 1);
		curl_easy_setopt(Curl, CURLOPT_WRITEFUNCTION, writeDataFromCurl);
		//nlinfo("ctor CWebigNotificationThread");
	}

	~CWebigNotificationThread()
	{
		if(Curl)
		{
			curl_easy_cleanup(Curl);
			Curl = 0;
		}
	}

	void get(const std::string &url)
	{
		if(!Curl) return;
		curlresult.clear();
		//nlinfo("get '%s'", url.c_str());
		curl_easy_setopt(Curl, CURLOPT_URL, url.c_str());
		CURLcode res = curl_easy_perform(Curl);
		long r;
		curl_easy_getinfo(Curl, CURLINFO_RESPONSE_CODE, &r);
		//nlwarning("result : '%s'", curlresult.c_str());

		vector<string> notifs;
		explode(curlresult, string("|"), notifs);

		// Update the mail notification icon

		uint32 nbmail = 0;
		if(notifs.size() > 0 && fromString(notifs[0], nbmail))
		{
			//nlinfo("nb mail is a number %d", nbmail);
			CInterfaceManager *pIM = CInterfaceManager::getInstance();
			if(pIM)
			{
				CCDBNodeLeaf *_CheckMailNode = NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:MAIL_WAITING");
				if(_CheckMailNode)
				{
					_CheckMailNode->setValue32(nbmail==0?0:1);
					CInterfaceElement *elm = CWidgetManager::getInstance()->getElementFromId("ui:interface:compass:mail:mail_nb");
					if (elm)
					{
						CViewText *vt = dynamic_cast<CViewText*>(elm);
						vt->setText(toString("%d", nbmail));
					}
				}
			}
		}
		else
		{
			nlwarning("this is not a number '%s'", curlresult.c_str());
		}

		// Update the forum notification icon

		uint32 nbforum = 0;
		if(notifs.size() > 1 && fromString(notifs[1], nbforum))
		{
			//nlinfo("nb forum this is a number %d", nbforum);
			CInterfaceManager *pIM = CInterfaceManager::getInstance();
			if(pIM)
			{
				CCDBNodeLeaf *_CheckForumNode = NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:FORUM_UPDATED");
				if(_CheckForumNode)
				{
					_CheckForumNode->setValue32(nbforum==0?0:1);
					CInterfaceElement *elm = CWidgetManager::getInstance()->getElementFromId("ui:interface:compass:forum:forum_nb");
					if (elm)
					{
						CViewText *vt = dynamic_cast<CViewText*>(elm);
						vt->setText(toString("%d", nbforum));
					}
				}
			}
		}
		else
		{
			nlwarning("this is not a number '%s'", curlresult.c_str());
		}
	}

	std::string randomString()
	{
		std::string chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
		std::string s;
		for (int i = 0; i < 32; i++)
		{
			s += chars[uint(frand(float(chars.size())))];
		}
		return s;
	}

	void run()
	{
		// first time, we wait a small amount of time to be sure everything is initialized
		nlSleep(1*60*1000);
		while (true)
		{
			string url = "http://"+ClientCfg.WebIgMainDomain+"/index.php?app=notif&rnd="+randomString();
			addWebIGParams(url, true);
			get(url);
			nlSleep(10*60*1000);
		}
	}
};

void startWebigNotificationThread()
{
	static bool startedWebigNotificationThread = false;
	if(!startedWebigNotificationThread)
	{
		curl_global_init(CURL_GLOBAL_ALL);
		//nlinfo("startStatThread");
		CWebigNotificationThread *webigThread = new CWebigNotificationThread();
		IThread	*thread = IThread::create (webigThread);
		nlassert (thread != NULL);
		thread->start ();
		startedWebigNotificationThread = true;
	}
}

// ***************************************************************************
// ***************************************************************************


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
	addWebIGParams(url, trustedDomain);
}

// ***************************************************************************

void CGroupHTMLAuth::addHTTPPostParams (HTAssocList *formfields, bool trustedDomain)
{
	if(!UserEntity) return;

	uint32 cid = NetMngr.getLoginCookie().getUserId() * 16 + PlayerSelectedSlot;
	HTParseFormInput(formfields, ("shardid="+toString(CharacterHomeSessionId)).c_str());
	HTParseFormInput(formfields, ("name="+UserEntity->getLoginName().toUtf8()).c_str());
	HTParseFormInput(formfields, ("lang="+CI18N::getCurrentLanguageCode()).c_str());
	HTParseFormInput(formfields, "ig=1");
	if (trustedDomain)
	{
		HTParseFormInput(formfields, ("cid="+toString(cid)).c_str());
		HTParseFormInput(formfields, ("authkey="+getWebAuthKey()).c_str());
	}
}

// ***************************************************************************

string CGroupHTMLAuth::home ()
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
	startWebigNotificationThread();
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

void CGroupHTMLWebIG::addHTTPPostParams (HTAssocList *formfields, bool trustedDomain)
{
	CGroupHTMLAuth::addHTTPPostParams(formfields, trustedDomain);
}

// ***************************************************************************

string CGroupHTMLWebIG::home ()
{
	return Home;
}

// ***************************************************************************

void CGroupHTMLWebIG::handle ()
{
	CGroupHTMLAuth::handle ();
}

