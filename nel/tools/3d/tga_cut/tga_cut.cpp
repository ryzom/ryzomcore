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


#include <iostream>

#include "nel/misc/file.h"
#include "nel/misc/bitmap.h"
#include "nel/misc/file.h"
#include "nel/misc/debug.h"
#include <math.h>

using namespace NLMISC;
using namespace std;

#define	TGA16	16
#define NOT_DEFINED 0xff

const int CutSize = 160;
const int SaveSize = 256;

void writeInstructions();
int main(int argc, char **argv);

// ***************************************************************************

bool getZoneNameFromXY (sint32 x, sint32 y, std::string &zoneName)
{
	if ((y>0) || (y<-255) || (x<0) || (x>255))
		return false;
	zoneName = toString(-y) + "_";
	zoneName += ('A' + (x/26));
	zoneName += ('A' + (x%26));
	return true;
}

void writeInstructions()
{
	cout<<endl;
	cout<<"tga_cut"<<endl;
	cout<<"  Cut TGA image file (24bits or 32 bits) into smaller TGA"<<endl;
	cout<<"syntax : tga_cut <input.tga>"<<endl;
	cout<<endl;
	cout<<"/? for this help"<<endl;
	cout<<endl; 
}

// ***************************************************************************
void dividSize (CBitmap &bitmap)
{
	// Must be RGBA
	nlassert (bitmap.getPixelFormat () == CBitmap::RGBA);

	// Copy the bitmap
	CBitmap temp = bitmap;

	// Resize the destination
	const uint width = temp.getWidth ();
	const uint height = temp.getHeight ();
	const uint newWidth = temp.getWidth ()/2;
	const uint newHeight = temp.getHeight ()/2;
	bitmap.resize (newWidth, newHeight, CBitmap::RGBA);

	// Pointers
	uint8 *pixelSrc = &(temp.getPixels ()[0]);
	uint8 *pixelDest = &(bitmap.getPixels ()[0]);

	// Resample
	uint x, y;
	for (y=0; y<newHeight; y++)
	for (x=0; x<newWidth; x++)
	{
		const uint offsetSrc = ((y*2)*width+x*2)*4;
		const uint offsetDest = (y*newWidth+x)*4;
		uint i;
		for (i=0; i<4; i++)
		{
			pixelDest[offsetDest+i] = ((uint)pixelSrc[offsetSrc+i] + (uint)pixelSrc[offsetSrc+4+i] + 
				(uint)pixelSrc[offsetSrc+4*width+i] + (uint)pixelSrc[offsetSrc+4*width+4+i])>>2;
		}
	}
}

