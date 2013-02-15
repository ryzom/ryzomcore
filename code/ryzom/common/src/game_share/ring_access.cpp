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

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "stdpch.h"

#include "game_share/ring_access.h"

#include "game_share/scenario_entry_points.h"
#include "game_share/object.h"

#include "nel/misc/file.h"
#include "nel/misc/command.h"
#include "game_share/utils.h"
#include "scenario_entry_points.h"
#include "nel/misc/o_xml.h"
#include "nel/misc/i_xml.h"
#include "nel/misc/algo.h"
#include "nel/misc/xml_auto_ptr.h"



using namespace std;
using namespace R2;
using namespace NLMISC;

static const std::string RingAccessFilename = "r2_ring_access.xml";
/*
namespace std
{
	// STL port fixup
#ifdef NL_COMP_VC8
	inline void bit_swap(priv::_Bit_reference __x, priv::_Bit_reference __y)
#else
	inline void bit_swap(_Bit_reference __x, _Bit_reference __y)
#endif
	{
		  bool __tmp = (bool)__x;
		  __x = __y;
		  __y = __tmp;
	}

}
*/

inline void bit_swap(bool __x, bool __y)
{
	  bool __tmp = __x;
	  __x = __y;
	  __y = __tmp;
}


std::string CVerfiyRightRtScenarioError::toString() const
{

	const char*  typeTrad[] = { "None", "InvalidData", "InvalidIslandLevel", "InvalidBotLevel" };
	TType type = Type;
	if ( static_cast<uint>(type) >  static_cast<uint>(InvalidBotLevel) )
	{
		type = None;
	}

	std::string typeStr( typeTrad[static_cast<uint>(Type)]);

	return NLMISC::toString("Type: '%s' '%s'  : Package: '%s' needed: %d, user: %d)", typeStr.c_str(), Name.c_str(), Package.c_str(), Level, CharLevel );

}

CRingAccess::CRingAccess()
{
	_Initialised = false;
}

bool CRingAccess::isPlotItemSheetId(const NLMISC::CSheetId& sheetId ) const
{
	return _R2PlotItemSheetId.find(sheetId) != _R2PlotItemSheetId.end();
}

void CRingAccess::init()
{



	if (_Initialised ) { return; } // no double initialisation
	_CustomNpcSheetId.clear();
	//CSheetId::init() must be called first
	_CustomNpcSheetId.insert(CSheetId("basic_matis_male.creature"));
	_CustomNpcSheetId.insert(CSheetId("basic_fyros_male.creature"));
	_CustomNpcSheetId.insert(CSheetId("basic_tryker_male.creature"));
	_CustomNpcSheetId.insert(CSheetId("basic_zorai_male.creature"));

	_CustomNpcSheetId.insert(CSheetId("basic_matis_female.creature"));
	_CustomNpcSheetId.insert(CSheetId("basic_fyros_female.creature"));
	_CustomNpcSheetId.insert(CSheetId("basic_tryker_female.creature"));
	_CustomNpcSheetId.insert(CSheetId("basic_zorai_female.creature"));


	for (uint32 i = 0 ; i <= 184 ; ++i)
	{
		_R2PlotItemSheetId.insert( CSheetId( NLMISC::toString("r2_plot_item_%d.sitem", i)));
	}

	_SheetIdToAccess.clear();//only useful when manualy re init file
	// File stream
	CIFile file;


	std::string pathFileName = CPath::lookup(RingAccessFilename, false, false, false);
	// Open the file
	if (pathFileName.empty() || !file.open(pathFileName.c_str()))
	{
		nlinfo("Can't open the file for reading : %s", RingAccessFilename.c_str());
		return;
	}



	// Create the XML stream
	CIXml input;

	// Init
	if(input.init(file))
	{
		xmlNodePtr entitiesAccess = input.getRootNode();
		xmlNodePtr entityAccess = input.getFirstChildNode(entitiesAccess, "entityAccess");

		while (entityAccess != 0)
		{

			// island name
			CXMLAutoPtr namePtr( (const char*) xmlGetProp(entityAccess, (xmlChar*) "name") );
			CXMLAutoPtr packagePtr( (const char*) xmlGetProp(entityAccess, (xmlChar*) "package") );
			CXMLAutoPtr sheetClientPtr( (const char*) xmlGetProp(entityAccess, (xmlChar*) "sheetClient") );
			CXMLAutoPtr sheetPtr( (const char*) xmlGetProp(entityAccess, (xmlChar*) "sheetServer") );

			if (!namePtr.getDatas()|| !packagePtr.getDatas() || !sheetPtr.getDatas() || !sheetPtr.getDatas())
			{
				nlerror( "Syntax error in %s", pathFileName.c_str());
				return;
			}

			std::string sheet( sheetPtr.getDatas() );
			std::string package( packagePtr.getDatas() );
			std::string sheetClient(sheetClientPtr.getDatas());


			CSheetId sheetClientId(sheetClient);
			CSheetId sheetId; // no sheet server

			if (sheet.empty())
			{
				bool ok = _SheetIdToAccess.insert( std::make_pair(std::make_pair(sheetClientId, sheetId), package)).second;
				if (!ok)
				{
					std::string previousPackage = _SheetIdToAccess[std::make_pair(sheetClientId, sheetId)];
					// only display warning if one key has multiple package
					if ( previousPackage != package )
					{
						nlwarning("%s: Entity %s sheet(%s) is defined more than once with different package definition. Previous definition is '%s', current definition is '%s'", RingAccessFilename.c_str(), namePtr.getDatas(), sheetClientPtr.getDatas(), previousPackage.c_str(), package.c_str());
					}

				}
			}
			else
			{


				sheetId = CSheetId(sheet);
				if (_CustomNpcSheetId.find(sheetClientId) != _CustomNpcSheetId.end())
				{

					bool ok = _SheetIdToAccess.insert( std::make_pair(std::make_pair(sheetClientId, sheetId), package)).second;
					if (!ok)
					{
						std::string previousPackage = _SheetIdToAccess[std::make_pair(sheetClientId, sheetId)];
						// only display warning if one key has multiple package
						if ( previousPackage != package )
						{
							nlwarning("%s: Entity %s sheet(%s) is defined more than once. Previous definition is '%s', current definition is '%s'", RingAccessFilename.c_str(), namePtr.getDatas(), sheetPtr.getDatas(), previousPackage.c_str(), package.c_str());
						}
					}
				}
				else
				{
					nlwarning("%s: Entity %s has invalid sheets %s %s", RingAccessFilename.c_str(), namePtr.getDatas(), sheetClientPtr.getDatas(), sheetPtr.getDatas());
				}
			}


			entityAccess = input.getNextChildNode(entityAccess, "entityAccess");
		}
	}

	// Close the file
	file.close ();

	_Initialised = true;

}


