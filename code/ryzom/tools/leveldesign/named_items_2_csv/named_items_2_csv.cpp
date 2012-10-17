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

/*
	author	: Fabien Houlmann - houlmann@nevrax.com
	date	: 13/10/2005

	named2csv: convert named_items.txt to .csv format and convert it back
	to named_items.txt after changes.

	use: named2csv named_items.txt filter.script (to generate a .csv based on filter)
		 named2csv named_items.txt named_items.csv (to modify .txt with .csv)

 */

#include "nel/misc/common.h"
#include "nel/misc/debug.h"
#include "nel/misc/sstring.h"
#include "nel/misc/path.h"
#include "nel/misc/file.h"
#include <iostream>

using namespace NLMISC;
using namespace std;

// function declaration
int  verifItemsFile   (const char *filename);
int  verifCsvFile   (const char *filename);
void processItemLine  (const string &s);
int  getItemsFromFile (const char *filename);
int  getFieldsFromFile(const char *filename);
int  getNbItemFromFile(const char *filename);
int  exportCsv        (const char *filename);
int  importCsv        (const char *filename);
void getItemBounds    (const CVectorSString &lines, uint num, uint &a, uint &b);
void updateItemField  (CVectorSString &lines, uint itemIndex, uint fieldIndex, uint &a, uint &b);
void addNewItem       (CVectorSString &lines, uint itemIndex);
int  updateItems      (const char *filename);

// global access
CVectorSString fields;
vector<CVectorSString> items;

// check the items file (locSlot and item number coherence)
int verifItemsFile (const char *filename)
{
	FILE *f = fopen(filename, "r");
	if (f == NULL)
		nlerror("Can't open file : %s", filename);

	char buffer[1024];
	while (fgets(buffer, 1024, f))
	{
		string s(buffer);

		// null or comment
		if (s.empty() || s.find("//") == 0)
			continue;

		if (s.find("_LocSlot") == string::npos)
			continue;

		// get item numbers
		int n1, n2;
		sscanf(buffer, "_Items#%d._LocSlot=%d", &n1, &n2);

		// check
		if (n1 != n2)
			nlerror("item number (%d) and _LocSlot(%d) don't match !", n1, n2);
	}

	fclose(f);
	return 0;
}

// check csv file (locSlot and item number coherence)
int verifCsvFile (const char *filename)
{
	FILE *f = fopen(filename, "r");
	if (f == NULL)
		nlerror("Can't open file : %s", filename);

	uint prevId = -1;
	char buffer[1024];
	// skip first line
	fgets(buffer, 1024, f);
	while (fgets(buffer, 1024, f))
	{
		uint id = atoi(buffer);
		if (id-1 != prevId)
			nlerror("item number (%d) and previous item (%d) don't match !", id, prevId);
		prevId = id;
	}

	fclose(f);
	return 0;
}

// parse a line from source file
void processItemLine(const string &s)
{
	// null or comment
	if (s.empty() || s.find("//") == 0)
		return;

	// other stuff
	if (s.find("_Items#") == string::npos)
		return;

	// get item number
	int n;
	sscanf(s.c_str(), "_Items#%d", &n);

	// check fields
	for (uint i=0 ; i<fields.size() ; i++)
	{
		string field = "_Items#" + toString(n) + "." + fields[i];

		// compare line with field
		if (s.find(fields[i]) != string::npos)
		{
			// check is next char is not valid because of names like Protection in Protection1
			if (!CSString::isValidFileNameChar(s[field.size()]))
			{
				// get value
				string::size_type pos = s.find("=");
				nlassert(pos != string::npos);
				string val(s, pos+1);

				items[n][i] = (CSString(val).strtok("\n")).strip();
				break;
			}
		}
	}
}

// fill 'items' structure with values from file
int getItemsFromFile(const char *filename)
{
	FILE *f = fopen(filename, "r");
	if (f == NULL)
		nlerror("Can't open file : %s", filename);

	char buffer[1024];
	while (fgets(buffer, 1024, f))
		processItemLine(buffer);

	fclose(f);
	return 0;
}

// fill 'fields' structure with values from file
int getFieldsFromFile(const char *filename)
{
	FILE *f = fopen(filename, "r");
	if (f == NULL)
		nlerror("Can't open file : %s", filename);

	char buffer[1024];
	while (fgets(buffer, 1024, f))
	{
		CSString s(buffer);
		s = s.strtok("\n");

		// skip null or comment
		if (s.empty() || s.find("//") == 0)
			continue;

		// add the field
		fields.push_back(s);
	}

	fclose(f);
	return 0;
}

