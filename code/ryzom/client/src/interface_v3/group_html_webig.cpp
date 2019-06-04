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
#include "nel/gui/curl_certificates.h"

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
	// no extras parameters added to url if not in trusted domains list
	if (!trustedDomain) return;

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

class CWebigNotificationThread : public NLMISC::IRunnable
{
private:
	CURL *Curl;
	bool _Running;
	IThread *_Thread;

public:

	CWebigNotificationThread()
	{
		_Running = false;
		_Thread = NULL;
		curl_global_init(CURL_GLOBAL_ALL);

		Curl = NULL;
		//nlinfo("ctor CWebigNotificationThread");
	}

	void init()
	{
		if (Curl)
		{
			return;
		}

		Curl = curl_easy_init();
		if(!Curl) return;
		curl_easy_setopt(Curl, CURLOPT_COOKIEFILE, "");
		curl_easy_setopt(Curl, CURLOPT_NOPROGRESS, 1);
		curl_easy_setopt(Curl, CURLOPT_USERAGENT, getUserAgent().c_str());
		curl_easy_setopt(Curl, CURLOPT_FOLLOWLOCATION, 1);
		curl_easy_setopt(Curl, CURLOPT_WRITEFUNCTION, writeDataFromCurl);

		NLGUI::CCurlCertificates::useCertificates(Curl);
	}

	~CWebigNotificationThread()
	{
		if(Curl)
		{
			curl_easy_cleanup(Curl);
			Curl = NULL;
		}
		if (_Thread)
		{
			_Thread->terminate();
			delete _Thread;
			_Thread = NULL;
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

		char *ch;
		std::string contentType;
		res = curl_easy_getinfo(Curl, CURLINFO_CONTENT_TYPE, &ch);
		if (res == CURLE_OK && ch != NULL)
		{
			contentType = ch;
		}

		// "text/lua; charset=utf8"
		if (contentType.find("text/lua") == 0)
		{
			std::string script;
			script = "\nlocal __WEBIG_NOTIF__= true\n" + curlresult;
			CInterfaceManager::getInstance()->queueLuaScript(script);
		}
		else
		{
			nlwarning("Invalid content-type '%s', expected 'text/lua'", contentType.c_str());
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
		if (ClientCfg.WebIgNotifInterval == 0)
		{
			_Running = false;
			nlwarning("ClientCfg.WebIgNotifInterval == 0, notification thread not running");
			return;
		}

		std::string domain = ClientCfg.WebIgMainDomain;
		uint32 ms = ClientCfg.WebIgNotifInterval*60*1000;

		_Running = true;
		// first time, we wait a small amount of time to be sure everything is initialized
		nlSleep(30*1000);
		uint c = 0;
		while (_Running)
		{
			string url = "https://"+domain+"/index.php?app=notif&format=lua&rnd="+randomString();
			addWebIGParams(url, true);
			get(url);

			sleepLoop(ms);
		}
	}

	void sleepLoop(uint ms)
	{
		// use smaller sleep time so stopThread() will not block too long
		// tick == 100ms
		uint32 ticks = ms / 100;
		while (_Running && ticks > 0) {
			nlSleep(100);
			ticks--;
		}
	}

	void startThread()
	{
		// initialize curl outside thread
		init();

		if (!_Thread)
		{
			_Thread = IThread::create(this);
			nlassert(_Thread != NULL);
			_Thread->start();
			nlwarning("WebIgNotification thread started");
		}
		else
		{
			nlwarning("WebIgNotification thread already started");
		}
	}

	void stopThread()
	{
		_Running = false;
		if (_Thread)
		{
			_Thread->wait();
			delete _Thread;
			_Thread = NULL;
			nlwarning("WebIgNotification thread stopped");
		}
		else
		{
			nlwarning("WebIgNotification thread already stopped");
		}
	}

	bool isRunning() const
	{
		return _Running;
	}
};

static CWebigNotificationThread webigThread;
void startWebIgNotificationThread()
{
	if (!webigThread.isRunning())
	{
		webigThread.startThread();
	}
}

void stopWebIgNotificationThread()
{
	if (webigThread.isRunning())
	{
		webigThread.stopThread();
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
	formfields.add("name", UserEntity->getLoginName().toUtf8());
	formfields.add("lang", CI18N::getCurrentLanguageCode());
	formfields.add("ig", "1");
	if (trustedDomain)
	{
		formfields.add("cid", toString(cid));
		formfields.add("authkey", getWebAuthKey());
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

string CGroupHTMLWebIG::home ()
{
	return Home;
}

// ***************************************************************************

void CGroupHTMLWebIG::handle ()
{
	CGroupHTMLAuth::handle ();
}

