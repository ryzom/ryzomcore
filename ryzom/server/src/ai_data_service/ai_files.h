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



#ifndef RYAI_AI_FILES_H
#define RYAI_AI_FILES_H

#include "nel/misc/types_nl.h"
#include "nel/misc/sheet_id.h"
#include "nel/misc/entity_id.h"

#include "nel/net/transport_class.h"

#include <map>
#include <string>

class CAIFiles
{
public:
	// clear out the data strustures and rescan the directories listed in the config
	// file for source and object files
	static void scan();

	// delete the object files (and temporary files if any) for a given manager
	static void clean(sint mgrId);

	// source file name, extension, etc
	static std::vector<std::string> srcPaths();		// the paths for source files
	static std::string srcExtension();				// the standard src extension
	static std::string srcName(sint mgrId);			// without path or extension
	static std::string srcFileName(sint mgrId);		// without path
	static std::string fullSrcName(sint mgrId);		// without extension
	static std::string fullSrcFileName(sint mgrId);	// with path and extension

	// object file name, extension, etc
	static std::string objPath();					// the path for obj files
	static std::string objExtension();				// the standard obj file extension
	static std::string objName(sint mgrId);			// without path or extension
	static std::string objFileName(sint mgrId);		// without extension
	static std::string fullObjFileName(sint mgrId);	// with path and extension

	// saved data file name, extension, etc
	static std::string savPath();					// the path for sav files
	static std::string savExtension();				// the standard sav file extension
	static std::string savName(sint mgrId);			// without path or extension
	static std::string savFileName(sint mgrId);		// without extension
	static std::string fullSavFileName(sint mgrId);	// with path and extension

	// Reading & writing object files
	static void CAIFiles::writeObjFile(sint mgrId);	

private:
	static void CAIFiles::_addSrcFile(std::string fileName,std::string fullFileName, uint timestamp);
	static void CAIFiles::_addObjFile(sint mgrId,std::string fullFileName, uint timestamp);
	static void CAIFiles::_addSavFile(sint mgrId,std::string fullFileName);

	static void CAIFiles::_scanSrcFiles(const std::string &path);
	static void CAIFiles::_scanObjAndSavFiles();
};

#endif