bool CRingAccess::isSheetClient(const NLMISC::CSheetId& sheet) const
{

	static const std::string racesStr[] = { "matis_", "fyros_", "tryker_", "zorai_"};
	static const uint32 racesLen[] = { 6, 6, 7 ,6 };
	static const std::string sexsStr[] = { "male", "female"};
	static const uint32 sexsLen[] = { 4, 6};
	static const std::string basic = "basic_";
	static const uint32 basicLength = 6;


	std::string sheetStr = sheet.toString();
	uint32 sheetLength = (uint32)sheetStr.length();



	if (sheetLength >= basicLength && sheetStr.substr(0, basicLength) == basic)
	{
		// good thing is a basic_* something
		for (uint32 i = 0; i < 4 ; ++i)
		{
			const uint32 raceLength = racesLen[i];
			if (  sheetLength > basicLength + raceLength && sheetStr.substr(basicLength, raceLength) ==  racesStr[i])
			{

				// good thing is a basic_race_
				for (uint32 j = 0; j < 2 ; ++j)
				{
					uint32 sexLength = sexsLen[j];
					if (sheetLength > basicLength + raceLength + sexLength && sheetStr.substr(basicLength + raceLength, sexLength)  == sexsStr[j])
					{
						return true;
					}
				}
			}
		}
	}

	return false;
}

std::string CRingAccess::getSheetIdAccessInfo(const NLMISC::CSheetId& sheetClientId, const NLMISC::CSheetId& sheetServerId) const
{
	const_cast<R2::CRingAccess*>(this)->init(); //lazy initialization;


	// We can not verify ring access based only on sheet
	// eg a tryker  level 250 whith VPA of a matis must be seen as a matis :(
	if ( isSheetClient(sheetClientId) )
	{
		return "a1";
	}

	TSheetIdToAccess::const_iterator found(_SheetIdToAccess.find(std::make_pair(sheetClientId, sheetServerId)));
	if (found == _SheetIdToAccess.end()) { return ""; }
	return found->second;

}

std::string CRingAccess::getSheetAccessInfo(const std::string& sheetClient, const std::string&sheetServer) const
{


	const_cast<R2::CRingAccess*>(this)->init(); //lazy initialization;



	NLMISC::CSheetId sheetClientId(sheetClient);
	NLMISC::CSheetId sheetServerId;
	if (!sheetServer.empty()) { sheetServerId = NLMISC::CSheetId(sheetServer); }



	return getSheetIdAccessInfo(sheetClientId, sheetServerId);
}

std::string CRingAccess::getIslandAccessInfo(const std::string& islandeName) const
{
	CScenarioEntryPoints::CCompleteIsland* island = CScenarioEntryPoints::getInstance().getIslandFromId(islandeName);
	if (!island) { return "";}
	return island->Package;
}

