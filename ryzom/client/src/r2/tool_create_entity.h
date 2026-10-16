// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
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

#ifndef R2_TOOL_CREATE_ENTITY_H
#define R2_TOOL_CREATE_ENTITY_H

#include "tool_choose_pos.h"
#include "nel/misc/vector.h"
#include "prim_render.h"
#include "../decal.h"
#include "auto_group.h"
#include "displayer_visual_entity.h"

class CEntity;
namespace NLGUI
{
	class CLuaObject;
}

namespace R2
{

/**
  * Tool to create an entity or an array of entities in the scene
  * A transparent version of the entity to be created is drawn under the mouse
  * at the creation position
  */
class CToolCreateEntity : public CToolChoosePos
{
public:
	NLMISC_DECLARE_CLASS(R2::CToolCreateEntity);
	CToolCreateEntity() { nlassert(0); }
	~CToolCreateEntity() NL_OVERRIDE;
	//
	CToolCreateEntity(uint ghostSlot, const std::string &paletteId, bool arrayMode);
	const std::string &getPaletteId() const { return _PaletteId; }
protected:
	// from CTool
	virtual void onActivate() NL_OVERRIDE;
	//
	virtual void updateBeforeRender() NL_OVERRIDE;
	virtual void updateAfterRender() NL_OVERRIDE;
	virtual bool onMouseLeftButtonClicked() NL_OVERRIDE;
	virtual bool onMouseRightButtonClicked() NL_OVERRIDE;
	// from CToolChoosePos
	virtual void commit(const NLMISC::CVector &createPosition, float createAngle) NL_OVERRIDE;
	virtual void updateInvalidCursorOnUI() NL_OVERRIDE;
	virtual bool stopAfterCommit() const NL_OVERRIDE;
	virtual void cancel() NL_OVERRIDE;
private:
	enum TCreateState
	{
		CreateSingle = 0,
		ChooseArrayOrigin,
		DrawArray
	};
private:
	TCreateState			 _CreateState;
	std::string				 _PaletteId;
	std::string				 _EntityCategory;
	CAutoGroup				 _AutoGroup;
	CVector					 _ArrayOrigin;
	CVector					 _ArrayEnd;
	float					 _ArrayDefaultAngle;
	bool					 _ValidArray;
	enum TArrayWantedAction
	{
		ArrayActionNone = 0, ArrayActionValidate, ArrayActionCancel
	};
	TArrayWantedAction		 _ArrayWantedAction;
	std::vector<CDisplayerVisualEntity::TRefPtr> _ArrayElements; // ghost elements forming the array before it is drawn

private:
	/** Create an entity in scenario (possibly a ghost one) from a CEntityCL
	  * \return InstanceId of the newly created entity.
	  */
	std::string cloneEntityIntoScenario(CEntityCL *clonee,
										const NLMISC::CVector &createPosition,
										float createAngle,
										bool  newAction,
										bool  createGhost
									   );
	// array management
	void updateAutoGroup();
	void updateArray(CEntityCL *clonee);
	void commitArray();
	void clearArray();


	bool isBotObjectSheet(const NLMISC::CSheetId &sheetId) const;
};




} // R2

#endif
