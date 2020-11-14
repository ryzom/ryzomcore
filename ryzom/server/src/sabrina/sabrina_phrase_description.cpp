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




#include "nel/misc/sstring.h"
#include "game_share/egs_sheets/egs_sheets.h"
#include "sabrina_phrase_description.h"
#include "sabrina_phrase_model_factory.h"
#include "sabrina_pointers.h"
#include "sabrina_phrase_model.h"
 
/**
 * CSabrinaStaticPhraseDescriptionManager container class for the static phrase descriptions...
 * \author Sadge
 * \author Nevrax France
 * \date 2003
 */

// singleton data
std::vector<ISabrinaPhraseDescriptionPtr> CSabrinaStaticPhraseDescriptionManager::_StaticPhrases;
bool CSabrinaStaticPhraseDescriptionManager::_Initialised=false;
bool CSabrinaStaticPhraseDescriptionManager::_Released=false;

// init
void CSabrinaStaticPhraseDescriptionManager::init()
{
	// make sure we don't try to initialise more than once without releasing in between
	if(_Initialised && !_Released)
		return;
	_Initialised=true;
	_Released=false;

	// get a handle to the map of sheet ids to sphrase sheet records
	CSheets::init();
	std::vector<NLMISC::CSheetId> phraseSheets;
	NLMISC::CSheetId::buildIdVector(phraseSheets,"sphrase");
	nlassert(!phraseSheets.empty());

	// iterate through the set of sphrases to generate static sabrina phrase description records
	for (uint32 j=0;j<phraseSheets.size();++j)
	{
		// extract the sheet id from the map iterator
		NLMISC::CSheetId sheetId= phraseSheets[j];

		// make sure the phrase table is big enough to hold the new phrase
		// NOTE: we do this even if we fail to build the phrase (due to a data bug) as an attempt
		// to access passed the end of the table can then be classed as a code bug and asserted against.
		uint32 phraseId= sheetId.getShortId();
		if (_StaticPhrases.size()<=phraseId)
			_StaticPhrases.resize(phraseId+1,NULL);

		// try building the phrase description for the new phrase and record it in our phrase table
		// note that we use a generic smart pointer as well as a specialised pointer
		CSabrinaPhraseDescriptionStatic* newPhrase= new CSabrinaPhraseDescriptionStatic(sheetId);
		ISabrinaPhraseDescriptionPtr newPhrasePtr= newPhrase;
		if (newPhrase->getPhraseModel()!=NULL)
			_StaticPhrases[phraseId]=newPhrasePtr;
	}

	// count the number of empty slots in the phrase table in order to generate an approppriate warning message
	uint32 count=0;
	for (uint32 i=_StaticPhrases.size();i--;)
		if (_StaticPhrases[i]==NULL)
			++count;
	nlinfo("CSabrinaStaticPhraseDescriptionManager::init(): %i of %i static phrases built successfully",
		_StaticPhrases.size()-count,_StaticPhrases.size());
	if (count!=0)
		nlwarning("CSabrinaStaticPhraseDescriptionManager::init(): %i of %i static phrases missing due to missing files or invalid brick lists",
			count,_StaticPhrases.size());
}

// release
void CSabrinaStaticPhraseDescriptionManager::release()
{
	// make sure we don't release more than once
	if (!_Initialised || _Released)
		return;
	_Released=true;

	// we're using smart pointers so the following clears up memory very effectively
	_StaticPhrases.clear();
}

// getPhrase
CSabrinaPhraseDescriptionStatic* CSabrinaStaticPhraseDescriptionManager::getPhrase(NLMISC::CSheetId sheetId)
{
	#ifdef NL_DEBUG
		nlassert(_Initialised);
		nlassert(!_Released);
	#endif

	// make sure the sheet id exists within our vector
	uint32 phraseId= sheetId.getShortId();
	if (phraseId>=_StaticPhrases.size())
	{
		nlwarning("CSabrinaStaticPhraseDescriptionManager::getPhrase(): Attempt to access past end of phrase vector: %s",
			sheetId.toString().c_str());
		#ifdef NL_DEBUG
			nlstop
		#endif
		return NULL;
	}

	// make sure the phrase was built correctly during the init
	CSabrinaPhraseDescriptionStatic* thePhrase=NLMISC::safe_cast<CSabrinaPhraseDescriptionStatic*>((ISabrinaPhraseDescription*)_StaticPhrases[phraseId]);
	if (thePhrase==NULL)
	{
		nlwarning("CSabrinaStaticPhraseDescriptionManager::getPhrase(): Attempt to access missing phrase: %s",
			sheetId.toString().c_str());
		return NULL;
	}

	// make sure the sheet id's 'type' matches (typically, the sheetId::Unknown constant will fail here)
	if (thePhrase->getSheetId()!=sheetId)
	{
		nlwarning("CSabrinaStaticPhraseDescriptionManager::getPhrase(): Invalid file type - should be a .sphrase: %s",
			sheetId.toString().c_str());
		#ifdef NL_DEBUG
			nlstop
		#endif
		return NULL;
	}

	// success - return the pointer to our phrase...
	return thePhrase;
}



