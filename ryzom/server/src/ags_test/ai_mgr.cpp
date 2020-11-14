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

#include "ai_mgr.h"

//--------------------------------------------------------------------------
// singleton data declaration

std::vector <CAiMgrFile *>	CAiMgr::_files;


//--------------------------------------------------------------------------
// Utility functions

// generate indentation strings
static std::string tab(uint indent)
{
	std::string s;
	s.resize(indent,'\t');
	return s;
}

// opening a STRUCT clause in output XML file
static void writeOpenStruct(FILE *outf, uint indent, std::string name=std::string())
{
	if (name.empty())
		fprintf(outf,"%s<STRUCT>\n",tab(1).c_str());
	else
		fprintf(outf,"%s<STRUCT Name=\"%s\">\n",tab(1).c_str(),name.c_str());
}

// closing a STRUCT clause in output XML file
static void writeCloseStruct(FILE *outf, uint indent)
{
	fprintf(outf,"%s</STRUCT>\n",tab(1).c_str());
}

// opening an ARRAY clause in output XML file
static void writeOpenArray(FILE *outf, uint indent, std::string name=std::string())
{
	if (name.empty())
		fprintf(outf,"%s<ARRAY>\n",tab(1).c_str());
	else
		fprintf(outf,"%s<ARRAY Name=\"%s\">\n",tab(1).c_str(),name.c_str());
}

// closing an ARRAY clause in output XML file
static void writeCloseArray(FILE *outf, uint indent)
{
	fprintf(outf,"%s</ARRAY>\n",tab(1).c_str());
}

// writing an ATOM clause in output XML file
static void writeAtom(FILE *outf, uint indent, std::string name, std::string value)
{
	if (name.empty())
		fprintf(outf,"%s<ATOM Value=\"%s\"/>\n",tab(indent).c_str(),value.c_str());
	else
		fprintf(outf,"%s<ATOM Name=\"%s\" Value=\"%s\"/>\n",tab(indent).c_str(),name.c_str(),value.c_str());
}

static void writeAtom(FILE *outf, uint indent, std::string name, int value)
{
	if (name.empty())
		fprintf(outf,"%s<ATOMValue=\"%d\"/>\n",tab(indent).c_str(),value);
	else
		fprintf(outf,"%s<ATOM Name=\"%s\" Value=\"%d\"/>\n",tab(indent).c_str(),name.c_str(),value);
}

static void writeAtom(FILE *outf, uint indent, std::string name, double value)
{
	if (name.empty())
		fprintf(outf,"%s<ATOM Value=\"%f\"/>\n",tab(indent).c_str(),value);
	else
		fprintf(outf,"%s<ATOM Name=\"%s\" Value=\"%f\"/>\n",tab(indent).c_str(),name.c_str(),value);
}


//--------------------------------------------------------------------------
// Display
// display contents of hierarchy

void CAiMgrSpawn::display(uint indent, NLMISC::CLog *log)
{
	log->displayNL("%sSPAWN:",tab(indent).c_str());
	log->displayNL("%s%s",tab(indent+1).c_str(),_place.c_str());
}

void CAiMgrPopulation::display(uint indent, NLMISC::CLog *log)
{
	log->displayNL("%sPOPULATION: %s",tab(indent).c_str(),_name.c_str());

	log->displayNL("%stype:  %s",tab(indent+1).c_str(),_creatureType.toString().c_str());
	log->displayNL("%scount: %i",tab(indent+1).c_str(),_quantity);
	log->displayNL("%sface:  pi * %4.2f",tab(indent+1).c_str(),_orientation/3.14159265359);
	log->displayNL("%sspawn: %i..%i ms",tab(indent+1).c_str(),_minTime,_maxTime);
}

