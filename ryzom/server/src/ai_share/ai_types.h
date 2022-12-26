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



#ifndef RYAI_TYPES_H
#define RYAI_TYPES_H

#include <string.h>

#include "nel/misc/common.h"
#include "nel/misc/string_mapper.h"
#include "nel/misc/algo.h"

//	Be carefull when implementing new types :
//	always create a badtype at the end of lists ( see others implementations as examples ).
//	don't forget to fill the .cpp part .. :)

bool	StrToBool	(bool &res, const	std::string	&str, bool defaultVal=false);


namespace AITYPES
{
	// energy value are stored in integer and coded as fixed point decimal.
	const uint32 ENERGY_SCALE = 100000;
	const uint32 ENERGY_LEVEL_0 = uint32(0.25f*ENERGY_SCALE);
	const uint32 ENERGY_LEVEL_1 = uint32(0.5f*ENERGY_SCALE);
	const uint32 ENERGY_LEVEL_2 = uint32(0.75f*ENERGY_SCALE);
	const uint32 ENERGY_LEVEL_3 = uint32(1.0f*ENERGY_SCALE);


	template	<class T>
	class	CDescType
	{
	public:
		class	CDescTypeEntry	
		{
		public:
			CDescTypeEntry(const char *name, T value)	: _name(name), _value(value)	{}
			const char	*_name;
			T		_value;
		};
		static	CDescTypeEntry	_entries[];
	};

	template	<class T>
	class	CTypeSpecialization	{	};


	template<class type>	const char *getName( type	t )
	{
		return CDescType<type>::_entries[t]._name;
	}		

	//	find the type associated with this string content.
	//	cando : upgrade with a logarithm search.
	template<class	type>	extern	type	getType(const char *s)
	{
		int		index	=	0;
		do 
		{
			if (NLMISC::nlstricmp(CDescType<type>::_entries[index]._name,s)==0	)
				return	CDescType<type>::_entries[index]._value;
			index++;
		} while	(CDescType<type>::_entries[index]._value != (type)CTypeSpecialization<type>::BadType);
		return	(type) CTypeSpecialization<type>::BadType;
	}

	//	to speed up writing code, test with the type of the first parameter.
	template<class	type>	extern	type	getTypeAs(const	type&	t,	const char *s)
	{
		return getType<type>(s);
	}
		
	//	to speed up writing code, get the result in the first param.
	template<class	type>	extern	void	getType(type&	t,	const char *s)
	{
		t =	getType<type>(s);
	}
	
////	TYPES DECLARATIONS ////


	//----------------------------------------------------------------------
	// types of ia
	
	enum TAIType {
		AITypeManager=0,
		AITypeNoGo,
		AITypeGrp,
//		AITypeGrpParameters,
		AITypePlace,
		AITypePlaceFauna,
		AITypeGrpFaunaPop,
		AITypeBot,
		AITypeNpcStateZone,
		AITypeNpcStateRoute,
		AITypePunctualState,
		AITypeNpcStateProfile,
		AITypeEvent,

		AITypeEventAction,
		AITypeFolder,
		AITypeState,
		AITypeKamiDeposit,
		AITypeKaravanState,		
		AITypeNpcStateChat,
		
		AITypeDynamicSystem,
		AITypeDynamicRegion,
		AITypeCellZone,
		AITypeCell,
		AITypeDynFaunaZone,
		AITypeDynNpcZonePlace,
		AITypeDynNpcZoneShape,
		AITypeDynRoad,
		AITypeRoadTrigger,
		AITypeGroupFamily,
		
		AITypeGroupTemplate,
		AITypeGroupTemplateMultiLevel,
		AITypeGroupTemplateFauna,
		AITypeGroupTemplateNpc,
	//	AITypeGroupTemplateGeneric, // to test
		AITypeGroupFamilyProfileFauna,
		AITypeGroupFamilyProfileTribe,
		AITypeGroupFamilyProfileNpc,
	//	AITypeGroupFamilyProfileGeneric, // to test
		
		AITypeGroupConfig,
		AITypeBotTemplate,
		AITypeBotTemplateMultiLevel,
		AITypeSquadTemplate,
		AITypeSquadTemplateVariant,
		AITypeSquadTemplateMember,
		AITypeOutpost,
		AITypeOutpostCharge,
		AITypeOutpostSquadFamily,
		AITypeOutpostSpawnZone,
		AITypeOutpostBuilding,
		AITypeOutpostManager,
		AITypeOutpostGroup,
		AITypeNogoPoint,
		AITypeNogoPointList,
		AITypeCellZoneEnergy,
		AITypeFaunaSpawnAtom,
		