// ***************************************************************************
int main(int argc, char **argv)
{
	// Parse Command Line.
	//====================
	if(argc<2)
	{
		writeInstructions();
		return 0;
	}
	if(!strcmp(argv[1],"/?"))
	{
		writeInstructions();
		return 0;
	}
	if(!strcmp(argv[1],"-?"))
	{
		writeInstructions();
		return 0;
	}
	if(argc != 2)
	{
		writeInstructions();
		return 0;
	}

	// Reading TGA and converting to RGBA
	//====================================
	CBitmap picTga;
	CBitmap picSrc;

	std::string inputFileName(argv[1]);
	NLMISC::CIFile input;
	if(!input.open(inputFileName))
	{
		cerr<<"Can't open input file "<<inputFileName<<endl;
		exit(1);
	}
	uint8 imageDepth = picTga.load(input);
	if(imageDepth==0)
	{
		cerr<<"Can't load file : "<<inputFileName<<endl;
		exit(1);
	}
	if(imageDepth!=16 && imageDepth!=24 && imageDepth!=32 && imageDepth!=8)
	{
		cerr<<"Image not supported : "<<imageDepth<<endl;
		exit(1);
	}

	if(!input.seek (0, NLMISC::IStream::begin))
	{
		cerr << "Seek to beginning failed"<<endl;
		exit(1);
	}
	
	// Reading header, 
	// To make sure that the bitmap is TGA, we check imageType and imageDepth.
	uint8	lengthID;
	uint8	cMapType;
	uint8	imageType;
	uint16	tgaOrigin;
	uint16	length;
	uint8	depth;
	uint16	xOrg;
	uint16	yOrg;
	uint16	width2;
	uint16	height2;
	uint8	imageDepth2;
	uint8	desc;
	
	input.serial(lengthID);
	input.serial(cMapType);
	input.serial(imageType);
	input.serial(tgaOrigin);
	input.serial(length);
	input.serial(depth);
	input.serial(xOrg);
	input.serial(yOrg);
	input.serial(width2);
	input.serial(height2);
	input.serial(imageDepth2);
	input.serial(desc);

	input.close();

	sint32 height = picTga.getHeight();
	sint32 width= picTga.getWidth();
	picTga.convertToType (CBitmap::RGBA);


	// Vectors for RGBA data
	CObjectVector<uint8> RGBASrc = picTga.getPixels();
	CObjectVector<uint8> RGBASrc2;
	CObjectVector<uint8> RGBADest;
	RGBADest.resize(SaveSize*SaveSize*4);
	uint	dstRGBADestId= 0;

	// Copy to the dest bitmap.
	picSrc.resize(SaveSize, SaveSize, CBitmap::RGBA);
	picSrc.getPixels(0) = RGBADest;

	// Must be RGBA
	nlassert (picSrc.getPixelFormat () == CBitmap::RGBA);

	// Pointers
	uint8 *pixelSrc = &(picTga.getPixels ()[0]);
	uint8 *pixelDest = &(picSrc.getPixels ()[0]);
	
	// clear the whole texture
	for (sint y = 0;y < SaveSize;++y)
	{
		for (sint x = 0;x < SaveSize;++x)
		{
			pixelDest[(y*SaveSize+x)*4]=-1;
			pixelDest[(y*SaveSize+x)*4+1]=-1;
			pixelDest[(y*SaveSize+x)*4+2]=-1;
			pixelDest[(y*SaveSize+x)*4+3]=-1;
		}
	}
	// Resample
	sint xzone, yzone;
	for (yzone = int(yOrg/CutSize)*CutSize; yzone < yOrg+height; yzone += CutSize)
	{
		for (xzone = int(xOrg/CutSize)*CutSize; xzone < xOrg+width; xzone += CutSize)
		{
			sint x, y;
			for (y=0; y<CutSize; y++)
			{
				for (x=0; x<CutSize; x++)
				{
					const uint offsetDest = (y*SaveSize+x)*4;
					uint i;
					if (x+xzone-xOrg>= width || y+yzone-yOrg>=height || x+xzone-xOrg<0 || y+yzone-yOrg<0)
					{
						// black outside the bitmap
						for (i=0; i<4; i++)
						{
							pixelDest[offsetDest+i] = 0;
						}
					}
					else
					{
						const uint offsetSrc = ((y+yzone-yOrg)*width+x+xzone-xOrg)*4;
						for (i=0; i<4; i++)
						{
							pixelDest[offsetDest+i] = pixelSrc[offsetSrc+i];
						}
					}
				}
			}
#if 0
			// if we don't want to save the empty pictures (useless now)
			int empty = 1;
			for (y = 0;y < CutSize;++y)
			{
				for (x=0;x<CutSize;++x)
				{
					// test if pixel is black (RGB==0)
					int offset = (y*SaveSize+x)*4;
					if (pixelDest[offset] || pixelDest[offset+1] || pixelDest[offset+2])
					{
						empty = 0;
						break;
					}
				}
			}
			if (empty) continue;
#endif

			// if the picture is empty, we don't save it !!!
			
			NLMISC::COFile output;

			string ZoneName;
			if (!getZoneNameFromXY(xzone/CutSize, -(yzone/CutSize+1), ZoneName))
			{
				cerr<<"Too large image"<<endl;
				exit(1);
			}

			ZoneName += ".tga";

			if(!output.open(ZoneName))
			{
				cerr<<"Can't open output file "<<ZoneName<<endl;
				exit(1);
			}

			cout<<"Saving "<<ZoneName<<endl;
			// Saving TGA file
			try 
			{
				picSrc.writeTGA (output, 16);
			}
			catch(const NLMISC::EWriteError &e)
			{
				cerr<<e.what()<<endl;
				exit(1);
			}
			
			output.close();
		}
	}
}	
