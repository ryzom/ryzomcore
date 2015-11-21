// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2015  Winch Gate Property Limited
// Author: Jan Boon <jan.boon@kaetemi.be>
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

#ifndef NL_MESH_UTILS_H
#define NL_MESH_UTILS_H
#include <nel/misc/types_nl.h>

#include <string>

struct CMeshUtilsSettings
{
	CMeshUtilsSettings();

	// Absolute Paths
	std::string SourceFilePath;
	std::string DestinationDirectoryPath;
	std::string ToolDependLog;
	std::string ToolErrorLog;

	// Relative Directories
	/*std::string ShapeDirectory;
	std::string IGDirectory;
	std::string SkelDirectory;*/
};

int exportScene(const CMeshUtilsSettings &settings);

#endif /* NL_MESH_UTILS_H */

/* end of file */
