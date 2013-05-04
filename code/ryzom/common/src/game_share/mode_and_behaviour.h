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



#ifndef MODE_AND_BEHAVIOUR_H
#define MODE_AND_BEHAVIOUR_H

#include "nel/misc/types_nl.h"
#include "base_types.h"

namespace MBEHAV
{

	// Mode
	enum EMode
	{
		// IMPORTANT : IF YOU MODIFY THIS ENUM DO NOT FORGET TO CHANGE stringToMode() TOO

		UNKNOWN_MODE = 0,

		NORMAL,
		COMBAT_FLOAT,
		COMBAT,

		SWIM,
		SIT,

		//---------- MOUNT --------//
		MOUNT_NORMAL,
		MOUNT_SWIM,
		//-------------------------//

		EAT,
		REST,
		ALERT,
		HUNGRY,

		//--------- DEAD ----------//
		// Do not ungroup the tree next behaviours
//		RESURECTED,
		DEATH,
		SWIM_DEATH,
//		PERMANENT_DEATH,
		//-------------------------//

		//-------TELEPORT----------//
//		TELEPORT,
		//-------------------------//

		NUMBER_OF_MODES
		// IMPORTANT : IF YOU MODIFY THIS ENUM DO NOT FORGET TO CHANGE stringToMode() TOO
	};


	/**
	 * get the right mode from the input string
	 * \param str the input string
	 * \return the EMode associated to this string (UNKNOWN_MODE if the string cannot be interpreted)
	 */
	EMode stringToMode(const std::string &str);

	/**
	 * get the right mode from the input string
	 * \param mode the EMode
	 * \return the mode as a string
	 */
	const std::string & modeToString(EMode mode);


	/**
	 * Mode structure
	 */
	struct TMode
	{
		union
		{
			/// Raw full mode version
			uint64		RawModeAndParam;

			// Structure with mode and param version
			struct
			{
				/// Mode identifier (from enum)
				sint32		Mode;

				/// Param union
				union
				{
					/// Raw full param version
					uint32		RawParam;

					/// Compressed pos param versiob
					struct
					{
						uint16		X16;
						uint16		Y16;
					}			Pos;

					/// Angle param version
					float		Theta;
				};
			};
		};

		/// Default constructor
		TMode() : RawModeAndParam(0) {}

		/// Constructor 1
		TMode( EMode mode, sint32 posX, sint32 posY )
		{
			Mode = mode;
			Pos.X16 = (uint16)(posX >> 4);
			Pos.Y16 = (uint16)(posY >> 4);
		}

		/// Constructor 2
		TMode( EMode mode, float theta )
		{
			Mode = mode;
			Theta = theta;
		}

		/// Constructor 3
		TMode( uint64 raw ) : RawModeAndParam(raw) {}

		/// Cast operator (uint64)
		inline	operator uint64() const
		{
			return RawModeAndParam;
		}

		/// Assignment operator
		TMode&	operator=( const TMode& src )
		{
			RawModeAndParam = src.RawModeAndParam;
			return *this;
		}

		/// Set mode and fill pos from mirror and compress (must be defined in every service calling it!)
		void	setModeAndPos( EMode mode, const TDataSetRow& entityIndex );

		/// Set mode and theta
		void	setModeAndTheta( EMode mode, float theta ) { Mode = mode; Theta = theta; }

		/// Comparison operator
		bool	operator==( const TMode& other )
		{
			return RawModeAndParam == other.RawModeAndParam;
		}

		/// Comparison operator
		bool	operator!=( const TMode& other )
		{
			return RawModeAndParam != other.RawModeAndParam;
		}

		/// toString()
		inline std::string toString() const
		{
			return modeToString( (EMode)Mode ); // TODO: param
		}

		inline void serial (NLMISC::IStream &f) throw (NLMISC::EStream)
		{
			f.serial( RawModeAndParam );
		}

		/// Return true if the entity is in a mount mode
		bool isMountMode() const { return (Mode == MOUNT_NORMAL) || (Mode == MOUNT_SWIM); }
	};


	// Behaviour
	enum EBehaviour
	{
		// IMPORTANT : IF YOU MODIFY THIS ENUM DO NOT FORGET TO CHANGE stringToBehaviour() TOO
		UNKNOWN_BEHAVIOUR = 0,
		IDLE,							// 1

		STUNNED,
		STUN_END,

