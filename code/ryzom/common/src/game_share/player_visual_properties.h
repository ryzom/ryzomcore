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



#ifndef RY_PLAYER_VISUAL_PROPERTIES_H
#define RY_PLAYER_VISUAL_PROPERTIES_H

#include "nel/misc/types_nl.h"


 /**
 * Visual properties A & B
 * \author Alain Saffray
 * \author Nevrax France
 * \date 2002
 */
struct SPropVisualA
{
	struct SPropSubData // 64 bits used
	{
		uint64	Sex				: 1;  // max: 2		current: 2
		uint64	JacketModel		: 8;  // max: 256	current: 93
		uint64	JacketColor		: 3;  // max: 8		current: 8
		uint64	TrouserModel	: 8;  // max: 256	current: 104
		uint64	TrouserColor	: 3;  // max: 8		current: 8
		uint64	WeaponRightHand	: 10; // max: 1024	current: 457
		uint64	WeaponLeftHand	: 8;  // max: 256	current: 63
		uint64	ArmModel		: 8;  // max: 256	current: 94
		uint64	ArmColor		: 3;  // max: 8		current: 8
		uint64	HatModel		: 9;  // max: 512	current: 192
		uint64	HatColor		: 3;  // max: 8		current: 8
	};

// old VPA before bit extension
// 	struct SPropSubData // 64 bits used
// 	{
// 		uint64	Sex				: 1; // max: 2		current: 2
// 		uint64	JacketModel		: 7; // max: 128	current: 93
// 		uint64	JacketColor		: 3; // max: 8		current: 8
// 		uint64	TrouserModel	: 7; // max: 128	current: 104
// 		uint64	TrouserColor	: 3; // max: 8		current: 8
// 		uint64	WeaponRightHand	: 9; // max: 512	current: 457
// 		uint64	WeaponLeftHand	: 6; // max: 64		current: 63
// 		uint64	ArmModel		: 7; // max: 128	current: 94
// 		uint64	ArmColor		: 3; // max: 8		current: 8
// 		uint64	HatModel		: 8; // max: 256	current: 192
// 		uint64	HatColor		: 3; // max: 8		current: 8
// 		uint64	RTrail			: 4;
// 		uint64	LTrail			: 3;
// 	};

	union
	{
		uint64			PropertyA;
		SPropSubData	PropertySubData;
	};

	SPropVisualA() { PropertyA = 0; }

	SPropVisualA( uint64 propertyA ) { PropertyA = propertyA; }

	uint64 get() const { return PropertyA; }
	void set(const uint64& val) { PropertyA=val; }

	SPropVisualA&  operator = ( const SPropVisualA& p ) { PropertyA = p.PropertyA; return *this; }

	bool operator == ( const SPropVisualA& p ) const { return PropertyA == p.PropertyA; }

	bool operator != ( const SPropVisualA& p ) const { return PropertyA != p.PropertyA; }

	void serial (NLMISC::IStream &f) throw (NLMISC::EStream)
	{
		f.serial( PropertyA );
	}

	std::string toString() const;
};

struct SPropVisualB
{
	struct SPropSubData // 47 bits used
	{
		uint64	Name				: 16;
		uint64	HandsModel			: 9; // max: 512	current: 90
		uint64	HandsColor			: 3; // max: 8		current: 8
		uint64	FeetModel			: 9; // max: 512	current: 94
		uint64	FeetColor			: 3; // max: 8		current: 8
		uint64	RTrail				: 4;
		uint64	LTrail				: 3;
	};

// old VPB before bit extension
// 	struct SPropSubData // 40 bits used
// 	{
// 		uint64	Name				: 16;
// 		uint64	HandsModel			: 9; // max: 512	current: 90
// 		uint64	HandsColor			: 3; // max: 8		current: 8
// 		uint64	FeetModel			: 9; // max: 512	current: 94
// 		uint64	FeetColor			: 3; // max: 8		current: 8
// 	};

