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



#ifndef RY_ITEM_FX_SHEET_H
#define RY_ITEM_FX_SHEET_H

namespace NLGEORGES
{
	class UFormElm;
}

/**
 * FX linked to an item
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2003
 */
class CItemFXSheet
{
public:
	float			    TrailMinSliceTime;	// slice time for shortest trail
	float			    TrailMaxSliceTime;  // slice time for longest trail
	NLMISC::CVector     AttackFXOffset;     // offset applied to the attack fx (some fxs are used for several kinds of guns, and need to be offseted differently)
	NLMISC::CVector		AttackFXRot;		// rotation around y axe for attack fxs (in degrees)
	float				ImpactFXDelay;
	// ctor
	CItemFXSheet();
	// build that an external script
	void build(const NLGEORGES::UFormElm &item, const std::string &prefix);
	void serial(NLMISC::IStream &f) throw(NLMISC::EStream);
	// access to string ids values
	const char *getTrail() const;
	const char *getAdvantageFX() const;
	const char *getAttackFX() const;
	// static fxs
	uint getNumStaticFX() const { return (uint)_StaticFXs.size(); }
	const char *getStaticFXName(uint index) const;
	const char *getStaticFXBone(uint index) const;
	const NLMISC::CVector &getStaticFXOffset(uint index) const;
private:
	class CStaticFX
	{
	public:
		NLMISC::TSStringId Name;
		NLMISC::TSStringId Bone;
		NLMISC::CVector	   Offset;
	public:
		CStaticFX() : Offset(NLMISC::CVector::Null)
		{
		}
		void build(const NLGEORGES::UFormElm &item);
		void serial(NLMISC::IStream &f) throw(NLMISC::EStream);
	};
	NLMISC::TSStringId  _Trail;			    // a trail that is displayed when the item is used to attack
	NLMISC::TSStringId	_AdvantageFX;		// fx to played on some items when the player master it
	NLMISC::TSStringId  _AttackFX;			// fx to trigger when the item is used to attack
	std::vector<CStaticFX> _StaticFXs;
};






#endif