// parse the file to count the number of items
int getNbItemFromFile(const char *filename)
{
	FILE *f = fopen(filename, "r");
	if (f == NULL)
		nlerror("Can't open file : %s", filename);

	int max = 0;
	char buffer[1024];
	while (fgets(buffer, 1024, f))
	{
		int n;
		fscanf(f, "_Items#%d", &n);
		if (n > max)
			max = n;
	}

	fclose(f);
	return max;
}

// generate .csv file based on actual filled structure
int exportCsv(const char *filename)
{
	nlassert(fields.size() != 0);

	uint i, j;
	FILE *f = fopen(filename, "w");
	if (f == NULL)
		nlerror("Can't open file : %s", filename);

	// print fields name
	for (i=0 ; i<fields.size()-1 ; i++)
		fprintf(f, "%s;", fields[i].c_str());
	fprintf(f, "%s\n", fields[fields.size()-1].c_str());

	// print values for each item
	for (i=0 ; i<items.size() ; i++)
	{
		for (j=0 ; j<items[i].size()-1 ; j++)
			fprintf(f, "%s;", items[i][j].c_str());
		fprintf(f, "%s\n", items[i][items[i].size()-1].c_str());
	}

	fclose(f);
	return 0;
}

// read .csv file to fill the structures 'items' and 'fields'
int importCsv(const char *filename)
{
	verifCsvFile(filename);

	char buffer[1024];
	FILE *f = fopen(filename, "r");
	if (f == NULL)
		nlerror("Can't open file : %s", filename);

	// read fields name
	{
		fgets(buffer, 1024, f);
		CSString s(buffer);
		s = s.strtok("\n");

		do
		{
			fields.push_back(s.splitTo(';', true));
		} while (s != "");
	}

	// read values for each item
	while (fgets(buffer, 1024, f))
	{
		CSString s(buffer), val;

		// first is the number
		val = (s.splitTo(';', true));
		uint n = val.atosi();

		// resize if needed
		if (n+1 > items.size())
			items.resize(n+1);

		// add item id
		items[n].push_back(val);

		// add others
		do
		{
			val = s.splitTo(';', true);
			items[n].push_back(val);
		} while (s != "");
	}

	fclose(f);
	return 0;
}

// compute item boundary in the file (min and max lines)
void getItemBounds(const CVectorSString &lines, uint num, uint &a, uint &b)
{
	a = b = 0;
	uint i = -1;
	bool ok = false;

	while (++i < lines.size() && !ok)
	{
		if (lines[i].empty() || lines[i].find("//") != string::npos)
			continue;

		// get item number
		uint n;
		if (sscanf(lines[i].c_str(), "_Items#%d", &n) == 0)
			continue;

		// find it
		if (n == num)
		{
			// frist line
			if (a == 0)
				a = b = i+1;
			// more line
			else
				b++;
		}
		else
		{
			// end
			if (a != 0)
				ok = true;
		}
	}

	// found it ?
	if (a != 0)
	{
		ok = true;
		b++;
	}
}

// update an item field with a new value
void updateItemField(CVectorSString &lines, uint itemIndex, uint fieldIndex, uint &a, uint &b)
{
	string field = fields[fieldIndex];
	string val = items[itemIndex][fieldIndex];
	string s = "_Items#";
	s += toString(itemIndex);
	s += ".";
	s += field;

	// remove jump
	val = CSString(val).strtok("\n");

	uint craftLine = 0;
	bool found = false;
	uint i = a-1;

	// first pass to check if param have changed
	for (i=a ; i<b ; i++)
	{
		string line = s + "= " + val;
		string::size_type pos = lines[i].find(line.c_str());
		if (pos != string::npos)
		{
			found = true;
			break;
		}
	}

	// second pass if new value
	i = a-1;
	while (++i<b && !found)
	{
		string::size_type pos;

		// store the line "_CraftParameters" : reference line
		if (craftLine == 0)
		{
			pos = lines[i].find("_CraftParameters");
			if (pos != string::npos)
				craftLine = i;
		}

		// search string
		pos = lines[i].find(s.c_str());
		if (pos == string::npos)
			continue;

		if (val != "")
		{
			// check if the attribute is the right one and not included in another one
			// for example: Protection is in ProtectionFactor
			if (!CSString::isValidFileNameChar(lines[i][s.size()]))
			{
				found = true;

				if (val != "nul")
				{
					// udpdate value
					lines[i] = s;
					lines[i] += "= ";
					lines[i] += val;
				}
				else
				{
					// remove value
					CVectorSString::iterator it = lines.begin() + i;
					lines.erase(it);
					i--;
					b--;
				}
			}
		}		
	}

	// param not found
	if (!found && !val.empty() && val != "nul")
	{
		// add it
		if (field.find("_CraftParameters") == string::npos)
		{
			// before craftLine
			CVectorSString::iterator it = lines.begin() + craftLine;
			lines.insert(it, s + "= " + val);
		}
		else
		{
			// after craftLine
			CVectorSString::iterator it = lines.begin() + craftLine + 1;
			lines.insert(it, s + "= " + val);
		}
	}
}

