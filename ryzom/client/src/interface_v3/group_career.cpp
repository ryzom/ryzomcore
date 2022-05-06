// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
// Copyright (C) 2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include "group_career.h"
#include "interface_manager.h"
#include "nel/gui/interface_element.h"

//#include "game_share/jobs.h"
#include "nel/misc/xml_auto_ptr.h"

using namespace std;

NLMISC_REGISTER_OBJECT(CViewBase, CGroupCareer, std::string, "career");
NLMISC_REGISTER_OBJECT(CViewBase, CGroupJob, std::string, "job");

// ***************************************************************************
bool CGroupCareer::parse (xmlNodePtr cur, CInterfaceGroup *parentGroup)
{
	if(!CGroupContainer::parse(cur, parentGroup))
		return false;

	CXMLAutoPtr ptr((const char*) xmlGetProp (cur, (xmlChar*)"career"));

	uint careerIndex;
	ROLES::ERole Career;

	if (ptr)
	{
		NLMISC::fromString((const char*)ptr, careerIndex);
		Career= (ROLES::ERole) careerIndex;
		if(Career==ROLES::role_unknown)
		{
			nlinfo ("unknown career %s in %s", (const char*)ptr, _Id.c_str());
			return false;
		}
	}
	else
	{
		nlinfo ("no career in %s", _Id.c_str());
		return false;
	}

	if (Career >= ROLES::NB_ROLES)
		Career = ROLES::fighter;

	string sTmp = ROLES::roleToUCString(Career);
	for (uint32 i= 0; i < sTmp.size(); ++i)
		if (sTmp[i] < 128)
			if ( (sTmp[i] >= 'a') && (sTmp[i] <= 'z') )
				sTmp[i] = sTmp[i] - 'a' + 'A';

	setTitle (sTmp);

	return true;
}

// ***************************************************************************
bool CGroupJob::parse (xmlNodePtr /* cur */, CInterfaceGroup * /* parentGroup */)
{
	return false;

//	if(!CGroupContainer::parse(cur, parentGroup))
//		return false;
//
//	CXMLAutoPtr ptr = (char*) xmlGetProp (cur, (xmlChar*)"career");
//
//	if (ptr == NULL)
//		return false;
//
//	uint careerIndex, jobIndex;
//	ROLES::ERole Career;
//	JOBS::TJob Job;
//
//	if (ptr)
//	{
//		fromString((const char*)ptr, careerIndex);
//		Career= (ROLES::ERole) careerIndex;
//		if(Career==ROLES::role_unknown)
//		{
//			nlinfo ("unknown career %s in %s", (const char*)ptr, _Id.c_str());
//			return false;
//		}
//	}
//	else
//	{
//		nlinfo ("no career in %s", _Id.c_str());
//		return false;
//	}
//
//	// get job
//	ptr = xmlGetProp (cur, (xmlChar*)"job");
//	if (ptr)
//	{
//		fromString((const char*)ptr, jobIndex);
//		if(jobIndex==-1)
//			Job= JOBS::All;
//		else
//			Job= JOBS::getJobForRace((ROLES::ERole)careerIndex, (EGSPD::CPeople::TPeople)jobIndex);
//		if(Job==JOBS::Unknown)
//		{
//			nlinfo ("unknown job %s in %s", (const char*)ptr, _Id.c_str());
//			return false;
//		}
//	}
//	else
//	{
//		nlinfo ("no job in %s", _Id.c_str());
//		return false;
//	}
//
//	string sTmp = JOBS::jobToUCString(Job);
//	for (uint32 i= 0; i < sTmp.size(); ++i)
//		if (sTmp[i] < 128)
//			if ( (sTmp[i] >= 'a') && (sTmp[i] <= 'z') )
//				sTmp[i] = sTmp[i] - 'a' + 'A';
//	seUCTitle (sTmp);
//
//	return true;
}