void CAiMgrLocation::display(uint indent, NLMISC::CLog *log)
{
	log->displayNL("%sLOCATION: %s",tab(indent).c_str(),_name.c_str());

	log->displayNL("%stype:  %s",tab(indent+1).c_str(),_type.c_str());
	log->displayNL("%spatat: %s",tab(indent+1).c_str(),_boundary.c_str());
}

void CAiMgrInstance::display(uint indent, NLMISC::CLog *log)
{
	log->displayNL("%sMANAGER: %s",tab(indent).c_str(),_name.c_str());

	log->displayNL("%spatat: %s",tab(indent+1).c_str(),_boundary.c_str());
	log->displayNL("%slimit: %i",tab(indent+1).c_str(),_creatureLimit);

	uint i;
	for (i=0;i<_spawn.size();i++)		_spawn[i]->display(indent+1);
	for (i=0;i<_population.size();i++)	_population[i]->display(indent+1);
	for (i=0;i<_location.size();i++)	_location[i]->display(indent+1);

	for (i=0;i<_children.size();i++)	_children[i]->display(indent+1);
}

void CAiMgrFile::display(uint indent, NLMISC::CLog *log)
{
	log->displayNL("%sFILE: %s",tab(indent).c_str(),_filename.c_str());

	log->displayNL("%sprim file: %s",tab(indent+1).c_str(),_primFilename.c_str());

	uint i;
	for (i=0;i<_children.size();i++)	_children[i]->display(indent+1);
}

void CAiMgr::display(uint indent, NLMISC::CLog *log)
{
	log->displayNL("%sDISPLAYING PARSED AI_MANAGER FILE CONTENTS",tab(indent).c_str());

	uint i;
	for (i=0;i<_files.size();i++)		_files[i]->display(indent+1);
}


//--------------------------------------------------------------------------
// Read
// read .AI_MGR 'george' files

void CAiMgrSpawn::read()
{
}

void CAiMgrPopulation::read()
{
}

void CAiMgrLocation::read()
{
}

void CAiMgrInstance::read()
{
}

void CAiMgrFile::read()
{
	// read a .AI_MGR file and parse the contents
}

void CAiMgr::read(std::string path,bool recursive)
{
	// recurse through sub directories looking for ai manager files to read
}


//--------------------------------------------------------------------------
// Write
// write .AI_MGR 'george' files

void CAiMgrSpawn::write(FILE *outf,uint indent)
{
	writeAtom(outf,indent,"",_place.c_str());
}

void CAiMgrPopulation::write(FILE *outf,uint indent)
{
	writeOpenStruct(outf,indent,_name);

	// writing the static fields
	writeAtom(outf,indent+1,"Name",_name);
	writeAtom(outf,indent+1,"CreatureType",_creatureType.toString());
	writeAtom(outf,indent+1,"Population",_quantity);
	writeAtom(outf,indent+1,"MinSpawnTime",_minTime);
	writeAtom(outf,indent+1,"MaxSpawnTime",_maxTime);
	writeAtom(outf,indent+1,"Orientation",_orientation);

	writeCloseStruct(outf,indent);
}

void CAiMgrLocation::write(FILE *outf,uint indent)
{
	writeOpenStruct(outf,indent,_name);

	// writing the static fields
	writeAtom(outf,indent+1,"Name",_name);
	writeAtom(outf,indent+1,"LocationType",_type);
	writeAtom(outf,indent+1,"Boundary",_boundary);

	writeCloseStruct(outf,indent);
}

