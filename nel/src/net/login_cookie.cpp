// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2023  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include "nel/net/login_cookie.h"
#include "nel/misc/wang_hash.h"

using namespace std;
using namespace NLMISC;


namespace NLNET {


/*
 * Comparison == operator
 */
bool operator== (const CLoginCookie &c1, const CLoginCookie &c2)
{
	nlassert (c1._Valid && c2._Valid);

	return c1._UserAddr==c2._UserAddr && c1._UserKey==c2._UserKey && c1._UserId==c2._UserId;
}

/*
 * Comparison != operator
 */
bool operator!= (const CLoginCookie &c1, const CLoginCookie &c2)
{
	return !(c1 == c2);
}

CLoginCookie::CLoginCookie (uint32 addr, uint32 id) : _Valid(true), _UserAddr(addr), _UserId(id)
{
	// generates the key for this cookie
	_UserKey = generateKey();
}

uint32 CLoginCookie::generateKey()
{
	// This is a very poor random number generator that ensures no duplicates are generated within the same hour
	
	static uint32 salt0;
	static uint32 salt1;
	static bool seeded;

	uint32 t = (uint32)time(NULL);

	if (!seeded)
	{
		// Generate a not very random number as an extra salt to obfuscate the key
#ifdef NL_CPP14
		srand(std::random_device()());
#else
		srand(wangHash(t));
#endif
		salt0 = wangHash(rand() | rand() << 8 | rand() << 16 | rand() << 24);
		salt1 = wangHash(rand() | rand() << 8 | rand() << 16 | rand() << 24);
		seeded = true;
	}

	// Random number and counter
	uint32 r = rand(); // FIXME: Not thread safe!
	static uint32 n = 0;
	n += rand() & 3;

	// Time moves the counter forward, but the counter may go ahead of time
	t <<= 12;
	if (t > n)
	{
		// Move forward along with extra random bits
		n = (t + (rand() & 0xFFF));
	}

	// 12bits for the time (in second) => loop in 1 hour
	// 12bits for the inc number => can generate 1024 keys per second without any problem (if you generate more than this number, time will just go faster)
	//  8bits for random => 256 case
	// double salted for obfuscating
	return wangHash(((n & 0xFFFFFF) << 8 | (r & 0xFF)) ^ salt0) ^ salt1;
}


/* test key generation
void main()
{
	set<uint32> myset;

	// generates the key for this cookie
	for(;;)
	{
		uint32 val = (t&0xFFF)<<20 | (r&0xFF)<<12 | (n&0xFFF);
		pair<set<uint32>::iterator,bool> p = myset.insert (val);
		if (!p.second) printf("%10u 0x%x already inserted\n", val, val);
	}
}
*/

} // NL.
