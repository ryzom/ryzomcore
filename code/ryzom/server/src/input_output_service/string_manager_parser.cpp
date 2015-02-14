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


#include "stdpch.h"
#include "nel/misc/diff_tool.h"
#include "string_manager.h"
#include "input_output_service.h"
#include "nel/misc/i18n.h"
#include "nel/misc/path.h"
#include "nel/misc/file.h"
#include "nel/misc/algo.h"
#include "nel/net/unified_network.h"
#include "nel/net/service.h"
#include "nel/misc/bit_mem_stream.h"
#include "game_share/ryzom_mirror_properties.h"
#include "nel/georges/u_form_elm.h"
#include "nel/georges/load_form.h"
#include <time.h>

using namespace STRING_MANAGER;
using namespace NLMISC;
using namespace NLNET;
using namespace std;

#define LOG if (!VerboseStringManager) {} else nlinfo
#define LOGPARSE if (!VerboseStringManagerParser) {} else nlinfo


extern NLMISC::CVariable<bool> VerboseStringManagerParser;
extern const ucstring		nl;

class CReadPhraseFile : public TPhraseDiff::IDiffCallback
{
public:
	void readPhraseFile(const string &filename, const string &workfilename, ucstring &text, vector<TPhrase> &phrases)
	{
		vector<TPhrase>	addition;
		vector<TPhrase>	reference;
		vector<TPhrase>	diff;

		std::string referenceFile;
		referenceFile = CPath::lookup(filename, false, VerboseStringManagerParser);

		if (!referenceFile.empty())
		{
			STRING_MANAGER::readPhraseFile(referenceFile, reference, false);
		}

		// try to find the working file
		string	workingFile = CPath::lookup(workfilename, false, VerboseStringManagerParser);
		if (workingFile.empty() || workingFile == referenceFile)
			workingFile = SM->TranslationWorkPath+CFile::getFilename(workfilename);
//		string	workingFile = CPath::lookup(workfilename, false, VerboseStringManagerParser);
		//string workingFile(SM->TranslationWorkPath+CFile::getFilename(filename));
		if (!workfilename.empty() 
			&& SM->ReadTranslationWork 
			&& !workingFile.empty() 
			&& CFile::fileExists(workingFile))
		{
			LOGPARSE("readPhraseFile : reading working file '%s'", workingFile.c_str());
			STRING_MANAGER::readPhraseFile(workingFile, addition, true);
			TPhraseDiffContext	context(addition, reference, diff);
			TPhraseDiff	differ;
			differ.makeDiff(this, context);

			phrases = diff;
			text = preparePhraseFile(phrases, true);
		}
		else
		{
			phrases = reference;
			text = preparePhraseFile(phrases, true);
		}
	}

	void readPhraseFileFromString(const ucstring &file, const string &filename, ucstring &text, vector<TPhrase> &phrases)
	{
		vector<TPhrase>	reference;

		std::string referenceFile;
		referenceFile = CPath::lookup(filename, false, VerboseStringManagerParser);

		if (!file.empty())
		{
			STRING_MANAGER::readPhraseFileFromString(file, filename, reference, false);
		}

		phrases = reference;
		text = preparePhraseFile(phrases, true);
	}

	void onEquivalent(uint addIndex, uint refIndex, TPhraseDiffContext &context)
	{
		context.Diff.push_back(context.Reference[refIndex]);
	}
	void onAdd(uint addIndex, uint refIndex, TPhraseDiffContext &context)
	{
		context.Diff.push_back(context.Addition[addIndex]);
		LOGPARSE("Using newly added phrase '%s'", context.Diff.back().Identifier.c_str());
	}
	void onRemove(uint addIndex, uint refIndex, TPhraseDiffContext &context)
	{
		// nothing to do because we don't insert bad value
		LOGPARSE("Removing phrase '%s'", context.Reference[refIndex].Identifier.c_str());
	}
	void onChanged(uint addIndex, uint refIndex, TPhraseDiffContext &context)
	{
		context.Diff.push_back(context.Addition[addIndex]);
		LOGPARSE("Using changed phrase '%s'", context.Diff.back().Identifier.c_str());
	}
	void onSwap(uint newIndex, uint refIndex, TPhraseDiffContext &context)
	{
		// don't swap.
	}
};

class CReadClauseFile : public TStringDiff::IDiffCallback
{
public:
	void readClauseFile(const string &filename, const vector<TPhrase> &phrases, ucstring &text)
	{
		vector<TStringInfo>	addition;
		vector<TStringInfo>	reference;
		vector<TStringInfo>	diff;

		std::string referenceFile;
		referenceFile = CPath::lookup(filename, false, VerboseStringManagerParser);

		if (!referenceFile.empty())
		{
			STRING_MANAGER::loadStringFile(referenceFile, reference, false);
		}


		// build the addition from the phrases clauses.
		if (SM->ReadTranslationWork)
		{
			{
				for (uint i=0; i<phrases.size(); ++i)
				{
					const TPhrase &phrase = phrases[i];

					for (uint j=0; j<phrase.Clauses.size(); ++j)
					{
						TStringInfo si;
						si.Identifier = phrase.Clauses[j].Identifier;
						si.Text = phrase.Clauses[j].Text;
						si.HashValue = CI18N::makeHash(si.Text);

						if (!si.Identifier.empty())
							addition.push_back(si);
					}
				}
			}


			TStringDiffContext	context(addition, reference, diff);
			TStringDiff	differ;
			differ.makeDiff(this, context);

			text = prepareStringFile(diff, true);
		}
		else
		{
			text = prepareStringFile(reference, true);
		}
//		string debug(text.toString());
//		nldebug("%s", debug.c_str());
	}