		//--------- LOOT ----------//
		LOOT_INIT,
		LOOT_END,
		//-------------------------//
		//-------- FORAGE ---------//
		PROSPECTING,
		PROSPECTING_END,
		EXTRACTING,
		EXTRACTING_END,
		CARE,
		CARE_END,
		//-------------------------//
		//--------HARVEST ---------//
		HARVESTING,
		HARVESTING_END,
		//-------------------------//

		//--------- FABER ---------//
		FABER,
		FABER_END,
		REPAIR,
		REPAIR_END,
		REFINE,
		REFINE_END,

		//--------- TRAINING ---------//
		TRAINING,
		TRAINING_END,

		//---- SPELL CASTING ------//
	MAGIC_CASTING_BEHAVIOUR_BEGIN,
		CAST_OFF= MAGIC_CASTING_BEHAVIOUR_BEGIN,	// Offensive Cast
		CAST_CUR,					// Curative Cast
		CAST_MIX,					// Mixed Cast

		CAST_ACID,
		CAST_BLIND,
		CAST_COLD,
		CAST_ELEC,
		CAST_FEAR,
		CAST_FIRE,
		CAST_HEALHP,
		CAST_MAD,
		CAST_POISON,
		CAST_ROOT,
		CAST_ROT,
		CAST_SHOCK,
		CAST_SLEEP,
		CAST_SLOW,
		CAST_STUN,
	MAGIC_CASTING_BEHAVIOUR_END = CAST_STUN,

	MAGIC_END_CASTING_BEHAVIOUR_BEGIN,
		// Offensive Cast
		CAST_OFF_FAIL = MAGIC_END_CASTING_BEHAVIOUR_BEGIN,// Offensive Cast
		CAST_OFF_FUMBLE,				// Offensive Cast
		CAST_OFF_SUCCESS,				// Offensive Cast
		CAST_OFF_LINK,					// Offensive Cast
		// Curative Cast
		CAST_CUR_FAIL,					// Curative Cast
		CAST_CUR_FUMBLE,				// Curative Cast
		CAST_CUR_SUCCESS,				// Curative Cast
		CAST_CUR_LINK,					// Curative Cast
		// Mixed Cast
		CAST_MIX_FAIL,					// Mixed Cast
		CAST_MIX_FUMBLE,				// Mixed Cast
		CAST_MIX_SUCCESS,				// Mixed Cast
		CAST_MIX_LINK,					// Mixed Cast

		CAST_FAIL,
		CAST_SUCCESS,

	MAGIC_END_CASTING_BEHAVIOUR_END = CAST_SUCCESS,
		//-------------------------//

		//--------- COMBAT---------//
	COMBAT_BEHAVIOUR_BEGIN, // not a behaviour, just here to know if a behaviour is a combat one
		DEFAULT_ATTACK = COMBAT_BEHAVIOUR_BEGIN,
		POWERFUL_ATTACK,
		AREA_ATTACK,

		RANGE_ATTACK,
	COMBAT_BEHAVIOUR_END = RANGE_ATTACK,

	CREATURE_ATTACK_BEGIN,
		CREATURE_ATTACK_0 = CREATURE_ATTACK_BEGIN,
		CREATURE_ATTACK_1,
	CREATURE_ATTACK_END = CREATURE_ATTACK_1,

	EMOTE_BEGIN,						// 46
	EMOTE_END = EMOTE_BEGIN+150,


		// IMPORTANT : IF YOU MODIFY THIS ENUM DO NOT FORGET TO CHANGE stringToBehaviour() TOO
	NUMBER_OF_BEHAVIOURS
	};


	/// Packed behaviour code
	struct TBehaviour8
	{
		uint8	Behaviour8;

		/// Constructor
		TBehaviour8( EBehaviour b=UNKNOWN_BEHAVIOUR )
		{
			Behaviour8 = (uint8)b;
		}

		/// Assignment from packed behaviour code
		TBehaviour8& operator=( TBehaviour8 b8 )
		{
			Behaviour8 = b8;
			return (*this);
		}

		/// Assignment from behaviour enum
		TBehaviour8& operator=( EBehaviour b )
		{
			Behaviour8 = (uint8)b;
			return (*this);
		}

		/// Cons cast into behaviour enum
		operator EBehaviour () const
		{
			return (EBehaviour)Behaviour8;
		}
	};


	/**
	 * get the right behaviour from the input string
	 * \param str the input string
	 * \return the EBehaviour associated to this string (UNKNOWN_BEHAVIOUR if the string cannot be interpreted)
	 */
	EBehaviour stringToBehaviour(const std::string &str);


