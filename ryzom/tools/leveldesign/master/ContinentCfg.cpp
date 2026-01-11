// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

#include "stdafx.h"
#include "ContinentCfg.h"
#include "nel/misc/config_file.h"

using namespace NLMISC;
using namespace std;

// ---------------------------------------------------------------------------
SContinentCfg::SContinentCfg ()
{
}

// ---------------------------------------------------------------------------
bool SContinentCfg::load (const string &filename)
{
	if (!openRead(filename))
		return false;

	LandFile			= getStr ("LandFile");
	LandDir				= getStr ("LandDir");
	DfnDir				= getStr ("DfnDir");
	GameElemDir			= getStr ("GameElemDir");

	LandBankFile		= getStr ("LandBankFile");
	LandFarBankFile		= getStr ("LandFarBankFile");
	LandTileNoiseDir	= getStr ("LandTileNoiseDir");
	LandZoneWDir		= getStr ("LandZoneWDir");
	OutIGDir			= getStr ("OutIGDir");
	close();
	return true;
}

// ---------------------------------------------------------------------------
bool SContinentCfg::save (const string &filename)
{
	if (!openWrite(filename))
		return false;

	putCommentLine ("-------------");
	putCommentLine ("Continent Cfg");
	putCommentLine ("-------------");
	putStr	("LandFile",			LandFile);
	putStr	("LandDir",				LandDir);
	putStr	("DfnDir",				DfnDir);
	putStr	("GameElemDir",			GameElemDir);

	putStr	("LandBankFile",		LandBankFile);
	putStr	("LandFarBankFile",		LandFarBankFile);
	putStr	("LandTileNoiseDir",	LandTileNoiseDir);
	putStr	("LandZoneWDir",		LandZoneWDir);
	putStr	("OutIGDir",			OutIGDir);

	close ();
	return true;
}