	void onEquivalent(uint addIndex, uint refIndex, TStringDiffContext &context)
	{
		context.Diff.push_back(context.Reference[refIndex]);
	}
	void onAdd(uint addIndex, uint refIndex, TStringDiffContext &context)
	{
		context.Diff.push_back(context.Addition[addIndex]);
		context.Diff.back().Text = ucstring("<NEW>")+context.Diff.back().Text;
		LOGPARSE("Adding new clause '%s'", context.Diff.back().Identifier.c_str());	
	}
	void onRemove(uint addIndex, uint refIndex, TStringDiffContext &context)
	{
		// nothing to do because we don't insert bad value
		LOGPARSE("removing clause '%s'", context.Reference[refIndex].Identifier.c_str());	
	}
	void onChanged(uint addIndex, uint refIndex, TStringDiffContext &context)
	{
		context.Diff.push_back(context.Addition[addIndex]);
		context.Diff.back().Text = ucstring("<CHG>")+context.Diff.back().Text;
		LOGPARSE("Changing clause '%s'", context.Diff.back().Identifier.c_str());	
	}
	void onSwap(uint newIndex, uint refIndex, TStringDiffContext &context)
	{
		// don't swap.
	}
};
class CReadWorkSheetFile : public TWorkSheetDiff::IDiffCallback
{
public:
	void readWorkSheetFile(const string &filename, const string &workfilename, ucstring &text)
	{
		TWorksheet	addition;
		TWorksheet	reference;
		TWorksheet	diff;

		std::string referenceFile;
		referenceFile = CPath::lookup(filename, false, VerboseStringManagerParser);

		if (!referenceFile.empty())
		{
			STRING_MANAGER::loadExcelSheet(referenceFile, reference);
			STRING_MANAGER::makeHashCode(reference, false);
		}

		// try to find the working file
		string workingFile = CPath::lookup(workfilename, false, VerboseStringManagerParser);
		if (workingFile.empty() || workingFile == referenceFile)
			workingFile = SM->TranslationWorkPath+CFile::getFilename(workfilename);
		if (SM->ReadTranslationWork && CFile::fileExists(workingFile))
		{
			LOGPARSE("readWorkSheetFile : reading working file '%s'", workingFile.c_str());
			STRING_MANAGER::loadExcelSheet(workingFile, addition);
			STRING_MANAGER::makeHashCode(addition, true);
		}
		else
		{
			text = prepareExcelSheet(reference);
			return;
		}

		if (addition.size() == 0)
			return;

		// create missing columns in reference and addition to make the diff
		for (uint i=0; i<reference.ColCount || i < addition.ColCount; ++i)
		{
			if (i >= reference.ColCount)
			{
				// need to add the missing column in reference
				reference.insertColumn(i);
				reference.setData(0, i, addition.getData(0, i));
			}
			else if (i >= addition.ColCount)
			{
				// need to add missing column in addition
				addition.insertColumn(i);
				addition.setData(0, i, reference.getData(0, i));
			}
			else if (reference.getData(0, i) != addition.getData(0, i))
			{
				// column don't match, look in add if the column exist
				uint colIndex;
				if (addition.findCol(reference.getData(0, i), colIndex))
				{
					// Ok, the column exist but at another position, move the column ? 
					nlassert(colIndex > i);
					
					addition.moveColumn(colIndex, i);
				}
				else
				{
					// the column don't exist in addition, insert an empty one
					addition.insertColumn(i);
					addition.setData(0, i, reference.getData(0, i));
				}
			}
		}
		// check column consistency.
		bool doDiff = true;
		uint i;
		if (reference.ColCount != addition.ColCount)
		{
			nlwarning("Can't check for difference for file %s, column number is not the same (found %u in translated and %u in works!", 
				filename.c_str(),
				reference.ColCount,
				addition.ColCount);
			doDiff = false;
		}
		if (doDiff)
		{
			// check each column name
			for (i=0; i<addition.ColCount; ++i)
			{
				if (addition.getData(0, i) != reference.getData(0, i))
				{
					nlwarning("Can't check difference for file %s, collumn name differ !", filename.c_str());
					doDiff = false;
					break;
				}
			}
			if (doDiff)
			{
				// create the column header.
				while (diff.ColCount < addition.ColCount)
					diff.insertColumn(0);
				diff.push_back(*addition.begin());
				TWordsDiffContext	context(addition, reference, diff);
				TWorkSheetDiff	differ;
				differ.makeDiff(this, context, true);
			}
		}
		
		if (!doDiff)
		{
			diff = reference;
		}

		text = prepareExcelSheet(diff);
	}

	void onEquivalent(uint addIndex, uint refIndex, TWordsDiffContext &context)
	{
		context.Diff.push_back(context.Reference[refIndex]);
	}
	void onAdd(uint addIndex, uint refIndex, TWordsDiffContext &context)
	{
		context.Diff.push_back(context.Addition[addIndex]);
		LOGPARSE("Using newly sheet row %s", context.Diff.getData(context.Diff.size()-1, 1).toString().c_str());
	}
	void onRemove(uint addIndex, uint refIndex, TWordsDiffContext &context)
	{
		// nothing to do because we don't insert bad value
	}
	void onChanged(uint addIndex, uint refIndex, TWordsDiffContext &context)
	{
		context.Diff.push_back(context.Addition[addIndex]);
		LOGPARSE("Using changed sheet row %s", context.Diff.getData(context.Diff.size()-1, 1).toString().c_str());
	}
	void onSwap(uint newIndex, uint refIndex, TWordsDiffContext &context)
	{
		// don't swap.
	}
};


bool CStringManager::parseClauseStrings(const ucstring &clausesStrings)
{
	std::string lastRead = "nothing";
	bool b = true;
	ucstring::const_iterator first(clausesStrings.begin()), last(clausesStrings.end());
	while (first != last)
	{
		NLMISC::CI18N::skipWhiteSpace(first, last);
		std::string label;
		ucstring text;

		if (first == last)
			break;

		if (!NLMISC::CI18N::parseLabel(first, last, label))
		{
			nlwarning("Error reading label in clause string file, aborting after %s.", lastRead.c_str());
			return false;
		}

		lastRead = label;

		NLMISC::CI18N::skipWhiteSpace(first, last);
		if (!NLMISC::CI18N::parseMarkedString('[', ']', first, last, text))
		{
			nlwarning("Error reading text in clause string, aborting on %s.", lastRead.c_str());
			return false;
		}

		std::pair<std::map<std::string, ucstring>::iterator, bool>	ret;
		ret = SM->TempClauseStrings.insert(std::make_pair(label, text));
		if (!ret.second)
		{
			nlwarning("Duplicate string label %s !", label.c_str());
			b = false;
		}
	}
	return b;
}


CStringManager::CEntityWords CStringManager::parseEntityWords(const ucstring &str)
{
	CEntityWords	ew;
	if (ew._Data != NULL)
		delete ew._Data;
	ew._Data = 0;
	ew._NbColums = 0;
	if (str.empty())
		return ew;

	TWorksheet	ws;
	STRING_MANAGER::readExcelSheet(str, ws);

	if (ws.size() == 0)
		return ew;

	uint i;
	// remove any unwanted column
	for (i=0; i<ws.ColCount; ++i)
	{
		const ucstring &colName = ws.getData(0, i);
		if (colName.empty() || colName[0] == '*')
		{
			ws.eraseColumn(i);
			--i;
		}
	}


	// init the column in the entity word
	ew._NbColums = ws.ColCount;
	for (i=0; i<ws.ColCount; ++i)
	{
		ew._ColumnInfo.insert(make_pair(ws.getData(0, i).toString(), i));
	}
	// fill the data
	ew._Data = new uint32[ws.size() * ws.ColCount];
	for (i=1; i<ws.size(); ++i)
	{
		// on the first col, we uncapitalize the id
		ws.setData(i, 0, ucstring(NLMISC::toLower(ws.getData(i, 0).toString())));

		ew._RowInfo.insert(make_pair(ws.getData(i, 0).toString(), i-1));
		for (uint j=0; j<ws.ColCount; ++j)
		{
			ucstring field = ws.getData(i, j);
			// parse any escape code
			ucstring::size_type pos;
			while ((pos = field.find(ucstring("\\"))) != ucstring::npos)
			{
				if (pos < field.size()-1)
				{
					if (field[pos+1] == '\\')
						field = field.substr(0, pos) + field.substr(pos+1);
					else if (field[pos+1] == 'd')
						field = field.substr(0, pos) + ucchar(8) + field.substr(pos+2);
					else if (field[pos+1] == 'n')
						field = field.substr(0, pos) + ucchar('\n') + field.substr(pos+2);
					else if (field[pos+1] == 't')
						field = field.substr(0, pos) + ucchar('\t') + field.substr(pos+2);
					else
					{
						nlwarning("Invalid escape code '\\%c' in field [%s]", (char)field[pos+1], field.toString().c_str());
						field = field.substr(0, pos) + field.substr(pos+2);
					}
				}
				else
				{
					nlwarning("Invalid escape code '\\<eol>' in field [%s]", field.toString().c_str());
					field = field.substr(0, pos);
				}
			}

			ew._Data[(i-1)*ws.ColCount + j] = storeString(field);
		}
	}




	return ew;
}


