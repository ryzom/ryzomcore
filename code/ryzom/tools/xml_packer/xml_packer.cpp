

#include "nel/misc/types_nl.h"
#include "nel/misc/app_context.h"
#include "nel/misc/path.h"
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

const string ExcludeFiles(".#*;*.log;*.bin");
const string ExcludeDirs("CVS");

bool isExcludedFile(const std::string &fileName)
{
	static vector<string> excludeFileVect;
	static bool init = false;
	
	if (!init)
	{
		explode(ExcludeFiles, ";", excludeFileVect, true);
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
		explode(ExcludeDirs, ";", excludeDirVect, true);
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
	printf("XML Packer/Unpacker V0.1\n(C) Nevrax 2006\n");
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
		printf("Usage : %s -u|-p [-r] [-f <filename>]\n", argv[0]);
		printf("  -p : pack the current folder\n");
		printf("  -u : unpack the current folder\n");
		printf("  -r : pack or unpack subdirectories recursively\n");
		//		printf("  -f <filename> : use the specified filename instead of current directory name\n");
		
		return -1;
	}
	
	vector<string>	dirStack;
	
	printf("Current patch is '%s'\n", CPath::getCurrentPath().c_str());
	
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

				// get the current directory content
				vector<string> files;
				CPath::getPathContent(dirName, false, false, true, files);

				vector<string>	validFiles;
				
				// first loop to build the list of valid file
				for (uint i=0; i<files.size(); ++i)
				{
					if (files[i].find(DefaultExt) != string::npos)
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
					uint32 packDate = CFile::getFileModificationDate(packFileName);
					// loop to check for file time stamp
					for (uint i=0; i<validFiles.size(); ++i)
					{
						uint32 fileDate = CFile::getFileModificationDate(validFiles[i]);

						if (fileDate >= packDate)
							// no more to check
							break;
					}

					// all files are older than the pack file ! no repack needed
					needRepack = false;
				}
				
				if (needRepack)
				{
					// open the pack file
					//				FILE *fp = fopen(filename.c_str(), "wt");
					FILE *fp = fopen(packFileName.c_str(), "wt");
					
					fprintf(fp, "<packed_xml>\n");
					
					for (uint i=0; i<validFiles.size(); ++i)
					{
						string &subFileName = validFiles[i];
						
						printf("Adding file '%s'...\n", CFile::getFilename(subFileName).c_str());
						fprintf(fp, "	<xml_file name=\"%s\">\n", CFile::getFilename(subFileName).c_str());
						
						FILE *subFp = fopen(subFileName.c_str(), "rt");
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
							char *finalReturn = "\n";
							fputs(finalReturn, fp);
						}
						
						fclose(subFp);
						
						fprintf(fp, "	</nel:xml_file>\n");
					}
					
					fprintf(fp, "</nel:packed_xml>\n");
					
					fclose(fp);
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
//				FILE *fp = fopen((dirName+"/tmp."+DefaultExt).c_str(), "rt");
				FILE *fp = fopen(filename.c_str(), "rt");
				nlassert(fp != NULL);
				uint linecount = 0;
				
				// read the first line
				char buffer[MaxLineSize];
				fgets(buffer, MaxLineSize, fp);
				linecount++;
				if (strcmp(buffer, "<nel:packed_xml>\n") != 0)
				{
					printf ("Error : invalid pack file '%s'\n", filename.c_str());
					exit(-1);
				}
				
				char *result = NULL;
				do 
				{
					// read a file line
					fgets(buffer, MaxLineSize, fp);
					CSString parser(buffer);
					linecount++;
					if (parser.find("	<nel:xml_file name=") != 0)
					{
						if (parser.find("</nel:packed_xml>") == 0)
						{
							// end of pack file
							fclose(fp);
							break;
						}
						printf ("Error : invalid pack file '%s' at line %u", filename.c_str(), linecount);
						exit(-1);
					}
					
					CSString subFileName = parser.leftCrop(sizeof("	<nel:xml_file name=")-1);
					subFileName = subFileName.matchDelimiters(false, false, true, false);
					subFileName = subFileName.unquoteIfQuoted();
					subFileName = dirName+"/"+subFileName;
					
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
			//			break;
	}
	
	if (recursive)
	{
		vector<string> subDirs;
		CPath::getPathContent(dirName, false, true, false, subDirs);
		
		// filter the directories
		for (uint i=subDirs.size(); i>0; --i)
		{
			if (!isExcludedDir(subDirs[i-1]))
				dirStack.push_back(subDirs[i-1]);
		}
	}
}
return 0;
}





















