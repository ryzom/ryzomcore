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

// lightmap_optimizer
// ------------------
// the goal is to regroup lightmap of a level into lightmap with a higher level

#include "nel/misc/common.h"
#include "nel/misc/file.h"
#include "nel/misc/bitmap.h"
#include "nel/misc/log.h"
#include "nel/misc/path.h"
#include "nel/misc/uv.h"
#include "nel/misc/cmd_args.h"

#include <vector>
#include <string>

// ---------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;

// ***************************************************************************
void outString(const string &sText)
{
	printf("%s\n", sText.c_str());
}

// ***************************************************************************
// test every 4 pixels for 2 reason: DXTC and speed
const	uint32	posStep= 4;

// ***************************************************************************
// Try all position to put pSrc in pDst
bool tryAllPos(NLMISC::CBitmap *pSrc, NLMISC::CBitmap *pDst, sint32 &x, sint32 &y)
{
	uint32 i, j;
	CObjectVector<uint8> &rSrcPix = pSrc->getPixels();
	CObjectVector<uint8> &rDstPix = pDst->getPixels();

	// Recalculate real size of the source (without padding to power of 2)
	uint32 nSrcWidth = pSrc->getWidth(), nSrcHeight = pSrc->getHeight();

	if (nSrcWidth > pDst->getWidth() ) return false;
	if (nSrcHeight > pDst->getHeight() ) return false;

	// For all position test if the Src plane can be put in
	for (j = 0; j <= (pDst->getHeight() - nSrcHeight); j+= posStep)
	for (i = 0; i <= (pDst->getWidth() - nSrcWidth); i+= posStep)
	{
		x = i; y = j;
		
		uint32 a, b;
		bool bCanPut = true;
		for (b = 0; b < nSrcHeight; ++b)
		{
			for (a = 0; a < nSrcWidth; ++a)
			{
				if (rDstPix[4*((x+a)+(y+b)*pDst->getWidth())+3] != 0)
				{
					bCanPut = false;
					break;
				}
			}
			if (bCanPut == false)
				break;
		}
		if (bCanPut)
			return true;
	}
	return false;
}

// ***************************************************************************
void putPixel(uint8 *dst, uint8 *src, bool alphaTransfert)
{
	dst[0] = src[0];
	dst[1] = src[1];
	dst[2] = src[2];
	if (alphaTransfert)
		dst[3] = src[3];
	else
		dst[3] = 255;
}

// ***************************************************************************
bool putIn(NLMISC::CBitmap *pSrc, NLMISC::CBitmap *pDst, sint32 x, sint32 y, bool alphaTransfert=true)
{
	uint8 *rSrcPix = &pSrc->getPixels()[0];
	uint8 *rDstPix = &pDst->getPixels()[0];

	uint	wSrc= pSrc->getWidth();
	uint	hSrc= pSrc->getHeight();
	for (uint b = 0; b < hSrc; ++b)
	for (uint a = 0; a < wSrc; ++a)
	{
		if (rDstPix[4*((x+a)+(y+b)*pDst->getWidth())+3] != 0)
			return false;

		// write
		putPixel(rDstPix + 4*((x+a)+(y+b)*pDst->getWidth()), rSrcPix+ 4*(a+b*pSrc->getWidth()), alphaTransfert);
	}

	// DXTC compression optim: fill last column block and last row block of 4 pixels with block color (don't let black or undefined)
	uint	wSrc4= 4*((wSrc+3)/4);
	uint	hSrc4= 4*((hSrc+3)/4);
	// expand on W
	if(wSrc<wSrc4)
	{
		for(uint a=wSrc;a<wSrc4;a++)
		{
			for(uint b=0;b<hSrc4;b++)
			{
				putPixel(rDstPix + 4*((x+a)+(y+b)*pDst->getWidth()), rDstPix + 4*((x+wSrc-1)+(y+b)*pDst->getWidth()), alphaTransfert);
			}
		}
	}
	// expand on H
	if(hSrc<hSrc4)
	{
		for(uint b=hSrc;b<hSrc4;b++)
		{
			for(uint a=0;a<wSrc4;a++)
			{
				putPixel(rDstPix + 4*((x+a)+(y+b)*pDst->getWidth()), rDstPix + 4*((x+a)+(y+hSrc-1)*pDst->getWidth()), alphaTransfert);
			}
		}
	}

	return true;
}

