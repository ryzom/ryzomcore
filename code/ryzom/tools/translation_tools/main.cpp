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




/** This tool is used to managed translation file.
*	I work with two different file format :
*		- phrase file witch contain a complex grammar description
*		- string file withc contain only pair of identifier / string value.
*	
*	This tool can do 6 different work :
*		- make diff string file file for each language from a reference string file.
*		
*		- merge the translated diff string file into there respective string file after
*			translation
*
*		- make diff phrase file for each language from a reference phrase file
*
*		- merge the translated diff phrase file into there respective phrase file after
*			translation
*
*		- make clause diff for each language by examining phrase files. Add comments
*			in the diff files for phrase parameter information.
*
*		- merge clause diff in all the clause file.
*
*		- remove "\*OLDVALUE: \*\/" from clause file or phrase file
*
*
*	Before invocation, you must be in the translation repository (see localisation_system_in_ryzom.doc)
*	Invocation should be as folow :
*		trans_tool make_string_diff 
*		trans_tool merge_string_diff 
*		trans_tool make_words_diff
*		trans_tool merge_words_diff 
*		trans_tool make_phrase_diff
*		trans_tool merge_phrase_diff
*		trans_tool make_clause_diff
*		trans_tool merge_clause_diff
*		trans_tool clean_string_diff
*		trans_tool clean_words_diff
*		trans_tool clean_clause_diff
*		trans_tool clean_phrase_diff
*		trans_tool make_phrase_diff_old
*		trans_tool merge_phrase_diff_old
*		trans_tool forget_phrase_diff
*		trans_tool update_phrase_work
*		trans_tool inject_clause
*		trans_tool sort_trans_phrase
*		trans_tool make_worksheet_diff
*		trans_tool merge_worksheet_diff
*		trans_tool crop_lines
*		trans_tool extract_bot_names
*		trans_tool extract_new_sheet_names

*/

#include "nel/misc/app_context.h"
#include "nel/misc/i18n.h"
#include "nel/misc/file.h"
#include "nel/misc/path.h"
#include "nel/misc/diff_tool.h"
#include "nel/misc/algo.h"
#include <vector>
#include <list>
#include <algorithm>
#include <stdio.h>
#include <time.h>
#include <iterator>

using namespace std;
using namespace NLMISC;
using namespace STRING_MANAGER;

int extractBotNames(int argc, char *argv[]);
int extractNewSheetNames(int argc, char *argv[]);
const std::string	addDir("work/");
const std::string	diffDir("diff/");
const std::string	transDir("translated/");
const std::string	historyDir("history/");

string				diffVersion;

#ifndef NL_OS_WINDOWS
char* itoa(int val, char *buffer, int base){ 
	static char buf[32] = {0}; 
	int i = 30; 
	for(; val && i ; --i, val /= base) 
		buf[i] = "0123456789abcdef"[val % base]; 
	return &buf[i+1]; 
} 
#endif // NL_OS_WINDOWS

#ifdef NL_DEBUG
# define LOG nldebug
#else
# define LOG printf
#endif

enum TDiffCommand
{
	diff_none,
	diff_add,
	diff_changed,
	diff_removed,
	diff_swap
};

struct TDiffInfo
{
	TDiffCommand	Command;
	uint			Index1;
	uint			Index2;
};

/// Store the list of language extracted from the languages.txt file
vector<string>	Languages;


void showUsage(char *exeName)
{
	LOG("%s usage : \n", exeName);
	LOG("  %s <command> [<filename>]\n", exeName);
	LOG("  Where command can be :\n");
	LOG("    make_string_diff\n");
	LOG("    merge_string_diff\n");
	LOG("    clean_string_diff\n");
	LOG("    make_phrase_diff\n");
	LOG("    merge_phrase_diff\n");
	LOG("    clean_phrase_diff\n");
	LOG("    make_clause_diff\n");
	LOG("    merge_clause_diff\n");
	LOG("    clean_clause_diff\n");
	LOG("    make_phrase_diff_old\n");
	LOG("    merge_phrase_diff_old\n");
	LOG("    forget_phrase_diff\n");
	LOG("    inject_clause\n");
	LOG("    sort_trans_phrase\n");
	LOG("    make_worksheet_diff <filename>\n");
	LOG("    merge_worksheet_diff <filename>\n");
	LOG("    crop_lines <filename> <nbLines>\n");
	LOG("    extract_bot_names [-r]\n");
	LOG("    extract_new_sheet_names [-r]\n");
	LOG("\n");
	LOG("Language code are ISO 639-2 + optionally ISO 3166 country code.\n");
	LOG("Reference language is always the first language in languages.txt\n");
}



void verifyVersion(ucstring& doc, int versionId)
{
	ucstring version1("// DIFF_VERSION 1\r\n");
	ucstring::size_type version1Size = version1.size();
	ucstring version2("// DIFF_VERSION 2\r\n");
	ucstring::size_type version2Size = version2.size();

	switch (versionId)
	{
		case 1: 
			if (doc.size() < version1Size|| doc.substr(0, version1Size) != version1 )
			{
				nlerror("Loading wrong diff version");
				nlassert(0);
			}			
			doc = doc.substr(version1Size);			
			break;

		case 2:
			if (doc.size() < version2Size || doc.substr(0, version2Size) != version2 )
			{
				nlerror("Loading wrong diff version");
				nlassert(0);
			}			
			doc = doc.substr(version2Size);
			break;

		default:
			nlassert(0);
	}


}
bool readPhraseFile1(const std::string &filename, vector<TPhrase> &phrases, bool forceRehash)
{
	ucstring doc;

	CI18N::readTextFile(filename, doc, false, false, false, CI18N::LINE_FMT_CRLF);	
	verifyVersion(doc, 1);
	return readPhraseFileFromString(doc, filename, phrases, forceRehash);
}

bool readPhraseFile2(const std::string &filename, vector<TPhrase> &phrases, bool forceRehash)
{
	ucstring doc;

	CI18N::readTextFile(filename, doc, false, false, false, CI18N::LINE_FMT_CRLF);
	verifyVersion(doc, 2);
	return readPhraseFileFromString(doc, filename, phrases, forceRehash);
}



void getPathContentFiltered(const string &baseName, const string &ext, vector<string> &result)
{
	CPath::getPathContent(diffDir, false, false, true, result);

	uint i;
	for (i=0; i<result.size(); ++i)
	{
		if (result[i].find(baseName) != 0 || result[i].rfind(ext) != result[i].size()-ext.size())
		{
			// remove it from the list
			result.erase(result.begin()+i);
			--i;
		}
	}
}


bool parseDiffCommandFromComment(const ucstring &comments, TDiffInfo &diffInfo)
{
	ucstring::size_type pos = comments.find(ucstring("DIFF "));
	if (pos == string::npos)
		return false;

	pos += 5;
	ucstring::const_iterator it(comments.begin()+pos), last(comments.end());

	string commandStr;
	if (!CI18N::parseLabel(it, last, commandStr))
		return false;

	CI18N::skipWhiteSpace(it, last);
	if (commandStr == "SWAP")
		diffInfo.Command = diff_swap;
	else if (commandStr == "ADD")
		diffInfo.Command = diff_add;
	else if (commandStr == "CHANGED")
		diffInfo.Command = diff_changed;
	else if (commandStr == "REMOVED")
		diffInfo.Command = diff_removed;
	else
	{
		nlwarning("Invalid diff command '%s'", commandStr.c_str());
		diffInfo.Command = diff_none;
		return false;
	}
	
	CI18N::skipWhiteSpace(it, last);
	// ok, parse the index.
	string indexStr;
	if (!CI18N::parseLabel(it, last, indexStr))
		return false;

	NLMISC::fromString(indexStr, diffInfo.Index1);

	if (diffInfo.Command == diff_swap)
	{
		CI18N::skipWhiteSpace(it, last);
		if (!CI18N::parseLabel(it, last, indexStr))
			return false;

		NLMISC::fromString(indexStr, diffInfo.Index2);
	}
	return true;
}


/// Read the languages.txt file.
int readLanguages()
{
	// read the language list file
	ucstring	f;
	CI18N::readTextFile("languages.txt", f);
	string	lang;

	if (f.empty())
	{
		LOG("Error : the file languages.txt is missing or empty !\n");
		return 1;
	}

	ucstring::const_iterator first(f.begin()), last(f.end());
	while (first != last)
	{
		CI18N::skipWhiteSpace(first, last);

		// read a language code
		while (*first != ' ' && *first != '\n' && *first != '\r' && *first != '\t')
			lang += char(*first++);

		if (!lang.empty())
		{
			LOG("Adding language %s\n", lang.c_str());
			Languages.push_back(lang);
			lang.erase();
		}

		CI18N::skipWhiteSpace(first, last);
	}
	if (Languages.empty())
	{
		LOG("Error : the file languages.txt is empty !\n");
		return 1;
	}

	LOG("Found %u language code\n", (uint) Languages.size());

	return 0;
}


/*void appendToFile(const std::string &filename, const ucstring &text)
{
	if (!CFile::fileExists(filename))
	{
		// create the new translatio file
		CI18N::writeTextFile(filename, text);
	}
	else
	{
		// append to the existing file
		FILE *fp = fopen(filename.c_str(), "ab");

		for (uint i=0; i<text.size(); ++i)
		{
			fputc(text[i] & 0xff, fp);
			fputc((text[i]>>8) & 0xff, fp);
		}

		fclose(fp);
	}
}
*/


bool mergeStringDiff(vector<TStringInfo> &strings, const string &language, const string &baseName, const string &ext, bool onlyTranslated, bool archiveDiff = false)
{
	vector<string>	diffs;

	getPathContentFiltered(diffDir+baseName+language+"_diff_", ext, diffs);

	for (uint i=0; i<diffs.size(); ++i)
	{
		if (onlyTranslated)
		{
			// Check if the diff is translated
			ucstring text;
			CI18N::readTextFile(diffs[i], text, false, false, false, CI18N::LINE_FMT_CRLF);
			if (text.find(ucstring("DIFF NOT TRANSLATED")) != ucstring::npos)
			{
				LOG("Diff file [%s] is not translated, merging it later.\n", CFile::getFilename(diffs[i]).c_str());
				for (i=i+1; i<diffs.size(); ++i)
					LOG("  Merge of Diff file [%s] delayed.\n", CFile::getFilename(diffs[i]).c_str());
				return true;
			}
		}

		// we found a diff file for the addition file.
		LOG("Adding %s diff as reference\n", diffs[i].c_str());
		vector<TStringInfo>	diff;
		if (!loadStringFile(diffs[i], diff, false))
			return false;

		for (uint j=0; j<diff.size(); ++j)
		{
/*			TDiffCommand command;
			uint		 index;
			uint		index2;
*/
			TDiffInfo	diffInfo;
			if (!parseDiffCommandFromComment(diff[j].Comments, diffInfo))
				return false;

			switch(diffInfo.Command)
			{
			case diff_swap:
				nlassertex(diffInfo.Index1 < strings.size(), ("Index %u out of max Range %u", diffInfo.Index1, strings.size()));
				nlassertex(diffInfo.Index2 < strings.size(), ("Index %u out of max Range %u", diffInfo.Index2, strings.size()));
				swap(strings[diffInfo.Index1], strings[diffInfo.Index2]);
				// remove the swap from the comments
				diff[j].Comments = diff[j].Comments.substr(diff[j].Comments.find(nl)+2);
				if (!diff[j].Comments.empty())
					j--;
				break;
			case diff_add:
				nlassert(diffInfo.Index1 <= strings.size());
				strings.insert(strings.begin()+diffInfo.Index1, diff[j]);
				break;
			case diff_changed:
				nlassert(diffInfo.Index1 < strings.size());
				strings[diffInfo.Index1] = diff[j];
				break;
			case diff_removed:
				nlassert(diffInfo.Index1 < strings.size());
				strings.erase(strings.begin()+diffInfo.Index1);
				break;
			default:
				nlassert(false);
			}
		}

		if (archiveDiff)
		{
			// move the diff file in the history dir
			CFile::moveFile((historyDir+CFile::getFilename(diffs[i])).c_str(), diffs[i].c_str());
		}
	}

	return true;
}


class CMakeStringDiff : CMakeDiff<TStringInfo, TStringDiffContext>::IDiffCallback
{
public:
	void run(const vector<TStringInfo> &addition, vector<TStringInfo> &reference, vector<TStringInfo> &diff)
	{
		TStringDiffContext context(addition, reference, diff);
		
		CMakeDiff<TStringInfo, TStringDiffContext> differ;
		differ.makeDiff(this, context);
	}

