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
#include "game_share/xml_auto_ptr.h"
#include "../client_cfg.h"
#include "../user_entity.h"
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
		CGroupHTML *groupHtml = dynamic_cast<CGroupHTML*>(pIM->getElementFromId(container));
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
		UserEntity->getDisplayName().toString() +
		toString(cid) +
		NetMngr.getLoginCookie().toString();
	string key = getMD5((const uint8*)rawKey.c_str(), rawKey.size()).toString();
	//nlinfo("rawkey = '%s'", rawKey.c_str());
	//nlinfo("authkey = %s", key.c_str());
	return key;
}

void addWebIGParams (string &url)
{
	if(!UserEntity) return;

	uint32 cid = NetMngr.getLoginCookie().getUserId() * 16 + PlayerSelectedSlot;
	url += ((url.find('?') != string::npos) ? "&" : "?") +
		string("shardid=") + toString(CharacterHomeSessionId) +
		string("&name=") + UserEntity->getDisplayName().toUtf8() +
		string("&cid=") + toString(cid) + 
		string("&authkey=") + getWebAuthKey() +
		string("&ig=1") +
		string("&lang=") + CI18N::getCurrentLanguageCode();
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
		if(Curl) {
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
				CCDBNodeLeaf *_CheckMailNode = pIM->getDbProp("UI:VARIABLES:MAIL_WAITING");
				if(_CheckMailNode)
				{
					_CheckMailNode->setValue32(nbmail==0?0:1);
					CInterfaceElement *elm = pIM->getElementFromId("ui:interface:compass:mail:mail_nb");
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
				CCDBNodeLeaf *_CheckForumNode = pIM->getDbProp("UI:VARIABLES:FORUM_UPDATED");
				if(_CheckForumNode)
				{
					_CheckForumNode->setValue32(nbforum==0?0:1);
					CInterfaceElement *elm = pIM->getElementFromId("ui:interface:compass:forum:forum_nb");
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
		for (int i = 0; i < 32; i++) {
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
			string url = "http://atys.ryzom.com/start/index.php?app=notif&rnd="+randomString();
			//string url = "http://ryapp.bmsite.net/app_mail.php?page=ajax/inbox/unread&rnd="+randomString();
			addWebIGParams(url);
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
NLMISC_REGISTER_OBJECT(CViewBase, CGroupHTMLWebIG, std::string, "webig_html");

CGroupHTMLWebIG::CGroupHTMLWebIG(const TCtorParam &param)
: CGroupHTML(param)
{
	startWebigNotificationThread();
}

// ***************************************************************************

CGroupHTMLWebIG::~CGroupHTMLWebIG()
{
}

void CGroupHTMLWebIG::addHTTPGetParams (string &url)
{
	addWebIGParams(url);
}

// ***************************************************************************

void CGroupHTMLWebIG::addHTTPPostParams (HTAssocList *formfields)
{
	if(!UserEntity) return;

	uint32 cid = NetMngr.getLoginCookie().getUserId() * 16 + PlayerSelectedSlot;
	HTParseFormInput(formfields, ("shardid="+toString(CharacterHomeSessionId)).c_str());
	HTParseFormInput(formfields, ("name="+UserEntity->getDisplayName().toUtf8()).c_str());
	HTParseFormInput(formfields, ("cid="+toString(cid)).c_str());
	HTParseFormInput(formfields, ("authkey="+getWebAuthKey()).c_str());
	HTParseFormInput(formfields, "ig=1");
	HTParseFormInput(formfields, ("lang="+CI18N::getCurrentLanguageCode()).c_str());
}

// ***************************************************************************

string CGroupHTMLWebIG::home ()
{
	return Home;
}

// ***************************************************************************

void CGroupHTMLWebIG::handle ()
{
	Home = "http://atys.ryzom.com/start/index.php";
	CGroupHTML::handle ();
}

// ***************************************************************************