// ***************************************************************************
string getBaseName(const string &fullname)
{
	string basename;
	string::size_type pos = fullname.rfind('_');
	if (pos != string::npos) basename = fullname.substr(0, pos+1);
	return basename;
}

// ***************************************************************************
// resize the bitmap to the next power of 2 and preserve content
void enlargeCanvas(NLMISC::CBitmap &b)
{
	sint32 nNewWidth = b.getWidth(), nNewHeight = b.getHeight();
	if (nNewWidth > nNewHeight)
		nNewHeight *= 2;
	else
		nNewWidth *= 2;

	NLMISC::CBitmap b2;
	b2.resize (nNewWidth, nNewHeight, NLMISC::CBitmap::RGBA);
	
	CObjectVector<uint8> &rPixelBitmap = b2.getPixels(0);
	for (sint32 i = 0; i < nNewWidth*nNewHeight*4; ++i)
		rPixelBitmap[i] = 0;
	
	putIn (&b, &b2, 0, 0);
	b = b2;
}

bool writeFileDependingOnFilename(const std::string &filename, CBitmap &bitmap)
{
	NLMISC::COFile out;

	if (out.open(filename))
	{
		if (toLowerAscii(filename).find(".png") != string::npos)
		{
			bitmap.writePNG(out, 32);
		}
		else
		{
			bitmap.writeTGA(out, 32);
		}

		out.close();

		return true;
	}

	return false;
}