		AITypeActionZone,
		AITypeFaunaActionZone,
		AITypeNpcActionZone,
		AITypeSafeZone,
		
		AITypeScript,
		
		AITypeSpire,
		AITypeUserModelList,
		AITypeUserModel,

		AITypeCustomLootTables,
		AITypeCustomLootTable,
		AITypeCustomLootSet,
		
		AITypeBadType
	};
	template	<>	class	CTypeSpecialization<TAIType>
	{	public:
		enum	{	BadType	=	AITypeBadType	};
	};
	

	//----------------------------------------------------------------------
	// specialization of ia types
	
	enum TAITypeSpec {
		AITypeSpecFauna=0,
		AITypeSpecNpc,
		AITypeSpecKami,
		AITypeSpecKaravan,
		AITypeSpecTribe,
		AITypeSpecNone,
		AITypeSpecBadType
	};	
	template	<>	class	CTypeSpecialization<TAITypeSpec>
	{	public:
	enum	{	BadType	=	AITypeSpecBadType	};
	};
	
	

	//----------------------------------------------------------------------
	// types of fauna
	
	enum TMgrType
	{
		MgrTypeFauna=0,
		MgrTypeKaravan,
		MgrTypeKami,
		MgrTypeNpc,
//		MgrTypeTribe,
		MgrTypePet,
		MgrTypeGuildNpc,
		MgrTypeOutpost,
		MgrTypeBadType
	};
	template	<>	class	CTypeSpecialization<TMgrType>
	{	public:
		enum	{	BadType	=	MgrTypeBadType	};
	};
	
		
	//----------------------------------------------------------------------
	// types of fauna
	
	enum TFaunaType
	{
		FaunaTypeHerbivore=0,
		FaunaTypePredator,
		FaunaTypePlant,
		FaunaTypeBadType
	};
	template	<>	class	CTypeSpecialization<TFaunaType>
	{	public:
		enum	{	BadType	=	FaunaTypeBadType	};
	};


	//----------------------------------------------------------------------
	// types of effects

	enum	TEffectType
	{
		EffectNegative=0,
		EffectPositive,
		EffectStun,
		EffectRoot,
		EffectBlind,
		EffectMaze,
		EffectSlow,
		
//		EffectSlowMagic,
//		EffectSlowMelee,
//		EffectSlowDepl,
//		EffectSlowRange,
//
//		EffectSkillMelee,
//		EffectSkillRange,
//		EffectSkillResistShock,
//		EffectSkillResistSlash,
//		EffectSkillResistAcid,
//		EffectSkillResistBlunt,
//		EffectSkillResistCold,
//		EffectSkillResistRot,
//		EffectSkillResistPierce,
//		EffectSkillResistPoison,
//		EffectSkillResistElec,
//		EffectSkillResistFire,
//
//		EffectCurseMazz,
//		EffectCurseRoot,
//		EffectCurseStun,
//
//		EffectCurseMadnessMagic,
//		EffectCurseMadnessMelee,
//		EffectCurseMadnessRange,
//		
//		EffectCurseDamageAmplMagic,
//		EffectCurseDamageAmplMelee,
//		EffectCurseDamageAmplRange,
//		EffectCurseCapRange,
//
//		EffectCurseBlind,
//		EffectCurseFear,
//		EffectCurseForbidItem,		//	disable active magic item use.
//		EffectCurseHatredHomin,
//		EffectCurseHatredFauna,
//		EffectCurseHatredFlora,
//		EffectCurseHatredKittins,
//		EffectCurseHatredDegeneres,
//		
//		EffectHeal,
//		EffectStamina,
//		EffectSap,
//
//		EffectDotShock,
//		EffectDotSlash,
//		EffectDotAcid,
//		EffectDotBlunt,
//		EffectDotCold,
//		EffectDotRot,
//		EffectDotPierce,
//		EffectDotPoison,
//		EffectDotElec,
//		EffectDotFire,
//
//		EffectCounterSpellDebufResist,
//		EffectCounterSpellDebufSkill,
//		EffectCounterSpellSlow,
//		EffectCounterDot,
//		EffectCounterHatred,
//		EffectCounterPhysical,		//	root/maze/stun.
//		EffectCounterMisfortune,	//	Blind/Fear/ForbidItem.
//		EffectCounterMental,		//	SelfDamage.
//		EffectCounterDamageAmpl,	//	Melee/Magic/Range
		
