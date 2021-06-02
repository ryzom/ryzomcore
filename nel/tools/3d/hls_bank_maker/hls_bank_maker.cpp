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

#include "hls_bank_maker.h"
#include "nel/3d/hls_texture_bank.h"
#include "../panoply_maker/hls_bank_texture_info.h"
#include "nel/misc/path.h"
#include "nel/misc/file.h"
#include <math.h>


using namespace std;
using namespace NLMISC;
using namespace NL3D;


// ***************************************************************************
/*
	Be aware that This project use and compile ../panoply_maker/hls_bank_texture_info.*
*/


// ***************************************************************************
void	addTextToBank(CHLSBankTextureInfo &textInfo, CHLSTextureBank &textBank)
{
	uint	i;

	// **** Build the color texture
	CHLSColorTexture	colText;
	CBitmap		bmp;
	// set the DXTC5 bmp.
	textInfo.SrcBitmap.buildBitmap(bmp);
	colText.setBitmap(bmp);
	// Add masks.
	for(i=0;i<textInfo.Masks.size();i++)
	{
		textInfo.Masks[i].buildBitmap(bmp);
		colText.addMask(bmp);
	}

	// Add it to the bank.
	uint	colTextid= textBank.addColorTexture(colText);

	// **** Add the texture instances.
	for(i=0;i<textInfo.Instances.size();i++)
	{
		CHLSBankTextureInfo::CTextureInstance	&instIn= textInfo.Instances[i];
		vector<CHLSColorDelta>	cols;
		cols.resize(instIn.Mods.size());
		for(uint j=0;j<cols.size();j++)
		{
			// compress Hue from 0..360 to 0..255
			float	dHue= (float)fmod(instIn.Mods[j].DHue, 360);
			dHue= (float)fmod(dHue+360, 360);
			cols[j].DHue= (uint8)(uint)floor(256*dHue/360);
			// compress DLum from -1..+1 to -127..+127
			float	v= 127*instIn.Mods[j].DLum;
			clamp(v, -127, 127);
			cols[j].DLum= (sint8)floor(v);
			// compress DSat from -1..+1 to -127..+127
			v= 127*instIn.Mods[j].DSat;
			clamp(v, -127, 127);
			cols[j].DSat= (sint8)floor(v);
		}

		// add the instance to the bank.
		textBank.addTextureInstance(instIn.Name, colTextid, cols);
	}

}


// ***************************************************************************
int		main(int argc, char *argv[])
{
	// Filter addSearchPath
	NLMISC::createDebug();
	NLMISC::InfoLog->addNegativeFilter ("adding the path");

	if (argc != 3)
	{
		nlwarning("usage : %s hlsinfo_dir output_name ", argv[0]);
		exit(-1);
	}

	// get all .hlsinfo file in directory.
	vector<string>		files;
	vector<string>		hlsInfofiles;
	NLMISC::CPath::getPathContent(argv[1], false, false, true, files);
	hlsInfofiles.reserve(files.size());
	uint	k;
	for (k = 0;  k < files.size(); ++k)
	{
		std::string fileExt = NLMISC::strupr(NLMISC::CFile::getExtension(files[k]));						
		if(fileExt=="HLSINFO")
			hlsInfofiles.push_back(files[k]);
	}

	//  If none, quit.
	if(hlsInfofiles.empty())
		exit(-1);

	// Concat all hlsinfo in a Bank
	CHLSTextureBank		textBank;
	for (k = 0;  k < hlsInfofiles.size(); ++k)
	{
		printf("HLSBank Process [%2d]\r", (uint)(100*k/hlsInfofiles.size()));
		try
		{
			CIFile	f(hlsInfofiles[k]);
			CHLSBankTextureInfo	textInfo;
			f.serial(textInfo);
			addTextToBank(textInfo, textBank);
		}
		catch(const Exception &e)
		{
			nlwarning("ERROR: Unable to process %s. Reason: %s. Processing next", hlsInfofiles[k].c_str(), e.what());
		}
	}

	// compile it
	textBank.compile();

	// save the bank.
	COFile	fOut;
	try
	{
		if(!fOut.open(argv[2]))
			throw int(0);
		fOut.serial(textBank);
		fOut.close();
	}
	catch(const Exception &e)
	{
		nlwarning("ERROR: Unable to write HLS Bank %s: %s", argv[2], e.what());
		exit(-1);
	}
	catch(...)
	{
		nlwarning("ERROR: Unable to write HLS Bank %s.", argv[2]);
		exit(-1);
	}

	return 0;
}