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





// misc
#include "nel/misc/types_nl.h"
#include "nel/misc/sheet_id.h"
#include "nel/misc/config_file.h"

#include "nel/georges/u_form.h"
#include "nel/georges/u_form_elm.h"
#include "nel/georges/load_form.h"

// std
#include <string>
#include <vector>


#include <iostream>



using namespace NLMISC;
using namespace std;




/**
 * Class to manage a sheet.
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2003
 */
class CSheet
{

public:

	/// Alias
	string Alias;

public:

	/// Constructor
	CSheet() {}
	
	/// Load the values using the george sheet
	void readGeorges (const CSmartPtr<NLGEORGES::UForm> &form, const CSheetId &sheetId);

	/// Load/Save the values using the serial system
	void serial (NLMISC::IStream &s);

	/** 
	 * Event to implement any action when the sheet is no longer existent.
	 * This method is called when a sheet have been read from the packed sheet
	 * and the associated sheet file no more exist in the directories.
	 */
	void removed() {}

	static uint getVersion();

private :

	static const uint _Version;
};

uint const CSheet::_Version = 1;



//-----------------------------------------------
//	readGeorges 
//
//-----------------------------------------------
void CSheet::readGeorges (const CSmartPtr<NLGEORGES::UForm> &form, const CSheetId &sheetId)
{
	// Load the form with given sheet id
	if (form)
	{
		if( !form->getRootNode().getValueByName(Alias, "Alias", NLGEORGES::UFormElm::NoEval) )
		{

		}
	}
}


//-----------------------------------------------
//	serial
//
//-----------------------------------------------
void CSheet::serial (NLMISC::IStream &s)
{
	s.serial( Alias );
}



//-----------------------------------------------
//	getVersion 
//
//-----------------------------------------------
uint CSheet::getVersion () 
{ 
	//return CSheetManager::_AliasFilenameVersion; 
	return 1;
}




//-----------------------------------------------
//	displayHelp
//
//-----------------------------------------------
void displayHelp()
{
	cout<<"This tool creates a packed file which associates sheet id with sheet alias"<<endl;
	cout<<"Vars used in make_alias_file.cfg :"<<endl;
	cout<<"  - SheetPaths : the paths where to find the sheets"<<endl;
	cout<<"  - AliasFilename : name used for alias file (default is 'alias.packed_sheets')"<<endl;
	cout<<"  - Extensions : the extensions list of sheets to read"<<endl;
	cout<<endl;
	cout<<"MAKE_ALIAS_FILE"<<endl;

} // displayHelp //






//-----------------------------------------------
//	MAIN
//
//-----------------------------------------------
int main( int argc, char ** argv )
{
	// get the output filename
	if( argc > 1 )
	{
		displayHelp();
		return EXIT_FAILURE;
	}
	
	// load the cfg
	CConfigFile configFile;
	string configFileName = "make_alias_file.cfg";
	if(!CFile::fileExists(configFileName))
	{
		nlwarning("Config file %s not found",configFileName.c_str());
		return 1;
	}
	configFile.load(configFileName);

	
	// read the sheet paths
	vector<string> sheetPaths;
	try
	{
		CConfigFile::CVar& cvSheetPaths = configFile.getVar("SheetPaths");
		sint i;
		for( i = 0; i< (sint)cvSheetPaths.size(); ++i)
		{
			sheetPaths.push_back( cvSheetPaths.asString(i) );
		}
	}
	catch(const EUnknownVar &) 
	{
		nlwarning("var 'SheetPaths' not found");
	}

	// add the sheet paths
	cout<<"adding the sheet paths ..."<<endl;
	vector<string>::iterator itPath;
	for( itPath = sheetPaths.begin(); itPath != sheetPaths.end(); ++itPath )
	{
		CPath::addSearchPath(*itPath,true,false);
	}

	// read the name that will be used for packed file
	string aliasFilename = "alias.packed_sheets";
	try
	{
		CConfigFile::CVar &cvAliasFilename = configFile.getVar("AliasFilename");
		aliasFilename = cvAliasFilename.asString();
	}
	catch(const EUnknownVar &) 
	{
		nlwarning("var 'AliasFilename' not found");
	}
	
	// read the extensions list of sheets to read
	vector<string> extensions;
	try
	{
		CConfigFile::CVar& cvExtensions = configFile.getVar("Extensions");
		sint i;
		for( i = 0; i< (sint)cvExtensions.size(); ++i)
		{
			extensions.push_back( cvExtensions.asString(i) );
		}
	}
	catch(const EUnknownVar &) 
	{
		nlwarning("var 'Extensions' not found");
	}

	// load form and create packed sheets
	cout<<"reading the sheets ..."<<endl;
	map<CSheetId, CSheet> sheetMap;
	bool updatePackedSheet = true;
	createDebug(); // needed to init WarningLog
	loadForm (extensions, aliasFilename, sheetMap, updatePackedSheet);
	cout<<"finished."<<endl;

	// display the content of the map
	/*
	map<CSheetId, CSheet>::iterator itAlias;
	for( itAlias = sheetMap.begin(); itAlias != sheetMap.end(); ++itAlias )
	{
		nlinfo("sheet %s  alias = %s",(*itAlias).first.toString().c_str(),(*itAlias).second.Alias.c_str());
	}
	*/

	return EXIT_SUCCESS;

} // main //