		EffectTypeBadType
	};

	template	<>	class	CTypeSpecialization<TEffectType>
	{	public:
	enum	{	BadType	=	EffectTypeBadType	};
	};
	
	//----------------------------------------------------------------------
	// types of spawn
	
	enum TSpawnType
	{
		SpawnTypeNever=0,
		SpawnTypeDay,
		SpawnTypeNight,
		SpawnTypeAlways,
		SpawnTypeBadType
	};
	template	<>	class	CTypeSpecialization<TSpawnType>
	{	public:
		enum	{	BadType	=	SpawnTypeBadType	};
	};
		
	
	//----------------------------------------------------------------------
	// types of fauna activity
	
	enum	TProfiles
	{
		BOT_FOLLOW_POS,
		BOT_STAND_AT_POS,
		BOT_FIGHT,
		BOT_HEAL,
		BOT_FLEE,
		BOT_RETURN_AFTER_FIGHT,
		BOT_GO_AWAY,
		BOT_MOVE_TO,
		BOT_FORAGE,
		
		MOVE_FOLLOW_ROUTE,
		MOVE_GOTO_POINT,
		MOVE_IDLE,
		MOVE_STAND_AT_POINT,
		MOVE_STAND_ON_VERTICES,
		MOVE_WAIT,
		MOVE_WANDER,
		
		ACTIVITY_NORMAL,
		ACTIVITY_GUARD,
		ACTIVITY_TRIBU,
		ACTIVITY_ESCORTED,
		ACTIVITY_BANDIT,
		ACTIVITY_GUARD_ESCORTED,
		
		FIGHT_NORMAL,

		ACTIVITY_WANDERING,
		ACTIVITY_GRAZING,
		ACTIVITY_RESTING,
		ACTIVITY_PLANTIDLE,
		ACTIVITY_CORPSE,
		ACTIVITY_EAT_CORPSE,
		ACTIVITY_CURIOSITY,
		ACTIVITY_CONTACT,
		ACTIVITY_HARVEST,
		ACTIVITY_FIGHT,
		ACTIVITY_FACTION,
		ACTIVITY_SQUAD,
		
		MOVE_DYN_FOLLOW_PATH,
		MOVE_CAMPING,
		ZONE_WAIT,

		PET_STAND,
		PET_FOLLOW,
		PET_GOTO,
		PET_GOTO_AND_DESPAWN,

		BAD_TYPE
	};
	
	template	<>	class	CTypeSpecialization<TProfiles>
	{	public:
		enum	{	BadType	=	BAD_TYPE	};
	};
	
//	enum	TZoneActivity
//	{
//		act_none				= 0,
//		act_fz_spawn			= 1<<0,
//		act_fz_food_herb		= 1<<1,
//		act_fz_food_carn		= 1<<2,
//		act_fz_rest_herb		= 1<<3,
//		act_fz_rest_carn		= 1<<4,
//
//		act_nz_harvest			= 1<<5,
//		act_nz_ambush			= 1<<6,
//		act_nz_rest				= 1<<7,
//		act_nz_outpost			= 1<<8,
//		act_nz_spawn			= 1<<9,
//		act_nz_outpost_def		= 1<<10,
//		act_nz_outpost_atk		= 1<<11,
//		act_nz_kami_wander		= 1<<12,
//		act_nz_escort			= 1<<13,
//		act_nz_convoy			= 1<<14,
//		act_nz_contact			= 1<<15,
//		act_nz_fight			= 1<<16,
//
//		act_nz_contact_camp		= 1<<17,
//		act_nz_contact_outpost	= 1<<18,
//		act_nz_contact_city		= 1<<19,
//
//		act_nz_fight_boss		= 1<<20,
//
//		act_fz_food_kitin		= 1<<21,
//		act_fz_food_degen		= 1<<22,
//		act_fz_plant			= 1<<23,
//		act_fz_rest_kitin		= 1<<24,
//		act_fz_rest_degen		= 1<<25,
//		act_fz_food_kitin_invasion	= 1<<26,
//		act_fz_rest_kitin_invasion	= 1<<27
//	};
//	const std::string &toString(TZoneActivity activity);

