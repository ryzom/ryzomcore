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


//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "nel/net/service.h"

#ifdef NL_OS_WINDOWS
#	define NOMINMAX
#	include <windows.h>
#endif // NL_OS_WINDOWS

using namespace std;
using namespace NLMISC;
using namespace NLNET;

// force admin module to link in
extern void admin_modules_forceLink();
void foo()
{
	admin_modules_forceLink();
}

//-----------------------------------------------------------------------------
// class CServiceClass 
//-----------------------------------------------------------------------------

class CServiceClass : public NLNET::IService
{
public :
	void init()
	{
	}

	bool update()
	{
		return true;
	}

	void release()
	{
	}
};




//-----------------------------------------------
//	NLNET_SERVICE_MAIN
//-----------------------------------------------

static const char* getCompleteServiceName(const IService* theService)
{
	static std::string s;
	s= "ryzom_admin_service";

	if (theService->haveLongArg("adminname"))
	{
		s+= "_"+theService->getLongArg("adminname");
	}

	if (theService->haveLongArg("fulladminname"))
	{
		s= theService->getLongArg("fulladminname");
	}

	return s.c_str();
}

static const char* getShortServiceName(const IService* theService)
{
	static std::string s;
	s= "RAS";

	if (theService->haveLongArg("shortadminname"))
	{
		s= theService->getLongArg("shortadminname");
	}
	
	return s.c_str();
}

NLNET_SERVICE_MAIN( CServiceClass, getShortServiceName(scn), getCompleteServiceName(scn), 0, EmptyCallbackArray, "", "" );

