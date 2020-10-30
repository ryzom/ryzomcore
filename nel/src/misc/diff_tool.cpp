// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
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

#include "nel/misc/diff_tool.h"
#include "nel/misc/path.h"

using namespace NLMISC;
using namespace std;

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace STRING_MANAGER
{

uint64 makePhraseHash(const TPhrase &phrase)
{
	ucstring text;
	text = phrase.Parameters;
	for (uint i=0; i<phrase.Clauses.size(); ++i)
	{
		text += phrase.Clauses[i].Conditions;
		text += phrase.Clauses[i].Identifier;
		text += phrase.Clauses[i].Text;
	}

	return CI18N::makeHash(text);
}




bool parseHashFromComment(const ucstring &comments, uint64 &hashValue)
{
	string str = comments.toString();

	string::size_type pos = str.find("HASH_VALUE ");
	if (pos == string::npos)
		return false;

	string hashStr = str.substr(pos + 11, 16);

	hashValue = CI18N::stringToHash(hashStr);
	return true;
}


uint32 countLine(const ucstring &text, const ucstring::const_iterator upTo)
{
	uint32 ret = 1;
	ucstring::const_iterator first(text.begin());

	for (; first != upTo; ++first)
	{
		if (*first == '\n')
			ret++;
	}

	return ret;
}

bool loadStringFile(const std::string filename, vector<TStringInfo> &stringInfos, bool forceRehash, ucchar openMark, ucchar closeMark, bool specialCase)
{
/*	uint8 *buffer = 0;
	uint	size;

	try
	{
		CIFile fp(filename);
		size = fp.getFileSize();
		buffer = new uint8[size];
		fp.serialBuffer(buffer, size);
	}
	catch(const Exception &e)
	{
		nlinfo("Can't open file [%s] (%s)\n", filename.c_str(), e.what());
		return true;
	}
*/
/*	FILE *fp = nlfopen(filename, "rb");

	if (fp == NULL)
	{
		nlinfo("Can't open file [%s]\n", filename.c_str());
		if (buffer != 0)
			delete [] buffer;
		return true;
	}

	// move to end of file
	fseek(fp, 0, SEEK_END);

	fpos_t	pos;
	fgetpos(fp, &pos);

	uint8 *buffer = new uint8[uint(pos)];

	rewind(fp);
	uint size = fread(buffer, 1, uint(pos), fp);
	fclose (fp);
*/
	ucstring text;

	CI18N::readTextFile(filename, text, false, true, CI18N::LINE_FMT_LF);
//	CI18N::readTextBuffer(buffer, size, text);
//	delete [] buffer;

	// ok, parse the file now.
	ucstring::const_iterator first(text.begin()), last(text.end());
	std::string lastLabel("nothing");

	while (first != last)
	{
		TStringInfo si;
		CI18N::skipWhiteSpace(first, last, &si.Comments);

		if (first == last)
		{
			// check if there is only swap command remaining in comment
			if (si.Comments.find(ucstring("// DIFF SWAP ")) != ucstring::npos)
			{
				stringInfos.push_back(si);
			}
			break;
		}

		// try to read a #fileline preprocessor command
		if (CI18N::matchToken("#fileline", first, last))
		{
			// for now, just skip
			uint32 lineCounter =0;	// we count line another way
			CI18N::skipLine(first, last, lineCounter);

			// begin parse of next line
			continue;
		}

		if (!CI18N::parseLabel(first, last, si.Identifier))
		{
			uint32 line = countLine(text, first);
			nlwarning("DT: Fatal : In '%s', line %u: Invalid label after '%s'",
				filename.c_str(),
				line,
				lastLabel.c_str());
			return false;
		}
		lastLabel = si.Identifier;

		CI18N::skipWhiteSpace(first, last, &si.Comments);

		if (!CI18N::parseMarkedString(openMark, closeMark, first, last, si.Text))
		{
			uint32 line = countLine(text, first);
			nlwarning("DT: Fatal : In '%s', line %u: Invalid text value for label %s",
				filename.c_str(),
				line,
				lastLabel.c_str());
			return false;
		}

		if (specialCase)
		{
			CI18N::skipWhiteSpace(first, last, &si.Comments);

			if (!CI18N::parseMarkedString(openMark, closeMark, first, last, si.Text2))
			{
				uint32 line = countLine(text, first);
				nlwarning("DT: Fatal: In '%s' line %u: Invalid text2 value label %s",
					filename.c_str(),
					line,
					lastLabel.c_str());
				return false;
			}

		}

		if (forceRehash || !parseHashFromComment(si.Comments, si.HashValue))
		{
			// compute the hash value from text.
			si.HashValue = CI18N::makeHash(si.Text);
//			nldebug("Generating hash for %s as %s", si.Identifier.c_str(), CI18N::hashToString(si.HashValue).c_str());
		}
		else
		{
//			nldebug("Comment = [%s]", si.Comments.toString().c_str());
//			nldebug("Retrieving hash for %s as %s", si.Identifier.c_str(), CI18N::hashToString(si.HashValue).c_str());
		}
		stringInfos.push_back(si);
	}


	// check identifier uniqueness
	{
		bool error = false;
		set<string>	unik;
		set<string>::iterator it;
		for (uint i=0; i<stringInfos.size(); ++i)
		{
			it = unik.find(stringInfos[i].Identifier);
			if (it != unik.end())
			{
				nlwarning("DT: loadStringFile : identifier '%s' exist twice", stringInfos[i].Identifier.c_str() );
				error = true;
			}
			else
				unik.insert(stringInfos[i].Identifier);

		}
		if (error)
			return false;
	}

	return true;
}


ucstring prepareStringFile(const vector<TStringInfo> &strings, bool removeDiffComments, bool noDiffInfo)
{
	string diff;

	vector<TStringInfo>::const_iterator first(strings.begin()), last(strings.end());
	for (; first != last; ++first)
	{
		string str;
		const TStringInfo &si = *first;
		string comment = si.Comments.toUtf8();
		vector<string> lines;
		explode(comment, string("\n"), lines, true);

		uint i;
		for (i=0; i<lines.size(); ++i)
		{
			if (removeDiffComments)
			{
				if (lines[i].find("// DIFF ") != string::npos)
				{
					lines.erase(lines.begin()+i);
					--i;
					continue;
				}
			}
			if (lines[i].find("// INDEX ") != string::npos)
			{
				lines.erase(lines.begin()+i);
				--i;
			}
			else if (lines[i].find("// HASH_VALUE ") != string::npos)
			{
				lines.erase(lines.begin()+i);
				--i;
			}
		}

		comment.erase();
		for (i=0; i<lines.size(); ++i)
		{
			comment += lines[i] + "\n";
		}
		si.Comments = ucstring(comment);

		str = comment;
		if (!si.Identifier.empty() || !si.Text.empty())
		{
			// add hash value comment if needed
//			if (si.Comments.find(ucstring("// HASH_VALUE ")) == ucstring::npos)
			if (!noDiffInfo)
			{
				str += "// HASH_VALUE " + CI18N::hashToString(si.HashValue) + "\n";
				str += "// INDEX " + NLMISC::toString("%u", first-strings.begin()) + "\n";
			}
			str += si.Identifier + '\t';

			string text = CI18N::makeMarkedString('[', ']', si.Text).toUtf8();
			string text2;
			// add new line and tab after each \n tag
			string::size_type pos;
			while ((pos = text.find("\\n")) != string::npos)
			{
				text2 += text.substr(0, pos+2) + "\n\t";
				text = text.substr(pos+2);
			}
			text2 += text;//.substr(0, pos+2);
			str += text2 + "\n\n";
//			str += CI18N::makeMarkedString('[', ']', si.Text) + nl + nl;
		}

//		nldebug("Adding string [%s]", str.toString().c_str());
		diff += str;
	}

	return ucstring::makeFromUtf8(diff);
}


bool readPhraseFile(const std::string &filename, vector<TPhrase> &phrases, bool forceRehash)
{
	ucstring doc;

	CI18N::readTextFile(filename, doc, false, true, CI18N::LINE_FMT_LF);

	return readPhraseFileFromString(doc, filename, phrases, forceRehash);
}

bool readPhraseFileFromString(ucstring const& doc, const std::string &filename, vector<TPhrase> &phrases, bool forceRehash)
{
	std::string lastRead("nothing");

	ucstring::const_iterator first(doc.begin()), last(doc.end());
	while (first != last)
	{
		TPhrase	phrase;
		// parse the phrase
		CI18N::skipWhiteSpace(first, last, &phrase.Comments);

		if (first == last)
		{
			if (!phrase.Comments.empty())
			{
				// push the resulting comment
				phrases.push_back(phrase);
			}
			break;
		}

		// try to read a #fileline preprocessor command
		if (CI18N::matchToken("#fileline", first, last))
		{
			// for now, just skip
			uint32 lineCounter =0;	// we count line another way
			CI18N::skipLine(first, last, lineCounter);

			// begin parse of next line
			continue;
		}

		if (!CI18N::parseLabel(first, last, phrase.Identifier))
		{
			uint32 line = countLine(doc, first);
			nlwarning("DT: In '%s' line %u: Error parsing phrase identifier after %s\n",
				filename.c_str(),
				line,
				lastRead.c_str());
			return false;
		}
//		nldebug("DT: parsing phrase '%s'", phrase.Identifier.c_str());
		lastRead = phrase.Identifier;
		CI18N::skipWhiteSpace(first, last, &phrase.Comments);
		if (!CI18N::parseMarkedString('(', ')', first, last, phrase.Parameters))
		{
			uint32 line = countLine(doc, first);
			nlwarning("DT: in '%s', line %u: Error parsing parameter list for phrase %s\n",
				filename.c_str(),
				line,
				phrase.Identifier.c_str());
			return false;
		}
		CI18N::skipWhiteSpace(first, last, &phrase.Comments);
		if (first == last || *first != '{')
		{
			uint32 line = countLine(doc, first);
			nlwarning("DT: In '%s', line %u: Error parsing block opening '{' in phase %s\n",
				filename.c_str(),
				line,
				phrase.Identifier.c_str());
			return false;
		}
		++first;

		ucstring temp;

		while (first != last && *first != '}')
		{
			TClause	clause;
			// append the comment preread at previous pass
			clause.Comments = temp;
			temp.erase();
			// parse the clauses
			CI18N::skipWhiteSpace(first, last, &clause.Comments);
			if (first == last)
			{
				nlwarning("DT: Found end of file in non closed block for phrase %s\n", phrase.Identifier.c_str());
				return false;
			}

			if (*first == '}')
				break;

			// skip the conditional expression
			ucstring cond;
			while (first != last && *first == '(')
			{
				if (!CI18N::parseMarkedString('(', ')', first, last, cond))
				{
					uint32 line = countLine(doc, first);
					nlwarning("DT: In '%s' line %u: Error parsing conditional expression in phrase %s, clause %u\n",
						filename.c_str(),
						line,
						phrase.Identifier.c_str(),
						phrase.Clauses.size()+1);
					return false;
				}

				// only prepend a space if required
				if (!clause.Conditions.empty()) clause.Conditions += " ";

				clause.Conditions += "(" + cond + ")";
				CI18N::skipWhiteSpace(first, last, &clause.Comments);
			}

			if (first == last)
			{
				nlwarning("DT: in '%s': Found end of file in non closed block for phrase %s\n",
					filename.c_str(),
					phrase.Identifier.c_str());
				return false;
			}
			// read the idnetifier (if any)
			CI18N::parseLabel(first, last, clause.Identifier);
			CI18N::skipWhiteSpace(first, last, &temp);
			// read the text
			if (CI18N::parseMarkedString('[', ']', first, last, clause.Text))
			{
				// the last read comment is for this clause.
				clause.Comments += temp;
				temp.erase();
			}
			else
			{
				uint32 line = countLine(doc, first);
				nlwarning("DT: in '%s' line %u: Error reading text for clause %u (%s) in  phrase %s\n",
					filename.c_str(),
					line,
					phrase.Clauses.size()+1,
					clause.Identifier.c_str(),
					phrase.Identifier.c_str());
				return false;

			}

			phrase.Clauses.push_back(clause);
		}
		CI18N::skipWhiteSpace(first, last);
		if (first == last || *first != '}')
		{
			uint32 line = countLine(doc, first);
			nlwarning("DT: in '%s' line %u: Missing block closing tag '}' in phrase %s\n",
				filename.c_str(),
				line,
				phrase.Identifier.c_str());
			return false;
		}
		++first;

		// handle hash value.
		if (forceRehash || !parseHashFromComment(phrase.Comments, phrase.HashValue))
		{
			// the hash is not in the comment, compute it.
			phrase.HashValue = makePhraseHash(phrase);
			if (forceRehash)
			{
				// the has is perhaps in the comment
				ucstring::size_type pos = phrase.Comments.find(ucstring("// HASH_VALUE"));
				if (pos != ucstring::npos)
				{
					phrase.Comments = phrase.Comments.substr(0, pos);
				}
			}
		}

//		nldebug("DT : storing phrase '%s'", phrase.Identifier.c_str());
		phrases.push_back(phrase);
	}

	// check identifier uniqueness
	{
		bool error = false;
		set<string>	unik;
		set<string>::iterator it;
		for (uint i=0; i<phrases.size(); ++i)
		{
			it = unik.find(phrases[i].Identifier);
			if (it != unik.end())
			{
				nlwarning("DT: readPhraseFile : identifier '%s' exist twice", phrases[i].Identifier.c_str() );
				error = true;
			}
			else
				unik.insert(phrases[i].Identifier);
		}
		if (error)
			return false;
	}

	return true;
}
ucstring tabLines(uint nbTab, const ucstring &str)
{
	ucstring ret;
	ucstring tabs;

	for (uint i =0; i<nbTab; ++i)
		tabs.push_back('\t');

	ret = tabs;
	ucstring::const_iterator first(str.begin()), last(str.end());
	for (; first != last; ++first)
	{
		ret += *first;
		if (*first == '\n')
			ret += tabs;
	}

	while (ret[ret.size()-1] == '\t')
		ret = ret.substr(0, ret.size()-1);

	return ret;
}

ucstring preparePhraseFile(const vector<TPhrase> &phrases, bool removeDiffComments)
{
	ucstring ret;
	vector<TPhrase>::const_iterator first(phrases.begin()), last(phrases.end());
	for (; first != last; ++first)
	{
		const TPhrase &p = *first;

		if (removeDiffComments)
		{
			string comment = p.Comments.toString();
			vector<string>	lines;
			explode(comment, string("\n"), lines, true);

			uint i;
			for (i=0; i<lines.size(); ++i)
			{
				if (lines[i].find("// DIFF ") != string::npos)
				{
					lines.erase(lines.begin()+i);
					--i;
				}
			}

			comment.erase();
			for (i=0; i<lines.size(); ++i)
			{
				comment += lines[i] + "\n";
			}
			p.Comments = ucstring(comment);
		}
		ret += p.Comments;

		if (!p.Identifier.empty() || !p.Clauses.empty())
		{
			if (p.Comments.find(ucstring("// HASH_VALUE ")) == ucstring::npos)
			{
				// add the hash value.
				ret += ucstring("// HASH_VALUE ")+CI18N::hashToString(p.HashValue) + nl;
			}
			ret += p.Identifier + " ("+p.Parameters + ")" + nl;
			ret += '{';
			ret += nl;
			for (uint i=0; i<p.Clauses.size(); ++i)
			{
				const TClause &c = p.Clauses[i];
				if (!c.Comments.empty())
				{
					ucstring comment = tabLines(1, c.Comments);
					ret += comment; // + '\n';
				}
				if (!c.Conditions.empty())
				{
					ucstring cond = tabLines(1, c.Conditions);
					ret += cond + nl;
				}
				ret += '\t';
//				ucstring text = CI18N::makeMarkedString('[', ']', c.Text);

				ucstring text = CI18N::makeMarkedString('[', ']', c.Text);;
				ucstring text2;
				// add new line and tab after each \n tag
				ucstring::size_type pos;
				const ucstring nlTag("\\n");
				while ((pos = text.find(nlTag)) != ucstring::npos)
				{
					text2 += text.substr(0, pos+2) + nl;
					text = text.substr(pos+2);
				}
				text2 += text;//.substr(0, pos+2);

				text.swap(text2);

				text = tabLines(3, text);
				// remove begin tabs
				text = text.substr(3);
				ret += '\t' + c.Identifier + '\t' + text + nl + nl;
			}
			ret += '}';
		}
		ret += nl + nl;
	}

	return ret;
}

bool loadExcelSheet(const string filename, TWorksheet &worksheet, bool checkUnique)
{
	// Yoyo: must test with CIFile because can be packed into a .bnp on client...
	CIFile	fp;
	if(!fp.open(filename))
	{
		nldebug("DT: Can't open file [%s]\n", filename.c_str());
		return true;
	}
	fp.close();

	ucstring str;
	CI18N::readTextFile(filename, str, false, false, CI18N::LINE_FMT_LF);

	if (!readExcelSheet(str, worksheet, checkUnique))
		return false;

	return true;
}

bool readExcelSheet(const ucstring &str, TWorksheet &worksheet, bool checkUnique)
{
	if(str.empty())
		return true;

	// copy the str to a big ucchar array => Avoid allocation / free
	vector<ucchar>	strArray;
	// append a '\0'
	strArray.resize(str.size()+1);
	strArray[strArray.size()-1]= 0;
	memcpy(&strArray[0], &str[0], str.size()*sizeof(ucchar));

	// size of new line characters
	size_t sizeOfNl = nl.length();

	// **** Build array of lines. just point to strArray, and fill 0 where appropriated
	vector<ucchar*> lines;
	lines.reserve(500);
	ucstring::size_type pos = 0;
	ucstring::size_type lastPos = 0;
	while ((pos = str.find(nl, lastPos)) != ucstring::npos)
	{
		if (pos>lastPos)
		{
			strArray[pos]= 0;
//			nldebug("Found line : [%s]", ucstring(&strArray[lastPos]).toString().c_str());
			lines.push_back(&strArray[lastPos]);
		}
		lastPos = pos + sizeOfNl;
	}

	// Must add last line if no \n ending
	if (lastPos < str.size())
	{
		pos= str.size();
		strArray[pos]= 0;
//		nldebug("Found line : [%s]", ucstring(&strArray[lastPos]).toString().c_str());
		lines.push_back(&strArray[lastPos]);
	}

//	nldebug("Found %u lines", lines.size());

	// **** Do 2 pass.1st count the cell number, then fill. => avoid reallocation
	uint		newColCount= 0;
	uint		i;
	for (i=0; i<lines.size(); ++i)
	{
		uint	numCells;
		numCells= 0;

		ucchar	*first= lines[i];
		for (; *first != 0; ++first)
		{
			if (*first == '\t')
			{
				numCells++;
			}
			else if (*first == '"' && first==lines[i])
			{
				// read a quoted field.
				first++;
				while (*first != 0 && *first != '"' && *(first+1) != 0 && *(first+1) != '"')
				{
					first++;
					if (*first != 0 && *first == '"')
					{
						// skip this
						first++;
					}
				}
			}
		}
		// last cell
		numCells++;

		// take max cell of all lines
		if (newColCount != max(newColCount, numCells))
		{
			newColCount = max(newColCount, numCells);
			nldebug("At line %u, numCol changed to %u",
				i, newColCount);
		}
	}


	// **** alloc / enlarge worksheet
	// enlarge Worksheet column size, as needed
	while (worksheet.ColCount < newColCount)
		worksheet.insertColumn(worksheet.ColCount);

	// enlarge Worksheet row size, as needed
	uint	startLine= worksheet.size();
	worksheet.resize(startLine + (uint)lines.size());


	// **** fill worksheet
	ucstring	cell;
	for (i=0; i<lines.size(); ++i)
	{
		uint	numCells;
		numCells= 0;
		cell.erase();

		ucchar	*first= lines[i];
		for (; *first != 0; ++first)
		{
			if (*first == '\t')
			{
//				nldebug("Found cell [%s]", cell.toString().c_str());
				worksheet.setData(startLine + i, numCells, cell);
				numCells++;
				cell.erase();
			}
			else if (*first == '"' && first==lines[i])
			{
				// read a quoted field.
				first++;
				while (*first != 0 && *first != '"' && *(first+1) != 0 && *(first+1) != '"')
				{
					cell += *first;
					first++;
					if (*first != 0 && *first == '"')
					{
						// skip this
						first++;
					}
				}
			}
			else
			{
				cell += *first;
			}
		}
//		nldebug("Found cell [%s]", cell.toString().c_str());
		/// append last cell
		worksheet.setData(startLine + i, numCells, cell);
		numCells++;
		nlassertex(numCells<=newColCount, ("readExcelSheet: bad row format: at line %u, the row has %u cell, max is %u", i, numCells, newColCount));
//		nldebug("Found %u cells in line %u", numCells, i);
	}


	// **** identifier uniqueness checking.
	if (checkUnique)
	{
		if (worksheet.size() > 0)
		{
			// look for the first non '* tagged' or 'DIFF_CMD' column
			uint nameCol = 0;
			while (nameCol < worksheet.ColCount && (*worksheet.getData(0, nameCol).begin() == uint16('*') || worksheet.getData(0, nameCol) == ucstring("DIFF_CMD")))
				++nameCol;

			if (nameCol < worksheet.ColCount )
			{
				// ok we can check unikness
				bool error = false;
				set<ucstring>	unik;
				set<ucstring>::iterator it;
				for (uint j=0; j<worksheet.size(); ++j)
				{
					it = unik.find(worksheet.getData(j, nameCol));
					if (it != unik.end())
					{
						nlwarning("DT: readExcelSheet : identifier '%s' exist twice", worksheet.getData(j, nameCol).toString().c_str() );
						error = true;
					}
					else
						unik.insert(worksheet.getData(j, nameCol));
				}
				if (error)
					return false;
			}
		}
	}

	return true;
}

void makeHashCode(TWorksheet &sheet, bool forceRehash)
{
	if (!sheet.Data.empty())
	{
		TWorksheet::TRow::iterator it = find(sheet.Data[0].begin(), sheet.Data[0].end(), ucstring("*HASH_VALUE"));
		if (forceRehash || it == sheet.Data[0].end())
		{
			// we need to generate HASH_VALUE column !
			if (it == sheet.Data[0].end())
			{
				sheet.insertColumn(0);
				sheet.Data[0][0] = ucstring("*HASH_VALUE");
			}

			// Check columns
			vector<bool>	columnOk;
			columnOk.resize(sheet.ColCount, false);
			for (uint k=1; k<sheet.ColCount; ++k)
			{
				if (sheet.Data[0][k].find(ucstring("*")) != 0 && sheet.Data[0][k].find(ucstring("DIFF ")) != 0)
				{
					columnOk[k]= true;
				}
			}

			// make hash for each line
			ucstring str;
			for (uint j=1; j<sheet.Data.size(); ++j)
			{
				str.erase();
				for (uint k=1; k<sheet.ColCount; ++k)
				{
					if (columnOk[k])
					{
						str += sheet.Data[j][k];
					}
				}
				uint64 hash = CI18N::makeHash(str);
				CI18N::hashToUCString(hash, sheet.Data[j][0]);
			}
		}
		else
		{
			uint index = (uint)(it - sheet.Data[0].begin());
			for (uint j=1; j<sheet.Data.size(); ++j)
			{
				ucstring &field = sheet.Data[j][index];

				if (!field.empty() && field[0] == '_')
					field = field.substr(1);
			}
		}
	}
}

ucstring prepareExcelSheet(const TWorksheet &worksheet)
{
	if(worksheet.Data.empty())
		return ucstring();

	// **** First pass: count approx the size
	uint	approxSize= 0;
	for (uint i=0; i<worksheet.Data.size(); ++i)
	{
		for (uint j=0; j<worksheet.Data[i].size(); ++j)
		{
			approxSize+= (uint)worksheet.Data[i][j].size() + 1;
		}
		approxSize++;
	}

	// Hash value for each column?
	vector<bool>	hashValue;
	hashValue.resize(worksheet.Data[0].size());
	for (uint j=0; j<worksheet.Data[0].size(); ++j)
	{
		hashValue[j]= worksheet.Data[0][j] == ucstring("*HASH_VALUE");
	}

	// **** Second pass: fill
	ucstring text;
	text.reserve(approxSize*2);
	for (uint i=0; i<worksheet.Data.size(); ++i)
	{
		for (uint j=0; j<worksheet.Data[i].size(); ++j)
		{
			if (i > 0 && hashValue[j] && (!worksheet.Data[i][j].empty() && worksheet.Data[i][j][0] != '_'))
				text += "_";
			text += worksheet.Data[i][j];
			if (j != worksheet.Data[i].size()-1)
				text += '\t';
		}
		text += nl;
	}

	return text;
}







}	// namespace STRING_MANAGER

