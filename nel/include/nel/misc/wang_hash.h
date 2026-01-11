// Public domain hash functions

#ifndef NLMISC_WANG_HASH_H
#define NLMISC_WANG_HASH_H

#include "types_nl.h"

namespace NLMISC {

// http://burtleburtle.net/bob/hash/integer.html
inline uint32 wangHash(uint32 a)
{
	a = (a ^ 61) ^ (a >> 16);
	a = a + (a << 3);
	a = a ^ (a >> 4);
	a = a * 0x27d4eb2d;
	a = a ^ (a >> 15);
	return a;
}

// http://naml.us/blog/2012/03
inline uint64 wangHash64(uint64 key)
{
	key = (~key) + (key << 21); // key = (key << 21) - key - 1;
	key = key ^ (key >> 24);
	key = (key + (key << 3)) + (key << 8); // key * 265
	key = key ^ (key >> 14);
	key = (key + (key << 2)) + (key << 4); // key * 21
	key = key ^ (key >> 28);
	key = key + (key << 31);
	return key;
}

// http://naml.us/blog/2012/03 Inverse 64-bit wang hash
inline uint64 wangHash64Inv(uint64 key)
{
	uint64 tmp;

	// Invert key = key + (key << 31)
	tmp = key - (key << 31);
	key = key - (tmp << 31);

	// Invert key = key ^ (key >> 28)
	tmp = key^key >> 28;
	key = key^tmp >> 28;

	// Invert key *= 21
	key *= UINT64_CONSTANT(14933078535860113213);

	// Invert key = key ^ (key >> 14)
	tmp = key^key >> 14;
	tmp = key^tmp >> 14;
	tmp = key^tmp >> 14;
	key = key^tmp >> 14;

	// Invert key *= 265
	key *= UINT64_CONSTANT(15244667743933553977);

	// Invert key = key ^ (key >> 24)
	tmp = key^key >> 24;
	key = key^tmp >> 24;

	// Invert key = (~key) + (key << 21)
	tmp = ~key;
	tmp = ~(key - (tmp << 21));
	tmp = ~(key - (tmp << 21));
	key = ~(key - (tmp << 21));

	return key;
}

} /* namespace NLMISC */

#endif // NLMISC_WANG_HASH_H
