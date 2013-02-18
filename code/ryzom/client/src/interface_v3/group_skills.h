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



#ifndef NL_GROUP_SKILLS_H
#define NL_GROUP_SKILLS_H

#include "nel/misc/types_nl.h"
#include "nel/gui/interface_group.h"
#include "nel/gui/group_tree.h"

// ***************************************************************************
/** A Group that display all skills by category in a job
 * \author Matthieu 'TrapII' Besson
 * \author Nevrax France
 * \date 2003
 */
class CGroupSkills : public CInterfaceGroup
{

public:

	CGroupSkills( const TCtorParam &param );
	virtual ~CGroupSkills();
	virtual bool parse (xmlNodePtr cur, CInterfaceGroup *parentGroup);
	virtual void checkCoords();
	virtual void clearGroups();

	static bool InhibitSkillUpFX;

// ******************
private:

	void	rebuild();
	void	createAllTreeNodes();

private:

	// observer to know that skills are modified
	struct CSkillsObs : public NLMISC::ICDBNode::IPropertyObserver
	{
		CGroupSkills *Owner;
		virtual void update (NLMISC::ICDBNode *node);
	} _SkillsObs;
	friend struct CSkillsObs;

private:

	/// Tell if we have to rebuild all the containers (only if new skill)
	bool		_MustRebuild;

	/// Template names for drawing a skill and a specialized skill
	std::string _TemplateSkill;

	/// AH for each ctrl node
	std::string	_AHCtrlNode;

	CGroupTree						*_Tree;
	CGroupTree::SNode::TRefPtr		_TreeRoot;
	std::vector<CGroupTree::SNode::TRefPtr>	_AllNodes;
};



#endif // NL_GROUP_SKILLS_H

/* End of group_skills.h */
