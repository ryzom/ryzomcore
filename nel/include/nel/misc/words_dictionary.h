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

#ifndef NL_WORDS_DICTIONARY_H
#define NL_WORDS_DICTIONARY_H

#include "types_nl.h"
#include "sstring.h"


namespace NLMISC {


/**
 * Words dictionary: allows to search for keys and words in <type>_words_<language>.txt
 * Unicode files. All searches are case not-sensitive.
 *
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2003
 */
class CWordsDictionary
{
	NL_INSTANCE_COUNTER_DECL(CWordsDictionary);
public:

	/// Constructor
	CWordsDictionary();

	/** Load the config file and the related words files. Return false in case of failure.
	 * Config file variables:
	 * - WordsPath: where to find <filter>_words_<languageCode>.txt
	 * - LanguageCode: language code (ex: en for English)
	 * - Utf8: results are in UTF8, otherwise in ANSI string
	 * - Filter: "*" for all files (default) or a name (ex: "item").
	 * - AdditionalFiles/AdditionalFileColumnTitles
	 */
	bool			init( const std::string& configFileName="words_dic.cfg" );

	/**
	 * Set the result vector with strings corresponding to the input string:
	 * - If inputStr is partially or completely found in the keys, all the matching <key,words> are returned;
	 * - If inputStr is partially or completely in the words, all the matching <key, words> are returned.
	 * The following tags can modify the behaviour of the search algorithm:
	 * - ^mystring returns mystring only if it is at the beginning of a key or word
	 * - mystring$ returns mystring only if it is at the end of a key or word
	 * All returned words are in UTF8 string or ANSI string, depending of the config file.
	 */
	void			lookup( const CSString& inputStr, CVectorSString& resultVec )const;

	/// Set the result vector with the word(s) corresponding to the key
	void			exactLookupByKey( const CSString& key, CVectorSString& resultVec );

	/// Return the key contained in the provided string returned by lookup() (without extension)
	CSString		getWordsKey( const CSString& resultStr );

	// Accessors
	const CVectorSString& getKeys() { return _Keys; }
	const CVectorSString& getWords() { return _Words; }

	/// Return the list of input file loaded at init
	const std::vector<std::string>& getFileList() const { return _FileList; }

protected:

	/// Make a result string
	static CSString		makeResult( const CSString &key, const CSString &word );

private:

	/// Keys (same indices as in _Words)
	CVectorSString				_Keys;

	/// Words (same indices as in _Keys)
	CVectorSString				_Words;

	/// Input file list
	std::vector<std::string>	_FileList;
};


} // NLMISC


#endif // NL_WORDS_DICTIONARY_H

/* End of words_dictionary.h */
