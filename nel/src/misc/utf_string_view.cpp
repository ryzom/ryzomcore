// NeL - MMORPG Framework <https://wiki.ryzom.dev/>
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

#include "stdmisc.h"

// Project includes
#include <nel/misc/utf_string_view.h>
#include <nel/misc/stream.h>

// References:
// - https://twiserandom.com/unicode/unicode-encoding-utf-8-utf-16-utf-32/
// - https://www.compart.com/en/unicode/U+1F30D
//   - 0xF0 0x9F 0x8C 0x8D
//   - 0xD83C 0xDF0D
//   - 0x0001F30D

namespace NLMISC
{

NL_FORCE_INLINE void appendUtf8(std::string &str, u32char c)
{
	if (c < 0x80)
	{
		// Encode as 1 byte
		str += (char)c;
	}
	else if (c < 0x0800)
	{
		// Encode as 2 bytes
		str += (char)((c & 0x07C0) >> 6) | 0xC0;
		str += (char)(c & 0x3F) | 0x80;
	}
	else if (c < 0x010000)
	{
		// Encode as 3 bytes
		str += (char)((c & 0xF000) >> 12) | 0xE0;
		str += (char)((c & 0x0FC0) >> 6) | 0x80;
		str += (char)(c & 0x3F) | 0x80;
	}
	else if (c < 0x110000)
	{
		// Encode as 4 bytes
		str += (char)((c & 0x1C0000) >> 18) | 0xF0;
		str += (char)((c & 0x03F000) >> 12) | 0x80;
		str += (char)((c & 0x0FC0) >> 6) | 0x80;
		str += (char)(c & 0x3F) | 0x80;
	}
	else
	{
		// Replacement character �
		str += "\xEF\xB\xBD";
	}
}

void CUtfStringView::append(std::string &str, u32char c)
{
	appendUtf8(str, c);
}

void CUtfStringView::append(IStream &s, u32char c)
{
	nlassert(!s.isReading());
	std::string tmp;
	tmp.reserve(4);
	append(tmp, c);
	s.serialBuffer((uint8 *)&tmp[0], tmp.size());
}

u32char CUtfStringView::get(IStream &s)
{
	nlassert(s.isReading());

	std::string tmp;
	tmp.reserve(4);
	uint8 c;
	s.serial(c);

	// Single byte
	if (c < 0x80)
		return c;

	// Do a fast check of length
	tmp += (char)c;
	size_t len;
	if ((c & 0xF0) == 0xF0) len = 4;
	if ((c & 0xE0) == 0xE0) len = 3;
	else len = 2;

	// Read from stream
	tmp.resize(len);
	s.serialBuffer((uint8 *)&tmp[1], len - 1);

	// Decode
	const void *str = tmp.c_str();
	return utf8Iterator(&str);
}

std::string CUtfStringView::toUtf8(bool reEncode) const
{
	// Decode UTF and encode UTF-8
	// This implementation makes no attempt at fixing invalid codepoints
	if (m_Iterator == utf8Iterator && !reEncode)
		return std::string((const char *)m_Str, (const char *)((ptrdiff_t)m_Str + m_Size));
	std::string res;
	res.reserve(m_Size);
	for (iterator it(begin()), end(this->end()); it != end; ++it)
	{
		appendUtf8(res, *it);
	}
	return res;
}

ucstring CUtfStringView::toUtf16(bool reEncode) const
{
	if (m_Iterator == utf16Iterator && !reEncode)
		return ucstring((const ucchar *)m_Str, (const ucchar *)((ptrdiff_t)m_Str + m_Size));
	ucstring res;
	res.reserve(m_Size << 1);
	for (iterator it(begin()), end(this->end()); it != end; ++it)
	{
		u32char c = *it;
		if (c < 0x10000)
		{
			res += c;
		}
		else
		{
			c -= 0x10000;
			res += (c >> 10) | 0xD800;
			res += (c & 0x3FF) | 0xDC00;
		}
	}
	return res;
}

::u32string CUtfStringView::toUtf32() const
{
	// Decode any UTF
	// This implementation makes no attempt at fixing bad encoding
	if (m_Iterator == utf32Iterator)
		return ::u32string((const u32char *)m_Str, (const u32char *)((ptrdiff_t)m_Str + m_Size));
	::u32string res;
	res.reserve(m_Size << 2);
	for (iterator it(begin()), end(this->end()); it != end; ++it)
		res += *it;
	return res;
}

std::string CUtfStringView::toAscii() const
{
	std::string res;
	res.reserve(m_Size);
	for (iterator it(begin()), end(this->end()); it != end; ++it)
	{
		u32char c = *it;
		if (c < 0x80)
			res += c;
		else
			res += '?';
	}
	return res;
}

std::string CUtfStringView::fromAscii(const std::string &str)
{
	std::string res;
	res.reserve(str.size());
	for (std::string::const_iterator it(str.begin()), end(str.end()); it != end; ++it)
	{
		unsigned char c = *it;
		if (c < 0x80)
			res += (char)c;
		else
			res += '?';
	}
	return res;
}

std::wstring CUtfStringView::toWide() const
{
#ifdef NL_OS_WINDOWS
	if (m_Iterator == utf16Iterator)
		return std::wstring((const wchar_t *)m_Str, (const wchar_t *)((ptrdiff_t)m_Str + m_Size));
	std::wstring res;
	res.reserve(m_Size << 1);
	for (iterator it(begin()), end(this->end()); it != end; ++it)
	{
		u32char c = *it;
		if (c < 0x10000)
		{
			res += c;
		}
		else
		{
			c -= 0x10000;
			res += (c >> 10) | 0xD800;
			res += (c & 0x3FF) | 0xDC00;
		}
	}
	return res;
#else
	if (m_Iterator == utf32Iterator)
		return std::wstring((const wchar_t *)m_Str, (const wchar_t *)((ptrdiff_t)m_Str + m_Size));
	std::wstring res;
	res.reserve(m_Size << 2);
	for (iterator it(begin()), end(this->end()); it != end; ++it)
		res += *it;
	return res;
#endif
}

size_t CUtfStringView::count() const
{
	size_t res = 0;
	for (iterator it(begin()), end(this->end()); it != end; ++it)
		++res;
	return res;
}

ptrdiff_t CUtfStringView::offset(ptrdiff_t i)
{
	size_t res = 0;
	for (iterator it(begin()), end(this->end()); it != end; ++it)
	{
		if (res == i)
			return (ptrdiff_t)it.ptr() - (ptrdiff_t)ptr();
		++res;
	}
	return res;
}

u32char CUtfStringView::utf8Iterator(const void **addr)
{
	// Decode UTF-8
	// This implementation makes no attempt at fixing bad encoding, except for bad UTF-16 surrogate pairs
	// Invalid characters are replaced with the replacement character
	const uint8 **pp = reinterpret_cast<const uint8 **>(addr);
	u32char c0 = **pp;
	++(*pp);
	if (c0 >= 0x80)
	{
		if (c0 < 0xC0)
		{
			// Replacement character �
			return 0xFFFD;
		}
		uint8 cx = **pp;
		if ((cx & 0xC0) == 0x80)
		{
			++(*pp);
			c0 &= 0x3F; // Drop first two bits
			c0 <<= 6;
			c0 |= (cx & 0x3F); // 12 bits now (6 + 6), 2-byte encoding
			if (c0 & 0x800)
			{
				cx = **pp;
				if ((cx & 0xC0) == 0x80)
				{
					++(*pp);
					c0 &= 0x07FF; // Drop first bit
					c0 <<= 6;
					c0 |= (cx & 0x3F); // 17 bits now (12 - 1 + 6), 3-byte encoding
					if (c0 & 0x10000)
					{
						cx = **pp;
						if ((cx & 0xC0) == 0x80)
						{
							++(*pp);
							c0 &= 0xFFFF; // Drop first bit
							c0 <<= 6;
							c0 |= (cx & 0x3F); // 22 bits now (17 - 1 + 6), 4-byte encoding
							if (c0 >= 0x110000)
							{
								// Replacement character �
								return 0xFFFD;
							}
							else if (c0 < 0x10000)
							{
								// Invalid encoding
								// Replacement character �
								return 0xFFFD;
							}
						}
						else
						{
							// Replacement character �
							return 0xFFFD;
						}
					}
					else if ((c0 & 0xFC00) == 0xD800) // Higher bits of nutcase UTF-16 encoded as UTF-8
					{
						uint8 cy;
						if ((*pp)[0] == 0xED && ((cx = (*pp)[1]) & 0xF0) == 0xB0 && ((cy = (*pp)[2]) & 0xC0) == 0x80)
						{
							// Lower bits of nutcase UTF-16 encoded as UTF-8
							(*pp) += 3;
							uint16 c1 = (cx & 0x0F);
							c1 <<= 6;
							c1 |= (cy & 0x3F);
							c0 &= 0x03FF;
							c0 <<= 10;
							c0 |= (c1 & 0x03FF);
							c0 += 0x10000;
						}
						else
						{
							// Replacement character �
							return 0xFFFD;
						}
					}
					else if ((c0 & 0xFC00) == 0xDC00) // Lower bits of nutcase UTF-16 encoded as UTF-8
					{
						// Replacement character �
						return 0xFFFD;
					}
					else if (c0 < 0x0800)
					{
						// Invalid encoding
						// Replacement character �
						return 0xFFFD;
					}
				}
				else
				{
					// Replacement character �
					return 0xFFFD;
				}
			}
			else if (c0 < 0x80)
			{
				// Invalid encoding
				// Replacement character �
				return 0xFFFD;
			}
		}
		else
		{
			// Replacement character �
			return 0xFFFD;
		}
	}
	return c0;
}

u32char CUtfStringView::utf16Iterator(const void **addr)
{
	// Decode UTF-16
	// This implementation makes no attempt at fixing bad encoding
	const uint16 **pp = reinterpret_cast<const uint16 **>(addr);
	u32char c0 = **pp;
	++(*pp);
	if ((c0 & 0xFC00) == 0xD800) // Higher bits
	{
		uint16 c1 = **pp;
		if ((c1 & 0xFC00) == 0xDC00) // Lower bits
		{
			++(*pp);
			c0 &= 0x03FF;
			c0 <<= 10;
			c0 |= (c1 & 0x03FF);
			c0 += 0x10000;
		}
	}
	return c0;
}

u32char CUtfStringView::utf32Iterator(const void **addr)
{
	// UTF-32
	// This implementation makes no attempt at fixing bad encoding
	const u32char **pp = reinterpret_cast<const u32char **>(addr);
	u32char c = **pp;
	++(*pp);
	return c;
}

} /* namespace NLMISC */

/* end of file */
