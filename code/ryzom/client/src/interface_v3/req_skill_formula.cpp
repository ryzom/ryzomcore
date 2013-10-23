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

#include "stdpch.h"

#include "skill_manager.h"
#include "req_skill_formula.h"
#include "../string_manager_client.h"
#include "nel/misc/i18n.h"
#include "nel/misc/algo.h"


using namespace std;
using namespace NLMISC;

// ***************************************************************************
bool		CSkillValue::andWith(const CSkillValue &sv)
{
	CSkillManager	*pSM= CSkillManager::getInstance();

	// if the skills are equal
	if(Skill==sv.Skill)
	{
		// take the more restricitve sv
		Value= max(Value, sv.Value);
		return true;
	}
	// else if the skills are one same branch
	else if(pSM->areSkillOnSameBranch(Skill, sv.Skill))
	{
		// The skillvalue are compatible if the ancestor skill has a lesser value than the son
		if(pSM->isSkillAncestor(Skill, sv.Skill))
		{
			if(Value<=sv.Value)
			{
				// take the more restricitve sv
				*this= sv;
				return true;
			}
		}
		else
		{
			if(sv.Value<=Value)
			{
				// take the more restricitve sv (no change)
				return true;
			}
		}
	}

	// other cases
	return false;
}

// ***************************************************************************
bool		CSkillValue::orWith(const CSkillValue &sv)
{
	CSkillManager	*pSM= CSkillManager::getInstance();

	// if the skills are equal
	if(Skill==sv.Skill)
	{
		// take the less restricitve sv
		Value= min(Value, sv.Value);
		return true;
	}
	// else if the skills are one same branch
	else if(pSM->areSkillOnSameBranch(Skill, sv.Skill))
	{
		// The skillvalue are compatible if the ancestor skill has a lesser value than the son
		if(pSM->isSkillAncestor(Skill, sv.Skill))
		{
			if(Value<=sv.Value)
			{
				// take the less restricitve sv (no change)
				return true;
			}
		}
		else
		{
			if(sv.Value<=Value)
			{
				// take the less restricitve sv
				*this= sv;
				return true;
			}
		}
	}

	// other cases
	return false;
}

// ***************************************************************************
void		CReqSkillFormula::CSkillValueAnd::assign(const CSkillValue &sv)
{
	if(sv.Skill!=SKILLS::unknown)
	{
		AndSkills.clear();
		AndSkills.push_back(sv);
	}
}

// ***************************************************************************
void		CReqSkillFormula::CSkillValueAnd::andV(const CSkillValueAnd &a)
{
	// for each skill value of a, AND with each skill value of this. if only one is compatible,
	// don't append the and member!
	list<CSkillValue>::const_iterator	ita;
	list<CSkillValue>::iterator		itb;
	for(ita=a.AndSkills.begin();ita!=a.AndSkills.end();ita++)
	{
		bool	match= false;
		for(itb=AndSkills.begin();itb!=AndSkills.end();itb++)
		{
			// it success to regroup in itb skillValue, don't need to append
			match= match || itb->andWith(*ita);
		}
		// no skill value match? => append to the AND
		if(!match)
			AndSkills.push_back(*ita);
	}
}

// ***************************************************************************
void		CReqSkillFormula::assign(const CSkillValue &sv)
{
	if(sv.Skill!=SKILLS::unknown)
	{
		OrSkills.clear();
		OrSkills.push_back(CSkillValueAnd());
		OrSkills.back().assign(sv);
	}
}


// ***************************************************************************
void		CReqSkillFormula::andV(const CSkillValue &sv)
{
	// if empty, juste assign
	if(empty())
	{
		assign(sv);
	}

	// else AND with this skill
	CSkillValueAnd	svAnd;
	svAnd.assign(sv);

	// expand product
	list<CSkillValueAnd>::iterator	it;
	for(it= OrSkills.begin();it!=OrSkills.end();it++)
	{
		OrSkills.back().andV(svAnd);
	}

	// optimize
	optimizeOR();
}