void CRingAccess::getRingAccessAsMap(const std::string&ringAccess, std::map<std::string, int> & ringAccessAsMap) const
{
	std::vector<std::string> vect;
	NLMISC::splitString(ringAccess,":" , vect);
	std::vector<std::string>::iterator  first(vect.begin()), last(vect.end());
	for ( ;first != last; ++first)
	{

		std::string::size_type f = 0;
		std::string::size_type l = first->size();
		const std::string &s = *first;
		for (; f != l && (s[f] < '0' || s[f] > '9') ; ++f) {}
		if (f != l)
		{
			std::string package = s.substr(0, f);
			int level;
			fromString(s.substr(f, l   - f), level);
			ringAccessAsMap[package] = level;
		}

	}
}


std::string CRingAccess::upgradeRingAccess(const std::string& defaultValue, const std::string& bonusValue) const
{

	std::map<std::string, int> defaultMap;
	std::map<std::string, int> bonusMap;

	getRingAccessAsMap(defaultValue, defaultMap);
	getRingAccessAsMap(bonusValue, bonusMap);
	// get the highest value between default value and bonus Value
	{
		std::map<std::string, int>::const_iterator first(bonusMap.begin()), last(bonusMap.end());
		for (; first != last; ++first)
		{
			std::map<std::string, int>::iterator found = defaultMap.find(first->first);
			if (found == defaultMap.end())
			{
				defaultMap.insert( *first);
			}
			else
			{
				if (found->second < first->second) { found->second = first->second; }
			}
		}
	}

	// return the new value as string
	std::string toRet ="";
	{

		std::map<std::string, int>::const_iterator first(defaultMap.begin()), last(defaultMap.end());
		for (; first != last; ++first)
		{
			if (!toRet.empty()) { toRet += ":"; }
			toRet += NLMISC::toString("%s%d", first->first.c_str(), first->second);
		}

	}
	return toRet;

}

bool CRingAccess::verifyRight(const std::string& askedAccess, const std::string& allowedAccess) const
{
	std::map<std::string, int> askedAccessMap;
	std::map<std::string, int> allowedAccessMap;
	std::string package; int neededLevel; int charLevel;

	getRingAccessAsMap(allowedAccess, allowedAccessMap);
	getRingAccessAsMap(askedAccess, askedAccessMap);
	return verifyRight(askedAccessMap, allowedAccessMap, package, neededLevel, charLevel);

}

bool CRingAccess::verifyRight(const std::map<std::string, int>& askedAccessMap, const std::map<std::string, int>& allowedAccessMap, std::string& package, int& neededLevel, int& charLevel) const
{
	std::map<std::string, int>::const_iterator first(askedAccessMap.begin()), last(askedAccessMap.end());
	for ( ; first != last; ++first)
	{
		std::map<std::string, int>::const_iterator found( allowedAccessMap.find(first->first));
		if (found == allowedAccessMap.end()) { package = first->first; neededLevel = first->second; charLevel = 0; return false;  }
		if ( found->second < first->second ) { package = first->first; neededLevel = first->second; charLevel = found->second; return false;  }
	}
	return true;
}




