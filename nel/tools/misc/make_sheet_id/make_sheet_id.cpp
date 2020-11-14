// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2012-2019  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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
#include <nel/misc/types_nl.h>
#include <nel/misc/file.h>
#include <nel/misc/common.h>
#include <nel/misc/path.h>
#include <nel/misc/config_file.h>

// std
#include <string>
#include <stdlib.h>

#include <stdio.h>


using namespace NLMISC;
using namespace std;

#ifndef NL_MK_SH_ID_CFG
#define NL_MK_SH_ID_CFG "."
#endif // NL_MK_SH_ID_CFG

/**
 *	TFormId
 */
union TFormId
{
	uint32		Id;
	
	struct
	{
		uint32	Type	: 8;
		uint32	Id		: 24;
	} FormIDInfos;

	void serial(NLMISC::IStream &f) { f.serial(Id); };
};

bool operator<(const TFormId& fid1, const TFormId& fid2) { return fid1.Id<fid2.Id; }


map<string,TFormId> FormToId;
map<TFormId,string> IdToForm;
map<string,uint8> FileTypeToId;
map<uint8,string> IdToFileType;
map<uint8,uint32> TypeToLastId;
set<string>	ExtensionsAllowed;

// stat

uint32 NbTypesAdded = 0;
uint32 NbFilesAdded = 0;
uint32 NbFilesAlreadyAdded = 0;
uint32 NbFilesUnknownType = 0;
uint32 NbFilesDiscarded = 0;


//manageFile
//void manageFile( WIN32_FIND_DATA& findFileData, list<string>& dirs, string& currentDir );

// addId
void addId( string fileName );

// getFileType
bool getFileType( string& fileName, string& fileType );

// displayHelp
void displayHelp();

// main
int main( int argc, char ** argv );





//-----------------------------------------------
//	displayHelp
//
//-----------------------------------------------
void displayHelp()
{
	printf("This tool associates an ID to the files in the given directory\n\n");
	printf("Usage: make_sheet_id -c<config file> -o<input/output file> [-k] [-e] [<start directory>] [<directory2>] ...\n");
	printf("-k : clean unwanted types from input\n");
	printf("-e : dump the list of extensions\n");

} // displayHelp //



//-----------------------------------------------
//	getFirstFreeFileTypeId
//
//-----------------------------------------------
sint16 getFirstFreeFileTypeId()
{
	for( sint16 id=0; id<256; ++id )
	{
		if( IdToFileType.find((uint8)id) == IdToFileType.end() )
		{
			return id;
		}
	}
	
	return -1;

} // getFirstFreeFileTypeId //


//-----------------------------------------------
//	readFormId
//
//-----------------------------------------------
void readFormId( string& outputFileName )
{
	CIFile f;
	if( f.open( outputFileName ) )
	{
		f.serialCont( IdToForm );
	}

	// insert an unknown entry
	TFormId formId;
	formId.Id = 0;
	IdToForm.insert( make_pair( formId, string("unknown.unknown") ) );

	// remove integer file extensions (created by CVS) and init FileTypeToId (associates the form type to the form type id)
	map<TFormId,string>::iterator itIF;
	for( itIF = IdToForm.begin(); itIF != IdToForm.end();  )
	{
		// get the file type from form name
		TFormId fid = (*itIF).first;

		if ((*itIF).second.empty() || (*itIF).second=="." || (*itIF).second==".." || (*itIF).second[0]=='_' || (*itIF).second.find(".#")==0)
		{
			map<TFormId,string>::iterator itErase = itIF;
			++itIF;
			IdToForm.erase(itErase);
		}
		else
		{
			string fileType;
			if (getFileType((*itIF).second, fileType))
			{	
				// insert the association (file type/file type id)
				map<string,uint8>::iterator itFT = FileTypeToId.find(fileType);
				if( itFT == FileTypeToId.end() )
				{
					uint8 type = (uint8)fid.FormIDInfos.Type;

					FileTypeToId.insert( std::pair<std::string, uint8>(fileType, type) );
				}
			}
			else
			{
				nlwarning("Unknown file type for the file : %s",(*itIF).second.c_str());
			}
			++itIF;
		}
	}


	// init FormToId (associates the form name to its id )
	for( itIF = IdToForm.begin(); itIF != IdToForm.end(); ++itIF )
	{
		FormToId.insert( make_pair((*itIF).second,(*itIF).first) );
	}
	
	// init IdToFileType (associates the form type id to the form type name)
	map<string,uint8>::iterator itIFT;
	for( itIFT = FileTypeToId.begin(); itIFT != FileTypeToId.end(); ++itIFT )
	{
		IdToFileType.insert( make_pair((*itIFT).second,(*itIFT).first) );
	}

	// init TypeToLastId (associates the type id to the last index used for this type)
	for( itIF = IdToForm.begin(); itIF != IdToForm.end(); ++itIF )
	{
		uint8 type = (*itIF).first.FormIDInfos.Type;
		uint32 id = (*itIF).first.FormIDInfos.Id;
		map<uint8,uint32>::iterator itTLI = TypeToLastId.find( type );
		if( itTLI != TypeToLastId.end() )
		{
			if( (*itTLI).second < id )
			{
				(*itTLI).second = id;
			}
		}
		else
		{
			TypeToLastId.insert( make_pair(type,id) );
		}
	}
	
} // readFormId //




