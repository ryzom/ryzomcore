// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2015  Winch Gate Property Limited
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
//
// Author: Jan BOON (Kaetemi) <jan.boon@kaetemi.be>

#include <nel/misc/types_nl.h>
#include "../mesh_utils/mesh_utils.h"

#include <nel/misc/cmd_args.h>
#include <nel/misc/path.h>

#include <nel/3d/register_3d.h>
#include <nel/3d/scene.h>

int main(int argc, char *argv[])
{
	NLMISC::CApplicationContext app;

	NLMISC::CCmdArgs args;

	args.addArg("d", "dst", "destination", "Destination directory path");
	args.addArg("", "coarse-dst", "destination", "With-coarse-mesh shape destination directory (nel-extras glTF import; defaults to dst)");
	args.addArg("", "ig-dst", "destination", "Instance-group (.ig) destination directory (nel-extras glTF import; defaults to dst)");
	args.addArg("", "anim-dst", "destination", "Animation (.anim) destination directory (nel-extras glTF import; defaults to dst)");
	args.addArg("", "zone-dst", "destination", "Ligo zone destination root (zones/ + zoneligos/ under it; nel-extras glTF import; defaults to dst)");
	args.addArg("m", "materials", "sidecar", "Materials sidecar .gltf (assimp route; nel_* materials bound by name; repeatable, later files override earlier — shared library first, per-asset second)", false);
	args.addArg("", "dependlog", "log", "Dependencies log path");
	args.addArg("", "errorlog", "log", "Errors log path");
	args.addArg("", "no-nel-extras", "", "Ignore nel_* per-node glTF extras; use Decompose(mTransformation) instead. Validation flag — see wiki: nel_gltf_extras.md.");
	args.addAdditionalArg("input", "Filename of 3D model to convert", false);

	if (!args.parse(argc, argv)) return EXIT_SUCCESS;

	const std::vector<std::string> &filePathes = args.getAdditionalArg("input");

	NL3D::CScene::registerBasics();
	NL3D::registerSerial3d();

	sint res = 0;

	for(uint i = 0; i < filePathes.size(); ++i)
	{
		std::string filePath = filePathes[i];

		if (!NLMISC::CFile::fileExists(filePath))
		{
			nlerror("File '%s' does not exist", filePath.c_str());
			return EXIT_FAILURE;
		}

		CMeshUtilsSettings settings;
		settings.SourceFilePath = filePath;

		if (args.haveArg("d"))
			settings.DestinationDirectoryPath = args.getArg("d").front();

		if (settings.DestinationDirectoryPath.empty())
			settings.DestinationDirectoryPath = filePath + "_export";

		settings.DestinationDirectoryPath = NLMISC::CPath::standardizePath(settings.DestinationDirectoryPath);

		if (args.haveLongArg("coarse-dst"))
			settings.ShapeCoarseDirectoryPath = NLMISC::CPath::standardizePath(args.getLongArg("coarse-dst").front());

		if (args.haveLongArg("ig-dst"))
			settings.IGDirectoryPath = NLMISC::CPath::standardizePath(args.getLongArg("ig-dst").front());

		if (args.haveLongArg("anim-dst"))
			settings.AnimDirectoryPath = NLMISC::CPath::standardizePath(args.getLongArg("anim-dst").front());

		if (args.haveLongArg("zone-dst"))
			settings.ZoneDirectoryPath = NLMISC::CPath::standardizePath(args.getLongArg("zone-dst").front());

		if (args.haveArg("m"))
			settings.MaterialSidecarPaths = args.getArg("m");

		if (args.haveLongArg("dependlog"))
			settings.ToolDependLog = args.getLongArg("dependlog").front();

		if (settings.ToolDependLog.empty())
			settings.ToolDependLog = settings.DestinationDirectoryPath + "depend.log";

		if (args.haveLongArg("errorlog"))
			settings.ToolErrorLog = args.getLongArg("errorlog").front();

		if (settings.ToolErrorLog.empty())
			settings.ToolErrorLog = settings.DestinationDirectoryPath + "error.log";

		if (args.haveLongArg("no-nel-extras"))
			settings.IgnoreNelExtras = true;

		res = exportScene(settings);

		if (res != EXIT_SUCCESS) break;
	}

	return res;
}

/* end of file */
