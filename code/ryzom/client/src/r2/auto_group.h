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

#ifndef R2_AUTO_GROUP_H
#define R2_AUTO_GROUP_H

#include "instance.h"
#include "prim_render.h"

namespace R2
{


class CAutoGroup
{
public:
	CAutoGroup();
	~CAutoGroup();
	// update new tesing position position for auto group & update the display (may disable auto grouping if 'valid' is false)
	void update(const NLMISC::CVector &testPos, const std::string &paletteEntry, bool valid);
	// compute current grouping candidate, based on the last info given to 'update'
	CInstance *getGroupingCandidate();
	/** Send the appropriate request to group the description of a new entity with the new grouping candidate
	  * The new grouping candidate is then reset (as if 'update' had been called with 'valid' == false)
	  * Caller should check that there's enough room in the scenario for that operation
	  * No-op if getGroupingCandidate() is false
	  */
	void group(CObject *newEntityDesc, const NLMISC::CVectorD &createPosition);
private:
	//CInstance::TRefPtr		 _GroupingCandidate;
	CPrimRender				 _AutoGroup; // display of auto group in scene / minimap
	struct CComponentSort
	{
		float Dist;
		CInstance *Instance;
		bool operator<(const CComponentSort &other) const { return Dist < other.Dist; }
	};
	std::vector<CComponentSort> _SortedComponents;
	std::vector<CVector>	 _PrimRenderVertices;
	//
	NLMISC::CVector			 _TestPos;		// testing pos for grouping candidate
	std::string				 _PaletteEntry; // palete entry used for group testing
	bool					 _AutoGroupEnabled;

private:
	void updateCandidateInstance(const CVector &testPos, const std::string &paletteEntry);
	CInstance *getGroupingCandidateInternal(std::vector<CComponentSort> &sortedComponents);
	void clear();
};


} // R2



#endif
