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

#include "nel/misc/types_nl.h"

#include "nel/misc/debug.h"
#include "nel/misc/common.h"
#include "nel/misc/string_common.h"
#include "nel/misc/string_view.h"
#include "nel/misc/utf_string_view.h"

#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <iomanip>

void printStringMap(const std::string &name, std::map<char, std::string> &m, bool trim)
{
	std::cout << "static const char " << name << "[" << std::dec << (trim ? "64" : "256") << " * 4] = {\n";
	bool zero = false;
	for (int i = 0; i < (trim ? 64 : 256); ++i)
	{
		int x = trim ? i + 0x80 : i;
		if (m.find(x) == m.end())
		{
			if (x % 8 == 7)
			{
				zero = false;
				std::cout << "0, 0, 0, 0,\n";
			}
			else
			{
				zero = true;
				std::cout << "0, 0, 0, 0, ";
			}
		}
		else
		{
			if (zero) std::cout << "\n";
			std::stringstream ss;
			ss << "'\\x" << std::hex << std::setw(2) << std::setfill('0') << std::uppercase << (m[x].length() > 0 ? (unsigned char)m[x][0] : 0)
				<< "', '\\x" << std::hex << std::setw(2) << std::setfill('0') << std::uppercase << (m[x].length() > 1 ? (unsigned char)m[x][1] : 0)
				<< "', '\\x" << std::hex << std::setw(2) << std::setfill('0') << std::uppercase << (m[x].length() > 2 ? (unsigned char)m[x][2] : 0)
				<< "', 0,\n";
			std::cout << ss.str();
			zero = false;
		}
	}
	if (zero) std::cout << "\n";
	std::cout << "};\n\n";
}

void printMapMap(const std::string &name, const std::string &strName, std::map<char, std::map<char, std::string>> &m, int base, int size)
{
	std::cout << "static const char *" << name << "[" << size << "] = {\n";
	bool zero = false;
	for (int i = base; i < (base + size); ++i)
	{
		int x = i;
		if (m.find(x) == m.end())
		{
			if (x % 32 == 1315)
			{
				zero = false;
				std::cout << "0, \n";
			}
			else
			{
				zero = true;
				std::cout << "0, ";
			}
		}
		else
		{
			if (zero) std::cout << "\n";
			std::stringstream n;
			n << strName;
			n << std::hex << std::setw(2) << std::setfill('0') << std::uppercase << (unsigned int)x;
			std::cout << n.str() << ",\n";
			zero = false;
		}
	}
	if (zero) std::cout << "\n";
	std::cout << std::dec << "};\n\n";
}

void printMapMapMap(const std::string &name, const std::string &mapName, std::map<char, std::map<char, std::map<char, std::string>>> &m, int base, int size)
{
	std::cout << "static const char **" << name << "[" << size << "] = {\n";
	bool zero = false;
	for (int i = base; i < (base + size); ++i)
	{
		int x = i;
		if (m.find(x) == m.end())
		{
			if (x % 32 == 1315)
			{
				zero = false;
				std::cout << "0, \n";
			}
			else
			{
				zero = true;
				std::cout << "0, ";
			}
		}
		else
		{
			if (zero) std::cout << "\n";
			std::stringstream n;
			n << mapName;
			n << std::hex << std::setw(2) << std::setfill('0') << std::uppercase << (unsigned int)x;
			std::cout << n.str() << ",\n";
			zero = false;
		}
	}
	if (zero) std::cout << "\n";
	std::cout << "};\n\n";
}

void printMapMapMapMap(const std::string &name, const std::string &mapName, std::map<char, std::map<char, std::map<char, std::map<char, std::string>>>> &m, int base, int size)
{
	std::cout << "static const char ***" << name << "[" << size << "] = {\n";
	bool zero = false;
	for (int i = base; i < (base + size); ++i)
	{
		int x = i;
		if (m.find(x) == m.end())
		{
			if (x % 32 == 1315)
			{
				zero = false;
				std::cout << "0, \n";
			}
			else
			{
				zero = true;
				std::cout << "0, ";
			}
		}
		else
		{
			if (zero) std::cout << "\n";
			std::stringstream n;
			n << mapName;
			n << std::hex << std::setw(2) << std::setfill('0') << std::uppercase << (unsigned int)x;
			std::cout << n.str() << ",\n";
			zero = false;
		}
	}
	if (zero) std::cout << "\n";
	std::cout << "};\n\n";
}

