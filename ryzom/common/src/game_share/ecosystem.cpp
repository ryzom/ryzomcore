// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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



#include "stdpch.h"

#include "nel/misc/debug.h"
#include "ecosystem.h"

using namespace std;

namespace ECOSYSTEM
{
	const static string UnknownString = "Unknown";
	const static string CommonString = "CommonEcosystem";
	const static string DesertString = "Desert";
	const static string ForestString = "Forest";
	const static string LacustreString = "Lacustre";
	const static string JungleString = "Jungle";
	const static string GooString = "Goo";
	const static string PrimaryRootString = "PrimaryRoot";

	//-----------------------------------------------
	// stringToEcosystem : case-unsensitive comparison
	//-----------------------------------------------
	EECosystem stringToEcosystem(const string &str)
	{
		string strL = NLMISC::toLowerAscii( str );
		if( strL == NLMISC::toLowerAscii( CommonString) ) return common_ecosystem;
		if( strL == NLMISC::toLowerAscii( DesertString ) ) return desert;
		if( strL == NLMISC::toLowerAscii( ForestString ) ) return forest;
		if( strL == NLMISC::toLowerAscii( LacustreString ) ) return lacustre;
		if( strL == NLMISC::toLowerAscii( JungleString ) ) return jungle;
		if( strL == NLMISC::toLowerAscii( GooString ) ) return goo;
		if( strL == NLMISC::toLowerAscii( PrimaryRootString ) ) return primary_root;
		return unknown;
	} // stringToEcosystem //


	//-----------------------------------------------
	// toString :
	//-----------------------------------------------
	const string & toString (EECosystem e)
	{
		switch( e )
		{
			case common_ecosystem:
				return CommonString;
			case desert:
				return DesertString;
			case forest:
				return ForestString;
			case lacustre:
				return LacustreString;
			case jungle:
				return JungleString;
			case goo:
				return GooString;
			case primary_root:
				return PrimaryRootString;
			default:
				return UnknownString;
		}
	} // toString  //
}; // ECOSYSTEM
