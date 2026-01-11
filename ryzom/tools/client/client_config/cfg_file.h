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



#ifndef NL_CFG_FILE_H
#define NL_CFG_FILE_H

// ***************************************************************************

bool LoadConfigFile ();

// ***************************************************************************

bool SaveConfigFile ();

// ***************************************************************************

void GetFromConfigFile ();

// ***************************************************************************

void SetToConfigFile ();

// ***************************************************************************

void InvalidateConfig ();

// ***************************************************************************

void ResetConfigFile ();

// ***************************************************************************

void MergeConfigFile (NLMISC::CConfigFile &configFile);

// ***************************************************************************

void ResetConfigFileToDefault ();

// ***************************************************************************

std::string GetString (const char *var);

// ***************************************************************************

sint GetInt (const char *var);

// ***************************************************************************

bool GetBool (const char *var);

// ***************************************************************************

void backupWindowHandleRec (CWnd *cwnd = NULL);

// ***************************************************************************

void removeWindowHandleRec (CWnd *cwnd);

// ***************************************************************************

void localizeWindowsRec (CWnd *cwnd=NULL, bool useMap=true);

// ***************************************************************************

// Special Vars that must not pop up a window if not present in the CFG (because coder features)
std::string	GetStringDriverSound();
void		SetStringDriverSound(const char *value);
sint		GetIntForceLanguage();
sint		GetIntTestConfig();

// ***************************************************************************


#endif // NL_CFG_FILE_H

/* End of database.h */
