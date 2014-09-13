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
#include "utils.h"

#include "nel/misc/file.h"
#include "nel/misc/command.h"
//#include "game_share/utils.h"
#include "scenario_entry_points.h"
#include "nel/misc/o_xml.h"
#include "nel/misc/i_xml.h"


//-----------------------------------------------------------------------------
// namespaces
//-----------------------------------------------------------------------------

using namespace NLMISC;

//-----------------------------------------------------------------------------
// R2 namespace
//-----------------------------------------------------------------------------

namespace R2
{
CScenarioEntryPoints	*CScenarioEntryPoints::_Instance = NULL;

CScenarioEntryPoints::CScenarioEntryPoints()
{
	_IsLoaded= false;
	_CompleteIslandsLoaded = false;
	_LastTestedCoords.set(FLT_MAX, FLT_MAX);
	_LastFoundIsland = NULL;
	init();
}

//-----------------------------------------------------------------------------

void CScenarioEntryPoints::init()
{
	_CompleteIslandsFilename = "r2_islands.xml";
	_EntryPointsFilename = "ring_map_entry_ponts.txt";
}
//-----------------------------------------------------------------------------

CScenarioEntryPoints &CScenarioEntryPoints::getInstance()
{
	// allocate our singleton if need be
	if(!_Instance)
	{
		_Instance = new CScenarioEntryPoints();
	}

	// return ref to our singleton
	return *_Instance;
}

//-----------------------------------------------------------------------------
void CScenarioEntryPoints::releaseInstance()
{
	if( _Instance )
		delete _Instance;
	_Instance = NULL;
}

//-----------------------------------------------------------------------------

const CScenarioEntryPoints::TEntryPoints& CScenarioEntryPoints::getEntryPoints()
{
	// if need be load the entry points vector from disk file
	if(!_IsLoaded)
	{
		loadFromFile();
	}

	// return the entry points vector
	return _EntryPoints;
}

//-----------------------------------------------------------------------------
// TEMP
uint32 CScenarioEntryPoints::getIslandId(const CSString& island)
{
	// if need be load the entry points vector from disk file
	loadCompleteIslands();

	uint32 count=0;

	for (uint32 i=0;i<_CompleteIslands.size();++i)
	{
		// if this entry point corresponds to the one we're looking for then stop here
		if (island==_CompleteIslands[i].Island)
			return count;

		// increment the entry point id each time we get here (ie for each entry point in package definition)
		++count;
	}

	return count;
}

//-----------------------------------------------------------------------------
CScenarioEntryPoints::CCompleteIsland * CScenarioEntryPoints::getIslandFromId(const NLMISC::CSString& islandId)
{
	// if need be load the entry points vector from disk file
	loadCompleteIslands();

	for (uint32 i=0;i<_CompleteIslands.size();++i)
	{
		// if this entry point corresponds to the one we're looking for then stop here
		if (islandId==_CompleteIslands[i].Island)
			return &_CompleteIslands[i];
	}

	return NULL;
}

//-----------------------------------------------------------------------------
CScenarioEntryPoints::CShortEntryPoint * CScenarioEntryPoints::getEntryPointFromIds(const NLMISC::CSString& islandId,
									   const NLMISC::CSString& entryPointId)
{
	// if need be load the entry points vector from disk file
	loadCompleteIslands();

	for (uint32 i=0;i<_CompleteIslands.size();++i)
	{
		// if this entry point corresponds to the one we're looking for then stop here
		if (islandId==_CompleteIslands[i].Island)
		{
			for (uint32 e=0;e<_CompleteIslands[i].EntryPoints.size();++e)
			{
				if (entryPointId==_CompleteIslands[i].EntryPoints[e].Location)
				{
					return &_CompleteIslands[i].EntryPoints[e];
				}
			}
		}
	}

	return NULL;
}

//-----------------------------------------------------------------------------

void CScenarioEntryPoints::loadFromFile()
{
	// clear out the entry point vector before we begin
	_EntryPoints.clear();

	// setup the file name
	std::string pathFileName = CPath::lookup(_EntryPointsFilename.c_str());

	// open our input file
	CIFile inf;
	inf.open(pathFileName);

	// read the file contents into a string 's'
	CSString s;
	s.resize(inf.getFileSize());
	inf.serialBuffer((uint8*)&s[0],(uint)s.size());

	// close the file
	inf.close();

	// split the string into lines
	CVectorSString lines;
	s.splitLines(lines);

	// run through the lines looking for stuff that's valid
	for (uint32 i=0;i<lines.size();++i)
	{
		// strip away any comment in the line and get rid of leading and trailing spaces
		CSString line= lines[i].splitTo("//").strip();
		if (line.empty())
			continue;

		// split the line into words
		CVectorSString words;
		while (!line.empty())
		{
			CSString s= line.strtok(" \t");
			words.push_back(s);
		}

		// make sure the syntax is correct
		BOMB_IF((words.size()!=6)
				|| NLMISC::toString("%u",words[3].atoui())!= words[3]
				|| NLMISC::toString("%d",words[4].atosi())!= words[4],
			NLMISC::toString("%s:%d: Invalid syntax: %s",_EntryPointsFilename.c_str(),i+1,lines[i].splitTo("//").c_str()), continue);

		// display an info message
		//nlinfo("Adding Entry point: %s",lines[i].splitTo("//").c_str());

		// split the line into constituent parts and feed it to the entry points vector
		_EntryPoints.push_back(CEntryPoint(words[0],words[1],words[2],words[3].atoui(),words[4].atosi()));
	}

	// flag the entry point set as 'loaded'
	_IsLoaded= true;
}

//-----------------------------------------------------------------------------
CScenarioEntryPoints::CCompleteIsland *CScenarioEntryPoints::getCompleteIslandFromCoords(const NLMISC::CVector2f &pos)
{
	loadCompleteIslands();
	if (fabs((double) pos.x - _LastTestedCoords.x) > 20.f ||
		fabs((double) pos.y - _LastTestedCoords.y) > 20.f)
	{
		_LastTestedCoords = pos;
		for(uint k = 0; k < _CompleteIslands.size(); ++k)
		{
			if (pos.x >= (float) _CompleteIslands[k].XMin &&
				pos.x <= (float) _CompleteIslands[k].XMax &&
				pos.y >= (float) _CompleteIslands[k].YMin &&
				pos.y <= (float) _CompleteIslands[k].YMax)
			{
				_LastFoundIsland = &_CompleteIslands[k];
				return _LastFoundIsland;
			}
		}
		_LastFoundIsland = NULL;
		return _LastFoundIsland;
	}
	else
	{
		// coords similar to previous test, reuse result
		return _LastFoundIsland;
	}
}

// FIXME : ugly duplicate from the client src ... put this in a lib
static bool getZonePosFromZoneName(const std::string &name, sint &x, sint &y)
{

	if (name.empty())
	{
		nlwarning ("getPosFromZoneName(): empty name, can't getPosFromZoneName");
		return false;
	}

	static std::string zoneName;
	static std::string xStr, yStr;
	xStr.clear();
	yStr.clear();
	zoneName = NLMISC::CFile::getFilenameWithoutExtension(name);
	uint32 i = 0;
	while (zoneName[i] != '_')
	{
		if (!::isdigit(zoneName[i])) return false;
		yStr += zoneName[i]; ++i;
		if (i == zoneName.size())
			return false;
	}
	++i;
	while (i < zoneName.size())
	{
		if (!::isalpha(zoneName[i])) return false;
		xStr += (char) ::toupper(zoneName[i]); ++i;
	}
	if (xStr.size() != 2) return false;
	x = (xStr[0] - 'A') * 26 + (xStr[1] - 'A');
	fromString(yStr, y);
	y = -y;
	return true;
}


//-----------------------------------------------------------------------------
void CScenarioEntryPoints::loadFromXMLFile()
{
	loadFromFile();

	// clear out the entry point vector before we begin
	_CompleteIslands.clear();

	// File stream
	CIFile file;

	// setup the file name
	std::string pathFileName = CPath::lookup(_CompleteIslandsFilename.c_str());

	// Open the file
	if (!file.open(pathFileName.c_str()))
	{
		nlinfo("Can't open the file for reading : %s", pathFileName.c_str());
	}

	// Create the XML stream
	CIXml input;

	// Init
	if(input.init(file))
	{
		xmlNodePtr islands = input.getRootNode();
		xmlNodePtr islandNode = input.getFirstChildNode(islands, "complete_island");

		while (islandNode != 0)
		{
			CCompleteIsland completeIsland;

			// island name
			const char *island = (const char*) xmlGetProp(islandNode, (xmlChar*) "island");
			if(island == 0)
			{
				nlinfo("no 'island' tag in %s", _CompleteIslandsFilename.c_str());
				continue;
			}
			else
				completeIsland.Island = CSString(island);

			// package
			/*
			const char *package = (const char*) xmlGetProp(islandNode, (xmlChar*) "package");
			if(package == 0)
				nlinfo("no 'package' tag in %s island", island);
			else
				completeIsland.Package = CSString(package);
			*/

			// continent
			const char *continent = (const char*) xmlGetProp(islandNode, (xmlChar*) "continent");
			if(continent == 0)
				nlinfo("no 'continent' tag in %s island", island);
			else
				completeIsland.Continent = CSString(continent);

			// xmin
			const char *xmin = (const char*) xmlGetProp(islandNode, (xmlChar*) "xmin");
			if(xmin == 0)
				nlinfo("no 'xmin' tag in %s island", island);
			else
				fromString(xmin, completeIsland.XMin);

			// ymin
			const char *ymin = (const char*) xmlGetProp(islandNode, (xmlChar*) "ymin");
			if(ymin == 0)
				nlinfo("no 'ymin' tag in %s island", island);
			else
				fromString(ymin, completeIsland.YMin);

			// xmax
			const char *xmax = (const char*) xmlGetProp(islandNode, (xmlChar*) "xmax");
			if(xmax == 0)
				nlinfo("no 'xmax' tag in %s island", island);
			else
				fromString(xmax, completeIsland.XMax);

			// ymax
			const char *ymax = (const char*) xmlGetProp(islandNode, (xmlChar*) "ymax");
			if(ymax == 0)
				nlinfo("no 'ymax' tag in %s island", island);
			else
				fromString(ymax, completeIsland.YMax);

			//entry points and package
			TShortEntryPoints entryPoints;
			std::string package = std::string("");
			for(uint e=0; e<_EntryPoints.size(); e++)
			{
				const CEntryPoint & entryPoint = _EntryPoints[e];
				CShortEntryPoint shortEntryPoint;

				if(entryPoint.Island == island)
				{
					shortEntryPoint.Location = entryPoint.Location;
					shortEntryPoint.X = entryPoint.X;
					shortEntryPoint.Y = entryPoint.Y;
					entryPoints.push_back(shortEntryPoint);

					if(package=="")
						package=entryPoint.Package;
					else if(package!=entryPoint.Package)
						nlinfo("Different packages for island '%s' in file %s", island, _EntryPointsFilename.c_str());
				}
			}
			if(package.empty())
				nlinfo("no 'package' tag in %s island", island);
			else
				completeIsland.Package = CSString(package);


			// zones
			xmlNodePtr zoneNode = input.getFirstChildNode(islandNode, "zone");

			while(zoneNode != 0)
			{
				// island name
				const char *zoneName = (const char*) xmlGetProp(zoneNode, (xmlChar*) "name");
				if(zoneName == 0)
				{
					nlinfo("no 'zone name' tag in %s", _CompleteIslandsFilename.c_str());
				}
				else
					completeIsland.Zones.push_back(std::string(zoneName));

				zoneNode = input.getNextChildNode(zoneNode, "zone");
			}

			// compute zones ids from zone names
			for(std::list<std::string>::iterator it = completeIsland.Zones.begin(); it != completeIsland.Zones.end(); ++it)
			{
				sint x, y;
				if (getZonePosFromZoneName(*it, x, y))
				{
					completeIsland.ZoneIDs.push_back(((uint16) x&255)+((uint16) (-y - 1)<<8));
				}
			}


			if(entryPoints.size()>0)
			{
				completeIsland.EntryPoints = entryPoints;
				_CompleteIslands.push_back(completeIsland);
			}

			islandNode = input.getNextChildNode(islandNode, "complete_island");
		}
	}

	// Close the file
	file.close ();

	_CompleteIslandsLoaded = true;
}

//-----------------------------------------------------------------------------
void CScenarioEntryPoints::loadCompleteIslands()
{
	if(!_CompleteIslandsLoaded)
	{
		loadFromXMLFile();
	}
}

//-----------------------------------------------------------------------------
const CScenarioEntryPoints::TCompleteIslands &  CScenarioEntryPoints::getCompleteIslands()
{
	loadCompleteIslands();

	// return the islands vector
	return _CompleteIslands;
}

//-----------------------------------------------------------------------------
void CScenarioEntryPoints::saveXMLFile(const TCompleteIslands & completeIslands, const std::string & fileName)
{

	// File stream
	COFile file;

	// setup the file name
	std::string pathFilename = CPath::lookup(fileName.c_str());

	// Open the file
	if (!file.open(pathFilename.c_str()))
	{
		nlinfo("Can't open the file for writing : %s", fileName.c_str());
		return;
	}

	// Create the XML stream
	COXml output;

	// Init
	if(output.init(&file, "1.0"))
	{
		xmlDocPtr xmlDoc = output.getDocument();

		// Create the first node
		xmlNodePtr root = xmlNewDocNode(xmlDoc, NULL, (const xmlChar*)"islands", NULL);
		xmlDocSetRootElement(xmlDoc, root);

		std::map< std::string, xmlNodePtr > islandNodes;
		for (uint32 i=0;i<completeIslands.size();++i)
		{
			char s[64];
			// island already exists?
			if(islandNodes.find(completeIslands[i].Island) == islandNodes.end())
			{
				xmlNodePtr islandNode = xmlNewChild(root, NULL, (const xmlChar*)"complete_island", NULL);
				xmlSetProp(islandNode, (const xmlChar*)"island", (const xmlChar*)completeIslands[i].Island.c_str());
				//xmlSetProp(islandNode, (const xmlChar*)"package", (const xmlChar*)completeIslands[i].Package.c_str());
				xmlSetProp(islandNode, (const xmlChar*)"continent", (const xmlChar*)completeIslands[i].Continent.c_str());

				smprintf(s, 64, "%i", completeIslands[i].XMin);
				xmlSetProp(islandNode, (const xmlChar*)"xmin", (const xmlChar*)s);

				smprintf(s, 64, "%i", completeIslands[i].YMin);
				xmlSetProp(islandNode, (const xmlChar*)"ymin", (const xmlChar*)s);

				smprintf(s, 64, "%i", completeIslands[i].XMax);
				xmlSetProp(islandNode, (const xmlChar*)"xmax", (const xmlChar*)s);

				smprintf(s, 64, "%i", completeIslands[i].YMax);
				xmlSetProp(islandNode, (const xmlChar*)"ymax", (const xmlChar*)s);

				std::list<std::string>::const_iterator itZone;
				for(itZone=completeIslands[i].Zones.begin(); itZone!=completeIslands[i].Zones.end(); itZone++)
				{
					xmlNodePtr zoneNode = xmlNewChild(islandNode, NULL, (const xmlChar*)"zone", NULL);
					xmlSetProp(zoneNode, (const xmlChar*)"name", (const xmlChar*)(*itZone).c_str());
				}

				islandNodes[completeIslands[i].Island] = islandNode;
			}
		}

		// Flush the stream, write all the output file
		output.flush();
	}

	// Close the file
	file.close();
}

/*
//-----------------------------------------------------------------------------

void CScenarioEntryPoints::getIslands(const CSString& packageDefinition, CVectorSString& islands)
{
	// if need be load the entry points vector from disk file
	if (!_IsLoaded)
	{
		loadFromFile();
	}

	islands.clear();
	std::set<CSString> found;
	for (uint32 i=0;i<_EntryPoints.size();++i)
	{
		CSString& island= _EntryPoints[i].Island;

		// skip entry points from inaccessible packages
		if (!packageDefinition.contains(_EntryPoints[i].Package.c_str()))
			continue;

		// skip dumplicate names
		if (found.find(island)!=found.end())
			continue;

		// add the island to the output vector and set of found islands
		islands.push_back(island);
		found.insert(island);
	}
}

//-----------------------------------------------------------------------------

void CScenarioEntryPoints::getEntryPoints(const CSString& packageDefinition, const CSString& island, CVectorSString& entryPoints)
{
	// if need be load the entry points vector from disk file
	if (!_IsLoaded)
	{
		loadFromFile();
	}

	entryPoints.clear();
	for (uint32 i=0;i<_EntryPoints.size();++i)
	{
		CSString& entryPoint= _EntryPoints[i].Location;

		// skip entry points from inaccessible packages
		if (!packageDefinition.contains(_EntryPoints[i].Package.c_str()))
			continue;

		// skip entry points from the wrong island
		if (island!=_EntryPoints[i].Island)
			continue;

		// add the island to the output vector and set of found islands
		entryPoints.push_back(entryPoint);
	}
}

//-----------------------------------------------------------------------------

uint32 CScenarioEntryPoints::getEntryPointId(const CSString& packageDefinition, const CSString& island, const CSString& entryPoint)
{
	// if need be load the entry points vector from disk file
	if (!_IsLoaded)
	{
		loadFromFile();
	}

	uint32 count=0;

	for (uint32 i=0;i<_EntryPoints.size();++i)
	{
		// skip entry points from inaccessible packages
		if (!packageDefinition.contains(_EntryPoints[i].Package.c_str()))
			continue;

		// if this entry point corresponds to the one we're looking for then stop here
		if (island==_EntryPoints[i].Island && entryPoint==_EntryPoints[i].Location)
			return count;

		// increment the entry point id each time we get here (ie for each entry point in package definition)
		++count;
	}

	// bomb out - we didn't find a match - on live shards this case is to be ignored as it only means we have a data
	// error and not necessarily a code bug... in debug we might as well STOP to flag that there's a problem
	STOP("ERROR: entry point '"+island+":"+entryPoint+"' not found in package description: "+packageDefinition);
	return count;
}

//-----------------------------------------------------------------------------

void CScenarioEntryPoints::getEntryPointCoordsFromId(const CSString& packageDefinition, uint32 id, sint32& x, sint32& y)
{
	// if need be load the entry points vector from disk file
	if (!_IsLoaded)
	{
		loadFromFile();
	}

	uint32 count=0;

	for (uint32 i=0;i<_EntryPoints.size();++i)
	{
		// skip entry points from inaccessible packages
		if (!packageDefinition.contains(_EntryPoints[i].Package.c_str()))
			continue;

		// if this entry point corresponds to the one we're looking for then stop here
		if (count==id)
		{
			x= _EntryPoints[i].X;
			y= _EntryPoints[i].Y;
			return;
		}

		// increment the entry point id each time we get here (ie for each entry point in package definition)
		++count;
	}

	// bomb out - we didn't find a match - on live shards this case is to be ignored as it only means we have a data
	// error and not necessarily a code bug... in debug we might as well STOP to flag that there's a problem
	STOP("ERROR: entry point '"+NLMISC::toString(id)+"' not found in package description: "+packageDefinition);

	x= y= 0;

	return;
}
*/

} // namespace R2

