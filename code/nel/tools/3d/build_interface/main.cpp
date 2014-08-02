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

//#include "windows.h"

#include <vector>
#include <string>

// ---------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;

// ***************************************************************************
//char sExeDir[MAX_PATH];
std::string sExeDir;
NLMISC::CApplicationContext _ApplicationContext;

void outString (const string &sText)
{
	std::string sCurDir = CPath::getCurrentPath();
	CPath::setCurrentPath(sExeDir.c_str());
	//char sCurDir[MAX_PATH];
	//GetCurrentDirectory (MAX_PATH, sCurDir);
	//SetCurrentDirectory (sExeDir);
	NLMISC::createDebug ();
	NLMISC::InfoLog->displayRaw(sText.c_str());
	//SetCurrentDirectory (sCurDir);
	CPath::setCurrentPath(sCurDir.c_str());
}

// ***************************************************************************
// test every 4 pixels for 2 reason: DXTC and speed
const	uint32	posStep= 4;

// ***************************************************************************
// Try all position to put pSrc in pDst
bool tryAllPos (NLMISC::CBitmap *pSrc, NLMISC::CBitmap *pDst, sint32 &x, sint32 &y)
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
void	putPixel(uint8 *dst, uint8 *src, bool alphaTransfert)
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
bool putIn (NLMISC::CBitmap *pSrc, NLMISC::CBitmap *pDst, sint32 x, sint32 y, bool alphaTransfert=true)
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
string getBaseName (const string &fullname)
{
	string sTmp2;
	string::size_type pos = fullname.rfind('_');
	if (pos != string::npos)
		sTmp2 = fullname.substr(0, pos+1);
	return sTmp2;
}

// ***************************************************************************
// resize the bitmap to the next power of 2 and preserve content
void enlargeCanvas (NLMISC::CBitmap &b)
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