	/// The different kind of population we can found in the continent.
//	enum	TFamilyTag
//	{
//		family_fauna_herbivore,
//		family_fauna_carnivore,
//		family_flora,
//		family_civil,
//		family_bandit,
//		family_tribe,
//		family_kitin,
//		family_kitin_invasion,
//		family_kami,
//		family_karavan,
//		family_degen,
//		family_goo,
//		family_mp,
//		family_bad
//	};

	/// Population family identified by kind and tribe name (if kind of tribe)
//	struct TPopulationFamily
//	{
//		TFamilyTag			FamilyTag;
//		NLMISC::TStringId	TribeName;
//
//		TPopulationFamily(TFamilyTag familyTag = family_bad, NLMISC::TStringId tribeName = NLMISC::CStringMapper::emptyId())
//			: FamilyTag(familyTag), TribeName(tribeName)
//		{
//		}
//
//		TPopulationFamily(const std::string &familyName);
//
//		bool operator == (const TPopulationFamily &other) const
//		{
//			if	(FamilyTag != other.FamilyTag)
//				return	false;
//			if	(FamilyTag == family_tribe)
//				return	TribeName == other.TribeName;
//			return	true;
//		}
//
//		bool operator != (const TPopulationFamily &other) const
//		{
//			return !(*this == other);
//		}
//
//		bool operator < (const  TPopulationFamily &other) const
//		{
//			if	(FamilyTag < other.FamilyTag)
//				return	true;
//			if	(	FamilyTag==other.FamilyTag
//				&&	FamilyTag==family_tribe	)	//	useful ?
//				return	TribeName < other.TribeName;			
//			return	false;
//		}
//
//		const std::string &getFamilyName() const;
//
//		static const std::vector<std::pair<std::string, NLMISC::TStringId> > &getTribesNames();
//		static void getFamilyNames(std::vector<std::string> &result);
//		static const std::string &toString(TFamilyTag familyTag);
//
//	private:
//		static void init();
//		static std::vector<std::pair<std::string, NLMISC::TStringId> >		_TribeNames;
//	};


	enum	TVerticalPos
	{
		vp_auto,
		vp_upper,
		vp_middle,
		vp_lower
	};

	TVerticalPos		verticalPosFromString(const std::string &vpName);
	const std::string	&verticalPosToString(TVerticalPos vPos);

	class CPropertySet;

	class CPropertyId
	{
	public:
		CPropertyId();
		CPropertyId(std::string const& str);
		CPropertyId(CPropertyId const& other);
		bool operator<(CPropertyId const& other) const;
		bool operator==(CPropertyId const& other) const;
		
		CPropertySet operator+(CPropertyId const& other) const;
		
		bool empty() const;
		
		static CPropertyId create(std::string const& str);
		
		std::string toString() const;
		NLMISC::TStringId toStringId() const;
		
	private:
		CPropertyId(NLMISC::TStringId id);
		NLMISC::TStringId _Id;
	};
	
	class	TProperty
	{
	public:
		TProperty(CPropertyId const& property, size_t id);
		TProperty(TProperty const& property);
		
		bool operator< (TProperty const& property) const;
		bool operator==(TProperty const& property) const;
		bool operator!=(TProperty const& property) const;
		
		bool operator==(CPropertyId const& property) const;
		
		std::string toString() const;
		
		CPropertyId	_Property;
		//	used to keep track of possessor (help to remove)
		size_t _Id;
	};
	
	class	CPropertySet
	{
	public:
		CPropertySet(CPropertyId const& property);
		CPropertySet(std::string const& str);
		CPropertySet();
		typedef std::set<TProperty> TAdditionalPropertyList;
		
		bool containsAllOf(CPropertySet const& other) const;
		bool containsPartOfStrict(CPropertySet const& other) const;
		bool containsPartOfNotStrict(CPropertySet const& other) const;
		bool containsPartOfStrictFilter(std::string const& other) const;
		
		std::string toString() const;
		
		bool have(CPropertyId const& property) const;
		bool empty() const;
		
		void addProperty(CPropertyId const& property, size_t id=~0);
		void removeProperties(size_t id=~0);
		void merge(CPropertySet const& activities, size_t id=~0);
		
		CPropertySet& operator+(CPropertyId const& other);
		
		size_t size() const;