void CStringManager::parsePhraseDoc(ucstring &doc, uint langNum)
{
	enum TToken
	{
		token_label,
		token_param_list,
		token_string_list
	};

	struct CToken
	{
		TToken		Type;
		ucstring	Value;
	};

	// remove any comment
	NLMISC::CI18N::removeCComment(doc);

	//broke the text into phrase block
	ucstring block;
	std::list<ucstring>	blocks;

	ucstring::const_iterator first(doc.begin()), last(doc.end());
	for (;first != last; ++first)
	{
		block.push_back(*first);

		if (*first == '}')
		{
			// end of the block
			blocks.push_back(block);
			block.erase();
		}
		else if (*first == '[')
		{
			while (++first != last)
			{
				block.push_back(*first);
				if (*first == ']')
					break;
			}
		}
	}
	// create a block with the rest (if any)
	if (first != last)
		blocks.push_back(block);

	// loaded phrases in this file
	std::set<std::string>	loadedPhrases;

	// parse indivudual blocks
	while (!blocks.empty())
	{
		CPhrase phrase;
		if (!parseBlock(blocks.front(), phrase))
		{
			nlwarning("Malformed block : [%s] in %s file", blocks.front().toString().c_str(), _LanguageCode[langNum].c_str());
		}
		else
		{
			// only yell if phrase is duplicated in the same file
			if (loadedPhrases.find(phrase.Name) != loadedPhrases.end())
			{
				nlwarning("Error : duplicate phrase [%s] in %s file", phrase.Name.c_str(), _LanguageCode[langNum].c_str());
			}
			else
			{
				_AllPhrases[langNum][phrase.Name] = phrase;
				loadedPhrases.insert(phrase.Name);
			}
		}
		blocks.pop_front();
	}
}

bool CStringManager::parseBlock(const ucstring &block, CPhrase &phrase)
{
//		CPhrase	phrase;
	ucstring::const_iterator first(block.begin()), last(block.end());

	// read the phrase name
	NLMISC::CI18N::skipWhiteSpace(first, last);
	if (first == last)
	{
		// nothing interesting in this block ! only space, just ignore it
		return true;
	}
	if (!NLMISC::CI18N::parseLabel(first, last, phrase.Name))
		return false;

//	nldebug("Found block named [%s]", phrase.Name.c_str());

	// Read the param list
	if (!parseParamList(first, last, phrase.Params))
		return false;
	// read the clauses
	if (!parseClauses(phrase, first, last, phrase.Clauses))
		return false;

	// check coherency between parameters conditions and strings
	{
		uint count = 1;
		std::vector<CClause>::iterator first(phrase.Clauses.begin()), last(phrase.Clauses.end());
		for (; first != last; ++first, ++count)
		{
			CClause &es = *first;
			for (uint i=0; i<es.Conditions.size(); ++i)
			{
				for (uint k=0; k<es.Conditions[i].size(); ++k)
				{
					// check args in conditions
	//				if (es.Conditions[i][k].Operand != "self")
					if (es.Conditions[i][k].ParamIndex != 0)
					{
						bool found = false;
						for (uint j=0; j<phrase.Params.size(); ++j)
						{
	//						if (phrase.Params[j].Name == es.Conditions[i][k].Operand)
							if (phrase.Params[j]->ParamId.Index == es.Conditions[i][k].ParamIndex)
							{
								found = true;
								break;
							}
						}
						if (!found)
						{
							nlwarning("Condition %d in clause %d use parameter [%s] that is unknown in block [%s]", i, count, phrase.Params[es.Conditions[i][k].ParamIndex]->ParamId.Name.c_str(), block.toString().c_str());
							return false;
						}
					}
				}
			}

			// check the arg in the string
			std::vector<TReplacement> rep;
			if (!extractReplacement(phrase, es.String, rep))
			{
				nlwarning("Failed to extract replacement point in clause %d", count);
				return false;
			}
/*			for (uint k=0; k<rep.size(); ++k)
			{
				// check that the var name exist in the param
				if (rep[k].ParamIndex != 0)
				{
					TParamId *pparamId;
					if (!findParam(phrase, rep[k].ParamName, pparamId))
					{
						nlwarning("Text in clause %d use parameter [%s] that is unkown in block", count, rep[k].ParamName.c_str());
						return false;
					}
				}
			}
*/		}
	}

	// Build the client strings
	for (uint i=0; i<phrase.Clauses.size(); ++i)
	{
		CClause &clause = phrase.Clauses[i];
//		std::vector<TReplacement>	reps;
		if (!extractReplacement(phrase, clause.String, clause.Replacements))
		{
			nlwarning("Error extrating replacement point in clause %u", i);
			return false;
		}

		if (!clause.Replacements.empty())
		{
			uint repCount = 0;
			ucstring::iterator first(clause.String.begin()), last(clause.String.end());
			for (; first != last; ++first)
			{
				// check for replacement point
				if (repCount < clause.Replacements.size()
					&& (first - clause.String.begin()) == (sint) clause.Replacements[repCount].InsertPlace)
				{
					// check parameter type
					const char *subst;
					uint paramIndex = clause.Replacements[repCount].ParamIndex;

					TParamId &paramId = phrase.Params[paramIndex]->ParamId;

					switch(paramId.Type)
					{
					case STRING_MANAGER::integer:
						subst = "%i";
						break;
					case STRING_MANAGER::time:
						subst = "%t";
						break;
					case STRING_MANAGER::money:
						subst = "%$";
						break;
					case STRING_MANAGER::dyn_string_id:
						subst = "%m";
						break;
					default:
						subst = "%s";
					}

					clause.ClientString += subst;

					// skip the tag
					first = clause.String.begin() + (clause.Replacements[repCount].ContinuePlace-1);

					// advance to next tag def
					repCount++;
				}
				else if (*first == '%')
				{
					// special case, need to add escape for the client
					clause.ClientString.push_back(*first);
					clause.ClientString.push_back(*first);
				}
				else if (*first == '$')
				{
					// this must be an escaped $, check and remove the second one
					nlassert((first+1) != last);
					nlassert(*(first+1) == '$');
					clause.ClientString.push_back(*first);
					++first;
				}
				else
				{
					clause.ClientString.push_back(*first);
				}
			}
		}
		else
		{
			// no replacement point, just copy and add % escape
			ucstring::iterator first(clause.String.begin()), last(clause.String.end());
			for (; first != last; ++first)
			{
				if (*first == '%')
				{
					// push 2 of them
					clause.ClientString.push_back(*first);
					clause.ClientString.push_back(*first);
				}
				else if (*first == '$')
				{
					// this must be an escaped $, check and remove the second one
					nlassert((first+1) != last);
					nlassert(*(first+1) == '$');
					clause.ClientString.push_back(*first);
					++first;
				}
				else
					clause.ClientString.push_back(*first);
			}
		}
//		nldebug("Client string result : \n       String = [%s]\nClient string = [%s]", clause.String.toString().c_str(), clause.ClientString.toString().c_str());
//		clause.ClientStringId = _DynDb.add(clause.ClientString, false);
//		clause.ClientStringId = _Mapper->map(clause.ClientString);
		clause.ClientStringId = storeString(clause.ClientString);
	
	}

	return true;
}

