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

#ifndef __TOOLS_H__
#define __TOOLS_H__

#include "nel/misc/types_nl.h"
#include <string>
#include <vector>

class CTools
{

public:
	
	// Make directory even if not the whole path is not created
	// Example : c:\temp is created and we call mkdir("c:\temp\aze\qsd")
	// The function create c:\temp\aze and then c:\temp\aze\qsd
	static void mkdir (const std::string &FullDirName);

	// Change the directory. We can give directory relative to the current one
	static void chdir (const std::string &dirName);

	// Get the current directory
	static std::string pwd ();

	// Convert a relative and not formated path to an absolute one
	static std::string normalizePath (const std::string &path);

	// Return true if file exists
	static bool fileExist (const std::string &sFileName);

	// Return -1 if file1.date < file2.date (file1 is older than file2)
	// Return  0 if file1.date = file2.date (file1 and file2 has the same date)
	// Return  1 if file1.date > file2.date (file1 is newer than file2)
	static int fileDateCmp (const std::string &file1, const std::string &file2);

	// Return -1 if file1.date < (nDateLow+(nDateHigh<<32)) (file1 is older than nDate)
	// Return  0 if file1.date = (nDateLow+(nDateHigh<<32)) (file1 and nDate has the same date)
	// Return  1 if file1.date > (nDateLow+(nDateHigh<<32)) (file1 is newer than nDate)
	static int fileDateCmp (const std::string &file1, uint32 nDateLow, uint32 nDateHigh);

	// Return in sAllFiles all the files listed in the current directory
	// bFullPath indicates if the returned names are fully qualified
	static void dir (const std::string &sFilter, std::vector<std::string> &sAllFiles, bool bFullPath);

	// Return in sAllFiles all the files listed in the current directory and subdirectories
	// bFullPath indicates if the returned names are fully qualified
	static void dirSub (const std::string &sFilter, std::vector<std::string> &sAllFiles, bool bFullPath);

	// Copy a file
	static void copy (const std::string &DstFile, const std::string &SrcFile);

private:

	// Throw the last error
	static void throwError (const char *message);

	// Used by dirSub
	static void dirSubRecurse (const std::string &sFilter, std::vector<std::string> &sAllFiles, bool bFullPath);

};

#endif // __TOOLS_H__