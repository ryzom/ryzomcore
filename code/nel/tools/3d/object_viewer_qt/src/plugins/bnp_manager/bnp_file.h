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

#ifndef BNP_FILE_H
#define BNP_FILE_H

// Project includes

// Nel includes
#include "nel/misc/types_nl.h"
#include <nel/misc/singleton.h>

// Qt includes
#include <QString>


namespace BNPManager 
{

struct PackedFile
{
	std::string m_name;
	uint32 m_size;
	uint32 m_pos;
};

typedef std::vector<PackedFile> TPackedFilesList;

class BNPFileHandle
{
	NLMISC_SAFE_SINGLETON_DECL(BNPFileHandle)

	/**
	 * Private constructor
	 */
	BNPFileHandle();

	/**
	 * Private destructor
	 */
	~BNPFileHandle();

public:
	// release memory
	static void releaseInstance();

	/*void append (const QString destFilename, const QString origFilename, uint32 sizeToRead);
	void packRecurse();*/

	/**
	 * Read the header from the bnp file and create a filelist
	 * \param filename (consisting the whole path)
	 */
	bool readHeader (const std::string &filename);

	/**
	 * Append the header to a created bnp file
	 * \param filename (consisting the whole path)
	 */
	void appendHeader (const std::string &filename) {};

	/**
	 * Create a list of all packed files inside the bnp file
	 * \param reference to the list, which has to be filled
	 */
	void list (TPackedFilesList& FileList);
	
	/**
	 * Unpack the selected packed files into user defined dir
	 * \param directory path, where the files should be unpacked
	 * \param list of files, which has to be unpacked
	 */
	bool unpack (const std::string &dirName, const std::vector<std::string>& fileList);

private:

	TPackedFilesList		m_packedFiles;

	// currently opened and displayed bnp file
	std::string				m_activeBNPFile;

	// offset where the header of the bnp file begins
	uint32					m_offsetFromBeginning;

};


}

#endif