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



#ifndef RY_STATISTIC_H
#define RY_STATISTIC_H

#include "game_share/mirror_prop_value.h"


namespace STAT_TYPES
{
	enum TStatType
	{
		Score,
		Skill,
		Speed,
		Unknown,
	};
	const std::string & toString(TStatType type);
	TStatType	toStatType(const std::string &str);

};

/**
 * SCharacteristicsAndScores
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2002
 */
struct SCharacteristicsAndScores
{
	// Start by declaring methods for persistent load/ save operations
	// The following macro is defined in persistent_data.h
	// At time of writing it evaluated to:
	//	void store(CPersistentDataRecord &pdr) const;
	//	void apply(CPersistentDataRecord &pdr);

	DECLARE_PERSISTENCE_METHODS

	enum TCharacteristicsAndScoreSubType 
	{ 
		base = 0, 
		max, 
		modifier, 
		current, 
		base_regenerate_repos,
		base_regenerate_action,
		regenerate_modifier,
		current_regenerate,
	};

	sint32							Base;
	CMirrorPropValueAlice<sint32>	Max;
	sint32							OldMax;
	sint32							Modifier;
	CMirrorPropValueAlice<sint32>	Current;
	sint32							OldCurrent;

	float						BaseRegenerateRepos;
	float						BaseRegenerateAction;
	float						RegenerateModifier;
	float						CurrentRegenerate;
	NLMISC::TGameCycle			RegenerateTickUpdate;
	float						KeepRegenerateDecimal;
	
	// Serial
	void serial(NLMISC::IStream &f) throw(NLMISC::EStream);
	
	// Constructor
	SCharacteristicsAndScores();

	// clear
	void clear();

	// Destructor
	~SCharacteristicsAndScores();
};


/**
 * CPhysicalCharacteristics
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2002
 */
struct CPhysicalCharacteristics
{
	// Start by declaring methods for persistent load/ save operations
	// The following macro is defined in persistent_data.h
	// At time of writing it evaluated to:
	//	void store(CPersistentDataRecord &pdr) const;
	//	void apply(CPersistentDataRecord &pdr);

	DECLARE_PERSISTENCE_METHODS


	/**
	 *	Constructor
	 */
	CPhysicalCharacteristics();

	void clear();

	/**
	 * Serial
	 */
	void serial(NLMISC::IStream &f) throw(NLMISC::EStream);

	/**
	 * getCharacteristicStruct
	 */
	SCharacteristicsAndScores* getCharacteristicStruct( const std::string& characteristicName );


	std::vector< SCharacteristicsAndScores > _PhysicalCharacteristics;
};


/**
 * CPhysicalScores, derivated of CPhysicalCharacteristics
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2002
 */
struct CPhysicalScores
{
	// Start by declaring methods for persistent load/ save operations
	// The following macro is defined in persistent_data.h
	// At time of writing it evaluated to:
	//	void store(CPersistentDataRecord &pdr) const;
	//	void apply(CPersistentDataRecord &pdr);

	DECLARE_PERSISTENCE_METHODS

	/// speed
	sint32						SpeedVariationModifier;
	
	float						BaseWalkSpeed;
	float						BaseRunSpeed;
	CMirrorPropValueAlice< float, CPropLocationPacked<2> >	CurrentWalkSpeed;;
	CMirrorPropValueAlice< float, CPropLocationPacked<2> >	CurrentRunSpeed;

	/**
	 *	Default constructor
	 */
	CPhysicalScores();

	void clear();

	/**
	 * Serial
	 */
	void serial(NLMISC::IStream &f) throw(NLMISC::EStream);

	std::vector< SCharacteristicsAndScores > _PhysicalScores;
};


/**
 * SSkill properties and variables need for one skill
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2002
 */
struct SSkill
{
	// Start by declaring methods for persistent load/ save operations
	// The following macro is defined in persistent_data.h
	// At time of writing it evaluated to:
	//	void store(CPersistentDataRecord &pdr) const;
	//	void apply(CPersistentDataRecord &pdr);

	DECLARE_PERSISTENCE_METHODS

	enum ESkillSubType 
	{ 
		base = 0, 
		modifier,
		current,
		maxLvlReached,
		xp,
		xpNextLvl
	};

	sint32						Base;
	sint32						Modifier;
	sint32						OldCurrent;
	sint32						Current;

	sint32						MaxLvlReached;
	double						Xp;
	double						XpNextLvl;
		
	/**
	 * Serial
	 */
	void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
	{
		f.serial( Base );
		f.serial( Modifier );
		f.serial( Current );

		f.serial( MaxLvlReached );
		f.serial( Xp );
		f.serial( XpNextLvl );
	}

	void clear();

	// constructor
	SSkill();

	// destructor
	~SSkill();
};


/**
 * CSkills
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2002
 */
struct CSkills
{
	// Start by declaring methods for persistent load/ save operations
	// The following macro is defined in persistent_data.h
	// At time of writing it evaluated to:
	//	void store(CPersistentDataRecord &pdr) const;
	//	void apply(CPersistentDataRecord &pdr);

	DECLARE_PERSISTENCE_METHODS

	/**
	 *	Default constructor
	 */
	CSkills(bool noSkills);

	/**
	 * Serial
	 */
	void serial(NLMISC::IStream &f) throw(NLMISC::EStream);

	void clear();

	// return SSkill for skill name
	SSkill* getSkillStruct( const std::string& skillName );

	// return SSkill for skill name (const version)
	const SSkill* getSkillStruct( const std::string& skillName ) const;

	// return SSkill for skill enum
	SSkill* getSkillStruct( SKILLS::ESkills skill );
	
	// return SSkill for skill enum (const version)
	const SSkill* getSkillStruct( SKILLS::ESkills skill ) const;

	// return the max XP to gain in a skill to gain a level, param used to store the chosen skill
	double getMaxXPToGain(SKILLS::ESkills &skill);
	
	// Skills
	std::vector< SSkill > _Skills;

	// Skill Points (needed for purchase new actions and increase characteristics
	double					_Sp;
};

#endif // RY_STATISTIC_H
/* statistic.h */