	/**
	 * get the right behaviour from the input string
	 * \param behav the EBehaviour
	 * \return str the input string
	 */
	const std::string & behaviourToString(EBehaviour behav);


	/**
	 * Structure for behaviour
	 * \author David Fleury
	 * \author Nevrax France
	 * \date 2002
	 */
	struct CBehaviour
	{
		/************************************************************************/
		/* total class size MUST be 64 bits (sizeof==8)                         */
		/************************************************************************/
		/************************************************************************/
		/* We have 24 bits of data per behaviour (16+8)                         */
		/************************************************************************/
		union
		{
			uint16 Data;

			struct
			{
				uint16	Time			: 4; // bits 3..6 of TGameCycle (mandatory for reissuing the behaviour)
				uint16	HitType		    : 3; // THitType
				uint16	ImpactIntensity : 3; // 1 - 5
				uint16  ActionDuration  : 2;
				uint16  Localisation    : 3; // 6 possible localisation (TBodyPart)
				uint16	KillingBlow		: 1; // 0 = false, 1 = killing blow (kill the main target)
				// 16 bits used
			} Combat;

			struct
			{
				uint16	Time			: 4; // bits 3..6 of TGameCycle (mandatory for reissuing the behaviour)
				// type d'arme
				uint16  WeaponType	    : 2; // from TRangeWeaponType
				uint16	ImpactIntensity : 3; // 1 - 5
				uint16  Localisation    : 3; // 6 possible localisation (TBodyPart). For generic weapons only
				uint16	Unused			: 4;
			} Range;

			struct
			{
				uint16	Time			: 4;	// bits 3..6 of TGameCycle (mandatory for reissuing the behaviour)
				uint16	SpellMode		: 2;	// MAGICFX::TSpellMode 0 = bomb, 1 = chain, 2 = spray
				uint16	SpellId			: 6;	// 0+ = index (MAGICFX::TMagicFx)
				uint16	SpellIntensity	: 3;	// 0=nothing, 1+ = intensity (up to 5)
				uint16	Resist			: 1;	// 1 = the target resisted
				// 16 bits used
			} Spell;

			struct
			{
				uint16	Time                 : 4; // bits 3..6 of TGameCycle (mandatory for reissuing the behaviour)
				uint16	Resist				 : 1; // 1 = the target resisted
				uint16	ImpactIntensity      : 3; // 1 - 5
				uint16	ActionDuration       : 2;
				uint16	Localisation         : 3; // 6 possible localisation (TBodyPart)
				uint16	MagicImpactIntensity : 3; // 1 - 5
				// 16 bits used
			} CreatureAttack;

			struct
			{
				uint16	Time			: 4; // bits 3..6 of TGameCycle (mandatory for reissuing the behaviour)
				uint16	Range			: 7;
				uint16	Angle			: 2;
				uint16	Level			: 3; // 0 - 4
				// 16 bits used
			} ForageProspection;

			struct
			{
				uint16	Time			: 4; // bits 3..6 of TGameCycle (mandatory for reissuing the behaviour)
				uint16	Level			: 4; // 1 - 15
				// 8 bits used
			} ForageExtraction;

			// IMPORTANT: If you add another member struct in the enum, reserve at the beginning:
			//	uint16	Time			: 3; // bits 3..6 of TGameCycle (mandatory for reissuing the behaviour)
			// It is set by PHRASE_UTILITIES::sendUpdateBehaviour().
		};

		union
		{
			uint8 Data2;

			struct
			{
				uint8	TargetMode		: 1; // 0 -> one target, 1->target list
				uint8	DamageType      : 2; // blunt/Slashing/Piercing
				// effectId ?
				// effectIntensity
				uint8	Unused			: 5;
			} Combat2;
			struct
			{
				uint8	SelfSpell		: 1; // 0 -> target list, 1-> self spell
				uint8	Unused			: 7;
			} Spell2;
			struct
			{
				uint8	TargetMode      : 1; // 0->target courant, 1->target list
				uint8	HitType         : 3; // THitType (critic etc..)
				uint8	DamageType      : 2; // blunt/Slashing/Piercing
				uint8	Unused			: 2;
			} CreatureAttack2;
		};

		TBehaviour8	Behaviour;

		sint16		DeltaHP;
		uint16		Unused; /// Keep it, used to make the class size = 64 bits (sizeof(CBehaviour) MUST return 8 (bytes))