bool CStringManager::extractReplacement(const CPhrase &phrase, const ucstring &str, std::vector<TReplacement> &result)
{
//		std::vector<TReplacement> ret;
	result.clear();
	TReplacement rep;
	uint count = 0;
	ucstring::const_iterator first(str.begin()), last(str.end());
	for (; first != last; ++first)
	{
		if (*first == '$')
		{
			count ++;
			// here is a replacement point !
			rep.InsertPlace = first - str.begin();

			// skip the '$'
			++first;

			if (first != last && *first != '$')
			{
				ucstring tag;

				while (first != last && *first != '$')
					tag.push_back(*first++);
				if (first == last)
				{
					nlwarning("Error during extraction of replacement point %u, missing a closing '$' in [%s]", count, str.toString().c_str());
					return false;
				}

				rep.ContinuePlace = (first+1) - str.begin();
				if (!parseTag(phrase, tag, rep))
				{
					nlwarning("Error during parsing tag [%s] in [%s] (replacement point %u)", tag.toString().c_str(), str.toString().c_str(), count);
					return false;
				}

				result.push_back(rep);
			}
		}
	}
	return true;
}

bool CStringManager::parseTag(const CPhrase &phrase, const ucstring &tag, TReplacement &rep)
{
	ucstring::const_iterator first(tag.begin()), last(tag.end());
	std::string name;
	std::string spec;
	ucstring temp;

	NLMISC::CI18N::skipWhiteSpace(first, last);
	if (!NLMISC::CI18N::parseLabel(first, last, name))
	{
		nlwarning("Error reading tag name in the tag [%s]", tag.toString().c_str());
		return false;
	}

//	name = temp.toString();
	if (first != last && *first == '.')
	{
		++first;
		if (!NLMISC::CI18N::parseLabel(first, last, spec))
		{
			nlwarning("Error reading tag property in the tag [%s]", tag.toString().c_str());
			return false;
		}
		spec = NLMISC::toLower(spec);
	}
	else
		spec = "name";

//	rep.ParamName = name;
	const TParamId *pparamId;
	if (!findParam(phrase, name, pparamId))
	{
			nlwarning("Error the tag [%s] use an unknown parameter.", tag.toString().c_str());
			return false;
	}
	rep.ParamIndex = pparamId->Index;
	rep.Format = spec;
	return true;
}

bool CStringManager::findParam(const CPhrase &phrase, const std::string paramName, const TParamId *&pparamId)
{
	for (uint i=0; i<phrase.Params.size(); ++i)
	{
		if (phrase.Params[i]->ParamId.Name == paramName)
		{
			pparamId = &phrase.Params[i]->ParamId;
			return true;
		}
	}

	return false;
}

bool CStringManager::parseClauses(const CPhrase &phrase, ucstring::const_iterator &it, ucstring::const_iterator &last, std::vector<CClause> &clauses)
{
	NLMISC::CI18N::skipWhiteSpace(it, last);

	if (it != last && *it == '{')
	{
		// skip opening bracket
		++it;
		uint count = 1;
		do
		{
			NLMISC::CI18N::skipWhiteSpace(it, last);
			ucstring cond;
			ucstring text;

			if (it != last && *it == '}')
				break;

			CClause	clause;

			uint condGroup = 1;
			while (it != last && *it == '(')
			{
				if (!NLMISC::CI18N::parseMarkedString('(', ')', it, last, cond))
				{
					nlwarning("Error parsing condition(s) in clause %u, condition group %u", count, condGroup);
					return false;
				}
				std::vector<TCondition> conds;
				if (!parseCondition(phrase, cond, conds))
				{
					nlwarning("Error parsing the condition [%s] in clause %u, condition group %u", cond.toString().c_str(), count, condGroup);
					return false;
				}
				NLMISC::CI18N::skipWhiteSpace(it, last);
				clause.Conditions.push_back(conds);
				condGroup++;
			}
			std::string stringLabel;

			NLMISC::CI18N::skipWhiteSpace(it, last);
			// check if we have a string label
			if (it != last && *it != '[')
			{
				if (!NLMISC::CI18N::parseLabel(it, last, stringLabel))
				{
					nlwarning("Error parsing label in clause %u", count);
					return false;
				}
				NLMISC::CI18N::skipWhiteSpace(it, last);
			}
			// check if we have the string literal
			if (it != last && *it == '[')
			{
				if (!NLMISC::CI18N::parseMarkedString('[', ']', it, last, text))
				{
					nlwarning("Error parsing string in clause %u", count);
					return false;
				}
			}
			else
			{
				// we must have the label
				if (stringLabel.empty())
				{
					nlwarning("Error parsing clause %u : clause must have at least a label or a string literal", count);
					return false;
				}
			}

			// try to replace the text with the one comming from the clause file.
			if (!stringLabel.empty())
			{
				std::map<std::string, ucstring>::iterator it = SM->TempClauseStrings.find(stringLabel);
				if (it != SM->TempClauseStrings.end())
				{
					text = it->second;
//					nldebug("Using indirection to resove %s as %s", stringLabel.c_str(), text.toString().c_str());
				}
			}
			
			clause.String = text;

			clauses.push_back(clause);
			NLMISC::CI18N::skipWhiteSpace(it, last);
				
			count++;
		} while (it != last && *it != '}');

		if (it == last || *it != '}')
		{
			nlwarning("Missing closing bracket for end of clauses block");
			return false;
		}
		else
			++it;
	}
	else
	{
		nlwarning("Malformed or non existent clauses block");
		return false;
	}

	return true;
}

bool CStringManager::parseCondition(const CPhrase &phrase, const ucstring &str, std::vector<TCondition> &result)
{
	TCondition cond;

	if (str.empty())
		return true;

	ucstring::const_iterator first(str.begin()), last(str.end());
	uint count = 1;

	do
	{
		// skip & between conditions
		if (count != 1 && *first == '&')
			first++;
//		cond.Property = none;
		// condition format : paramName[.genre](=0|=1|>1|=M|=F|=N)
		std::string paramName;
		std::string propertyName;
		ucstring temp;
		NLMISC::CI18N::skipWhiteSpace(first, last);
		if (!NLMISC::CI18N::parseLabel(first, last, paramName))
		{
			nlwarning("Error parsing parameter name in condition [%s], part %u", str.toString().c_str(), count);
			return false;
		}
		if (first != last && *first == '.')
		{
			++first;
			// there is a property name
			if (!NLMISC::CI18N::parseLabel(first, last, cond.Property))
			{
				nlwarning("Error parsing parameter property in condition [%s], part %u", str.toString().c_str(), count);
				return false;
			}

/*			if (propertyName != "genre")
			{
				nlwarning("Unknown property [%s] specified in part %u", propertyName.c_str(), count);
				return false;
			}
			else
				cond.Property = genre;
*/		}
		NLMISC::CI18N::skipWhiteSpace(first, last);

		// read the operator
		std::string opstr;
		while (first != last)
		{
			if (*first == '='
				|| *first == '<'
				|| *first == '>'
				|| *first == '!'
				)
				opstr.push_back(char(*first++));
			else
				break;
		}
		if (opstr == "=")
			cond.Operator = equal;
		else if (opstr == "!=")
			cond.Operator = notEqual;
		else if (opstr == ">")
			cond.Operator = greater;
		else if (opstr == ">=")
			cond.Operator = greaterEqual;
		else if (opstr == "<")
			cond.Operator = less;
		else if (opstr == "<=")
			cond.Operator = lessEqual;
		else
		{
			nlwarning("Unknown operator [%s] in condition [%s] part %u", opstr.c_str(), str.toString().c_str(), count);
			return false;
		}

		// read the reference
		NLMISC::CI18N::skipWhiteSpace(first, last);
		if (!NLMISC::CI18N::parseLabel(first, last, cond.ReferenceStr))
		{
			cond.ReferenceStr.erase();
			// perhaps parameter is a integer literal.
			while (first != last && *first >= '0' && *first <='9')
			{
				cond.ReferenceStr.push_back(char(*first++));
			}
			if (cond.ReferenceStr.empty())
			{
				nlwarning("Can't read the reference for condition [%s] part %u", str.toString().c_str(), count);
				return false;
			}
		}
		NLMISC::strlwr(cond.ReferenceStr);

		// try to eval value as an integer value
		NLMISC::fromString(cond.ReferenceStr, cond.ReferenceInt);

		if (paramName != "self")
		{
			const TParamId *pparamId;
			if (!findParam(phrase, paramName, pparamId))
			{
				nlwarning("The parameter named [%s] is unknown in condition [%s], part %u", paramName.c_str(), str.toString().c_str(), count);
				return false;
			}
			cond.ParamIndex = pparamId->Index;
		}
		else
			cond.ParamIndex = 0;
		result.push_back(cond);

		NLMISC::CI18N::skipWhiteSpace(first, last);
		count++;
	} while (first != last && *first == '&');

	return true;
}


