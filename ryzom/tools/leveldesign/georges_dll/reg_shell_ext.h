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

#ifndef REG_SHELL_EXT_H_INCLUDED
#define REG_SHELL_EXT_H_INCLUDED
 
// Register an application
bool RegisterApp (const char *appName, const char *appDescription, const char *icon, int iconIndex);

// Unregister an application
bool UnregisterApp (const char *appName);

// Register an application command
bool RegisterAppCommand (const char *appName, const char *command, const char *app);

// Unregister an application command
bool UnregisterAppCommand (const char *appName, const char *command);

// Register an application DDE command
bool RegisterDDECommand (const char *appName, const char *command, const char *ddeCommand, const char *application);

// Unregister an application DDE command
bool UnregisterDDECommand (const char *appName, const char *command);

// Register a file extension
bool RegisterShellFileExt (const char *ext, const char *appName);

#endif // REG_SHELL_EXT_H_INCLUDED