	void onEquivalent(uint addIndex, uint refIndex, TStringDiffContext &context)
	{
		// nothing to do
	}
	void onAdd(uint addIndex, uint refIndex, TStringDiffContext &context)
	{
			TStringInfo si = context.Addition[addIndex];
			char temp[1024];
			sprintf(temp, "// DIFF ADD %u ", addIndex);
			si.Comments = ucstring(temp) + nl + si.Comments;

			nlinfo("Added %s at %u", si.Identifier.c_str(), addIndex);
			context.Diff.push_back(si);
	}
	void onRemove(uint addIndex, uint refIndex, TStringDiffContext &context)
	{
		TStringInfo si = context.Reference[refIndex];
		char temp[1024];
		sprintf(temp, "// DIFF REMOVED %u ", addIndex);
		// NB : on vire les commentaires car il pourrais contenir des merdes..
		si.Comments = ucstring(temp) + nl;

		nlinfo("Removed %s at %u", si.Identifier.c_str(), addIndex);
		context.Diff.push_back(si);
	}
	void onChanged(uint addIndex, uint refIndex, TStringDiffContext &context)
	{
		TStringInfo si = context.Addition[addIndex];
		char temp[1024];
		sprintf(temp, "// DIFF CHANGED %u ", addIndex);
		si.Comments = ucstring(temp) + nl + si.Comments;
		si.Comments = si.Comments + ucstring("/* OLD VALUE : [") + context.Reference[refIndex].Text + "] */" + nl;

		nlinfo("Changed %s at %u", si.Identifier.c_str(), addIndex);
		context.Diff.push_back(si);
	}

	void onSwap(uint newIndex, uint refIndex, TStringDiffContext &context)
	{
		TStringInfo si;
		char temp[1024];
		sprintf(temp, "// DIFF SWAP %u %u   (swaping %s and %s)", newIndex, refIndex, context.Reference[newIndex].Identifier.c_str(), context.Reference[refIndex].Identifier.c_str());
//		sprintf(temp, "// DIFF SWAP %u %u", newIndex, refIndex);
		
		si.Comments = ucstring(temp) + nl +nl;
		context.Diff.push_back(si);
	}


};


void makeStringDiff(const vector<TStringInfo> &addition, vector<TStringInfo> &reference, vector<TStringInfo> &diff)
{
	// just building the object will to the job !
	CMakeStringDiff	differ;
	differ.run(addition, reference, diff);

/*
	// compare the reference an addition file, remove any equivalent strings.
	uint addCount=0, refCount=0;

	while (addCount < addition.size() || refCount < reference.size())
	{
		bool equal = true;
		if (addCount != addition.size() && refCount != reference.size())
		{
			equal = addition[addCount].HashValue == reference[refCount].HashValue;
		}

		vector<TStringInfo>::iterator it;

		if (addCount == addition.size()
			|| 
				(
					!equal
//				&&	find_if(addition.begin()+addCount, addition.end(), TFindStringInfo(reference[refCount].Identifier)) == addition.end()
				&&	find_if(addition.begin(), addition.end(), TFindStringInfo(reference[refCount].Identifier)) == addition.end()
				)
			)
		{
			// this can only be removed elements
			TStringInfo si = reference[refCount];
			char temp[1024];
			sprintf(temp, "// DIFF REMOVED %u ", addCount);
			// NB : on vire les commentaires car il pourrais contenir des merdes..
			si.Comments = ucstring(temp) + nl;

			nlinfo("Removed %s at %u", si.Identifier.c_str(), addCount);
			diff.push_back(si);
			++refCount;
		}
		else if (refCount == reference.size()
			|| 
				(
					!equal
//				&&	find_if(reference.begin()+refCount, reference.end(), TFindStringInfo(addition[addCount].Identifier)) == reference.end()
				&&	find_if(reference.begin(), reference.end(), TFindStringInfo(addition[addCount].Identifier)) == reference.end()
				)
			)
		{
			// this can only be addition
			TStringInfo si = addition[addCount];
			char temp[1024];
			sprintf(temp, "// DIFF ADD %u ", addCount);
			si.Comments = ucstring(temp) + nl + si.Comments;

			nlinfo("Added %s at %u", si.Identifier.c_str(), addCount);
			diff.push_back(si);
			++addCount;
		}
		else if (addition[addCount].Identifier != reference[refCount].Identifier)
		{
			// swap two element.
			vector<TStringInfo>::iterator it = find_if(reference.begin(), reference.end(), TFindStringInfo(addition[addCount].Identifier));
			if (it == reference.end())
			{
				// addition
				TStringInfo si = addition[addCount];
				char temp[1024];
				sprintf(temp, "// DIFF ADD %u ", addCount);
				si.Comments = ucstring(temp) + nl + si.Comments;

				nlinfo("Added %s at %u", si.Identifier.c_str(), addCount);
				diff.push_back(si);
				++addCount;
			}
			else
			{
				nlassert(it != reference.begin()+refCount);

				swap(*it, reference[refCount]);

				TStringInfo si;
				char temp[1024];
				sprintf(temp, "// DIFF SWAP %u %u", it - reference.begin(), refCount);
				
				si.Comments = ucstring(temp) + nl;
				diff.push_back(si);
			}
		}
		else if (addition[addCount].HashValue != reference[refCount].HashValue)
		{
			// changed element
			TStringInfo si = addition[addCount];
			char temp[1024];
			sprintf(temp, "// DIFF CHANGED %u ", addCount);
			si.Comments = ucstring(temp) + nl + si.Comments;
			si.Comments = si.Comments + ucstring("// OLD VALUE : [") + reference[refCount].Text + ']' + nl;

			nlinfo("Changed %s at %u", si.Identifier.c_str(), addCount);
			diff.push_back(si);
			++refCount;
			++addCount;
		}
		else
		{
			// same entry
			nlinfo("Same %s at %u", addition[addCount].Identifier.c_str(), addCount);
			addCount++;
			refCount++;
		}
	}
*/
}

int makeStringDiff(int argc, char *argv[])
{
	// this will generate diff from 'addition' directory 
	// for the reference <lang>.uxt file
	// with the same file in the 'translated' directory.

	// NB : we use standard C file access because there are mutiple file with the same name in different place.

	vector<TStringInfo>	addition;

	LOG("Generating string diffs\nLoading the working file for language %s\n", Languages[0].c_str());
	// load the addition file
	std::string addFile(Languages[0]+".uxt");
	if (!loadStringFile(addDir+addFile, addition, true))
	{
		LOG("Error loading file %s\n", (addDir+addFile).c_str());
		return 1;
	}

	// for each language
	for (uint l=0; l<Languages.size(); ++l)
	{
		LOG("Diffing with language %s...\n", Languages[l].c_str());
		
		if (l != 0)
		{
			addition.clear();

			std::string addFile(Languages[0]+".uxt");
			if (!loadStringFile(transDir+addFile, addition, true))
			{
				LOG("Error loading file %s\n", (transDir+addFile).c_str());
				return 1;
			}
		}

		vector<TStringInfo>	reference;
		// load the reference file
		std::string refFile(Languages[l]+".uxt");
		if (!loadStringFile(transDir+refFile, reference, false))
		{
			LOG("Error loading file %s\n", (transDir+refFile).c_str());
			return 1;
		}

		// load any not merged diff file
		if (!mergeStringDiff(reference, Languages[l], "", ".uxt", false))
		{
			LOG("Error will mergin diff file(s)\n");
			return 1;
		}

		vector<TStringInfo>	diff;
		makeStringDiff(addition, reference, diff);

		if (diff.empty())
		{
			LOG("No difference for %s.\n", Languages[l].c_str());
		}
		else
		{
			LOG("Writing difference file for %s.\n", Languages[l].c_str());
			// build the diff file for each language.
			ucstring str = prepareStringFile(diff, false);

			// add the tag for non translation
			str += nl + ucstring ("// REMOVE THE FOLOWING LINE WHEN TRANSLATION IS DONE")+nl+ucstring("// DIFF NOT TRANSLATED")+nl;

			std::string diffName(diffDir+Languages[l]+"_diff_"+diffVersion+".uxt");
			CI18N::writeTextFile(diffName, str);
			
		}
	}

	return 0;
}

/*
Remove the OLD VALUE from a file.
*/
void cleanComment(const std::string & filename)
{
	ucstring text;
	uint nbOldValue=0;

	CI18N::readTextFile(filename, text, false, false, false, CI18N::LINE_FMT_CRLF);

	ucstring newText;
	ucstring::size_type last = 0;
	while ( last != ucstring::npos)
	{
		ucstring::size_type commentBegin = text.find(ucstring("/* OLD VALUE :"), last);
		if (commentBegin == ucstring::npos)
		{					
			newText += text.substr(last);
			last = ucstring::npos;
		}
		else
		{
			ucstring::size_type size = commentBegin - last;												
			ucstring toAdd = text.substr(last, size);
			newText += toAdd;
			ucstring::size_type commentEnd = text.find(ucstring("*/"), commentBegin);
			if (commentEnd != ucstring::npos) { commentEnd += 4; }
			last = commentEnd;
			++nbOldValue;
		}				
	}
	text = newText;
	newText = ucstring("");
	last = 0;
	while ( last != ucstring::npos)
	{
		ucstring::size_type commentBegin = text.find(ucstring("//"), last);
		if (commentBegin == ucstring::npos)
		{					
			newText += text.substr(last);
			last = ucstring::npos;
		}
		else
		{
			ucstring::size_type size = commentBegin - last;
			ucstring toAdd =  text.substr(last, size); 
			newText += toAdd;
			// case where // is the part of an url and isn't a comment
			if (commentBegin > 4 && text.substr(commentBegin-1, 1) == ucstring(":"))
			{
				newText += "//";
				last = commentBegin+2;
			}
			else
			{
				ucstring::size_type commentEnd = text.find(ucstring("\n"), commentBegin);
				if (commentEnd != ucstring::npos)
				{
					commentEnd += 1;
					ucstring comment = text.substr(commentBegin, commentEnd - commentBegin);
					if (comment.find(ucstring("// HASH_VALUE")) != ucstring::npos
						|| comment.find(ucstring("// DIFF")) != ucstring::npos
						|| comment.find(ucstring("// REMOVE")) != ucstring::npos
						|| comment.find(ucstring("// INDEX")) != ucstring::npos
						)
					{
						newText += comment;
					}	
				}
				last = commentEnd;
				++nbOldValue;
			}
		}				
	}
	nlinfo("cleaning : %s, (%d comments deleted)...\n", filename.c_str(), nbOldValue);
	CI18N::writeTextFile(filename , newText);
}

/*
REMOVE OLDVALUE: from a diff string file
*/
int cleanStringDiff(int argc, char *argv[])
{
	
	LOG("Cleaning string diffs\n");

	uint i,l;

	for (l=0; l<Languages.size(); ++l)
	{
		
		vector<string>	diffs;

		getPathContentFiltered(diffDir+Languages[l]+"_diff_", ".uxt", diffs);
		for (i=0; i<diffs.size(); ++i)
		{
				cleanComment(diffs[i]);
		}
	}
	return 0;
}

int mergeStringDiff(int argc, char *argv[])
{
	LOG("Merging string diffs\n");

	// for each language
	uint l;
	for (l=0; l<Languages.size(); ++l)
	{
		LOG("Merging for language %s...\n", Languages[l].c_str());
		string filename = transDir+Languages[l]+".uxt";
		// load the translated file
		vector<TStringInfo>	translated;
		if (!loadStringFile(filename, translated, false))
		{
			LOG("Error will loading file %s\n", filename.c_str());
			return 1;
		}

		// append the translated diffs
		mergeStringDiff(translated, Languages[l], "", ".uxt", true, true);

		// prepare the addition string
		ucstring str = prepareStringFile(translated, true);

		{
			// backup the original file
			ucstring old;
			CI18N::readTextFile(filename, old, false, true, false, CI18N::LINE_FMT_CRLF);
			if (old != str)
				CFile::moveFile((historyDir+CFile::getFilenameWithoutExtension(filename)+"_"+diffVersion+"."+CFile::getExtension(filename)).c_str(), filename.c_str());
		}

		CI18N::writeTextFile(filename, str);
	}

	return 0;
}


/*
struct TFindPhrase : unary_function<TPhrase, bool>
{
	string	Identifier;
	TFindPhrase (const string &identifier)
		: Identifier(identifier)
	{}
	bool operator () (const TPhrase &phrase)
	{
		return phrase.Identifier == Identifier;
	}
};
*/


