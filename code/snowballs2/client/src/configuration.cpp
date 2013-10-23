// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#include <nel/misc/types_nl.h>
#include "configuration.h"

// STL includes

// NeL includes
// #include <nel/misc/debug.h>
#include <nel/misc/config_file.h>
#include <nel/misc/path.h>
#include <nel/misc/i18n.h>

// Project includes
#include "snowballs_client.h"
#include "snowballs_config.h"

using namespace std;
using namespace NLMISC;

namespace SBCLIENT {

void CConfiguration::setAndCallback(const std::string &varName, void (*cb)(CConfigFile::CVar &var))
{
	ConfigFile->setCallback(varName, cb);
	cb(*ConfigFile->getVarPtr(varName));
}

void CConfiguration::dropCallback(const std::string &varName)
{
	ConfigFile->setCallback(varName, NULL);
}

bool CConfiguration::init()
{
	nlassert(!ConfigFile); ConfigFile = new CConfigFile(); nlassert(ConfigFile);
	ConfigFile->load(SBCLIENT_CONFIG_FILE);

	// set the search paths (kinda important)
	CConfigFile::CVar *var;
	var = ConfigFile->getVarPtr("SearchPaths");
	uint varsize = var->size();
	for (uint i = 0; i < varsize; ++i)
		CPath::addSearchPath(var->asString(i), true, false);
	var = ConfigFile->getVarPtr("RemapExtensions");
	varsize = var->size();
	for (uint i = 0; i < varsize; i += 2)
		CPath::remapExtension(var->asString(i), var->asString(i + 1), true);

	return true;
}

bool CConfiguration::release()
{
	// save and release the config file
	if (ConfigFile) 
	{ 
		if (ConfigFile->exists("SaveConfig") && ConfigFile->getVarPtr("SaveConfig")->asBool()) 
		{
			ConfigFile->save();
		}
		delete ConfigFile; 
		ConfigFile = NULL; 
	} 
	else nlwarning("!ConfigFile");
		
	// release the search paths etc
	CPath::releaseInstance();

	return true;
}

void CConfiguration::updateUtilities()
{
	CConfigFile::checkConfigFiles();
}

float CConfiguration::getValue(const string &varName, float defaultValue)
{
	if (ConfigFile->exists(varName)) return ConfigFile->getVar(varName).asFloat();
	CConfigFile::CVar varToCopy;
	varToCopy.forceAsDouble((double)defaultValue);	
	ConfigFile->insertVar(varName, varToCopy);
	return defaultValue;
}

double CConfiguration::getValue(const string &varName, double defaultValue)
{
	if (ConfigFile->exists(varName)) return ConfigFile->getVar(varName).asDouble();
	CConfigFile::CVar varToCopy;
	varToCopy.forceAsDouble(defaultValue);	
	ConfigFile->insertVar(varName, varToCopy);
	return defaultValue;
}

int CConfiguration::getValue(const string &varName, int defaultValue)
{
	if (ConfigFile->exists(varName)) return ConfigFile->getVar(varName).asInt();
	CConfigFile::CVar varToCopy;
	varToCopy.forceAsInt(defaultValue);	
	ConfigFile->insertVar(varName, varToCopy);
	return defaultValue;
}

string CConfiguration::getValue(const string &varName, const string &defaultValue)
{
	if (ConfigFile->exists(varName)) return ConfigFile->getVar(varName).asString();
	CConfigFile::CVar varToCopy;
	varToCopy.forceAsString(defaultValue);
	ConfigFile->insertVar(varName, varToCopy);
	return defaultValue;
}

ucstring CConfiguration::getValue(const string &varName, const ucstring &defaultValue)
{
	if (ConfigFile->exists(varName)) return ucstring::makeFromUtf8(ConfigFile->getVar(varName).asString());
	CConfigFile::CVar varToCopy;
	varToCopy.forceAsString(defaultValue.toUtf8());
	ConfigFile->insertVar(varName, varToCopy);
	return defaultValue;
}

bool CConfiguration::getValue(const string &varName, bool defaultValue)
{
	if (ConfigFile->exists(varName)) return ConfigFile->getVar(varName).asBool();
	CConfigFile::CVar varToCopy;
	varToCopy.forceAsInt(defaultValue ? 1 : 0);	
	ConfigFile->insertVar(varName, varToCopy);
	return defaultValue;
}

CRGBA CConfiguration::getValue(const string &varName, const CRGBA &defaultValue)
{
	if (ConfigFile->exists(varName)) 
	{
		return getValue(ConfigFile->getVar(varName), defaultValue);
	}
	else
	{
		// create a new value only if one doesn't exist
		CConfigFile::CVar varToCopy;
		varToCopy.forceAsInt(defaultValue.R);
		varToCopy.setAsInt(defaultValue.G, 1);
		varToCopy.setAsInt(defaultValue.B, 2);
		varToCopy.setAsInt(defaultValue.A, 3);
		ConfigFile->insertVar(varName, varToCopy);
	}
	return defaultValue;
}

CRGBA CConfiguration::getValue(const CConfigFile::CVar &var, const CRGBA &defaultValue)
{
	if (var.size() >= 3)
	{
		if (var.size() > 4) nlwarning("RGBA value in config value '%s' is too long, ignoring unused values");
		return CRGBA(var.asInt(0), var.asInt(1), var.asInt(2), var.size() >= 4 ? var.asInt(3) : 255);
	}
	nlwarning("Invalid RGBA value in config value '%s', reverting to default { %i, %i, %i, %i }", var.Name.c_str(), (sint)defaultValue.R, (sint)defaultValue.G, (sint)defaultValue.B, (sint)defaultValue.A);	
	return defaultValue;
}

} /* namespace SBCLIENT */

/* end of file */
