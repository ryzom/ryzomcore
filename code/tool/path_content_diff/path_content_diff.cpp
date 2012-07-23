// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#include <nel/misc/path.h>
#include <nel/misc/debug.h>
#include <nel/misc/file.h>

#include <nel/misc/sha1.h>

using namespace std;
using namespace NLMISC;


//-----------------------------------------------
//	main
//
//-----------------------------------------------
int main( int argc, char ** argv )
{
	if( argc < 3 || argc > 4 )
	{
		printf("Build a listing of the diff of two path contents and copy new files in <dest_path>\n");
		printf("usage: path_content <ref_path> <new_path> [<dest_path>]\n");
		return EXIT_FAILURE;
	}
	
	string DestPath;
	if( argc == 4 )
	{
		DestPath = CPath::standardizeDosPath(argv[3]);
		if(CFile::isExists(DestPath))
		{
			if(!CFile::isDirectory(DestPath))
			{
				printf("'%s' is not a directory\n", DestPath.c_str());
				return EXIT_FAILURE;
			}
		}
		else
		{
			if (!CFile::createDirectory(DestPath))
			{
				printf("Can't create directory: '%s'\n", DestPath.c_str());
				return EXIT_FAILURE;
			}
		}
	}

	// content of new path
	string newPath(argv[2]);
	vector<string> newPathContent;
	CPath::getPathContent(newPath, true, false, true, newPathContent);
	
	string outputFileName = CFile::findNewFile("path_content_diff.txt");
	FILE *output = fopen (outputFileName.c_str(), "wt");
	if( output == NULL )
	{
		nlwarning("Can't open output file %s",outputFileName.c_str());
		return EXIT_FAILURE;
	}

	// add ref path in search paths
	string refPath(argv[1]);
	CPath::addSearchPath(refPath, true, false);
	
	map<string,CHashKey> refSHAMap;
	CIFile refSHAFile;
	if( refSHAFile.open(refPath +".sha1key") )
	{
		// load the map of SHA hash key for the ref files
		refSHAFile.serialCont( refSHAMap );
		refSHAFile.close();
	}
	else
	{
		// build the map of SHA hash key for the ref files
		string extension;
		vector<string> refPathContent;
		CPath::getFileList(extension, refPathContent);
		vector<string>::const_iterator itFile;
		for( itFile = refPathContent.begin(); itFile != refPathContent.end(); ++itFile )
		{
			refSHAMap.insert( make_pair(*itFile,getSHA1(*itFile)) );
		}
		COFile refSHAFile(refPath + ".sha1key");
		refSHAFile.serialCont( refSHAMap );
	}


	// build the map of SHA hash key for new files
	map<string,CHashKey> newSHAMap;
	vector<string>::const_iterator itFile;
	for( itFile = newPathContent.begin(); itFile != newPathContent.end(); ++itFile )
	{
		newSHAMap.insert( make_pair(*itFile,getSHA1(*itFile)) );
	}

// display (debug)
	map<string,CHashKey>::iterator itSHA;
	/*
	for( itSHA = refSHAMap.begin(); itSHA != refSHAMap.end(); ++itSHA )
	{
		nlinfo("(ref) %s : %s",(*itSHA).first.c_str(),(*itSHA).second.toString().c_str());
	}
	for( itSHA = newSHAMap.begin(); itSHA != newSHAMap.end(); ++itSHA )
	{
		nlinfo("(new) %s : %s",(*itSHA).first.c_str(),(*itSHA).second.toString().c_str());
	}
	*/
//

	uint32 LastDisplay = 0, curFile = 0;

	// get the list of new or modified files
	vector<string> differentFiles;
	for( itFile = newPathContent.begin(); itFile != newPathContent.end(); ++itFile )
	{
		string newFileName = *itFile;
		string newFileNameShort = CFile::getFilename(newFileName);

		curFile++;

		if (CTime::getSecondsSince1970() > LastDisplay + 5)
		{
			printf("%d on %d files, %d left\n", curFile, newPathContent.size(), newPathContent.size() - curFile);
			LastDisplay = CTime::getSecondsSince1970();
		}

		if( CFile::getExtension(newFileNameShort) == "bnp" )
		{
			nlwarning ("BNP PROBLEM: %s is a big file, content of big files is not managed", newFileName.c_str());
			nlwarning ("The <new_path> must *not* contains .bnp files");
		}

		bool keepIt = false;

		string refFileName = CPath::lookup(strlwr(newFileNameShort), false, false, false);
		if( refFileName.empty() )
		{
			keepIt = true;
			nlinfo ("new file : %s",newFileNameShort.c_str());
		}
		else
		{
			itSHA = refSHAMap.find( newFileNameShort );
			CHashKey refSHA;
			if( itSHA != refSHAMap.end() )
			{
				refSHA = (*itSHA).second;
			}

			itSHA = newSHAMap.find( newFileName );
			CHashKey newSHA;
			if( itSHA != newSHAMap.end() )
			{
				newSHA = (*itSHA).second;
			}

			if( !(refSHA==newSHA) )
			{
				keepIt = true;
				nlinfo("file : %s , key : %s(%s), size : %d(%d)",newFileNameShort.c_str(), newSHA.toString().c_str(),refSHA.toString().c_str(),CFile::getFileSize(newFileName),CFile::getFileSize(refFileName));
			}
			
			/*
			uint32 refModificationDate = CFile::getFileModificationDate( refFileName );
			uint32 newModificationDate = CFile::getFileModificationDate( newFileName );		

			if( newModificationDate > refModificationDate )
			{
				keepIt = true;
				nlinfo ("DATE CHANGED: %s", newFileName.c_str());
			}
			else
			{
				// same date, must be same size
				uint32 refSize = CFile::getFileSize( refFileName );
				uint32 newSize = CFile::getFileSize( newFileName );
				if( refSize != newSize )
				{
					nlwarning ("DATE PROBLEM: file '%s' have the same date but not the same size than '%s'", newFileName.c_str(), refFileName.c_str());
				}
			}
			*/
		}

		if( keepIt )
		{
			differentFiles.push_back( newFileName );

			//uint32 newCreationDate = CFile::getFileCreationDate( newFileName );
			//string outputLine = newFileName + "\t\t"+toString(newSize) + "\t" + toString(newModificationDate) + "\t" + toString(newCreationDate) + "\n";
			string outputLine = newFileName + "\n";
			fprintf (output, outputLine.c_str());
			
			if( !DestPath.empty() )
			{
				string systemStr = "copy /Y " + CPath::standardizeDosPath(newFileName) + " " + DestPath;
				//nlinfo("System call '%s'",systemStr.c_str());
				system( systemStr.c_str() );
			}
		}
	}

	return EXIT_SUCCESS;
}