bool CStringManager::parseParamList(ucstring::const_iterator &it, ucstring::const_iterator &last, std::vector<CParameterTraits*> &result)
{
//	std::vector<TParamId> params;
	result.clear();

	// always insert a first arg for self hidden var.
	CParameterTraits *pt = CParameterTraits::createParameterTraits(STRING_MANAGER::self);
	TParamId self;
	self.Index = 0;
	self.Type = STRING_MANAGER::self;
	self.Name = "self";
	TStringParam &sp = *pt;
	sp.ParamId = self;

	result.push_back(pt);

	NLMISC::CI18N::skipWhiteSpace(it, last);
	if (it != last && *it == '(')
	{
		// found start of param list
		uint count = 1;

		do
		{
			// skip opening brace or separating coma
			++it;

			if (it != last && *it == ')')
				break;
			std::string type;
			std::string name;
			ucstring temp;

			NLMISC::CI18N::skipWhiteSpace(it, last);
			if (!NLMISC::CI18N::parseLabel(it, last, type))
			{
				nlwarning("Error parsing parameter %u type in param list", count);
				return false;
			}
			type = NLMISC::toLower(type);

			NLMISC::CI18N::skipWhiteSpace(it, last);
			if (!NLMISC::CI18N::parseLabel(it, last, name))
			{
				nlwarning("Error parsing parameter %u name in param list", count);
				return false;
			}
//			name = temp.toString();

			if (type.empty() || name.empty())
			{
				nlwarning("Invalid parameter %u type or name : [%s] [%s]", count, type.c_str(), name.c_str());
				return false;
			}

			TParamId	paramId;
			if (type == "item")
			{
				paramId.Type = STRING_MANAGER::item;
			}
			else if (type == "place")
			{
				paramId.Type = STRING_MANAGER::place;
			}
			else if (type == "creature")
			{
				paramId.Type = STRING_MANAGER::creature;
			}
			else if (type == "skill")
			{
				paramId.Type = STRING_MANAGER::skill;
			}			
			else if (type == "role")
			{
				paramId.Type = STRING_MANAGER::role;
			}
/*			else if (type == "career")
			{
				paramId.Type = STRING_MANAGER::career;
			}
			else if (type == "job")
			{
				paramId.Type = STRING_MANAGER::job;
			}
*/			else if (type == "ecosystem")
			{
				paramId.Type = STRING_MANAGER::ecosystem;
			}
			else if (type == "race")
			{
				paramId.Type = STRING_MANAGER::race;
			}
			else if (type == "sbrick")
			{
				paramId.Type = STRING_MANAGER::sbrick;
			}
			else if (type == "faction")
			{
				paramId.Type = STRING_MANAGER::faction;
			}
			else if (type == "guild")
			{
				paramId.Type = STRING_MANAGER::guild;
			}
			else if (type == "player")
			{
				paramId.Type = STRING_MANAGER::player;
			}
			else if (type == "int")
			{
				paramId.Type = STRING_MANAGER::integer;
			}
			else if (type == "bot")
			{
				paramId.Type = STRING_MANAGER::bot;
			}
			else if (type == "time")
			{
				paramId.Type = STRING_MANAGER::time;
			}
			else if (type == "money")
			{
				paramId.Type = STRING_MANAGER::money;
			}
			else if (type == "compass")
			{
				paramId.Type = STRING_MANAGER::compass;
			}
			else if (type == "string_id")
			{
				paramId.Type = STRING_MANAGER::string_id;
			}
			else if (type == "dyn_string_id")
			{
				paramId.Type = STRING_MANAGER::dyn_string_id;
			}
			else if (type == "creature_model")
			{
				paramId.Type = STRING_MANAGER::creature_model;
			}
			else if (type == "entity")
			{
				paramId.Type = STRING_MANAGER::entity;
			}
			else if (type == "bodypart")
			{
				paramId.Type = STRING_MANAGER::body_part;
			}			
			else if (type == "score")
			{
				paramId.Type = STRING_MANAGER::score;
			}
			else if (type == "sphrase")
			{
				paramId.Type = STRING_MANAGER::sphrase;
			}
			else if (type == "characteristic")
			{
				paramId.Type = STRING_MANAGER::characteristic;
			}
			else if (type == "damagetype")
			{
				paramId.Type = STRING_MANAGER::damage_type;
			}
			else if (type == "bot_name")
			{
				paramId.Type = STRING_MANAGER::bot_name;
			}
			else if (type == "powertype")
			{
				paramId.Type = STRING_MANAGER::power_type;
			}
			else if (type == "literal")
			{
				paramId.Type = STRING_MANAGER::literal;
			}
			else if (type == "title")
			{
				paramId.Type = STRING_MANAGER::title;
			}
			else if (type == "event_faction")
			{
				paramId.Type = STRING_MANAGER::event_faction;
			}
			else if (type == "classificationtype")
			{
				paramId.Type = STRING_MANAGER::classification_type;
			}
			else if (type == "outpost")
			{
				paramId.Type = STRING_MANAGER::outpost;
			}
			
			else
			{
				nlwarning("Invalid parameter %u type [%s]", count, type.c_str());
				return false;
			}

			CParameterTraits *pt = CParameterTraits::createParameterTraits(paramId.Type);

			paramId.Name = name;
			paramId.Index = count;

			TStringParam &sp = *pt;
			sp.ParamId = paramId;

			result.push_back(pt);

			NLMISC::CI18N::skipWhiteSpace(it, last);
			count++;
		} while (it != last && *it == ',');

		// some checking code
		if (it == last)
		{
			nlwarning("Unterminated param list !");
			return false;
		}
		
		if (*it != ')')
		{
			nlwarning("Unterminated param list !");
			return false;
		}
		++it;
	}
	else
	{
		nlwarning ("Malformed or non existend param list !");
		return false;
	}
	return true;
}

/*
 * Load Phrase file for a specified language
 */
void	CStringManager::loadPhraseFile(const std::string& filename, TLanguages language, const std::string& workfilename, NLMISC::CLog *log)
{
	log->displayNL("Reading and parsing phrase file %s for language %s...", filename.c_str(), getLanguageCodeString(language).c_str());
	// pre-load the phrase file
	ucstring phraseText;
	vector<TPhrase>	phrases;
	{
		CReadPhraseFile reader;
		reader.readPhraseFile(filename, workfilename, phraseText, phrases);
	}

	// parse the phase file
	{
		parsePhraseDoc(phraseText, language);
	}
}

/*
 * Merge EntityWords
 */