//-----------------------------------------------
//	makeId
//
//-----------------------------------------------
void makeId( list<string>& dirs )
{
	list<string>::const_iterator itDir;
	for( itDir = dirs.begin(); itDir != dirs.end(); ++itDir )
	{
		nlinfo ("Searching files in directory '%s'...", (*itDir).c_str());
		vector<string> files;
		CPath::getPathContent(*itDir,true,false,true,files);

		nlinfo ("Found %d files in directory '%s'", files.size(), (*itDir).c_str());
		for(uint i = 0; i < files.size(); i++)
		  {
			addId(CFile::getFilename(files[i]));
		  }

	  /*		WIN32_FIND_DATA findFileData;
		HANDLE hFind;
		hFind = FindFirstFile((*itDir+"\\*").c_str(), &findFileData );
		string currentDir(*itDir);

		if( hFind == INVALID_HANDLE_VALUE ) 
		{
    		nlwarning ("Invalid File Handle");
		}
		else 
		{
			do
			{
				manageFile( findFileData, dirs, currentDir );
			}
    		while( FindNextFile( hFind, &findFileData ) );
			FindClose( hFind );
			}*/
	}

} // makeId //



//-----------------------------------------------
//	manageFile
//
//-----------------------------------------------
/*void manageFile( WIN32_FIND_DATA& findFileData, list<string>& dirs, string& currentDir )
{
	if( strcmp(".",findFileData.cFileName) && strcmp("..",findFileData.cFileName) && strcmp("CVS",findFileData.cFileName))
	{
		if( findFileData.cFileName[0] != '_' )
		{
			if( findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			{
				//cout<<"(D)"<<findFileData.cFileName<<endl;
				//nlinfo ("(D) %s", findFileData.cFileName);

				dirs.push_back( currentDir + "\\" + string(findFileData.cFileName) );
			}
			else
			{
				//nlinfo ("(F) %s", findFileData.cFileName);

				addId( string(findFileData.cFileName) );
			}
		}
	}

} // manageFile //
*/


