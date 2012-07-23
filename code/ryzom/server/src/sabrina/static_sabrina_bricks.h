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




#ifndef RY_STATIC_SABRINA_BRICKS_H
#define RY_STATIC_SABRINA_BRICKS_H

// Nel Misc
#include "nel/misc/types_nl.h"
#include "nel/misc/sheet_id.h"
// Nel georges
#include "nel/georges/u_form.h"
// game share
#include "game_share/brick_types.h"
#include "game_share/brick_families.h"
#include "game_share/skills.h"
#include "game_share/people.h"
#include "game_share/action_nature.h"
#include "game_share/slot_equipment.h"
#include "game_share/item_family.h"
#include "game_share/rm_family.h"
#include "game_share/crafting_tool_type.h"

//
#include "static_sabrina_bricks.cpp.h"


//------------------------------------------------------------------------
// CPlayerSkillValue - the family used for sabrina bricks
//------------------------------------------------------------------------

class CPlayerSkillValue
{
public:

	/** Return true if the current skill is the same as other or more skilled
	 * Ex: (FM1S:160,  FM1SF:150) -> true
	 *     (FM1S:160,  FM1S:140) -> true
	 *     (FM1SF:160, FM1S:140) -> true
	 *     (FM1SF:160, FM1SM:140) -> false
	 */
	bool		isAsSkilledAs( const CPlayerSkillValue& other ) const
	{
		// make sure the skill value is high enough (otherwise no point going any further
		if ( Value < other.Value )
			return false;

		// calculate the length of the shorter of the 2 strings
		uint32 size= Code.size();
		if (other.Code.size()<size)
			size=other.Code.size();

		// check that the start of the longer string matches the shorter string
		return ( Code.substr( 0, size ) == other.Code.substr( 0, size ) );
	}

	/// Serial
	void		serial( NLMISC::IStream& s )
	{
		s.serial( Code );
		s.serial( Value );
	}

	std::string	Code;
	uint32		Value;
};


//------------------------------------------------------------------------
// CSabrinaBrickFamily - the family used for sabrina bricks
//------------------------------------------------------------------------

class CSabrinaBrickFamilyId
{
public:
	//--------------------------------------------------------------------
	// data storage type
	typedef uint16 TDataType;

	//--------------------------------------------------------------------
	// singleton stuff

	// adding new brick families to the manager
	static void setBrickFamilyName(uint32 idx, const std::string& name)
	{
		// this is a set of idiot tests designed to pickup bugs including use of 
		// (TDataType)~0u as an 'invalid' return value
		nlassert(TDataType(idx)==idx);
		nlassert(idx!=TDataType(~0u));
		nlassert(idx<10000);			// this is an arbitrarily large 'idiot test' number

		if (_FamilyNames.size()<=idx)
			_FamilyNames.resize(idx+1);
		_FamilyNames[idx]= name;
	}

	// lookup the name of a brick family
	static const std::string& getBrickFamilyName(uint32 idx)
	{
		nlassert(idx<_FamilyNames.size());
		return _FamilyNames[idx];
	}
	
	// lookup the family index value from its name - return	~0u if not found
	static uint16 getBrickFamilyFromName(const std::string& name)
	{
		// make sure the name is not empty - this would have to be a bug
		nlassert(!name.empty())

		// try to find the named family in the existing list
		for (uint32 i=0;i<_FamilyNames.size();++i)
			if (_FamilyNames[i]==name)
				return i;

		// family not found so return a 'not found' value
		return ~0u;
	}

	// test an integer value to make sure it's plausible as a family id
	static bool isValidFamilyId(uint32 idx)
	{
		return idx<_FamilyNames.size();
	}


	//--------------------------------------------------------------------
	// instantiated object stuff

	// default ctor
	CSabrinaBrickFamilyId()
	{
		_Idx=~0u;
	}
	
	// ctor from string
	CSabrinaBrickFamilyId(const std::string& familyName)
	{
		_Idx=getBrickFamilyFromName();
	}

