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

#include "nel/misc/string_common.h"
#include "nel/misc/sstring.h"

using namespace std;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NLMISC
{

string addSlashR(const string &str)
{
	string formatedStr;
	// replace \n with \r\n
	for (uint i = 0; i < str.size(); i++)
	{
		if (str[i] == '\n' && i > 0 && str[i - 1] != '\r')
		{
			formatedStr += '\r';
		}
		formatedStr += str[i];
	}
	return formatedStr;
}

string removeSlashR(const string &str)
{
	string formatedStr;
	// remove \r
	for (uint i = 0; i < str.size(); i++)
	{
		if (str[i] != '\r')
			formatedStr += str[i];
	}
	return formatedStr;
}

bool fromString(const std::string &str, bool &val)
{
	if (str.length() == 1)
	{
		const char c = str[0];

		switch (c)
		{
		case '1':
		case 't':
		case 'T':
		case 'y':
		case 'Y':
			val = true;
			break;

		case '0':
		case 'f':
		case 'F':
		case 'n':
		case 'N':
			val = false;
			break;

		default:
			val = false;
			return false;
		}
	}
	else
	{
		std::string strl = toLower(str);
		if (strl == "true" || strl == "yes")
		{
			val = true;
		}
		else if (strl == "false" || strl == "no")
		{
			val = false;
		}
		else
		{
			val = false;
			return false;
		}
	}

	return true;
}

#if defined(NL_OS_WINDOWS)

std::string winWideToCp(const wchar_t *str, size_t len, UINT cp)
{
	if (!len)
		len = wcslen(str);
	if (!len)
		return std::string();

	// Convert from wide to codepage
	char *tmp = (char *)_malloca((len + 1) * 4);
	if (!tmp)
		return std::string();
	int tmpLen = WideCharToMultiByte(cp, 0,
	    str, (int)(len + 1),
	    tmp, (int)((len + 1) * 4),
	    NULL, NULL);
	if (tmpLen <= 1)
	{
		_freea(tmp);
		return std::string();
	}

	std::string res = tmp;
	_freea(tmp);
	return res;
}

std::string winCpToCp(const char *str, size_t len, UINT srcCp, UINT dstCp)
{
	if (!len)
		len = strlen(str);
	if (!len)
		return std::string();

	// First convert from codepage to wide
	wchar_t *tmp = (wchar_t *)_malloca((len + 1) * 4);
	if (!tmp)
		return std::string();
	int tmpLen = MultiByteToWideChar(srcCp, 0,
	    str, (int)(len + 1), /* include null-termination */
	    tmp, (int)((len + 1) * 2));
	if (tmpLen <= 1)
	{
		_freea(tmp);
		return std::string();
	}

	// Then convert from wide to codepage
	std::string res = winWideToCp(tmp, (size_t)tmpLen - 1, dstCp); /* tmpLen includes null-term */
	_freea(tmp);
	return res;
}

std::wstring winCpToWide(const char *str, size_t len, UINT cp)
{
	if (!len)
		len = strlen(str);
	if (!len)
		return std::wstring();

	// Convert from codepage to wide
	wchar_t *tmp = (wchar_t *)_malloca((len + 1) * 4);
	if (!tmp)
		return std::wstring();
	int tmpLen = MultiByteToWideChar(cp, 0,
	    str, (int)(len + 1), /* include null-termination */
	    tmp, (int)((len + 1) * 2));
	if (tmpLen <= 1)
	{
		_freea(tmp);
		return std::wstring();
	}

	std::wstring res = tmp;
	_freea(tmp);
	return res;
}

#endif

// Convert local codepage to UTF-8
// On Windows, the local codepage is undetermined
// On Linux, the local codepage is always UTF-8 (no-op)
std::string mbcsToUtf8(const char *str, size_t len)
{
#if defined(NL_OS_WINDOWS)
	UINT codePage = GetACP();
	// Windows 10 allows setting the local codepage to UTF-8
	if (codePage == CP_UTF8) /* 65001 */
		return str;
	return winCpToCp(str, len, CP_ACP, CP_UTF8);
#else
	return str; /* no-op */
#endif
}

std::string mbcsToUtf8(const std::string &str)
{
#if defined(NL_OS_WINDOWS)
	if (str.empty())
		return str;
	UINT codePage = GetACP();
	// Windows 10 allows setting the local codepage to UTF-8
	if (codePage == CP_UTF8) /* 65001 */
		return str;
	return winCpToCp(str.c_str(), str.size(), CP_ACP, CP_UTF8);
#else
	return str; /* no-op */
#endif
}

// Convert wide codepage to UTF-8
// On Windows, the wide codepage is UTF-16
// On Linux, the wide codepage is UTF-32
std::string wideToUtf8(const wchar_t *str, size_t len)
{
#if defined(NL_OS_WINDOWS)
	return winWideToCp(str, len, CP_UTF8);
#else
	// TODO: UTF-32 to UTF-8
	nlassert(false);
#endif
}

std::string wideToUtf8(const std::wstring &str)
{
	return wideToUtf8(str.c_str(), str.size());
}

// Convert UTF-8 to wide character set
std::wstring utf8ToWide(const char *str, size_t len)
{
#if defined(NL_OS_WINDOWS)
	return winCpToWide(str, len, CP_UTF8);
#else
	// TODO: UTF-32 to UTF-8
	nlassert(false);
#endif
}

std::wstring utf8ToWide(const std::string &str)
{
	return utf8ToWide(str.c_str(), str.size());
}

// Convert UTF-8 to local multibyte character set
std::string utf8ToMbcs(const char *str, size_t len)
{
#if defined(NL_OS_WINDOWS)
	UINT codePage = GetACP();
	// Windows 10 allows setting the local codepage to UTF-8
	if (codePage == CP_UTF8) /* 65001 */
		return str;
	return winCpToCp(str, len, CP_UTF8, CP_ACP);
#else
	return str; /* no-op */
#endif
}

std::string utf8ToMbcs(const std::string &str)
{
#if defined(NL_OS_WINDOWS)
	if (str.empty())
		return str;
	UINT codePage = GetACP();
	// Windows 10 allows setting the local codepage to UTF-8
	if (codePage == CP_UTF8) /* 65001 */
		return str;
	return winCpToCp(str.c_str(), str.size(), CP_UTF8, CP_ACP);
#else
	return str; /* no-op */
#endif
}

// Convert wide to local multibyte character set
std::string wideToMbcs(const wchar_t *str, size_t len)
{
#if defined(NL_OS_WINDOWS)
	return winWideToCp(str, len, CP_ACP);
#else
	return wideToUtf8(str, len);
#endif
}

std::string wideToMbcs(const std::wstring &str)
{
#if defined(NL_OS_WINDOWS)
	return winWideToCp(str.c_str(), str.size(), CP_ACP);
#else
	return wideToUtf8(str);
#endif
}

// Convert local multibyte to wide character set
std::wstring mbcsToWide(const char *str, size_t len)
{
#if defined(NL_OS_WINDOWS)
	return winCpToWide(str, len, CP_ACP);
#else
	return utf8ToWide(str, len);
#endif
}

std::wstring mbcsToWide(const std::string &str)
{
#if defined(NL_OS_WINDOWS)
	return winCpToWide(str.c_str(), str.size(), CP_ACP);
#else
	return utf8ToWide(str);
#endif
}

}
