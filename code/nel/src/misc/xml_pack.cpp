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

#include "stdmisc.h"
#include "nel/misc/xml_pack.h"
#include "nel/misc/file.h"

using namespace std;

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC
{

	NLMISC_SAFE_SINGLETON_IMPL(CXMLPack);


	// For a simple parser, we read by line, with a limit to 1Ko
	const uint32 MaxLineSize = 1*1024;

	/// Consume space and tab characters (but NOT newlines)
	void CXMLPack::skipWS(string::iterator &it, string::iterator end)
	{
		while (it != end && (*it == ' ' || *it == '\t'))
			++it;
	}
	/// Try to match the specified text at current position. Return false of no match
	bool CXMLPack::matchString(string::iterator &it, string::iterator end, const char *text)
	{
		string::iterator rewind = it;
		// skip leading WS
		skipWS(it, end);

		while (it != end && *text && *text == *it)
		{
			++it;
			++text;
		}
		if (*text == 0)
		{
			// we have advanced up to the end of text, so the match is OK
			return true;
		}
		// no match !
		// rewind
		it = rewind;
		return false;
	}

	/// Advance up to the beginning of the next line, incrementing the in/out param lineCount
	void CXMLPack::skipLine(string::iterator &it, string::iterator end, uint32 &lineCount)
	{
		// advance up to end of string or newline char
		while (it != end && *it != '\n')
		{
			++it;
		}
		// skip the new line char
		if (it != end && *it == '\n')
		{
			++it;
			++lineCount;
		}
	}


	// Add an xml pack to the manager
	bool CXMLPack::add (const std::string &xmlPackFileName)
	{
		// prepare the container to store this pack file
		TStringId packId = CStringMapper::map(xmlPackFileName);
		TPackList::iterator packIt(_XMLPacks.find(packId));
		if (packIt != _XMLPacks.end())
		{
			nlwarning("CXMLPack::add : can't add xml_pack file '%s' because already added", xmlPackFileName.c_str());
			return false;
		}
		TXMLPackInfo &packInfo = _XMLPacks[packId];

		// open the xml pack for later access
//		packInfo.FileHandler = fopen(xmlPackFileName.c_str(), "rb");

		// open the xml pack for parsing
		CIFile packFile;
		packFile.open(xmlPackFileName);

		uint32 packSize = packFile.getFileSize();
		string buffer;
		buffer.resize(packSize);

		// read the file in memory for parsing
		packFile.serialBuffer((uint8*)buffer.data(), packSize);

		string::iterator it=buffer.begin(), end(buffer.end());
		uint32 lineCount = 0;

		// check the xml pack header element
		if (!matchString(it, end, "<nel:packed_xml>"))
		{
			nlwarning("Error : invalid pack file '%s', invalid header", xmlPackFileName.c_str());
			return false;
		}
		// advance to next line
		skipLine(it, end, lineCount);

		// now enter the sub file loop
		for(;;)
		{
			TXMLFileInfo fileInfo;
			// match a sub file header
			if (!matchString(it, end, "<nel:xml_file"))
			{
				nlwarning("Error : invalid pack file content at line %u in '%s'", lineCount, xmlPackFileName.c_str());
				return false;
			}

			// ok, extract the file name from the header, match 'name' then '=' then '"'
			if (!matchString(it, end, "name") || !matchString(it, end, "=") || !matchString(it, end, "\""))
			{
				nlwarning("Error : invalid pack file sub header at line %u in '%s', can't found attribute 'name'", lineCount, xmlPackFileName.c_str());
				return false;
			}
			string::iterator nameBegin = it;
			// advance up to closing quote
			while (it != end && *it != '\"')
				++it;
			if (it == end)
			{
				nlwarning("Error : invalid pack file sub header at line %u in '%s', can't found attribute closing quote for name", lineCount, xmlPackFileName.c_str());
				return false;
			}
			string subFileName(buffer, nameBegin-buffer.begin(), it-nameBegin);
			if (subFileName.empty())
			{
				nlwarning("Error : invalid pack file sub header at line %u in '%s', empty filename", lineCount, xmlPackFileName.c_str());
				return false;
			}
			// advance to the closing '>'
			while (it != end && *it != '>')
				++it;
			if (it == end)
			{
				nlwarning("Error : invalid pack file sub header at line %u in '%s', can't found element closing '>'", lineCount, xmlPackFileName.c_str());
				return false;
			}
			// advance to next line (beginning of sub file)
			skipLine(it, end, lineCount);

			string::iterator beginOfFile = it;
			string::iterator endOfFile = it;

			// now, advance up to the end of file
			while (it != end && !matchString(it, end, "</nel:xml_file>"))
			{
				skipLine(it, end, lineCount);
				endOfFile = it;
			}

			// we must not be at end of file
			if (it == end)
			{
				nlwarning("Error : invalid sub file at line %u in '%s', reach end of file without closing file and pack elements", lineCount, xmlPackFileName.c_str());
				return false;
			}

			// ok, the file is parsed, store it
			fileInfo.FileName = CStringMapper::map(subFileName);
			fileInfo.FileOffset = (uint32)(beginOfFile - buffer.begin());
			fileInfo.FileSize = (uint32)(endOfFile - beginOfFile);
//			fileInfo.FileHandler = fopen(xmlPackFileName.c_str(), "rb");
			packInfo._XMLFiles.insert(make_pair(fileInfo.FileName, fileInfo));

			// advance to next line
			skipLine(it, end, lineCount);

			// check for end of pack
			if (matchString(it, end, "</nel:packed_xml>"))
			{
				// ok, the parse is over
				break;
			}

			// continue to next file in pack
		}

		nldebug("XMLPack : xml_pack '%s' added to the collection with %u files", xmlPackFileName.c_str(), packInfo._XMLFiles.size());
		// ok, parsing ended
		return true;
	}

	// List all files in an xml_pack file
	void CXMLPack::list (const std::string &xmlPackFileName, std::vector<std::string> &allFiles)
	{
		TStringId key = CStringMapper::map(xmlPackFileName);

		TPackList::const_iterator it(_XMLPacks.find(key));
		if (it != _XMLPacks.end())
		{
			const TXMLPackInfo &packInfo = it->second;
			// we found it, fill the out vector
			TXMLPackInfo::TFileList::const_iterator first(packInfo._XMLFiles.begin()), last(packInfo._XMLFiles.end());
			for (; first != last; ++first)
			{
				const TXMLFileInfo &fileInfo = first->second;
				allFiles.push_back(CStringMapper::unmap(fileInfo.FileName));
			}
		}
	}

	// Used by CIFile to get information about the files within the xml pack
	FILE* CXMLPack::getFile (const std::string &sFileName, uint32 &rFileSize, uint32 &rFileOffset,
						bool &rCacheFileOnOpen, bool &rAlwaysOpened)
	{
		// split the name appart from the '@@' separator to get the pack file name
		// and subfile name
		vector<string>	parts;
		explode(sFileName, string("@@"), parts, true);
		if (parts.size() != 2)
		{
			nlwarning("CXMLPack::getFile : Can't extract pack and filename from '%s', found %u part instead of 2 when spliting apart from '@@'",
				sFileName.c_str(),
				parts.size());
			return NULL;
		}

		TStringId packId = CStringMapper::map(parts[0]);
		TStringId fileId = CStringMapper::map(parts[1]);

		TPackList::iterator packIt(_XMLPacks.find(packId));
		if (packIt == _XMLPacks.end())
		{
			nlwarning("CXMLPack::getFile : Can't find xml pack file named '%s' to open '%s'", parts[0].c_str(), sFileName.c_str());
			return NULL;
		}
		TXMLPackInfo &packInfo = packIt->second;
		TXMLPackInfo::TFileList::iterator fileIt = packInfo._XMLFiles.find(fileId);
		if (fileIt == packInfo._XMLFiles.end())
		{
			nlwarning("CXMLPack::getFile : Can't find xml file named '%s' in pack '%s'",
				parts[1].c_str(), parts[0].c_str());
			return NULL;
		}

		// ok, we have found it !
		TXMLFileInfo &fileInfo = fileIt->second;

		// fill the return value
		rFileSize = fileInfo.FileSize;
		rFileOffset = fileInfo.FileOffset;
		rCacheFileOnOpen = false;
		rAlwaysOpened = false;
		FILE *fp = fopen(parts[0].c_str(), "rb");
		return fp;
	}


} // namespace NLMISC

