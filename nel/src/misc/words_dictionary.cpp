// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2015  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include "nel/misc/words_dictionary.h"
#include "nel/misc/config_file.h"
#include "nel/misc/path.h"
#include "nel/misc/diff_tool.h"

using namespace std;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

const string DefaultColTitle = "name";

namespace NLMISC {

NL_INSTANCE_COUNTER_IMPL(CWordsDictionary);

/*
 * Constructor
 */
CWordsDictionary::CWordsDictionary()
{
}


/* Load the config file and the related words files. Return false in case of failure.
 * Config file variables:
 * - WordsPath: where to find <filter>_words_<languageCode>.txt
 * - LanguageCode: language code (ex: en for English)
 * - Utf8: results are in UTF8, otherwise in ANSI string
 * - Filter: "*" for all files (default) or a name (ex: "item").
 * - AdditionalFiles/AdditionalFileColumnTitles
 */
bool CWordsDictionary::init( const string& configFileName )
{
	// Read config file
	bool cfFound = false;
	CConfigFile cf;
	try
	{
		cf.load( configFileName );
		cfFound = true;
	}
	catch (const EConfigFile &e)
	{
		nlwarning( "WD: %s", e.what() );
	}
	string wordsPath, languageCode, filter = "*";
	vector<string> additionalFiles, additionalFileColumnTitles;
	bool filterAll = true, utf8 = false;
	if ( cfFound )
	{
		CConfigFile::CVar *v = cf.getVarPtr( "WordsPath" );
		if ( v )
		{
			wordsPath = v->asString();
			/*if ( (!wordsPath.empty()) && (wordsPath[wordsPath.size()-1]!='/') )
				wordsPath += '/';*/
		}
		v = cf.getVarPtr( "LanguageCode" );
		if ( v )
			languageCode = v->asString();
		v = cf.getVarPtr( "Utf8" );
		if ( v )
			utf8 = (v->asInt() == 1);
		v = cf.getVarPtr( "Filter" );
		if ( v )
		{
			filter = v->asString();
			filterAll = (filter == "*");
		}
		v = cf.getVarPtr( "AdditionalFiles" );
		if ( v )
		{
			for ( uint i=0; i!=v->size(); ++i )
				additionalFiles.push_back( v->asString( i ) );
			v = cf.getVarPtr( "AdditionalFileColumnTitles" );
			if ( v->size() != additionalFiles.size() )
			{
				nlwarning( "AdditionalFiles and AdditionalFileColumnTitles have different size, ignoring second one" );
				additionalFileColumnTitles.resize( v->size(), DefaultColTitle );
			}
			else
			{
				for ( uint i=0; i!=v->size(); ++i )
					additionalFileColumnTitles.push_back( v->asString( i ) );
			}
		}

	}
	if ( languageCode.empty() )
		languageCode = "en";

	// Load all found words files
	const string ext = ".txt";
	vector<string> fileList;
	CPath::getPathContent( wordsPath, false, false, true, fileList );
	for ( vector<string>::const_iterator ifl=fileList.begin(); ifl!=fileList.end(); ++ifl )
	{
		const string& filename = (*ifl);
		string::size_type p = string::npos;
		bool isAdditionalFile = false;

		// Test if filename is in additional file list
		uint iAdditionalFile;
		for ( iAdditionalFile=0; iAdditionalFile!=additionalFiles.size(); ++iAdditionalFile )
		{
			if ( (p = filename.find( additionalFiles[iAdditionalFile] )) != string::npos )
			{
				isAdditionalFile = true;
				break;
			}
		}

		// Or test if filename is a words_*.txt file
		string pattern = string("_words_") + languageCode + ext;
		if ( isAdditionalFile ||
			 ((p = filename.find( pattern )) != string::npos) )
		{
			// Skip if a filter is specified and does not match the current file
			if ( (!filterAll) && (filename.find( filter+pattern ) == string::npos) )
				continue;

			// Load file
			nldebug( "WD: Loading %s", filename.c_str() );
			_FileList.push_back( filename );
			string::size_type origSize = filename.size() - ext.size();
			const string truncFilename = CFile::getFilenameWithoutExtension( filename );
			const string wordType = isAdditionalFile ? "" : truncFilename.substr( 0, p - (origSize - truncFilename.size()) );
			const string colTitle = isAdditionalFile ? additionalFileColumnTitles[iAdditionalFile] : DefaultColTitle;

			// Load Unicode Excel words file
			STRING_MANAGER::TWorksheet worksheet;
			STRING_MANAGER::loadExcelSheet( filename, worksheet );
			uint ck, cw = 0;
			if ( worksheet.findId( ck ) && worksheet.findCol( ucstring(colTitle), cw ) ) // =>
			{
				for ( std::vector<STRING_MANAGER::TWorksheet::TRow>::iterator ip = worksheet.begin(); ip!=worksheet.end(); ++ip )
				{
					if ( ip == worksheet.begin() ) // skip first row
						continue;
					STRING_MANAGER::TWorksheet::TRow& row = *ip;
					_Keys.push_back( row[ck].toString() );
					string word = utf8 ? row[cw].toUtf8() : row[cw].toString();
					_Words.push_back( word );
				}
			}
			else
				nlwarning( "WD: %s ID or %s not found in %s", wordType.c_str(), colTitle.c_str(), filename.c_str() );
		}
	}

	if ( _Keys.empty() )
	{
		if ( wordsPath.empty() )
			nlwarning( "WD: WordsPath missing in config file %s", configFileName.c_str() );
		nlwarning( "WD: %s_words_%s.txt not found", filter.c_str(), languageCode.c_str() );
		return false;
	}
	else
		return true;
}


/*
 * Set the result vector with strings corresponding to the input string:
 * - If inputStr is partially or completely found in the keys, all the matching <key,words> are returned;
 * - If inputStr is partially or completely in the words, all the matching <key, words> are returned.
 * The following tags can modify the behaviour of the search algorithm:
 * - ^mystring returns mystring only if it is at the beginning of a key or word
 * - mystring$ returns mystring only if it is at the end of a key or word
 * All returned words are in UTF8.
 */
void CWordsDictionary::lookup( const CSString& inputStr, CVectorSString& resultVec ) const
{
	// Prepare search string
	if ( inputStr.empty() )
		return;

	CSString searchStr = inputStr;
	bool findAtBeginning = false, findAtEnd = false;
	if ( searchStr[0] == '^' )
	{
		searchStr = searchStr.substr( 1 );
		findAtBeginning = true;
	}
	if ( searchStr[searchStr.size()-1] == '$' )
	{
		searchStr = searchStr.rightCrop( 1 );
		findAtEnd = true;
	}

	// Search
	for ( CVectorSString::const_iterator ivs=_Keys.begin(); ivs!=_Keys.end(); ++ivs )
	{
		const CSString& key = *ivs;
		string::size_type p;
		if ( (p = key.findNS( searchStr.c_str() )) != string::npos )
		{
			if ( ((!findAtBeginning) || (p==0)) && ((!findAtEnd) || (p==key.size()-searchStr.size())) )
				resultVec.push_back( makeResult( key, _Words[ivs-_Keys.begin()] ) );
		}
	}
	for ( CVectorSString::const_iterator ivs=_Words.begin(); ivs!=_Words.end(); ++ivs )
	{
		const CSString& word = *ivs;
		string::size_type p;
		if ( (p = word.findNS( searchStr.c_str() )) != string::npos )
		{
			if ( ((!findAtBeginning) || (p==0)) && ((!findAtEnd) || (p==word.size()-searchStr.size())) )
				resultVec.push_back( makeResult( _Keys[ivs-_Words.begin()], word ) );
		}
	}
}


/*
 * Set the result vector with the word(s) corresponding to the key
 */
void CWordsDictionary::exactLookupByKey( const CSString& key, CVectorSString& resultVec )
{
	// Search
	for ( CVectorSString::const_iterator ivs=_Keys.begin(); ivs!=_Keys.end(); ++ivs )
	{
		if ( key == *ivs )
			resultVec.push_back( _Words[ivs-_Keys.begin()] );
	}
}


/*
 * Make a result string
 */
inline CSString CWordsDictionary::makeResult( const CSString &key, const CSString &word )
{
	return key + ": " + word.c_str();
}


/*
 * Return the key contained in the provided string returned by lookup() (without extension)
 */
CSString CWordsDictionary::getWordsKey( const CSString& resultStr )
{
	return resultStr.splitTo( ':' );
}

} // NLMISC




