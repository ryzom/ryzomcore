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




#ifndef RY_SABRINA_PHRASE_DESCRIPTION_H
#define RY_SABRINA_PHRASE_DESCRIPTION_H

#include "nel/misc/sheet_id.h"
#include "sabrina_pointers.h"
 
/**
 * ISabrinaPhraseDescription phrase description base class
 * \author Sadge
 * \author Nevrax France
 * \date 2003
 */

class ISabrinaPhraseDescription : public NLMISC::CRefCount
{
public:
	enum TType { STATIC, USER };

	// constructors
	ISabrinaPhraseDescription();
	ISabrinaPhraseDescription(const ISabrinaPhraseDescription&);

	// read accessors
	virtual TType							getType()		 const =0;
	virtual const std::string&				getName()		 const =0;
	virtual const ISabrinaPhraseModel*		getPhraseModel() const =0;
	const std::vector<NLMISC::CSheetId>&	getBricks()		 const;

	// converting a phrase description to/ from a string for saving in player phrase books, etc
	void writeToString(std::string& output) const;
	static ISabrinaPhraseDescriptionPtr readFromString(std::string& input);

protected:
	virtual void appendToString(std::string& output)  const =0;
	virtual bool extractFromString(std::string& input) =0;

protected:
	std::vector<NLMISC::CSheetId> _Bricks;
};


/**
 * CSabrinaPhraseDescriptionStatic phrase description base class
 * \author Sadge
 * \author Nevrax France
 * \date 2003
 */

class CSabrinaPhraseDescriptionStatic : public ISabrinaPhraseDescription
{
public:
	// constructors
	CSabrinaPhraseDescriptionStatic();
	CSabrinaPhraseDescriptionStatic(NLMISC::CSheetId sheet);
	CSabrinaPhraseDescriptionStatic(const CSabrinaPhraseDescriptionStatic&);

	// initialisation of the private data & construction of a sabrina phrase model record
	void init(NLMISC::CSheetId sheet);

	// specialisation of public methods from parent class
	virtual TType						getType()		 const;
	virtual const std::string&			getName()		 const;
	virtual const ISabrinaPhraseModel*	getPhraseModel() const;
	NLMISC::CSheetId					getSheetId()	 const;

protected:
	// specialisation of protected methods from parent class
	virtual void appendToString(std::string& output) const;
	virtual bool extractFromString(std::string& input);

private:
	// private data
	NLMISC::CSheetId _SheetId;
	ISabrinaPhraseModelPtr _Model;
};


/**
 * CSabrinaPhraseDescriptionUser phrase description base class
 * \author Sadge
 * \author Nevrax France
 * \date 2003
 */

class CSabrinaPhraseDescriptionUser : public ISabrinaPhraseDescription
{
public:
	// construction
	CSabrinaPhraseDescriptionUser();
	CSabrinaPhraseDescriptionUser(const std::string& name,const std::vector<NLMISC::CSheetId>& bricks);

	// specialisation of public methods from parent class
	virtual TType						getType()		 const;
	virtual const std::string&			getName()		 const;
	virtual const ISabrinaPhraseModel*	getPhraseModel() const;

protected:
	// specialisation of protected methods from parent class
	virtual void appendToString(std::string& output) const;
	virtual bool extractFromString(std::string& input);

private:
	// private data
	std::string _Name;
};


/**
 * CSabrinaStaticPhraseDescriptionManager container class for the static phrase descriptions...
 * \author Sadge
 * \author Nevrax France
 * \date 2003
 */

class CSabrinaStaticPhraseDescriptionManager
{
public:
	static void init();
	static void release();

	static CSabrinaPhraseDescriptionStatic* getPhrase(NLMISC::CSheetId);

private:
	static std::vector<ISabrinaPhraseDescriptionPtr> _StaticPhrases;
	static bool _Initialised;
	static bool _Released;
};


#endif
