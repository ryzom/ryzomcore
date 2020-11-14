// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2019-2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef NL_SHA1_H
#define NL_SHA1_H

#include "types_nl.h"
#include "common.h"
#include "stream.h"

#include <string>

namespace NLMISC {

struct CHashKey
{
	CHashKey() { HashKeyString.resize(20); }

	CHashKey(const unsigned char Message_Digest[20])
	{
		HashKeyString.clear();
		for (sint i = 0; i < 20; ++i)
		{
			HashKeyString += Message_Digest[i];
		}
	}

	// Init the hash key with a binary key format or a text key format
	CHashKey(const std::string &str)
	{
		if (str.size() == 20)
		{
			HashKeyString = str;
		}
		else if (str.size() == 40)
		{
			HashKeyString.clear();
			for (size_t i = 0; i < str.size(); i += 2)
			{
				uint8 val;
				if (isdigit((unsigned char)str[i + 0]))
					val = str[i + 0] - '0';
				else
					val = 10 + tolower(str[i + 0]) - 'a';
				val *= 16;
				if (isdigit((unsigned char)str[i + 1]))
					val += str[i + 1] - '0';
				else
					val += 10 + tolower(str[i + 1]) - 'a';

				HashKeyString += val;
			}
		}
		else
		{
			nlwarning("SHA: Bad hash key format");
		}
	}

	// return the hash key in text format
	std::string toString() const
	{
		std::string str;
		for (uint i = 0; i < HashKeyString.size(); i++)
		{
			str += NLMISC::toString("%02X", (uint8)(HashKeyString[i]));
		}
		return str;
	}

	// serial the hash key in binary format
	void serial(NLMISC::IStream &stream)
	{
		stream.serial(HashKeyString);
	}

	bool operator==(const CHashKey &v) const
	{
		return HashKeyString == v.HashKeyString;
	}

	bool operator!=(const CHashKey &v) const
	{
		return !(*this == v);
	}

	// this string is always 20 bytes long and is the code in binary format (can't print it directly)
	std::string HashKeyString;
};

inline bool operator<(const struct CHashKey &a, const struct CHashKey &b)
{
	return a.HashKeyString < b.HashKeyString;
}

// This function get a filename (it works with big files) and returns his SHA hash key
// when forcePath is true, it doesn't use NLMISC::lookup
CHashKey getSHA1(const std::string &filename, bool forcePath = false);

// This function get a buffer with size and returns his SHA hash key
CHashKey getSHA1(const uint8 *buffer, uint32 size);

// This function get a buffer and key with size and returns his HMAC-SHA1 hash key
CHashKey getHMacSHA1(const uint8 *text, uint32 text_len, const uint8 *key, uint32 key_len);

}

#endif // NL_SHA1_H