		bool operator==(CPropertySet const& other) const;
		bool operator<(CPropertySet const &other) const;
		std::set<NLMISC::TStringId> properties() const;
	protected:
		TAdditionalPropertyList::iterator begin();
		TAdditionalPropertyList::iterator end();
		TAdditionalPropertyList::const_iterator begin() const;
		TAdditionalPropertyList::const_iterator end() const;
		
	private:
		TAdditionalPropertyList	_Activities;
	};
	
	/**
	 * Derived version of CPropertySet with an additional templated list.
	 * For T, the type 'TProperty' is forbidden.
	 */
	template <class T>
	class CPropertySetWithExtraList : public CPropertySet
	{
	public:
		/// Add to the set of additional properties (silent if adding a property that is already present)
		void addExtraProperty(T const& property);
		/// Search in the set of additional properties only
		bool haveInExtra(T const& property) const;
		/// Return true if the additional set is empty
		bool extraSetEmpty() const;
		/// Clear the set of additional properties
		void clearExtra();
	private:
		std::set<T>	_ExtraList;
	};

	inline
	CPropertyId::CPropertyId()
	: _Id(0)
	{
	}
	inline
	CPropertyId::CPropertyId(std::string const& str)
	: _Id(NLMISC::CStringMapper::map(str))
	{
	}
	inline
	CPropertyId::CPropertyId(CPropertyId const& other)
	: _Id(other._Id)
	{
	}
	inline
	CPropertyId::CPropertyId(NLMISC::TStringId id)
	: _Id(id)
	{
	}
	inline
	bool CPropertyId::operator<(CPropertyId const& other) const
	{
		return _Id < other._Id;
	}
	inline
	bool CPropertyId::operator==(CPropertyId const& other) const
	{
		return _Id == other._Id;
	}
	inline
	CPropertySet CPropertyId::operator+(CPropertyId const& other) const
	{
		CPropertySet set(*this);
		set.addProperty(other);
		return set;
	}
	
	inline
	bool CPropertyId::empty() const
	{
		return _Id != 0;
	}
	
	inline
	CPropertyId CPropertyId::create(std::string const& str)
	{
		return NLMISC::CStringMapper::map(str);
	}

	inline
	std::string CPropertyId::toString() const
	{
		return NLMISC::CStringMapper::unmap(_Id);
	}
	inline
	NLMISC::TStringId CPropertyId::toStringId() const
	{
		return _Id;
	}

	inline
	TProperty::TProperty(CPropertyId const& property, size_t id)
	: _Property(property)
	, _Id(id)
	{
	}
	inline
	TProperty::TProperty(TProperty const& property)
	: _Property(property._Property)
	, _Id(property._Id)
	{
	}
			
	inline
	bool TProperty::operator<(TProperty const& property) const
	{
		return _Property<property._Property;
	}
	inline
	bool TProperty::operator==(TProperty const& property) const
	{
		return _Property==property._Property;
	}
	inline
	bool TProperty::operator!=(TProperty const& property) const
	{
		return !(_Property==property._Property);
	}
	inline
	bool TProperty::operator==(CPropertyId const& property) const
	{
		return _Property==property;
	}
	
	inline
	std::string	 TProperty::toString() const
	{
		return _Property.toString();
	}

	
	inline
	CPropertySet::CPropertySet(CPropertyId const& property)
	{
		addProperty(property);
	}
	inline
	CPropertySet::CPropertySet(std::string const& str)
	{
		addProperty(str);
	}
	inline
	CPropertySet::CPropertySet()
	{
	}
	
	inline
	bool CPropertySet::containsAllOf(CPropertySet const& other) const
	{
		if (other.empty())
			return true;
		return std::includes(_Activities.begin(), _Activities.end(), other.begin(), other.end());
	}
	
	inline
	bool CPropertySet::containsPartOfStrict(CPropertySet const& other) const
	{
		if (other.empty())
			return false;
		for (TAdditionalPropertyList::const_iterator it=other.begin(), itEnd=other.end(); it!=itEnd; ++it)
		{
			if (_Activities.find(*it)!=_Activities.end())
				return true;
		}
		return false;
	}
	
	inline
	bool CPropertySet::containsPartOfNotStrict(CPropertySet const& other) const
	{
		if (other.empty())
			return true;
		for (TAdditionalPropertyList::const_iterator it=other.begin(), itEnd=other.end(); it!=itEnd; ++it)
		{
			if (_Activities.find(*it)!=_Activities.end())
				return true;
		}
		return false;
	}
	
