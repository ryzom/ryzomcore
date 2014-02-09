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

#include "nel/misc/path.h"
#include "nel/misc/i18n.h"

using namespace std;

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC {

CI18N::StrMapContainer	CI18N::_StrMap;
CI18N::StrMapContainer	CI18N::_StrMapFallback;
bool					CI18N::_StrMapLoaded = false;
const ucstring			CI18N::_NotTranslatedValue("<Not Translated>");
bool					CI18N::_LanguagesNamesLoaded = false;
string					CI18N::_SelectedLanguageCode;
CI18N::ILoadProxy		*CI18N::_LoadProxy = 0;
vector<string>			CI18N::_LanguageCodes;
vector<ucstring>		CI18N::_LanguageNames;
bool CI18N::noResolution = false;

void CI18N::setLoadProxy(ILoadProxy *loadProxy)
{
	_LoadProxy = loadProxy;
}

void CI18N::initLanguages()
{
	if (!_LanguagesNamesLoaded)
	{
		_LanguageCodes.push_back("en");
		_LanguageCodes.push_back("fr");
		_LanguageCodes.push_back("de");
		_LanguageCodes.push_back("ru");

		_LanguageNames.push_back(ucstring("English"));
		_LanguageNames.push_back(ucstring("French"));
		_LanguageNames.push_back(ucstring("German"));
		_LanguageNames.push_back(ucstring("Russian"));

		_LanguagesNamesLoaded = true;
	}
}

const std::vector<ucstring> &CI18N::getLanguageNames()
{
	initLanguages();

	return _LanguageNames;
}

const std::vector<std::string> &CI18N::getLanguageCodes()
{
	initLanguages();

	return _LanguageCodes;
}

void CI18N::load (const string &languageCode, const string &fallbackLanguageCode)
{
	if (_StrMapLoaded)	_StrMap.clear ();
	else				_StrMapLoaded = true;
	_SelectedLanguageCode = languageCode;
	loadFileIntoMap(languageCode + ".uxt", _StrMap);

	_StrMapFallback.clear();
	if(!fallbackLanguageCode.empty())
	{
		loadFileIntoMap(fallbackLanguageCode + ".uxt", _StrMapFallback);
	}
}

bool CI18N::loadFileIntoMap(const string &fileName, StrMapContainer &destMap)
{
	ucstring text;
	// read in the text
	if (_LoadProxy)
		_LoadProxy->loadStringFile(fileName, text);
	else
		readTextFile(fileName, text);

	// remove any comment
	removeCComment(text);

	ucstring::const_iterator first(text.begin()), last(text.end());
	string lastReadLabel("nothing");

	while (first != last)
	{
		skipWhiteSpace(first, last);
		string label;
		ucstring ucs;
		if (!parseLabel(first, last, label))
		{
			nlwarning("I18N: Error reading label field in %s. Stop reading after %s.", fileName.c_str(), lastReadLabel.c_str());
			return false;
		}
		lastReadLabel = label;
		skipWhiteSpace(first, last);
		if (!parseMarkedString('[', ']', first, last, ucs))
		{
			nlwarning("I18N: Error reading text for label %s in %s. Stop reading.", label.c_str(), fileName.c_str());
			return false;
		}

		// ok, a line read.
		pair<map<string, ucstring>::iterator, bool> ret;
		ret = destMap.insert(make_pair(label, ucs));
		if (!ret.second)
		{
			nlwarning("I18N: Error in %s, the label %s exists twice !", fileName.c_str(), label.c_str());
		}
		skipWhiteSpace(first, last);
	}

	// a little check to ensure that the lang name has been set.
	StrMapContainer::iterator it(destMap.find("LanguageName"));
	if (it == destMap.end())
	{
		nlwarning("I18N: In file %s, missing LanguageName translation (should be first in file)", fileName.c_str());
	}
	return true;
}

void CI18N::loadFromFilename(const string &filename, bool reload)
{
	StrMapContainer destMap;
	if (!loadFileIntoMap(filename, destMap))
	{
		return;
	}
	// merge with existing map
	for(StrMapContainer::iterator it = destMap.begin(); it != destMap.end(); ++it)
	{
		if (!reload)
		{
			if (_StrMap.count(it->first))
			{
				nlwarning("I18N: Error in %s, the label %s exist twice !", filename.c_str(), it->first.c_str());
			}
		}
		_StrMap[it->first] = it->second;
	}
}

const ucstring &CI18N::get (const string &label)
{
	if( noResolution )
	{
		static ucstring labelString;
		labelString = label;
		return labelString;
	}

	if (label.empty())
	{
		static ucstring	emptyString;
		return emptyString;
	}

	StrMapContainer::iterator it(_StrMap.find(label));

	if (it != _StrMap.end())
		return it->second;

	static CHashSet<string>	missingStrings;
	if (missingStrings.find(label) == missingStrings.end())
	{
		nlwarning("I18N: The string %s did not exist in language %s (display once)", label.c_str(), _SelectedLanguageCode.c_str());
		missingStrings.insert(label);
	}

	// use the fall back language if it exists
	it = _StrMapFallback.find(label);
	if (it != _StrMapFallback.end())
		return it->second;

	static ucstring	badString;

	badString = ucstring(string("<NotExist:")+label+">");

	return badString;
}

bool CI18N::hasTranslation(const string &label)
{
	if (label.empty()) return true;

	if(_StrMap.find(label) != _StrMap.end())
			return true;

	// use the fall back language if it exists
	if (_StrMapFallback.find(label) != _StrMapFallback.end())
		return true;

	return false;
}

ucstring CI18N::getCurrentLanguageName ()
{
	return get("LanguageName");
}

string CI18N::getCurrentLanguageCode ()
{
	return _SelectedLanguageCode;
}

void CI18N::removeCComment(ucstring &commentedString)
{
	ucstring temp;
	temp.reserve(commentedString.size());
	ucstring::const_iterator first(commentedString.begin()), last(commentedString.end());
	for (;first != last; ++first)
	{
		temp.push_back(*first);
		if (*first == '[')
		{
			// no comment inside string literal
			while (++first != last)
			{
				temp.push_back(*first);
				if (*first == ']')
					break;
			}
		}
		else if (*first == '/')
		{
			// start of comment ?
			++first;
			if (first != last && *first == '/')
			{
				temp.resize(temp.size()-1);
				// one line comment, skip until end of line
				while (first != last && *first != '\n')
					++first;
			}
			else if (first != last && *first == '*')
			{
				temp.resize(temp.size()-1);
				// start of multi line comment, skip until we found '*/'
				while (first != last && !(*first == '*' && (first+1) != last && *(first+1) == '/'))
					++first;
				// skip the closing '/'
				if (first != last)
					++first;
			}
			else
			{
				temp.push_back(*first);
			}
		}
	}
	commentedString.swap(temp);
}

void CI18N::skipWhiteSpace(ucstring::const_iterator &it, ucstring::const_iterator &last, ucstring *storeComments, bool newLineAsWhiteSpace)
{
	while (it != last &&
			(
					(*it == 0xa && newLineAsWhiteSpace)
				||	(*it == 0xd && newLineAsWhiteSpace)
				||	*it == ' '
				||	*it == '\t'
				||	(storeComments && *it == '/' && it+1 != last && *(it+1) == '/')
				||	(storeComments && *it == '/' && it+1 != last && *(it+1) == '*')
			))
	{
		if (storeComments && *it == '/' && it+1 != last && *(it+1) == '/')
		{
			// found a one line C comment. Store it until end of line.
			while (it != last && *it != '\n')
				storeComments->push_back(*it++);
			// store the final '\n'
			if (it != last)
				storeComments->push_back(*it++);
		}
		else if (storeComments && *it == '/' && it+1 != last && *(it+1) == '*')
		{
			// found a multi line C++ comment. store until we found the closing '*/'
			while (it != last && !(*it == '*' && it+1 != last && *(it+1) == '/'))
				storeComments->push_back(*it++);
			// store the final '*'
			if (it != last)
				storeComments->push_back(*it++);
			// store the final '/'
			if (it != last)
				storeComments->push_back(*it++);
			// and a new line.
			storeComments->push_back('\r');
			storeComments->push_back('\n');
		}
		else
		{
			// just skip white space or don't store comments
			++it;
		}
	}
}

bool CI18N::parseLabel(ucstring::const_iterator &it, ucstring::const_iterator &last, string &label)
{
	ucstring::const_iterator rewind = it;
	label.erase();

	// other char must be [0-9A-Za-z@_]*
	while (it != last &&
			(
				(*it >= '0' && *it <= '9')
			||	(*it >= 'A' && *it <= 'Z')
			||	(*it >= 'a' && *it <= 'z')
			||	(*it == '_')
			||	(*it == '@')
			)
		)
		label.push_back(char(*it++));

	return true;
}

bool CI18N::parseMarkedString(ucchar openMark, ucchar closeMark, ucstring::const_iterator &it, ucstring::const_iterator &last, ucstring &result, uint32 *lineCounter, bool allowNewline)
{
	result.erase();

	// parse a string delimited by the specified opening and closing mark
	if (it != last && *it == openMark)
	{
		++it;

		while (it != last && *it != closeMark && (allowNewline || *it != '\n'))
		{
			// ignore tab, new lines and line feed
			if (*it == openMark)
			{
				nlwarning("I18N: Found a non escaped openmark %c in a delimited string (Delimiters : '%c' - '%c')", char(openMark), char(openMark), char(closeMark));
				return false;
			}
			if (*it == '\t'
				|| (*it == '\n' && allowNewline)
				|| *it == '\r')
				++it;
			else if (*it == '\\' && it+1 != last && *(it+1) != '\\')
			{
				++it;
				// this is an escape sequence !
				switch(*it)
				{
				case 't':
					result.push_back('\t');
					break;
				case 'n':
					result.push_back('\n');
					break;
				case 'd':
					// insert a delete
					result.push_back(8);
					break;
				default:
					// escape the close mark ?
					if(*it == closeMark)
						result.push_back(closeMark);
					// escape the open mark ?
					else if(*it == openMark)
						result.push_back(openMark);
					else
					{
						nlwarning("I18N: Ignoring unknown escape code \\%c (char value : %u)", char(*it), *it);
						return false;
					}
				}
				++it;
			}
			else if (*it == '\\' && it+1 != last && *(it+1) == '\\')
			{
				// escape the \ char
				++it;
				result.push_back(*it);
				++it;
			}
			else
			{
				if (*it == '\n' && lineCounter != NULL)
					// update line counter
					++(*lineCounter);

				result.push_back(*it++);
			}
		}

		if (it == last || *it != closeMark)
		{
			nlwarning("I18N: Missing end of delimited string (Delimiters : '%c' - '%c')", char(openMark), char(closeMark));
			return false;
		}
		else
			++it;
	}
	else
	{
		nlwarning("I18N: Malformed or non existent delimited string (Delimiters : '%c' - '%c')", char(openMark), char(closeMark));
		return false;
	}

	return true;
}

void CI18N::readTextFile(const string &filename,
						 ucstring &result,
						 bool forceUtf8,
						 bool fileLookup,
						 bool preprocess,
						 TLineFormat lineFmt,
						 bool warnIfIncludesNotFound)
{
	// create the read context
	TReadContext readContext;

	// call the inner function
	_readTextFile(filename, result, forceUtf8, fileLookup, preprocess, lineFmt, warnIfIncludesNotFound, readContext);

	if (!readContext.IfStack.empty())
	{
		nlwarning("Preprocess: Missing %u closing #endif after parsing %s", readContext.IfStack.size(), filename.c_str() );
	}
}

bool CI18N::matchToken(const char* token, ucstring::const_iterator &it, ucstring::const_iterator end)
{
	ucstring::const_iterator rewind = it;
	skipWhiteSpace(it, end, NULL, false);
	while (it != end && *token != 0 && *it == *token)
	{
		++it;
		++token;
	}

	if (*token == 0)
	{
		// we fund the token
		return true;
	}

	// not found
	it = rewind;
	return false;
}

void CI18N::skipLine(ucstring::const_iterator &it, ucstring::const_iterator end, uint32 &lineCounter)
{
	while (it != end && *it != '\n')
		++it;

	if (it != end)
	{
		++lineCounter;
		++it;
	}
}

void CI18N::_readTextFile(const string &filename,
						 ucstring &result,
						 bool forceUtf8,
						 bool fileLookup,
						 bool preprocess,
						 TLineFormat lineFmt,
						 bool warnIfIncludesNotFound,
						 TReadContext &readContext)
{
	string fullName;
	if (fileLookup)
		fullName = CPath::lookup(filename, false,warnIfIncludesNotFound);
	else
		fullName = filename;

	if (fullName.empty())
		return;

	// If ::lookup is used, the file can be in a bnp and CFile::fileExists fails.
	bool isInBnp = fullName.find('@') != string::npos;
	if (!isInBnp && !CFile::fileExists(fullName))
	{
		nlwarning("CI18N::readTextFile : file '%s' does not exist, returning empty string", fullName.c_str());
		return;
	}

	NLMISC::CIFile	file(fullName);

	// Fast read all the text in binary mode.
	string text;
	text.resize(file.getFileSize());
	if (file.getFileSize() > 0)
		file.serialBuffer((uint8*)(&text[0]), (uint)text.size());

	// Transform the string in ucstring according to format header
	if (!text.empty())
		readTextBuffer((uint8*)&text[0], (uint)text.size(), result, forceUtf8);

	if (preprocess)
	{
		// a string to old the result of the preprocess
		ucstring final;
		// make rooms to reduce allocation cost
		final.reserve(raiseToNextPowerOf2((uint)result.size()));

		// parse the file, looking for preprocessor command.
		ucstring::const_iterator it(result.begin()), end(result.end());

		// input line counter
		uint32 currentLine = 1;

		// set the current file and line info
		final += toString("#fileline \"%s\" %u\n", filename.c_str(), currentLine);

		while (it != end)
		{
			// remember the begin of the line
			ucstring::const_iterator beginOfLine = it;

			// advance in the line, looking for a preprocessor command
			skipWhiteSpace(it, end, NULL, false);

			if (it != end && *it == '#')
			{
				// skip the '#' symbol
				++it;
				// we found a preprocessor command !
				skipWhiteSpace(it, end, NULL, false);

				if (matchToken("include", it, end))
				{
					if (readContext.IfStack.empty() || readContext.IfStack.back())
					{
						// we have an include command
						skipWhiteSpace(it, end, NULL, false);

						// read the file name between quote
						ucstring str;
						breakable
						{
							if (!parseMarkedString(ucchar('\"'), ucchar('\"'), it, end, str, &currentLine, false))
							{
								nlwarning("Preprocess: In file %s(%u) : Error parsing include file command", filename.c_str(), currentLine);

								break;
							}
							else
							{
								// ok, read the subfile
								string subFilename = str.toString();

								// check is file exist
								if (!CFile::fileExists(subFilename))
								{
									// look for the file relative to current file
									subFilename = CFile::getPath(filename)+subFilename;
									if (!CFile::fileExists(subFilename))
									{
										// the include file is not found, issue a warning
										nlwarning("Preprocess: In file %s(%u) : Cannot include file '%s'",
											filename.c_str(), currentLine,
											str.toString().c_str());

										break;
									}
								}

								nlinfo("Preprocess: In file %s(%u) : Including '%s'",
									filename.c_str(), currentLine,
									subFilename.c_str());

								ucstring inserted;
								_readTextFile(subFilename, inserted, forceUtf8, fileLookup, preprocess, lineFmt, warnIfIncludesNotFound, readContext);
								final += inserted;
							}
						}
						// advance to next line
						skipLine(it, end, currentLine);
						// reset filename and line counter
						final += toString("#fileline \"%s\" %u\n", filename.c_str(), currentLine);
					}
				}
				else if (matchToken("optional", it, end))
				{
					if (readContext.IfStack.empty() || readContext.IfStack.back())
					{
						// we have an optional include command
						skipWhiteSpace(it, end, NULL, false);

						// read the file name between quote
						ucstring str;
						breakable
						{
							if (!parseMarkedString('\"', '\"', it, end, str, &currentLine, false))
							{
								nlwarning("Preprocess: In file %s(%u) : Error parsing optional file command", filename.c_str(), currentLine);

								break;
							}
							else
							{
								// ok, read the subfile
								string subFilename = str.toString();

								// check is file exist
								if (!CFile::fileExists(subFilename))
								{
									// look for the file relative to current file
									subFilename = CFile::getPath(filename)+subFilename;
									if (!CFile::fileExists(subFilename))
									{
										// not found but optional, only emit a debug log
										// the include file is not found, issue a warning
										nldebug("Preprocess: In file %s(%u) : Cannot include optional file '%s'",
											filename.c_str(), currentLine,
											str.toString().c_str());

										break;
									}
								}

								nlinfo("Preprocess: In file %s(%u) : Including optional '%s'",
									filename.c_str(), currentLine,
									subFilename.c_str());

								ucstring inserted;
								_readTextFile(subFilename, inserted, forceUtf8, fileLookup, preprocess, lineFmt, warnIfIncludesNotFound, readContext);
								final += inserted;
							}
						}
						// advance to next line
						skipLine(it, end, currentLine);
						// reset filename and line counter
						final += toString("#fileline \"%s\" %u\n", filename.c_str(), currentLine);
					}
				}
				else if (matchToken("define", it, end))
				{
					if (readContext.IfStack.empty() || readContext.IfStack.back())
					{
						skipWhiteSpace(it, end, NULL, false);

						string label;
						if (parseLabel(it, end, label))
						{
							if (readContext.Defines.find(label) != readContext.Defines.end())
							{
								nlinfo("Preprocess: In file %s(%u) : symbol '%s' already defined",
									filename.c_str(), currentLine,
									label.c_str());
							}
							else
							{
								readContext.Defines.insert(label);
							}
						}
						else
						{
							nlwarning("Preprocess: In file %s(%u) : Error parsing #define command", filename.c_str(), currentLine);
						}

						// advance to next line
						skipLine(it, end, currentLine);
						// update filename and line number
						final += toString("#fileline \"%s\" %u\n", filename.c_str(), currentLine);
					}
				}
				else if (matchToken("ifdef", it, end))
				{
					if (readContext.IfStack.empty() || readContext.IfStack.back())
					{
						skipWhiteSpace(it, end, NULL, false);
						string label;
						if (parseLabel(it, end, label))
						{
							if (readContext.Defines.find(label) != 	readContext.Defines.end())
							{
								// symbol defined, push a true
								readContext.IfStack.push_back(true);
							}
							else
							{
								// symbol not defines, push a false
								readContext.IfStack.push_back(false);
							}
						}
						else
						{
							nlwarning("Preprocess: In file %s(%u) : Error parsing #ifdef command", filename.c_str(), currentLine);
						}

						// advance to next line
						skipLine(it, end, currentLine);
						// update filename and line number
						final += toString("#fileline \"%s\" %u\n", filename.c_str(), currentLine);
					}
					else
					{
						// just push to false
						readContext.IfStack.push_back(false);

						skipLine(it, end, currentLine);
					}
				}
				else if (matchToken("ifndef", it, end))
				{
					if (readContext.IfStack.empty() || readContext.IfStack.back())
					{
						skipWhiteSpace(it, end, NULL, false);
						string label;
						if (parseLabel(it, end, label))
						{
							if (readContext.Defines.find(label) == 	readContext.Defines.end())
							{
								// symbol defined, push a true
								readContext.IfStack.push_back(true);
							}
							else
							{
								// symbol not defines, push a false
								readContext.IfStack.push_back(false);
							}
						}
						else
						{
							nlwarning("Preprocess: In file %s(%u) : Error parsing #ifndef command", filename.c_str(), currentLine);
						}

						// advance to next line
						skipLine(it, end, currentLine);
						// update filename and line number
						final += toString("#fileline \"%s\" %u\n", filename.c_str(), currentLine);
					}
					else
					{
						// just push to false
						readContext.IfStack.push_back(false);

						skipLine(it, end, currentLine);
					}
				}
				else if (matchToken("endif", it, end))
				{
					bool previous = false;
					if (readContext.IfStack.empty())
					{
						nlwarning("Preprocess: In file %s(%u) : Error found '#endif' without matching #if", filename.c_str(), currentLine);
					}
					else
					{
						previous = readContext.IfStack.back();

						readContext.IfStack.pop_back();
					}
					skipLine(it, end, currentLine);

					if (!previous && (readContext.IfStack.empty() || readContext.IfStack.back()))
					{
						// end of ignored file part, restore the file and line number
						final += toString("#fileline \"%s\" %u\n", filename.c_str(), currentLine);
					}
					// update filename and line number
//					final += toString("#fileline \"%s\" %u\n", filename.c_str(), currentLine);
				}
				else
				{
					// unrecognized command, ignore line
					nlwarning("Preprocess: In file %s(%u) : Error unrecognized preprocessor command",
						filename.c_str(), currentLine);

					skipLine(it, end, currentLine);
					// update filename and line number
					final += toString("#fileline \"%s\" %u\n", filename.c_str(), currentLine);
				}
			}
			else
			{
				// normal line
				skipLine(it, end, currentLine);

				if (readContext.IfStack.empty() || readContext.IfStack.back())
				{
					// copy the line to the final string
					final.append(beginOfLine, it);
				}
			}
		}

		// set the result with the preprocessed content
		result.swap(final);
	}

	// apply line delimiter conversion if needed
	if (lineFmt != LINE_FMT_NO_CARE)
	{
		if (lineFmt == LINE_FMT_LF)
		{
			// we only want \n
			// easy, just remove or replace any \r code
			string::size_type pos;
			string::size_type lastPos = 0;
			ucstring temp;
			// reserve some place to reduce re-allocation
			temp.reserve(result.size() +result.size()/10);

			// look for the first \r
			pos = result.find('\r');
			while (pos != string::npos)
			{
				if (pos < result.size()-1 && result[pos+1] == '\n')
				{
					temp.append(result.begin()+lastPos, result.begin()+pos);
					pos += 1;
				}
				else
				{
					temp.append(result.begin()+lastPos, result.begin()+pos);
					temp[temp.size()-1] = '\n';
				}

				lastPos = pos;
				// look for next \r
				pos = result.find('\r', pos);
			}

			// copy the rest
			temp.append(result.begin()+lastPos, result.end());

			result.swap(temp);
		}
		else if (lineFmt == LINE_FMT_CRLF)
		{
			// need to replace simple '\n' or '\r' with a '\r\n' double
			string::size_type pos = 0;
			string::size_type lastPos = 0;

			ucstring temp;
			// reserve some place to reduce re-allocation
			temp.reserve(result.size() +result.size()/10);


			// first loop with the '\r'
			pos = result.find('\r', pos);
			while (pos != string::npos)
			{
				if (pos >= result.size()-1 || result[pos+1] != '\n')
				{
					temp.append(result.begin()+lastPos, result.begin()+pos+1);
					temp += '\n';
					lastPos = pos+1;
				}
				// skip this char
				pos++;

				// look the next '\r'
				pos = result.find('\r', pos);
			}

			// copy the rest
			temp.append(result.begin()+lastPos, result.end());
			result.swap(temp);

			temp = "";

			// second loop with the '\n'
			pos = 0;
			lastPos = 0;
			pos = result.find('\n', pos);
			while (pos != string::npos)
			{
				if (pos == 0 || result[pos-1] != '\r')
				{
					temp.append(result.begin()+lastPos, result.begin()+pos);
					temp += '\r';
					temp += '\n';
					lastPos = pos+1;
				}
				// skip this char
				pos++;

				pos = result.find('\n', pos);
			}

			// copy the rest
			temp.append(result.begin()+lastPos, result.end());
			result.swap(temp);
		}
	}
}

void CI18N::readTextBuffer(uint8 *buffer, uint size, ucstring &result, bool forceUtf8)
{
	static uint8 utf16Header[] = { 0xffu, 0xfeu };
	static uint8 utf16RevHeader[] = { 0xfeu, 0xffu };
	static uint8 utf8Header[] = { 0xefu, 0xbbu, 0xbfu };

	if (forceUtf8)
	{
		if (size>=3 &&
			buffer[0]==utf8Header[0] &&
			buffer[1]==utf8Header[1] &&
			buffer[2]==utf8Header[2]
			)
		{
			// remove utf8 header
			buffer+= 3;
			size-=3;
		}
		string text((char*)buffer, size);
		result.fromUtf8(text);
	}
	else if (size>=3 &&
			 buffer[0]==utf8Header[0] &&
			 buffer[1]==utf8Header[1] &&
			 buffer[2]==utf8Header[2]
			)
	{
		// remove utf8 header
		buffer+= 3;
		size-=3;
		string text((char*)buffer, size);
		result.fromUtf8(text);
	}
	else if (size>=2 &&
			 buffer[0]==utf16Header[0] &&
			 buffer[1]==utf16Header[1]
			)
	{
		// remove utf16 header
		buffer+= 2;
		size-= 2;
		// check pair number of bytes
		nlassert((size & 1) == 0);
		// and do manual conversion
		uint16 *src = (uint16*)(buffer);
		result.resize(size/2);
		for (uint j=0; j<result.size(); j++)
			result[j]= *src++;
	}
	else if (size>=2 &&
			 buffer[0]==utf16RevHeader[0] &&
			 buffer[1]==utf16RevHeader[1]
			)
	{
		// remove utf16 header
		buffer+= 2;
		size-= 2;
		// check pair number of bytes
		nlassert((size & 1) == 0);
		// and do manual conversion
		uint16 *src = (uint16*)(buffer);
		result.resize(size/2);
		uint j;
		for (j=0; j<result.size(); j++)
			result[j]= *src++;
		//  Reverse byte order
		for (j=0; j<result.size(); j++)
		{
			uint8 *pc = (uint8*) &result[j];
			swap(pc[0], pc[1]);
		}
	}
	else
	{
		// hum.. ascii read ?
		// so, just do a direct conversion
		string text((char*)buffer, size);
		result = text;
	}
}

void CI18N::writeTextFile(const string filename, const ucstring &content, bool utf8)
{
	COFile file(filename);

	if (!utf8)
	{
		// write the Unicode 16 bits tag
		uint16 unicodeTag = 0xfeff;
		file.serial(unicodeTag);

		uint i;
		for (i=0; i<content.size(); ++i)
		{
			uint16 c = content[i];
			file.serial(c);
		}
	}
	else
	{
		static char utf8Header[] = {char(0xef), char(0xbb), char(0xbf), 0};

		string str = encodeUTF8(content);
		// add the UTF-8 'not official' header
		str = utf8Header + str;

		uint i;
		for (i=0; i<str.size(); ++i)
		{
			file.serial(str[i]);
		}
	}
}

ucstring CI18N::makeMarkedString(ucchar openMark, ucchar closeMark, const ucstring &text)
{
	ucstring ret;

	ret.push_back(openMark);

	ucstring::const_iterator first(text.begin()), last(text.end());
	for (; first != last; ++first)
	{
		if (*first == '\n')
		{
			ret += '\\';
			ret += 'n';
		}
		else if (*first == '\t')
		{
			ret += '\\';
			ret += 't';
		}
		else if (*first == closeMark)
		{
			// escape the embedded closing mark
			ret += '\\';
			ret += closeMark;
		}
		else
		{
			ret += *first;
		}
	}

	ret += closeMark;

	return ret;
}

string CI18N::encodeUTF8(const ucstring &str)
{
	return str.toUtf8();
}

/* UTF-8 conversion table
U-00000000 - U-0000007F:  0xxxxxxx
U-00000080 - U-000007FF:  110xxxxx 10xxxxxx
U-00000800 - U-0000FFFF:  1110xxxx 10xxxxxx 10xxxxxx
// not used as we convert from 16 bits unicode
U-00010000 - U-001FFFFF:  11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
U-00200000 - U-03FFFFFF:  111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
U-04000000 - U-7FFFFFFF:  1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
*/

uint64	CI18N::makeHash(const ucstring &str)
{
	// we do at least 8 pass on each result byte
	if (str.empty())
		return 0;
	const	uint32	MIN_TURN = 8*8;
	uint64	hash = 0;
	uint8	*ph = (uint8*)&hash;
	uint8	*pc = (uint8*)str.data();

	uint nbLoop = max(uint32(str.size()*2), MIN_TURN);
	uint roll = 0;

	for (uint i=0; i<nbLoop; ++i)
	{
		ph[(i/2) & 0x7] = uint8((ph[(i/2) & 0x7] + (pc[i%(str.size()*2)] << roll)) & 0xff);
		ph[(i/2) & 0x7] = uint8((ph[(i/2) & 0x7] + (pc[i%(str.size()*2)] >> (8-roll))) & 0xff);

		roll++;
		roll &= 0x7;
	}

	return hash;
}

// convert a hash value to a readable string
string CI18N::hashToString(uint64 hash)
{
	char temp[] = "0011223344556677";
	sprintf(temp, "%08X%08X", (uint32)(hash & 0xffffffff), (uint32)(hash >> 32));

	return string(temp);
}

// fast convert a hash value to a ucstring
void	CI18N::hashToUCString(uint64 hash, ucstring &dst)
{
	static ucchar	cvtTable[]= {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

	dst.resize(16);
	for(sint i=15;i>=0;i--)
	{
		// Must decal dest of 8, cause of hashToString code (Little Endian)
		dst[(i+8)&15]= cvtTable[hash&15];
		hash>>=4;
	}
}

// convert a readable string into a hash value.
uint64 CI18N::stringToHash(const string &str)
{
	nlassert(str.size() == 16);
	uint32	low, high;

	string sl, sh;
	sh = str.substr(0, 8);
	sl = str.substr(8, 8);

	sscanf(sh.c_str(), "%08X", &high);
	sscanf(sl.c_str(), "%08X", &low);

	uint64 hash;

	memcpy(&hash, &high, sizeof(high));
	memcpy((uint32*)&hash + 1, &low, sizeof(low));

	return hash;
}

} // namespace NLMISC
