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

#ifndef NL_BIG_FILE_H
#define NL_BIG_FILE_H

#include "types_nl.h"
#include "tds.h"
#include "singleton.h"


namespace NLMISC {

/**
 * Big file management
 * \author Matthieu Besson
 * \author Nevrax France
 * \date 2002
 */

const uint32 BF_ALWAYS_OPENED		=	0x00000001;
const uint32 BF_CACHE_FILE_ON_OPEN	=	0x00000002;

// ***************************************************************************
class CBigFile
{
	NLMISC_SAFE_SINGLETON_DECL(CBigFile);

	CBigFile() {}
	~CBigFile() {}

public:
	// release memory
	static void releaseInstance();

	// Retrieve the global instance
//	static CBigFile &getInstance ();

	// Add a big file to the manager
	bool add (const std::string &sBigFileName, uint32 nOptions);

	// get path of all added bigfiles
	void getBigFilePaths(std::vector<std::string> &bigFilePaths);

	// Remove a big file from the manager
	void remove (const std::string &sBigFileName);

	// true if a bigFile is added
	bool isBigFileAdded(const std::string &sBigFileName) const;

	// return name of Big File
	std::string getBigFileName(const std::string &sBigFileName) const;

	// List all files in a bigfile
	void list (const std::string &sBigFileName, std::vector<std::string> &vAllFiles);

	// Remove all big files added
	void removeAll ();

	/** Signal that the current thread has ended : all file handles "permanently" allocated for that thread
	  * can be released then, preventing them from accumulating.
	  */
	void currentThreadFinished();


	// Used by CIFile to get information about the files within the big file
	FILE* getFile (const std::string &sFileName, uint32 &rFileSize, uint32 &rBigFileOffset,
					bool &rCacheFileOnOpen, bool &rAlwaysOpened);

	// Used by Sound to get information for async loading of mp3 in .bnp. Return false if file not found in registered bnps
	bool getFileInfo (const std::string &sFileName, uint32 &rFileSize, uint32 &rBigFileOffset);

	// Used for CPath only for the moment !
	char *getFileNamePtr(const std::string &sFileName, const std::string &sBigFileName);

// ***************
private:
	class	CThreadFileArray;
	friend class	CThreadFileArray;

	// A ptr to a file.
	struct	CHandleFile
	{
		FILE		*File;
		CHandleFile() : File(NULL) { }
	};

	// A class which return a FILE * handle per Thread.
	class	CThreadFileArray
	{
	public:
		CThreadFileArray();

		// Allocate a FileId for a BNP.
		uint32			allocate();
		// Given a BNP File Id, return its FILE* handle for the current thread.
		CHandleFile		&get(uint32 index);

		void currentThreadFinished();

	private:
		// Do it this way because a few limited TDS is possible (64 on NT4)
		CTDS		_TDS;
		// The array is grow only!!
		uint32		_CurrentId;
	};

	// A BNPFile header
	struct BNPFile
	{
		BNPFile() : Name(NULL), Size(0), Pos(0) { }
		char		*Name;
		uint32		Size;
		uint32		Pos;
	};

	struct CBNPFileComp
	{
		bool operator()(const BNPFile &f, const BNPFile &s )
		{
			return strcmp( f.Name, s.Name ) < 0;
		}
	};

	// A BNP structure
	struct BNP
	{
		BNP() : FileNames(NULL) { }

		// FileName of the BNP. important to open it in getFile() (for other threads or if not always opened).
		std::string						BigFileName;
		// map of files in the BNP.
		char							*FileNames;
		std::vector<BNPFile>			Files;
		// Since many seek may be done on a FILE*, each thread should have its own FILE opened.
		uint32							ThreadFileId;
		bool							CacheFileOnOpen;
		bool							AlwaysOpened;
	};
private:

//	CBigFile(); // Singleton mode -> access it with the getInstance function

//	static CBigFile				*_Singleton;

	// This is an array of CHandleFile, unique to each thread
	CThreadFileArray			_ThreadFileArray;

	std::map<std::string, BNP> _BNPs;

	// common for getFile and getFileInfo
	bool getFileInternal (const std::string &sFileName, BNP *&zeBnp, BNPFile *&zeBnpFile);
};

} // NLMISC


#endif // NL_BIG_FILE_H

/* End of big_file.h */
