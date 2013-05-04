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



#ifndef RY_MISSION_STEP_TEMPLATE_H
#define RY_MISSION_STEP_TEMPLATE_H

#include "nel/ligo/primitive.h"

#include "mission_action.h"
#include "mission_manager/mission_event.h"

class CCharacter;

/**
 * template for mission steps
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2003
 */
class IMissionStepTemplate
{
	NL_INSTANCE_COUNTER_DECL(IMissionStepTemplate);

public:
	///\name init method
	//@{
	/// ctor
	inline IMissionStepTemplate()
		:_OOOStepIndex(0xFFFFFFFF),_Any(false),_Displayed(true),_IconDisplayedOnStepNPC(true),_IsInOverridenOOO(false),_User(NULL) {}	

	//BRIANCODE my appologies, need access to this data from CMissionStepGiveItem	
	struct CSubStep
	{
		NLMISC::CSheetId	Sheet;
		uint16		Quality;
		uint32		Quantity;
	};


	/// return a copy of the step
	IMissionStepTemplate * getCopy();
	/// dtor
	virtual ~IMissionStepTemplate();

	/// build the step
	virtual bool	buildStep( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData ) = 0;
	/// override the step texts
	void			overrideTexts( const std::string & phraseId, const TVectorParamCheck & params, bool addDefaultParams );
	/// set the roleplay text
	void			setRoleplayText( const std::string & phraseId, const TVectorParamCheck & params);
	/// resolve all texts params
	virtual bool	solveTextsParams( CMissionSpecificParsingData & missionData,CMissionTemplate * templ  );
	/// add an action to this step
	void			addAction(IMissionAction * action);
	//@}



	/// process a mission event. Return the "event value", by which the step state will be modified
	virtual uint	processEvent( const TDataSetRow & userRow, const CMissionEvent & event,uint subStepIndex,const TDataSetRow & giverRow ) = 0;
	/// return the initial states of all sub steps
	virtual void	getInitState( std::vector<uint32>& ret ) = 0;
	/// send the step context menu entry if necessary
	virtual uint32	sendContextText(const TDataSetRow& user, const TDataSetRow & interlocutor, CMission * instance, bool & gift, const NLMISC::CEntityId & giver ){ return 0;}
	///\return true if a menu entry must be displayed
	virtual bool	hasBotChatOption(const TDataSetRow & interlocutor, CMission * instance, bool & gift){ return false;}
	///\return the text id of the step roleplay text
	virtual uint32	sendRpStepText(CCharacter * user,const std::vector<uint32>& stepStates,const NLMISC::CEntityId & giver);
	///\return the text id of the step text
	virtual uint32	sendStepText(CCharacter * user,const std::vector<uint32>& stepStates,const NLMISC::CEntityId & giver);
	///\return the alias of the NPC/bot involved in the step, or CAIAliasTranslator::Invalid if there is none (or if specified as "giver", in which case invalidIsGiver is set to true)
	virtual TAIAlias getInvolvedBot(bool& invalidIsGiver) const { invalidIsGiver=false; return CAIAliasTranslator::Invalid; }
	/// check if the current player gift is ok
	virtual bool	checkPlayerGift( CMission* instance, CCharacter * user );
	/// callback called when the step is activated by a player (mission beginning or later when character comes back to game). stepIndex starts at 0 here.
	virtual void	onActivation(CMission* inst,uint32 stepIndex, std::list< CMissionEvent * > & eventList) {}
	/// When the mission step is removed when not successfully processed (e.g. aborted by user). stepIndex starts at 0 here.
	virtual void	onFailure(CMission *inst, uint32 stepIndex) {}
	/// When the mission step has type 'any' and is removed because another one was successfully processed. stepIndex starts at 0 here.
	virtual void	onCancelStepAny(CMission *inst, uint32 stepIndex) {}
	/// method called when an item bot gift is done. It returns true if the step handled the bot gift correctly
	virtual bool	itemGiftDone ( CCharacter & user , const std::vector< CGameItemPtr > & itemsGiven,const EGSPD::CActiveStepPD & step, std::vector<uint32>& result )
	{
		return false; 
	}
	