// ***************************************************************************
void		CReqSkillFormula::orV(const CSkillValue &sv)
{
	CSkillValueAnd	svAnd;
	svAnd.assign(sv);
	OrSkills.push_back(svAnd);

	// optimize
	optimizeOR();
}

// ***************************************************************************
void		CReqSkillFormula::andV(const CReqSkillFormula &req)
{
	// if empty, juste assign
	if(empty())
	{
		*this= req;
	}

	// else AND with each andSkill
	CReqSkillFormula	temp;

	// expand product
	list<CSkillValueAnd>::const_iterator	ita;
	list<CSkillValueAnd>::const_iterator	itb;
	for(ita= req.OrSkills.begin();ita!=req.OrSkills.end();ita++)
	{
		for(itb= OrSkills.begin();itb!=OrSkills.end();itb++)
		{
			temp.OrSkills.push_back(CSkillValueAnd());
			temp.OrSkills.back()= *ita;
			temp.OrSkills.back().andV(*itb);
		}
	}

	// copy
	*this= temp;

	// then optimize
	optimizeOR();
}

// ***************************************************************************
void		CReqSkillFormula::orV(const CReqSkillFormula &req)
{
	// easy: append the ored in current
	list<CSkillValueAnd>::const_iterator	it;
	for(it= req.OrSkills.begin();it!=req.OrSkills.end();it++)
	{
		OrSkills.push_back(*it);
	}

	// then optimize
	optimizeOR();
}


// ***************************************************************************
void		CReqSkillFormula::optimizeOR()
{
	/*
	 *	There is multiple possible optimisation. eg:
	 *		SME10 & SF5 | SME10			=>  SME10
	 *		SME10 & SF5 | SME5			=>  SME5
	 *		SME10 & SF5 | SME20			=>  NOT OPTIMIZABLE
	 *		SME5  & SF5 | SME10 & SF10	=>  SME5  & SF5
	 *		... more?
	 *
	 *		For now, optimize only the 2 first case.
	 */

	list<CSkillValueAnd>::iterator	it;
	for(it= OrSkills.begin();it!=OrSkills.end();it++)
	{
		// if this skillValueAnd is a single SkillValue, may optimize with others
		if(it->AndSkills.size()==1)
		{
			const CSkillValue	&svComp= *it->AndSkills.begin();
			list<CSkillValueAnd>::iterator	itb;
			// run all other ORed skills (but me)
			for(itb=OrSkills.begin();itb!=OrSkills.end();)
			{
				if(itb!=it)
				{
					bool	remove= false;

					// if the OR with only one of the AndSkill result to comp
					list<CSkillValue>::iterator	itSv;
					for(itSv=itb->AndSkills.begin();itSv!=itb->AndSkills.end();itSv++)
					{
						CSkillValue		sv= *itSv;
						if(sv.orWith(svComp) && sv==svComp)
						{
							// then all this ORed skill can be removed!
							remove= true;
							break;
						}
					}

					// if can remove this ORed skill
					if(remove)
						itb= OrSkills.erase(itb);
					else
						itb++;
				}
				else
					itb++;
			}
		}
	}
}

// ***************************************************************************
uint		CReqSkillFormula::getMaxRequiredValue() const
{
	uint	maxVal= 0;

	list<CSkillValueAnd>::const_iterator	it;
	for(it= OrSkills.begin();it!=OrSkills.end();it++)
	{
		list<CSkillValue>::const_iterator	itSv;
		for(itSv=it->AndSkills.begin();itSv!=it->AndSkills.end();itSv++)
		{
			maxVal= max(maxVal, itSv->Value);
		}
	}

	return maxVal;
}

