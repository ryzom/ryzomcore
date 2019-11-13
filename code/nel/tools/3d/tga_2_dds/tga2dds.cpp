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
#include "nel/misc/common.h"
#include "nel/misc/bitmap.h"
#include "nel/misc/path.h"
#include "nel/misc/debug.h"
#include "nel/misc/cmd_args.h"

#include <math.h>

#include "../s3tc_compressor_lib/s3tc_compressor.h"


using namespace NLMISC;
using namespace std;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

#define	TGA8	8
#define	TGA16	16
#define	PNG8	108
#define	PNG16	116
#define NOT_DEFINED 0xff


bool sameType(const std::string &sFileNameDest, uint8 algo);
bool dataCheck(const std::string &sFileNameSrc, const std::string &FileNameDest, uint8 algo);
std::string getOutputFileName(const std::string &inputFileName);




uint8 getType(const std::string &sFileNameDest)
{
	uint32 dds;
	FILE *f = nlfopen(sFileNameDest, "rb");
	if(f==NULL)
	{
		return NOT_DEFINED;
	}
	CS3TCCompressor::DDS_HEADER h;

	if (fread(&dds,1,4,f) != 4)
	{
		fclose(f);
		return NOT_DEFINED;
	}

#ifdef NL_BIG_ENDIAN
	NLMISC_BSWAP32(dds);
#endif

	if (fread(&h,sizeof(CS3TCCompressor::DDS_HEADER),1,f) != 1)
	{
		fclose(f);
		return NOT_DEFINED;
	}

	if(fclose(f))
	{
		cerr<<sFileNameDest<< "is not closed"<<endl;
	}

	if(h.ddpf.dwFourCC==MAKEFOURCC('D','X', 'T', '1')
		&& h.ddpf.dwRGBBitCount==0)
	{
		return DXT1;
	}

	if(h.ddpf.dwFourCC==MAKEFOURCC('D','X', 'T', '1')
		&& h.ddpf.dwRGBBitCount>0)
	{
		return DXT1A;
	}

	if(h.ddpf.dwFourCC==MAKEFOURCC('D','X', 'T', '3'))
	{
		return DXT3;
	}

	if(h.ddpf.dwFourCC==MAKEFOURCC('D','X', 'T', '5'))
	{
		return DXT5;
	}

	return NOT_DEFINED;
}

bool sameType(const std::string &sFileNameDest, uint8 &algo, bool wantMipMap)
{
	uint32 dds;
	FILE *f = nlfopen(sFileNameDest, "rb");
	if(f==NULL)
	{
		return false;
	}

	CS3TCCompressor::DDS_HEADER h;

	if (fread(&dds,1,4,f) != 4)
	{
		fclose(f);
		return false;
	}

	if (fread(&h,sizeof(::DDS_HEADER),1,f) != 1)
	{
		fclose(f);
		return false;
	}

	if(fclose(f))
	{
		cerr<<sFileNameDest<< "is not closed"<<endl;
	}

	bool	algoOk= false;
	switch(algo)
	{
		case DXT1:
			if(h.ddpf.dwFourCC==MAKEFOURCC('D','X', 'T', '1')
				&& h.ddpf.dwRGBBitCount==0)
				algoOk=true;
			break;

		case DXT1A:
			if(h.ddpf.dwFourCC==MAKEFOURCC('D','X', 'T', '1')
				&& h.ddpf.dwRGBBitCount>0)
				algoOk=true;
			break;

		case DXT3:
			if(h.ddpf.dwFourCC==MAKEFOURCC('D','X', 'T', '3'))
				algoOk=true;
			break;

		case DXT5:
			if(h.ddpf.dwFourCC==MAKEFOURCC('D','X', 'T', '5'))
				algoOk=true;
			break;
	}
	if(!algoOk)
		return false;

	// Test Mipmap.
	bool	fileHasMipMap= (h.dwFlags&DDSD_MIPMAPCOUNT) && (h.dwMipMapCount>1);
	if(fileHasMipMap==wantMipMap)
		return true;

	return false;
}



