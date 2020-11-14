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

#ifndef NL_I18N_H
#define NL_I18N_H

#include "types_nl.h"
#include "debug.h"
#include "file.h"

#include <string>
#include <map>
#include <algorithm>


namespace NLMISC {


/**
 * Class for the internationalization. It's a singleton pattern.
 *
 * This class provide an easy way to localize all string.
 * First you load the language file with \c load().
 * Now, you can get a localized string with its association with \c get().
 *
 *\code
	// load the language French
	CI18N::load ("fr");
	// display "Salut" that is the "hi" string in the selected language (French).
	nlinfo (CI18N::get("Hi").c_str ());
	// display "rms est un master", the French version of the string
	nlinfo ("rms"+CI18N::get("Master").c_str ());
 *\endcode
 *
 *	Update 26-02-2002 Boris Boucher
 *
 *	Languages are now preferably handled via official language code.
 *	We use the ISO 639-1 code for language.
 *	Optionally, we can append a country code (ISO 3066) to differentiate
 *	between language flavor (eg Chinese is ISO 639-1 zh, but come in
 *	traditional or simplified form. So we append the country code :
 *	zh-CN (china) for simplified, zh for traditional).
 *
 *
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2000
 */

class CI18N
{
public:

	/// Control over text loading
	enum TLineFormat
	{
		// the text file is just loaded, no conversion or checks done on line delimiters
		LINE_FMT_NO_CARE,
		// the line delimiters are forced to LF (\n, code 0x0a)
		LINE_FMT_LF,
		// the line delimiters are forced to CRLF (\r\n , code 0x0d 0x0a)
		LINE_FMT_CRLF
	};

	/** Proxy interface for loading string file.
	 *	Implement this interface in client code and set it inside I18N
	 *	in order to be able to do any work on string file before they
	 *	are read by CI18N.
	 *	This is used by Ryzom to merge the working string file with
	 *	the exploitation one when they are more recent.
	 */
	struct ILoadProxy
	{
		virtual ~ILoadProxy() {}
		virtual void loadStringFile(const std::string &filename, ucstring &text) =0;
	};

	/// Set the load proxy class. Proxy can be NULL to unregister.
	static void setLoadProxy(ILoadProxy *loadProxy);

	// Get the current load proxy
	static ILoadProxy *getLoadProxy() { return _LoadProxy; }

	/// Return a vector with all language available. The vector contains the name of the language.
	/// The index in the vector is used in \c load() function
	static const std::vector<ucstring> &getLanguageNames();

	/** Return a vector with all language code available.
	 *	Code are ISO 639-2 compliant.
	 *	As in getLanguageNames(), the index in the vector can be used to call load()
	 */
	static const std::vector<std::string> &getLanguageCodes();
 
	/** Check if a language code is supported.
	 *	Code are ISO 639-2 compliant.
	 */
	static bool isLanguageCodeSupported(const std::string &lang);

	/// Load a language file depending of the language code("en", "fr", ...). Code are ISO 639-2 compliant.
	static void load (const std::string &languageCode, const std::string &fallbackLanguageCode="");

	/** Load a language file from its filename
	  * \param filename name of the language file to load, with its extension
	  * \param reload The file is being reloaded so error message won't be issued for strings that are overwritten
	  */
	static void loadFromFilename (const std::string &filename, bool reload);

	/// Returns the name of the language in the language name (English, Francais, ...)
	static ucstring getCurrentLanguageName ();

	/// Returns the code of the language ("fr", "en", ...)
	static std::string getCurrentLanguageCode ();

	/// Returns the code of the language ("fr", "en", ...) defined on system
	static std::string getSystemLanguageCode ();

	/// Define the code of the language ("fr", "en", ...) defined on system
	static bool setSystemLanguageCode (const std::string &languageCode);

	/// Find a string in the selected language and return his association.
	static const ucstring &get (const std::string &label);

	// Test if a string has a translation in the selected language.
	// NB : The empty string is considered to have a translation
	static bool			   hasTranslation(const std::string &label);

	/** Read the content of a file as a Unicode text.
	 *	The method support 16 bits or 8bits utf-8 tagged files.
	 *	8 bits UTF-8 encoding can be recognized by a non official header :
	 *	EF,BB, BF.
	 *	16 bits encoding can be recognized by the official header :
	 *	FF, FE, witch can be reversed if the data are MSB first.
	 *
	 *	Optionally, you can ask the reader to interpret #include commands.
	 */
	static void readTextFile(const std::string &filename,
								ucstring &result,
								bool fileLookup = true,
								bool preprocess = false,
								TLineFormat lineFmt = LINE_FMT_NO_CARE,
							    bool warnIfIncludesNotFound = true);

