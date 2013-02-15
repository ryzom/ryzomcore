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
#include "login_registry.h"

#ifdef NL_OS_WINDOWS

#include <windows.h>


const char *CLoginRegistry::AppRegEntry = "Software\\Nevrax\\RyzomInstall";
static const char *LoginStepKeyHandle = "LoginStep";
static const char *InstallIdKeyHandle = "InstallId";

//===========================================================================================
std::string CLoginRegistry::getProductInstallId()
{


	// read value
	HKEY hKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, AppRegEntry, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
	{
		const uint keyMaxLength = 1024;
		DWORD	dwType  = 0L;
		DWORD	dwSize	= keyMaxLength;
		char	buffer[keyMaxLength];
		if(RegQueryValueEx(hKey, InstallIdKeyHandle, NULL, &dwType, (unsigned char *) buffer, &dwSize) == ERROR_SUCCESS && dwType == REG_SZ)
		{
			RegCloseKey(hKey);
			return buffer;
		}
		RegCloseKey(hKey);
	}
	DWORD dwDisp;
	// do not exist, create a new key
	if(RegCreateKeyEx(HKEY_CURRENT_USER, AppRegEntry, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDisp) == ERROR_SUCCESS)
    {
		srand((uint32)nl_time(0));
		uint32 r = rand();
		r <<= 16;
		r |= rand();
		std::string id = NLMISC::toString(r);
		if (RegSetValueEx(hKey, InstallIdKeyHandle, 0L, REG_SZ, (const BYTE *) id.c_str(), (DWORD)(id.size())+1) == ERROR_SUCCESS)
		{
			return id;
		}
	}
	return "";
}

//===========================================================================================
uint CLoginRegistry::getLoginStep()
{
	HKEY hKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, AppRegEntry, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
	{
		DWORD loginStep = 0;
		DWORD type;
		DWORD dataSize = sizeof(DWORD);
		RegQueryValueEx (hKey, LoginStepKeyHandle, 0, &type, (LPBYTE) &loginStep, &dataSize);
		if (type == REG_DWORD && dataSize == sizeof(DWORD))
		{
			return (uint) loginStep;
		}
	}
	return 0;
}

//===========================================================================================
void CLoginRegistry::setLoginStep(uint step)
{
	HKEY hKey;
	DWORD dwDisp;
	if(RegCreateKeyEx(HKEY_CURRENT_USER, AppRegEntry, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDisp) == ERROR_SUCCESS)
	{
		DWORD loginStep = step;
		RegSetValueEx(hKey, LoginStepKeyHandle, 0L, REG_DWORD, (const BYTE *) &loginStep, sizeof(DWORD));
	}
}

#else

static uint LoginStep = 0;

//===========================================================================================
std::string CLoginRegistry::getProductInstallId()
{
	srand((uint32)nl_time(0));
	uint32 r = rand();
	r <<= 16;
	r |= rand();
	return NLMISC::toString(r);
}

//===========================================================================================
uint CLoginRegistry::getLoginStep()
{
	return LoginStep;
}

//===========================================================================================
void CLoginRegistry::setLoginStep(uint step)
{
	LoginStep = step;
}

#endif
