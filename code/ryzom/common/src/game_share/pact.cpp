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



/////////////
// INCLUDE //
/////////////
#include "stdpch.h"	// First include for pre-compiled headers.
#include "pact.h"

using namespace std;

namespace GSPACT
{

static std::string pactNatureStrings[]=
{
	"Unknown",
	"Kamique",
	"Caravane",
};

static std::string pactTypeStrings[]=
{
	"Type1",
	"Type2",
	"Type3",
	"Type4",
	"Type5",
	"unknown"
};

const std::string& toString (EPactNature pactNature)
{
	if ((uint)pactNature >= sizeof(pactNatureStrings)/sizeof(string) )
	{
		nlwarning("<toString (EPactNature pactNature) : invalid pact nature %d",pactNature);
		return pactNatureStrings[(uint)Unknown];
	}
	return pactNatureStrings[(uint)pactNature];
}

const std::string& toString (EPactType pactType)
{
	if ((uint)pactType >= sizeof(pactTypeStrings)/sizeof(string) )
	{
		nlwarning("<toString (EPactType pactType) : invalid pact type %d",pactType);
		return pactTypeStrings[(uint)UnknownType];
	}
	return pactTypeStrings[(uint)pactType];
}


};// namespace GSPACT