		inline CBehaviour() : Data(0), Data2(0), Behaviour(UNKNOWN_BEHAVIOUR), DeltaHP(0), Unused(0) {}

		inline CBehaviour( EBehaviour behaviour )
		{
			Unused = 0;
			Data = 0;
			Data2 = 0;
			DeltaHP = 0;
			Behaviour = behaviour;
		}

		inline CBehaviour( EBehaviour behaviour, uint16 data1, uint8 data2 = 0 )
		{
			Unused = 0;
			Data = data1;
			Data2 = data2;
			DeltaHP = 0;
			Behaviour = behaviour;
		}

		inline CBehaviour( uint64 rawdata )
		{
			*this = rawdata;
		}

		/// Cast operator (uint64).
		inline operator uint64() const
		{
			/************************************************************************/
			/* Problem: Olivier use a (sint64*) cast on a CBehaviour*, so this operator isn't called ! :'( */
			/* must use Member declaration order to match 'brutal' cast             */
			/************************************************************************/
			//return (((uint64)(DeltaHP)) << 32) + (((uint64)(Data2)) << 24) + (((uint64)(Data)) << 8) + Behaviour;
			return (((uint64)(DeltaHP)) << 32) + (((uint64)(Behaviour)) << 24) + (((uint64)(Data2)) << 16) + Data;
		}

		inline CBehaviour& operator= ( uint64 raw )
		{
			/*
			DeltaHP = (sint16)(raw >> 32) ;
			Data2 = (uint8)(raw >> 24) ;
			Data = (uint16)(raw >> 8) ;
			Behaviour = TBehaviour8( (EBehaviour)(raw & 0xFF) );
			*/
			DeltaHP = (sint16)(raw >> 32) ;
			Behaviour = TBehaviour8( (EBehaviour)((raw >> 24)& 0xFF) );
			Data2 = uint8((raw >> 16) & 0xff);
			Data = uint16(raw & 0xffff);
			return *this;
		}

		inline CBehaviour& operator= ( EBehaviour behaviour )
		{
			DeltaHP = 0;
			Data = 0;
			Data2 = 0;
			Behaviour = behaviour;
			return *this;
		}

		inline CBehaviour&  operator = ( const CBehaviour& p )
		{ DeltaHP = p.DeltaHP; Behaviour = p.Behaviour; Data = p.Data; Data2 = p.Data2; return *this; }

		inline bool operator == ( const CBehaviour& p ) const
		{ return (DeltaHP == p.DeltaHP && Behaviour == (EBehaviour)p.Behaviour && Data == p.Data && Data2 == p.Data2); }

		inline bool operator != ( const CBehaviour& p ) const
		{ return (Behaviour != (EBehaviour)p.Behaviour || Data != p.Data || Data2 != p.Data2 || DeltaHP != p.DeltaHP); }

		inline void serial (NLMISC::IStream &f) throw (NLMISC::EStream)
		{
			if (f.isReading() )
			{
				uint64 data = 0;
				f.serial( data );
				*this = data;
			}
			else
			{
				uint64 data = (uint64)(*this);
				f.serial( data );
			}
		}


		/// methode toString()
		inline std::string toString() const
		{
			std::string text = behaviourToString( Behaviour ) + " " + NLMISC::toString(Data) + " "+NLMISC::toString(Data2)+" "+ NLMISC::toString(DeltaHP);
			return text;
		}

		/// Return 'true' if the behaviour is a magic one.
		inline bool isMagic() const  {return (Behaviour >= MAGIC_CASTING_BEHAVIOUR_BEGIN && Behaviour <= MAGIC_END_CASTING_BEHAVIOUR_END);}
		/// Return 'true' if the behaviour is a combat one.
		inline bool isCombat() const {return (Behaviour >= COMBAT_BEHAVIOUR_BEGIN        && Behaviour <= COMBAT_BEHAVIOUR_END);}
		/// Return 'true' if the behaviour is a creature related one
		inline bool isCreatureAttack() const {return (Behaviour >= CREATURE_ATTACK_BEGIN && Behaviour <= CREATURE_ATTACK_END);}
		/// Return 'true' if the behaviour is an emote one.
		inline bool isEmote() const  {return (Behaviour >= EMOTE_BEGIN                   && Behaviour <= EMOTE_END);}
	};
}; // MBEHAV //

/// method toString()
namespace NLMISC
{
	std::string toString ( const MBEHAV::CBehaviour &b);
};

#endif // MODE_AND_BEHAVIOUR_H

/* End of mode_and_behaviour.h */