void	CStringManager::mergeEntityWordsFile(const std::string& filename, TLanguages language, STRING_MANAGER::TParamType wordType, NLMISC::CLog *log)
{
	TParameterTraitList	typeNames = CParameterTraits::getParameterTraitsNames();

	uint	i;
	for (i=0; i<typeNames.size(); ++i)
		if (typeNames[i].first == wordType)
			break;

	if (i >= typeNames.size())
	{
		log->displayNL("Failed to merge file '%s', specified wordType %d not found", filename.c_str(), wordType);
		return;
	}

	CEntityWords&	mergeInto = _AllEntityWords[language][i];
	CEntityWords	words;

	SM->loadEntityWordsFile(filename, filename, words);
	SM->mergeEntityWords(mergeInto, words);

	if (words._Data != NULL)
		delete words._Data;
}


/*
 * Merge EntityWords
 */
void	CStringManager::mergeEntityWords(CEntityWords& dest, const CEntityWords& source, NLMISC::CLog *log)
{
	std::vector<std::pair<std::string, uint32> >	extraColumns;
	uint32	extraRows = 0;

	// extra columns not supported yet
	uint32	osz = (uint32)source._ColumnInfo.size();
	std::map<std::string, uint32>::const_iterator	iti;
	for (iti=source._ColumnInfo.begin(); iti!=source._ColumnInfo.end(); ++iti)
		if (dest._ColumnInfo.find((*iti).first) == dest._ColumnInfo.end())
			extraColumns.push_back(std::make_pair<std::string, uint32>((*iti).first, osz+(uint32)extraColumns.size()));

	for (iti=source._RowInfo.begin(); iti!=source._RowInfo.end(); ++iti)
		if (dest._RowInfo.find((*iti).first) == dest._RowInfo.end())
			++extraRows;

	bool	dataSizeChanged = (extraRows != 0);
	uint32*	data = dest._Data;

	uint	sCols = (uint)source._ColumnInfo.size();
	uint	sRows = (uint)source._RowInfo.size();
	uint	oCols = (uint)dest._ColumnInfo.size();
	uint	oRows = (uint)dest._RowInfo.size();
	uint	nCols = oCols;
	uint	nRows = oRows+extraRows;

	if (dataSizeChanged != 0)
	{
		data = new uint32[nCols*nRows];
		memset(data, 0, nCols*nRows*sizeof(uint32));
		memcpy(data, source._Data, nCols*oRows*sizeof(uint32));
		// here add space at the end of each new line for room for new columns

		delete dest._Data;
		dest._Data = data;
	}

	for (iti=source._RowInfo.begin(); iti!=source._RowInfo.end(); ++iti)
	{
		std::map<std::string, uint32>::const_iterator	itf = dest._RowInfo.find((*iti).first);

		uint	row = (itf == dest._RowInfo.end() ? oRows++ : (*itf).second);
		nlassert(row < nRows);
		uint32*	pDstRow = data + nCols*row;
		uint32*	pSrcRow = source._Data + sCols*((*iti).second);

		// write (or rewrite) row index..
		dest._RowInfo[(*iti).first] = row;

		// write columns one by one in good place (in case columns were not sorted the same way in source and destination
		std::map<std::string, uint32>::const_iterator	its, itd;
		for (its=source._ColumnInfo.begin(); its!=source._ColumnInfo.end(); ++its)
		{
			itd = dest._ColumnInfo.find((*its).first);
			if (itd == dest._ColumnInfo.end())
				continue;

			pDstRow[(*itd).second] = pSrcRow[(*its).second];
		}
	}
}

/*
 * Load EntityWords file
 */
void	CStringManager::loadEntityWordsFile(const std::string& filename, const string &workfilename, CEntityWords& words, NLMISC::CLog *log)
{
	ucstring			ucs;
	log->displayNL("Loading words file '%s'", filename.c_str());

	CReadWorkSheetFile	reader;

	// read the worksheet
	reader.readWorkSheetFile(filename, workfilename, ucs);

	// parse the worksheet
	words = parseEntityWords(ucs);
}

struct CDisplayColumnInfo
{
	CDisplayColumnInfo() : MaxWidth(0), InRow(0)	{ }

	string	Name;
	uint	MaxWidth;
	uint	InRow;

	bool	operator < (const CDisplayColumnInfo& r) const	{ return InRow < r.InRow; }
};

static inline string	padAndCropString(const string& str, uint sz)
{
	if (str.size() > sz)
		return str.substr(0, sz);
	string	r = str;
	r.insert(r.size(), sz-r.size(), ' ');
	return r;
}

/*
 * display EntityWords
 */
void	CStringManager::displayEntityWords(TLanguages language, STRING_MANAGER::TParamType wordType, const std::string& wc, NLMISC::CLog *log)
{
	TParameterTraitList	typeNames = CParameterTraits::getParameterTraitsNames();

	uint	i;
	for (i=0; i<typeNames.size(); ++i)
		if (typeNames[i].first == wordType)
			break;

	if (i >= typeNames.size())
	{
		log->displayNL("Failed to display words, specified wordType %d not found", wordType);
		return;
	}

	const CEntityWords&	words = _AllEntityWords[language][i];

	vector<CDisplayColumnInfo>		columns;

	std::map<std::string, uint32>::const_iterator	iti;
	for (iti=words._ColumnInfo.begin(); iti!=words._ColumnInfo.end(); ++iti)
	{
		CDisplayColumnInfo	inf;
		inf.Name = (*iti).first;
		inf.MaxWidth = (uint)inf.Name.size();
		inf.InRow = (*iti).second;
		columns.push_back(inf);
	}

	std::sort(columns.begin(), columns.end());

	uint	nCols = (uint)words._ColumnInfo.size();
	uint	rRows = (uint)words._RowInfo.size();

	for (iti=words._RowInfo.begin(); iti!=words._RowInfo.end(); ++iti)
	{
		uint32*	pData = words._Data + (*iti).second*nCols;

		if (!wc.empty())
		{
			std::string	rname = getString(pData[columns[0].InRow]).toString();
			if (!testWildCard(rname, wc))
				continue;
		}

		for (uint col=0; col<columns.size(); ++col)
		{
			uint	sz = (uint)getString(pData[columns[col].InRow]).toString().size();
			if (columns[col].MaxWidth < sz)
				columns[col].MaxWidth = sz;
		}
	}

	for (i=0; i<columns.size(); ++i)
		log->displayRaw("%s ", padAndCropString(columns[i].Name, columns[i].MaxWidth).c_str());
	log->displayRawNL("");

	for (iti=words._RowInfo.begin(); iti!=words._RowInfo.end(); ++iti)
	{
		uint32*	pData = words._Data + (*iti).second*nCols;

		if (!wc.empty())
		{
			std::string	rname = getString(pData[columns[0].InRow]).toString();
			if (!testWildCard(rname, wc))
				continue;
		}

		for (uint col=0; col<columns.size(); ++col)
		{
			string	s = getString(pData[columns[col].InRow]).toString();
			log->displayRaw("%s ", padAndCropString(s, columns[col].MaxWidth).c_str());
		}
		log->displayRawNL("");
	}
}


/*
 * reset entity word
 */