void generateMap(const std::string &file, const std::string &name, const std::vector<u32char> &map)
{
	std::map<char, std::string> m1;
	std::map<char, std::map<char, std::string>> m2;
	std::map<char, std::map<char, std::map<char, std::string>>> m3;
	std::map<char, std::map<char, std::map<char, std::map<char, std::string>>>> m4;
	for (u32char i = 0; i < 0x110000; ++i)
	{
		if (map[i] != i)
		{
			std::string from;
			NLMISC::CUtfStringView::append(from, i);
			std::string to;
			NLMISC::CUtfStringView::append(to, map[i]);
			// assert(from.size() == to.size());
			if (from.length() == 1)
			{
				m1[from[0]] = to;
			}
			else if (from.length() == 2)
			{
				if (m2.find(from[0]) == m2.end())
					m2[from[0]] = std::map<char, std::string>();
				m2[from[0]][from[1]] = to;
			}
			else if (from.length() == 3)
			{
				if (m3.find(from[0]) == m3.end())
					m3[from[0]] = std::map<char, std::map<char, std::string>>();
				if (m3[from[0]].find(from[1]) == m3[from[0]].end())
					m3[from[0]][from[1]] = std::map<char, std::string>();
				m3[from[0]][from[1]][from[2]] = to;
			}
			else if (from.length() == 4)
			{
				if (m4.find(from[0]) == m4.end())
					m4[from[0]] = std::map<char, std::map<char, std::map<char, std::string>>>();
				if (m4[from[0]].find(from[1]) == m4[from[0]].end())
					m4[from[0]][from[1]] = std::map<char, std::map<char, std::string>>();
				if (m4[from[0]][from[1]].find(from[2]) == m4[from[0]][from[1]].end())
					m4[from[0]][from[1]][from[2]] = std::map<char, std::string>();
				m4[from[0]][from[1]][from[2]][from[3]] = to;
			}
		}
	}
	printStringMap("s_" + name, m1, false);
	for (int i = 0; i < 256; ++i)
	{
		std::stringstream n;
		n << "s_" << name;
		if (m2.find(i) != m2.end())
		{
			n << std::hex << std::setw(2) << std::setfill('0') << std::uppercase << (unsigned int)i;
			printStringMap(n.str(), m2[i], true);
		}
		else if (m3.find(i) != m3.end())
		{
			n << std::hex << std::setw(2) << std::setfill('0') << std::uppercase << (unsigned int)i;
			for (int j = 0; j < 256; ++j)
			{
				if (m3[i].find(j) != m3[i].end())
				{
					std::stringstream nn;
					nn << n.str();
					nn << std::hex << std::setw(2) << std::setfill('0') << std::uppercase << (unsigned int)j;
					printStringMap(nn.str(), m3[i][j], true);
				}
			}
		}
		else if (m4.find(i) != m4.end())
		{
			n << std::hex << std::setw(2) << std::setfill('0') << std::uppercase << (unsigned int)i;
			for (int j = 0; j < 256; ++j)
			{
				if (m4[i].find(j) != m4[i].end())
				{
					std::stringstream nn;
					nn << n.str();
					nn << std::hex << std::setw(2) << std::setfill('0') << std::uppercase << (unsigned int)j;
					for (int k = 0; k < 256; ++k)
					{
						if (m4[i][j].find(k) != m4[i][j].end())
						{
							std::stringstream nnn;
							nnn << nn.str();
							nnn << std::hex << std::setw(2) << std::setfill('0') << std::uppercase << (unsigned int)k;
							printStringMap(nnn.str(), m4[i][j][k], true);
						}
					}
				}
			}
		}
	}
	printMapMap("s_" + name + "Map", "s_" + name, m2, 0xC0, 32);

	for (int i = 0; i < 256; ++i)
	{
		std::stringstream n;
		n << "s_" << name << "Map";
		std::stringstream nn;
		nn << "s_" << name;
		if (m3.find(i) != m3.end())
		{
			n << std::hex << std::setw(2) << std::setfill('0') << std::uppercase << (unsigned int)i;
			nn << std::hex << std::setw(2) << std::setfill('0') << std::uppercase << (unsigned int)i;
			printMapMap(n.str(), nn.str(), m3[i], 0x80, 64);
		}
	}
	printMapMapMap("s_" + name + "MapMap", "s_" + name + "Map", m3, 0xE0, 16);

	for (int i = 0; i < 256; ++i)
	{
		std::stringstream n;
		n << "s_" << name << "Map";
		std::stringstream nn;
		nn << "s_" << name;
		std::stringstream nnn;
		nnn << "s_" << name << "MapMap";
		if (m4.find(i) != m4.end())
		{
			n << std::hex << std::setw(2) << std::setfill('0') << std::uppercase << (unsigned int)i;
			nn << std::hex << std::setw(2) << std::setfill('0') << std::uppercase << (unsigned int)i;
			nnn << std::hex << std::setw(2) << std::setfill('0') << std::uppercase << (unsigned int)i;
			for (int j = 0; j < 256; ++j)
			{
				if (m4[i].find(j) != m4[i].end())
				{
					std::stringstream n2, nn2;
					n2 << n.str() << std::hex << std::setw(2) << std::setfill('0') << std::uppercase << (unsigned int)j;
					nn2 << nn.str() << std::hex << std::setw(2) << std::setfill('0') << std::uppercase << (unsigned int)j;
					printMapMap(n2.str(), nn2.str(), m4[i][j], 0x80, 64);
				}
			}
			printMapMapMap(nnn.str(), n.str(), m4[i], 0x80, 64);
		}
	}
	printMapMapMapMap("s_" + name + "MapMapMap", "s_" + name + "MapMap", m4, 0xF0, 8);
}

