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

#include "group_html_qcm.h"
#include "nel/misc/xml_auto_ptr.h"
#include "../client_cfg.h"
#include "interface_manager.h"
#include "../user_entity.h"

// used for login cookie to be sent to the web server
#include "../net_manager.h"

using namespace std;
using namespace NLMISC;


// ***************************************************************************
NLMISC_REGISTER_OBJECT(CViewBase, CGroupHTMLQCM, std::string, "qcm_html");

CGroupHTMLQCM::CGroupHTMLQCM(const TCtorParam &param)
:	CGroupHTML(param)
{
}

// ***************************************************************************

CGroupHTMLQCM::~CGroupHTMLQCM()
{
}

// ***************************************************************************
void CGroupHTMLQCM::addText (const char * buf, int len)
{
	string sTmp = buf;
	if (sTmp.find("zzz_QUIT_RYZOM_zzz") != string::npos)
	{
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CAHManager::getInstance()->runActionHandler("quit_ryzom", NULL);
		CInterfaceElement *pIE = CWidgetManager::getInstance()->getElementFromId("ui:interface:web_on_quit");
		pIE->setActive(false);
	}

	CGroupHTML::addText(buf, len);
}

