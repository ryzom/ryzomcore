// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2015  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include "nel/misc/types_nl.h"
#include "nel/misc/app_context.h"
#include "nel/misc/path.h"
#include "nel/misc/common.h"
#include "nel/misc/sstring.h"
#include "nel/misc/algo.h"

using namespace std;
using namespace NLMISC;

enum TAction
{
	pack,
	unpack,
	undefined
};

const string DefaultExt("xml_pack");
const uint32 MaxLineSize = 16*1024;

const string ExcludeFiles(".#*;*.log;*.bin;*.xml_pack_index");
const string ExcludeDirs("CVS");

bool isExcludedFile(const std::string &fileName)
{
	static vector<string> excludeFileVect;
	static bool init = false;
	
	if (!init)
	{
		explode(ExcludeFiles, string(";"), excludeFileVect, true);
		init = true;
	}
	
	bool excluded = false;
	
	for (uint i=0; i<excludeFileVect.size(); ++i)
	{
		if (testWildCard(fileName, excludeFileVect[i]))
		{
			excluded = true;
			break;
		}
	}
	
	return excluded;
}

bool isExcludedDir(const std::string &dirName)
{
	static vector<string> excludeDirVect;
	static bool init = false;
	
	if (!init)
	{
		explode(ExcludeDirs, string(";"), excludeDirVect, true);
	}
	
	bool excluded = false;
	
	for (uint i=0; i<excludeDirVect.size(); ++i)
	{
		if (testWildCard(dirName, excludeDirVect[i]))
		{
			excluded = true;
			break;
		}
	}
	
	return excluded;
}

string getLastDirName(const std::string &path)
{
	string dirName;
	string::size_type pos = path.size()-1;
	
	// skip any terminal directory separator
	if (pos > 0 && (path[pos] == '\\' || path[pos] == '/'))
		--pos;
	
	while(pos > 0 && path[pos] != '\\' && path[pos] != '/' )
		dirName = path[pos--] + dirName;
	
	return dirName;
}