	// copy ctor
	explicit CSabrinaBrickFamilyId(uint32 idx)
	{
		// make sure the value fits in our data representation
		nlassert(idx==TDataType(idx));
		// make sure the idx reffers to a valid family
		nlassert(idx==~0u || idx<_FamilyNames.size());
		// make sure the family realy exists
		nlassert(!_FamilyNames[idx].empty());

		_Idx=idx;
	}

	// convert to numeric equivalent
	operator uint32() 
	{
		return _Idx; 
	}


private:
	//--------------------------------------------------------------------
	// singleton data

	// the vector of family names
	static std::vector<std::string> _FamilyNames;


	//--------------------------------------------------------------------
	// instantiated object data

	TDataType _Idx;
};


/**
 * Crafting class for sabrina bricks (for bricks are crafting plan mandatory)
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
struct CSabrinaBrickCraftInfo
{
	struct TRawMaterial 
	{
		// Needed Rm family craft component
		ITEMFAMILY::EItemFamily	MpFamily;
		// Needed Rm craft part of item 
		RM_CRAFT_TYPE::TRMFType	MpType;
		// Needed Rm physical property
		RM_CRAFT_PROPERTY::TRMFProperty MpProperty;
		// Quantity of this Mp needed
		uint8	Quantity;
		
		void serial(class NLMISC::IStream &f)
		{
			f.serialEnum( MpFamily );
			f.serialEnum( MpType );
			f.serialEnum( MpProperty );
			f.serial( Quantity );
		}
	};
	
	CSabrinaBrickCraftInfo()
	{
		Skill = SKILLS::unknown;
		ToolType = TOOL_TYPE::Unknown;
		NbItemsPerUnit = 1;
	}
	
	void serial(class NLMISC::IStream &f)
	{
		f.serialEnum( Skill );
		f.serialEnum( ToolType );
		f.serial( DurabilityFactor );		
		f.serial( WeightFactor );
		f.serial( DMGFactor );
		f.serial( SlashingProtectionFactor );
		f.serial( BluntProtectionFactor );
		f.serial( PiercingProtectionFactor );
		f.serial( DodgeFactor );
		f.serial( ParryFactor );
		f.serial( SpeedFactor );
		f.serial( RangeFactor );
		f.serial( SapLoadFactor );
		
		f.serial( NbItemsPerUnit );
		
		if (f.isReading())
		{
			uint8 size;
			f.serial(size);
			NeededMps.resize(size);
			for (uint8 i = 0; i < size ; ++i)
			{
				f.serial( NeededMps[i]);
			}
		}
		else
		{
			uint8 size = NeededMps.size();
			f.serial(size);
			for (uint8 i = 0; i < size ; ++i)
			{
				f.serial( NeededMps[i] ); 
			}
		}		
	}
	
	/// skill used to create/repair/refine this item
	SKILLS::ESkills					Skill;
	TOOL_TYPE::TCraftingToolType	ToolType;
	
	// Craft Mp related factors
	float							DurabilityFactor;
	float							WeightFactor;
	float							DMGFactor;
	float							SlashingProtectionFactor;
	float							BluntProtectionFactor;
	float							PiercingProtectionFactor;
	float							DodgeFactor;
	float							ParryFactor;
	float							SpeedFactor;
	float							RangeFactor;
	float							SapLoadFactor;
	
	/// number of items built with a single 'pass' (eg. with 1 wood I can make 2 arrows, so i'll never make 1 arrow but at least 2)
	uint8							NbItemsPerUnit;	
	/// creation specific
	std::vector< TRawMaterial >		NeededMps;
};


//------------------------------------------------------------------------
// CStaticSabrinaBricks - static brick instance
//------------------------------------------------------------------------

class CStaticSabrinaBricks
{
public:	
	//--------------------------------------------------------------------
	// basic parameters

	// the brick sheet id
	NLMISC::CSheetId			SheetId;

	// the brick family and index in the family
	CSabrinaBrickFamilyId		Family;
	uint8						IndexInFamily;

	// The brick type (magic, craft, etc)
	BRICK_TYPE::EBrickType		Type;

	// skill linked with the brick
	SKILLS::ESkills				Skill;

	/// Cost (>0) or credit (<0) (sabrina)
	sint16						SabrinaValue;

	/// params 
	std::vector<TBrickParam::IId *> Params;

	/// params as strings - for ease of serialisation
	std::vector<std::string>	StringParams;


	//--------------------------------------------------------------------
	// grammer

	/// mandatory families
	std::set<BRICK_FAMILIES::TBrickFamily>	MandatoryFamilies;
	/// optional families
	std::set<BRICK_FAMILIES::TBrickFamily>	OptionalFamilies;
	/// credit families
	std::set<BRICK_FAMILIES::TBrickFamily>	CreditFamilies;
	/// forbidden families
	std::set<BRICK_FAMILIES::TBrickFamily>	ForbiddenFamilies;


	//--------------------------------------------------------------------
	// rules for role master apprenticeship

	/// Bricks required to learn the brick (AND: all of them are required)
	std::vector<NLMISC::CSheetId>			LearnRequiresBricks;

	/// Skills required to learn the brick (OR)
	std::vector<CPlayerSkillValue>			LearnRequiresOneOfSkills;

	/// skill points price
	uint32									SkillPointPrice;


	//--------------------------------------------------------------------
	// parameters shared by magic and fight (combat)

	/// range table used
	NLMISC::CSheetId						RangeTable;


	//--------------------------------------------------------------------
	// parameters for magic

	// nature of the action of the brick (offensive, curative, etc)
	ACTNATURE::EActionNature				Nature;


	//--------------------------------------------------------------------
	// parameters for fight (combat)

	/// localisation forced by this brick if any,a phrase cannot have more than one brick setting the localisation (unless they set the same one)
	SLOT_EQUIPMENT::TSlotEquipment			ForcedLocalisation;


	//--------------------------------------------------------------------
	// parameters for craft

	/// Crafting plan
	CSabrinaBrickCraftInfo*					CraftInfo;


	//--------------------------------------------------------------------
	/// Methods

	CStaticBrick();

	/// Destructor
	virtual ~CStaticBrick();
	
	/// Serialisation
	virtual void serial(class NLMISC::IStream &f);

	/// read georges sheet
	void readGeorges (const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId);

	// return the version of this class, increments this value when the content of this class has changed
	inline static uint getVersion () { return 8; }

	// read static bricks from given root
//	void readStaticBrick ( const NLGEORGES::UFormElm &root, const NLMISC::CSheetId &sheetId );
		
	// add params to brick
	void addParam(const std::string &paramStr);

	// Load crafting info
	void loadCraftInfo( const NLGEORGES::UFormElm &root, const NLMISC::CSheetId &sheetId );
};


//------------------------------------------------------------------------
// CStaticSabrinaBricks - singleton static brick manager
//------------------------------------------------------------------------

class CStaticSabrinaBricks
{
public:	
	// get sabrina brick
	static const CStaticSabrinaBrick* lookup( const NLMISC::CSheetId& sheetId );
		
	/// static returns the CStaticBrick object of specified family and pos in family
	static CStaticBrick * getBrick(uint16 familyId, uint16 indexInFamily)
	{
		std::map< std::pair<uint16, uint16>, CStaticBrick *>::const_iterator it = _Bricks.find( std::make_pair( familyId,indexInFamily) );
		if ( it != _Bricks.end() )
		{
			CStaticBrick *brick = (*it).second;
			return brick;
		}
		else
		{
			return NULL;
		}
	}

private:
	/// brick static infos
	static std::map< NLMISC::CSheetId,CStaticSabrinaBrick > _SBrickSheets;
};


//------------------------------------------------------------------------
#endif