bool mergePhraseDiff2(vector<TPhrase> &phrases, const string &language, bool onlyTranslated, bool archiveDiff);
bool mergePhraseDiff(vector<TPhrase> &phrases, const string &language, bool onlyTranslated, bool archiveDiff = false)
{
	vector<string>	diffs;

	getPathContentFiltered(diffDir+"phrase_"+language+"_diff_", ".txt", diffs);

	for (uint i=0; i<diffs.size(); ++i)
	{
		if (onlyTranslated)
		{
			// Check if the diff is translated
			ucstring text;
			CI18N::readTextFile(diffs[i], text, false, false, false, CI18N::LINE_FMT_CRLF);
			verifyVersion(text, 1);
			if (text.find(ucstring("DIFF NOT TRANSLATED")) != ucstring::npos)
			{
				LOG("Diff file [%s] is not translated, merging it later.\n", CFile::getFilename(diffs[i]).c_str());
				for (i=i+1; i<diffs.size(); ++i)
					LOG("  Merge of Diff file [%s] delayed.\n", CFile::getFilename(diffs[i]).c_str());
				return true;
			}
		}

		// we found a diff file for the addition file.
		LOG("Adding %s diff as reference\n", diffs[i].c_str());
		vector<TPhrase>	diff;
		if (!readPhraseFile1(diffs[i], diff, false))
			return false;

		for (uint j=0; j<diff.size(); ++j)
		{
/*			TDiffCommand command;
			uint		 index;
			uint		index2;
*/
			TDiffInfo	diffInfo;
			if (!parseDiffCommandFromComment(diff[j].Comments, diffInfo))
			{
				if (j == diff.size()-1)
					break;
				else
				{
					nlwarning("Failed to parse diff command in '%s'", diff[j].Identifier.c_str());
					return false;
				}
			}

			switch(diffInfo.Command)
			{
			case diff_swap:
				nlassertex(diffInfo.Index1 <= phrases.size(), 
					("In SWAP, Index1 (%u) is not less than number of phrase (%u)", diffInfo.Index1, phrases.size()));
				nlassertex(diffInfo.Index2 <= phrases.size(), 
					("In SWAP Index2 (%u) is not less than number of phrase (%u)", diffInfo.Index2, phrases.size()));
				swap(phrases[diffInfo.Index1], phrases[diffInfo.Index2]);
				// remove the swap from the comments
				diff[j].Comments = diff[j].Comments.substr(diff[j].Comments.find(nl)+2);
				j--;
				break;
			case diff_add:
				nlassertex(diffInfo.Index1 <= phrases.size(), 
					("In ADD, Index1 (%u) is not less than number of phrase (%u)", diffInfo.Index1, phrases.size()));
				phrases.insert(phrases.begin()+diffInfo.Index1, diff[j]);
				break;
			case diff_changed:
				nlassertex(diffInfo.Index1 < phrases.size(),
					("In CHANGED, Index1 (%u) is not less than number of phrase (%u)", diffInfo.Index1, phrases.size()));
				phrases[diffInfo.Index1] = diff[j];
				break;
			case diff_removed:
				nlassertex(diffInfo.Index1 < phrases.size(),
					("In REMOVED, Index1 (%u) is not less than number of phrase (%u)", diffInfo.Index1, phrases.size()));
				phrases.erase(phrases.begin()+diffInfo.Index1);
				break;
			default:
				nlassert(false);
			}
		}

		if (archiveDiff)
		{
			// move the diff file in the history dir
			CFile::moveFile((historyDir+CFile::getFilename(diffs[i])).c_str(), diffs[i].c_str());
		}
	}

	return true;
}



class CMakePhraseDiff : CMakeDiff<TPhrase, TPhraseDiffContext>::IDiffCallback
{
public:
	void run(const vector<TPhrase> &addition, vector<TPhrase> &reference, vector<TPhrase> &diff)
	{
		TPhraseDiffContext context(addition, reference, diff);
		
		CMakeDiff<TPhrase, TPhraseDiffContext> differ;
		differ.makeDiff(this, context);
	}

	void onEquivalent(uint addIndex, uint refIndex, TPhraseDiffContext &context)
	{
		// nothing to do
	}
	void onAdd(uint addIndex, uint refIndex, TPhraseDiffContext &context)
	{
		TPhrase phrase = context.Addition[addIndex];
		char temp[1024];
		sprintf(temp, "// DIFF ADD %u ", addIndex);
		phrase.Comments = ucstring(temp) + nl + phrase.Comments;

		nlinfo("Added %s at %u", phrase.Identifier.c_str(), addIndex);
		context.Diff.push_back(phrase);
	}
	void onRemove(uint addIndex, uint refIndex, TPhraseDiffContext &context)
	{
		TPhrase phrase = context.Reference[refIndex];
		char temp[1024];
		sprintf(temp, "// DIFF REMOVED %u ", addIndex);
		// NB : on vire les commentaires car il pourrai contenir des merdes..
		phrase.Comments = ucstring(temp) + nl;
		for (uint i=0; i<phrase.Clauses.size(); ++i)
			phrase.Clauses[i].Comments.erase();

		nlinfo("Removed %s at %u", phrase.Identifier.c_str(), addIndex);
		context.Diff.push_back(phrase);
	}
	void onChanged(uint addIndex, uint refIndex, TPhraseDiffContext &context)
	{
		ucstring chg;
		char temp[1024];
		// check what is changed.
		if (context.Addition[addIndex].Parameters != context.Reference[refIndex].Parameters)
			chg += "// Parameter list changed." + nl;
		if (context.Addition[addIndex].Clauses.size() != context.Reference[refIndex].Clauses.size())
			chg += "// Clause list changed." + nl;
		else
		{
			for (uint i=0; i<context.Addition[addIndex].Clauses.size(); ++i)
			{
				if (context.Addition[addIndex].Clauses[i].Identifier != context.Reference[refIndex].Clauses[i].Identifier)
					chg += ucstring("// Clause ")+itoa(i, temp, 10) + " : identifier changed." + nl;
				else if (context.Addition[addIndex].Clauses[i].Conditions != context.Reference[refIndex].Clauses[i].Conditions)
					chg += ucstring("// Clause ")+itoa(i, temp, 10) + " : condition changed." + nl;	
				else if (context.Addition[addIndex].Clauses[i].Text != context.Reference[refIndex].Clauses[i].Text)
					chg += ucstring("// Clause ")+itoa(i, temp, 10) + " : text changed." + nl;	
			}
		}

		if (chg.empty())
		{
			chg = ucstring("// WARNING : Hash code changed ! check translation workflow.") + nl;
		}
		nldebug("Changed detected : %s", chg.toString().c_str());
		
		// changed element
		TPhrase phrase = context.Addition[addIndex];
//				char temp[1024];
		sprintf(temp, "// DIFF CHANGED %u ", addIndex);
		vector<TPhrase>	tempV;
		tempV.push_back(context.Reference[refIndex]);
		ucstring tempT = preparePhraseFile(tempV, false); 
		CI18N::removeCComment(tempT);
		phrase.Comments = ucstring(temp) + nl + phrase.Comments;
		phrase.Comments = phrase.Comments + ucstring("/* OLD VALUE : ["+nl) + tabLines(1, tempT) +nl + "] */" + nl;
		phrase.Comments = phrase.Comments + chg;

		nlinfo("Changed %s at %u", phrase.Identifier.c_str(), addIndex);
		context.Diff.push_back(phrase);
	}

	void onSwap(uint newIndex, uint refIndex, TPhraseDiffContext &context)
	{
		TPhrase phrase;
		char temp[1024];
		sprintf(temp, "// DIFF SWAP %u %u   (swaping %s and %s)", newIndex, refIndex, context.Reference[newIndex].Identifier.c_str(), context.Reference[refIndex].Identifier.c_str());
		
		nldebug("Swap for %u %u", newIndex, refIndex);
		phrase.Comments = ucstring(temp) + nl;
		context.Diff.push_back(phrase);
	}

};



int makePhraseDiff(int argc, char *argv[])
{
	// Generate the diff file from phrase_<lang>.txt compared to the same file in translated.
	// The diff is generated only from the reference language for and all the languages
	
	LOG("Generating phrase diffs\nLoading the working file for language %s\n", Languages[0].c_str());


	vector<TPhrase>	addition;

	// read	addition
	if (!readPhraseFile(addDir+"phrase_"+Languages[0]+".txt", addition, true))
	{
		LOG("Error will loading file %s", (addDir+"phrase_"+Languages[0]+".txt").c_str());
		return 1;
	}

	for (uint l =0; l<Languages.size(); ++l)
	{
		LOG("Diffing with language %s...\n", Languages[l].c_str());
		if (l == 1)
		{
			addition.clear();
			// read the language 0 translated version as addition for other language
			if (!readPhraseFile(transDir+"phrase_"+Languages[0]+".txt", addition, true))
			{
				LOG("Error will loading file %s", (addDir+"phrase_"+Languages[0]+".txt").c_str());
				return 1;
			}
		}
		vector<TPhrase>	reference;
		// read the reference file
		if (!readPhraseFile(transDir+"phrase_"+Languages[l]+".txt", reference, false))
		{
			LOG("Error will loading file %s", (transDir+"phrase_"+Languages[l]+".txt").c_str());
			return 1;
		}

		if (!mergePhraseDiff(reference, Languages[l], false))
		{
			LOG("Error will merging phrase diff for language %s\n", Languages[l].c_str());
			return 1;
		}

		// compare the reference an addition file, remove any equivalent strings.
		uint addCount=0, refCount=0;
		vector<TPhrase>	diff;

		CMakePhraseDiff	differ;
		differ.run(addition, reference, diff);

		if (diff.empty())
		{
			LOG("No difference for language %s\n", Languages[l].c_str());
		}
		else
		{
			LOG("Writing difference file for language %s\n", Languages[l].c_str());
			ucstring text; 
			text += "// DIFF_VERSION 1\r\n";
			text += preparePhraseFile(diff, false);
			// add the tag for non translation
			text += nl + ucstring ("// REMOVE THE FOLOWING LINE WHEN TRANSLATION IS DONE")+nl+ucstring("// DIFF NOT TRANSLATED")+nl;
			CI18N::writeTextFile(diffDir+"phrase_"+Languages[l]+"_diff_"+diffVersion+".txt", text);
		}
	}

	return 0;
}


/*
REMOVE OLDVALUE: from a diff clause file
*/
int cleanPhraseDiff(int argc, char *argv[])
{	
	
	LOG("Cleaning phrase diffs\n");
	
	uint i,l;

	for (l=0; l<Languages.size(); ++l)
	{
		
		vector<string>	diffs;

		getPathContentFiltered(diffDir+"phrase_"+Languages[l]+"_diff_", ".txt", diffs);
		for (i=0; i<diffs.size(); ++i)
		{
			cleanComment(diffs[i]);
				
		}
	}
	return 0;
}


int mergePhraseDiff(int argc, char *argv[], int version)
{
	// merge all the phrase diff back into there repective translated phrase.
	uint l;

	LOG("Merging phrase diffs\n");

	for (l=0; l<Languages.size(); ++l)
	{
		LOG("Merging for language %s...\n", Languages[l].c_str());
		std::string basename("phrase_"+Languages[l]);
		string filename = transDir+basename+".txt";
		// build the addition diff
		vector<TPhrase>	reference;

		ucstring doc;


		if (!readPhraseFile(transDir+basename+".txt", reference, false))
		{
			LOG("Error will loading file %s", (transDir+basename+".txt").c_str());
			return 1;
		}


		switch(version)
		{
			case 1:				
				if (!mergePhraseDiff(reference, Languages[l], true, true))
				{
					LOG("Error will merging phrase diff");
					return 1;
				}
				break;

			case 2:
				
				if (!mergePhraseDiff2(reference, Languages[l], true, true))
				{
					LOG("Error will merging phrase diff");
					return 1;
				}
				break;

			default:
				nlassert(0);				

		}
		
		ucstring str = preparePhraseFile(reference, true);

		{
			// backup the original file
			ucstring old;
			CI18N::readTextFile(filename, old, false, true, false, CI18N::LINE_FMT_CRLF);
			if (old != str)
				CFile::moveFile((historyDir+CFile::getFilenameWithoutExtension(filename)+"_"+diffVersion+"."+CFile::getExtension(filename)).c_str(), filename.c_str());
		}

		CI18N::writeTextFile(transDir+basename+".txt", str);

	}

	return 0;
}