	inline
	bool CPropertySet::containsPartOfStrictFilter(std::string const& other) const
	{
		if (other.empty())
			return false;
		for (TAdditionalPropertyList::const_iterator it=_Activities.begin(), itEnd=_Activities.end(); it!=itEnd; ++it)
		{
			std::string activity = it->toString();
			if (NLMISC::testWildCard(activity, other))
				return true;
		}
		return false;
	}
	
	inline
	std::string CPropertySet::toString() const
	{
		std::string returnString;
		bool firstTime = true;
		for (TAdditionalPropertyList::const_iterator it=begin(), itEnd=end(); it!=itEnd; ++it)
		{
			if (!firstTime)
				returnString += ",";
			else
				firstTime = false;

			returnString += it->toString();
		}
		return returnString;
	}

	inline
	bool CPropertySet::have(CPropertyId const& property) const
	{
		const TProperty searchedProp( property, ~0 );
		return (_Activities.find( searchedProp ) != _Activities.end());
	}

	template <class T>
	inline bool CPropertySetWithExtraList<T>::haveInExtra(T const& property) const
	{
		return (_ExtraList.find( property ) != _ExtraList.end());
	}

	inline
	bool CPropertySet::empty() const
	{
		return _Activities.empty();
	}

	template <class T>
	inline bool CPropertySetWithExtraList<T>::extraSetEmpty() const
	{
		return _ExtraList.empty();
	}

	template <class T>
	inline void CPropertySetWithExtraList<T>::clearExtra()
	{
		_ExtraList.clear();
	}

	inline
	void CPropertySet::addProperty(CPropertyId const& property, size_t id)
	{
		_Activities.insert(TProperty(property,id));
	}
	
	template <class T>
	inline void CPropertySetWithExtraList<T>::addExtraProperty(T const& property)
	{
		_ExtraList.insert(property);
	}
	
	inline
	void CPropertySet::removeProperties(size_t id)
	{
		for	(TAdditionalPropertyList::iterator it=_Activities.begin(), itEnd=_Activities.end(); it!=itEnd; )
		{
			if (it->_Id==id)
			{
				TAdditionalPropertyList::iterator last = it;
				++it;
				_Activities.erase(last);
				continue;
			}
			++it;
		}
	}

	inline
	void CPropertySet::merge(CPropertySet const& activities, size_t id)
	{
		for (TAdditionalPropertyList::const_iterator it=activities.begin(), itEnd=activities.end(); it!=itEnd; ++it)
			_Activities.insert(TProperty(it->_Property,	id));
	}
	
	inline
	CPropertySet& CPropertySet::operator+(CPropertyId const& other)
	{
		addProperty(other);
		return *this;
	}
	
	inline
	size_t CPropertySet::size() const
	{
		return _Activities.size();
	}

	inline
	bool CPropertySet::operator==(CPropertySet const& other) const
	{
		return _Activities == other._Activities;
	}
	
	// utility for set/map storage only
	inline
	bool CPropertySet::operator<(CPropertySet const &other) const
	{
		if (_Activities.size() != other._Activities.size())
			return _Activities.size() < other._Activities.size();

		TAdditionalPropertyList::const_iterator i1(_Activities.begin()), i2(other._Activities.begin());
	
		while (i1 != _Activities.end() && i2 != other._Activities.end())
		{
			if (*i1 < *i2)
				return true;
			else if (*i1 != *i2)
				return false;
			++i1;
			++i2;
		}
		return false;
	}
	inline
	std::set<NLMISC::TStringId> CPropertySet::properties() const
	{
		TAdditionalPropertyList::const_iterator it, end=this->end();
		std::set<NLMISC::TStringId> ret;
		for (it=begin(); it!=end; ++it)
		{
			ret.insert(it->_Property.toStringId());
		}
		return ret;
	}
	inline
	CPropertySet::TAdditionalPropertyList::iterator CPropertySet::begin()
	{
		return _Activities.begin();
	}
	
	inline
	CPropertySet::TAdditionalPropertyList::iterator CPropertySet::end()
	{
		return _Activities.end();
	}

	inline
	CPropertySet::TAdditionalPropertyList::const_iterator CPropertySet::begin() const
	{
		return _Activities.begin();
	}
	
	inline
	CPropertySet::TAdditionalPropertyList::const_iterator CPropertySet::end() const
	{
		return _Activities.end();
	}
}
#endif

