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
#include <nel/misc/debug.h>
#include <nel/misc/hierarchical_timer.h>
#include <nel/misc/config_file.h>
#include <nel/misc/path.h>
#include <nel/misc/i18n.h>

// Project includes
#include "nel_qt_config.h"

using namespace std;
using namespace NLMISC;

namespace NLQT {

CConfiguration::CConfiguration()
{
	
}

CConfiguration::~CConfiguration()
{
	
}

void CConfiguration::init()
{	
	// verify data
	nlassert(!m_ConfigCallbacks.size());
	
	// load config
	m_ConfigFile.load(NLQT_CONFIG_FILE);
	
	// log config
	CConfiguration::setAndCallback("NegFiltersDebug", CConfigCallback(this, &CConfiguration::cfcbLogFilter));
	CConfiguration::setAndCallback("NegFiltersInfo", CConfigCallback(this, &CConfiguration::cfcbLogFilter));
	CConfiguration::setAndCallback("NegFiltersWarning", CConfigCallback(this, &CConfiguration::cfcbLogFilter));
	CConfiguration::setAndCallback("NegFiltersAssert", CConfigCallback(this, &CConfiguration::cfcbLogFilter));
	CConfiguration::setAndCallback("NegFiltersError", CConfigCallback(this, &CConfiguration::cfcbLogFilter));

	// set the search paths (kinda important)
	CConfigFile::CVar *var;
	var = m_ConfigFile.getVarPtr("SearchPaths");
	uint varsize = var->size();
	for (uint i = 0; i < varsize; ++i)
		CPath::addSearchPath(var->asString(i), true, false);
	var = m_ConfigFile.getVarPtr("RemapExtensions");
	varsize = var->size();
	for (uint i = 0; i < varsize; i += 2)
		CPath::remapExtension(var->asString(i), var->asString(i + 1), true);
}

void CConfiguration::release()
{
	// bye bye log config
	CConfigFile::CVar emptyVar;
	emptyVar.Name = "NegFiltersDebug";
	CConfiguration::dropCallback(emptyVar.Name);
	cfcbLogFilter(emptyVar);
	emptyVar.Name = "NegFiltersInfo";
	CConfiguration::dropCallback(emptyVar.Name);
	cfcbLogFilter(emptyVar);
	emptyVar.Name = "NegFiltersWarning";
	CConfiguration::dropCallback(emptyVar.Name);
	cfcbLogFilter(emptyVar);
	emptyVar.Name = "NegFiltersAssert";
	CConfiguration::dropCallback(emptyVar.Name);
	cfcbLogFilter(emptyVar);
	emptyVar.Name = "NegFiltersError";
	CConfiguration::dropCallback(emptyVar.Name);
	cfcbLogFilter(emptyVar);
	
	// save and release the config file
	if (m_ConfigFile.exists("SaveConfig") && m_ConfigFile.getVarPtr("SaveConfig")->asBool()) 
	{
		m_ConfigFile.save();
	}
	m_ConfigFile.clear();
		
	// release the search paths etc
	CPath::releaseInstance();
	
	// verify data
	nlassert(!m_ConfigCallbacks.size());
}

void CConfiguration::updateUtilities()
{
	//H_AUTO2
	
	CConfigFile::checkConfigFiles();
}

void CConfiguration::setAndCallback(const std::string &varName, CConfigCallback configCallback)
{
	m_ConfigCallbacks[varName] = configCallback;
	m_ConfigFile.setCallback(varName, cbConfigCallback);
	configCallback(*m_ConfigFile.getVarPtr(varName));
}

void CConfiguration::setCallback(const std::string &varName, CConfigCallback configCallback)
{
	m_ConfigCallbacks[varName] = configCallback;
	m_ConfigFile.setCallback(varName, cbConfigCallback);
}

void CConfiguration::dropCallback(const std::string &varName)
{
	m_ConfigFile.setCallback(varName, NULL);
	m_ConfigCallbacks.erase(varName);
}

float CConfiguration::getValue(const string &varName, float defaultValue)
{
	if (m_ConfigFile.exists(varName)) return m_ConfigFile.getVar(varName).asFloat();
	CConfigFile::CVar varToCopy;
	varToCopy.forceAsDouble((double)defaultValue);	
	m_ConfigFile.insertVar(varName, varToCopy);
	return defaultValue;
}

double CConfiguration::getValue(const string &varName, double defaultValue)
{
	if (m_ConfigFile.exists(varName)) return m_ConfigFile.getVar(varName).asDouble();
	CConfigFile::CVar varToCopy;
	varToCopy.forceAsDouble(defaultValue);	
	m_ConfigFile.insertVar(varName, varToCopy);
	return defaultValue;
}

int CConfiguration::getValue(const string &varName, int defaultValue)
{
	if (m_ConfigFile.exists(varName)) return m_ConfigFile.getVar(varName).asInt();
	CConfigFile::CVar varToCopy;
	varToCopy.forceAsInt(defaultValue);	
	m_ConfigFile.insertVar(varName, varToCopy);
	return defaultValue;
}

string CConfiguration::getValue(const string &varName, const string &defaultValue)
{
	if (m_ConfigFile.exists(varName)) return m_ConfigFile.getVar(varName).asString();
	CConfigFile::CVar varToCopy;
	varToCopy.forceAsString(defaultValue);
	m_ConfigFile.insertVar(varName, varToCopy);
	return defaultValue;
}

ucstring CConfiguration::getValue(const string &varName, const ucstring &defaultValue)
{
	if (m_ConfigFile.exists(varName)) return ucstring::makeFromUtf8(m_ConfigFile.getVar(varName).asString());
	CConfigFile::CVar varToCopy;
	varToCopy.forceAsString(defaultValue.toUtf8());
	m_ConfigFile.insertVar(varName, varToCopy);
	return defaultValue;
}

bool CConfiguration::getValue(const string &varName, bool defaultValue)
{
	if (m_ConfigFile.exists(varName)) return m_ConfigFile.getVar(varName).asBool();
	CConfigFile::CVar varToCopy;
	varToCopy.forceAsInt(defaultValue ? 1 : 0);	
	m_ConfigFile.insertVar(varName, varToCopy);
	return defaultValue;
}

CRGBA CConfiguration::getValue(const string &varName, const CRGBA &defaultValue)
{
	if (m_ConfigFile.exists(varName)) 
	{
		return getValue(m_ConfigFile.getVar(varName), defaultValue);
	}
	else
	{
		// create a new value only if one doesn't exist
		CConfigFile::CVar varToCopy;
		varToCopy.forceAsInt(defaultValue.R);
		varToCopy.setAsInt(defaultValue.G, 1);
		varToCopy.setAsInt(defaultValue.B, 2);
		varToCopy.setAsInt(defaultValue.A, 3);
		m_ConfigFile.insertVar(varName, varToCopy);
	}
	return defaultValue;
}

CRGBA CConfiguration::getValue(const CConfigFile::CVar &var, const CRGBA &defaultValue)
{
	if (var.size() >= 3)
	{
		if (var.size() > 4) nlwarning("RGBA value in config value '%s' is too long, ignoring unused values");
		return CRGBA((uint8)var.asInt(0), (uint8)var.asInt(1), (uint8)var.asInt(2), var.size() >= 4 ? (uint8)var.asInt(3) : 255);
	}
	nlwarning("Invalid RGBA value in config value '%s', reverting to default { %i, %i, %i, %i }", var.Name.c_str(), (sint)defaultValue.R, (sint)defaultValue.G, (sint)defaultValue.B, (sint)defaultValue.A);	
	return defaultValue;
}

void CConfiguration::cbConfigCallback(NLMISC::CConfigFile::CVar &var)
{
	CConfiguration::getInstance()->m_ConfigCallbacks[var.Name](var);
}

void CConfiguration::cfcbLogFilter(CConfigFile::CVar &var)
{
	// from nel/net/service.cpp	
	CLog *log = NULL;
	if (var.Name == "NegFiltersDebug") log = DebugLog;
	else if (var.Name == "NegFiltersInfo") log = InfoLog;
	else if (var.Name == "NegFiltersWarning") log = WarningLog;
	else if (var.Name == "NegFiltersAssert") log = AssertLog;
	else if (var.Name == "NegFiltersError") log = ErrorLog;
	else nlstop;

	// remove all old filters from config file
	CConfigFile::CVar &oldvar = m_ConfigFile.getVar(var.Name);
	for (uint j = 0; j < oldvar.size(); j++)
		log->removeFilter(oldvar.asString(j).c_str());
	
	// add all new filters from config file
	for (uint i = 0; i < var.size(); i++)
		log->addNegativeFilter(var.asString(i).c_str());
}

} /* namespace NLQT */

/* end of file */