int makeClauseDiff(int argc, char *argv[])
{
	// this will generate diff from 'addition' directory 
	// for all the clause_<lang>.txt file
	// with the same file in the 'translated' directory.

	// NB : we use standard C file access because there are mutiple file with the same name in different place.

	LOG("Generating clause diffs\n");

	uint i,l;

	for (l=0; l<Languages.size(); ++l)
	{
		LOG("Diffing with language %s...\n", Languages[l].c_str());
		std::string basename("clause_"+Languages[l]);
		vector<TStringInfo>	addition;
		vector<TStringInfo>	reference;
		vector<TPhrase>		phrases;
		std::vector<std::string> warnings;

		// load the reference file
		std::string refFile(basename+".txt");
		if (!loadStringFile(transDir+refFile, reference, false))
		{
			LOG("Error will loading file %s", (transDir+refFile).c_str());
			return 1;
		}

		// load the addition file
		std::string addFile("phrase_"+Languages[l]+".txt");
		if (!readPhraseFile(transDir+addFile, phrases, true))
		{
			LOG("Error will loading file %s", (transDir+addFile).c_str());
			return 1;
		}

		// extract all the clauses from the phrases file
		vector<TPhrase>::iterator first(phrases.begin()), last(phrases.end());
		for (; first != last; ++first)
		{
			TPhrase &p = *first;
			for (i=0; i<p.Clauses.size(); ++i)
			{
				TStringInfo si;
				si.Comments = p.Clauses[i].Comments;
				si.Identifier = p.Clauses[i].Identifier;
				si.Text = p.Clauses[i].Text;
				si.HashValue = CI18N::makeHash(si.Text);


				if (!si.Identifier.empty())
				{				
					vector<TStringInfo>::const_iterator first2 = addition.begin();
					vector<TStringInfo>::const_iterator last2 = addition.end();
					for ( ;first2!=last2 && first2->Identifier != si.Identifier; ++first2) {}
					bool isAllreadyThere = first2 != last2;
					if (isAllreadyThere)
					{
						warnings.push_back("The clause " +si.Identifier +" in the phrase " +  p.Identifier +" exists more than once.");						
					}
					else
					{
						addition.push_back(si);
					}								
				}
			}
		}

		if (!warnings.empty())
		{
			std::vector<std::string>::const_iterator first = warnings.begin();
			std::vector<std::string>::const_iterator last = warnings.end();
			for (;first != last; ++first) { nlwarning("%s", first->c_str()); }
			return -1;
		}
		mergeStringDiff(reference, Languages[l], "clause_", ".txt", false);
	
		vector<TStringInfo>	diff;

		makeStringDiff(addition, reference, diff);

		if (diff.empty())
		{
			LOG("No difference for language %s\n", Languages[l].c_str());
		}
		else
		{
			LOG("Writing difference file for %s.\n", Languages[l].c_str());
			// build the diff file for each language.
			ucstring str = prepareStringFile(diff, false);

			// add the tag for non translation
			str += nl + ucstring ("// REMOVE THE FOLOWING LINE WHEN TRANSLATION IS DONE")+nl+ucstring("// DIFF NOT TRANSLATED")+nl;

			std::string diffName(diffDir+"clause_"+Languages[l]+"_diff_"+diffVersion+".txt");
			CI18N::writeTextFile(diffName, str);
		}
	}

	return 0;
}


/*
REMOVE OLDVALUE: from a diff clause file
*/
int cleanClauseDiff(int argc, char *argv[])
{
	
	LOG("Cleaning clause diffs\n");

	uint i,l;

	for (l=0; l<Languages.size(); ++l)
	{
		
		std::string basename("clause_"+Languages[l]);
		
		vector<string>	diffs;

		getPathContentFiltered(diffDir+"clause_"+Languages[l]+"_diff_", ".txt", diffs);
		for (i=0; i<diffs.size(); ++i)
		{
				cleanComment(diffs[i]);
		}
	}
	return 0;
}

int mergeClauseDiff(int argc, char *argv[])
{
	LOG("Merging clause diffs\n");
	// for each language
	uint l;
	for (l=0; l<Languages.size(); ++l)
	{
		LOG("Merging for language %s...\n", Languages[l].c_str());
		string filename = transDir+"clause_"+Languages[l]+".txt";
		// load the translated file
		vector<TStringInfo>	translated;
		if (!loadStringFile(filename, translated, false))
		{
			LOG("Error will loading file %s", filename.c_str());
			return 1;
		}

		// append the translated diffs
		mergeStringDiff(translated, Languages[l], "clause_", ".txt", true, true);

		// prepare the addition string
		ucstring str = prepareStringFile(translated, true);

		{
			// backup the original file
			ucstring old;
			CI18N::readTextFile(filename, old, false, true, false, CI18N::LINE_FMT_CRLF);
			if (old != str)
				CFile::moveFile((historyDir+CFile::getFilenameWithoutExtension(filename)+"_"+diffVersion+"."+CFile::getExtension(filename)).c_str(), filename.c_str());
		}

		CI18N::writeTextFile(filename, str);
	}

	return 0;

	return 0;
}

bool mergeWorksheetDiff(const std::string filename, TWorksheet &sheet, bool onlyTranslated, bool archiveDiff)
{
	std::string fn(CFile::getFilenameWithoutExtension(filename)), ext(CFile::getExtension(filename));
	vector<string> fileList;
	getPathContentFiltered(diffDir+fn+"_diff_", ext, fileList);

	uint i;
	for (i=0; i<fileList.size(); ++i)
	{
		if (onlyTranslated)
		{
			ucstring text;
			CI18N::readTextFile(fileList[i], text, false, false, false, CI18N::LINE_FMT_CRLF);
			if (text.find(ucstring("DIFF NOT TRANSLATED")) != ucstring::npos)
			{
				LOG("Diff file [%s] is not translated, merging it later.\n", CFile::getFilename(fileList[i]).c_str());
				for (i=i+1; i<fileList.size(); ++i)
					LOG("  Merge of Diff file [%s] delayed.\n", CFile::getFilename(fileList[i]).c_str());
				return true;
			}
		}

		TWorksheet diff;
		if (!loadExcelSheet(fileList[i], diff, false))
			return false;
		makeHashCode(diff, false);

		uint cmdCol = 0;
		if (!diff.findCol(ucstring("DIFF_CMD"), cmdCol))
		{
			LOG("Can't find DIFF_CMD column in %s ! Invalid diff file.\n", CFile::getFilename(fileList[i]).c_str());
			return false;
		}

		// we found a diff file for the addition file.
		LOG("Adding %s diff as reference\n", fileList[i].c_str());

		for (uint j=1; j<diff.Data.size(); ++j)
		{
			TDiffInfo	diffInfo;
			if (!parseDiffCommandFromComment(diff.getData(j, cmdCol), diffInfo))
			{
				if (diff.getData(j, cmdCol).find(ucstring("REMOVE THE FOLOWING TWO LINE WHEN TRANSLATION IS DONE")) == ucstring::npos
					&& diff.getData(j, cmdCol).find(ucstring("DIFF NOT TRANSLATED")) == ucstring::npos)
					return false;
				else
					continue;
			}

			switch(diffInfo.Command)
			{
			case diff_add:
				{
					nlassertex(diffInfo.Index1 <= sheet.Data.size(),
						("ADD cmd in diff file reference row %u, but worksheet only contains %u entries",
							diffInfo.Index1, sheet.Data.size()));
					TWorksheet::TRow row(sheet.ColCount);
					sheet.Data.insert(sheet.Data.begin()+diffInfo.Index1, row);
					for (uint k=0; k<diff.ColCount; ++k)
					{
						if (k != cmdCol)
							sheet.setData(diffInfo.Index1, diff.Data[0][k], diff.Data[j][k]);
					}
				}
				break;
			case diff_changed:
				{
					nlassertex(diffInfo.Index1 <= sheet.Data.size(),
						("CHANGED cmd in diff file reference row %u, but worksheet only contains %u entries",
							diffInfo.Index1, sheet.Data.size()));
					for (uint k=0; k<diff.ColCount; ++k)
					{
						if (k != cmdCol)
							sheet.setData(diffInfo.Index1, diff.Data[0][k], diff.Data[j][k]);
					}
				}
				break;
			case diff_removed:
				nlassertex(diffInfo.Index1 < sheet.Data.size(),
						("REMOVE cmd in diff file reference row %u, but worksheet only contains %u entries",
							diffInfo.Index1, sheet.Data.size()));
//				nlassertex(diffInfo.Index1 > 0);
				sheet.Data.erase(sheet.Data.begin() + diffInfo.Index1);
				break;
			case diff_swap:
				nlassertex(diffInfo.Index1 < sheet.Data.size(),
						("SWAP cmd in diff file, first index reference row %u, but worksheet only contains %u entries",
							diffInfo.Index1, sheet.Data.size()));
//				nlassertex(diffInfo.Index1 > 0);
				nlassertex(diffInfo.Index2 < sheet.Data.size(),
						("SWAP cmd in diff file, second index reference row %u, but worksheet only contains %u entries",
							diffInfo.Index1, sheet.Data.size()));
//				nlassertex(diffInfo.Index2 > 0);
				swap(sheet[diffInfo.Index1], sheet[diffInfo.Index2]);
				break;
			default:
				nlassert(false);
			}

		}

		if (archiveDiff)
		{
			// move the diff file in the history dir
			CFile::moveFile((historyDir+CFile::getFilename(fileList[i])).c_str(), fileList[i].c_str());
		}
	}

	return true;
}



bool mergeSheetDiff(const string &type, TWorksheet &sheet, const string &language, bool onlyTranslated, bool archiveDiff)
{
	return mergeWorksheetDiff(type+"_words_"+language+".txt", sheet, onlyTranslated, archiveDiff);
}


class CMakeWordsDiff : public TWorkSheetDiff::IDiffCallback
{
public:
	void run(const TWorksheet &addition, TWorksheet &reference, TWorksheet &diff)
	{
		TWordsDiffContext context(addition, reference, diff);
		
		TWorkSheetDiff	differ;
		differ.makeDiff(this, context, true);
	}

	void onEquivalent(uint addIndex, uint refIndex, TWordsDiffContext &context)
	{
		// nothing to do
	}
	void onAdd(uint addIndex, uint refIndex, TWordsDiffContext &context)
	{
		TWorksheet::TRow row(context.Reference.ColCount+1);
		for (uint j=0; j<context.Addition.ColCount; ++j)
		{
			uint colIndex = 0;
			if (context.Reference.findCol(context.Addition.Data[0][j], colIndex))
			{
				row[colIndex+1] = context.Addition.Data[addIndex][j];
			}
		}
		char temp[1024];
		sprintf(temp, "DIFF ADD %u ", addIndex);
		row[0] = ucstring(temp);

		nlinfo("Added %s at %u", row[2].toString().c_str(), addIndex);
		context.Diff.insertRow((uint)context.Diff.Data.size(), row);
	}
	void onRemove(uint addIndex, uint refIndex, TWordsDiffContext &context)
	{
		TWorksheet::TRow row(context.Reference.ColCount+1);
		for (uint j=0; j<context.Reference.ColCount; ++j)
		{
			uint colIndex = 0;
			if (context.Reference.findCol(context.Reference.Data[0][j], colIndex))
			{
				row[colIndex+1] = context.Reference.Data[refIndex][j];
			}
		}
		char temp[1024];
		sprintf(temp, "DIFF REMOVED %u ", refIndex);
		row[0] = ucstring(temp);

		nlinfo("Removed %s at %u", row[2].toString().c_str(), refIndex);
		context.Diff.insertRow((uint)context.Diff.Data.size(), row);
	}
	void onChanged(uint addIndex, uint refIndex, TWordsDiffContext &context)
	{
		TWorksheet::TRow row; //(context.Reference.ColCount+1);
		// copy the old content (this fill data in column that don't exist in addition worksheet)
		row = context.Reference.Data[refIndex];
		row.insert(row.begin(), ucstring());

		// changed element
		for (uint j=0; j<context.Addition.ColCount; ++j)
		{
			uint colIndex = 0;
			if (context.Reference.findCol(context.Addition.Data[0][j], colIndex))
			{
				row[colIndex+1] = context.Addition.Data[addIndex][j];
			}
		}

		char temp[1024];
		sprintf(temp, "DIFF CHANGED %u ", addIndex);
		row[0] = temp;

		nlinfo("Changed %s at %u", row[2].toString().c_str(), addIndex);
		context.Diff.insertRow((uint)context.Diff.Data.size(), row);
	}

	void onSwap(uint newIndex, uint refIndex, TWordsDiffContext &context)
	{
		TWorksheet::TRow row(context.Reference.ColCount+1);
		// swap
		char temp[1024];
		sprintf(temp, "DIFF SWAP %u %u", newIndex, refIndex);
		row[0] = temp;

		nlinfo("Swap %u with %u", newIndex, refIndex);
		context.Diff.insertRow((uint)context.Diff.Data.size(), row);
	}

};

/*
REMOVE OLDVALUE: from a diff words file
*/
int cleanWordsDiff(int argc, char *argv[])
{
	
	LOG("Cleaning words diffs\n");
/*
	uint i,l;

	for (l=0; l<Languages.size(); ++l)
	{
		
		vector<string>	diffs;

		getPathContentFiltered(diffDir+"clause_"+Languages[l]+"_diff_", ".txt", diffs);
		for (i=0; i<diffs.size(); ++i)
		{
				cleanComment(diffs[i]);
		}
	}
*/
	return 0;
}

