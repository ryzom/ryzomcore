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

void deleteKey (HKEY hKey, const char *name)
{
	HKEY subKey;
	if (RegOpenKey (hKey, name, &subKey) == ERROR_SUCCESS)
	{
		char subName[512];
		while (RegEnumKey (subKey, 0, subName, 512) == ERROR_SUCCESS)
		{
			deleteKey (subKey, subName);
		}
		nlverify (RegDeleteKey (hKey, name) == ERROR_SUCCESS);
	}
}

// Register an application
bool RegisterApp (const char *appName, const char *appDescription, const char *icon, int iconIndex)
{
	// Create the app key
	HKEY hKey;
	if (RegCreateKey (HKEY_CLASSES_ROOT, appName, &hKey) == ERROR_SUCCESS)
	{
		// Set the description
		RegSetValue (hKey, "", REG_SZ, appDescription, strlen (appDescription));
 
		// Create the icon
		HKEY hKey2;
		if (RegCreateKey (hKey, "DefaultIcon", &hKey2) == ERROR_SUCCESS)
		{
			// Set the description
			char tmp[512];
			smprintf (tmp, 512, "%s,%d", icon, iconIndex);
			RegSetValue (hKey2, "", REG_SZ, tmp, strlen (tmp));
		}

		// Create the shell/open/command
		if (RegCreateKey (hKey, "shell", &hKey) == ERROR_SUCCESS)
		{
			if (RegCreateKey (hKey, "open", &hKey) == ERROR_SUCCESS)
			{
				if (RegCreateKey (hKey, "command", &hKey) == ERROR_SUCCESS)
				{
					// Set the description
					string tmp = string(icon)+" \"%1\"";
					RegSetValue (hKey, "", REG_SZ, tmp.c_str(), tmp.size());
				}
			}
		}

		return true;
	}
	return false;
}

// Unregister an application
bool UnregisterApp (const char *appName)
{
	// Delete the app key
	deleteKey (HKEY_CLASSES_ROOT, appName);
	return true;
}

// Unregister an application command
bool UnregisterAppCommand (const char *appName, const char *command)
{
	// Create the app key
	HKEY hKey;
	if (RegOpenKey (HKEY_CLASSES_ROOT, appName, &hKey) == ERROR_SUCCESS)
	{
		// Create the icon
		char tmp[512];
		smprintf (tmp, 512, "shell\\%s", command);
		deleteKey (hKey, tmp);		
		return true;
	}
	return false;
}

// Register an application command
bool RegisterAppCommand (const char *appName, const char *command, const char *app)
{
	// Create the app key
	HKEY hKey;
	if (RegCreateKey (HKEY_CLASSES_ROOT, appName, &hKey) == ERROR_SUCCESS)
	{
		// Create the icon
		char tmp[512];
		smprintf (tmp, 512, "shell\\%s\\command", command);
		if (RegCreateKey (hKey, tmp, &hKey) == ERROR_SUCCESS)
		{
			// Set the description
			RegSetValue (hKey, "", REG_SZ, app, strlen (app));
		}
		return true;
	}
	return false;
}

// Unregister an application DDE command
bool UnregisterDDECommand (const char *appName, const char *command)
{
	// Create the app key
	HKEY hKey;
	if (RegOpenKey (HKEY_CLASSES_ROOT, appName, &hKey) == ERROR_SUCCESS)
	{
		// Create the icon
		char tmp[512];
		smprintf (tmp, 512, "shell\\%s\\ddeexec", command);
		deleteKey (hKey, tmp);
	}
	return false;
}

// Register an application DDE command
bool RegisterDDECommand (const char *appName, const char *command, const char *ddeCommand, const char *application)
{
	// Create the app key
	HKEY hKey;
	if (RegCreateKey (HKEY_CLASSES_ROOT, appName, &hKey) == ERROR_SUCCESS)
	{
		// Create the icon
		char tmp[512];
		smprintf (tmp, 512, "shell\\%s\\ddeexec", command);
		if (RegCreateKey (hKey, tmp, &hKey) == ERROR_SUCCESS)
		{
			// Set the description
			RegSetValue (hKey, "", REG_SZ, ddeCommand, strlen (ddeCommand));
			HKEY hKey2;
			if (RegCreateKey (hKey, "application", &hKey2) == ERROR_SUCCESS)
			{
				RegSetValue (hKey2, "", REG_SZ, application, strlen (application));
				if (RegCreateKey (hKey, "topic", &hKey2) == ERROR_SUCCESS)
				{
					RegSetValue (hKey2, "", REG_SZ, "system", strlen ("system"));
					return true;
				}
			}
		}
	}
	return false;
}

// Register a file extension
bool RegisterShellFileExt (const char *ext, const char *appName)
{
	// Remove key in explorer registry if exist
	HKEY hKey;
	string key = "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\"+string (ext);
	deleteKey (HKEY_CURRENT_USER, key.c_str ());

	// Create the app key
	if (RegCreateKey (HKEY_CLASSES_ROOT, ext, &hKey) == ERROR_SUCCESS)
	{
		// Set the description
		RegSetValue (hKey, "", REG_SZ, appName, strlen (appName));
		return true;
	}
	return false;
}

// Register a file extension
bool UnregisterShellFileExt (const char *ext)
{
	// Delete the app key
	if (RegDeleteKey  (HKEY_CLASSES_ROOT, ext) == ERROR_SUCCESS)
	{
		return true;
	}
	return false;
}