	///\name accessors
	//@{
	/// return the step actions
	inline const std::vector< IMissionAction* > &	getActions(){ return _Actions; }
	/// set the out of order index of the step ( it is the index of the OOO/ANY bloc embedding the step ) 0xFF is invalid
	inline	void									setOOOStepIndex(uint32 idx){ _OOOStepIndex = idx; }
	///\return the OOO index ( see previous line )
	inline	uint32									getOOOStepIndex(){ return _OOOStepIndex; }
	///\mark the step as "any"  (all the steps of an any bloc are ended if one of them is ended)
	inline void										setAsAny(){ _Any = true;}
	///\return true if the step was flagged as "any"
	inline bool										isAny(){ return _Any;}
	///\return true if the step is displayed
	bool											isDisplayed(){ return _Displayed;}
	///\set displayed value
	void											setDisplayed(bool displayed){ _Displayed = displayed;}
	///\return true if the step is displayed
	bool											isIconDisplayedOnStepNPC() const { return _IconDisplayedOnStepNPC;}
	///\set displayed value
	void											setIconDisplayedOnStepNPC(bool displayed){ _IconDisplayedOnStepNPC = displayed;}
	/// return true if the step is in an OOO block which text wad overriden
	bool isInOverridenOOO() { return _IsInOverridenOOO; }
	/// set the isInOverridenOOO flag ( see previous method )
	void setAsInOverridenOOO() { _IsInOverridenOOO = true; }
	/// return true if there is a roleplay text in this step
	bool isThereRoleplayText() { return !_RoleplayText.empty(); }
	/// check if the current player gift is ok
	virtual std::vector< CSubStep > getSubSteps() { return std::vector< CSubStep >();}


	/// retrieve the escort groups
	virtual void getEscortGroups( std::vector< TAIAlias > & groups ){}
	virtual bool checkEscortFailure( bool groupWiped ){return false;}

	/// check the consistency of a text step
	virtual bool checkTextConsistency()	{ return true;}
		
protected:
	///\fill the text parameters. Callled by sendStepText
	virtual void getTextParams( uint & nbSubSteps, const std::string* & textPtr,TVectorParamCheck& retParams, const std::vector<uint32>& subStepStates) = 0;
	/// allocate a new step
	virtual IMissionStepTemplate *		getNewPtr() = 0;
	/// actions triggered at the end of the step
	std::vector< IMissionAction* >		_Actions;

	/// Overriden text of the step
	std::string							_OverridenText;
	/// Roleplay text of the step
	std::string							_RoleplayText;
	/// additional params of the step
	TVectorParamCheck					_AdditionalParams;
	/// index of the out of order value of the step
	uint32								_OOOStepIndex;
	/// true if the step is an "ANY" step
	bool								_Any;
	/// true if the step is displayed
	bool								_Displayed;
	/// true if an icon is displayed on NPC having an interaction for current step
	bool								_IconDisplayedOnStepNPC;
	/// The source line of the mission
	uint32								_SourceLine;
	/// true if we have to add default params to the step overriden text
	bool								_AddDefaultParams;
	/// flag set to true if the step is in an OOO block which text wad overriden
	bool								_IsInOverridenOOO;
	/// Player running the mission
	CCharacter * 						_User;
};


/// factory used to build steps
class IMissionStepTemplateFactory
{
public:
	/// build a step from script
	static IMissionStepTemplate* buildStep( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData );
protected:
	/// little init trick to avoid the problem of static data init order
	static void init();
	///instanciate a step
	virtual IMissionStepTemplate * instanciate() = 0;
	/// the factory container
	static std::vector< std::pair< std::string, IMissionStepTemplateFactory* > > * Entries;
};

/// macro used to register steps
#define MISSION_REGISTER_STEP(_class_,_name_) \
	class _class_##StepTemplateFactory : public IMissionStepTemplateFactory \
{\
public:\
	_class_##StepTemplateFactory()\
	{\
		init();\
		std::string str = std::string(_name_); \
		for (uint i = 0; i < (*Entries).size(); i++ ) \
		{\
			if ( (*Entries)[i].first == str || (*Entries)[i].second == this )nlstop;\
		}\
		(*Entries).push_back( std::make_pair( str, this ) );\
	}\
	IMissionStepTemplate * instanciate()\
	{ \
		return new _class_; \
	} \
};\
_class_##StepTemplateFactory* _class_##StepTemplateFactoryInstance = new _class_##StepTemplateFactory;

#define MISSION_STEP_GETNEWPTR(_class_) \
IMissionStepTemplate* getNewPtr()\
{ \
	_class_ * ptr = new _class_;\
	*ptr = *this;\
	return ptr;\
}

extern void logMissionStep(uint32 line, const TDataSetRow & userRow, uint32 subStepIndex, const std::string &prefix, const std::string &stepNameAndParams);
#define LOGMISSIONSTEPSUCCESS(xxx) logMissionStep(_SourceLine, userRow, subStepIndex, "SUCCESS", xxx);
#define LOGMISSIONSTEPERROR(xxx) logMissionStep(_SourceLine, userRow, subStepIndex, "ERROR", xxx);

// ****************************************************************************

// Some forward definition

/**
 * Mission step for dyn chat
 */


