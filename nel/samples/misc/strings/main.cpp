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


#include "nel/misc/types_nl.h"

#include <cstdlib>

#include "nel/misc/string_common.h"

using namespace std;
using namespace NLMISC;


int main (int argc, char **argv)
{
	{
		uint8 val;
		val = 0; printf("'%d' should be equal to '%s' \n", val, toString(val).c_str());
		val = -1; printf("'%d' should be equal to '%s' \n", val, toString(val).c_str());
		val = 1; printf("'%d' should be equal to '%s' \n", val, toString(val).c_str());
		val = 255; printf("'%d' should be equal to '%s' \n", val, toString(val).c_str());
	}

	{
		sint8 val;
		val = 0; printf("'%d' should be equal to '%s' \n", val, toString(val).c_str());
		val = -128; printf("'%d' should be equal to '%s' \n", val, toString(val).c_str());
		val = 1; printf("'%d' should be equal to '%s' \n", val, toString(val).c_str());
		val = 127; printf("'%d' should be equal to '%s' \n", val, toString(val).c_str());
	}

	{
		uint16 val;
		val = -12158; printf("'%d' should be equal to '%s' \n", val, toString(val).c_str());
		val = 12557; printf("'%d' should be equal to '%s' \n", val, toString(val).c_str());
	}

	{
		sint16 val;
		val = -12158; printf("'%d' should be equal to '%s' \n", val, toString(val).c_str());
		val = 12557; printf("'%d' should be equal to '%s' \n", val, toString(val).c_str());
	}

	{
		uint32 val;
		val = -121556418; printf("'%u' should be equal to '%s' \n", val, toString(val).c_str());
		val = 125848847; printf("'%u' should be equal to '%s' \n", val, toString(val).c_str());
	}

	{
		sint32 val;
		val = -120547158; printf("'%d' should be equal to '%s' \n", val, toString(val).c_str());
		val = 12555857; printf("'%d' should be equal to '%s' \n", val, toString(val).c_str());
	}

	string str;
	str += "This is an example of the use of toString(). It's a number " + toString(156);
	str += " and now a float " + toString(15.55f);

	printf("%s\n", str.c_str());

	printf ("\nPress <return> to exit\n");
	getchar ();

	return EXIT_SUCCESS;
}