// ***************************************************************************
// main
// ***************************************************************************
int main(int nNbArg, char **ppArgs)
{
	//GetCurrentDirectory (MAX_PATH, sExeDir);
	sExeDir = CPath::getCurrentPath();

	if (nNbArg < 3)
	{
		outString ("ERROR : Wrong number of arguments\n");
		outString ("USAGE : build_interface [-s<existing_uv_txt_name>] <out_tga_name> <path_maps1> [path_maps2] [path_maps3] ....\n");
		outString ("   -s : build a subset of an existing interface definition while preserving the existing texture ids,");
		outString (" to support freeing up VRAM by switching to the subset without rebuilding the entire interface\n");
		return -1;
	}
	
	// build as a subset of existing interface
	bool buildSubset = false;
	string existingUVfilename;
	list<string> inputDirs;
	for ( uint i=1; (sint)i<nNbArg; ++i )
	{
		if ( ppArgs[i][0] == '-' )
		{
			switch ( ppArgs[i][1] )
			{
			case 'S':
			case 's':
				buildSubset = true;
				existingUVfilename = string( ppArgs[i]+2 );
				break;
			default:
				break;
			}
		}
		else
			inputDirs.push_back(ppArgs[i]);
	}

	string fmtName;
	uint iNumDirs =  (uint)inputDirs.size(); 
	if( iNumDirs )
	{
		fmtName = inputDirs.front();
		inputDirs.pop_front();
		--iNumDirs;
	}
	vector<string> AllMapNames;
	list<string>::iterator it = inputDirs.begin();
	list<string>::iterator itEnd = inputDirs.end();
	while( it != itEnd )
	{
		string sDir = *it++;
		if( !CFile::isDirectory(sDir) )
		{
			outString (string("ERROR : directory ") + sDir + " does not exist\n");
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
		try
		{
			NLMISC::CBitmap *pBtmp = new NLMISC::CBitmap;
			NLMISC::CIFile inFile;
			inFile.open( AllMapNames[i] );
			pBtmp->load(inFile);
			AllMaps[i] = pBtmp;
		}
		catch (const NLMISC::Exception &e)
		{
			outString (string("ERROR :") + e.what());
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
		UVMin[i].U = (float)x;
		UVMin[i].V = (float)y;
		UVMax[i].U = (float)x+AllMaps[i]->getWidth();
		UVMax[i].V = (float)y+AllMaps[i]->getHeight();

		/* // Do not remove this is useful for debugging
		{
			NLMISC::COFile outTga;
			string fmtName = ppArgs[1];
			if (fmtName.rfind('.') == string::npos)
				fmtName += ".tga";
			if (outTga.open(fmtName))
			{
				GlobalTexture.writeTGA (outTga, 32);
				outTga.close();
			}
		}
		{
			NLMISC::COFile outTga;
			string fmtName = ppArgs[1];
			if (fmtName.rfind('.') == string::npos)
				fmtName += "_msk.tga";
			else
				fmtName = fmtName.substr(0,fmtName.rfind('.')) + "_msk.tga";
			if (outTga.open(fmtName))
			{
				GlobalMask.writeTGA (outTga, 32);
				outTga.close();
			}
		}*/


	}

	// Convert UV from pixel to ratio
	for (sint i = 0; i < mapSize; ++i)
	{
		UVMin[i].U = UVMin[i].U / (float)GlobalTexture.getWidth();
		UVMin[i].V = UVMin[i].V / (float)GlobalTexture.getHeight();
		UVMax[i].U = UVMax[i].U / (float)GlobalTexture.getWidth();
		UVMax[i].V = UVMax[i].V / (float)GlobalTexture.getHeight();
	}

	// Write global texture file
	//SetCurrentDirectory (sExeDir);
	CPath::setCurrentPath(sExeDir.c_str());

	NLMISC::COFile outTga;
	if (fmtName.rfind('.') == string::npos)
		fmtName += ".tga";
	if (outTga.open(fmtName))
	{
		std::string ext;
		if (toLower(fmtName).find(".png") != string::npos)
		{
			ext = "png";
			GlobalTexture.writePNG (outTga, 32);
		}
		else
		{
			ext = "tga";
			GlobalTexture.writeTGA (outTga, 32);
		}

		outTga.close();
		outString (toString("Writing %s file : %s\n", ext.c_str(), fmtName.c_str()));
	}
	else
	{
		outString (string("ERROR: Cannot write tga file : ") + fmtName + "\n");
	}

	// Write UV text file
	if( !buildSubset )
	{
		fmtName = fmtName.substr(0, fmtName.rfind('.'));
		fmtName += ".txt";
		FILE *f = fopen (fmtName.c_str(), "wt");
		if (f != NULL)
		{
			for (sint i = 0; i < mapSize; ++i)
			{
				// get the string whitout path
				string fileName= CFile::getFilename(AllMapNames[i]);
				fprintf (f, "%s %.12f %.12f %.12f %.12f\n", fileName.c_str(), UVMin[i].U, UVMin[i].V, 
												UVMax[i].U, UVMax[i].V);
			}
			fclose (f);
			outString (string("Writing UV file : ") + fmtName + "\n");
		}
		else
		{
			outString (string("ERROR: Cannot write UV file : ") + fmtName + "\n");
		}
	}
	else // build as a subset
	{
		// Load existing uv file
		CIFile iFile;
		string filename = CPath::lookup (existingUVfilename, false);
		if( (filename == "") || (!iFile.open(filename)) )
		{
			outString (string("ERROR : could not open file ") + existingUVfilename + "\n");
			return -1;
		}
		
		// Write subset UV text file
		fmtName = fmtName.substr(0, fmtName.rfind('.'));
		fmtName += ".txt";
		FILE *f = fopen (fmtName.c_str(), "wt");
		if (f == NULL)
		{
			outString (string("ERROR: Cannot write UV file : ") + fmtName + "\n");
//			fclose (iFile);
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

			sint i;
			
			sTGAname = toLower(string(tgaName));
			string findTGAName;
			for (i = 0; i < mapSize; ++i)
			{
				// get the string whitout path
				findTGAName = toLower(CFile::getFilename(AllMapNames[i]));
				if( findTGAName == sTGAname )
					break;
			}
			
			if( i == mapSize )
			{
				// not present in subset: offset existing uv's to (0,0), preserving size
				fprintf (f, "%s %.12f %.12f %.12f %.12f\n", sTGAname.c_str(), 0.0f, 0.0f, uvMaxU - uvMinU, uvMaxV - uvMinV); 
			}
			else
			{
				// present in subset: use new uv's
				fprintf (f, "%s %.12f %.12f %.12f %.12f\n", sTGAname.c_str(), UVMin[i].U, UVMin[i].V, 
					UVMax[i].U, UVMax[i].V);
			}
		}	
//		fclose (iFile);
		fclose (f);
		outString (string("Writing UV file : ") + fmtName + "\n");
	}
	
	return 0;
}