// ***************************************************************************
// main
// ***************************************************************************
int main(int argc, char **argv)
{
	CApplicationContext applicationContext;

	// Parse Command Line.
	NLMISC::CCmdArgs args;

	args.setDescription("Build a huge interface texture from several small elements to optimize video memory usage.");
	args.addArg("f", "format", "format", "Output format (png or tga)");
	args.addArg("s", "subset", "existing_uv_txt_name", "Build a subset of an existing interface definition while preserving the existing texture ids, to support freeing up VRAM by switching to the subset without rebuilding the entire interface.");
	args.addArg("x", "extract", "", "Extract all interface elements from <output_filename> to <input_path>.");
	args.addAdditionalArg("output_filename", "PNG or TGA file to generate", true);
	args.addAdditionalArg("input_path", "Path that containts interfaces elements", false);
	args.addArg("", "no-border", "", "Disable border duplication. Enabled by default");

	if (!args.parse(argc, argv)) return 1;

	// build as a subset of existing interface
	bool buildSubset = false;
	string existingUVfilename;

	if (args.haveArg("s"))
	{
		buildSubset = true;
		existingUVfilename = args.getArg("s").front();
	}

	//
	uint borderSize = 1;
	if (args.haveLongArg("no-border"))
	{
		borderSize = 0;
	}

	// extract all interface elements
	bool extractElements = args.haveArg("x");

	// output format
	std::string outputFormat;

	if (args.haveArg("f"))
	{
		outputFormat = args.getArg("f").front();

		if (outputFormat != "png" && outputFormat != "tga")
		{
			outString(toString("ERROR: Format %s not supported, only png and tga formats are", outputFormat.c_str()));
			return -1;
		}
	}

	std::vector<std::string> inputDirs = args.getAdditionalArg("input_path");

	string fmtName = args.getAdditionalArg("output_filename").front();

	// append PNG extension if no one provided
	if (fmtName.rfind('.') == string::npos) fmtName += "." + (outputFormat.empty() ? "png":outputFormat);

	if (extractElements)
	{
		if (inputDirs.empty())
		{
			outString(toString("ERROR: No input directories specified"));
			return -1;
		}

		// name of UV file
		existingUVfilename = fmtName.substr(0, fmtName.rfind('.'));
		existingUVfilename += ".txt";

		// Load existing UV file
		CIFile iFile;
		string filename = CPath::lookup(existingUVfilename, false);

		if (filename.empty() || !iFile.open(filename))
		{
			outString(toString("ERROR: Unable to open %s", existingUVfilename.c_str()));
			return -1;
		}

		// Load existing bitmap file
		CIFile bitmapFile;

		if (!bitmapFile.open(fmtName))
		{
			outString(toString("ERROR: Unable to open %s", fmtName.c_str()));
			return -1;
		}

		// load bitmap
		CBitmap textureBitmap;
		uint8 colors = textureBitmap.load(bitmapFile);

		// file already loaded in memory, close it
		bitmapFile.close();

		if (colors != 32)
		{
			outString(toString("ERROR: %s is not a RGBA bitmap", existingUVfilename.c_str()));
			return -1;
		}

		// make sure transparent pixels are black
		textureBitmap.makeTransparentPixelsBlack();

		float textureWidth = (float)textureBitmap.getWidth();
		float textureHeight = (float)textureBitmap.getHeight();

		char bufTmp[256], tgaName[256];
		string sTGAname;
		float uvMinU, uvMinV, uvMaxU, uvMaxV;
		while (!iFile.eof())
		{
			iFile.getline(bufTmp, 256);

			if (sscanf(bufTmp, "%s %f %f %f %f", tgaName, &uvMinU, &uvMinV, &uvMaxU, &uvMaxV) != 5)
			{
				nlwarning("Can't parse %s", bufTmp);
				continue;
			}

			float xf = uvMinU * textureWidth;
			float yf = uvMinV * textureHeight;
			float widthf = (uvMaxU - uvMinU) * textureWidth;
			float heightf = (uvMaxV - uvMinV) * textureHeight;

			uint x = (uint)xf;
			uint y = (uint)yf;
			uint width = (uint)widthf;
			uint height = (uint)heightf;

			if ((float)x != xf || (float)y != yf || (float)width != widthf || (float)height != heightf)
			{
				nlwarning("Wrong round");
			}

			if (width && height)
			{
				// create bitmap
				CBitmap bitmap;
				bitmap.resize(width, height);
				bitmap.blit(textureBitmap, x, y, width, height, 0, 0);

				sTGAname = inputDirs.front() + "/" + tgaName;

				// force specific format instead of using original one
				if (!outputFormat.empty())
				{
					sTGAname = sTGAname.substr(0, sTGAname.rfind('.'));
					sTGAname += "." + outputFormat;
				}

				// write the file
				if (writeFileDependingOnFilename(sTGAname, bitmap))
				{
					outString(toString("Writing file %s", sTGAname.c_str()));
				}
				else
				{
					outString(toString("Unable to writing file %s", sTGAname.c_str()));
				}
			}
			else
			{
				outString(toString("Bitmap with wrong size"));
			}
		}

		return 0;
	}

	vector<string> AllMapNames;
	vector<string>::iterator it = inputDirs.begin(), itEnd = inputDirs.end();

	while( it != itEnd )
	{
		string sDir = *it++;

		if( !CFile::isDirectory(sDir) )
		{
			outString(toString("ERROR: directory %s does not exist", sDir.c_str()));
			return -1;
		}

		CPath::getPathContent(sDir, false, false, true, AllMapNames);
	}

	vector<NLMISC::CBitmap*> AllMaps;
	sint32 j;

	// Load all maps
	sint32 mapSize = (sint32)AllMapNames.size();
	AllMaps.resize( mapSize );
	for(sint i = 0; i < mapSize; ++i )
	{
		NLMISC::CBitmap *pBtmp = NULL;

		try
		{
			pBtmp = new NLMISC::CBitmap;
			NLMISC::CIFile inFile;

			if (!inFile.open(AllMapNames[i])) throw NLMISC::Exception(toString("Unable to open %s", AllMapNames[i].c_str()));

			uint8 colors = pBtmp->load(inFile);

			if (!colors) throw NLMISC::Exception(toString("%s is not a bitmap", AllMapNames[i].c_str()));

			if (pBtmp->getPixelFormat() != CBitmap::RGBA)
			{
				outString(toString("Converting %s to RGBA (32 bits), originally using %u bits...", AllMapNames[i].c_str(), (uint)colors));
				pBtmp->convertToType(CBitmap::RGBA);
			}

			// duplicate icon border
			if (borderSize > 0)
			{
				NLMISC::CBitmap *tmp = new NLMISC::CBitmap;
				tmp->resize(pBtmp->getWidth(), pBtmp->getHeight());
				tmp->blit(pBtmp, 0, 0);
				// corners
				tmp->resample(tmp->getWidth() + borderSize * 2, tmp->getHeight() + borderSize * 2);
				// top, bottom
				tmp->blit(pBtmp, borderSize, 0);
				tmp->blit(pBtmp, borderSize, borderSize*2);
				// left, right
				tmp->blit(pBtmp, 0, borderSize);
				tmp->blit(pBtmp, borderSize*2, borderSize);
				// center
				tmp->blit(pBtmp, borderSize, borderSize);

				delete pBtmp;
				pBtmp = tmp;
			}

			AllMaps[i] = pBtmp;
		}
		catch (const NLMISC::Exception &e)
		{
			if (pBtmp) delete pBtmp;

			outString(toString("ERROR : %s", e.what()));
			return -1;
		}
	}

	// Sort all maps by decreasing size
	for (sint i = 0; i < mapSize-1; ++i)
	for (j = i+1; j < mapSize; ++j)
	{
		NLMISC::CBitmap *pBI = AllMaps[i];
		NLMISC::CBitmap *pBJ = AllMaps[j];
		if ((pBI->getWidth()*pBI->getHeight()) < (pBJ->getWidth()*pBJ->getHeight()))
		{
			NLMISC::CBitmap *pBTmp = AllMaps[i];
			AllMaps[i] = AllMaps[j];
			AllMaps[j] = pBTmp;

			string sTmp = AllMapNames[i];
			AllMapNames[i] = AllMapNames[j];
			AllMapNames[j] = sTmp;
		}
	}

	// Place all maps into the global texture
	NLMISC::CBitmap GlobalTexture, GlobalMask;
	GlobalTexture.resize (1, 1, NLMISC::CBitmap::RGBA);
	GlobalMask.resize (1, 1, NLMISC::CBitmap::RGBA);
	CObjectVector<uint8> &rPixelBitmap = GlobalTexture.getPixels(0);
	rPixelBitmap[0] = rPixelBitmap[1] = rPixelBitmap[2] = rPixelBitmap[3] = 0;
	CObjectVector<uint8> &rPixelMask = GlobalMask.getPixels(0);
	rPixelMask[0] = rPixelMask[1] = rPixelMask[2] = rPixelMask[3] = 0;
	vector<NLMISC::CUV> UVMin, UVMax;
	UVMin.resize (mapSize, NLMISC::CUV(0.0f, 0.0f));
	UVMax.resize (mapSize, NLMISC::CUV(0.0f, 0.0f));

	for (sint i = 0; i < mapSize; ++i)
	{
		sint32 x, y;
		while (!tryAllPos(AllMaps[i], &GlobalMask, x, y))
		{
			// Enlarge global texture
			enlargeCanvas (GlobalTexture);
			enlargeCanvas (GlobalMask);
		}

		putIn (AllMaps[i], &GlobalTexture, x, y);
		putIn (AllMaps[i], &GlobalMask, x, y, false);

		UVMin[i].U = (float)x + borderSize;
		UVMin[i].V = (float)y + borderSize;
		UVMax[i].U = (float)x + AllMaps[i]->getWidth() - borderSize;
		UVMax[i].V = (float)y + AllMaps[i]->getHeight() - borderSize;

#if 0
		// Do not remove this is useful for debugging
		writeFileDependingOnFilename(fmtName.substr(0, fmtName.rfind('.')) + "_txt.png", GlobalTexture);
		writeFileDependingOnFilename(fmtName.substr(0, fmtName.rfind('.')) + "_msk.png", GlobalMask);
#endif
	}

	// Convert UV from pixel to ratio
	for (sint i = 0; i < mapSize; ++i)
	{
		UVMin[i].U = UVMin[i].U / (float)GlobalTexture.getWidth();
		UVMin[i].V = UVMin[i].V / (float)GlobalTexture.getHeight();
		UVMax[i].U = UVMax[i].U / (float)GlobalTexture.getWidth();
		UVMax[i].V = UVMax[i].V / (float)GlobalTexture.getHeight();
	}

	// make sure transparent pixels are black
	GlobalTexture.makeTransparentPixelsBlack();

	// Write global texture file
	if (writeFileDependingOnFilename(fmtName, GlobalTexture))
	{
		outString(toString("Writing %s", fmtName.c_str()));
	}
	else
	{
		outString(toString("ERROR: Unable to write %s", fmtName.c_str()));
	}

	// Write UV text file
	if( !buildSubset )
	{
		fmtName = fmtName.substr(0, fmtName.rfind('.'));
		fmtName += ".txt";
		FILE *f = nlfopen(fmtName, "wb");
		if (f != NULL)
		{
			for (sint i = 0; i < mapSize; ++i)
			{
				// get the string whitout path
				string fileName = CFile::getFilename(AllMapNames[i]);
				fprintf (f, "%s %.12f %.12f %.12f %.12f\n", fileName.c_str(), UVMin[i].U, UVMin[i].V, UVMax[i].U, UVMax[i].V);
			}

			fclose (f);

			outString(toString("Writing UV file %s", fmtName.c_str()));
		}
		else
		{
			outString(toString("ERROR: Cannot write UV file %s", fmtName.c_str()));
		}
	}
	else // build as a subset
	{
		// Load existing uv file
		CIFile iFile;
		string filename = CPath::lookup (existingUVfilename, false);

		if( filename.empty() || !iFile.open(filename) )
		{
			outString(toString("ERROR: Unable to open %s", existingUVfilename.c_str()));
			return -1;
		}
		
		// Write subset UV text file
		fmtName = fmtName.substr(0, fmtName.rfind('.'));
		fmtName += ".txt";
		FILE *f = nlfopen(fmtName, "wb");

		if (f == NULL)
		{
			outString(toString("ERROR: Unable to write UV file %s", fmtName.c_str()));
			return -1;
		}

		char bufTmp[256], tgaName[256];
		string sTGAname;
		float uvMinU, uvMinV, uvMaxU, uvMaxV;
		while (!iFile.eof())
		{
			iFile.getline (bufTmp, 256);
			if (sscanf (bufTmp, "%s %f %f %f %f", tgaName, &uvMinU, &uvMinV, &uvMaxU, &uvMaxV) != 5)
			{
				nlwarning("Can't parse %s", bufTmp);
				continue;
			}

			sTGAname = toLowerAscii(string(tgaName));

			// search position of extension
			std::string tgaExt = CFile::getExtension(sTGAname);

			// remove extension
			sTGAname = CFile::getFilenameWithoutExtension(sTGAname);

			sint i;
			
			string findTGAName;
			for (i = 0; i < mapSize; ++i)
			{
				// get the string whitout path
				findTGAName = toLowerAscii(CFile::getFilenameWithoutExtension(AllMapNames[i]));
				if( findTGAName == sTGAname )
					break;
			}

			// append extension
			sTGAname += "." + tgaExt;
			
			if( i == mapSize )
			{
				// not present in subset: offset existing uv's to (0,0), preserving size
				fprintf (f, "%s %.12f %.12f %.12f %.12f\n", sTGAname.c_str(), 0.0f, 0.0f, uvMaxU - uvMinU, uvMaxV - uvMinV); 
			}
			else
			{
				// present in subset: use new uv's
				fprintf (f, "%s %.12f %.12f %.12f %.12f\n", sTGAname.c_str(), UVMin[i].U, UVMin[i].V, UVMax[i].U, UVMax[i].V);
			}
		}	
		fclose (f);
		outString(toString("Writing UV file: %s", fmtName.c_str()));
	}
	
	return 0;
}