int makeWorksheetDiff(int argc, char *argv[], const std::string &additionFilename, const std::string &referenceFilename, bool firstLanguage)
{
/*	if (argc != 3)
	{
		LOG("ERROR : makeWorksheetDiff need a worksheet file in parameter !");
		return 1;
	}
	std::string filename = argv[2];
*/
	LOG("Loading working for %s...\n", referenceFilename.c_str());

	// loads the working file
	TWorksheet addition;
	if (firstLanguage)
	{
		if (!loadExcelSheet(addDir + additionFilename, addition))
			return false;
	}
	else
	{
		if (!loadExcelSheet(transDir + additionFilename, addition))
			return false;
	}

	makeHashCode(addition, true);

	TWorksheet reference;
	if (CFile::fileExists(transDir+referenceFilename))
	{
		// load the sheet
		if (!loadExcelSheet(transDir+referenceFilename, reference))
		{
			LOG("Error reading worksheet file '%s'", (transDir+referenceFilename).c_str());
			return false;
		}
	}
	if (!CFile::fileExists(transDir+referenceFilename))
	{
		// init the reference column with addition column
		TWorksheet::TRow	row(addition.ColCount);
		for (uint j=0; j<addition.ColCount; ++j)
		{
			nldebug("Adding column %s into reference sheet", addition.Data[0][j].toString().c_str());
			row[j] = addition.Data[0][j];
			reference.insertColumn(0);
		}
		reference.insertRow(0, row);
	}
	makeHashCode(reference, false);

	mergeWorksheetDiff(referenceFilename, reference, false, false);
//	mergeSheetDiff(type, reference, Languages[l], false, false);

	// generate the diff
	TWorksheet diff;
	TWorksheet::TRow row(reference.ColCount+1);
	// create the needed column.
	row[0] = ucstring("DIFF_CMD");
	diff.insertColumn(0);
	for (uint j=0; j<reference.ColCount; ++j)
	{
		row[j+1] = reference.Data[0][j];
		diff.insertColumn(j+1);
	}
	diff.insertRow(0, row);

	CMakeWordsDiff differ;
	differ.run(addition, reference, diff);


	// write the diff file
	if (diff.Data.size() <= 1)
	{
		LOG("No difference for '%s'.\n", referenceFilename.c_str());
	}
	else
	{
		LOG("Writing difference file for %s.\n", referenceFilename.c_str());
		// build the diff file for each language.
		ucstring str = prepareExcelSheet(diff);

		// add the tag for non translation
		str += ucstring ("REMOVE THE FOLOWING TWO LINE WHEN TRANSLATION IS DONE")+nl+ucstring("DIFF NOT TRANSLATED")+nl;

		string fn(CFile::getFilenameWithoutExtension(referenceFilename)), ext(CFile::getExtension(referenceFilename));
		std::string diffName(diffDir+fn+"_diff_"+diffVersion+"."+ext);
		CI18N::writeTextFile(diffName, str, false);
		
	}

	return 0;
}

int mergeWorksheetDiff(int argc, char *argv[], const std::string &filename, const string &additionFile)
{
/*	if (argc != 3)
	{
		LOG("ERROR : mergeWorksheetDiff need a worksheet file in parameter !");
		return 1;
	}
	std::string filename = argv[2];
*/
	LOG("Merging for file '%s'...\n", filename.c_str());
//	string filename = transDir+types[t]+"_words_"+Languages[l]+".txt";
	// load the translated file
	TWorksheet translated;
	if (!CFile::fileExists(transDir+filename) || !loadExcelSheet(transDir+filename, translated))
	{
		// there is no translated file yet, build one from the working file.
		ucstring str;
		string addfn = addDir+additionFile;
		CI18N::readTextFile(addfn, str, false, false, false, CI18N::LINE_FMT_CRLF);
		str = str.substr(0, str.find(nl)+2);
		CI18N::writeTextFile(transDir+filename, str, false);
		// reread the file.
		bool res = loadExcelSheet(transDir+filename, translated);
		nlassert(res);

	}
	makeHashCode(translated, false);

	// append the translated diffs
	mergeWorksheetDiff(filename, translated, true, true);
//	mergeSheetDiff(types[t], translated, Languages[l], true, true);

	// prepare the addition string
	ucstring str = prepareExcelSheet(translated);

	{
		// backup the original file
		ucstring old;
		CI18N::readTextFile(transDir+filename, old, false, true, false, CI18N::LINE_FMT_CRLF);
		if (old != str)
		{
			string fn(CFile::getFilenameWithoutExtension(filename)), ext(CFile::getExtension(filename));
			CFile::moveFile((historyDir+fn+"_"+diffVersion+"."+ext).c_str(), (transDir+filename).c_str());
		}
	}

	if (translated.size() > 0)
		CI18N::writeTextFile(transDir+filename, str, false);

	return 0;
}


int makeWordsDiff(int argc, char *argv[])
{
	vector<string> fileList;
	CPath::getPathContent(addDir, false, false, true, fileList);

	// filter in words file only
	uint i;
	for (i=0; i<fileList.size(); ++i)
	{
		if (fileList[i].find("_words_"+Languages[0]+".txt") == string::npos || fileList[i].find(".#") != string::npos )
		{
			fileList.erase(fileList.begin()+i);
			--i;
		}
	}

	int ret = 0;

	// for each word file
	for (uint i=0; i<fileList.size(); ++i)
	{
		string	type;
		type = CFile::getFilename(fileList[i]);
		type = type.substr(0, type.find("_") );

		for (uint l=0; l<Languages.size(); ++l)
		{
			LOG("Diffing for language %s, type %s...\n", Languages[l].c_str(), type.c_str());

			if (l == 0)
				ret += makeWorksheetDiff(argc, argv, CFile::getFilename(fileList[i]), CFile::getFilename(fileList[i]), true);
			else
				ret += makeWorksheetDiff(argc, argv, CFile::getFilename(fileList[i]), type+"_words_"+Languages[l]+".txt", false);
		}
	}

	return ret;

}

int mergeWordsDiff(int argc, char *argv[])
{
	LOG("Merging words diffs\n");

	int ret = 0;

	vector<string> fileList;
	CPath::getPathContent(addDir, false, false, true, fileList);

	// filter in words file only
	for (uint i=0; i<fileList.size(); ++i)
	{
		if (fileList[i].find("_words_"+Languages[0]+".txt") == string::npos)
		{
			fileList.erase(fileList.begin()+i);
			--i;
		}
	}
	
	// for each language
	for (uint l=0; l<Languages.size(); ++l)
	{
		// for each file
		for (uint i=0; i<fileList.size(); ++i)
		{
			string	type;
			type = CFile::getFilename(fileList[i]);
			type = type.substr(0, type.find("_") );

			ret += mergeWorksheetDiff(argc, argv, type+"_words_"+Languages[l]+".txt", CFile::getFilename(fileList[i]));
		}
	}

	return ret;

}



/// temporary code
struct TMissionInfo
{
	ucstring	Info;
	string	Title;
	string	Detail;
	string	EndDetail;
	string	Step0Desc;
	string	Step0Prog;
	string	Step0ProgDesc;
};

struct TCompCondNum
{
	bool operator() (const TClause &c1, const TClause &c2)
	{
		return count(c1.Conditions.begin(), c1.Conditions.end(), '&') > count(c2.Conditions.begin(), c2.Conditions.end(), '&');
	}
};

int recupAround(int argc, char *argv[])
{
	string clause1(diffDir+"clause_en_diff_3E896220.txt");
	string clause2(addDir+"clause_en_diff_3E7B4CE4 TRANSLATED.txt");

	vector<TStringInfo> reference;
	loadStringFile(clause1, reference, true);
	vector<TStringInfo> around;
	loadStringFile(clause2, around, true, '[', ']', true);

	vector<TStringInfo> result;

	nlassert(reference.size() == around.size());

	for (uint i=0; i<reference.size(); ++i)
	{
		TStringInfo	si = reference[i];
		si.Text = around[i].Text2;
		si.Comments = around[i].Comments;

		result.push_back(si);
	}

	ucstring str = prepareStringFile(result, false);

	CI18N::writeTextFile(addDir+"test_clause.txt", str);

	return 0;
}

//int mergeYannTaf();
int addStringNumber();


void cropLines(const std::string &filename, uint32 nbLines)
{
	ucstring utext;

	LOG("Cropping %u lines from file '%s'\n", nbLines, filename.c_str());

	CI18N::readTextFile(filename, utext, false, false, false, CI18N::LINE_FMT_CRLF);

	string text = utext.toUtf8();

	vector<string>	lines;
	explode(text, std::string("\n"), lines);

	text.clear();
	if (lines.size() > nbLines)
	{
		for (uint i=0; i<lines.size()-nbLines; ++i)
			text += lines[i] + "\n";
	}

	utext.fromUtf8(text);

	CI18N::writeTextFile(filename, utext, true);
}



int	makeWork()
{
	vector<string>	files;
	uint			i;

	// move en.uxt file to wk.uxt
	CFile::moveFile((CPath::standardizePath(addDir)+"wk.uxt").c_str(), (CPath::standardizePath(addDir)+"en.uxt").c_str());

	files.clear();
	CPath::getPathContent(addDir, true, false, true, files);

	string	strreplaced("_en.txt");
	string	strtoreplace("_wk.txt");

	for (i=0; i<files.size(); ++i)
	{
		if (testWildCard(CFile::getFilename(files[i]).c_str(), "*_en.txt"))
		{
			std::string	filename = files[i];
			nlinfo("checking file '%s'", filename.c_str());

			// change #include "*_en.txt" into #include "*_wk.txt"
			ucstring	utext;

			CI18N::readTextFile(filename, utext, false, false, false, CI18N::LINE_FMT_CRLF);
			string text = utext.toUtf8();

			bool	changedFile = false;

			string::size_type	p = 0;
			while ( (p=text.find("#include", p)) != string::npos)
			{
				string::size_type	start = p, end;
				while (start < text.size() && text[start++] != '"')
					;
				end = start;
				while (end < text.size() && text[end] != '"')
					++end;

				string	includefilename = text.substr(start, end-start);

				if (testWildCard(includefilename.c_str(), "*_en.txt"))
				{
					string	originalfilename = includefilename;
					includefilename.replace(includefilename.size()-strreplaced.size(), strreplaced.size(), strtoreplace);
					text.replace(start, end-start, includefilename);

					nlinfo("replaced '#include \"%s\"' into '#include \"%s\"'", originalfilename.c_str(), includefilename.c_str());
					changedFile = true;
				}

				p = end;
			}

			if (changedFile)
			{
				utext.fromUtf8(text);
				CI18N::writeTextFile(filename, utext, true);
			}

			// change filename
			std::string	movetofilename = filename;
			movetofilename.replace(movetofilename .size()-strreplaced.size(), strreplaced.size(), strtoreplace);

			if (CFile::moveFile(movetofilename.c_str(), filename.c_str()))
			{
				nlinfo("moved file '%s' to '%s'", filename.c_str(), movetofilename.c_str());
			}
			else
			{
				nlwarning("FAILED to move file '%s' to '%s'", filename.c_str(), movetofilename.c_str());
			}
		}
	}

	// move en.uxt file to wk.uxt
	CFile::moveFile((CPath::standardizePath(transDir)+"wk.uxt").c_str(), (CPath::standardizePath(transDir)+"en.uxt").c_str());

	files.clear();
	CPath::getPathContent(transDir, true, false, true, files);

	for (i=0; i<files.size(); ++i)
	{
		if (testWildCard(CFile::getFilename(files[i]).c_str(), "*_en.txt"))
		{
			std::string	filename = files[i];
			nlinfo("checking file '%s'", filename.c_str());

			// change filename
			std::string	movetofilename = filename;
			movetofilename.replace(movetofilename .size()-strreplaced.size(), strreplaced.size(), strtoreplace);

			nlinfo("moved file '%s' to '%s'", filename.c_str(), movetofilename.c_str());

			CFile::moveFile(movetofilename.c_str(), filename.c_str());
		}
	}

	return 0;
}


void preprocessTextFile(const std::string &filename,
						std::vector< std::pair<ucstring, std::string> > & outputResult);

void assertUniq(const vector<TPhrase>& reference)
{

	std::set< std::string > phraseIdentifier;
	std::set< std::string > clauseIdentifier;
	vector<TPhrase>::const_iterator first( reference.begin() );
	vector<TPhrase>::const_iterator last(  reference.end() );
	for( ; first != last; ++first)
	{
		if ( phraseIdentifier.find(first->Identifier) != phraseIdentifier.end())
		{
			nlwarning("Phrase %s defined more than once.", first->Identifier.c_str());
			exit(-1);
		}
		else
		{
			phraseIdentifier.insert(first->Identifier);
			vector<TClause>::const_iterator first2( first->Clauses.begin() );
			vector<TClause>::const_iterator last2(  first->Clauses.end() );
			for( ; first2 != last2; ++first2)
			{
				if (clauseIdentifier.find(first2->Identifier) != clauseIdentifier.end() )
				{
					nlwarning("Clause %s defined more than once.", first2->Identifier.c_str());
					exit(-1);
				}
			}
		}
	}
	
}


