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



#ifndef NL_GROUP_PHRASE_SKILL_FILTER_H
#define NL_GROUP_PHRASE_SKILL_FILTER_H

#include "nel/misc/types_nl.h"
#include "game_share/brick_families.h"
#include "game_share/skills.h"
#include "nel/gui/interface_group.h"
#include "nel/gui/group_tree.h"


// ***************************************************************************
/** A Group that display all skills according to Brick usage
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2003
 */
class CGroupPhraseSkillFilter : public CInterfaceGroup
{
public:
	CGroupPhraseSkillFilter(const TCtorParam &param);

	virtual bool parse (xmlNodePtr cur, CInterfaceGroup *parentGroup);

	virtual void checkCoords();

	void touch()
	{
		_MustRebuild = true;
	}

private:

	void	rebuild();

private:

	// observer to know that brick family are modified
	struct CBrickFamilyObs : public NLMISC::ICDBNode::IPropertyObserver
	{
		CGroupPhraseSkillFilter			*Owner;
		BRICK_FAMILIES::TBrickFamily	BrickFamily;

		virtual void update (NLMISC::ICDBNode *node);
	};
	friend struct CBrickFamilyObs;
	CBrickFamilyObs		_BrickFamilyObs[BRICK_FAMILIES::NbFamilies];

private:

	/// Tell if we have to rebuild all the containers (only if new skill)
	bool			_MustRebuild;

	/// AH for each ctrl node
	std::string		_AHCtrlNode;

	CGroupTree		*_Tree;

	// true if a brick use this skill or one of his parent
	bool			_BrickSkillUsage[SKILLS::NUM_SKILLS];

	// bkupSkillOpenedStateRecurs
	void			bkupSkillOpenedStateRecurs(bool *skillOpened, CGroupTree::SNode *node);
};



#endif // NL_GROUP_PHRASE_SKILL_FILTER_H

/* End of group_phrase_skill_filter.h */
