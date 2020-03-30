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

#ifndef RY_REQ_SKILL_FORMULA_H
#define RY_REQ_SKILL_FORMULA_H

#include "nel/misc/types_nl.h"
#include "game_share/skills.h"


// ***************************************************************************
/** A pair Required Skill / Required Level
 */
class CSkillValue
{
public:
	SKILLS::ESkills		Skill;
	uint				Value;

	CSkillValue()
	{
		Skill= SKILLS::unknown;
		Value= 0;
	}
	CSkillValue(SKILLS::ESkills	eSkill, uint value=0) : Skill(eSkill), Value(value) {}


	bool		operator==(const CSkillValue &sv) const
	{
		return Skill==sv.Skill && Value==sv.Value;
	}

	// true if can AND or OR
	bool		andWith(const CSkillValue &sv);
	bool		orWith(const CSkillValue &sv);
};


// ***************************************************************************
/**	Required Skill Algebra: eg (SF10&SM10) | SC10 means
 *	that something is possible only if the player has either SF and SM to level 10, or SC to level 10
 */
class CReqSkillFormula
{
public:
	// a list of all required Skill/Level
	class CSkillValueAnd
	{
	public:
		std::list<CSkillValue>	AndSkills;

		// assign to a single skillvalue. no op if unknown!
		void	assign(const CSkillValue &sv);

		// and with a. optimize where possible
		void	andV(const CSkillValueAnd &a);
	};

public:
	// the ORed list of req SkillValues
	std::list<CSkillValueAnd>		OrSkills;

	// assign to a single skillvalue. no op if unknown!
	void		assign(const CSkillValue &sv);

	// And operation with a SkillValue. NB: if this.empty, assign()
	void		andV(const CSkillValue &req);

	// Or operation with a SkillValue.
	void		orV(const CSkillValue &req);

	// And operation with another Req Skill Formula. NB: if this.empty, ope=
	void		andV(const CReqSkillFormula &req);

	// Or operation with another Req Skill Formula
	void		orV(const CReqSkillFormula &req);

	// empty formula?
	bool		empty() const {return OrSkills.empty();}

	// single value formula?
	bool		singleValue() const {return OrSkills.size()==1 && OrSkills.begin()->AndSkills.size()==1;}

	// get the max required skill value for all skillvalue (either OR and AND)
	uint		getMaxRequiredValue() const;

	// for debug
	void		log(const char *prefix) const;

	// For SPhrase Info
	void		getInfoText(ucstring &info) const;

	// return true if the requirement formula completes regarding the actual player state (through CSkillMananger). return true if empty()
	bool		evaluate() const;

private:
	// regroup SkillValueAnd together where possible
	void		optimizeOR();

	// true if the sv.Skill current base value is >= sv.Value
	bool		isSkillValueTrained(const CSkillValue	&sv) const;
};


#endif // RY_REQ_SKILL_FORMULA_H

/* End of req_skill_formula.h */
