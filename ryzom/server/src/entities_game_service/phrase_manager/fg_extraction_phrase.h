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



#ifndef NL_FG_EXTRACTION_PHRASE_H
#define NL_FG_EXTRACTION_PHRASE_H

#include "forage_phrase.h"


/**
 * <Class description>
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2003
 */
class CFgExtractionPhrase : public CForagePhrase
{
public:

	/// Constructor
	CFgExtractionPhrase();

	/// \name Override methods from CSPhrase
	//@{
	virtual bool build( const TDataSetRow & actorRowId, const std::vector< const CStaticBrick* >& bricks, bool buildToExecute = true );

	/**
	 * evaluate phrase
	 * \param evalReturnInfos struct that will receive evaluation results
	 * \return true if eval has been made without errors
	 */
	virtual bool evaluate();
	
	/**
	 * validate phrase
	 * \return true if phrase is valide
	 */
	virtual bool validate();
	virtual bool update();
	virtual void execute();
	virtual bool launch();
	virtual void apply();
	//@}
	
	/**
	 * set the actor
	 */
	virtual void setActor( const TDataSetRow &entityRowId ){}
	
	/**
	 * called at the end of the latency time
	 */
	virtual void end();
	//@}

	/**
	 * called when the action is interrupted
	 */
	virtual void stop();
	//@}

	///\unused basic methods from CSPhrase
	//@{
	virtual void setPrimaryTarget( const TDataSetRow& ) {}
	//@}

	/// Return the number of extractions or cares since the build()
	uint16 nbExtractions() const { return _NbExtractions; }

	/// Return true if a cancellation of the phrase due to character's movement must be ignored at the moment
	virtual bool mustOverrideCancelStaticAction() const { return _NbExtractions==0; }

protected:

	///
	void				applyExtraction( CCharacter *player, float successFactor );

	///
	NLMISC::TGameCycle	averageDeliveryPeriod() { return (_RequestedProps[CHarvestSource::S]!=0) ? (NLMISC::TGameCycle)(1.0f / _RequestedProps[CHarvestSource::S]) : 100; }

	///
	bool				isCareAction() const { return _RequestedProps[CHarvestSource::A] == 0; }

	///
	void				doKamiOffering( CCharacter *player );

private:

	/// The target source 
	TDataSetRow		_SourceRowId;	

	/// Link to the source being foraged (can't move in memory, the deleted case is check in validate())
	CHarvestSource	*_Source;

	/**
	 * Requested properties.
	 * "Care" mode if A=0, then ReduceDmg used.
	 */
	float			_RequestedProps[CHarvestSource::NbPosRTProps];

	union
	{
		struct
		{

			/// Absorption
			float			Absorption[CHarvestSource::NbPosRTProps];

			/// S=action period, A,Q=obtained properties
			float			ObtainedProps[CHarvestSource::NbPosRTProps];
			
			/// Terrain specialization
			TEcotype					EcotypeSpec;

			/// Material specialization (group)
			RM_GROUP::TRMGroup			MaterialGroupFilter;

			/// Material specialization (family)
			//RM_FAMILY::TRMFamily		MaterialFamilyFilter;

			/// Factor for increase speed of quality
			float						QualitySlowFactor;

		} Extraction;

		struct
		{
			/// Delta
			float			Deltas[CHarvestSource::NbCareDeltas];

			/// S=action period, Kami offering number, Kami anger decrease delta
			float			KamiAngerDec[CHarvestSource::NbKamiAngerProps];
			
		} Care;

	} _Props;

	/// Delta level of action (difficulty of the action)
	sint32						_DeltaLvlAction;

	/// Used/progressing skill (depending on the terrain, set by build())
	SKILLS::ESkills				_UsedSkill;

	/// Number of extractions since the start of the action
	uint16						_NbExtractions;

	/// Ratio of source life (D) that is converted into damage to player, in percent (0 for no conversion)
	uint8						_LifeAbsorberRatio;

	/// Skill value of the used skill
	uint8						_SkillValue;

	// True if stopping must end the player's forage session
	//bool						_StopEndsForageSession;
};


#endif // NL_FG_EXTRACTION_PHRASE_H

/* End of fg_extraction_phrase.h */
