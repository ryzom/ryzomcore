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

#ifndef RY_LOGIN_REGISTRY
#define RY_LOGIN_REGISTRY

struct CLoginRegistry
{
	// key for the login registry infos
	static const char *AppRegEntry;
	// Utility function to get a unique install id from the registry
	static std::string getProductInstallId();
	// retrieve login step from the registry (0 if no step yet)
	static uint getLoginStep();
	// set current login step in the registry
	static void setLoginStep(uint step);
};

#endif