void CAiMgrInstance::write(FILE *outf,uint indent)
{
	uint i;

	writeOpenStruct(outf,indent,_name);

	// writing the static fields
	writeAtom(outf,indent+1,"Name",_name);
	writeAtom(outf,indent+1,"Boundary",_boundary);
	writeAtom(outf,indent+1,"CreatureLimit",_creatureLimit);

	// writing spawn points
	if (!_spawn.empty())
	{
		writeOpenArray(outf,indent+1,"Spawn");
		for (i=0;i<_spawn.size();i++)		_spawn[i]->write(outf,indent+2);
		writeCloseArray(outf,indent+1);
	}

	// writing populations
	if (!_population.empty())
	{
		writeOpenArray(outf,indent+1,"Population");
		for (i=0;i<_population.size();i++)	_population[i]->write(outf,indent+2);
		writeCloseArray(outf,indent+1);
	}

	// writing locations
	if (!_location.empty())
	{
		writeOpenArray(outf,indent+1,"Location");
		for (i=0;i<_location.size();i++)	_location[i]->write(outf,indent+2);
		writeCloseArray(outf,indent+1);
	}

	// writing children
	if (!_children.empty())
	{
		writeOpenArray(outf,indent+1,"Children");
		for (i=0;i<_children.size();i++)	_children[i]->write(outf,indent+2);
		writeCloseArray(outf,indent+1);
	}

	writeCloseStruct(outf,indent);
}

void CAiMgrFile::write()
{
	// opening the file
	FILE *outf=fopen(_filename.c_str(),"wt");
	if (outf==0)
	{
		nlwarning("Failed to open file for writing: %s",_filename.c_str());
		return;
	}
	nlinfo("WRITING: %s",_filename.c_str());

	// writing the file header
	fprintf(outf,"<?xml version=\"1.0\"?>\n");
	fprintf(outf,"<FORM Version=\"0.1\" State=\"modified\">\n");
	writeOpenStruct(outf,1);

	// writing the static fields
	writeAtom(outf,2,"Name",_filename);
	writeAtom(outf,2,"Prim",_primFilename);

	// iterating through the managers to be written to the file
	uint i;
	writeOpenArray(outf,2,"Child");
	for (i=0;i<_children.size();i++)
		_children[i]->write(outf,3);
	writeCloseArray(outf,2);

	// writing the file footer
	writeCloseStruct(outf,1);
	fprintf(outf,"</FORM>\n");

	// closing the file
	fclose(outf);
}

void CAiMgr::write()
{
	uint i;
	for (i=0;i<_files.size();i++)
		_files[i]->write();
}


//--------------------------------------------------------------------------
// Test
// test code to test the interfaces of the different classes

void CAiMgrSpawn::test()
{
}

void CAiMgrPopulation::test()
{
}

void CAiMgrLocation::test()
{
}

void CAiMgrInstance::test()
{
}

void CAiMgrFile::test()
{
}