	/** Read the content of a buffer as a Unicode text.
	 *	This is to read preloaded Unicode files.
	 *	The method support 16 bits or 8bits utf-8 tagged buffer.
	 *	8 bits UTF-8 encoding can be recognized by a non official header :
	 *	EF,BB, BF.
	 *	16 bits encoding can be recognized by the official header :
	 *	FF, FE, witch can be reversed if the data are MSB first.
	 */
	static void readTextBuffer(uint8 *buffer, uint size, ucstring &result);

	/** Remove any C style comment from the passed string.
	 */
	static void removeCComment(ucstring &commentedString);

	/** Encode a Unicode string into a string using UTF-8 encoding.
	*/
	static std::string encodeUTF8(const ucstring &str);

	/** Write a Unicode text file using Unicode 16 or UTF-8 encoding.
	 */
	static void writeTextFile(const std::string filename, const ucstring &content, bool utf8 = true);

	static ucstring makeMarkedString(ucchar openMark, ucchar closeMark, const ucstring &text);

	//@{
	//\name Parsing utility
	/** Skip the white space.
	 *	You can optionally pass a ucstring pointer to receive any comments string that build the
	 *	white space.
	 *	This is useful if you want to keep the comments.
	 *	NB : comments are appended to the comments string.
	 */
	static void		skipWhiteSpace		(ucstring::const_iterator &it, ucstring::const_iterator &last, ucstring *storeComments = NULL, bool newLineAsWhiteSpace = true);
	/// Parse a label
	static bool		parseLabel			(ucstring::const_iterator &it, ucstring::const_iterator &last, std::string &label);
	/// Parse a marked string. NB : usually, we use [ and ] as string delimiters in translation files.
	static bool		parseMarkedString	(ucchar openMark, ucchar closeMark, ucstring::const_iterator &it, ucstring::const_iterator &last, ucstring &result, uint32 *lineCounter = NULL, bool allowNewline = true);
	/** Try to read a given token at current position.
	 *	The function will first skip any white space then try to read the token
	 *	If found, the function return true and 'it' is advanced after the matched token,
	 *	Otherwise, the function return false and 'it' is unchanged.
	 */
	static bool		matchToken(const char* token, ucstring::const_iterator &it, ucstring::const_iterator end);
	/// Advance iterator to the start of next line or to the end of string
	static void		skipLine(ucstring::const_iterator &it, ucstring::const_iterator end, uint32 &lineCounter);
	//@}

	//@{
	//\name Hash code tools.
	// Generate a hash value for a given string
	static uint64	makeHash(const ucstring &str);
	// convert a hash value to a readable string
	static std::string hashToString(uint64 hash);
	// convert a readable string into a hash value.
	static uint64 stringToHash(const std::string &str);
	// fast convert a hash value to a ucstring
	static void	hashToUCString(uint64 hash, ucstring &dst);
	//@}

	static void setNoResolution( bool b ){ noResolution = b; }

private:

	typedef std::map<std::string, ucstring>						StrMapContainer;

	static ILoadProxy											*_LoadProxy;

	static StrMapContainer										_StrMap;
	static bool													_StrMapLoaded;

	// the alternative language that will be used if the sentence is not found in the original language
	static StrMapContainer										_StrMapFallback;

	static std::vector<std::string>								_LanguageCodes;
	static std::vector<ucstring>								_LanguageNames;
	static std::string											_SystemLanguageCode;

	static bool													_LanguagesNamesLoaded;

	static std::string											_SelectedLanguageCode;
	static const ucstring										_NotTranslatedValue;

	/** Structure to hold contextual info during
	 *	read of preprocessed file
	 */
	struct TReadContext
	{
		/// The defined symbols
		std::set<std::string>		Defines;

		/// The if stack (push true until a bad test is found, push false for
		/// all subsequent if imbrication)
		std::vector<bool>			IfStack;
	};

private:

	/// Init _LanguageCodes and _LanguageNames
	static void initLanguages();

	static bool loadFileIntoMap(const std::string &filename, StrMapContainer &dest);

	/// The internal read function, it does the real job of readTextFile
	static void _readTextFile(const std::string &filename,
								ucstring &result,
								bool fileLookup,
								bool preprocess,
								TLineFormat lineFmt,
							    bool warnIfIncludesNotFound,
								TReadContext &readContext);

	// Don't resolve labels
	static bool noResolution;
};

} // NLMISC


#endif // NL_I18N_H

/* End of i18n.h */
