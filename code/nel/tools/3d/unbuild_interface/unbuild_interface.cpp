/**
 * \file unbuild_interface.cpp
 * \brief Unbuild Interface
 * \date 2009-03-14 13:25GMT
 * \author Jan Boon (Kaetemi)
 * Unbuild Interface
 */

/* 
 * Copyright (C) 2009  by authors
 * 
 * This file is part of NEVRAX NEL.
 * NEVRAX NEL is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 2 of the License,
 * or (at your option) any later version.
 * 
 * NEVRAX NEL is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NEL; see the file COPYING.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <nel/misc/types_nl.h>

// STL includes
#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <fstream>

// NeL includes
#include <nel/misc/common.h>
#include <nel/misc/file.h>
#include <nel/misc/bitmap.h>
#include <nel/misc/log.h>
#include <nel/misc/path.h>
#include <nel/misc/uv.h>
#include <nel/misc/vector_h.h>

// Project includes
// ...

using namespace std;
using namespace NLMISC;

namespace {

} /* anonymous namespace */

int main(int nNbArg, char **ppArgs)
{
	// create debug stuff
	createDebug();

	// verify all params
	if (nNbArg < 3)
	{
		nlinfo("ERROR : Wrong number of arguments\n");
		nlinfo("USAGE : unbuild_interface  <in_tga_name> <out_path_maps>\n");
		return EXIT_FAILURE;
	}
	string in_tga_name = string(ppArgs[1]);
	if (!CFile::fileExists(in_tga_name))
	{
		nlerrornoex("File %s does not exist", in_tga_name.c_str());
		return EXIT_FAILURE;
	}
	string in_txt_name = in_tga_name.substr(0, in_tga_name.size() - 4) + ".txt";
	if (!CFile::fileExists(in_txt_name))
	{
		nlerrornoex("File %s does not exist", in_txt_name.c_str());
		return EXIT_FAILURE;
	}
	string out_path_maps = ppArgs[2];
	if (!CFile::isDirectory(out_path_maps)) 
	{
		nlerrornoex("Directory %s does not exist", out_path_maps.c_str());
		return EXIT_FAILURE;
	}

	// read in the tga
	CBitmap in_bitmap;
	try
	{
		CIFile in_file;
		in_file.open(in_tga_name);
		in_bitmap.load(in_file);
		in_file.close();
	}
	catch (NLMISC::Exception &e)
	{
		nlerrornoex("(NLMISC::Exception : %s", e.what());
		return EXIT_FAILURE;
	}

	// read maps infos from txt
	try
	{
		ifstream in_file;
		in_file.open(in_txt_name.c_str());
		string line;
		vector<string> line_expl;
		line_expl.reserve(5);
		while (getline(in_file, line))
		{
			// parse from file
			line_expl.clear();
			explode<string>(line, " ", line_expl, false);
			nlassertex((line_expl.size() == 5), ("Bad line entry count"));
			CVectorH vh; // x, y, z, w;
			if (!fromString(line_expl[1], vh.x)
				|| !fromString(line_expl[2], vh.y)
				|| !fromString(line_expl[3], vh.z)
				|| !fromString(line_expl[4], vh.w))
				nlerror("Bad line formatting '%s'", line.c_str());
			vh.x *= (float)in_bitmap.getWidth();
			nlassertex((vh.x == (float)(int)vh.x), ("Bad coordinate"));
			vh.y *= (float)in_bitmap.getHeight();
			nlassertex((vh.y == (float)(int)vh.y), ("Bad coordinate"));
			vh.z *= (float)in_bitmap.getWidth();
			nlassertex((vh.z == (float)(int)vh.z), ("Bad coordinate"));
			vh.w *= (float)in_bitmap.getHeight();
			nlassertex((vh.w == (float)(int)vh.w), ("Bad coordinate"));
			nlinfo("%s %f %f %f %f", line_expl[0].c_str(), vh.x, vh.y, vh.z, vh.w);
			
			// write to tga
			CBitmap out_bitmap;
			out_bitmap.resize((sint32)vh.z - (sint32)vh.x, (sint32)vh.w - (sint32)vh.y, CBitmap::RGBA, true);
			out_bitmap.blit(in_bitmap, (sint)vh.x, (sint)vh.y, (sint)vh.z - (sint)vh.x, (sint)vh.w - (sint)vh.y, 0, 0);
			COFile out_file;
			if (out_file.open(out_path_maps + "/" + line_expl[0]))
			{
				out_bitmap.writeTGA(out_file, 32);
				out_file.close();
			}
			else nlerror("Cannot write tga file '%s/%s'", out_path_maps.c_str(), line_expl[0].c_str());
		}
		in_file.close();
	}
	catch (std::exception &e)
	{
		nlerrornoex("std::exception : %s", e.what());
		return EXIT_FAILURE;
	}
	
	return EXIT_SUCCESS;
}

/* end of file */