/*
NLMISC_COMMAND(displayScenarioEntryPoints,"display the list of scenario entry points (either the complete list or the list for a given input string","[<package definition string>]")
{
	// check that we have a valid number of arguments
	if (args.size()>1)
		return false;

	// setup the package definition string
	CSString packageDefinition;
	if (!args.empty())
	{
		packageDefinition= args[0];
	}

	// display a fancy title line
	nlinfo("Displaying scenario entry points correspoding to package definition: '%s'",packageDefinition.c_str());

	// get the vector of islands that we are allowed access to
	CVectorSString islands;
	R2::CScenarioEntryPoints::getInstance().getIslands(packageDefinition,islands);

	// run through the islands displaying them with their lists of entry points
	for (uint32 i=0;i<islands.size();++i)
	{
		nlinfo("- Island: %s", islands[i].c_str());

		// get the set of valid entry points for this island
		CVectorSString entryPoints;
		R2::CScenarioEntryPoints::getInstance().getEntryPoints(packageDefinition,islands[i],entryPoints);

		// for each entry point...
		for (uint32 j=0;j<entryPoints.size();++j)
		{
			// lookup the entry point id from the package definition, island name and entry point name
			uint32 entryPointId= R2::CScenarioEntryPoints::getInstance().getEntryPointId(packageDefinition,islands[i],entryPoints[j]);

			// get the coordinates for the given entry point
			sint32 x, y;
			R2::CScenarioEntryPoints::getInstance().getEntryPointCoordsFromId(packageDefinition,entryPointId,x,y);

			// display a debug message
			nlinfo("    - Entry point %d: %s (%d,%d)", entryPointId, entryPoints[j].c_str(), x, y);
		}
	}

	return true;
}
*/
