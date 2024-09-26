/*
Georges Editor Qt
Copyright (C) 2010 Adrian Jaekel <aj at elane2k dot com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "configuration.h"

// STL includes

// Qt includes
#include <QFile>

// NeL includes
#include <nel/misc/debug.h>
#include <nel/misc/hierarchical_timer.h>
#include <nel/misc/config_file.h>
#include <nel/misc/path.h>

// Project includes
#include "modules.h"
#include "progress_dialog.h"

using namespace std;
using namespace NLMISC;

namespace NLQT {

	CConfiguration::CConfiguration() : _progressCB(0)
	{

	}

	CConfiguration::~CConfiguration()
	{

	}

	void CConfiguration::init()
	{	
		// load config
		QFile file(NLQT_CONFIG_FILE);
		if (!file.exists())
		{
			file.open( QIODevice::WriteOnly | QIODevice::Text );
			file.write("GraphicsDrivers = { \"OpenGL\", \"Direct3D\" };");
			file.write("\nSearchPaths = {\"\"};");
			file.write("\nRemapExtensions = { \"png\", \"tga\" };");
			file.write("\nBackgroundColor = { 0, 0, 0 };");
			//file.write("\nQtStyle = \"\";");
			//file.write("\nQtPalette = \"\";");
			file.close();
		}

		try
		{
			ConfigFile.load(NLQT_CONFIG_FILE);
		}
		catch(...)
		{
		}

		addLeveldesignPath();
		addSearchPaths();
		configRemapExtensions();
	}

	void CConfiguration::release()
	{
		//Modules::config().dropCallback("SearchPaths");

		// save and release the config file
		if (ConfigFile.exists("SaveConfig") && ConfigFile.getVarPtr("SaveConfig")->asBool()) 
		{
			ConfigFile.save();
		}
		ConfigFile.clear();

		// release the search paths etc
		CPath::releaseInstance();
	}

	void CConfiguration::updateUtilities()
	{
		//H_AUTO2
		CConfigFile::checkConfigFiles();
	}

	void CConfiguration::addLeveldesignPath()
	{
		std::vector<std::string> list;
		list.push_back(Modules::config().getValue("LeveldesignPath", std::string()));
		addSearchPaths(&list);
	}

	void CConfiguration::configRemapExtensions()
	{
		CConfigFile::CVar *var;
		var = ConfigFile.getVarPtr("RemapExtensions");
		uint varsize = var->size();
		for (uint i = 0; i < varsize; i += 2)
			CPath::remapExtension(var->asString(i), var->asString(i + 1), true);
	}

	float CConfiguration::getValue(const string &varName, float defaultValue)
	{
		if (ConfigFile.exists(varName)) return ConfigFile.getVar(varName).asFloat();
		CConfigFile::CVar varToCopy;
		varToCopy.forceAsDouble((double)defaultValue);	
		ConfigFile.insertVar(varName, varToCopy);
		return defaultValue;
	}

	double CConfiguration::getValue(const string &varName, double defaultValue)
	{
		if (ConfigFile.exists(varName)) return ConfigFile.getVar(varName).asDouble();
		CConfigFile::CVar varToCopy;
		varToCopy.forceAsDouble(defaultValue);	
		ConfigFile.insertVar(varName, varToCopy);
		return defaultValue;
	}

	int CConfiguration::getValue(const string &varName, int defaultValue)
	{
		if (ConfigFile.exists(varName)) return ConfigFile.getVar(varName).asInt();
		CConfigFile::CVar varToCopy;
		varToCopy.forceAsInt(defaultValue);	
		ConfigFile.insertVar(varName, varToCopy);
		return defaultValue;
	}

	string CConfiguration::getValue(const string &varName, const string &defaultValue)
	{
		if (ConfigFile.exists(varName)) return ConfigFile.getVar(varName).asString();
		CConfigFile::CVar varToCopy;
		varToCopy.forceAsString(defaultValue);
		ConfigFile.insertVar(varName, varToCopy);
		return defaultValue;
	}

	ucstring CConfiguration::getValue(const string &varName, const ucstring &defaultValue)
	{
		if (ConfigFile.exists(varName)) return ucstring::makeFromUtf8(ConfigFile.getVar(varName).asString());
		CConfigFile::CVar varToCopy;
		varToCopy.forceAsString(defaultValue.toUtf8());
		ConfigFile.insertVar(varName, varToCopy);
		return defaultValue;
	}

	bool CConfiguration::getValue(const string &varName, bool defaultValue)
	{
		if (ConfigFile.exists(varName)) return ConfigFile.getVar(varName).asBool();
		CConfigFile::CVar varToCopy;
		varToCopy.forceAsInt(defaultValue ? 1 : 0);	
		ConfigFile.insertVar(varName, varToCopy);
		return defaultValue;
	}

	CRGBA CConfiguration::getValue(const string &varName, const CRGBA &defaultValue)
	{
		if (ConfigFile.exists(varName)) 
		{
			return getValue(ConfigFile.getVar(varName), defaultValue);
		}
		else
		{
			// create a new value only if one doesn't exist
			CConfigFile::CVar varToCopy;
			varToCopy.forceAsInt(defaultValue.R);
			varToCopy.setAsInt(defaultValue.G, 1);
			varToCopy.setAsInt(defaultValue.B, 2);
			varToCopy.setAsInt(defaultValue.A, 3);
			ConfigFile.insertVar(varName, varToCopy);
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

	void CConfiguration::addSearchPaths(std::vector<std::string>* list)
	{
		//Modules::config().getConfigFile().getVar("SearchPaths");

		std::vector<std::string> *tmpList = list;
		if (!tmpList)
		{
			NLMISC::CConfigFile::CVar v = getConfigFile().getVar("SearchPaths");
			tmpList = new std::vector<std::string>();
			for (uint i = 0; i < v.size(); ++i)
			{
				tmpList->push_back(v.asString(i));
			}
		}

		uint listsize = tmpList->size();
		for (uint i = 0; i < listsize; ++i) 
		{
			if(_progressCB)
			{
				_progressCB->DisplayString = tmpList->at(i);
				CPath::addSearchPath(tmpList->at(i), true, false, _progressCB);
			}
			else
			{
				CProgressDialog pcb;
				pcb.DisplayString = tmpList->at(i);
				pcb.show();
				CPath::addSearchPath(tmpList->at(i), true, false, &pcb);
			}
		}
		if (!list)
			delete tmpList;
	}

	void CConfiguration::setProgressCallback(NLMISC::IProgressCallback *cb)
	{
		_progressCB = cb;
	}

} /* namespace NLQT */