// ***************************************************************************
void		CReqSkillFormula::log(const char *prefix) const
{
	string	tmp;

	if(empty())
	{
		tmp= "EMPTY";
	}
	else
	{
		list<CSkillValueAnd>::const_iterator	it;
		bool	firstOr= true;
		for(it= OrSkills.begin();it!=OrSkills.end();it++)
		{
			if(firstOr)
				firstOr= false;
			else
				tmp+= "| ";

			bool	firstAnd= true;
			list<CSkillValue>::const_iterator	itSv;
			for(itSv=it->AndSkills.begin();itSv!=it->AndSkills.end();itSv++)
			{
				if(firstAnd)
					firstAnd= false;
				else
					tmp+= "& ";
				tmp+= SKILLS::toString(itSv->Skill);
				tmp+= NLMISC::toString(itSv->Value) + " ";
			}
		}
	}

	if(prefix)
		nlinfo("%s: %s", prefix, tmp.c_str());
	else
		nlinfo("??: %s", tmp.c_str());
}

// ***************************************************************************
void		CReqSkillFormula::getInfoText(ucstring &info) const
{
	info.clear();

	if(!empty())
	{
		list<CSkillValueAnd>::const_iterator	it;
		bool	firstOr= true;
		for(it= OrSkills.begin();it!=OrSkills.end();it++)
		{
			if(firstOr)
				firstOr= false;
			else
				info+= CI18N::get("uihelpPhraseRequirementOR");

			list<CSkillValue>::const_iterator	itSv;
			for(itSv=it->AndSkills.begin();itSv!=it->AndSkills.end();itSv++)
			{
				const CSkillValue	&sv= *itSv;
				// get the colored line if the skill don't reach the req level
				ucstring	line;
				if(!isSkillValueTrained(sv))
					line= CI18N::get("uihelpPhraseRequirementNotMetLine");
				else
					line= CI18N::get("uihelpPhraseRequirementLine");
				// replace approriated text
				strFindReplace(line, "%s", STRING_MANAGER::CStringManagerClient::getSkillLocalizedName(sv.Skill));
				strFindReplace(line, "%d", toString(sv.Value));
				info+= line;
			}
		}
	}
}

// ***************************************************************************
bool		CReqSkillFormula::evaluate() const
{
	if(empty())
		return true;

	// For all the Ored values
	std::list<CSkillValueAnd>::const_iterator
		itOr(OrSkills.begin()), endOr(OrSkills.end());

	// if only one OR succed, ok.
	bool	ok= false;
	for(;itOr!=endOr;itOr++)
	{
		// to succeed, all ands must be ok
		std::list<CSkillValue>::const_iterator
			itAnd(itOr->AndSkills.begin()), endAnd(itOr->AndSkills.end());
		ok= true;
		for(;itAnd!=endAnd;itAnd++)
		{
			const CSkillValue	&sv= *itAnd;
			// failed for this val?
			if(!isSkillValueTrained(sv))
			{
				ok= false;
				break;
			}
		}

		// if all ANDS pass, ok!
		if(ok)
			break;
	}

	return ok;
}


// ***************************************************************************
bool		CReqSkillFormula::isSkillValueTrained(const CSkillValue	&sv) const
{
	CSkillManager	*pSM= CSkillManager::getInstance();

	sint	reqValue= sv.Value;
	sint	currentValue= pSM->getBaseSkillValue(sv.Skill);
	sint	maxValue= pSM->getMaxSkillValue(sv.Skill);

	// if the current skill value is >= the required value, ok!
	if(currentValue >= reqValue)
		return true;
	else
	{
		// if the required value is <= MaxValue, then it's a fail,
		// cause the currentValue could reach this val (eg SM 15), but it actually doesn't
		if(reqValue <= maxValue)
			return false;
		// else if the sv is SM 90 for example, we have to test if one of sons is OK.
		else
		{
			// the skill must have reached the MAX, else no chance that sons succeed
			if(currentValue >= maxValue)
			{
				// recurs test for each sons
				const vector<SKILLS::ESkills>	&children= pSM->getChildren(sv.Skill);
				for(uint i=0;i<children.size();i++)
				{
					CSkillValue		sonSV;
					sonSV.Skill= children[i];
					sonSV.Value= reqValue;
					// if works with this branch ok!
					if(isSkillValueTrained(sonSV))
						return true;
				}
			}
		}
	}

	return false;
}

