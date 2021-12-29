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

#ifndef R2_DISPLAYER_VISUAL_ACTIVITY_SEQUENCE_H
#define R2_DISPLAYER_VISUAL_ACTIVITY_SEQUENCE_H

#include "displayer_visual.h"
#include "../decal.h"
#include "editor.h"
#include "nel/misc/line.h"
//
#include "../interface_v3/group_map.h"


namespace NLMISC
{
	class CLine;
}

namespace R2
{

class CDisplayerVisualActivitySequence : public CDisplayerVisual, public CEditor::IInstanceObserver,
										 public CGroupMap::IDeco
{
public:
	NLMISC_DECLARE_CLASS(R2::CDisplayerVisualActivitySequence);
	CDisplayerVisualActivitySequence();
	~CDisplayerVisualActivitySequence();
	// from CDisplayerVisual
	virtual bool isSelectable() const { return false; }
	virtual void onAttrModified(const std::string &attrName, sint32 attrIndex);
	virtual void onPostRender();
	virtual void onPreRender();
	virtual void updateWorldPos();
	virtual void setActive(bool active);
	virtual bool getActive() const;
private:
	bool _AddedToWorldMap;
	bool _Touched;
	bool _Active;
	std::vector<CDecal::TSmartPtr>					_Decals;
	std::vector<CCtrlQuad *>						_WorldMapEdges;
	std::vector<CEditor::TInstanceObserverHandle>	_ObserverHandles;
	// need to know when one of the component world pos is changed (no notification message reach us if
	// word pos update is due to a parent)
	// TODO : add a 'world pos changed' event...
	struct CWorldPosCache
	{
		CDisplayerVisual::TRefPtr DV;
		NLMISC::CVector2f		  WorldPos2f;
	};
	std::vector<CWorldPosCache> _WPCache;
	std::vector<NLMISC::CLine> _FootSteps;
	NLMISC::CRGBA _DecalColor;
	// keep list of primitives traversed by this sequence
	// allows to know when to update us if visibility of one of the primitive has changed
	struct CTraversedPrimInfo
	{
		CDisplayerVisual::TRefPtr	   PrimDisplay;
		bool						   Visible;
	};
	std::vector<CTraversedPrimInfo> _TraversedPrimInfos;
protected:
	void touch();
	void update();
	void clear(bool wantRemoveFromWorldMap = true);
	void addFootSteps(const NLMISC::CLine &line);
	void addWanderSteps(const NLMISC::CVector &pos);
	void setWorldMapColor(NLMISC::CRGBA color);
	void setWorldMapNumEdges(uint count);
	CDisplayerVisual *getParentDV() const;
	// from CEditor::IInstanceObserver
	virtual void onInstanceCreated(CInstance &instance);
	virtual void onInstanceErased(CInstance &instance);
	virtual void onAttrModified(CInstance &instance, const std::string &attrName, sint32 attrIndex);
protected:
	// from CGroupMap::IDeco
	virtual void onAdd(CGroupMap &owner);
	virtual void onRemove(CGroupMap &owner);
	virtual void onPreRender(CGroupMap &owner);
	virtual void onUpdate(CGroupMap &owner);
	// return true if visible
	// if entiyDV == groupDV, then an ungrouped entity, else the group an its entity
	bool isVisible(CDisplayerVisual *groupDV, CDisplayerVisual *entityDV);
	// get visual displayer of parent group, or return the entity if not grouped
	CDisplayerVisual *getPossibleGroupDV(CDisplayerVisual *entityDV);
	//
	void removeFromWorldMap();
};

} // R2

#endif