void mergePhraseDiff2Impl(vector<TPhrase>& reference, const vector<TPhrase>& addition)
{
	assertUniq(reference);
	assertUniq(addition);

	typedef std::map<std::string, TPhrase> TMap;

	TMap phrases;

	{
		vector<TPhrase>::const_iterator first( reference.begin() );
		vector<TPhrase>::const_iterator last(  reference.end() );
		for( ; first != last ; ++first )
		{
			std::string identifier = first->Identifier;
			phrases[identifier] = *first;
		}
	}

	{
		vector<TPhrase>::const_iterator first( addition.begin() );
		vector<TPhrase>::const_iterator last(  addition.end() );
		for( ; first != last ; ++first )
		{
			if ( first->Comments.find(ucstring("DIFF CHANGED")) != ucstring::npos)
			{
				nlassert( phrases.find(first->Identifier) != phrases.end() );
				phrases[first->Identifier] = *first;
			}			
			else if ( first->Comments.find(ucstring("DIFF ADD")) != ucstring::npos)
			{
				nlassert( phrases.find(first->Identifier) == phrases.end() );
				phrases[first->Identifier] = *first;

			}
			else if ( first->Comments.find(ucstring("DIFF REMOVED")) != ucstring::npos)
			{
				nlassert( phrases.find(first->Identifier) != phrases.end() );
				phrases.erase( phrases.find(first->Identifier));
			}
			else
			{
			//	nlassert(0 && "INVALID DIFF COMMAND");
			}	
		}
	}

	{
		reference.clear();
		reference.reserve(phrases.size());		
		TMap::const_iterator first( phrases.begin() );
		TMap::const_iterator last(  phrases.end() );
		for( ; first != last; ++first) {	reference.push_back(first->second); }
	}

}

void removeHashValueComment(ucstring & comments)
{
	ucstring::size_type first;
	ucstring::size_type last;
	first = comments.rfind(ucstring("// HASH_VALUE"));
	if (first != ucstring::npos)
	{
		last = comments.find(ucstring("\n"), first);
		if (last != ucstring::npos) 
		{ 
			last += 1;
			ucstring tmp1 = comments.substr(0, first);
			ucstring tmp2 =	last !=comments.size() 
					? comments.substr(last)
					: ucstring("");
			comments = tmp1 + tmp2;
		}
		else
		{
			comments = comments.substr(0, first);
		}
	}
	else
	{
		//comments = comments;
	}

}

bool updateClauseHashValue(const std::map<std::string, std::pair<uint64, uint64> >& validValues, const std::string & dirPath = "")
{
	
	for (uint l=0; l<Languages.size() ; ++l)
	{
				
		std::string basename("clause_"+Languages[l]);
		vector<TStringInfo>	clauses;
		std::string refFile(basename+".txt");
		if (!loadStringFile(transDir+refFile, clauses, false))
		{
			LOG("Error will loading file %s", (transDir+refFile).c_str());
			return false;
		}

		bool changed = false;
		for ( uint i=0; i < clauses.size() ; ++i)
		{
			std::string Identifier = clauses[i].Identifier;
			if ( validValues.find(Identifier) != validValues.end())
			{
				if (!validValues.find(Identifier)->second.second
					|| clauses[i].HashValue == validValues.find(Identifier)->second.second)
				{
					clauses[i].HashValue = validValues.find(Identifier)->second.first;
					removeHashValueComment(clauses[i].Comments);
					changed = true;
				}				
			}
		}

		if (!changed)
		{
			nlwarning("Clauses file don't need update for language %s\n", Languages[l].c_str());
		}
		else
		{
			nlinfo("Updating hashcode of clause file for %s.\n", Languages[l].c_str());
			// build the diff file for each language.
			ucstring str = prepareStringFile(clauses, false);
			std::string clauseName(dirPath+ transDir + basename +".txt");						
			CFile::createDirectoryTree( CFile::getPath(clauseName) );
			CI18N::writeTextFile(clauseName, str);
		}

	}
	return true;
}

ucstring preparePhraseFile2(const vector<TPhrase> &phrases, bool removeDiffComments)
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
			explode(comment, std::string("\n"), lines, true);

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
			/*if (p.Comments.find(ucstring("// HASH_VALUE ")) == ucstring::npos)
			{
				// add the hash value.
				ret += ucstring("// HASH_VALUE ")+CI18N::hashToString(p.HashValue) + nl;
			}*/
			ret += p.Identifier + "("+p.Parameters + ")" + nl;
			ret += '{';
			ret += nl;
			for (uint i=0; i<p.Clauses.size(); ++i)
			{
				const TClause &c = p.Clauses[i];
				if (!c.Comments.empty())
				{
					ucstring comment = tabLines(1, c.Comments);
					if (comment[comment.size()-1] == ucchar(' ')) comment=comment.substr(0, comment.size() - 1);
					ret += comment; // + '\r'+'\n';
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
				ret += '\t' + (c.Identifier.empty()? "" : c.Identifier + ' ' )+ text + nl + nl;
			}
			ret += '}';
		}
		ret += nl + nl;
	}

	return ret;
}

bool updatePhraseHashValue(const std::map<std::string, std::pair<uint64, uint64> > & validValues, const std::string & dirPath = "")
{
	
	for (uint l=0; l<Languages.size() ; ++l)
	{
				
		std::string basename("phrase_"+Languages[l]);
		vector<TPhrase>	phrases;
		std::string refFile(basename+".txt");
		if (!readPhraseFile(transDir+refFile, phrases, false))
		{
			LOG("Error will loading file %s", (transDir+refFile).c_str());
			return false;
		}

		bool changed = false;
		for ( uint i=0; i < phrases.size() ; ++i)
		{
			std::string Identifier = phrases[i].Identifier;
			if ( validValues.find(Identifier) != validValues.end())
			{
				if (!validValues.find(Identifier)->second.second || phrases[i].HashValue == validValues.find(Identifier)->second.second )
				{
				
					phrases[i].HashValue = validValues.find(Identifier)->second.first;
					removeHashValueComment(phrases[i].Comments);
					changed = true;
				}
			}
		}

		if (!changed)
		{
			nlinfo("Phrase file don't need update for language %s\n", Languages[l].c_str());
		}
		else
		{
			nlinfo("Updating hashcode of phrase file for %s.\n", Languages[l].c_str());
			// build the diff file for each language.
			ucstring str = preparePhraseFile(phrases, false);
			std::string pharseName(dirPath+ transDir + basename +".txt");						
			CFile::createDirectoryTree( CFile::getPath(pharseName) );
			CI18N::writeTextFile(pharseName, str);
		}

	}
	return true;
}


bool sortTransPhrase()
{
	
	for (uint l=0; l<Languages.size() ; ++l)
	{
				
		std::string basename("phrase_"+Languages[l]);
		vector<TPhrase>	phrases;
		vector<TPhrase>	phrases2;
		std::map<std::string, TPhrase> phraseMap;
		std::string refFile(basename+".txt");
		if (!readPhraseFile(transDir+refFile, phrases, false))
		{
			LOG("Error will loading file %s", (transDir+refFile).c_str());
			return false;
		}
		
		{		
		
			std::vector<TPhrase>::const_iterator first(phrases.begin());
			std::vector<TPhrase>::const_iterator last(phrases.end());
			for ( ; first != last; ++first)
			{
				phraseMap[first->Identifier] = *first;
			}
		}
		{
			std::map<std::string, TPhrase>::const_iterator first(phraseMap.begin());
			std::map<std::string, TPhrase>::const_iterator last(phraseMap.end());
			for ( ; first != last; ++first)
			{
				phrases2.push_back( first->second);
			}
		}


		nlinfo("Updating hashcode of phrase file for %s.\n", Languages[l].c_str());
		// build the diff file for each language.
		ucstring str = preparePhraseFile(phrases2, false);
		std::string pharseName(transDir+refFile);						
		CFile::createDirectoryTree( CFile::getPath(pharseName) );
		CI18N::writeTextFile(pharseName, str);
	}
	return true;
}



void patchWorkFile(vector<TPhrase> &updatedPhrase, const std::string & filename)
{
	ucstring text;
	if ( updatedPhrase.empty() ) { return; }
	CI18N::readTextFile(filename, text, false, false, false, CI18N::LINE_FMT_CRLF);
	vector<TPhrase>::const_iterator first(updatedPhrase.begin());
	vector<TPhrase>::const_iterator last(updatedPhrase.end());
	for (; first != last; ++first)
	{
		
		ucstring::size_type firstFun = text.find( ucstring(first->Identifier));
		if (firstFun == ucstring::npos)
		{
			nlwarning("Error can't patch %s: %s not found", filename.c_str(), first->Identifier.c_str());
		}
		else
		{
			ucstring::size_type lastFun = text.find( ucstring("}") , firstFun);
			if (lastFun == ucstring::npos)
			{
				nlwarning("Error can't patch %s: syntax error near %s", filename.c_str(), first->Identifier.c_str());
			}
			else
			{
				std::vector<TPhrase> param;
				param.push_back(*first);
		
				ucstring before = text.substr(0,firstFun);
				ucstring str = preparePhraseFile2(param, false);
				ucstring after = text.substr(lastFun+1);				
				text = "";								
				text += before;
				text += str;
				text += after;
			}			
		}
	}
	CI18N::writeTextFile( filename, text);
			
}

int updatePhraseWork()
{
	std::string saveDir = diffDir + "update_"+ diffVersion + "/";
	vector<TPhrase>	transPhrase;
	std::map<std::string, TPhrase> transPhraseMap;	
	std::map<std::string, std::pair<uint64,uint64> > validClauseHashValue;
	std::map<std::string, std::pair<uint64, uint64> > validPhraseHashValue;
	std::vector< std::pair<ucstring, std::string> > outputResult;

	if (!readPhraseFile(transDir+"phrase_wk.txt", transPhrase, false))
	{
			LOG("Error will loading file %s", (addDir+"phrase_"+Languages[0]+".txt").c_str());
			return 1;
	}

	{
		std::vector<TPhrase>::const_iterator first(transPhrase.begin());
		std::vector<TPhrase>::const_iterator last(transPhrase.end());
		for (; first != last;++first)
		{
			transPhraseMap[first->Identifier] = *first;
		}
	}
	
	preprocessTextFile(addDir+"phrase_wk.txt", outputResult);

	uint firstFile = 0;
	uint lastFile = (uint)outputResult.size();
	for (; firstFile != lastFile ; ++firstFile)
	{

		ucstring doc = outputResult[firstFile].first;
		std::vector<TPhrase> phrases;		
		readPhraseFileFromString(outputResult[firstFile].first,  outputResult[firstFile].second, phrases, true);

		std::vector<TPhrase>::iterator first(phrases.begin());
		std::vector<TPhrase>::iterator last(phrases.end());
		std::vector<TPhrase> updatedPhrases;
		for (; first != last; ++first)
		{
			if (transPhraseMap.find(first->Identifier) !=  transPhraseMap.end() )
			{
				TPhrase workPhrase = *first;
				TPhrase& transPhrase = transPhraseMap[first->Identifier];
				if (first->HashValue == transPhrase.HashValue)
				{
					uint64 oldHash = transPhrase.HashValue;
					uint64 newHash = STRING_MANAGER::makePhraseHash(transPhrase);
					if (newHash != transPhrase.HashValue)
					{
						//translation phrase_wk.txt has been manually changed
						validPhraseHashValue[transPhrase.Identifier] = std::pair<uint64, uint64>(newHash, oldHash);
						std::vector<TClause>::iterator firstClause ( transPhrase.Clauses.begin() );
						std::vector<TClause>::iterator lastClause  ( transPhrase.Clauses.end() );
						for (; firstClause != lastClause; ++firstClause)
						{
							uint64 clauseHashValue = CI18N::makeHash(firstClause->Text);
								
							validClauseHashValue[firstClause->Identifier] = std::pair<uint64, uint64>(clauseHashValue, firstClause->HashValue);
							firstClause->HashValue = clauseHashValue;
						}
						updatedPhrases.push_back(transPhrase);
						updatedPhrases.back().Comments= ucstring("");
					}						
				}
			}
		}

		
		std::string newFile = saveDir + outputResult[firstFile].second;
		std::string oldFile = outputResult[firstFile].second;
		CFile::createDirectoryTree(CFile::getPath(newFile));
		if (  CFile::copyFile(newFile, oldFile) )
		{
			
			patchWorkFile(updatedPhrases,  newFile);
		}
		else
		{
			nlwarning("Can't copy %s", newFile.c_str());
		}
		
	}	

	updatePhraseHashValue(validPhraseHashValue, saveDir);
	updateClauseHashValue(validClauseHashValue,  saveDir);
	return 0;
}




