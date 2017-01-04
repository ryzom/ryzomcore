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
#include "nel/misc/common.h"
#include "nel/misc/bitmap.h"
#include "nel/misc/path.h"
#include "nel/misc/cmd_args.h"
#include "nel/misc/vector_2d.h"
#include "nel/misc/uv.h"
#include "nel/misc/algo.h"

struct CPoint
{
	CPoint(sint _x, sint _y) :x(_x), y(_y)
	{
	}

	CPoint operator + (const CPoint &p) const
	{
		return CPoint(x + p.x, y + p.y);
	}

	sint x;
	sint y;
};

const CPoint Up(0, -1);
const CPoint Down(0, 1);
const CPoint Left(-1, 0);
const CPoint Right(1, 0);

uint TextureSize = 4096;

const NLMISC::CRGBA DiscardColor = NLMISC::CRGBA::Red;
const NLMISC::CRGBA KeepColor = NLMISC::CRGBA::Blue;

typedef std::vector<CPoint> CPoints;

struct CFace
{
	static const uint INDICES_COUNT = 3;
	uint indices[INDICES_COUNT];
};

bool fillPoint(NLMISC::CBitmap &bitmap, sint width, CPoints &points)
{
	if (points.empty()) return false;

	// take last point in queue
	CPoint p(points.back());
	points.pop_back();

	NLMISC::CRGBA c = bitmap.getPixelColor(p.x, p.y);

	if (c == NLMISC::CRGBA::White)
	{
		// white is used for background

		// replace with color we want to discard
		bitmap.setPixelColor(p.x, p.y, DiscardColor);

		uint w = bitmap.getWidth();
		uint h = bitmap.getHeight();

		// put adjacent pixels in queue to process later
		if (p.y > 0) points.push_back(p + Up);
		if (p.y < h-1) points.push_back(p + Down);
		if (p.x > 0) points.push_back(p + Left);
		if (p.x < w-1) points.push_back(p + Right);
	}
	else if (c == NLMISC::CRGBA::Black)
	{
		// black is used for vertices

		// increase them by border width
		for (sint y = -width; y <= width; ++y)
		{
			for (sint x = -width; x <= width; ++x)
			{
				bitmap.setPixelColor(p.x + x, p.y + y, KeepColor);
			}
		}
	}

	return true;
}