int main(int argc, char *argv[])
{
	printf("NeL XML Packer/Unpacker V0.3\n");
	CApplicationContext appContext;
	
	TAction action = undefined;
	bool	recursive = false;
	
	// compute the current folder name
	string currentPath = CPath::getCurrentPath();
	string dirName = getLastDirName(currentPath);;
	
	string filename = dirName + "."+DefaultExt;
	
	// check the params to choose action
	for (uint i=0; i<uint(argc); ++i)
	{
		if (strcmp(argv[i], "-p") == 0)
		{
			// pack the current folder
			action = pack;
			printf("Packing files\n");
		}
		else if (strcmp(argv[i], "-u") == 0)
		{
			// unpack the current folder
			action = unpack;
			printf("Unpacking files\n");
		}
		else if (strcmp(argv[i], "-r") == 0)
		{
			// unpack the current folder
			recursive = true;
			printf("Recursive mode\n");
		}
		//		else if (strcmp(argv[i], "-f") == 0)
		//		{
		//			if (uint(argc) < i+i)
		//			{
		//				printf("Error : missing file name after -f\n");
		//				return -1;
		//			}
		//			// use the specified file archive instead of the directory name
		//			filename = argv[++i];
		//		}
	}
	
	if (action == undefined)
	{
		printf("Usage : %s -u|-p [-r]\n", argv[0]);
		printf("  -p : pack the current folder\n");
		printf("  -u : unpack the current folder\n");
		printf("  -r : pack or unpack subdirectories recursively\n");
		
		return -1;
	}
	
	vector<string>	dirStack;
	
	printf("Current path is '%s'\n", CPath::getCurrentPath().c_str());
	
	// push the current directory to start the loop
	dirStack.push_back(CPath::getCurrentPath());
	
	
	while(!dirStack.empty())
	{
		string dirName = dirStack.back();
		dirStack.pop_back();
		string filename = dirName+"/"+getLastDirName(dirName) + "."+DefaultExt;
		switch (action)
		{
		case pack:
			{
				printf("Packing directory '%s'...\n", dirName.c_str());
//				string packFileName = dirName+"/tmp."+DefaultExt;
				string packFileName = filename;
				string indexFileName = dirName+"/.xml_pack_index";

				// get the current directory content
				vector<string> files;
				CPath::getPathContent(dirName, false, false, true, files);

				vector<string>	validFiles;
				
				// first loop to build the list of valid file
				for (uint i=0; i<files.size(); ++i)
				{
					if (files[i].find(DefaultExt) == files[i].size() - DefaultExt.size())
						continue; 
					
					string &subFileName = files[i];
					
					// check exclude filter
					if (isExcludedFile(subFileName))
					{
						continue;
					}
					
					// ok, this file is valid
					validFiles.push_back(subFileName);
				}

				bool needRepack = true;

				// if an xml pack already exist in the folder...
				if (CFile::fileExists(packFileName))
				{
					breakable
					{
						if (validFiles.empty())
						{
							// no file in the directory, erase the pack file
							CFile::deleteFile(packFileName);
							break;
						}

						uint32 packDate = CFile::getFileModificationDate(packFileName);

						if (!CFile::fileExists(indexFileName) || CFile::getFileModificationDate(indexFileName) < packDate)
						{
							// no index file or index file older than pack file, repack
							break;
						}

						// read the index file
						set<string> fileInIndex;
						char lineBuffer[1024];
						FILE *fp = nlfopen(indexFileName, "rt");
						while (fgets(lineBuffer, 1024, fp))
							fileInIndex.insert(CSString(lineBuffer).strip());

						fclose(fp);

						// loop to check for file time stamp
						for (uint i=0; i<validFiles.size(); ++i)
						{
							uint32 fileDate = CFile::getFileModificationDate(validFiles[i]);

							if (fileDate >= packDate)
								// no more to check
								break;

							// remove this file from the file index
							fileInIndex.erase(CFile::getFilename(validFiles[i]));
						}

						// check if there are some some deleted in the directory
						if (!fileInIndex.empty())
						{
							// need to repack, there are erased files
							break;
						}

						// all files are older than the pack file ! no repack needed
						needRepack = false;
					}
				}
				
				// we need to repack and have some file to store ?
				if (!validFiles.empty() && needRepack)
				{
					// open the pack file
					//				FILE *fp = nlfopen(filename, "wt");
					FILE *fp = nlfopen(packFileName, "wt");
					
					fprintf(fp, "<nel:packed_xml>\n");
					
					for (uint i=0; i<validFiles.size(); ++i)
					{
						string &subFileName = validFiles[i];
						
						printf("Adding file '%s'...\n", CFile::getFilename(subFileName).c_str());
						fprintf(fp, "	<nel:xml_file name=\"%s\">\n", CFile::getFilename(subFileName).c_str());
						
						FILE *subFp = nlfopen(subFileName, "rt");
						nlassert(subFp != NULL);
						char buffer[MaxLineSize];
						char *result;
						bool needFinalReturn = false;
						result = fgets(buffer, MaxLineSize, subFp);
						needFinalReturn = result != NULL ? buffer[strlen(buffer)-1] != '\n' : true;
						while(result != 0)
						{
							fputs(buffer, fp);
							result = fgets(buffer, MaxLineSize, subFp);
							needFinalReturn = result != NULL ? buffer[strlen(buffer)-1] != '\n' : needFinalReturn;
						}
						if (needFinalReturn)
						{
							const char *finalReturn = "\n";
							fputs(finalReturn, fp);
						}
						
						fclose(subFp);
						
						fprintf(fp, "	</nel:xml_file>\n");
					}
					
					fprintf(fp, "</nel:packed_xml>\n");
					
					fclose(fp);

					// write the disposable index file used by pack to check for erased file
					fp = nlfopen(indexFileName, "wt");
					for (uint i=0; i<validFiles.size(); ++i)
					{
						fprintf(fp, "%s\n", CFile::getFilename(validFiles[i]).c_str());
					}
					fclose(fp);
					// set the file 'hidden'n use the plain old system command...
					sint res = system(toString("attrib +h %s", indexFileName.c_str()).c_str());
					if (res)
					{
						nlwarning("attrib failed with return code %d", res);
					}
				}
				else
				{
					printf("Directory %s is up to date, no repack\n", dirName.c_str());
				}
			}
			break;
		case unpack:
			{
				printf("Unpacking directory '%s'...\n", dirName.c_str());
				// open the pack file
//				FILE *fp = nlfopen(dirName+"/tmp."+DefaultExt, "rt");
				FILE *fp = nlfopen(filename, "rt");
				if (!recursive)
				{
					// if we are not recursive, we MUST have a file here
					printf("Error : can't find a xml_pack file in current directory\n");
					exit(-1);
				}
				else
				{
					// just continue to recurse, there is no file at this level
					break;
				}
				uint linecount = 0;
				
				// read the first line
				char buffer[MaxLineSize];
				if (!fgets(buffer, MaxLineSize, fp) || strcmp(buffer, "<nel:packed_xml>\n") != 0)
				{
					printf ("Error : invalid pack file '%s'\n", filename.c_str());
					return -1;
				}

				linecount++;

				char *result = NULL;
				do 
				{
					// read a file line
					linecount++;
					if (!fgets(buffer, MaxLineSize, fp))
					{
						fclose(fp);
						printf ("Error : invalid pack file '%s' at line %u", filename.c_str(), linecount);
						return -1;
					}
					CSString parser(buffer);
					if (parser.find("	<nel:xml_file name=") != 0)
					{
						fclose(fp);

						// end of pack file
						if (parser.find("</nel:packed_xml>") == 0)
							break;

						printf ("Error : invalid pack file '%s' at line %u", filename.c_str(), linecount);
						return -1;
					}
					
					CSString subFileName = parser.leftCrop(sizeof("	<nel:xml_file name=")-1);
					subFileName = subFileName.matchDelimiters(false, false, true, false);
					subFileName = subFileName.unquoteIfQuoted();
					subFileName = dirName + "/" + subFileName.c_str();
					
					printf("Extracting file '%s'...\n", CFile::getFilename(subFileName).c_str());
					// open the output file 
					FILE *output = fopen (subFileName.c_str(), "wt");
					if (output == NULL)
					{
						printf ("Error : can not open output file '%s' from pack file '%s'", subFileName.c_str(), filename.c_str());
						exit(-1);
					}
					
					result = fgets(buffer, MaxLineSize, fp);
					linecount++;
					while (result != NULL && strcmp(buffer, "	</nel:xml_file>\n") != 0)
					{
						fputs(result, output);
						// read next line
						result = fgets(buffer, MaxLineSize, fp);
						linecount++;
					}
					
					fclose(output);
					
				} while(result != NULL);
				
			}
			break;
		default:
			// this shouldn't happen / keep compiler happy
			break;
		}
		
		if (recursive)
		{
			vector<string> subDirs;
			CPath::getPathContent(dirName, false, true, false, subDirs);
			
			// filter the directories
			for (uint i=(uint)subDirs.size(); i>0; --i)
			{
				if (!isExcludedDir(subDirs[i-1]))
					dirStack.push_back(subDirs[i-1]);
			}
		}
	}
	return 0;
}





















