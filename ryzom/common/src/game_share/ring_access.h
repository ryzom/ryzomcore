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

#ifndef R2_RING_ACCESS_H
#define R2_RING_ACCESS_H

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "nel/misc/singleton.h"
#include "nel/misc/sheet_id.h"

//-----------------------------------------------------------------------------
// R2 namespace
//-----------------------------------------------------------------------------

namespace R2
{
	class CObject;
	//-----------------------------------------------------------------------------
	// class CRingAccess
	//-----------------------------------------------------------------------------

	class CVerfiyRightRtScenarioError
	{
	public:
		enum TType {None, InvalidData, InvalidIslandLevel, InvalidBotLevel};
		TType Type;
		std::string Name; //bot or island name
		std::string Package; //package
		int Level;
		int CharLevel;

		CVerfiyRightRtScenarioError(TType varType = None, std::string name = "", std::string package = "",
		int level = 0, int charLevel = 0):
		Type(varType), Name(name), Package(package), Level(level), CharLevel(charLevel) {}

	public:
		std::string toString() const;


	};

	class CRingAccess : public NLMISC::CSingleton<R2::CRingAccess>
	{
	public:


		// load (load data from xml)
		// If not call explicityl this function is automaticly called by lazy initialisation
		void init();

		// lookup the integer id for a given island
		std::string getSheetIdAccessInfo(const NLMISC::CSheetId& sheetClientId, const NLMISC::CSheetId& sheetServerId) const;
		std::string getSheetAccessInfo(const std::string& sheetClient, const std::string& sheetServer="") const;
		std::string getIslandAccessInfo(const std::string& islandeName) const;
		void getRingAccessAsMap(const std::string& ringAccess, std::map<std::string, int> & ringAccessAsMap) const;
		// Upgrade a ring access by another eg "d1:f3:l1" +  "d3:f1:j1" => "d3:f2:l1:j1"
		std::string upgradeRingAccess(const std::string& defaultValue, const std::string& bonusValue) const;
		bool verifyRight(const std::string& askedAcces, const std::string& allowedAccess) const;
		bool verifyRight(const std::map<std::string, int>& askedAccesMap, const std::map<std::string, int>& allowedAccessMap, std::string& package, int& neededLevel, int& charLevel) const;
		//return null if ok otherwise return error
		bool verifyRtScenario(CObject* rtScenario, const std::string& charRingAccess, CVerfiyRightRtScenarioError* &err) const;
		static uint32 cypherCharId(uint32 id);
		static uint32 uncypherCharId(uint32 id);
		bool isPlotItemSheetId(const NLMISC::CSheetId& sheetId ) const;
		bool isSheetClient(const NLMISC::CSheetId& sheet) const;
	private: // private types
		typedef std::map< std::pair<NLMISC::CSheetId, NLMISC::CSheetId>, std::string> TSheetIdToAccess;	// Map  sheetClientId, sheetServerId => RingAccess(as string)

	public: // private functions

		// this is a singleton so prevent instantiation
		CRingAccess();

	private: // private members
		TSheetIdToAccess _SheetIdToAccess; //SheetId to access
		std::set<NLMISC::CSheetId> _CustomNpcSheetId;
		std::set<NLMISC::CSheetId> _R2PlotItemSheetId;
		bool _Initialised;


	};

} // namespace R2

//-----------------------------------------------------------------------------

#endif