bool mergePhraseDiff2(vector<TPhrase> &phrases, const string &language, bool onlyTranslated, bool archiveDiff = false)
{
	vector<string>	diffs;

	getPathContentFiltered(diffDir+"phrase_"+language+"_diff_", ".txt", diffs);

	for (uint i=0; i<diffs.size(); ++i)
	{		
		if (onlyTranslated)
		{
			// Check if the diff is translated
			ucstring text;
			CI18N::readTextFile(diffs[i], text, false, false, false, CI18N::LINE_FMT_CRLF);
			verifyVersion(text, 2);
			if (text.find(ucstring("DIFF NOT TRANSLATED")) != ucstring::npos)
			{
				LOG("Diff file [%s] is not translated, merging it later.\n", CFile::getFilename(diffs[i]).c_str());
				for (i=i+1; i<diffs.size(); ++i)
					LOG("  Merge of Diff file [%s] delayed.\n", CFile::getFilename(diffs[i]).c_str());
				return true;
			}
		}

	

		// we found a diff file for the addition file.
		LOG("Adding %s diff as reference\n", diffs[i].c_str());
		vector<TPhrase>	diff;
		if (!readPhraseFile2(diffs[i], diff, false))
			return false;

	
		mergePhraseDiff2Impl(phrases, diff);
	

		if (archiveDiff)
		{
			// move the diff file in the history dir
			CFile::moveFile((historyDir+CFile::getFilename(diffs[i])).c_str(), diffs[i].c_str());
		}
	}

	return true;
}



class CMakePhraseDiff2
{
public:

	class CPhraseEqual
	{

	public:
		CPhraseEqual(){}

		bool operator()( const TPhrase& left, const TPhrase& right) const;

//		bool clausesEqual( const std::vector<TClause>& left, const std::vector<TClause>& right) const;

//		bool clauseEqual(const TClause& left, const TClause& right) const;

	};
	
	void run(const vector<TPhrase> &addition, vector<TPhrase> &reference, vector<TPhrase> &diff);

	void onEquivalent(uint addIndex, uint refIndex, TPhraseDiffContext &context);

	void onAdd(uint addIndex, uint refIndex, TPhraseDiffContext &context);
	
	void onRemove(uint addIndex, uint refIndex, TPhraseDiffContext &context);

	void onChanged(uint addIndex, uint refIndex, TPhraseDiffContext &context);


};






void CMakePhraseDiff2::run(const vector<TPhrase> &addition, vector<TPhrase> &reference, vector<TPhrase> &diff)
{
	
	TPhraseDiffContext context(addition, reference, diff);

	std::set<std::string> phraseIdentifier;
	std::map<std::string, uint> mapAdd;
	std::map<std::string, uint> mapRef;		

	{
		uint first = 0;
		uint last = (uint)reference.size();

		for ( ;first != last; ++first)
		{
			std::string Identifier(reference[first].Identifier);
			mapRef[Identifier] = first;
			phraseIdentifier.insert(Identifier);
		}
	}

	{
		uint first = 0;
		uint last = (uint)addition.size();

		for ( ;first != last; ++first)
		{
			std::string Identifier(addition[first].Identifier);
			mapAdd[Identifier] = first;
			phraseIdentifier.insert(Identifier);
		}
	}

	if (mapAdd.size() != addition.size())
	{
		nlwarning("Phrases are defined more than once in works directory");
	}
	
	if (mapAdd.size() != addition.size())
	{
		nlwarning("Phrases are defined more than once in translation directory");
	}
	
	
	std::set<std::string>::iterator first(phraseIdentifier.begin());
	std::set<std::string>::iterator last(phraseIdentifier.end());

	for (; first != last; ++first)
	{
		if ( mapAdd.find(*first) != mapAdd.end() 
			&& mapRef.find(*first) != mapRef.end())
		{

			if ( CPhraseEqual()(addition[mapAdd[*first]], reference[mapRef[*first]]) )
			{
				onEquivalent(mapAdd[*first], mapRef[*first], context);
			}
			else
			{
				onChanged(mapAdd[*first], mapRef[*first], context);
			}
		}
		else if ( mapAdd.find(*first) != mapAdd.end() 
			&& mapRef.find(*first) == mapRef.end())
		{
			onAdd(mapAdd[*first], 0, context);
		}
		else if ( mapAdd.find(*first) == mapAdd.end() 
			&& mapRef.find(*first) != mapRef.end())
		{
			onRemove(0, mapRef[*first], context);
		}
		
	}

}


void CMakePhraseDiff2::onEquivalent(uint addIndex, uint refIndex, TPhraseDiffContext &context)
{
	// nothing to do
}
void CMakePhraseDiff2::onAdd(uint addIndex, uint refIndex, TPhraseDiffContext &context)
{
	TPhrase phrase = context.Addition[addIndex];
	char temp[1024];
	sprintf(temp, "// DIFF ADD");
	phrase.Comments = ucstring(temp) + nl + phrase.Comments;

	nlinfo("Added %s at %u", phrase.Identifier.c_str(), addIndex);
	context.Diff.push_back(phrase);
}

void CMakePhraseDiff2::onRemove(uint addIndex, uint refIndex, TPhraseDiffContext &context)
{
	TPhrase phrase = context.Reference[refIndex];
	char temp[1024];
	sprintf(temp, "// DIFF REMOVED");
	// NB : on vire les commentaires car il pourrai contenir des merdes..
	phrase.Comments = ucstring(temp) + nl;
	for (uint i=0; i<phrase.Clauses.size(); ++i)
		phrase.Clauses[i].Comments.erase();

	nlinfo("Removed %s at %u", phrase.Identifier.c_str(), addIndex);
	context.Diff.push_back(phrase);
}

void CMakePhraseDiff2::onChanged(uint addIndex, uint refIndex, TPhraseDiffContext &context)
{
	ucstring chg;
	char temp[1024];
	// check what is changed.
	if (context.Addition[addIndex].Parameters != context.Reference[refIndex].Parameters)
		chg += "// Parameter list changed." + nl;
	if (context.Addition[addIndex].Clauses.size() != context.Reference[refIndex].Clauses.size())
		chg += "// Clause list changed." + nl;
	else
	{
		for (uint i=0; i<context.Addition[addIndex].Clauses.size(); ++i)
		{
			if (context.Addition[addIndex].Clauses[i].Identifier != context.Reference[refIndex].Clauses[i].Identifier)
				chg += ucstring("// Clause ")+itoa(i, temp, 10) + " : identifier changed." + nl;
			else if (context.Addition[addIndex].Clauses[i].Conditions != context.Reference[refIndex].Clauses[i].Conditions)
				chg += ucstring("// Clause ")+itoa(i, temp, 10) + " : condition changed." + nl;	
			else if (context.Addition[addIndex].Clauses[i].Text != context.Reference[refIndex].Clauses[i].Text)
				chg += ucstring("// Clause ")+itoa(i, temp, 10) + " : text changed." + nl;	
		}
	}

	if (chg.empty())
	{
		chg = ucstring("// WARNING : Hash code changed ! check translation workflow.") + nl;
	}
	nldebug("Changed detected : %s", chg.toString().c_str());
	
	// changed element
	TPhrase phrase = context.Addition[addIndex];
//				char temp[1024];
	sprintf(temp, "// DIFF CHANGED");
	vector<TPhrase>	tempV;
	tempV.push_back(context.Reference[refIndex]);
	ucstring tempT = preparePhraseFile(tempV, false); 
	CI18N::removeCComment(tempT);
	phrase.Comments = ucstring(temp) + nl + phrase.Comments;
	phrase.Comments = phrase.Comments + ucstring("/* OLD VALUE : ["+nl) + tabLines(1, tempT) +nl + "] */" + nl;
	phrase.Comments = phrase.Comments + chg;

	nlinfo("Changed %s at %u", phrase.Identifier.c_str(), addIndex);
	context.Diff.push_back(phrase);
}


bool CMakePhraseDiff2::CPhraseEqual::operator()( const TPhrase& left, const TPhrase& right) const
{
	bool identifierOk = left.Identifier == right.Identifier;
//	bool parameterOk = left.Parameters == right.Parameters;
//	bool commentsOk = left.Comments == right.Comments;
//	bool clausesOk = clausesEqual(left.Clauses, right.Clauses);
	bool hashOk = left.HashValue== right.HashValue;

	return identifierOk	&& hashOk;// && parameterOk && clausesOk;	
	
}
/*
bool CMakePhraseDiff2::CPhraseEqual::clausesEqual( const std::vector<TClause>& left, const std::vector<TClause>& right) const
{
	std::vector<TClause>::const_iterator first1(left.begin());
	std::vector<TClause>::const_iterator last1(left.end());
	std::vector<TClause>::const_iterator first2(right.begin());

	if (left.size() != right.size()) return false;
	
	for ( ; first1 != last1 && !clauseEqual(*first1, *first2); ++first1, ++first2){}
	
	return first1 == last1;
}

bool CMakePhraseDiff2::CPhraseEqual::clauseEqual(const TClause& left, const TClause& right) const
{
	return left.Identifier != right.Identifier
	&& left.Conditions != right.Conditions
	&& left.Text != right.Text
	&& left.Comments != right.Comments
	&& left.HashValue != right.HashValue;	
}

*/



int makePhraseDiff2(int argc, char *argv[])
{
	// Generate the diff file from phrase_<lang>.txt compared to the same file in translated.
	// The diff is generated only from the reference language for and all the languages
	
	LOG("Generating phrase diffs\nLoading the working file for language %s\n", Languages[0].c_str());


	vector<TPhrase>	addition;

	// read	addition
	if (!readPhraseFile(addDir+"phrase_"+Languages[0]+".txt", addition, true))
	{
		LOG("Error will loading file %s", (addDir+"phrase_"+Languages[0]+".txt").c_str());
		return 1;
	}

	for (uint l =0; l<Languages.size(); ++l)
	{
		LOG("Diffing with language %s...\n", Languages[l].c_str());
		if (l == 1)
		{
			addition.clear();
			// read the language 0 translated version as addition for other language
			if (!readPhraseFile(transDir+"phrase_"+Languages[0]+".txt", addition, true))
			{
				LOG("Error will loading file %s", (addDir+"phrase_"+Languages[0]+".txt").c_str());
				return 1;
			}
		}
		vector<TPhrase>	reference;
		// read the reference file
		if (!readPhraseFile(transDir+"phrase_"+Languages[l]+".txt", reference, false))
		{
			LOG("Error will loading file %s", (transDir+"phrase_"+Languages[l]+".txt").c_str());
			return 1;
		}

		if (!mergePhraseDiff2(reference, Languages[l], false))
		{
			LOG("Error will merging phrase diff for language %s\n", Languages[l].c_str());
			return 1;
		}

		// compare the reference an addition file, remove any equivalent strings.
		uint addCount=0, refCount=0;
		vector<TPhrase>	diff;

		CMakePhraseDiff2	differ;
		differ.run(addition, reference, diff);

		if (diff.empty())
		{
			LOG("No difference for language %s\n", Languages[l].c_str());
		}
		else
		{
			LOG("Writing difference file for language %s\n", Languages[l].c_str());
			ucstring text;
			text += "// DIFF_VERSION 2\r\n";
			text += preparePhraseFile(diff, false);
			// add the tag for non translation
			text += nl + ucstring ("// REMOVE THE FOLOWING LINE WHEN TRANSLATION IS DONE")+nl+ucstring("// DIFF NOT TRANSLATED")+nl;
			CI18N::writeTextFile(diffDir+"phrase_"+Languages[l]+"_diff_"+diffVersion+".txt", text);
		}
	}

	return 0;
}



