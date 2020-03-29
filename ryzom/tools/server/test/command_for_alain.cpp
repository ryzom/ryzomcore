/* 
	command for alain

	project: RYZOM / TEST
*/

#include "nel/misc/file.h"
#include "nel/misc/path.h"
#include "nel/misc/o_xml.h"
#include "nel/misc/command.h"
#include "game_share/file_description_container.h"
#include "game_share/singleton_registry.h"

using namespace std;
using namespace NLMISC;

NLMISC_COMMAND(buildBannedNameFromFile,"","<input file>")
{
	if (args.size()!=1)
		return false;

	std::map<string,uint32> result;

	FILE *inf=fopen(args[0].c_str(),"rb");
	if (inf==NULL)
	{
		nlwarning("Failed to open file: %s",args[0].c_str());
		return false;
	}

	CSString input;
	input.resize(CFile::getFileSize(inf));
	uint32 bytesRead= fread(&input[0],1,input.size(),inf);
	fclose(inf);
	if (bytesRead!=input.size())
	{
		nlwarning("Failed to read contents of file: %s",args[0].c_str());
		return false;
	}

	while(!input.empty())
	{
		CSString line=input.firstLine(true);
		if ()
			continue;
		CSString account= line.first
		CSString 
	}

	const char* outputName= "c:/banned_names.xml";
	nlinfo("Writing output: %s",outputName);
	try
	{
		COFile f;
		f.open(outputName);
		COXml fx;
		fx.init(&f,"1.0");
		fx.serialCont(result);
		fx.flush();
		f.close();
	}
	catch(...)
	{
		nlinfo("Problem with output file: %s",outputName);
	}
	
	return true;
}

NLMISC_COMMAND(buildReservedNameListFromFile,"","<input file>")
{
	if (args.size()!=1)
		return false;

	std::map<string,uint32> result;


	const char* outputName= "c:/reserved_names.xml";
	nlinfo("Writing output: %s",outputName);
	try
	{
		COFile f;
		f.open(outputName);
		COXml fx;
		fx.init(&f,"1.0");
		fx.serialCont(result);
		fx.flush();
		f.close();
	}
	catch(...)
	{
		nlinfo("Problem with output file: %s",outputName);
	}
	
	return true;
}

NLMISC_COMMAND(buildReservedNameListFromBinFiles,"","<path>[<path>...]")
{
	if (args.size()<1)
		return false;

	CFileDescriptionContainer fdc;
	for (uint32 j=0;j<args.size();++j)
		fdc.addFileSpec(args[j]);

	typedef std::map<std::string,CFileDescription const*> TMyMap;
	TMyMap myMap;

	for (uint32 i=0;i<fdc.size();++i)
	{
		std::string fileName=NLMISC::CFile::getFilename(fdc[i].FileName);
		if (myMap.find(fileName)!=myMap.end())
		{
			if (myMap[fileName]->FileTimeStamp>fdc[i].FileTimeStamp)
				continue;
		}
		myMap[fileName]= &fdc[i];
	}
	
	std::map<ucstring,uint32> result;
	uint32 lastAccount=~0u;
	for (TMyMap::iterator it=myMap.begin();it!=myMap.end();++it)
	{
		CSString s= (*it).first;
		s.strtok("_");
		uint32 account=s.strtok("_").atoi();

		if (account!=lastAccount)
		{
			lastAccount=account;

			try
			{
				CIFile f;
				ucstring ucs;
				f.open((*it).second->FileName);
				f.seek(18,NLMISC::IStream::begin);
				f.serial(ucs);
				f.close();
				result[ucs]=account;
				nlinfo("%s: name: %s",(*it).second->FileName.c_str(),ucs.toUtf8().c_str());
			}
			catch(...)
			{
				nlinfo("Problem with: %s",(*it).second->FileName.c_str());
			}
		}
	}

	const char* outputName= "c:/reserved_names_from_bin_files.xml";
	nlinfo("Writing output: %s",outputName);
	try
	{
		COFile f;
		f.open(outputName);
		COXml fx;
		fx.init(&f,"1.0");
		fx.serialCont(result);
		fx.flush();
		f.close();
	}
	catch(...)
	{
		nlinfo("Problem with output file: %s",outputName);
	}
	
	return true;
}