class CMissionStepDynChat : public IMissionStepTemplate
{
public:
	virtual bool	buildStep( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData );
	virtual bool	solveTextsParams( CMissionSpecificParsingData & missionData,CMissionTemplate * templ  );
	uint processEvent( const TDataSetRow & userRow, const CMissionEvent & event,uint subStepIndex,const TDataSetRow & giverRow );
	void getInitState( std::vector<uint32>& ret );
	virtual void getTextParams( uint & nbSubSteps,const std::string* & textPtr,TVectorParamCheck& retParams, const std::vector<uint32>& subStepStates);
	void onActivation(CMission* inst,uint32 stepIndex, std::list< CMissionEvent * > & eventList);

	struct CAnswer
	{
		std::string							PhraseId;
		TVectorParamCheck					Params;
		std::string							Jump;
	};
	std::vector<CAnswer>				Answers;
	TAIAlias							Bot;
	std::string							PhraseId;
	TVectorParamCheck					Params;

	MISSION_STEP_GETNEWPTR(CMissionStepDynChat)
};


/**
 * Mission step for items collecting.
 */
class IMissionStepItem : public IMissionStepTemplate
{
public:
	
	struct CSubStep
	{
		NLMISC::CSheetId	Sheet;
		uint16				Quality;
		uint16				Quantity;
	};

	/// Return the properties of the requested items
	const std::vector< CSubStep >&	getRequestedItems() const { return _SubSteps; }

protected:

	virtual bool	buildStep( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData );
	void			getInitState( std::vector<uint32>& ret );
	void			getTextParams(uint & nbSubSteps, TVectorParamCheck& retParams, const std::vector<uint32>& subStepStates);

	std::vector< CSubStep > _SubSteps;
};


/**
 * Mission step for looting raw materials
 */
class CMissionStepLootRm : public IMissionStepItem
{
	uint					processEvent( const TDataSetRow & userRow, const CMissionEvent & event,uint subStepIndex,const TDataSetRow & giverRow );
	virtual void			getTextParams( uint & nbSubSteps, const std::string* & textPtr,TVectorParamCheck& retParams, const std::vector<uint32>& subStepStates);

	MISSION_STEP_GETNEWPTR(CMissionStepLootRm)
};


/**
 * Mission step for visiting a place/region...
 */
class CMissionStepVisit : public IMissionStepTemplate
{
	std::vector<NLMISC::CSheetId>	_WornItems;
	uint16							_PlaceId;

	virtual bool		buildStep( uint32 line, const std::vector< std::string > & script, CMissionGlobalParsingData & globalData, CMissionSpecificParsingData & missionData );

	/*
	 * When the mission step is activated (after a character takes it or when he is back in game), 
	 * insert the character id to the list of characters for which we have to check evenly
	 * if they are in the place at the right time. stepIndex starts at 0 here.
	 * events can be generated in onActivation and pushed in eventList
	 */
	virtual void		onActivation(CMission* inst, uint32 stepIndex, std::list< CMissionEvent * > & eventList);

	/// When the mission step is removed when not successfully processed (e.g. aborted by user). stepIndex starts at 0 here.
	virtual void		onFailure(CMission *inst, uint32 stepIndex)
	{
		IMissionStepTemplate::onFailure( inst, stepIndex );
		onDeleteStepPrematurely( inst, stepIndex );
	}

	/// When the mission step has type 'any' and is removed because another one was successfully processed. stepIndex starts at 0 here.
	virtual void		onCancelStepAny(CMission *inst, uint32 stepIndex)
	{
		IMissionStepTemplate::onCancelStepAny( inst, stepIndex );
		onDeleteStepPrematurely( inst, stepIndex );
	}

	/// React to the step deletion when not done by the visit place check in mission manager update
	void		onDeleteStepPrematurely(CMission *inst, uint32 stepIndex);

	/*
	 * Process an event.
	 * See also testMatchEvent()
	 */
	uint				processEvent( const TDataSetRow & userRow, const CMissionEvent & event,uint subStepIndex,const TDataSetRow & giverRow );

	void				getInitState( std::vector<uint32>& ret );
	virtual void		getTextParams( uint & nbSubSteps,const std::string* & textPtr,TVectorParamCheck& retParams, const std::vector<uint32>& subStepStates);

	MISSION_STEP_GETNEWPTR(CMissionStepVisit)

public:

	/*
	 * Test if the event matches the step (including generic contraints from inst)
	 * Preconditions: character and inst must be non-null
	 */
	 bool				testMatchEvent( const CCharacter *character, const CMission *inst, uint16 placeId ) const;
};


#endif // RY_MISSION_STEP_TEMPLATE_H

/* End of mission_step_template.h */