	union
	{
		uint64			PropertyB;
		SPropSubData	PropertySubData;
	};

	SPropVisualB( uint64 propertyB ) { PropertyB = propertyB; }

	SPropVisualB() { PropertyB = 0; }

	uint64 get() const { return PropertyB; }
	void set(const uint64& val) { PropertyB=val; }

	SPropVisualB&  operator = ( const SPropVisualB& p ) { PropertyB = p.PropertyB; return *this; }

	bool operator == ( const SPropVisualB& p ) const { return PropertyB == p.PropertyB; }

	bool operator != ( const SPropVisualB& p ) const { return PropertyB != p.PropertyB; }

	void serial (NLMISC::IStream &f) throw (NLMISC::EStream)
	{
		f.serial( PropertyB );
	}

	std::string toString() const;
};

struct SPropVisualC
{
	struct SPropSubData // 54 bits used
	{
		uint64	MorphTarget1		: 3; // max: 8		current: 8
		uint64	MorphTarget2		: 3; // max: 8		current: 8
		uint64	MorphTarget3		: 3; // max: 8		current: 8
		uint64	MorphTarget4		: 3; // max: 8		current: 8
		uint64	MorphTarget5		: 3; // max: 8		current: 8
		uint64	MorphTarget6		: 3; // max: 8		current: 8
		uint64	MorphTarget7		: 3; // max: 8		current: 8
		uint64	MorphTarget8		: 3; // max: 8		current: 8
		uint64	EyesColor			: 3; // max: 8		current: 8
		uint64	Tattoo				: 7; // max: 128	current: 64
		uint64	CharacterHeight		: 4; // max: 16		current: 16
		uint64	TorsoWidth			: 4; // max: 16		current: 16
		uint64	ArmsWidth			: 4; // max: 16		current: 16
		uint64	LegsWidth			: 4; // max: 16		current: 16
		uint64	BreastSize			: 4; // max: 16		current: 16
	};

// old VPC before bit extension
// 	struct SPropSubData // 53 bits used
// 	{
// 		uint64	MorphTarget1		: 3; // max: 8		current: 8
// 		uint64	MorphTarget2		: 3; // max: 8		current: 8
// 		uint64	MorphTarget3		: 3; // max: 8		current: 8
// 		uint64	MorphTarget4		: 3; // max: 8		current: 8
// 		uint64	MorphTarget5		: 3; // max: 8		current: 8
// 		uint64	MorphTarget6		: 3; // max: 8		current: 8
// 		uint64	MorphTarget7		: 3; // max: 8		current: 8
// 		uint64	MorphTarget8		: 3; // max: 8		current: 8
// 		uint64	EyesColor			: 3; // max: 8		current: 8
// 		uint64	Tattoo				: 6; // max: 64		current: 64
// 		uint64	CharacterHeight		: 4; // max: 16		current: 16
// 		uint64	TorsoWidth			: 4; // max: 16		current: 16
// 		uint64	ArmsWidth			: 4; // max: 16		current: 16
// 		uint64	LegsWidth			: 4; // max: 16		current: 16
// 		uint64	BreastSize			: 4; // max: 16		current: 16
// 	};

	union
	{
		uint64			PropertyC;
		SPropSubData	PropertySubData;
	};

	uint64 get() const { return PropertyC; }
	void set(const uint64& val) { PropertyC=val; }

	SPropVisualC( uint64 propertyC ) { PropertyC = propertyC; }

	SPropVisualC() { PropertyC = 0; }

	SPropVisualC&  operator = ( const SPropVisualC& p ) { PropertyC = p.PropertyC; return *this; }

	bool operator == ( const SPropVisualC& p ) const { return PropertyC == p.PropertyC; }

	bool operator != ( const SPropVisualC& p ) const { return PropertyC != p.PropertyC; }

	void serial (NLMISC::IStream &f) throw (NLMISC::EStream)
	{
		f.serial( PropertyC );
	}

