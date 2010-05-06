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

#include <nel/misc/path.h>
#include <nel/misc/md5.h>
#include "shapes_exporter.h"

using namespace NLMISC;
using namespace NL3D;

// function to split a string into several substrings delimited by specified characters
void split(const std::string &str, std::vector<std::string> &tokens, const std::string &delimiters)
{
	// Skip delimiters at beginning.
	std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
	// Find first "non-delimiter".
	std::string::size_type pos     = str.find_first_of(delimiters, lastPos);

	while (std::string::npos != pos || std::string::npos != lastPos)
	{
		// Found a token, add it to the vector.
		tokens.push_back(str.substr(lastPos, pos - lastPos));
		// Skip delimiters.  Note the "not_of"
		lastPos = str.find_first_not_of(delimiters, pos);
		// Find next "non-delimiter"
		pos = str.find_first_of(delimiters, lastPos);
	}
}


#if defined(NL_OS_WINDOWS) && !defined(_CONSOLE)
sint WINAPI WinMain(HINSTANCE /* hInstance */, HINSTANCE /* hPrevInstance */, LPSTR cmdline, int /* nCmdShow */)
{
	// we can specify several shapes on command line
	// so we need to process it
	std::vector<std::string> argv;

	// argv[0] is the fullpath to the executable but since it's not used, we can let it empty
	argv.push_back("");

	// split the whole cmdline into args
	split(cmdline, argv, " ");

	// args number
	int argc = (int)argv.size();

#else
sint main(int argc, char **argv)
{
#endif

	NLMISC::CApplicationContext myApplicationContext;

	ShapesExporter exporter;

	exporter.parseConfigFile("shapes_exporter.cfg");
	exporter.init();
	exporter.setupLight();

	if (argc > 1)
	{
		// process all files specified on command line
		for(int i = 1; i < argc; ++i)
		{
			// TODO: create a different directory for each of them
			exporter.exportShape(argv[i], CPath::standardizePath(exporter.settings.output_path));
		}
	}
	else
	{
		std::vector<std::string> filenames;

		// search all .max files
//		CPath::getFileList("max", filenames);
		// search .max files corresponding to a filter
//		CPath::getFileListByName("ps", "braziera", filenames);
//		CPath::getFileListByName("ps", "fireworka", filenames);
//		CPath::getFileListByName("ps", "fireworkf", filenames);
//		CPath::getFileListByName("ps", "burntreeexplo", filenames);
//		CPath::getFileListByName("ps", "dustdoor", filenames); // trop court
//		CPath::getFileListByName("ps", "aura_recept", filenames);
//		CPath::getFileListByName("ps", "bloblight", filenames);
		CPath::getFileList("ps", filenames);
		// search all .shape and .ps files
		std::vector<std::string> shapes;
		CPath::getFileList("shape", shapes);
		CPath::getFileList("ps", shapes);
		for(size_t i = 0; i < filenames.size(); ++i)
		{
			// compute the md5 of .max file
			std::string md5 = getMD5(filenames[i]).toString();
			nlinfo("processing %s with md5 = %s", filenames[i].c_str(), md5.c_str());

			// the final directory with images
			std::string output_path = exporter.settings.output_path + md5.substr(0, 2) + "/" + md5;

			std::vector<std::string> filtered_shapes;

			if(CFile::getExtension(filenames[i]) == "ps")
			{
				// no filter if it's a PS
				filtered_shapes.push_back(filenames[i]);
			}
			else
			{
				// create a temporary list with shapes which could correspond to .max file
				for(size_t j = 0; j < shapes.size(); ++j)
				{
					// only add files with the same beginning
					if (shapes[j].find(CFile::getFilenameWithoutExtension(filenames[i])) == 0)
					{
						filtered_shapes.push_back(shapes[j]);
					}
				}
			}

			// if there is no corresponding file, we can't render it
			if (filtered_shapes.empty())
			{
				nlinfo("no shape found");
				continue;
			}

			// create output directory if it doesn't already exists
			if (!CFile::isExists(output_path) && !CFile::createDirectoryTree(output_path))
			{
				nlwarning("can't create %s", output_path.c_str());
				continue;
			}

			// if we found only one shape, we don't need a skeleton
			if (filtered_shapes.size() == 1)
			{
				if (!exporter.exportShape(filtered_shapes[0], output_path))
				{
					// an error occured, try to delete directory
					nlwarning("can't export shape");
					CFile::deleteDirectory(output_path);
					continue;
				}
			}
			else
			{
				// TODO: search for the right skeleton
				if (!exporter.exportSkeleton("fy_hof_skel.skel", filtered_shapes, output_path))
				{
					// an error occured, try to delete directory
					nlwarning("can't export shape");
					CFile::deleteDirectory(output_path);
					continue;
				}
			}

			if (!exporter.createThumbnail(filenames[i], output_path))
			{
				nlwarning("can't create thumbnail");
			}
		}
	}

	return 0;
}