int main(int argc, char **argv)
{
	NLMISC::CApplicationContext applicationContext;

	// Parse Command Line.
	//====================
	NLMISC::CCmdArgs args;

	args.setDescription("Textures tool");
	args.addArg("c", "colorize", "color", "Colorize textures using a color (in HTML hexdecimal format like #rrggbb)");
	args.addArg("f", "fill", "color or image", "Fill background part with color or image");
	args.addArg("u", "uvmap", "", "Generate a UV Map texture from OBJ file");
	args.addArg("w", "width", "width of border", "Width of the border to fill (default 0)");
	args.addArg("s", "size", "size of output bitmap", "Width and height of generated bitmap (default 4096)");
	args.addArg("b", "background", "background color", "Color to use to fill background");
	args.addArg("o", "output", "filename", "Output filename");
	args.addAdditionalArg("filename", "File to process", true, true);

	if (!args.parse(argc, argv)) return 1;

	std::string filename = args.getAdditionalArg("filename").front();

	std::string output = args.haveArg("o") ? args.getArg("o").front() : "";

	if (args.haveArg("s"))
	{
		// size of generated bitmap
		NLMISC::fromString(args.getArg("s").front(), TextureSize);
	}

	if (args.haveArg("c"))
	{
		// colorize
		NLMISC::CIFile file;

		NLMISC::CRGBA color;
		color.fromString(args.getArg("c").front());

		if (file.open(filename))
		{
			NLMISC::CBitmap bitmap;

			if (bitmap.load(file))
			{
				NLMISC::CObjectVector<uint8> &pixels = bitmap.getPixels();

				NLMISC::CRGBA *pRGBA = (NLMISC::CRGBA*)&pixels[0];

				uint32 size = bitmap.getSize();

				for (uint j = 0; j < size; ++j)
				{
					// TODO: find what computation do
//					*(pRGBA++)-> = color;
				}

				NLMISC::COFile out;

				if (out.open(output))
				{
					bitmap.writePNG(out, 24);
				}
			}
		}
	}

	if (args.haveArg("f"))
	{
		// fill areas in a bitmap with another texture or color

		// for example :
		// textures_tool -f normal.png -w 2 -b #000000 uvmap.png -o test_normal.png
		// will use a copy 1024x1024 texture map on a 4096x4096 UV Map preserving the different areas

		std::string foregroundColorOrFilename = args.getArg("f").front();

		NLMISC::CRGBA foregroundColor = NLMISC::CRGBA::Black, backgroundColor = NLMISC::CRGBA::Black;

		NLMISC::CBitmap textureBitmap;

		bool useTexture = false;

		// f parameter is required
		if (NLMISC::CFile::fileExists(foregroundColorOrFilename))
		{
			// load texture
			NLMISC::CIFile textureFile;

			if (!textureFile.open(foregroundColorOrFilename))
			{
				nlwarning("Unable to open %s", foregroundColorOrFilename.c_str());
				return 1;
			}

			// decode texture
			if (!textureBitmap.load(textureFile))
			{
				nlwarning("Unable to decode %s", foregroundColorOrFilename.c_str());
				return 1;
			}

			useTexture = true;
		}
		else
		{
			// parse color from argument
			foregroundColor.fromString(foregroundColorOrFilename);
		}

		if (args.haveArg("b"))
		{
			// parse HTML color from argument
			backgroundColor.fromString(args.getArg("b").front());
		}

		sint width = 0;

		if (args.haveArg("w"))
		{
			// parse width of borders
			NLMISC::fromString(args.getArg("w").front(), width);
		}

		// load bitmap
		NLMISC::CIFile file;

		if (!file.open(filename))
		{
			nlwarning("Unable to open %s", filename.c_str());
			return 1;
		}

		// decode bitmap
		NLMISC::CBitmap inBitmap;

		if (!inBitmap.load(file))
		{
			nlwarning("Unable to decode %s", filename.c_str());
			return 1;
		}

		CPoints Points;

		// we can't have more than width * height points, so allocate memory for all of them
		Points.reserve(inBitmap.getWidth() * inBitmap.getHeight());

		// first point to process
		Points.push_back(CPoint(0, 0));

		// process all points from 0, 0
		while(fillPoint(inBitmap, width, Points)) { }

		// create a new bitmap for output
		NLMISC::CBitmap outBitmap;
		outBitmap.resize(inBitmap.getWidth(), inBitmap.getHeight());

		// copy points colors to new bitmap
		for (sint y = 0, h = inBitmap.getHeight(); y < h; ++y)
		{
			for (sint x = 0, w = inBitmap.getWidth(); x < w; ++x)
			{
				if (inBitmap.getPixelColor(x, y) != DiscardColor)
				{
					// we copy this point, repeat texture image if using it
					outBitmap.setPixelColor(x, y, useTexture ? textureBitmap.getPixelColor(x % textureBitmap.getWidth(), y % textureBitmap.getHeight()) : foregroundColor);
				}
				else
				{
					// put a background color
					outBitmap.setPixelColor(x, y, backgroundColor);
				}
			}
		}

		// save output bitmap
		NLMISC::COFile outFile;

		if (outFile.open(output))
		{
			outBitmap.writePNG(outFile, 24);
		}
	}

	if (args.haveArg("u"))
	{
		std::vector<NLMISC::CUV> verticeTextureCoords;
		std::vector<CFace> faces;

		NLMISC::CIFile objFile;

		if (!objFile.open(filename))
		{
			nlwarning("Unable to open %s", filename.c_str());
			return 1;
		}

		char buffer[1024];

		while (!objFile.eof())
		{
			objFile.getline(buffer, 1024);
			buffer[1023] = '\0';

			std::string line(buffer);

			if (line.size() < 3) continue;

			// texture coordinate
			if (line.substr(0, 3) == "vt ")
			{
				std::vector<std::string> tokens;
				NLMISC::explode(line, std::string(" "), tokens);

				if (tokens.size() == 3)
				{
					float u, v;
					NLMISC::fromString(tokens[1], u);
					NLMISC::fromString(tokens[2], v);

					// V coordinates are inverted
					verticeTextureCoords.push_back(NLMISC::CUV(u * (float)TextureSize, (1.f - v) * (float)TextureSize));
				}
				else
				{
					nlwarning("Not 3 arguments for VT");
				}
			}
			else if (line.substr(0, 2) == "f ")
			{
				std::vector<std::string> tokens;
				NLMISC::explode(line, std::string(" "), tokens);

				if (tokens.size() == 4)
				{
					CFace face;

					for (uint i = 1; i < 4; ++i)
					{
						std::vector<std::string> tokens2;
						NLMISC::explode(tokens[i], std::string("/"), tokens2);

						if (tokens2.size() == 3)
						{
							NLMISC::fromString(tokens2[1], face.indices[i - 1]);

							// we want indices start from 0 instead of 1
							--face.indices[i - 1];
						}
						else
						{
							nlwarning("Not 3 arguments for indices");
						}
					}

					faces.push_back(face);
				}
				else
				{
					nlwarning("Not 4 arguments for F");
				}
			}
		}

		nlinfo("OBJ file processed with %u vertices and %u faces", (uint)verticeTextureCoords.size(), (uint)faces.size());

		objFile.close();

		// draw UV Map
		// create a new bitmap for output
		NLMISC::CBitmap outBitmap;
		outBitmap.resize(TextureSize, TextureSize);

		// white background
		memset(&outBitmap.getPixels()[0], 255, TextureSize * TextureSize * 4);

		// process all faces
		for (uint i = 0, len = faces.size(); i < len; ++i)
		{
			const CFace &face = faces[i];

			// the 3 pixels of a vertice
			NLMISC::CUV uv0 = verticeTextureCoords[face.indices[0]];
			NLMISC::CUV uv1 = verticeTextureCoords[face.indices[1]];
			NLMISC::CUV uv2 = verticeTextureCoords[face.indices[2]];

			std::vector<std::pair<sint, sint> > pixels, temp;

			// draw the triangle with vertices UV coordinates
			NLMISC::drawFullLine(uv0.U, uv0.V, uv1.U, uv1.V, pixels);

			// append pixels
			NLMISC::drawFullLine(uv1.U, uv1.V, uv2.U, uv2.V, temp);
			pixels.reserve(pixels.size() + temp.size());
			pixels.insert(pixels.end(), temp.begin(), temp.end());

			// append pixels
			NLMISC::drawFullLine(uv2.U, uv2.V, uv0.U, uv0.V, temp);
			pixels.reserve(pixels.size() + temp.size());
			pixels.insert(pixels.end(), temp.begin(), temp.end());

			// for each pixels, set them to black
			for (uint j = 0, jlen = pixels.size(); j < jlen; ++j)
			{
				outBitmap.setPixelColor(pixels[j].first, pixels[j].second, NLMISC::CRGBA::Black);
			}
		}

		// save output bitmap
		NLMISC::COFile outFile;

		if (outFile.open(output))
		{
			outBitmap.writePNG(outFile, 24);
		}
	}

	return 0;
}
