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

#ifndef R2_SCENARIO_ENTRY_POINTS_H
#define R2_SCENARIO_ENTRY_POINTS_H

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "nel/misc/sstring.h"
#include "nel/misc/vector_2f.h"

//-----------------------------------------------------------------------------
// R2 namespace
//-----------------------------------------------------------------------------

namespace R2
{
	//-----------------------------------------------------------------------------
	// class CScenarioEntryPoints
	//-----------------------------------------------------------------------------

	class CScenarioEntryPoints
	{

	friend class CScreenshotIslands;

	public:
		//-------------------------------------------------------------------------
		// public data types

		struct CEntryPoint
		{
			NLMISC::CSString Package;
			NLMISC::CSString Island;
			NLMISC::CSString Location;
			sint32 X;
			sint32 Y;


			CEntryPoint()
			{
				X= Y =0;
			}

			CEntryPoint(NLMISC::CSString package, NLMISC::CSString island, NLMISC::CSString location,
				sint32 x, sint32 y)
			{
				Package=	package;
				Island=		island;
				Location=	location;
				X=			x;
				Y=			y;
			}
		};

		typedef std::vector<CEntryPoint> TEntryPoints;


		struct CShortEntryPoint
		{
			NLMISC::CSString Location;
			sint32 X;
			sint32 Y;
		};

		typedef std::vector<CShortEntryPoint> TShortEntryPoints;


		class CCompleteIsland
		{
		public:
			NLMISC::CSString Package;
			NLMISC::CSString Island;
			NLMISC::CSString Continent;
			sint32 XMin;
			sint32 YMin;
			sint32 XMax;
			sint32 YMax;
			TShortEntryPoints EntryPoints;
			std::list<std::string> Zones;
			std::vector<uint16>	ZoneIDs; // IDs of the zones of this island
		public:
			CCompleteIsland()
			{
				XMin =YMin =XMax =YMax =0;
			}
			bool isIn(const NLMISC::CVector2f &pos) const
			{
				return (sint32) pos.x >= XMin &&
					   (sint32) pos.x <  XMax &&
					   (sint32) pos.y >= YMin &&
					   (sint32) pos.y <  YMax;
			}
		};

		typedef std::vector<CCompleteIsland> TCompleteIslands;

	public:
		//-------------------------------------------------------------------------
		// public interface

		// get hold of the singleton instance
		static CScenarioEntryPoints& getInstance();

		// release memory
		static void releaseInstance();

		void loadCompleteIslands();

		// lookup the integer id for a given island
		uint32 getIslandId(const NLMISC::CSString& island); //TEMP

		CCompleteIsland * getIslandFromId(const NLMISC::CSString& islandId);

		CShortEntryPoint * getEntryPointFromIds(const NLMISC::CSString& islandId, const NLMISC::CSString& entryPointId);

		// get entry point from coords in world
		CCompleteIsland *getCompleteIslandFromCoords(const NLMISC::CVector2f &pos);

		// get the vector of complete islands
		const TCompleteIslands&  getCompleteIslands();

	private:
		//-------------------------------------------------------------------------
		// this is a singleton so prevent instantiation
		CScenarioEntryPoints();

		void init();

		// the unique instance of the singleton
		static CScenarioEntryPoints *_Instance;

		void saveXMLFile(const TCompleteIslands & entryPoints, const std::string & fileName);

		void loadFromXMLFile();

		// load the scenario entry points from disk file
		// Note that the file format for this routine is as follows:
		// - empty lines are ignored
		// - any text following a // on a line is ignored
		// - meaningful lines have the following syntax: <package name> <island name> <entry point name> <x> <y> <orientation>
		//   eg: j1 R2UI_Goo_et_bambous R2UI_Main_entry_point 33100 -11200 NW
		void loadFromFile();

		// get the entry points - calls loadFRomFile() if need be
		const TEntryPoints& getEntryPoints();

		// get the vector of island names that correspond to the given package definition string
		//void getIslands(const NLMISC::CSString& packageDefinition, NLMISC::CVectorSString& islands);

		// get the vector of valid entry points for a given island that correspond to the given package definition string
		//void getEntryPoints(const NLMISC::CSString& packageDefinition, const NLMISC::CSString& island, NLMISC::CVectorSString& entryPoints);

		// lookup the integer id for an entry point for a given island and package definition
		//uint32 getEntryPointId(const NLMISC::CSString& packageDefinition, const NLMISC::CSString& island, const NLMISC::CSString& entryPoint);

		// lookup the coordinates of an entry point from its id and package definition
		// sets x,y to 0,0 if the id is invalid
		//void getEntryPointCoordsFromId(const NLMISC::CSString& packageDefinition, uint32 id, sint32& x, sint32& y);

	private:
		//-------------------------------------------------------------------------
		// private data
		bool _IsLoaded;
		bool _CompleteIslandsLoaded;
		TEntryPoints _EntryPoints;
		TCompleteIslands _CompleteIslands;

		NLMISC::CVector2f	_LastTestedCoords;
		CCompleteIsland		*_LastFoundIsland;

		std::string _CompleteIslandsFilename;
		std::string _EntryPointsFilename;
	};

} // namespace R2

//-----------------------------------------------------------------------------

#endif
