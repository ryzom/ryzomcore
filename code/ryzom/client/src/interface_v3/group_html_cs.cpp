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

#include "group_html_cs.h"
#include "nel/misc/xml_auto_ptr.h"
#include "../client_cfg.h"
#include "interface_manager.h"

// used for login cookie to be sent to the web server
#include "../net_manager.h"

using namespace std;
using namespace NLMISC;


// ***************************************************************************
NLMISC_REGISTER_OBJECT(CViewBase, CGroupHTMLCS, std::string, "cs_html");

CGroupHTMLCS::CGroupHTMLCS(const TCtorParam &param)
: CGroupHTML(param)
{
	Home = "";
}

// ***************************************************************************

CGroupHTMLCS::~CGroupHTMLCS()
{
}

// ***************************************************************************

void CGroupHTMLCS::addHTTPGetParams (string &url, bool /*trustedDomain*/)
{
	url += ((url.find('?') != string::npos) ? "&" : "?");

	std::vector<CParameter> parameters;
	getParameters (parameters, true);

	uint i;
	for (i=0; i<parameters.size(); i++)
	{
		if (i!=0)
			url += "&";
		url += parameters[i].Name;
		url += "=";
		url += parameters[i].Value;
	}
}

// ***************************************************************************

void CGroupHTMLCS::addHTTPPostParams (HTAssocList *formfields, bool /*trustedDomain*/)
{
	std::vector<CParameter> parameters;
	getParameters (parameters, false);

	uint i;
	for (i=0; i<parameters.size(); i++)
	{
		HTParseFormInput(formfields, (parameters[i].Name+"="+parameters[i].Value).c_str());
	}
}

// ***************************************************************************

string	CGroupHTMLCS::home ()
{
	return Home;
}

// ***************************************************************************

extern string getDebugInformation();
extern string getSystemInformation();

string convertToHTML (const string &s)
{
	string dest;
	uint i;
	for (i=0; i<s.size(); i++)
	{
		if  ( ((s[i] >= 'a') && (s[i] <= 'z')) ||
			((s[i] >= 'A') && (s[i] <= 'Z')) ||
			((s[i] >= '0') && (s[i] <= '9')))
		{
			dest.push_back (s[i]);
		}
		else
		{
			dest += toString ("%%%02x", (int) (unsigned char) s[i]);
		}
	}
	return dest;
}

// ***************************************************************************

void CGroupHTMLCS::getParameters (std::vector<CParameter> &parameters, bool encodeForUrl)
{
	string s = getDebugInformation();
	s += getSystemInformation();

	static bool webIgReady = false;

	if (!webIgReady) // Webig is ready when getParameters of CGroupHTMLCS is called
	{
		webIgReady = true;
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CLuaManager::getInstance().executeLuaScript("game:onWebIgReady()");
	}

	// For each line
	string::size_type startOfLine = 0;
	string::size_type endOfLine;
	endOfLine = s.find_first_of("\n");
	while (endOfLine != string::npos)
	{
		string line = s.substr (startOfLine, endOfLine-startOfLine);
		string dest;
		string::size_type endOfName = line.find_first_of(":");
		if (endOfName != string::npos)
		{
			CParameter param;
			param.Name = line.substr (0, endOfName);
			if (encodeForUrl)
				param.Name = convertToHTML (param.Name);
			param.Value = line.substr (endOfName+1);
			if (encodeForUrl)
				param.Value = convertToHTML (param.Value);
			parameters.push_back (param);
		}
		startOfLine = endOfLine+1;
		endOfLine = s.find_first_of("\n", startOfLine);
	}
}

// ***************************************************************************
