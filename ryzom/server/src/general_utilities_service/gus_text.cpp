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

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "nel/misc/i18n.h"
#include "game_share/utils.h"
#include "gus_text.h"


//-----------------------------------------------------------------------------
// namespaces
//-----------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;


//-----------------------------------------------------------------------------
// some local constants
//-----------------------------------------------------------------------------

static const ucstring EmptyString;
static const char* DefaultLanguageName= "default";


//-----------------------------------------------------------------------------
// GUS namespace
//-----------------------------------------------------------------------------

namespace GUS
{
	//-----------------------------------------------------------------------------
	// methods CLangText
	//-----------------------------------------------------------------------------

	CLangText::CLangText(const CSString& languageCode)
	{
		_Code= languageCode;
	}

	const CSString& CLangText::getLanguageCode() const
	{
		return _Code;
	}

	const ucstring& CLangText::get(const CSString& tokenName) const
	{
		TTexts::const_iterator it= _Texts.find(tokenName);
		return (it==_Texts.end())? EmptyString: it->second;
	}

	void CLangText::set(const CSString& tokenName,const ucstring& txt)
	{
		ucstring& ucs= _Texts[tokenName];
		if (!ucs.empty() && ucs!=txt)
			nlwarning("Language %s: Replacing text for token: %s with: %s",_Code.c_str(),tokenName.c_str(),txt.toUtf8().c_str());
		ucs= txt;
	}

	void CLangText::set(const CSString& tokenName,const CSString& txt)
	{
		ucstring uct;
		uct.fromUtf8(txt);
		set(tokenName,uct);
	}

	void CLangText::display() const
	{
		InfoLog->displayNL("Language %s: %d texts",_Code.c_str(),_Texts.size());
	}

	void CLangText::getTokenNameSet(std::set<NLMISC::CSString>& result,bool clearResultFirst)
	{
		if (clearResultFirst)
			result.clear();
		for (TTexts::iterator it=_Texts.begin();it!=_Texts.end();++it)
			result.insert(it->first);
	}


	//-----------------------------------------------------------------------------
	// methods CTextSet
	//-----------------------------------------------------------------------------

	void CText::setLanguage(const CSString& languageCode)
	{
		_ActiveLanguage= languageCode;
	}

	bool CText::read(const CSString& fileName)
	{
		// load the file
		ucstring ucFileBody;
		CI18N::readTextFile(fileName,ucFileBody);
		CSString fileBody=	ucFileBody.toUtf8();
		if (fileBody.empty())
			return false;

		// parse the file
		CVectorSString lines;
		fileBody.splitLines(lines);
		bool ok=true;
		for (uint32 i=0;i<lines.size();++i)
		{
			CSString line= lines[i].strip();
			if (line.empty())
				continue;
			// skip comments
			if (line.left(2)=="//")
				continue;
			// see whether this is a "<lang>:<keyword>" or just a "<keyword>"
			if (line.word(1)==":")
			{
				// get the langugae code from the line
				CSString lang= line.firstWord(true);
				// skip the ':'
				line.firstWord(true);
				// get the keyword from the line
				CSString keyword= line.firstWord(true);
				// make sure there's still some text left in the line
				DROP_IF (line.empty(),"Invalid line: "+lines[i],ok=false;continue);
				// add the text to our database...
				set(lang,keyword,line);
			}
			else
			{
				// get the keyword from the line
				CSString keyword= line.firstWord(true);
				// make sure there's still some text left in the line
				DROP_IF (line.empty(),"Invalid line: "+lines[i],ok=false;continue);
				// add the text to our database...
				set(keyword,line);
			}
		}

		return ok;
	}

	const ucstring& CText::get(const CSString& tokenName) const
	{
		const ucstring *result;
		result= &get(_ActiveLanguage,tokenName);
		if (result->empty() && _ActiveLanguage!=DefaultLanguageName)
			return get(DefaultLanguageName,tokenName);
		return *result;
	}

	const ucstring& CText::get(const CSString& languageCode,const CSString& tokenName) const
	{
		for (uint32 i=0;i<_LangTexts.size();++i)
		{
			if (_LangTexts[i].getLanguageCode()==languageCode)
				return _LangTexts[i].get(tokenName);
		}
		return EmptyString;
	}

	void CText::set(const CSString& languageCode,const CSString& tokenName,const ucstring& txt)
	{
		CLangText* theLangText= NULL;

		// look for the container for the given language code...
		for (uint32 i=0;i<_LangTexts.size();++i)
		{
			if (_LangTexts[i].getLanguageCode()==languageCode)
			{
				theLangText= &_LangTexts[i];
				break;
			}
		}

		// create a new container ifor this lang if needed
		if (theLangText==NULL)
		{
			_LangTexts.push_back(CLangText(languageCode));
			theLangText= &_LangTexts.back();
		};

		// perform the 'set' operation
		theLangText->set(tokenName,txt);
	}

	void CText::set(const CSString& tokenName,const ucstring& txt)
	{
		set(DefaultLanguageName,tokenName,txt);
	}

	void CText::display() const
	{
		for (uint32 i=0;i<_LangTexts.size();++i)
			_LangTexts[i].display();
	}

	void CText::getTokenNameSet(std::set<NLMISC::CSString>& result)
	{
		result.clear();
		for (uint32 i=0;i<_LangTexts.size();++i)
			_LangTexts[i].getTokenNameSet(result,false);
	}
}

//-----------------------------------------------------------------------------
