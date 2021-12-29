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

#include "stdnet.h"

#include "nel/misc/types_nl.h"

#include "nel/misc/debug.h"
#include "nel/misc/config_file.h"
#include "nel/misc/displayer.h"
#include "nel/misc/log.h"

#include "nel/net/varpath.h"


//
// Namespaces
//

using namespace std;
using namespace NLMISC;


//
// Variables
//



//
// Functions
//

/**
 *
 * VarPath ::= [bloc '.']* bloc
 * bloc    ::= '[' [VarPath ',']* VarPath ']'  |  name
 * name    ::= [ascii]* ['*']
 *
 *
 */
/*bool CVarPath::getDest (uint level, vector<string> &dest)
{
	return true;
}*/

string CVarPath::getToken ()
{
	string res;

	if (TokenPos >= RawVarPath.size())
		return res;

	res += RawVarPath[TokenPos];

	switch (RawVarPath[TokenPos++])
	{
	case '.': case '*': case '[': case ']': case ',': case '=': case ' ':
		break;
	default :
		while (TokenPos < RawVarPath.size() && RawVarPath[TokenPos] != '.' && RawVarPath[TokenPos] != '[' && RawVarPath[TokenPos] != ']' && RawVarPath[TokenPos] != ',' && RawVarPath[TokenPos] != '=' && RawVarPath[TokenPos] != ' ')
		{
			res += RawVarPath[TokenPos++];
		}
		break;
	}
	return res;
}


void CVarPath::decode ()
{
	vector<string> dest;
	TokenPos = 0;
	Destination.clear ();

	string val = getToken ();

	if (val.empty())
		return;

	if (val == "[" )
	{
		for(;;)
		{
			uint osbnb = 0;
			string d;
			for(;;)
			{
				val = getToken ();
				if (val == "[")
					osbnb++;

				// end of token
				if (val.empty())
				{
					nlwarning ("VP: Bad VarPath '%s', suppose it s an empty varpath", RawVarPath.c_str());
					Destination.clear ();
					return;
				}

				if (osbnb == 0 && (val == "," || val == "]"))
					break;

				if (val == "]")
					osbnb--;

				d += val;
			}
			dest.push_back (d);
			if (val == "]")
				break;
		}
	}
	else if (val != "." && val != "," && val != "]")
	{
		dest.push_back (val);
	}
	else
	{
		nlwarning ("VP: Malformated VarPath '%s' before position %d", RawVarPath.c_str (), TokenPos);
		return;
	}

	// must be a . or end of string
	val = getToken ();
	if (val == " ")
	{
		// special case, there s a space that means that everything after is not really part of the varpath.
		Destination.push_back (make_pair(RawVarPath, string("")));
		return;
	}
	else if (val != "." && !val.empty() && val != "=")
	{
		nlwarning ("VP: Malformated VarPath '%s' before position %d", RawVarPath.c_str (), TokenPos);
		return;
	}

	for (uint i = 0; i < dest.size(); ++i)
	{
		string srv, var;
		string::size_type pos;

		if ((pos = dest[i].find ('.')) != string::npos)
		{
			srv = dest[i].substr(0, pos);
			var = dest[i].substr(pos+1);
			if (TokenPos < RawVarPath.size())
				var += val + RawVarPath.substr (TokenPos);
		}
		else
		{
			srv = dest[i];
			if (val == "=")
			{
				srv += val + RawVarPath.substr (TokenPos);
				var.clear();
			}
			else
				var = RawVarPath.substr (TokenPos);
		}

		Destination.push_back (make_pair(srv, var));
	}

	//display ();
}

bool CVarPath::isFinal ()
{
	if(Destination.empty()) return true;
	if(Destination[0].second.empty()) return true;
	return false;
}

void CVarPath::display ()
{
	nlinfo ("VP: VarPath dest = %d", Destination.size ());
	for (uint i = 0; i < Destination.size (); i++)
	{
		nlinfo ("VP:  > '%s' '%s'", Destination[i].first.c_str(), Destination[i].second.c_str());
	}
}

NLMISC_CATEGORISED_COMMAND(nel, varPath, "Test a varpath (for debug purpose)", "<rawvarpath>")
{
	nlunreferenced(rawCommandString);
	nlunreferenced(quiet);
	nlunreferenced(human);

	if(args.size() != 1) return false;

	CVarPath vp (args[0]);

	log.displayNL ("VarPath contains %d destination", vp.Destination.size ());
	for (uint i = 0; i < vp.Destination.size (); i++)
	{
		log.displayNL ("Dest '%s' value '%s'", vp.Destination[i].first.c_str(), vp.Destination[i].second.c_str());
	}
	log.displayNL ("End of varpath");

	return true;
}