void CAiMgr::test()
{
	CAiMgrSpawn			*aiMgrSpawn0 =		new CAiMgrSpawn;
	CAiMgrSpawn			*aiMgrSpawn1 =		new CAiMgrSpawn;
	CAiMgrSpawn			*aiMgrSpawn2 =		new CAiMgrSpawn;
	CAiMgrPopulation	*aiMgrPopulation0 =	new CAiMgrPopulation;
	CAiMgrPopulation	*aiMgrPopulation1 =	new CAiMgrPopulation;
	CAiMgrPopulation	*aiMgrPopulation2 =	new CAiMgrPopulation;
	CAiMgrLocation		*aiMgrLocation0 =	new CAiMgrLocation;
	CAiMgrLocation		*aiMgrLocation1 =	new CAiMgrLocation;
	CAiMgrLocation		*aiMgrLocation2 =	new CAiMgrLocation;
	CAiMgrInstance		*aiMgrInstance0 =	new CAiMgrInstance;
	CAiMgrInstance		*aiMgrInstance1 =	new CAiMgrInstance;
	CAiMgrInstance		*aiMgrInstance2 =	new CAiMgrInstance;
	CAiMgrFile			*aiMgrFile0 =		new CAiMgrFile;
	CAiMgrFile			*aiMgrFile1 =		new CAiMgrFile;
	CAiMgrFile			*aiMgrFile2 =		new CAiMgrFile;

	//--------------------------------------

	aiMgrSpawn0->setPlace("aiMgrSpawn0");
	aiMgrSpawn1->setPlace("aiMgrSpawn1");
	aiMgrSpawn2->setPlace("aiMgrSpawn2");
	
	//--------------------------------------

	aiMgrPopulation0->setName("aiMgrPopulation0_name");
	aiMgrPopulation0->setType(NLMISC::CSheetId("aiMgrPopulation0_type"));
	aiMgrPopulation0->setQuantity(0);
	aiMgrPopulation0->setOrientation(0);
	aiMgrPopulation0->setSpawnRate(0,1);
	
	aiMgrPopulation1->setName("aiMgrPopulation1_name");
	aiMgrPopulation1->setType(NLMISC::CSheetId("aiMgrPopulation1_type"));
	aiMgrPopulation1->setQuantity(1);
	aiMgrPopulation1->setOrientation(3.14159265359f/2.0f);
	aiMgrPopulation1->setSpawnRate(2,3);
	
	aiMgrPopulation2->setName("aiMgrPopulation2_name");
	aiMgrPopulation2->setType(NLMISC::CSheetId("aiMgrPopulation2_type"));
	aiMgrPopulation2->setQuantity(2);
	aiMgrPopulation2->setOrientation(3.14159265359f);
	aiMgrPopulation2->setSpawnRate(4,5);

	//--------------------------------------

	aiMgrLocation0->setName("aiMgrLocation0_name");
	aiMgrLocation0->setType("aiMgrLocation0_type");
	aiMgrLocation0->setBoundary("aiMgrLocation0_boundary");
	
	aiMgrLocation1->setName("aiMgrLocation1_name");
	aiMgrLocation1->setType("aiMgrLocation1_type");
	aiMgrLocation1->setBoundary("aiMgrLocation1_boundary");
	
	aiMgrLocation2->setName("aiMgrLocation2_name");
	aiMgrLocation2->setType("aiMgrLocation2_type");
	aiMgrLocation2->setBoundary("aiMgrLocation2_boundary");
	
	//--------------------------------------

	aiMgrInstance0->setName("aiMgrInstance0");
	aiMgrInstance0->setBoundary("boundary0");
	aiMgrInstance0->setCreatureLimit(0);

	aiMgrInstance1->setName("aiMgrInstance1");
	aiMgrInstance1->setBoundary("boundary1");
	aiMgrInstance1->setCreatureLimit(1);

	aiMgrInstance2->setName("aiMgrInstance2");
	aiMgrInstance2->setBoundary("boundary2");
	aiMgrInstance2->setCreatureLimit(2);

	aiMgrInstance1->addSpawn(aiMgrSpawn0);
	aiMgrInstance1->addSpawn(aiMgrSpawn1);
	aiMgrInstance1->addSpawn(aiMgrSpawn2);

	aiMgrInstance1->addPopulation(aiMgrPopulation0);
	aiMgrInstance1->addPopulation(aiMgrPopulation1);
	aiMgrInstance1->addPopulation(aiMgrPopulation2);

	aiMgrInstance1->addLocation(aiMgrLocation0);
	aiMgrInstance1->addLocation(aiMgrLocation1);
	aiMgrInstance1->addLocation(aiMgrLocation2);

	aiMgrInstance1->addChild(aiMgrInstance2);

	//--------------------------------------

	aiMgrFile0->setName("aiMgrFile0");
	aiMgrFile0->setPrim("prim0.prim");

	aiMgrFile1->setName("aiMgrFile1");
	aiMgrFile0->setPrim("prim1.prim");

	aiMgrFile2->setName("aiMgrFile2");
	aiMgrFile0->setPrim("prim2.prim");

	aiMgrFile1->addChild(aiMgrInstance0);
	aiMgrFile1->addChild(aiMgrInstance1);

	//--------------------------------------

	CAiMgr::addFile(aiMgrFile0);
	CAiMgr::addFile(aiMgrFile1);
	CAiMgr::addFile(aiMgrFile2);
}

