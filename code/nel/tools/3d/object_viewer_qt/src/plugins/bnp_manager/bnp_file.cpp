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

PackedFile::PackedFile()
{
	m_size = 0;
	m_pos = 0;
}

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
	FILE *bnp = fopen (m_openedBNPFile.c_str(), "rb");
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
bool BNPFileHandle::readHeader(const std::string &filePath)
{
	m_packedFiles.clear();

	m_openedBNPFile = filePath;

	FILE *f = fopen (filePath.c_str(), "rb");
	if (f == NULL)
	{
		nlwarning("Could not open file!");
		return false;
	}

	nlfseek64 (f, 0, SEEK_END);
	uint32 nFileSize=CFile::getFileSize (filePath );
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
		tmpPackedFile.m_path = m_openedBNPFile;
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
		tmpFile.m_path = it->m_path;
		FileList.push_back(tmpFile);
		it++;
	}
}
// ***************************************************************************
bool BNPFileHandle::writeHeader( const std::string &filePath, uint32 offset )
{
	FILE *f = fopen (filePath.c_str(), "ab");
		if (f == NULL) return false;

		uint32 nNbFile = (uint32)m_packedFiles.size();
		if (fwrite (&nNbFile, sizeof(uint32), 1, f) != 1)
		{
			fclose(f);
			return false;
		}

		for (uint32 i = 0; i < nNbFile; ++i)
		{
			uint8 nStringSize = (uint8)m_packedFiles[i].m_name.size();
			if (fwrite (&nStringSize, 1, 1, f) != 1)
			{
				fclose(f);
				return false;
			}

			if (fwrite (m_packedFiles[i].m_name.c_str(), 1, nStringSize, f) != nStringSize)
			{
				fclose(f);
				return false;
			}

			if (fwrite (&m_packedFiles[i].m_size, sizeof(uint32), 1, f) != 1)
			{
				fclose(f);
				return false;
			}

			if (fwrite (&m_packedFiles[i].m_pos, sizeof(uint32), 1, f) != 1)
			{
				fclose(f);
				return false;
			}
		}

		if (fwrite (&offset, sizeof(uint32), 1, f) != 1)
		{
			fclose(f);
			return false;
		}

		fclose (f);
		return true;
}
// ***************************************************************************
void BNPFileHandle::fileNames(std::vector<std::string> &fileNames)
{
	TPackedFilesList::iterator it = m_packedFiles.begin();
	while (it != m_packedFiles.end() )
	{
		fileNames.push_back(it->m_name);
		it++;
	}
}
// ***************************************************************************
void BNPFileHandle::addFiles( const vector<string> &filePathes)
{
	uint32 OffsetFromBegining = 0;

	// create packed files and add them to the private vector
	vector<string>::const_iterator it_vec = filePathes.begin();
	while (it_vec != filePathes.end() )
	{
		PackedFile tmpFile;
		tmpFile.m_name = CFile::getFilename (*it_vec);
		// Leave position to 0 and set the value during the new bnp file is creating
		// We need the position only for the header at the end
		tmpFile.m_pos = 0;
		tmpFile.m_size = CFile::getFileSize(*it_vec);
		tmpFile.m_path = *it_vec;
		m_packedFiles.push_back( tmpFile );

		it_vec++;
	}

	// sort packed files alphabetic
	std::sort ( m_packedFiles.begin(), m_packedFiles.end(), compare );

	// create a new temporary bnp file with extension *.tmp
	TPackedFilesList::iterator it_packed = m_packedFiles.begin();
	while (it_packed != m_packedFiles.end() )
	{
		append(m_openedBNPFile + ".tmp", *it_packed);
		// Set now the new offset for the new header
		it_packed->m_pos = OffsetFromBegining;
		OffsetFromBegining += it_packed->m_size;

		it_packed++;
	}

	writeHeader(m_openedBNPFile + ".tmp", OffsetFromBegining);
	
	CFile::deleteFile( m_openedBNPFile );
	string src = m_openedBNPFile + ".tmp";
	CFile::moveFile( m_openedBNPFile.c_str(), src.c_str() );
}
// ***************************************************************************
void BNPFileHandle::deleteFiles( const vector<string>& fileNames)
{
	vector<string>::const_iterator it_vec;
	TPackedFilesList::iterator it_packed;
	uint32 OffsetFromBegining = 0;
	string tmpFile = m_openedBNPFile + ".tmp";

	// create a new temporary bnp file with extension *.tmp
	it_packed = m_packedFiles.begin();
	while (it_packed != m_packedFiles.end() )
	{
		// check each packed file if it should be deleted
		it_vec = find (fileNames.begin(), fileNames.end(), it_packed->m_name );
		if ( it_vec != fileNames.end() )
		{
			nlinfo("Deleting file %s.", it_packed->m_name.c_str() );
			it_packed = m_packedFiles.erase(it_packed);
		}
		else
		{
			append(tmpFile, *it_packed);
			// Set now the new offset for the new header
			it_packed->m_pos = OffsetFromBegining;
			OffsetFromBegining += it_packed->m_size;

			it_packed++;
		}
	}
	nldebug("Writing header...");

	writeHeader(tmpFile, OffsetFromBegining);
	
	CFile::deleteFile( m_openedBNPFile );
	string src = m_openedBNPFile + ".tmp";
	CFile::moveFile( m_openedBNPFile.c_str(), src.c_str() );
}
// ***************************************************************************
void BNPFileHandle::append(const string &destination, const PackedFile &source)
{
	// check if the file exists and create one if not
	if ( !CFile::fileExists(destination) )
		CFile::createEmptyFile( destination );

	FILE *bnpfile = fopen(destination.c_str(), "ab");
	FILE *packedfile = fopen(source.m_path.c_str(), "rb");
	if (bnpfile == NULL) return;
	if (packedfile == NULL) { fclose(bnpfile); return; }
	
	uint8 *ptr = new uint8[source.m_size];

	// check if the source is a bnp file.
	if ( nlstricmp( CFile::getExtension(source.m_path), "bnp" ) == 0 )
	{
		// Jump to the file position inside the bnp
		nlfseek64(packedfile, source.m_pos, SEEK_SET);
	}
	// Read the source
	if (fread (ptr, source.m_size, 1, packedfile) != 1)
		nlwarning("%s read error", source.m_path.c_str());

	// Append the data to the destination
	if (fwrite (ptr, source.m_size, 1, bnpfile) != 1)
		nlwarning("%s write error", destination.c_str());

	delete [] ptr;
	
	fclose(packedfile);
	fclose(bnpfile);
}
// ***************************************************************************
bool BNPFileHandle::compare(const PackedFile &left, const PackedFile &right)
{
	return nlstricmp (left.m_name.c_str(), right.m_name.c_str()) < 0;
}
} // namespace BNPManager