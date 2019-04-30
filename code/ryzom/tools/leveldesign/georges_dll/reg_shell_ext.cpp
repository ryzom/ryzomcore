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

#include "stdafx.h"
#include "reg_shell_ext.h"
#include "nel/misc/common.h"

using namespace std;
using namespace NLMISC;

void deleteKey(HKEY hKey, const TCHAR *name)
{
	HKEY subKey;
	if (RegOpenKey(hKey, name, &subKey) == ERROR_SUCCESS)
	{
		TCHAR subName[512];
		while (RegEnumKey(subKey, 0, subName, 512) == ERROR_SUCCESS)
		{
			deleteKey(subKey, subName);
		}
		nlverify(RegDeleteKey(hKey, name) == ERROR_SUCCESS);
	}
}

// Register an application
bool RegisterApp(const char *appName, const char *appDescription, const char *icon, int iconIndex)
{
	// Create the app key
	HKEY hKey;
	if (RegCreateKey(HKEY_CLASSES_ROOT, nlUtf8ToTStr(appName), &hKey) == ERROR_SUCCESS)
	{
		// Set the description
		tstring tAppDescription = utf8ToTStr(appDescription);
		RegSetValue(hKey, _T(""), REG_SZ, tAppDescription.c_str(), (tAppDescription.size() + 1) * sizeof(TCHAR));

		// Create the icon
		HKEY hKey2;
		if (RegCreateKey(hKey, _T("DefaultIcon"), &hKey2) == ERROR_SUCCESS)
		{
			// Set the description
			char tmp[512];
			smprintf(tmp, 512, "%s,%d", icon, iconIndex);
			tstring ttmp = utf8ToTStr(tmp);
			RegSetValue(hKey2, _T(""), REG_SZ, ttmp.c_str(), (ttmp.size() + 1) * sizeof(TCHAR));
		}

		// Create the shell/open/command
		if (RegCreateKey(hKey, _T("shell"), &hKey) == ERROR_SUCCESS)
		{
			if (RegCreateKey(hKey, _T("open"), &hKey) == ERROR_SUCCESS)
			{
				if (RegCreateKey(hKey, _T("command"), &hKey) == ERROR_SUCCESS)
				{
					// Set the description
					string tmp = string(icon) + " \"%1\"";
					tstring ttmp = utf8ToTStr(tmp);
					RegSetValue(hKey, _T(""), REG_SZ, ttmp.c_str(), (ttmp.size() + 1) * sizeof(TCHAR));
				}
			}
		}

		return true;
	}
	return false;
}

// Unregister an application
bool UnregisterApp(const char *appName)
{
	// Delete the app key
	deleteKey(HKEY_CLASSES_ROOT, nlUtf8ToTStr(appName));
	return true;
}

// Unregister an application command
bool UnregisterAppCommand(const char *appName, const char *command)
{
	// Create the app key
	HKEY hKey;
	if (RegOpenKey(HKEY_CLASSES_ROOT, nlUtf8ToTStr(appName), &hKey) == ERROR_SUCCESS)
	{
		// Create the icon
		char tmp[512];
		smprintf(tmp, 512, "shell\\%s", command);
		deleteKey(hKey, nlUtf8ToTStr(tmp));
		return true;
	}
	return false;
}

// Register an application command
bool RegisterAppCommand(const char *appName, const char *command, const char *app)
{
	// Create the app key
	HKEY hKey;
	if (RegCreateKey(HKEY_CLASSES_ROOT, nlUtf8ToTStr(appName), &hKey) == ERROR_SUCCESS)
	{
		// Create the icon
		char tmp[512];
		smprintf(tmp, 512, "shell\\%s\\command", command);
		if (RegCreateKey(hKey, nlUtf8ToTStr(tmp), &hKey) == ERROR_SUCCESS)
		{
			// Set the description
			tstring tapp = utf8ToTStr(app);
			RegSetValue(hKey, _T(""), REG_SZ, tapp.c_str(), (tapp.size() + 1) * sizeof(TCHAR));
		}
		return true;
	}
	return false;
}

// Unregister an application DDE command
bool UnregisterDDECommand(const char *appName, const char *command)
{
	// Create the app key
	HKEY hKey;
	if (RegOpenKey(HKEY_CLASSES_ROOT, nlUtf8ToTStr(appName), &hKey) == ERROR_SUCCESS)
	{
		// Create the icon
		char tmp[512];
		smprintf(tmp, 512, "shell\\%s\\ddeexec", command);
		deleteKey(hKey, nlUtf8ToTStr(tmp));
	}
	return false;
}

// Register an application DDE command
bool RegisterDDECommand(const char *appName, const char *command, const char *ddeCommand, const char *application)
{
	// Create the app key
	HKEY hKey;
	if (RegCreateKey(HKEY_CLASSES_ROOT, nlUtf8ToTStr(appName), &hKey) == ERROR_SUCCESS)
	{
		// Create the icon
		char tmp[512];
		smprintf(tmp, 512, "shell\\%s\\ddeexec", command);
		if (RegCreateKey(hKey, nlUtf8ToTStr(tmp), &hKey) == ERROR_SUCCESS)
		{
			// Set the description
			tstring tddeCommand = utf8ToTStr(ddeCommand);
			RegSetValue(hKey, _T(""), REG_SZ, tddeCommand.c_str(), (tddeCommand.size() + 1) * sizeof(TCHAR));
			HKEY hKey2;
			if (RegCreateKey(hKey, _T("application"), &hKey2) == ERROR_SUCCESS)
			{
				tstring tapplication = utf8ToTStr(application);
				RegSetValue(hKey2, _T(""), REG_SZ, tapplication.c_str(), (tapplication.size() + 1) * sizeof(TCHAR));
				if (RegCreateKey(hKey, _T("topic"), &hKey2) == ERROR_SUCCESS)
				{
					RegSetValue(hKey2, _T(""), REG_SZ, _T("system"), (strlen("system") + 1) * sizeof(TCHAR));
					return true;
				}
			}
		}
	}
	return false;
}

// Register a file extension
bool RegisterShellFileExt(const char *ext, const char *appName)
{
	// Remove key in explorer registry if exist
	HKEY hKey;
	string key = "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\" + string(ext);
	deleteKey(HKEY_CURRENT_USER, nlUtf8ToTStr(key));

	// Create the app key
	if (RegCreateKey(HKEY_CLASSES_ROOT, nlUtf8ToTStr(ext), &hKey) == ERROR_SUCCESS)
	{
		// Set the description
		tstring tAppName = utf8ToTStr(appName);
		RegSetValue(hKey, _T(""), REG_SZ, tAppName.c_str(), (tAppName.size() + 1) * sizeof(TCHAR));
		return true;
	}
	return false;
}

// Register a file extension
bool UnregisterShellFileExt(const char *ext)
{
	// Delete the app key
	if (RegDeleteKey(HKEY_CLASSES_ROOT, nlUtf8ToTStr(ext)) == ERROR_SUCCESS)
	{
		return true;
	}
	return false;
}