	// helper to get a morph target value
	uint getMorphTargetValue(uint index) const
	{
		switch(index)
		{
			case 0: return PropertySubData.MorphTarget1;
			case 1: return PropertySubData.MorphTarget2;
			case 2: return PropertySubData.MorphTarget3;
			case 3: return PropertySubData.MorphTarget4;
			case 4: return PropertySubData.MorphTarget5;
			case 5: return PropertySubData.MorphTarget6;
			case 6: return PropertySubData.MorphTarget7;
			default: nlassert(0); break;
		}
		return 0;
	}

	std::string toString() const;
};

/**
 * Describe the NPC Alternative Look Property.
 * \author PUZIN Guillaume
 * \author Nevrax France
 * \date 2003
 */
struct SAltLookProp
{
	struct SPropSubData // 62 bits used
	{
		uint64	ColorTop		: 3;	// Color for the Chest
		uint64	ColorBot		: 3;	// Color for the Legs
		uint64	WeaponRightHand	: 10;
		uint64	WeaponLeftHand	: 8;
		uint64	Hat				: 1;
		uint64	Seed			: 18;
		uint64	ColorHair		: 3;	// Color for the Hair or the Helm
		uint64	RTrail			: 4;
		uint64	LTrail			: 3;
		uint64	ColorGlove		: 3;	// Color for the Gloves
		uint64	ColorBoot		: 3;	// Color for the Boots
		uint64	ColorArm		: 3;	// Color for the Arms
	};

// old VPC before bit extension
// 	struct SPropSubData // 59 bits used
// 	{
// 		uint64	ColorTop		: 3;	// Color for the Chest
// 		uint64	ColorBot		: 3;	// Color for the Legs
// 		uint64	WeaponRightHand	: 9;
// 		uint64	WeaponLeftHand	: 6;
// 		uint64	Hat				: 1;
// 		uint64	Seed			: 18;
// 		uint64	ColorHair		: 3;	// Color for the Hair or the Helm
// 		uint64	RTrail			: 4;
// 		uint64	LTrail			: 3;
// 		uint64	ColorGlove		: 3;	// Color for the Gloves
// 		uint64	ColorBoot		: 3;	// Color for the Boots
// 		uint64	ColorArm		: 3;	// Color for the Arms
// 	};

	union
	{
		uint64			Summary;
		SPropSubData	Element;
	};

	/// \name Constructors
	//@{
	SAltLookProp()               {Summary = 0;}
	SAltLookProp(uint64 summary) {Summary = summary;}
	//@}

	/// \name Operators
	//@{
	SAltLookProp &operator =  (const SAltLookProp& p)       { Summary = p.Summary; return *this; }
	bool          operator == (const SAltLookProp& p) const { return Summary == p.Summary; }
	bool          operator != (const SAltLookProp& p) const { return Summary != p.Summary; }
	//@}

	// Serial
	void serial (NLMISC::IStream &f) throw (NLMISC::EStream)
	{
		f.serial( Summary );
	}

	std::string toString() const;
};

struct SAltLookProp2
{
	struct SPropSubData // 8 bits
	{
		uint64	Scale			: 8; // 8 bits for an integer percentage from 0% to 255%
	};

	union
	{
		uint64			Summary;
		SPropSubData	PropertySubData;
	};

	/// \name Constructors
	//@{
	SAltLookProp2()               {Summary = 0;}
	SAltLookProp2(uint64 summary) {Summary = summary;}
	//@}

	/// \name Operators
	//@{
	SAltLookProp2 &operator =  (const SAltLookProp2& p)       { Summary = p.Summary; return *this; }
	bool           operator == (const SAltLookProp2& p) const { return Summary == p.Summary; }
	bool           operator != (const SAltLookProp2& p) const { return Summary != p.Summary; }
	//@}

	// Serial
	void serial (NLMISC::IStream &f) throw (NLMISC::EStream)
	{
		f.serial( Summary );
	}

	std::string toString() const;
};


#endif // RY_PLAYER_VISUAL_PROPERTIES_H
/* End of player_visual_properties.h */
