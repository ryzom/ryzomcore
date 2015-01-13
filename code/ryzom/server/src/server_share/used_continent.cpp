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
#include <nel/net/service.h>
#include "used_continent.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;


CUsedContinent	*CUsedContinent::_Instance = NULL;


CUsedContinent::CUsedContinent()
{
	// read the UsedContinent service var
	CConfigFile &cf = IService::getInstance()->ConfigFile;

	CConfigFile::CVar *usedCont = cf.getVarPtr("UsedContinents");

	if (usedCont == NULL)
	{
		nlwarning("UsedContinent: can't find the configuration var 'UsedContinenent' !");
	}
	else
	{
		set<uint32>	usedNumbers;
		for (uint i=0; i<usedCont->size()/2; ++i)
		{
			TContinentInfo ci;
			ci.ContinentName = usedCont->asString(i*2);
			ci.ContinentEnum = CONTINENT::toContinent(ci.ContinentName);
			ci.ContinentInstance = atoui(usedCont->asString(i*2+1).c_str());

			if (usedNumbers.find(ci.ContinentInstance) != usedNumbers.end())
			{
				nlwarning("UsedContinent: error: continent '%s': instance number '%s' invalid or already used, continent will not be available",
					ci.ContinentName.c_str(),
					usedCont->asString(i*2+1).c_str());
			}
			else
			{
				nlinfo("UsedContinent: using continent '%s' with instance %u'",
					ci.ContinentName.c_str(),
					ci.ContinentInstance);
				_Continents.push_back(ci);
				usedNumbers.insert(ci.ContinentInstance);
			}
		}
	}

	// read the continent name translator var
	CConfigFile::CVar *translator = cf.getVarPtr("ContinentNameTranslator");
	if (translator == NULL)
	{
		nlwarning("Can't find the logical to physical continent name translation table !");
	}
	else
	{
		for (uint i=0; i<translator->size()/2; ++i)
		{
			string logical;
			string physical;
			logical = translator->asString(i*2);
			physical = translator->asString(i*2+1);

			nlinfo("Mapping logical continent '%s' to physical continent '%s'",
				logical.c_str(),
				physical.c_str());

			_PhysicalNames.insert(make_pair(logical, physical));
		}
	}
}


bool CUsedContinent::isContinentUsed(const std::string &continentName) const
{
	TUsedContinentCont::const_iterator it(find_if(_Continents.begin(), _Continents.end(), TFindContinent(continentName)));

	return it != _Continents.end();
}

uint32	CUsedContinent::getInstanceForContinent(const std::string &continentName) const
{
	TUsedContinentCont::const_iterator it(find_if(_Continents.begin(), _Continents.end(), TFindContinent(continentName)));

	if (it != _Continents.end())
		return it->ContinentInstance;
	else
		return std::numeric_limits<uint32>::max();
}

uint32	CUsedContinent::getInstanceForContinent(CONTINENT::TContinent continentEnum) const
{
	TUsedContinentCont::const_iterator it(find_if(_Continents.begin(), _Continents.end(), TFindContinentFromEnum(continentEnum)));
	
	if (it != _Continents.end())
		return it->ContinentInstance;
	else
		return std::numeric_limits<uint32>::max();
}

const std::string &CUsedContinent::getContinentForInstance(uint32 instanceNumber) const
{
	static const string emptyString;
	TUsedContinentCont::const_iterator it(find_if(_Continents.begin(), _Continents.end(), TFindInstance(instanceNumber)));

	if (it != _Continents.end())
		return it->ContinentName;
	else

		return emptyString;
}


std::string CUsedContinent::getPhysicalContinentName(const std::string &logicalName) const
{
	std::map<std::string, std::string>::const_iterator it(_PhysicalNames.find(logicalName));
	if (it == _PhysicalNames.end())
	{
		// no translation for this name, juste return it
		return logicalName;
	}
	else
		return it->second;
}