int main (int argc, char **argv)
{
	std::ifstream fi("UnicodeData.txt");

	std::vector<u32char> upper;
	std::vector<u32char> lower;
	std::vector<u32char> title;
	std::vector<u32char> ci;

	upper.resize(0x110000);
	lower.resize(0x110000);
	title.resize(0x110000);
	ci.resize(0x110000);

	for (u32char i = 0; i < 0x110000; ++i)
	{
		upper[i] = i;
		lower[i] = i;
		title[i] = i;
		ci[i] = i;
	}

	std::string line;
	while (std::getline(fi, line))
	{
		std::vector<std::string> cols;
		NLMISC::explode(line, nlstr(";"), cols, false);
		nlassert(cols.size() == 15);

		u32char c = NLMISC::atoiInt64(cols[0].c_str(), 16);
		u32char up = NLMISC::atoiInt64(cols[12].c_str(), 16);
		u32char low = NLMISC::atoiInt64(cols[13].c_str(), 16);
		u32char tit = NLMISC::atoiInt64(cols[14].c_str(), 16);
		
		if (up) upper[c] = up;
		if (low) lower[c] = low;
		if (tit) title[c] = tit;
	}

	std::vector<u32char> ref;
	int rounds = 0;
	for (;;)
	{
		ref = ci;

		for (u32char i = 0; i < 0x110000; ++i)
		{
			ci[i] = title[ci[i]];
		}

		for (u32char i = 0; i < 0x110000; ++i)
		{
			ci[i] = upper[ci[i]];
		}

		for (u32char i = 0; i < 0x110000; ++i)
		{
			ci[i] = lower[ci[i]];
		}

		bool equal = true;
		for (u32char i = 0; i < 0x110000; ++i)
		{
			if (ci[i] != ref[i])
				equal = false;
		}
		++rounds;
		std::cout << rounds << std::endl;
		if (equal)
			break;
	}

	for (u32char i = 0; i < 0x110000; ++i)
	{
		if (ci[i] != lower[i])
			std::cout << i << std::endl;
	}

	generateMap("string_to_upper", "StringToUpper", upper);
	//generateMap("string_to_lower", "StringToLower", lower);
	//generateMap("string_to_title", "StringToTitle", title);
	//generateMap("string_to_ci", "StringToCaseInsensitive", ci);

	std::string test = nlstr("Οὐχὶ ταὐτὰ παρίσταταί μοι γιγνώσκειν, ὦ ἄνδρες ᾿Αθηναῖοι,");
	std::string testUpper = NLMISC::toUpper(test);
	std::string testLower = NLMISC::toLower(test);
	std::string testUpper2 = NLMISC::toUpper(testLower);
	std::string testLower2 = NLMISC::toLower(testUpper);
	std::cout << test << std::endl;
	std::cout << testUpper << std::endl;
	std::cout << testLower << std::endl;
	std::cout << testUpper2 << std::endl;
	std::cout << testLower2 << std::endl;

	int cci1 = NLMISC::compareCaseInsensitive("bAAAAfdsklj", "Cldsfjslkf");
	int cci2 = NLMISC::compareCaseInsensitive("Cldsfjslkf", "bAAAAfdsklj");
	int strc1 = strcmp(NLMISC::toLower("bAAAAfdsklj").c_str(), NLMISC::toLower("Cldsfjslkf").c_str());
	int strc2 = strcmp(NLMISC::toLower("Cldsfjslkf").c_str(), NLMISC::toLower("bAAAAfdsklj").c_str());

	int bcci1 = NLMISC::compareCaseInsensitive("bAAAAfdsklj", "AnlsqFDS");
	int bcci2 = NLMISC::compareCaseInsensitive("AnlsqFDS", "bAAAAfdsklj");
	int bstrc1 = strcmp(NLMISC::toLower("bAAAAfdsklj").c_str(), NLMISC::toLower("AnlsqFDS").c_str());
	int bstrc2 = strcmp(NLMISC::toLower("AnlsqFDS").c_str(), NLMISC::toLower("bAAAAfdsklj").c_str());

	std::vector<std::string> arr;
	arr.push_back("AnlsqFDS");
	arr.push_back("yozeRNZE");
	arr.push_back("yOzeihfn");
	arr.push_back("bAAAAfdsklj");
	arr.push_back("Cldsfjslkf");
	std::sort(arr.begin(), arr.end(), NLMISC::ltCaseInsensitive);
	for (int i = 0; i < arr.size(); ++i)
		std::cout << arr[i] << std::endl;

	return EXIT_SUCCESS;
}

/* end of file */
