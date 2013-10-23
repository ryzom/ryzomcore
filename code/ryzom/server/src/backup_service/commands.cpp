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

#include "nel/misc/types_nl.h"
#include "nel/misc/command.h"
#include "nel/misc/path.h"
#include "nel/misc/file.h"
#include "nel/misc/md5.h"

#include "backup_service.h"
#include "game_share/persistent_data.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;


CDirectoryRateStat	DirStats;

extern CVariable<string>	SaveShardRoot;


NLMISC_COMMAND(displayFileStats, "display file read/write stats for the last minute", "")
{
	DirStats.display(log);
	return true;
}

NLMISC_DYNVARIABLE(string, FileReadStat, "mean file read in bytes per seconds for the last minute")
{
	if (get)
		*pointer = 	bytesToHumanReadable(DirStats.getMeanReadRate());
	return;
}

NLMISC_DYNVARIABLE(string, FileWriteStat, "mean file write in bytes per seconds for the last minute")
{
	if (get)
		*pointer = 	bytesToHumanReadable(DirStats.getMeanWriteRate());
	return;
}


NLMISC_COMMAND ( resumeBackupService, "reset stall mode for resume backup process", "no params")
{
	CBackupService::getInstance()->setStall( false );
	CMessage msgOut("RESUME_SHARD");
	CUnifiedNetwork::getInstance()->send("EGS", msgOut );
	return true;
}

NLMISC_COMMAND ( stallBackupService, "set backup service un stall mode", "no params")
{
	CBackupService::getInstance()->stallShard( std::string("Stalled by admin command") );
	return true;
}

NLMISC_COMMAND ( dumpCharacterFile, "dump the content of the save file for a character. Invoke with no param to display a list of available shardId.", "<shardId> <playerID> <slotNum> [<onlyThisTag>|listTokens]")
{
	if (args.size() == 0)
	{
		// just output the list of available shard id
		vector<string>	shards;
		CPath::getPathContent(SaveShardRoot, false, true, false, shards);

		log.displayNL("Listing %u available shard id in path '%s':", shards.size(), SaveShardRoot.c_str());
		for (uint i=0; i<shards.size(); ++i)
		{
			string id = shards[i];
			id = CPath::standardizePath(id, false);
			if (!id.empty() && (id[id.size()-1] == '\\' || id[id.size()-1] == '/'))
				id.resize(id.size()-1);
			id = CFile::getFilename(id);
			
			log.displayNL("  %s", id.c_str());
		}
		log.displayNL("End of list.");

		// build a list

		return true;
	}
	if (args.size() < 3)
		return false;

	bool useFilter = false;
	string filterIn;
	string filterOut;

	if (args.size() == 4)
	{
		filterIn = string("<")+args[3];
		filterOut = string("</")+args[3];
		useFilter = true;
	}

	string fileName = SaveShardRoot.toString()+"/"+args[0]+"/characters/account_"+args[1]+"_"+args[2]+"_pdr.bin";
	if (!CFile::isExists(fileName))
	{
		log.displayNL("The file '%s' (located here '%s') cannot be found in '%s' backup directory", 
			CFile::getFilename(fileName).c_str(), 
			fileName.c_str(),
			args[0].c_str());
		return true;
	}

	// the file exist !
	static CPersistentDataRecord	pdr;
	pdr.clear();

	if (!pdr.readFromBinFile(fileName.c_str()))
	{
		log.displayNL("Error while reading file '%s'", fileName.c_str());
		return true;
	}
	
	string xml;
	if (!pdr.toString(xml))
	{
		log.displayNL("Error while converting file '%s' to XML", fileName.c_str());
		return true;
	}

	if (useFilter && args[3] == "listTokens")
	{
		// build a list of tokens and output it
		set<string> tokens;

		string::size_type pos = 0;
		pos = xml.find("<");
		while (pos != string::npos)
		{
			++pos;
			string token;
			while (xml[pos] != ' ' && xml[pos] != '\t' && xml[pos] != '/' && xml[pos] != '>')
				token += xml[pos++];

			tokens.insert(token);

			pos = xml.find("<", pos);
		}
		
		// output the result
		log.displayNL("Displaying %u tokens in characters file :", tokens.size());
		string line;
		set<string>::iterator first(tokens.begin()), last(tokens.end());
		for (; first != last; ++first)
		{
			line += *first+" ";

			if (line.size() > 80)
			{
				log.displayNL("%s", line.c_str());
				line = "";
			}
		}

		if (!line.empty())
			log.displayNL("%s", line.c_str());

		log.displayNL("End of token list");

		return true;
	}

	// ok, now display the data
	vector<string> lines;
	explode(xml, string("\n"), lines);

	bool logOn = !useFilter;

	for (uint i=0; i<lines.size(); ++i)
	{
		// check filters
		if (useFilter)
			if (!logOn && lines[i].find(filterIn) != string::npos)
				logOn = true;

		if (logOn)
			log.displayNL("%s", lines[i].c_str());

		// check filter
		if (useFilter)
		{
			if (logOn 
				&& (lines[i].find(filterOut) != string::npos 
				|| (lines[i].find(filterIn) != string::npos 
					&& lines[i].find("/>") != string::npos)
				))
					logOn = false;
		}
	}

	return true;
}

