// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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


#include <nel/misc/types_nl.h>
#include <nel/misc/ucstring.h>
#include <nel/misc/sstring.h>
#include <nel/misc/i18n.h>

using namespace std;
using namespace NLMISC;

void usage()
{
	printf("uni_conv [option] <output_mode> <input_file> <output_file>\n");
	printf("output_mode is one of:\n");
	printf("\t-8  output in UTF-8\n");
	printf("\t-16 output in UTF-16\n");
	printf("\t-a  output in ascii, non ascii char are silently removed\n");
	printf("option:\n");
	printf("\t-x  minimal support for XML : encode &, < and > in XML chat tag\n");
}

int main(int argc, char *argv[])
{
	new NLMISC::CApplicationContext;

	bool	xmlSupport = false;
	enum	TOutMode {UNDEF, UTF8, UTF16, ASCII};
	CSString	inputFile;
	CSString	outputFile;
	TOutMode	outMode = UNDEF;

	if (argc < 4)
	{
		usage();
		return -1;
	}
	for (int i=1; i<argc; ++i)
	{
		CSString arg = argv[i];
		if (arg == "-x")
			xmlSupport = true;
		else if (arg == "-8")
			outMode = UTF8;
		else if (arg == "-16")
			outMode = UTF16;
		else if (arg == "-a")
			outMode = ASCII;
		else if (i == argc-1)
			outputFile = arg;
		else if (i == argc-2)
			inputFile = arg;
	}

	if (outputFile.empty() || inputFile.empty() || outMode == UNDEF)
	{
		usage();
		return -1;
	}

	ucstring	str;
	CI18N::readTextFile(inputFile, str, false, false, false);

	if (outMode == ASCII)
	{
		// remove any outof ascii char
		ucstring temp;
		for (uint i=0; i<str.size(); ++i)
		{
			if (str[i] < 256)
				temp += str[i];
		}
		str = temp;
	}

	if (xmlSupport)
	{
		ucstring temp;
		for (uint i=0; i<str.size(); ++i)
		{
			switch(str[i])
			{
			case '&':
				temp += "&amp;";
				break;
			case '<':
				temp += "&gt;";
				break;
			case '>':
				temp += "&lt;";
				break;
			default:
				temp += str[i];
			}
		}
		str = temp;
	}

	switch(outMode)
	{
	case UTF8:
		CI18N::writeTextFile(outputFile, str, true);
		break;
	case UTF16:
		CI18N::writeTextFile(outputFile, str, false);
		break;
	case ASCII:
		{
			string s = str.toString();
			FILE *fp = fopen(outputFile.c_str(), "wt");
			fwrite(s.data(), s.size(), 1, fp);
			fclose(fp);
		}
		break;
	default:
		break;

	}
}
