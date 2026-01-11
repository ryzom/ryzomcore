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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <string>
#include <vector>
#include <iostream>
using namespace std;

#include "nel/misc/types_nl.h"
#include "nel/misc/string_stream.h"
#include "nel/misc/common.h"
#include "nel/misc/debug.h"
using namespace NLMISC;

template <class T> bool check (T value)
{
	T result;
	CMemStream msgout;
	msgout.serial (value);
	CMemStream msgin (true);
	msgin.fill (msgout.buffer(), msgout.size());
	msgin.serial (result);
	
	if (value != result)
	{
		nlwarning ("CHECK FAILED: %s != %s", toString (value).c_str(), toString (result).c_str());
	}

	if (toString(value) != toString(result))
	{
		nlwarning ("toString() CHECK FAILED: %s != %s", toString (value).c_str(), toString (result).c_str());
	}
		
	return value == result;
}


#define checkInt(__type, __start, __end, __step) \
{ \
	__type t = (__type)(__start); \
	nlinfo ("checking "#__type" first %s", toString (t).c_str()); \
	for (; t < (__type)(__end); t += (__type)(__step)) \
		check ((__type)(t)); \
	nlinfo ("checking "#__type" last %s", toString (t).c_str()); \
	check ((__type)(t)); \
}



void checkInts ()
{
	checkInt (uint8, 0, 255, 1);
	checkInt (sint8, -128, 127, 1);

	checkInt (uint16, 0, 65535, 1);
	checkInt (sint16, -32768, 32767, 1);

	checkInt (uint32, 0, 65535, 1);
	checkInt (uint32, 4294967295-65535, 4294967295, 1);

	checkInt (sint32, -2147483648, -2147483648+65535, 1);
	checkInt (sint32, 2147483647-65535, 2147483647, 1);

	checkInt (uint32, 0, 4294967295, 65535);
	checkInt (sint32, -2147483648, 2147483647, 65535);

	uint64 start, end;
	end=0; end--; start=end-65535;
	checkInt (uint64, 0, 65535, 1);
	checkInt (uint64, start, end, 1);

	checkInt (sint64, -9223372036854775808, -9223372036854775808+65535, 1);
	checkInt (sint64, 9223372036854775807-65535, 9223372036854775807, 1);
}


int main (int argc, char **argv)
{
	if (sizeof (uint) != sizeof (void*)) nlwarning ("sizeof (uint) != sizeof (void*) : %d != %d", sizeof (uint), sizeof (void*));

	if (sizeof (uint8) != 1) nlwarning ("sizeof (uint8) != 1 : %d != %d", sizeof (uint8), 1);
	if (sizeof (sint8) != 1) nlwarning ("sizeof (sint8) != 1 : %d != %d", sizeof (sint8), 1);
	if (sizeof (uint16) != 2) nlwarning ("sizeof (uint16) != 2 : %d != %d", sizeof (uint16), 2);
	if (sizeof (sint16) != 2) nlwarning ("sizeof (sint16) != 2 : %d != %d", sizeof (sint16), 2);
	if (sizeof (uint32) != 4) nlwarning ("sizeof (uint32) != 4 : %d != %d", sizeof (uint32), 4);
	if (sizeof (sint32) != 4) nlwarning ("sizeof (sint32) != 4 : %d != %d", sizeof (sint32), 4);
	if (sizeof (uint64) != 8) nlwarning ("sizeof (uint64) != 8 : %d != %d", sizeof (uint64), 8);
	if (sizeof (sint64) != 8) nlwarning ("sizeof (sint64) != 8 : %d != %d", sizeof (sint64), 8);
	
	checkInts ();

	printf ("\nPress <return> to exit\n");
	getchar ();

	return EXIT_SUCCESS;
}