bool CRingAccess::verifyRtScenario(CObject* rtScenario, const std::string& charRingAccess, CVerfiyRightRtScenarioError* &err) const
{
	if (!rtScenario) { err= new CVerfiyRightRtScenarioError(CVerfiyRightRtScenarioError::InvalidData); return false;}

	std::map<std::string, int> charRingAccessMap;
	std::map<std::string, int> entityAccessMap;


	std::string package;
	int neededLevel;
	int charLevel;


	getRingAccessAsMap(charRingAccess, charRingAccessMap);


	// verify Location
	CObject* locations = rtScenario->getAttr("Locations");
	if (!locations || !locations->isTable() )
	{
		err = new CVerfiyRightRtScenarioError(CVerfiyRightRtScenarioError::InvalidData);
		return false;
	}

	uint32 firstLocation = 0;
	uint32 lastLocation = locations->getSize();


	for ( ;firstLocation != lastLocation; ++firstLocation)
	{
		CObject* location = locations->getValue(firstLocation);
		if (!location || !location->isString("Island"))
		{
			err = new CVerfiyRightRtScenarioError(CVerfiyRightRtScenarioError::InvalidData);
			return false;
		}
		std::string islandName = location->toString("Island");
		std::string access = getIslandAccessInfo(islandName);
		getRingAccessAsMap(access, entityAccessMap);
		if (!verifyRight(entityAccessMap, charRingAccessMap, package, neededLevel, charLevel))
		{
			err = new CVerfiyRightRtScenarioError(CVerfiyRightRtScenarioError::InvalidIslandLevel, islandName, package, neededLevel, charLevel);
			return false;
		}
	}

	// verify npcs
	CObject* acts = rtScenario->getAttr("Acts");
	if (!acts || !acts->isTable()) { return false; }

	uint32 firstActIndex = 0;
	uint32 lastActIndex = acts->getSize();
	for ( ;firstActIndex != lastActIndex; ++firstActIndex)
	{
		CObject* act = acts->getValue(firstActIndex);
		if (!act || !act->isTable() || !act->isTable("Npcs"))
		{
			err = new CVerfiyRightRtScenarioError(CVerfiyRightRtScenarioError::InvalidData);
			return false;
		}

		CObject* npcs = act->getAttr("Npcs");
		uint32 firstNpcIndex = 0;
		uint32 lastNpcIndex = npcs->getSize();
		for (; firstNpcIndex != lastNpcIndex; ++firstNpcIndex)
		{
			CObject* npc = npcs->getValue(firstNpcIndex);
			if (npc && npc->isTable() && npc->isString("SheetClient") && npc->isString("Sheet"))
			{
				std::string botName;
				if ( npc->isString("Name"))
				{
					botName = npc->toString("Name");
				}
				std::string sheetClient = npc->toString("SheetClient");
				std::string sheet = npc->toString("Sheet");
				std::string access = getSheetAccessInfo(sheetClient, sheet);
				getRingAccessAsMap(access, entityAccessMap);
				if (!verifyRight(entityAccessMap, charRingAccessMap, package, neededLevel, charLevel))
				{
					err = new CVerfiyRightRtScenarioError(CVerfiyRightRtScenarioError::InvalidBotLevel, botName, package, neededLevel, charLevel);
					return false;
				}
			}
		}

	}

	return true;

}

uint32 CRingAccess::cypherCharId(uint32 id)
{
	std::vector<bool> v(32);
	std::vector<bool> v2(32);

	static uint32 mask=0x7ce3b52d;
	id ^= mask;
	uint32 i;
	for (i=0; i != 32; ++i)
	{
		v[i] = (id &( 1 << i)) != 0;
	}

	v2[ 0] = v[ 5];	v2[ 1] = v[ 8]; v2[ 2] = v[16]; v2[ 3] = v[23];
	v2[ 4] = v[ 2]; v2[ 5] = v[31]; v2[ 6] = v[27]; v2[ 7] = v[12];
	v2[ 8] = v[ 6]; v2[ 9] = v[24]; v2[10] = v[30]; v2[11] = v[21];
	v2[12] = v[17]; v2[13] = v[14]; v2[14] = v[20]; v2[15] = v[18];

	v2[16] = v[ 4]; v2[17] = v[29]; v2[18] = v[22]; v2[19] = v[10];
	v2[20] = v[ 0]; v2[21] = v[26]; v2[22] = v[ 9]; v2[23] = v[28];
	v2[24] = v[15]; v2[25] = v[ 3]; v2[26] = v[11]; v2[27] = v[19];
	v2[28] = v[ 7]; v2[29] = v[ 1]; v2[30] = v[25]; v2[31] = v[13];

	uint32 id2=0;
	for (i=0; i != 32; ++i)
	{
		id2 |= (v2[i] << i);
	}
	return id2;
}

uint32 CRingAccess::uncypherCharId(uint32 id)
{
	std::vector<bool> v(32);
	std::vector<bool> v2(32);
	static uint32 mask=0x7ce3b52d;

	uint32 i;
	for (i=0; i != 32; ++i)
	{
		v2[i] = (id &( 1 << i)) != 0;
	}

	v[ 5] = v2[ 0]; v[ 8] = v2[ 1]; v[16] = v2[ 2]; v[23] = v2[ 3];
	v[ 2] = v2[ 4]; v[31] = v2[ 5]; v[27] = v2[ 6]; v[12] = v2[ 7];
	v[ 6] = v2[ 8]; v[24] = v2[ 9]; v[30] = v2[10]; v[21] = v2[11];
	v[17] = v2[12]; v[14] = v2[13]; v[20] = v2[14]; v[18] = v2[15];

	v[ 4] = v2[16]; v[29] = v2[17]; v[22] = v2[18]; v[10] = v2[19];
	v[ 0] = v2[20]; v[26] = v2[21]; v[ 9] = v2[22]; v[28] = v2[23];
	v[15] = v2[24]; v[ 3] = v2[25]; v[11] = v2[26]; v[19] = v2[27];
	v[ 7] = v2[28]; v[ 1] = v2[29]; v[25] = v2[30]; v[13] = v2[31];

	uint32 id2=0;
	for (i=0; i != 32; ++i)
	{
		id2 |= (v[i] << i);
	}

	id2 ^= mask;
	return id2;
}


