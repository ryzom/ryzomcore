// Object Viewer Qt - BNP Manager Plugin - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2011  Roland Winklmeier <roland.m.winklmeier@googlemail.com>
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

// Project includes
#include "bnp_file.h"

// Nel includes
#include <nel/misc/debug.h>
#include <nel/misc/file.h>
#include <nel/misc/path.h>
#include <nel/misc/algo.h>
#include <nel/misc/common.h>

// Qt includes

using namespace NLMISC;
using namespace std;


namespace BNPManager
{

NLMISC_SAFE_SINGLETON_IMPL(BNPFileHandle);

BNPFileHandle::BNPFileHandle()
{
	m_offsetFromBeginning = 0;
}
// ***************************************************************************
BNPFileHandle::~BNPFileHandle()
{
	// Erase the list
	m_packedFiles.clear();
}
// ***************************************************************************
void BNPFileHandle::releaseInstance()
{
	if (_Instance)
	{
		NLMISC::INelContext::getInstance().releaseSingletonPointer("BNPFileHandle", _Instance);
		delete _Instance;
		_Instance = NULL;
	}
}
// ***************************************************************************
bool BNPFileHandle::unpack(const string &dirName, const vector<string>& fileList)
{
	FILE *bnp = fopen (m_activeBNPFile.c_str(), "rb");
	FILE *out;
	if (bnp == NULL)
		return false;

	TPackedFilesList::iterator it_files = m_packedFiles.begin();

	for (it_files; it_files != m_packedFiles.end(); it_files++)
	{
		// Check if the file should be unpacked or not
		if (find(fileList.begin(), fileList.end(), it_files->m_name) != fileList.end())
		{
			string filename = dirName + "/" + it_files->m_name;

			out = fopen (filename.c_str(), "wb");
			if (out != NULL)
			{
				nlfseek64 (bnp, it_files->m_pos, SEEK_SET);
				uint8 *ptr = new uint8[it_files->m_size];
				if (fread (ptr, it_files->m_size, 1, bnp) != 1)
				{
					nlwarning("%s read error", filename.c_str());
					return false;
				}
				if (fwrite (ptr, it_files->m_size, 1, out) != 1)
				{
					nlwarning("%s write error", filename.c_str());
					return false;
				}
				fclose (out);
				delete [] ptr;
			}
		}
	}
	fclose (bnp);
	return true;
}
// ***************************************************************************
// Read the header from a big file
bool BNPFileHandle::readHeader(const std::string &filename)
{
	m_packedFiles.clear();

	m_activeBNPFile = filename;

	FILE *f = fopen (filename.c_str(), "rb");
	if (f == NULL)
	{
		nlwarning("Could not open file!");
		return false;
	}

	nlfseek64 (f, 0, SEEK_END);
	uint32 nFileSize=CFile::getFileSize (filename );
	nlfseek64 (f, nFileSize-sizeof(uint32), SEEK_SET);

	uint32 nOffsetFromBegining;

	if (fread (&nOffsetFromBegining, sizeof(uint32), 1, f) != 1)
	{
		fclose (f);
		return false;
	}
#ifdef NL_BIG_ENDIAN
	NLMISC_BSWAP32(nOffsetFromBegining);
#endif
	
	if (nlfseek64 (f, nOffsetFromBegining, SEEK_SET) != 0)
	{
		nlwarning("Could not read offset from begining");
		fclose (f);
		return false;
	}

	uint32 nNbFile;
	if (fread (&nNbFile, sizeof(uint32), 1, f) != 1)
	{
		nlwarning("Could not read number of files!");
		fclose (f);
		return false;
	}

#ifdef NL_BIG_ENDIAN
	NLMISC_BSWAP32(nNbFile);
#endif

	for (uint32 i = 0; i < nNbFile; ++i)
	{
		uint8 nStringSize;
		char sName[256];
		if (fread (&nStringSize, 1, 1, f) != 1)
		{
			nlwarning("Error reading packed filename!");
			fclose (f);
			return false;
		}
		if (fread (sName, 1, nStringSize, f) != nStringSize)
		{
			fclose (f);
				return false;
		}
		sName[nStringSize] = 0;
		PackedFile tmpPackedFile;
		tmpPackedFile.m_name = sName;
		if (fread (&tmpPackedFile.m_size, sizeof(uint32), 1, f) != 1)
		{
			nlwarning("Error reading packed file size!");
			fclose (f);
			return false;
		}
#ifdef NL_BIG_ENDIAN
		NLMISC_BSWAP32(tmpBNPFile.Size);
#endif
		if (fread (&tmpPackedFile.m_pos, sizeof(uint32), 1, f) != 1)
		{
			nlwarning("Error reading packed file position!");
			fclose (f);
			return false;
		}
#ifdef NL_BIG_ENDIAN
		NLMISC_BSWAP32(tmpBNPFile.Pos);
#endif
		m_packedFiles.push_back (tmpPackedFile);
	}

	fclose (f);
	return true;
}
// ***************************************************************************
void BNPFileHandle::list(TPackedFilesList& FileList)
{
	PackedFile tmpFile;
	TPackedFilesList::iterator it = m_packedFiles.begin();
	while (it != m_packedFiles.end() )
	{
		tmpFile.m_name = it->m_name;
		tmpFile.m_pos = it->m_pos;
		tmpFile.m_size = it->m_size;
		FileList.push_back(tmpFile);
		it++;
	}
}
// ***************************************************************************
} // namespace BNPManager