void	CStringManager::setEntityWord(const std::string& path, const ucstring& value)
{
	std::string::size_type	start = 0, end = 0;

	if ((end = path.find('.', start)) == string::npos)
		return;
	string	lang = toLower(path.substr(start, end-start));
	start = end+1;

	if ((end = path.find('.', start)) == string::npos)
		return;
	string	wordentity = toLower(path.substr(start, end-start));
	start = end+1;

	if ((end = path.find('.', start)) == string::npos)
		return;
	string	wordid = path.substr(start, end-start);
	start = end+1;

	string	det = path.substr(start);

	TLanguages	language = checkLanguageCode(lang);
	if (toLower(getLanguageCodeString(language)) != lang)
		return;

	TParameterTraitList	typeNames = CParameterTraits::getParameterTraitsNames();

	uint	i;
	for (i=0; i<typeNames.size(); ++i)
		if (toLower(typeNames[i].second) == wordentity)
			break;

	if (i >= typeNames.size())
		return;

	CEntityWords&	words = _AllEntityWords[language][i];

	std::map<std::string, uint32>::const_iterator	itr = words._RowInfo.find(wordid);
	std::map<std::string, uint32>::const_iterator	itc = words._ColumnInfo.find(det);

	if (itr == words._RowInfo.end() || itc == words._ColumnInfo.end())
		return;

	words._Data[((*itr).second)*words._ColumnInfo.size() + (*itc).second] = storeString(value);
}


/*
 * Load bot names file
 */
void	CStringManager::loadBotNames(const std::string& filename, bool resetBotNames, NLMISC::CLog *log)
{
	if (resetBotNames)
		_BotNameTranslation.clear();

	ucstring ucs;
	CReadWorkSheetFile reader;
	reader.readWorkSheetFile(filename, filename, ucs);

	if (!ucs.empty())
		log->displayNL("Loading '%s'", filename.c_str());

	TWorksheet	ws;
	STRING_MANAGER::readExcelSheet(ucs, ws);

	if (ws.size() != 0)
	{
		// remove any unwanted column
		for (uint i=0; i<ws.ColCount; ++i)
		{
			const ucstring &colName = ws.getData(0, i);
			if (colName.empty() || colName[0] == '*')
			{
				ws.eraseColumn(i);
				--i;
			}
		}

		if (ws.ColCount >= 2)
		{
			// and read the worksheet content : first colum = untranslatedBotName, second column = translatedBotName.
			for (uint i=0; i<ws.size(); ++i)
			{
				const ucstring &name = ws.getData(i, 0);
				const ucstring &transName = ws.getData(i, 1);

				pair<std::map<uint32, uint32>::iterator, bool> ret = _BotNameTranslation.insert(make_pair(storeString(name), storeString(transName)));
//				_BotNameTranslation[storeString(name)] = storeString(transName);
				if (!ret.second)
				{
					nlwarning("Duplicate bot name '%s' in bot_name.txt, second definition ignored", name.toString().c_str());
				}
			}
		}
		else
		{
			nlwarning("Error parsing '%s', need at least 2 columns, %u found !", filename.c_str(), ws.ColCount);
		}
	}

	remapBotNames();
}

/*
 * Set bot name
 */
void	CStringManager::setBotName(const ucstring& botname, const ucstring& translation)
{
	_BotNameTranslation[storeString(botname)] = storeString(translation);
	remapBotNames();
}


/*
 * Remap bot names
 */
void	CStringManager::remapBotNames()
{
	// remap the name of all spawned bots...
	std::map<NLMISC::CEntityId, CCharacterInfos *> &idToInfo = IOS->getCharInfosCont();
	std::map<NLMISC::CEntityId, CCharacterInfos *>::iterator first(idToInfo.begin()), last(idToInfo.end());
	for (; first != last; ++first)
	{
		CCharacterInfos *ci = first->second;

		if (ci->EntityId.getType() == RYZOMID::player || translateShortName(ci->ShortNameIndex) == ci->UntranslatedShortNameIndex)
			continue;

//		CEntityId id(first->first);
		TDataSetRow dsr = first->second->DataSetIndex;
		IOS->addCharacterName(dsr, getString(ci->UntranslatedNameIndex), TSessionId(0));
	}
}


/*
 * read repository
 */
void	CStringManager::readRepository(const std::string& path, TLanguages language, NLMISC::CLog *log)
{
	TParameterTraitList	typeNames = CParameterTraits::getParameterTraitsNames();

	std::vector<std::string>	files;
	CPath::getPathContent(path, false, false, true, files);

	if (files.empty())
	{
		log->displayNL("path '%s' is empty", path.c_str());
		return;
	}

	bool oldMode = SM->ReadTranslationWork;
	// We don't want diff with work file now
	SM->ReadTranslationWork = false;

	uint	i;
	for (i=0; i<files.size(); ++i)
	{
		const std::string&	file = files[i];
		std::string			f = CFile::getFilename(file);

		// filename matches phrase name ?
		if (f == toString("phrase_%s.txt", getLanguageCodeString(language).c_str()))
		{
			log->displayNL("Loading phrase file '%s'...", file.c_str());
			loadPhraseFile(file, language, "", log);
		}
		// filename matches word name ?
		else if (testWildCard(f, toString("*_words_%s.txt", getLanguageCodeString(language).c_str())))
		{
			std::string	wordtype = f.substr(0, f.find(toString("_words_%s.txt", getLanguageCodeString(language).c_str())));

			uint	i;
			for (i=0; i<typeNames.size(); ++i)
				if (typeNames[i].second == wordtype)
					break;

			if (i >= typeNames.size())
				continue;

			log->displayNL("Loading word file '%s'...", file.c_str());
			mergeEntityWordsFile(file, language, typeNames[i].first, log);
		}
	}
	SM->ReadTranslationWork = oldMode;
}

void CStringManager::reloadEventFactions(NLMISC::CLog * log, std::string fileName)
{
	if (fileName.empty())
	{
		CConfigFile::CVar * var = IService::getInstance()->ConfigFile.getVarPtr("EventFactionTranslationFile");
		if (var)
			fileName = var->asString();
		else
			fileName = "event_factions.txt";
	}

	if (log)
		log->displayNL("Loading event factions from file '%s'", fileName.c_str());

	_EventFactionTranslation.clear();

	ucstring ucs;
	CReadWorkSheetFile reader;
	reader.readWorkSheetFile(fileName, fileName, ucs);

	TWorksheet	ws;
	STRING_MANAGER::readExcelSheet(ucs, ws);

	if (ws.size() != 0)
	{
		// remove any unwanted column
		for (uint i=0; i<ws.ColCount; ++i)
		{
			const ucstring &colName = ws.getData(0, i);
			if (colName.empty() || colName[0] == '*')
			{
				ws.eraseColumn(i);
				--i;
			}
		}

		if (ws.ColCount >= 2)
		{
			// and read the worksheet content : first colum = event faction ID, second column = translated event faction
			for (uint i=0; i<ws.size(); ++i)
			{
				const ucstring &name = ws.getData(i, 0);
				const ucstring &transName = ws.getData(i, 1);
				const uint32 nameId = storeString(name);
				const uint32 transNameId = storeString(transName);

				pair<std::map<uint32, uint32>::iterator, bool> ret = _EventFactionTranslation.insert(make_pair(nameId, transNameId));
				if (!ret.second)
				{
					if (log)
						log->displayNL("Warning: duplicated event faction '%s' in '%s', second definition ignored", name.toString().c_str(), fileName.c_str());
				}
				else if (VerboseStringManagerParser)
				{
					log->displayNL("Add event faction : '%s' (%u) -> '%s' (%u)",
						name.toString().c_str(), nameId,
						transName.toString().c_str(), transNameId
						);
				}
			}
		}
		else
		{
			if (log)
				log->displayNL("Error parsing '%s', need at least 2 columns, %u found !", fileName.c_str(), ws.ColCount);
		}
	}
	else
	{
		if (log)
			log->displayNL("Warning: no faction found in '%s'", fileName.c_str());
	}

	// remap the event faction of all players
	std::map<NLMISC::CEntityId, CCharacterInfos *> &idToInfo = IOS->getCharInfosCont();
	std::map<NLMISC::CEntityId, CCharacterInfos *>::iterator first(idToInfo.begin()), last(idToInfo.end());
	for (; first != last; ++first)
	{
		CCharacterInfos *ci = first->second;
		if (ci && ci->EntityId.getType() == RYZOMID::player)
		{
			if ( TheDataset.isAccessible(ci->DataSetIndex) )
			{
//				CMirrorPropValue<TYPE_EVENT_FACTION_ID> propEventFactionId( TheDataset, ci->DataSetIndex, DSPropertyEVENT_FACTION_ID );
//				propEventFactionId = SM->translateEventFaction( ci->UntranslatedEventFactionId );
			}
		}
	}
}