//-----------------------------------------------
//	addId
//
//-----------------------------------------------
void addId( string fileName )
{
	if (fileName.empty() || fileName == "." || fileName == ".." || fileName[0] == '_' || fileName.find(".#") == 0)
	{
		// nlinfo("Discarding file '%s'", fileName.c_str());
		NbFilesDiscarded++;
		return;
	}
	else
	{
		if (!ExtensionsAllowed.empty())
		{
			string extStr = CFile::getExtension(fileName);
			if (ExtensionsAllowed.find(extStr) == ExtensionsAllowed.end())
			{
				NbFilesDiscarded++;
				return;
			}
		}
	}

	// if the file is new
	map<string,TFormId>::iterator itFI = FormToId.find( fileName );
	if( itFI == FormToId.end() )
	{
		// double check : if file not found we check with lower case version of filename
		map<string,TFormId>::iterator itFILwr = FormToId.find( toLowerAscii(fileName) );
		if( itFILwr != FormToId.end() )
		{
			nlwarning("Trying to add %s but the file %s is already known ! becareful with lower case and upper case.", fileName.c_str(), toLowerAscii(fileName).c_str());
			NbFilesDiscarded++;
			return;
		}

		string fileType;
		if( getFileType( fileName, fileType ) )
		{
			map<string,uint8>::iterator itFTI = FileTypeToId.find( fileType );
			TFormId fid;
			
			// if the type of this file is a new type
			if( itFTI == FileTypeToId.end() )
			{
				sint16 firstFreeFileTypeId = getFirstFreeFileTypeId();
				if( firstFreeFileTypeId == -1 )
				{
					nlwarning("MORE THAN 256 FILE TYPES!!!!");
					return;
				}
				else
				{
					FileTypeToId.insert( make_pair(fileType,(uint8)firstFreeFileTypeId) );
					IdToFileType.insert( make_pair((uint8)firstFreeFileTypeId,fileType) );

					// Reserve id 0 for unknown.newtype.
					// User may supply a sheet called unknown.newtype
					// that can safely be used as a fallback when a
					// requested sheet does not exist.
					// Only for newly added sheet types.
					fid.FormIDInfos.Type = (uint8)firstFreeFileTypeId;
					fid.FormIDInfos.Id = 0;
					std::string unknownNewType = std::string("unknown." + fileType);
					FormToId.insert(make_pair(unknownNewType, fid));
					IdToForm.insert(make_pair(fid, unknownNewType));

					TypeToLastId.insert( make_pair((uint8)firstFreeFileTypeId,1) );
					fid.FormIDInfos.Id = 1;

					nlinfo("Adding file type '%s' with id %d", fileType.c_str(), firstFreeFileTypeId);
					NbTypesAdded++;
				}
			}
			// else the file type already exist
			else
			{
				// id of the file type
				uint8 fileTypeId = (*itFTI).second;

				// last id used for this file type
				map<uint8,uint32>::iterator itTLI = TypeToLastId.find(fileTypeId);
				nlassert(itTLI != TypeToLastId.end());
				(*itTLI).second++;

				// add the new association
				fid.FormIDInfos.Type = fileTypeId;
				fid.FormIDInfos.Id = (*itTLI).second;
			}
			FormToId.insert( make_pair(fileName,fid) );
			IdToForm.insert( make_pair(fid,fileName) );
			nlinfo("Adding file '%s' id %d with type '%s' id %d", fileName.c_str(), fid.FormIDInfos.Id, fileType.c_str(), fid.FormIDInfos.Type);
			NbFilesAdded++;
		}
		else
		{
		  //nlinfo("Unknown file type for the file : '%s' --> not added",fileName.c_str());
			NbFilesUnknownType++;
		}
	}
	else
	  {
		//nlinfo("Skipping file '%s', already in the file", fileName.c_str());
			NbFilesAlreadyAdded++;
	  }
} // addId //



//-----------------------------------------------
//	getFileType
//
//-----------------------------------------------
bool getFileType( string& fileName, string& fileType )
{
  fileType = CFile::getExtension(CFile::getFilename(fileName));
  return !fileType.empty();
  
} // getFileType //



//-----------------------------------------------
//	display
//
//-----------------------------------------------
void display()
{
	nldebug ("Output :");
	map<TFormId,string>::iterator it1;
	for( it1 = IdToForm.begin(); it1 != IdToForm.end(); ++it1 )
	{
		nldebug("type: %d id: %d file: %s", (*it1).first.FormIDInfos.Type, (*it1).first.FormIDInfos.Id, (*it1).second.c_str());
	}

} // display //