static char		base64Table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static uint8	base64Revert[256];
static bool		base64Init = false;

static void	base64Encode(const std::vector<uint8>& buffer, std::string& encoded)
{
	encoded.clear();
	encoded.reserve(buffer.size()*4/3+3);

	uint	i;
	for (i=0; i<buffer.size(); )
	{
		uint32	b = 0;

		b |= (buffer[i++] << 16);
		encoded += base64Table[(b >> 18) & 0x3f];

		if (i < buffer.size())
		{
			b |= (buffer[i++] << 8);
			encoded += base64Table[(b >> 12) & 0x3f];

			if (i < buffer.size())
			{
				b |= buffer[i++];
				encoded += base64Table[(b >> 6) & 0x3f];
				encoded += base64Table[b & 0x3f];
			}
			else
			{
				encoded += base64Table[(b >> 6) & 0x3f];
				encoded += '=';
			}
		}
		else
		{
			encoded += base64Table[(b >> 12) & 0x3f];
			encoded += '=';
			encoded += '=';
		}

	}
}

static void	base64Decode(std::vector<uint8>& buffer, const std::string& encoded)
{
	uint	i;

	if (!base64Init)
	{
		for (i=0; i<256; ++i)
			base64Revert[i] = 255;
		for (i=0; i<sizeof(base64Table); ++i)
			base64Revert[base64Table[i]] = i;
	}

	uint	sz = (uint)encoded.size();
	uint	inbits = 0;
	uint	bitbuffer = 0;
	for (i=0; i<sz; ++i)
	{
		// padding? -> leave
		if (encoded[i] == '=')
			break;

		// unknown char? -> skip
		if (base64Revert[(uint8)encoded[i]] == 255)
			continue;

		bitbuffer = (bitbuffer << 6) | base64Revert[(uint8)encoded[i]];
		inbits += 6;

		if (inbits >= 8)
		{
			buffer.push_back((uint8)(bitbuffer >> (inbits-8)));
			inbits -= 8;
		}
	}
}




NLMISC_COMMAND ( stallShard, "stall backup service and connected shards (via EGS)", "")
{
	CBackupService::getInstance()->FileManager.setMode(CFileAccessManager::Stalled, "MANUAL BS STALL");
	return true;
}

NLMISC_COMMAND ( resumeShard, "resume backup service and connected shards (via EGS)", "")
{
	CBackupService::getInstance()->FileManager.setMode(CFileAccessManager::Normal, "MANUAL BS RESUME");
	return true;
}

NLMISC_COMMAND ( removeFileAccess, "remove a file access from file access manager (see displayFileAccesses)", "<hexa pointer>")
{
	if (args.size() != 1)
		return false;

	IFileAccess*	access = NULL;
	sscanf(args[0].c_str(), "%p", &access);

	CBackupService::getInstance()->FileManager.removeFileAccess(access);
	return true;
}

NLMISC_COMMAND ( displayFileAccesses, "display stacked file accesses", "")
{
	CBackupService::getInstance()->FileManager.displayFileAccesses(log);
	return true;
}






NLMISC_COMMAND (getFileBase64Content, "dump file content in Base64 encoded form", "<file>")
{
	if (args.size() != 1)
	{
		log.displayRawNL("file INVALIDFILE lines 0 size 0");
		return true;
	}

	CIFile	f;
	if (!CFile::fileExists(args[0]) || CFile::getFileSize(args[0]) == 0 || !f.open(args[0]))
	{
		log.displayRawNL("file %s lines 0 size 0", args[0].c_str());
		return true;
	}

	uint	filesize = f.getFileSize();

	std::vector<uint8>	buffer;
	buffer.resize(filesize);
	f.serialBuffer(&(buffer[0]), filesize);
	f.close();

	CHashKeyMD5	key = getMD5(&(buffer[0]), filesize);

	std::string			encoded;
	base64Encode(buffer, encoded);

	uint	numcharperline = 224;
	uint	numlines = ((uint)encoded.size()+numcharperline-1)/numcharperline;

	log.displayRawNL("file %s lines %d size %d haskey %s", args[0].c_str(), numlines, filesize, key.toString().c_str());

	uint	cpos = 0;
	while (cpos < encoded.size())
	{
		std::string	line = encoded.substr(cpos, numcharperline);
		log.displayRawNL("%s", line.c_str());
		cpos += numcharperline;
	}

	return true;
}

NLMISC_COMMAND (putFileBase64Content, "fill file with content in Base64 encoded form", "<file> <base64content>")
{
	if (args.size() < 2)
		return false;

	COFile	f;
	if (!f.open(args[0]))
		return false;

	std::string	encoded;
	uint	i;
	for (i=1; i<args.size(); ++i)
		encoded += args[i];

	std::vector<uint8>	buffer;
	base64Decode(buffer, encoded);

	f.serialBuffer(&(buffer[0]), (uint)buffer.size());
	f.close();

	return true;
}