/**
 * ISabrinaPhraseDescription phrase description base class
 * \author Sadge
 * \author Nevrax France
 * \date 2003
 */

ISabrinaPhraseDescription::ISabrinaPhraseDescription()
{
}

ISabrinaPhraseDescription::ISabrinaPhraseDescription(const ISabrinaPhraseDescription& other)
{
	this->_Bricks= other._Bricks;
}

const std::vector<NLMISC::CSheetId>& ISabrinaPhraseDescription::getBricks() const
{
	return _Bricks; 
}

void ISabrinaPhraseDescription::writeToString(std::string& output) const	
{
	output+= (getType()==STATIC)? "S": "U";
	appendToString(output);
}

ISabrinaPhraseDescriptionPtr ISabrinaPhraseDescription::readFromString(std::string& input)
{
	ISabrinaPhraseDescriptionPtr result= NULL;
	switch(CSString(input).strip()[0])
	{
	case 'S':
		input=input.substr(1);
		result= new CSabrinaPhraseDescriptionStatic;
		if (!result->extractFromString(input))
			result=NULL;
		break;

	case 'U':
		input=input.substr(1); 
		result= new CSabrinaPhraseDescriptionUser;
		if (!result->extractFromString(input))
			result=NULL;
		break;

	default:
		nlwarning("ISabrinaPhraseDescription::readFromString: Failed due to invalid input: '%s'",input.c_str());
		return NULL;
	}
	return result;
}



/**
 * CSabrinaPhraseDescriptionStatic phrase description base class
 * \author Sadge
 * \author Nevrax France
 * \date 2003
 */

// construction
CSabrinaPhraseDescriptionStatic::CSabrinaPhraseDescriptionStatic()
{
	init(NLMISC::CSheetId::Unknown); 
}
CSabrinaPhraseDescriptionStatic::CSabrinaPhraseDescriptionStatic(NLMISC::CSheetId sheet)
{
	init(sheet); 
}

CSabrinaPhraseDescriptionStatic::CSabrinaPhraseDescriptionStatic(const CSabrinaPhraseDescriptionStatic& other)
{
	_Bricks		= other._Bricks;
	_Model		= other._Model;
	_SheetId	= other._SheetId;
}

// initialisation of the private data & construction of a sabrina phrase model record
void CSabrinaPhraseDescriptionStatic::init(NLMISC::CSheetId sheetId)
{  
	_SheetId=sheetId;
	_Model=NULL;

	// lookup the packed_sheet record for this sheet
	const CStaticRolemasterPhrase*	theSPhrase	= CSheets::getSRolemasterPhrase(sheetId);
	if (theSPhrase==NULL)
	{
		nlwarning("CSabrinaPhraseDescriptionStatic::init(): Ignoring phrase because not found in packed sheets: %s",
			sheetId.toString().c_str());
		return;
	}

	// try setting up the new phrase record from the brick list in the sphrase...
	_Model= CSabrinaPhraseModelFactory::newPhraseModel(theSPhrase->Bricks);

	// if the phrase isn't valid then display a warning
	if (_Model==NULL)
	{
		nlwarning("CSabrinaPhraseDescriptionStatic::init(): Ignoring phrase because failed to build phrase model: %s",
			sheetId.toString().c_str());
	}
}

// specialisation of public methods from parent class
CSabrinaPhraseDescriptionStatic::TType CSabrinaPhraseDescriptionStatic::getType() const
{
	return STATIC; 
}

const std::string& CSabrinaPhraseDescriptionStatic::getName() const	
{
	return _SheetId.toString(); 
}

const ISabrinaPhraseModel* CSabrinaPhraseDescriptionStatic::getPhraseModel() const
{
	return _Model; 
}

NLMISC::CSheetId CSabrinaPhraseDescriptionStatic::getSheetId() const
{
	return _SheetId; 
}

