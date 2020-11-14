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

#include "nel/misc/file.h"
#include "nel/misc/bitmap.h"
#include "nel/misc/path.h"
#include "nel/misc/debug.h"

#include <iostream>

void writeInstructions()
{
	std::cout << "Syntax: textures_optimizer [-a] [-g] <input>" << std::endl;
	std::cout << std::endl;
	std::cout << "  Try to optimize TGA or PNG textures by removing useless alpha channel or converting a RGB with black and white values to grayscale" << std::endl;
	std::cout << "  By default, it only make checks and display if texture can optimized or not" << std::endl;
	std::cout << std::endl;
	std::cout << "with" << std::endl;
	std::cout << "-a : Remove alpha channel if useless (255)" << std::endl;
	std::cout << "-g : Convert to grayscale if all pixels are gray" << std::endl;
	std::cout << "-t : Apply texture optimizations (same as -a -g)" << std::endl;
	std::cout << "-m : Apply mask optimizations (convert to grayscale using red value and remove alpha)" << std::endl;
	std::cout << std::endl;
	std::cout << "-h or -? for this help" << std::endl;
	std::cout << std::endl;
}

bool FixAlpha = false;
bool FixGrayscale = false;
bool TextureOptimizations = false;
bool MaskOptimizations = false;

std::vector<std::string> InputFilenames;

bool parseOptions(int argc, char **argv)
{
	// process each argument
	for(sint i = 1; i < argc; ++i)
	{
		std::string option = argv[i];

		if (option.length() > 0)
		{
			bool isOption = option[0] == '-';

#ifdef NL_OS_WINDOWS
			// authorize / for options only under Windows,
			// because under Linux it could be a full path
			if (!isOption) isOption = (option[0] == '/');
#endif

			// Option
			if (isOption)
			{
				// remove option prefix
				option = option.substr(1);
				
				// Fix alpha
				if (option == "a")
				{
					FixAlpha = true;
				}
				// Fix grayscale
				else if (option == "g")
				{
					FixGrayscale = true;
				}
				// Texture optimizations
				else if (option == "t")
				{
					TextureOptimizations = true;
					FixAlpha = true;
					FixGrayscale = true;
				}
				// Mask optimizations
				else if (option == "m")
				{
					MaskOptimizations = true;
				}
				else if (option == "h" || option == "?")
				{
					return false;
				}
				else
				{
					nlwarning("Unknown option -%s", option.c_str());

					return false;
				}
			}
			// Filename
			else
			{
				std::string ext = NLMISC::toLowerAscii(NLMISC::CFile::getExtension(option));

				if (ext == "png" || ext == "tga")
				{
					InputFilenames.push_back(option);
				}
				else
				{
					nlwarning("Only PNG and TGA files supported, %s won't be processed", option.c_str());
				}
			}
		}
	}

	return !InputFilenames.empty();
}

#include "nel/misc/system_utils.h"

int main(int argc, char **argv)
{
	NLMISC::CApplicationContext applicationContext;

	if (!parseOptions(argc, argv))
	{
		writeInstructions();
		return 0;
	}

	for(uint i = 0; i < InputFilenames.size(); ++i)
	{
		std::string ext = NLMISC::toLowerAscii(NLMISC::CFile::getExtension(InputFilenames[i]));

		NLMISC::CIFile input;

		if (!input.open(InputFilenames[i]))
		{
			std::cerr << "Unable to open " << InputFilenames[i] << std::endl;
			return 1;
		}

		NLMISC::CBitmap bitmap;

		// all 8 bits textures are grayscale and not alpha
		bitmap.loadGrayscaleAsAlpha(false);

		uint8 depth = bitmap.load(input);

		// don't need file so close it
		input.close();

		if (depth == 0)
		{
			std::cerr << "Unable to decode " << InputFilenames[i] << std::endl;
			return 1;
		}

		bool modified = false;
		bool hasAlpha = false;
		bool isGrayscale = false;
		
		if (bitmap.getPixelFormat() == NLMISC::CBitmap::RGBA && depth == 32)
		{
			hasAlpha = true;
		}
		else if (bitmap.getPixelFormat() == NLMISC::CBitmap::AlphaLuminance)
		{
			hasAlpha = true;
			isGrayscale = true;
		}
		else if (bitmap.getPixelFormat() == NLMISC::CBitmap::Luminance)
		{
			isGrayscale = true;
		}
		else if (bitmap.getPixelFormat() == NLMISC::CBitmap::Alpha)
		{
			hasAlpha = true;
			isGrayscale = true;
		}

		if (MaskOptimizations && (!isGrayscale || hasAlpha))
		{
			std::cout << InputFilenames[i] << " (mask with wrong format)" << std::endl;

			if (!isGrayscale)
			{
				// get a pointer on original RGBA data
				uint32 size = bitmap.getPixels().size();
				uint32 *data = (uint32*)bitmap.getPixels().getPtr();
				uint32 *endData = (uint32*)((uint8*)data + size);

				NLMISC::CRGBA *color = NULL;

				// process all pixels
				while(data < endData)
				{
					color = (NLMISC::CRGBA*)data;

					// copy red value to green and blue,
					// because only red is used for mask
					color->B = color->G = color->R;

					// make opaque
					color->A = 255;

					++data;
				}
			}

			// already in grayscale, just remove alpha
			bitmap.convertToType(NLMISC::CBitmap::Luminance);

			isGrayscale = true;
			hasAlpha = false;
			modified = true;
		}
		else
		{
			if (!isGrayscale && bitmap.isGrayscale())
			{
				std::cout << InputFilenames[i] << " (grayscale image with RGB colors)" << std::endl;

				if (FixGrayscale)
				{
					if (!bitmap.convertToType(hasAlpha ? NLMISC::CBitmap::AlphaLuminance:NLMISC::CBitmap::Luminance))
					{
						std::cerr << "Unable to convert to Luminance" << std::endl;
						return 1;
					}

					isGrayscale = true;
					modified = true;
				}
			}

			uint8 alpha = 0;

			if (hasAlpha && bitmap.isAlphaUniform(&alpha))
			{
				std::cout << InputFilenames[i] << " (image with uniform alpha channel " << (sint)alpha << ")" << std::endl;

				if (FixAlpha && alpha == 255)
				{
					bitmap.makeOpaque();

					hasAlpha = false;
					modified = true;
				}
			}
		}

		if (!modified) continue;

		NLMISC::COFile output;

		if (!output.open(InputFilenames[i]))
		{
			std::cerr << "Unable to open" << std::endl;
			return 1;
		}

		uint32 newDepth = isGrayscale ? 8:24;

		if (hasAlpha) newDepth += 8;

		bool res = false;

		if (ext == "png")
		{
			res = bitmap.writePNG(output, newDepth);
		}
		else if (ext == "tga")
		{
			res = bitmap.writePNG(output, newDepth);
		}

		if (!res)
		{
			std::cerr << "Unable to encode" << std::endl;
			return 1;
		}
	}

	return 0;
}