int forgetPhraseDiff(int argc, char *argv[])
{
	// merge all the phrase diff back into there repective translated phrase.

	LOG("forgeting phrase diffs\n");

	std::string basename("phrase_"+Languages[0]);
	string filename = transDir+basename+".txt";
	// build the addition diff
	vector<TPhrase>	reference;


	if (!readPhraseFile(transDir+basename+".txt", reference, false))
	{
		LOG("Error will loading file %s", (transDir+basename+".txt").c_str());
		return 1;
	}
	//assert only change

	std::vector<std::string> diffs;
	getPathContentFiltered(diffDir+"phrase_wk_diff_", ".txt", diffs);
	std::vector<TPhrase> newPhrase;
	for (uint i=0; i<diffs.size(); ++i)
	{
		// we found a diff file for the addition file.
		LOG("Adding %s diff as reference\n", diffs[i].c_str());
		vector<TPhrase>	subDiff;
		if (!readPhraseFile2(diffs[i], subDiff, false))
			return false;
		std::copy (subDiff.begin (), subDiff.end (), std::back_inserter (newPhrase));
	}

	// a optimiser par une map	
	std::map<std::string, std::pair<uint64, uint64> > validClauseHashValue;
	std::map<std::string, std::pair<uint64, uint64> > validPhraseHashValue;
	for (uint i=0; i < newPhrase.size() ; ++i)
	{
		for (uint j=0; j < reference.size() ; ++j)
		{
			if (newPhrase[i].Identifier == reference[j].Identifier)
			{		
				

				uint64 newPhraseHash = STRING_MANAGER::makePhraseHash( newPhrase[i] );
				uint64 oldPhraseHash = reference[j].HashValue;
				validPhraseHashValue[newPhrase[i].Identifier] = std::pair<uint64, uint64>(newPhraseHash, oldPhraseHash);
				
				for (uint k=0; k < newPhrase[i].Clauses.size() ; ++k)
				{
					if (reference[j] .Clauses.size() != newPhrase[i].Clauses.size())
					{						
						nlwarning("Want to forget minor update but phrase %s changes too much. The number of clauses has changed.", newPhrase[i].Identifier.c_str() );
						exit(-1);
					}
					const TClause& newClause = newPhrase[i].Clauses[k];
					const TClause& oldClause = reference[j].Clauses[k];

					if (!newClause.Identifier.empty() )
					{
						if (newClause.Identifier != oldClause.Identifier)
						{						
							nlwarning("Want to forget minor update but phrase %s changes too much. Clauses order or clause identifier changed (%s).", newPhrase[i].Identifier.c_str(), newClause.Identifier.c_str());
							exit(-1);
						}
						uint64 newClauseHashValue = CI18N::makeHash(newClause.Text);
						uint64 oldClauseHashValue = CI18N::makeHash(oldClause.Text);
						validClauseHashValue[ newClause.Identifier ] = std::pair<uint64, uint64>(newClauseHashValue, oldClauseHashValue);
					}
				}
			}
		}
	}
			

	if (!mergePhraseDiff2(reference, Languages[0], true, false))
	{
		LOG("Error will merging phrase diff");
		return 1;
	}
	ucstring str = preparePhraseFile(reference, true);
	CI18N::writeTextFile(transDir+basename+".txt", str);

		
	updatePhraseHashValue(validPhraseHashValue);
//	updateClauseHashValue(validClauseHashValue);
	

	for (uint i=0; i<diffs.size(); ++i)
	{	
		std::string diffHistory = historyDir + CFile::getFilename(diffs[i]);
		CFile::moveFile(diffHistory.c_str(),  diffs[i].c_str());
	}
	return 0;
}


void preprocessTextFile(const std::string &filename,
						std::vector< std::pair<ucstring, std::string> > & outputResult						 
						)
						 
{
	//nlinfo("preprocessing %s",	filename.c_str());
	ucstring result;
	std::string fullName;
	fullName = filename;

	if (fullName.empty())
		return;

	NLMISC::CIFile	file;

	/// Open a file for reading. false if failed. close() if a file was opened.
	if (!file.open (fullName))
	{
		nlwarning("Can't open %s",	fullName.c_str());
		return ;
	}



	// Fast read all the text in binary mode.
	std::string text;
	text.resize(file.getFileSize());
	file.serialBuffer((uint8*)(&text[0]), (uint)text.size());


	// Transform the string in ucstring according to format header
	if (!text.empty())
		CI18N::readTextBuffer((uint8*)&text[0], (uint)text.size(), result, false);

	

	ucstring final;
	// parse the file, looking for preprocessor command.
	ucstring::size_type pos = 0;
	ucstring::size_type lastPos = 0;
	ucstring	includeCmd("#include");
	ucstring current;

	while (  pos != ucstring::npos)
	{
		pos = result.find(ucstring("\n"), pos);
		if (pos != ucstring::npos) { ++pos; }
		
		ucstring line( result.substr(lastPos, pos - lastPos) );
		

		if ( line.find(includeCmd) != ucstring::npos)
		{
			ucstring::size_type firstFilename = line.find(ucstring("\""));
			ucstring::size_type lastFilename = line.find(ucstring("\""), firstFilename+1);

			ucstring name = line.substr(firstFilename +1, lastFilename - firstFilename  -1);
			string subFilename = name.toString();			
			{
				CIFile testFile;
				if (!testFile.open(subFilename))
				{
				// try to open the include file relative to current file
					subFilename = CFile::getPath(filename)+subFilename;
				}
			}
			preprocessTextFile(subFilename, outputResult);
		}
		else
		{
			current += line;
		}
		lastPos = pos;
			
	}



	outputResult.push_back( std::pair<ucstring, std::string> ( current, fullName ) );
}

int mergePhraseDiff(int argc, char *argv[])
{
	// merge all the phrase diff back into there repective translated phrase.
	uint l;

	LOG("Merging phrase diffs\n");

	for (l=0; l<Languages.size(); ++l)
	{
		LOG("Merging for language %s...\n", Languages[l].c_str());
		std::string basename("phrase_"+Languages[l]);
		string filename = transDir+basename+".txt";
		// build the addition diff
		vector<TPhrase>	reference;

		if (!readPhraseFile(transDir+basename+".txt", reference, false))
		{
			LOG("Error will loading file %s", (transDir+basename+".txt").c_str());
			return 1;
		}

		if (!mergePhraseDiff(reference, Languages[l], true, true))
		{
			LOG("Error will merging phrase diff");
			return 1;

		}

		ucstring str = preparePhraseFile(reference, true);

		{
			// backup the original file
			ucstring old;
			CI18N::readTextFile(filename, old, false, true, false, CI18N::LINE_FMT_CRLF);
			if (old != str)
				CFile::moveFile((historyDir+CFile::getFilenameWithoutExtension(filename)+"_"+diffVersion+"."+CFile::getExtension(filename)).c_str(), filename.c_str());
		}

		CI18N::writeTextFile(transDir+basename+".txt", str);

	}

	return 0;
}


int injectClause()
{
	uint l;

	LOG("Update translation from clauses.\n");

	for (l=0; l<Languages.size(); ++l)
	{

		nlinfo("Update phrase %s", Languages[l].c_str());
		vector<TStringInfo>	clauses;
		vector<TPhrase>		phrases;

		// load the clause file			
		std::string clausePath( transDir+"clause_"+Languages[l]+".txt" );
		if (!loadStringFile(clausePath, clauses, false))
		{
			LOG("Error will loading file %s", clausePath.c_str());
			return 1;
		}

		// load the phrase file		
		std::string phrasePath( transDir+"phrase_"+Languages[l]+".txt" );
		if (!readPhraseFile(phrasePath, phrases, false))
		{
			LOG("Error will loading file %s", phrasePath.c_str());
			return 1;
		}

		vector<TPhrase>::iterator first(phrases.begin());
		vector<TPhrase>::iterator last(phrases.end());
		for ( ; first != last; ++first)
		{
			
			vector<TClause>::iterator firstClause( first->Clauses.begin());
			vector<TClause>::iterator lastClause( first->Clauses.end());
			for ( ; firstClause != lastClause; ++firstClause)
			{
				uint64 hashValue = CI18N::makeHash(firstClause->Text);
				vector<TStringInfo>::iterator firstRefClause(clauses.begin());
				vector<TStringInfo>::iterator lastRefClause(clauses.end());
				for ( ; firstRefClause != lastRefClause ; ++firstRefClause)
				{
					if (hashValue == firstRefClause->HashValue && firstClause->Text != firstRefClause->Text)
					{
						firstClause->Text = firstRefClause->Text;
						firstClause->HashValue = CI18N::makeHash(firstClause->Text);
						firstRefClause->HashValue = firstClause->HashValue;


						nlinfo("update clause %s from clause file %s.", firstClause->Identifier.c_str(), clausePath.c_str());
					}					
				}				
			}	
		}
		
		std::string desDir(diffDir + "inject_clause_" + diffVersion + "/");
		CFile::createDirectoryTree(desDir+ CFile::getPath(phrasePath));
		ucstring str = preparePhraseFile(phrases, true);
		CI18N::writeTextFile(desDir + phrasePath, str);

		str = prepareStringFile(clauses, true);
		CI18N::writeTextFile(desDir + clausePath, str);
	}


	return 0;
}



int main(int argc, char *argv[])
{
	NLMISC::CApplicationContext context;
/*	createDebug();
	CStdDisplayer *display = new CStdDisplayer;
	NLMISC::InfoLog->addDisplayer(display);
	NLMISC::WarningLog->addDisplayer(display);
	NLMISC::ErrorLog->addDisplayer(display);
*/

/*	for (uint i=0; i<20; ++i)
	{
		uint64 hash = makeHash(ucstring("Bonjour le monde !"));
		nldebug("%s", hashToString(hash).c_str());
		hash = makeHash(ucstring("Une autre clef"));
		nldebug("%s", hashToString(hash).c_str());
	}
*/

	if (argc < 2)
	{
		showUsage(argv[0]);
		return 1;
	}
	std::string argv1(argv[1]);
	
	// create the diff version.
	char temp[16];
	sprintf(temp, "%8.8X", (uint) ::time(NULL));
	diffVersion = temp;

	if (strcmp(argv[1], "make_work") == 0)
	{
		return makeWork();
	}

	// generic worksheet comparison
	if (strcmp(argv[1], "make_worksheet_diff") == 0)
	{
		if (argc != 3)
		{
			showUsage(argv[0]);
			return 1;
		}

		return makeWorksheetDiff(argc, argv, argv[2], argv[2], true);
	}
	else if (strcmp(argv[1], "merge_worksheet_diff") == 0)
	{
		if (argc != 3)
		{
			showUsage(argv[0]);
			return 1;
		}

		return mergeWorksheetDiff(argc, argv, argv[2], argv[2]);
	}
	else if (strcmp(argv[1], "crop_lines") == 0)
	{
		if (argc != 4)
		{
			showUsage(argv[0]);
			return 1;
		}

		uint nbLines;
		NLMISC::fromString(argv[3], nbLines);

		cropLines(argv[2], nbLines);

		return 0;
	}
	else if (strcmp(argv[1], "extract_bot_names") == 0)
		return extractBotNames(argc, argv);
	else if (strcmp(argv[1], "extract_new_sheet_names") == 0)
		return extractNewSheetNames(argc, argv);
	



	if (argc != 2)
	{
		showUsage(argv[0]);
		return 1;
	}

//	if (strcmp(argv[1], "yann") == 0)
//		return mergeYannTaf();


	string currentPath("./");
	CPath::addSearchPath(currentPath+addDir, true, false);
	CPath::addSearchPath(currentPath+diffDir, true, false);
//	CPath::addSearchPath(currentPath+transDir, true, false);
	if (readLanguages() != 0)
	{
		LOG("Error will loading language file (language.txt)");
		return 1;
	}

	if (strcmp(argv[1], "make_string_diff") == 0)
		return makeStringDiff(argc, argv);
	else if (strcmp(argv[1], "merge_string_diff") == 0)
		return mergeStringDiff(argc, argv);
	else if (strcmp(argv[1], "clean_string_diff") == 0)
		return cleanStringDiff(argc, argv);

	else if (argv1 == "make_phrase_diff_old") 
		return makePhraseDiff(argc, argv);
	else if (argv1 == "merge_phrase_diff_old")
		return mergePhraseDiff(argc, argv, 1);
	

	else if (argv1 == "make_phrase_diff") 
		return makePhraseDiff2(argc, argv);
	else if (argv1 == "merge_phrase_diff")
		return mergePhraseDiff(argc, argv, 2);
	else if (argv1 == "forget_phrase_diff")
		return forgetPhraseDiff(argc, argv);	
	else if (argv1 == "update_phrase_work")
		return updatePhraseWork();
	else if (argv1 == "clean_phrase_diff")
		return cleanPhraseDiff(argc, argv);	

	else if (argv1 == "inject_clause")
		return injectClause();
	
	else if (argv1 == "sort_trans_phrase")
		return sortTransPhrase();
	
	else if (strcmp(argv[1], "make_clause_diff") == 0)
		return makeClauseDiff(argc, argv);
	else if (strcmp(argv[1], "merge_clause_diff") == 0)
		return mergeClauseDiff(argc, argv);
	else if (argv1 == "clean_clause_diff")
		return cleanClauseDiff(argc, argv);	

	else if (strcmp(argv[1], "make_words_diff") == 0)
		return makeWordsDiff(argc, argv);
	else if (strcmp(argv[1], "merge_words_diff") == 0)
		return mergeWordsDiff(argc, argv);
	else if (strcmp(argv[1], "clean_words_diff") == 0)
		return cleanWordsDiff(argc, argv);

	else if (strcmp(argv[1], "recup_around") == 0)
		return recupAround(argc, argv);
	else if (strcmp(argv[1], "add_string_number") == 0)
		return addStringNumber();


	return -1;

}



int addStringNumber()
{
	vector<TStringInfo>	strings;

	LOG("Generating string diffs\nLoading the working file for language %s\n", Languages[0].c_str());
	// load the addition file
	std::string addFile(Languages[0]+".uxt");
	if (!loadStringFile(addDir+addFile, strings, true))
	{
		LOG("Error loading file %s\n", (addDir+addFile).c_str());
		return 1;
	}

	ucstring str = prepareStringFile(strings, false);

	string filename = addDir+Languages[0]+".uxt";
	CI18N::writeTextFile(filename, str);

	return 0;
}