//-----------------------------------------------
//	MAIN
//
//-----------------------------------------------
int main( int argc, char ** argv )
{
	// Create an application context.
	NLMISC::CApplicationContext appContext;

#ifdef NL_OS_UNIX
	NLMISC::CPath::addSearchPath(NLMISC::CPath::getApplicationDirectory("NeL"));
#endif // NL_OS_UNIX

	NLMISC::CPath::addSearchPath(NL_MK_SH_ID_CFG);

	// read args
	if( argc < 2 )
	{
		displayHelp();
		return EXIT_FAILURE;
	}
	if( strcmp(argv[1],"/?") == 0 )
	{
		displayHelp();
		return EXIT_FAILURE;
	}
	
	string configFileName;
	string outputFileName;

	bool clean = false;
	bool dumpExtensions = false;
	list<string> inputDirs;
	if( argc < 3 )
		inputDirs.push_back(".");
	else
	{
		for ( uint i=1; (sint)i<argc; ++i )
		{
			if ( argv[i][0] == '-' )
			{
				switch ( argv[i][1] )
				{
					case 'C':
					case 'c':
						configFileName = string( argv[i]+2 );
						break;
					case 'O':
					case 'o':
						outputFileName = string( argv[i]+2 );
						break;
					case 'k':
						clean = true;
						break;
					case 'e':
						dumpExtensions = true;
						break;
					default:
						break;
				}
			}
			else
				inputDirs.push_back(argv[i]);
		}
	}

	if( outputFileName.empty() )
	{
		nlwarning( "No input/output file !!! (mettre a jour update_sheet_id.bat)");

		displayHelp();
		return EXIT_FAILURE;
	}

	
	// load the config files
	CConfigFile	configFile;
	if( configFileName.empty() )
	{
		configFileName = "make_sheet_id.cfg";
	}
	if(!CFile::fileExists(configFileName))
	{
		nlinfo( "config file '%s' not found, working whithout filter", configFileName.c_str() );
	}
	else
	{
		configFile.load (configFileName);
		CConfigFile::CVar * varPtr = configFile.getVarPtr("ExtensionsAllowed");
		if(varPtr)										
		{	
			for( uint i=0; i<varPtr->size(); ++i )							
			{	
				ExtensionsAllowed.insert( varPtr->asString(i) );
			}												
		}
	}
		
	// get the current associations (read the sheet_id and fill the working structures)
	readFormId( outputFileName );

	// output path
	std::string::size_type lastSeparator = CFile::getLastSeparator(outputFileName);
	string outputPath;
	if( lastSeparator != std::string::npos )
	{
		outputPath = outputFileName.substr(0,lastSeparator+1);
	}

	// erase the unwanted extensions from map (modify the map, save it, and quit)
	if( clean )
	{
		if( ExtensionsAllowed.empty() )
			nlwarning("None extension list provided, the input will not be cleaned");
		else
		{
			map<TFormId,string>::iterator itSheets;
			for( itSheets = IdToForm.begin(); itSheets != IdToForm.end();  )
			{
				string extStr = CFile::getExtension( (*itSheets).second );
				if( !extStr.empty() )
				{
					if( ExtensionsAllowed.find(extStr) == ExtensionsAllowed.end() )
					{
						map<TFormId,string>::iterator itDel = itSheets++;
						IdToForm.erase( itDel );
					}
					else
						++itSheets;
				}
			}
			COFile f( outputFileName );
			f.serialCont( IdToForm );
		}
		nlinfo("The file has been cleaned");
		return 0;
	}

	// dump the list of extensions in a txt file
	if( dumpExtensions )
	{
		string extListFileName = outputPath + "sheet_ext.txt";
		FILE *extListOutput = nlfopen(extListFileName, "w");
		if (!extListOutput)
		{
			nlwarning("Can't open output file %s",extListFileName.c_str());
			return 1;
		}
		set<string> extensions;
		map<TFormId,string>::iterator itSheets;
		for( itSheets = IdToForm.begin(); itSheets != IdToForm.end(); ++itSheets )
		{
			string extStr = CFile::getExtension( (*itSheets).second );
			if( !extStr.empty() )
			{
				extensions.insert( extStr );
			}
		}
		set<string>::iterator itExt;
		for( itExt = extensions.begin(); itExt != extensions.end(); ++itExt )
		{
			fprintf(extListOutput,"%s\n",(*itExt).c_str());
		}
		fclose(extListOutput);
		return 0;
	}
	

	nlinfo("Generating '%s' file...", outputFileName.c_str());

	
		
	// make the ids
	makeId( inputDirs );

	// save the new map
	COFile f( outputFileName );
	f.serialCont( IdToForm );

	// display the map
	//display();

	string sheetListFileName = outputPath + "sheets.txt";
	COFile output;
	if( !output.open(sheetListFileName,false,true) )
	{
		nlwarning("Can't open output file %s",sheetListFileName.c_str());
		return 1;
	}
	map<TFormId,string>::iterator it1;
	for( it1 = IdToForm.begin(); it1 != IdToForm.end(); ++it1 )
	{
		//string outputLine = "type: " + toString((*it1).first.FormIDInfos.Type) +" id: " + toString((*it1).first.FormIDInfos.Id) + " file: " + (*it1).second +"\n";
		string outputLine = " id: " + toString((*it1).first.Id) + " file: " + (*it1).second +"\n";
		output.serialBuffer((uint8*)(const_cast<char*>(outputLine.data())),(uint)outputLine.size());
	}

	nlinfo ("------------- results ----------------");
	nlinfo ("%d files added in '%s'", NbFilesAdded, outputFileName.c_str());
	nlinfo ("%d files discarded because they are empty, begin with .# _ and so on", NbFilesDiscarded);
	nlinfo ("%d files skipped because don't have extension", NbFilesUnknownType);
	nlinfo ("%d types added in '%s'", NbTypesAdded, outputFileName.c_str());

	nlinfo ("%d supported file types :",FileTypeToId.size());
	for ( map<string,uint8>::iterator it = FileTypeToId.begin(); it != FileTypeToId.end(); ++it )
	{
		nlinfo("%s",(*it).first.c_str());
	}

	return EXIT_SUCCESS;

} // main //