// add a new item a the end of the file
void addNewItem(CVectorSString &lines, uint itemIndex)
{
	CVectorSString::iterator it = lines.end();

	string s = "_Items#" + toString(itemIndex);
	lines.insert(lines.end(), s);
	lines.insert(lines.end(), s + "._SheetId=undef");
	lines.insert(lines.end(), s + "._ClientInventoryPosition= -1");
	lines.insert(lines.end(), s + "._Recommended= 250");
	lines.insert(lines.end(), s + "._LocSlot= " + toString(itemIndex));
	lines.insert(lines.end(), s + "._PhraseId=undef");
	lines.insert(lines.end(), s + "._CraftParameters");
	lines.insert(lines.end(), "\n");
}

// update all items with new values
int updateItems(const char *filename)
{
	// verify file
	verifItemsFile(filename);

	CSString data;
	data.readFromFile(filename);

	CVectorSString lines;
	data.splitLines(lines);

	for (uint itemIndex=0 ; itemIndex<items.size() ; itemIndex++)
	{
		nlassert(fields.size() >= items[itemIndex].size());
		cout << "Updating item " << itemIndex << endl;

		uint a, b;
		getItemBounds(lines, itemIndex, a, b);

		// no bound found, it's a new item
		if (b == 0)
		{
			addNewItem(lines, itemIndex);
			getItemBounds(lines, itemIndex, a, b);
		}

		for (uint fieldIndex=0 ; fieldIndex<items[itemIndex].size() ; fieldIndex++)
			updateItemField(lines, itemIndex, fieldIndex, a, b);
	}

	// rewrite file
	data.clear();
	for (uint i=0 ; i<lines.size() ; i++)
		data += lines[i] + "\n";
	data.writeToFile(filename);

	return 0;
}

NLMISC::CApplicationContext AppContext;

int main(int argc, char *argv[])
{
	string csvFile, itemsFile, scriptFile, curDir;

	

	// check number of arguments
	if (argc != 3)
		nlerror("Bad arguments number !");

	curDir = CPath::standardizePath(CPath::getCurrentPath());

	// check extensions
	{
		string ext = CFile::getExtension(argv[2]);

		if (ext == "csv")
			csvFile = argv[2];
		else if (ext == "script")
			scriptFile = argv[2];
		else
			nlerror("Bad extension : %s (use .csv or .script)", ext.c_str());

		itemsFile = argv[1];
	}

	// create a .csv file
	if (scriptFile != "")
	{
		if (CFile::getFilename(scriptFile) == scriptFile)
			scriptFile = curDir + scriptFile;

		if (CFile::getFilename(itemsFile) == itemsFile)
			itemsFile = curDir + itemsFile;

		// auto-add : _LocSlot & _PhraseId & _SheetId & _Recommended
		fields.push_back("_LocSlot");
		fields.push_back("_PhraseId");
		fields.push_back("_SheetId");
		fields.push_back("_Recommended");

		// add other fields from file
		getFieldsFromFile(scriptFile.c_str());

		// verify file
		verifItemsFile(itemsFile.c_str());
		
		// How many items ?
		uint n = getNbItemFromFile(itemsFile.c_str()) + 1;
		items.resize(n);

		// reserve memory
		for (uint i=0 ; i<n ; i++)
			items[i].resize(fields.size());

		// read values from items file
		getItemsFromFile(itemsFile.c_str());

		// generate the new file
		string csv = CFile::getFilename(itemsFile);
		string ext = CFile::getExtension(csv);
		exportCsv((CSString(csv).replace(string('.' + ext).c_str(), ".csv")).c_str());
	}

	// create a .txt file
	if (csvFile != "")
	{
		if (CFile::getFilename(csvFile) == csvFile)
			csvFile = curDir + csvFile;

		// load csv values
		importCsv(csvFile.c_str());
		if (!itemsFile.empty() && CFile::isExists(itemsFile.c_str()))
			updateItems(itemsFile.c_str());
		else
			nlerror("Can't find file : %s", itemsFile.c_str());
	}

	return 0;
}
