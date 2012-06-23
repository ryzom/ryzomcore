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



#ifndef RY_S_PHRASE_FACTORY_H
#define RY_S_PHRASE_FACTORY_H

#include "phrase_manager/s_phrase.h"
#include "egs_mirror.h"


/// Macro used to declare a Sabrina phrase class factory with a default implementation

#define DEFAULT_SPHRASE_FACTORY(_class_,_type_) \
class _class_##Factory : public ISPhraseFactory \
{\
public:\
	_class_##Factory()\
	{\
		init();\
		for (uint i = 0; i < (*Factories).size(); i++ ){ \
		if ( (*Factories)[i].first == _type_){nlerror("<ISPhraseFactory buildPhrase> brick type %s is affected to more than one class",BRICK_TYPE::toString(_type_).c_str() );}} \
		Factories->push_back(std::make_pair(_type_,this));\
	};\
protected:\
	CSPhrasePtr buildPhrase( const TDataSetRow & actorRowId, const std::vector< const CStaticBrick* >& bricks, bool execution )\
	{\
		_class_ *inst = new _class_;\
		if ( !inst->build( actorRowId, bricks, execution ) ){delete inst;return NULL;} \
		return inst;\
	}\
};\
_class_##Factory* _class_##FactoryInstance = new _class_##Factory;



/// Macro used to declare a Sabrina phrase class factory and implement the build phrase method
#define IMPLEMENT_SPHRASE_FACTORY(_class_,_type_) \
	class _class_##Factory : public ISPhraseFactory\
	{\
public:\
	_class_##Factory ()\
	{\
		init();\
		for (uint i = 0; i < (*Factories).size(); i++ ){ \
		if ( (*Factories)[i].first == _type_){nlerror("<ISPhraseFactory buildPhrase> brick type %s  is affected to more than one class",BRICK_TYPE::toString(_type_) );}} \
		Factories->push_back(std::make_pair(_type_,this));\
	};\
protected:\
	CSPhrasePtr buildPhrase( const TDataSetRow & actorRowId, const std::vector< const CStaticBrick* >& bricks, bool execution );\
}; \
_class_##Factory* _class_##FactoryInstance = new _class_##Factory;\
CSPhrasePtr _class_##Factory::buildPhrase( const TDataSetRow & actorRowId, const std::vector< const CStaticBrick* >& bricks )



/**
 * class factory for sabrina phrase
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2003
 */
class ISPhraseFactory
{
public:
	/// clear the class factory
	static void clear();

	///\build a phrase from the user row and the bricks composing the phrase
	static CSPhrasePtr buildPhrase( const TDataSetRow & actorRowId,const std::vector< NLMISC::CSheetId>& brickIds, bool execution );

	/// init the factory
	static void init()
	{
		if ( Factories == NULL )
			Factories = new std::vector< std::pair< BRICK_TYPE::EBrickType, ISPhraseFactory* > >;
	}

protected:

	/**
	 * Create a step from parameters
	 * \param params : a vector of vector of strings describing the step params
	 * \return a pointer on the built step (NULL if failure)
	 */
	virtual CSPhrasePtr buildPhrase( const TDataSetRow & actorRowId, const std::vector< const CStaticBrick* >& brickIds, bool execution ) = 0;

	///the phrase factories. We use a pointer here because we cant control the order in which ctor of static members are called
	static std::vector< std::pair< BRICK_TYPE::EBrickType, ISPhraseFactory* > >* Factories;
};

#endif // RY_S_PHRASE_FACTORY_H

/* End of s_phrase_factory.h */
