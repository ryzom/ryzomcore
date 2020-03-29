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

//-------------------------------------------------------------------------
// some stuff for manageing the output of AI_MANAGER files

/*
===========================================================================

  -------------------------------------------------------------------------
  ** Common methods of the following classes:

  display(uint indent=0)
  display the class and all of its child classes recursively
  the indent parameter gives the display indentation depth
  
  read()
  read the class and its child classes from a file recursively

  write()
  write the class and its child classes to an output file recursively

  test()
  runs the class' test routine

  -------------------------------------------------------------------------
  ** The list of classes includes:

  CAiMgrSpawn		- A spawn point or spawn zone within an ai manager
  CAiMgrPopulation	- A population of creatures	within an ai manager
  CAiMgrLocation	- A location with type (eg grass, denn, etc) within an ai manager 
  CAiMgrInstance	- An individual AI manager
  CAiMgrFile		- The collection of AI managers loaded from or to be saved to a file

  CAiMgr			- The singleton that contains the collection of CAiMgrFiles

===========================================================================
*/




//-------------------------------------------------------------------------
// includes

#include "nel/misc/types_nl.h"
#include "nel/misc/sheet_id.h"

#include <string>
#include <vector>


//-------------------------------------------------------------------------
// class to manage a spawn point or patat for an ai manager

class CAiMgrSpawn
{
public:
	void setPlace(std::string place)		{_place=place;}
public:
	void display(uint indent=0,NLMISC::CLog *log = NLMISC::InfoLog);
	void read();
	void write(FILE *outf,uint indent);
	void test();
private:
	std::string _place;	// names of patat or point (or x y z)
};


//-------------------------------------------------------------------------
// class to manage a bot population for an ai manager

class CAiMgrPopulation
{
public:
	void setName(std::string name)					{_name=name;}
	void setType(NLMISC::CSheetId creatureType)		{_creatureType=creatureType;}
	void setQuantity(int quantity)					{_quantity=quantity;}
	void setOrientation(float orientation)			{_orientation=orientation;}
	void setSpawnRate(int minTimeMS, int maxTimeMS) {_minTime=minTimeMS; _maxTime=maxTimeMS;}
public:
	void display(uint indent=0,NLMISC::CLog *log = NLMISC::InfoLog);
	void read();
	void write(FILE *outf,uint indent);
	void test();
private:
	std::string _name;
	NLMISC::CSheetId _creatureType;
	int _quantity;
	float _orientation;
	int _minTime;
	int _maxTime;
};


//-------------------------------------------------------------------------
// class to manage a location patat for an ai manager

class CAiMgrLocation
{
public:
	void setName(std::string name)			{_name=name;}
	void setType(std::string type)			{_type=type;}
	void setBoundary(std::string boundary)	{_boundary=boundary;}
public:
	void display(uint indent=0,NLMISC::CLog *log = NLMISC::InfoLog);
	void read();
	void write(FILE *outf,uint indent);
	void test();
private:
	std::string _name;
	std::string _type;
	std::string _boundary;
};


//-------------------------------------------------------------------------
// class to manage an instance of an ai manager

class CAiMgrInstance
{
public:
	void setName(const std::string &name)				{_name=name;}
	void setBoundary(const std::string &boundary)		{_boundary=boundary;}
	void setCreatureLimit(int creatureLimit)			{_creatureLimit=creatureLimit;}

	void addSpawn(CAiMgrSpawn *spawn)					{_spawn.push_back(spawn);}
	void addPopulation(CAiMgrPopulation *population)	{_population.push_back(population);}
	void addLocation(CAiMgrLocation *location)			{_location.push_back(location);}

	void addChild(CAiMgrInstance *mgr)					{_children.push_back(mgr);}
public:
	void display(uint indent=0,NLMISC::CLog *log = NLMISC::InfoLog);
	void read();
	void write(FILE *outf,uint indent);
	void test();
private:
	std::string _name;
	std::string _boundary;
	int			_creatureLimit;

	std::vector <CAiMgrSpawn *>			_spawn;
	std::vector <CAiMgrPopulation *>	_population;
	std::vector <CAiMgrLocation *>		_location;

	std::vector <CAiMgrInstance *>		_children;
};


//-------------------------------------------------------------------------
// class to manage the set of ai managers stored in a given file

class CAiMgrFile
{
public:
	void setName(std::string filename)		{_filename=filename;}
	void setPrim(std::string primFilename)	{_primFilename=primFilename;}
	void addChild(CAiMgrInstance *mgr)		{_children.push_back(mgr);}
public:
	const std::string &getName()			{return _filename;}
	const std::string &getPrim()			{return _primFilename;}
	const CAiMgrInstance *getChild(uint i)	{return _children[i];}
	uint getChildCount()					{return _children.size();}
public:
	void display(uint indent=0,NLMISC::CLog *log = NLMISC::InfoLog);
	void read();
	void write();
	void test();
private:
	std::string _filename;
	std::string _primFilename;

	std::vector <CAiMgrInstance *>	_children;
};


//-------------------------------------------------------------------------
// singleton manager of CAiMgrFile instances

class CAiMgr
{
public:
	static void addFile(CAiMgrFile *file)	{_files.push_back(file);}
public:
	static void display(uint indent=0,NLMISC::CLog *log = NLMISC::InfoLog);
	static void read(std::string path,bool recursive=true);
	static void write();
	static void test();
private:
	static std::vector <CAiMgrFile *>	_files;
};


//-------------------------------------------------------------------------  
