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

#include "stdmisc.h"
#include "nel/misc/ucstring.h"

std::string ucstring::toUtf8() const
{
	std::string	res;
	ucstring::const_iterator first(begin()), last(end());
	for (; first != last; ++first)
	{
		//ucchar	c = *first;
		uint nbLoop = 0;
		if (*first < 0x80)
			res += char(*first);
		else if (*first < 0x800)
		{
			ucchar c = *first;
			c = c >> 6;
			c = c & 0x1F;
			res += char(c) | 0xC0;
			nbLoop = 1;
		}
		else /*if (*first < 0x10000)*/
		{
			ucchar c = *first;
			c = c >> 12;
			c = c & 0x0F;
			res += char(c) | 0xE0;
			nbLoop = 2;
		}

		for (uint i=0; i<nbLoop; ++i)
		{
			ucchar	c = *first;
			c = c >> ((nbLoop - i - 1) * 6);
			c = c & 0x3F;
			res += char(c) | 0x80;
		}
	}
	return res;
}

void ucstring::fromUtf8(const std::string &stringUtf8)
{
	// clear the string
	erase();

	uint8 c;
	ucchar code;
	sint iterations = 0;

	std::string::const_iterator first(stringUtf8.begin()), last(stringUtf8.end());
	for (; first != last; )
	{
		c = *first++;
		code = c;

		if ((code & 0xFE) == 0xFC)
		{
			code &= 0x01;
			iterations = 5;
		}
		else if ((code & 0xFC) == 0xF8)
		{
			code &= 0x03;
			iterations = 4;
		}
		else if ((code & 0xF8) == 0xF0)
		{
			code &= 0x07;
			iterations = 3;
		}
		else if ((code & 0xF0) == 0xE0)
		{
			code &= 0x0F;
			iterations = 2;
		}
		else if ((code & 0xE0) == 0xC0)
		{
			code &= 0x1F;
			iterations = 1;
		}
		else if ((code & 0x80) == 0x80)
		{
			// If it's not a valid UTF8 string, just copy the line without utf8 conversion
			rawCopy(stringUtf8);
			return;
		}
		else
		{
			push_back(code);
			iterations = 0;
		}

		if (iterations)
		{
			for (sint i = 0; i < iterations; i++)
			{
				if (first == last)
				{
					// If it's not a valid UTF8 string, just copy the line without utf8 conversion
					rawCopy(stringUtf8);
					return;
				}

				uint8 ch;
				ch = *first ++;

				if ((ch & 0xC0) != 0x80)
				{
					// If it's not a valid UTF8 string, just copy the line without utf8 conversion
					rawCopy(stringUtf8);
					return;
				}

				code <<= 6;
				code |= (ucchar)(ch & 0x3F);
			}
			push_back(code);
		}
	}
}

void ucstring::rawCopy(const std::string &str)
{
	// We need to convert the char into 8bits unsigned int before promotion to 16 bits
	// otherwise, as char are signed on some compiler (MSCV for ex), the sign bit is extended to 16 bits.
	resize(str.size());
	std::string::const_iterator first(str.begin()), last(str.end());
	iterator dest(begin());
	for (;first != last; ++first, ++dest)
	{
		*dest = uint8(*first);
	}
}

/* end of file */