void CStringManager::init(NLMISC::CLog *log)
{

	IService::getInstance()->setCurrentStatus("ReadingPhrases");

	ReadTranslationWork = false;
	TranslationWorkPath = "./translation/work/";

	// read the translation work file?
	try
	{
		CConfigFile::CVar& cvDebugString = NLNET::IService::getInstance()->ConfigFile.getVar("ReadTranslationWork");
		ReadTranslationWork = cvDebugString.asInt() != 0;
	}
	catch(const EUnknownVar &) 
	{
		log->displayNL("<CStringManager::init> using default ReadTranslationWork (false)");
	}

	// read only this translation work file (not en,fr,de...)
	bool	readWorkOnly= false;
	try
	{
		CConfigFile::CVar& cvDebugString = NLNET::IService::getInstance()->ConfigFile.getVar("ReadWorkOnly");
		readWorkOnly = cvDebugString.asInt() != 0;
	}
	catch(const EUnknownVar &) 
	{
		log->displayNL("<CStringManager::init> using default ReadWorkOnly (false)");
	}

	// get the translation work path
	try
	{
		CConfigFile::CVar& cvWorkPath = NLNET::IService::getInstance()->ConfigFile.getVar("TranslationWorkPath");
		TranslationWorkPath = cvWorkPath.asString();
		TranslationWorkPath = CPath::standardizePath(TranslationWorkPath, true);
	}
	catch(const EUnknownVar &) 
	{
		log->displayNL("<CStringManager::init> using default DebugStringManager (%s)", TranslationWorkPath.c_str());
	}
	

	if (!_TestOnly)
	{
		// load the string cache
		loadCache();
	}

	// store the initial empty string
	storeString(ucstring(""));

	// Load the sheets Id.
	NLMISC::CSheetId::init(false);
	
	if (_SheetInfo.empty())
	{
		// std::map<std::string, TSheetInfo> container;
		// Load the sheet
		std::vector<std::string> exts;
		exts.push_back("creature");
		//exts.push_back("item");
		//exts.push_back("sitem");	// not more needed !
		exts.push_back("race_stats");

		// if the 'GeorgePaths' config file var exists then we try to perform a mini-scan for sheet files
		if (IService::isServiceInitialized() && (IService::getInstance()->ConfigFile.getVarPtr(std::string("GeorgePaths"))!=NULL))
		{
			loadForm(exts, NLNET::IService::getInstance()->WriteFilesDirectory.toString() + "ios_sheets.packed_sheets", _SheetInfo, false, false);
		}

		if (_SheetInfo.empty())
		{
			CConfigFile::CVar *var;
			// we failed to load any sheet, try to add the georges path and reload
			if ((var = IService::getInstance()->ConfigFile.getVarPtr ("GeorgePaths")) != NULL)
			{
				for (uint i = 0; i < var->size(); i++)
				{
					CPath::addSearchPath (var->asString(i), true, false);
				}
			}

			// reload with 'update' true this time
			loadForm(exts, NLNET::IService::getInstance()->WriteFilesDirectory.toString() + "ios_sheets.packed_sheets", _SheetInfo, true);
		}
	}

	// load the bot name translation table
	{
		_BotNameTranslation.clear();
		std::string fileName = "bot_names.txt";

		CConfigFile::CVar *var = IService::getInstance()->ConfigFile.getVarPtr("BotNameTranslationFile");
		if (var)
		{
			fileName = var->asString();
		}

		loadBotNames(fileName, true, log);
	}

	// load event faction translation table
	reloadEventFactions(log);

	// load each language file
	uint nbLanguages = NB_LANGUAGES;
	// Speed up for Devs: read translation work only
	if(readWorkOnly)
	{
		nlctassert(work==0);
		nbLanguages = 1;
		nlinfo( "Limiting to language 'wk' to speed-up testing" );
	}
	for (uint l=0; l<nbLanguages; ++l)
	{
		log->displayNL ("====================================");
		log->displayNL ("Reading text file for language '%s'.", _LanguageCode[l].c_str());

		// don't load 'work' language setup
//		if (l == (uint)work)
//			continue;

		{
			std::string	filename = std::string("phrase_")+_LanguageCode[l]+".txt";
			loadPhraseFile(filename, (TLanguages)l, "phrase_wk.txt", log);
		}

/*
		log->displayNL("Reading phrase file...");
		// pre-load the phrase file
		ucstring phraseText;
		vector<TPhrase>	phrases;
		{
			std::string filename = std::string("phrase_")+_LanguageCode[l]+".txt";
			CReadPhraseFile reader;
			reader.readPhraseFile(filename, phraseText, phrases);
		}

		// read the labeled string file in a temporary storage.
		//TempClauseStrings.clear();
		//{
		//	std::string filename = std::string("clause_")+_LanguageCode[l]+".txt";

		//	ucstring ucs;
//		//	NLMISC::CI18N::readTextFile(filename, ucs);
		//	CReadClauseFile reader;
		//	reader.readClauseFile(filename, phrases, ucs);
		//	NLMISC::CI18N::removeCComment(ucs);

		//	if (!parseClauseStrings(ucs))
		//	{
		//		log->displayNL ("There were some error in %s.", filename.c_str());
		//	}
		//}

		// parse the phase file
		{
			log->displayNL("Parsing phrase file for %s", _LanguageCode[l].c_str());
			parsePhraseDoc(phraseText, l);
		}
*/

		// read words files
		{

			std::vector<std::pair<STRING_MANAGER::TParamType, std::string> > typeNames = CParameterTraits::getParameterTraitsNames();
			for (uint e=0; e<typeNames.size(); ++e)
			{
				if (typeNames[e].first < STRING_MANAGER::NB_PARAM_TYPES)
				{
					std::string fileName = typeNames[e].second+"_words_"+_LanguageCode[l]+".txt";
					std::string workfileName = typeNames[e].second+"_words_wk.txt";
					loadEntityWordsFile(fileName, workfileName, _AllEntityWords[l][e]);
				}
				else
				{
					log->displayNL("Ignoring parameter type %s, type out of bound !", typeNames[e].second.c_str());
				}
			}
		}
	}

	// this is no more needed.
	TempClauseStrings.clear();

	IService::getInstance()->clearCurrentStatus("ReadingPhrases");
}


/*
 * Replace a phrase
 */
void CStringManager::setPhrase(std::string const& phraseName, ucstring const& phraseContent, TLanguages language)
{
	ucstring phraseText;
	vector<TPhrase>	phrases;
	{
		CReadPhraseFile reader;
		reader.readPhraseFileFromString(phraseContent, phraseName, phraseText, phrases);
	}
	ucstring localPhraseText = phraseText;
	parsePhraseDoc(localPhraseText, language);
}