bool dataCheck(const std::string &sFileNameSrc, const std::string &sFileNameDest, uint8& algo, bool wantMipMap)
{
	if (!CFile::fileExists(sFileNameSrc))
	{
		cerr << "Can't open file " << sFileNameSrc << endl;
		return false;
	}

	if (!CFile::fileExists(sFileNameDest))
	{
		return false; // destination file doesn't exist yet
	}

	uint32 lastWriteTime1 = CFile::getFileModificationDate(sFileNameSrc);
	uint32 lastWriteTime2 = CFile::getFileModificationDate(sFileNameDest);

	if(lastWriteTime1 > lastWriteTime2)
	{
		return false;
	}
	if (lastWriteTime1 < lastWriteTime2)
	{
		if(!sameType(sFileNameDest, algo, wantMipMap))
		{
			return false; // file exists but a new compression type is required
		}
	}
	return true;
}

std::string getOutputFileName(const std::string &inputFileName)
{
	std::string::size_type pos = inputFileName.rfind(".");
	if(pos == std::string::npos)
	{
		// name whithout extension
		return inputFileName + ".dds";
	}
	else
	{
		return inputFileName.substr(0,pos) + ".dds";
	}
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

const int bayerDiv8R[4][4] = {
	{ 7, 3, 6, 2 },
	{ 1, 5, 0, 4 },
	{ 6, 2, 7, 3 },
	{ 0, 4, 1, 5 }
};

const int bayerDiv8G[4][4] = {
	{ 0, 4, 1, 5 },
	{ 6, 2, 7, 3 },
	{ 1, 5, 0, 4 },
	{ 7, 3, 6, 2 }
};

const int bayerDiv8B[4][4] = {
	{ 5, 1, 4, 0 },
	{ 3, 7, 2, 6 },
	{ 4, 0, 5, 1 },
	{ 2, 6, 3, 7 }
};

// ***************************************************************************
int main(int argc, char **argv)
{
	CApplicationContext applicationContext;

	// Parse Command Line.
	//====================
	NLMISC::CCmdArgs args;

	args.setDescription(
		"Convert TGA or PNG image file to DDS compressed file using DXTC compression (DXTC1, DXTC1 with alpha, DXTC3, or DXTC5).\n"
		"  The program looks for possible user color files and load them automatically, a user color file must have the same name that the original tga file, plus the extension \"_usercolor\"\n"
		"Eg.: pic.tga, the associated user color file must be: pic_usercolor.tga\n"
		);
	args.addArg("o", "output", "output.dds", "Output DDS filename or directory");
	args.addArg("a", "algo", "algo", "Conversion algorithm to use\n"
		"  1      for DXTC1 (no alpha)\n"
		"  1A     for DXTC1 with alpha\n"
		"  3      for DXTC3\n"
		"  5      for DXTC5\n"
		"  tga16  for 16 bits TGA\n"
		"  tga8   for 8  bits TGA\n"
		"  png16  for 16 bits PNG\n"
		"  png8   for 8  bits PNG\n"
		"\n"
		"  default : DXTC1 if 24 bits, DXTC5 if 32 bits."
		);
	args.addArg("g", "grayscale", "", "Don't load grayscape images as alpha but as grayscale");
	args.addArg("m", "mipmap", "", "Create MipMap");
	args.addArg("r", "reduce", "FACTOR", "Reduce the bitmap size before compressing\n  FACTOR is 0, 1, 2, 3, 4, 5, 6, 7 or 8");
	args.addAdditionalArg("input", "PNG or TGA files to convert", false);

	if (!args.parse(argc, argv)) return 1;

	string OptOutputFileName;
	uint8 OptAlgo = NOT_DEFINED;
	bool OptMipMap = false;
	bool OptGrayscale = false;
	uint Reduce = 0;

	if (args.haveArg("o"))
		OptOutputFileName = args.getArg("o").front();

	if (args.haveArg("m"))
		OptMipMap = true;

	if (args.haveArg("g"))
		OptGrayscale = true;

	if (args.haveArg("a"))
	{
		std::string strAlgo = args.getArg("a").front();

		if (strAlgo == "1")					OptAlgo = DXT1;
		else if (toLower(strAlgo) == "1a") 	OptAlgo = DXT1A;
		else if (strAlgo == "3")			OptAlgo = DXT3;
		else if (strAlgo == "5") 			OptAlgo = DXT5;
		else if (strAlgo == "tga8")			OptAlgo = TGA8;
		else if (strAlgo == "tga16")		OptAlgo = TGA16;
		else if (strAlgo == "png8")			OptAlgo = PNG8;
		else if (strAlgo == "png16")		OptAlgo = PNG16;
		else
		{
			cerr << "Unknown algorithm: " << strAlgo << endl;
			return 1;
		}
	}

	if (args.haveArg("r"))
	{
		std::string strReduce = args.getArg("r").front();

		// Reduce size of the bitmap
		if (fromString(strReduce, Reduce))
		{
			if (Reduce > 8) Reduce = 8;
		}
	}

	std::vector<std::string> inputFileNames = args.getAdditionalArg("input");

	for(uint i = 0; i < inputFileNames.size(); ++i)
	{
		uint8 algo;

		// Reading TGA or PNG and converting to RGBA
		//====================================
		CBitmap picTga;
		CBitmap picTga2;
		CBitmap picSrc;

		std::string inputFileName = inputFileNames[i];

		if(inputFileName.find("_usercolor")<inputFileName.length())
		{
			return 0;
		}

		NLMISC::CIFile input;
		if(!input.open(inputFileName))
		{
			cerr<<"Can't open input file " << inputFileName << endl;
			return 1;
		}

		// allow to load an image as grayscale instead of alpha
		if (OptGrayscale) picTga.loadGrayscaleAsAlpha(false);

		uint8 imageDepth = picTga.load(input);
		if(imageDepth==0)
		{
			cerr<<"Can't load file: "<<inputFileName<<endl;
			return 1;
		}
		if(imageDepth!=16 && imageDepth!=24 && imageDepth!=32 && imageDepth!=8)
		{
			cerr<<"Image not supported: "<<imageDepth<<endl;
			return 1;
		}
		input.close();
		uint32 height = picTga.getHeight();
		uint32 width= picTga.getWidth();
		picTga.convertToType (CBitmap::RGBA);


		// Output file name and algo.
		//===========================
		std::string outputFileName;

		if (!OptOutputFileName.empty())
		{
			// if OptOutputFileName is a directory, append the original filename
			if (CFile::isDirectory(OptOutputFileName))
			{
				outputFileName = CPath::standardizePath(OptOutputFileName) + CFile::getFilename(getOutputFileName(inputFileName));
			}
			else
			{
				outputFileName = OptOutputFileName;

				if (inputFileNames.size() > 1)
				{
					cerr<<"WARNING! Several files to convert to the same output filename! Use an output directory instead."<<endl;
					return 1;
				}
			}
		}
		else
		{
			outputFileName = getOutputFileName(inputFileName);
		}

		// Check dest algo
		if (OptAlgo==NOT_DEFINED)
			OptAlgo = getType (outputFileName);

		// Choose Algo.
		if(OptAlgo!=NOT_DEFINED)
		{
			algo= OptAlgo;
		}
		else
		{
			// TODO: if alpha channel is 0, use DXTC1a instead DXTC1
			if(imageDepth==24)
				algo = DXT1;
			else
				algo = DXT5;
		}

		// Data check
		//===========
		if(dataCheck(inputFileName,outputFileName, OptAlgo, OptMipMap))
		{
			cout<<outputFileName<<" : a recent dds file already exists"<<endl;
			return 0;
		}


		// Vectors for RGBA data
		CObjectVector<uint8> RGBASrc = picTga.getPixels();
		CObjectVector<uint8> RGBASrc2;
		CObjectVector<uint8> RGBADest;
		RGBADest.resize(height*width*4);
		uint	dstRGBADestId= 0;

		// UserColor
		//===========
		/*
		// Checking if option "usercolor" has been used
		std::string userColorFileName;
		if(argc>4)
		{
			if(strcmp("-usercolor",argv[4])==0)
			{
				if(argc!=6)
				{
					writeInstructions();
					return;
				}
				userColorFileName = argv[5];
			}
			else
			{
				writeInstructions();
				return;
			}
		}
		*/
		// Checking if associate usercolor file  exists
		std::string userColorFileName;
		std::string::size_type pos = inputFileName.rfind(".");
		if (pos == std::string::npos)
		{
			// name without extension
			userColorFileName = inputFileName + "_usercolor";
		}
		else
		{
			// append input filename extension
			userColorFileName = inputFileName.substr(0,pos) + "_usercolor" + inputFileName.substr(pos);
		}

		// Reading second Tga for user color, don't complain if _usercolor is missing
		NLMISC::CIFile input2;
		if (CPath::exists(userColorFileName) && input2.open(userColorFileName))
		{
			picTga2.load(input2);
			uint32 height2 = picTga2.getHeight();
			uint32 width2 = picTga2.getWidth();
			nlassert(width2==width);
			nlassert(height2==height);
			picTga2.convertToType (CBitmap::RGBA);

			RGBASrc2 = picTga2.getPixels();

			NLMISC::CRGBA *pRGBASrc = (NLMISC::CRGBA*)&RGBASrc[0];
			NLMISC::CRGBA *pRGBASrc2 = (NLMISC::CRGBA*)&RGBASrc2[0];

			for(uint32 i = 0; i<width*height; i++)
			{
				// If no UserColor, must take same RGB, and keep same Alpha from src1 !!! So texture can have both alpha
				// userColor and other alpha usage.
				if(pRGBASrc2[i].A==255)
				{
					RGBADest[dstRGBADestId++]= pRGBASrc[i].R;
					RGBADest[dstRGBADestId++]= pRGBASrc[i].G;
					RGBADest[dstRGBADestId++]= pRGBASrc[i].B;
					RGBADest[dstRGBADestId++]= pRGBASrc[i].A;
				}
				else
				{
					// Old code.
					/*uint8 F = (uint8) ((float)pRGBASrc[i].R*0.3 + (float)pRGBASrc[i].G*0.56 + (float)pRGBASrc[i].B*0.14);
					uint8 Frgb;
					if((F*pRGBASrc2[i].A/255)==255)
						Frgb = 0;
					else
						Frgb = (255-pRGBASrc2[i].A)/(255-F*pRGBASrc2[i].A/255);
					RGBADest[dstRGBADestId++]= Frgb*pRGBASrc[i].R/255;
					RGBADest[dstRGBADestId++]= Frgb*pRGBASrc[i].G/255;
					RGBADest[dstRGBADestId++]= Frgb*pRGBASrc[i].B/255;
					RGBADest[dstRGBADestId++]= F*pRGBASrc[i].A/255;*/

					// New code: use new restrictions from IDriver.
					float	Rt, Gt, Bt, At;
					float	Lt;
					float	Rtm, Gtm, Btm, Atm;

					// read 0-1 RGB pixel.
					Rt= (float)pRGBASrc[i].R/255;
					Gt= (float)pRGBASrc[i].G/255;
					Bt= (float)pRGBASrc[i].B/255;
					Lt= Rt*0.3f + Gt*0.56f + Bt*0.14f;

					// take Alpha from userColor src.
					At= (float)pRGBASrc2[i].A/255;
					Atm= 1-Lt*(1-At);

					// If normal case.
					if(Atm>0)
					{
						Rtm= Rt*At / Atm;
						Gtm= Gt*At / Atm;
						Btm= Bt*At / Atm;
					}
					// Else special case: At==0, and Lt==1.
					else
					{
						Rtm= Gtm= Btm= 0;
					}

					// copy to buffer.
					sint	r,g,b,a;
					r= (sint)floor(Rtm*255+0.5f);
					g= (sint)floor(Gtm*255+0.5f);
					b= (sint)floor(Btm*255+0.5f);
					a= (sint)floor(Atm*255+0.5f);
					clamp(r, 0,255);
					clamp(g, 0,255);
					clamp(b, 0,255);
					clamp(a, 0,255);
					RGBADest[dstRGBADestId++]= r;
					RGBADest[dstRGBADestId++]= g;
					RGBADest[dstRGBADestId++]= b;
					RGBADest[dstRGBADestId++]= a;
				}
			}
		}
		else
			RGBADest = RGBASrc;

		// Copy to the dest bitmap.
		picSrc.resize(width, height, CBitmap::RGBA);
		picSrc.getPixels(0)= RGBADest;

		// Resize the destination bitmap ?
		while (Reduce != 0)
		{
			dividSize (picSrc);
			Reduce--;
		}

		if (algo == TGA16)
		{
			// Apply bayer dither
			CObjectVector<uint8> &rgba = picSrc.getPixels(0);
			const uint32 w = picSrc.getWidth(0);
			uint32 x = 0;
			uint32 y = 0;
			for (uint32 i = 0; i < rgba.size(); i += 4)
			{
				NLMISC::CRGBA &c = reinterpret_cast<NLMISC::CRGBA &>(rgba[i]);
				c.R = (uint8)std::min(255, (int)c.R + bayerDiv8R[x % 4][y % 4]);
				c.G = (uint8)std::min(255, (int)c.G + bayerDiv8G[x % 4][y % 4]);
				c.B = (uint8)std::min(255, (int)c.B + bayerDiv8B[x % 4][y % 4]);
				++x;
				x %= w;
				if (x == 0)
					++y;
			}
		}

		// 8 or 16 bits TGA or PNG ?
		if ((algo == TGA16) || (algo == TGA8) || (algo == PNG16) || (algo == PNG8))
		{
			// Saving TGA or PNG file
			//=================
			NLMISC::COFile output;
			if(!output.open(outputFileName))
			{
				cerr<<"Can't open output file "<<outputFileName<<endl;
				return 1;
			}
			try
			{
				if (algo == TGA16)
				{
					picSrc.writeTGA (output, 16);
				}
				else if (algo == TGA8)
				{
					picSrc.convertToType(CBitmap::Luminance);
					picSrc.writeTGA (output, 8);
				}
				else if (algo == PNG16)
				{
					picSrc.writePNG (output, 16);
				}
				else if (algo == PNG8)
				{
					picSrc.convertToType(CBitmap::Luminance);
					picSrc.writePNG (output, 8);
				}
			}
			catch(const NLMISC::EWriteError &e)
			{
				cerr<<e.what()<<endl;
				return 1;
			}

			output.close();
		}
		else
		{
			// Compress
			//===========

			// log.
			std::string algostr;
			switch(algo)
			{
				case DXT1:
					algostr = "DXTC1";
					break;
				case DXT1A:
					algostr = "DXTC1A";
					break;
				case DXT3:
					algostr = "DXTC3";
					break;
				case DXT5:
					algostr = "DXTC5";
					break;
			}
			cout<<"compressing ("<<algostr<<") "<<inputFileName<<" to "<<outputFileName<<endl;


			// Saving compressed DDS file
			// =================
			NLMISC::COFile output;
			if(!output.open(outputFileName))
			{
				cerr<<"Can't open output file "<<outputFileName<<endl;
				return 1;
			}
			try
			{
				CS3TCCompressor		comp;
				comp.compress(picSrc, OptMipMap, algo, output);
			}
			catch(const NLMISC::EWriteError &e)
			{
				cerr<<e.what()<<endl;
				return 1;
			}

			output.close();
		}
	}

	return 0;
}