// specialisation of protected methods from parent class
void CSabrinaPhraseDescriptionStatic::appendToString(std::string& output) const
{
	output+= '('; 
	output+= _SheetId.toString(); 
	output+= ')'; 
}
bool CSabrinaPhraseDescriptionStatic::extractFromString(std::string& input)
{
	// our phrase is built as '('<sheet name>')'

	// make sure the input begins with a '(' and contains a ')' 
	CSString s=input;
	if (s.strip()[0]!='(' || !s.contains(")"));
	{
		nlwarning("CSabrinaPhraseDescriptionStatic::extractFromString: Failed due to invalid input: '%s'",input.c_str());
		return false;
	}
	// extract the sub-string between the '(' and ')' and make sure there are no more '('s between the 2
	s=s.splitFrom('(').splitTo(')').strip();
	if (s.contains("("))
	{
		nlwarning("CSabrinaPhraseDescriptionStatic::extractFromString: Failed due to bracket missmatch in input: '%s'",input.c_str());
		return false;
	}

	// extract the sheet id	and make sure it exists
	NLMISC::CSheetId sheet= NLMISC::CSheetId(s);
	if (sheet==NLMISC::CSheetId::Unknown)
	{
		nlwarning("CSabrinaPhraseDescriptionStatic::extractFromString: Failed due to brick name not found in sheetid.bin: '%s'",s.c_str());
		return false;
	}

	// build the phrase description record
	init(sheet);

	// remove the parsed segment from the input string and return
	input=CSString(input).splitFrom(')');
	return true;
}



/**
 * CSabrinaPhraseDescriptionUser
 * \author Sadge
 * \author Nevrax France
 * \date 2003
 */

// construction
CSabrinaPhraseDescriptionUser::CSabrinaPhraseDescriptionUser()
{
}
CSabrinaPhraseDescriptionUser::CSabrinaPhraseDescriptionUser(const std::string& name,const std::vector<NLMISC::CSheetId>& bricks)
{
	_Name= name;
	_Bricks= bricks;
}

// specialisation of public methods from parent class
CSabrinaPhraseDescriptionUser::TType CSabrinaPhraseDescriptionUser::getType() const	
{
	return STATIC; 
}

const std::string& CSabrinaPhraseDescriptionUser::getName() const	
{
	return _Name; 
}

const ISabrinaPhraseModel* CSabrinaPhraseDescriptionUser::getPhraseModel() const	
{
	return CSabrinaPhraseModelFactory::newPhraseModel(_Bricks); 
}

// specialisation of protected methods from parent class
void CSabrinaPhraseDescriptionUser::appendToString(std::string& output) const
{
	output+="(";
	output+=_Name;
	for (uint32 i=0; i<_Bricks.size();++i)
	{
		output+=':';
		output+=_Bricks[i].toString();
	}
	output+=")";
}

bool CSabrinaPhraseDescriptionUser::extractFromString(std::string& input)
{
	// our phrase is built as '(' <phrase name>[':'<brick name>[...]]')'

	// make sure the input begins with a '(' and contains a ')' 
	CSString s=input;
	if (s.strip()[0]!='(' || !s.contains(")"));
	{
		nlwarning("CSabrinaPhraseDescriptionUser::extractFromString: Failed due to invalid input: '%s'",input.c_str());
		return false;
	}
	// extract the sub-string between the '(' and ')' and make sure there are no more '('s between the 2
	s=s.splitFrom('(').splitTo(')').strip();
	if (s.contains("("))
	{
		nlwarning("CSabrinaPhraseDescriptionUser::extractFromString: Failed due to bracket missmatch in input: '%s'",input.c_str());
		return false;
	}

	// extract the phrase name
	_Name= s.strtok(": \t");

	// extract the sheet ids
	bool allSheetIdsOk=true;
	while (!s.strip().empty())
	{
		CSString sheetName=	s.strtok(": \t");
		NLMISC::CSheetId sheet= NLMISC::CSheetId(sheetName);
		if (sheet==NLMISC::CSheetId::Unknown)
		{
			nlwarning("CSabrinaPhraseDescriptionUser::extractFromString: Failed due to brick name not found in sheetid.bin: '%s'",sheetName.c_str());
			allSheetIdsOk= false;
		}
		_Bricks.push_back(sheet);
	}

	// remove the parsed segment from the input string and return
	input=CSString(input).splitFrom(')');
	return allSheetIdsOk;
}

