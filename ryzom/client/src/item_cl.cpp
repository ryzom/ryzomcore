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




/////////////
// INCLUDE //
/////////////
#include "stdpch.h"
// Misc.
#include "nel/misc/path.h"
// Interface 3D
#include "nel/3d/u_scene.h"
// Client Sheets
#include "client_sheets/item_sheet.h"
// Client
#include "item_cl.h"
#include "ingame_database_manager.h"
#include "pacs_client.h"
#include "misc.h"
#include "debug_client.h"


////////////
// USING  //
////////////
using namespace NL3D;
using namespace NLMISC;


////////////
// EXTERN //
////////////
extern UScene * Scene;


//-----------------------------------------------
// CItemCL :
//
//-----------------------------------------------
CItemCL::CItemCL() : CEntityCL()
{
}// CItemCL //

//-----------------------------------------------
// CItemCL :
//
//-----------------------------------------------
CItemCL::CItemCL(const std::string &fileName) : CEntityCL()
{
	initShape( fileName );
}// CItemCL //

//-----------------------------------------------
// CItemCL :
//
//-----------------------------------------------
CItemCL::~CItemCL()
{
}// CItemCL //

//-----------------------------------------------
// build :
// Build the entity from a sheet.
//-----------------------------------------------
bool CItemCL::build(const CEntitySheet *sheet )	// virtual
{
	// Cast the sheet in the right type.
	const CItemSheet *sh = dynamic_cast<const CItemSheet *>(sheet);
	if(sh==0)
	{
		nlwarning("Item:build: the sheet is not an item sheet -> entity not initialized.");
		return false;
	}
	// Get the DB Entry
	if(IngameDbMngr.getNodePtr())
	{
		CCDBNodeBranch *nodeRoot = dynamic_cast<CCDBNodeBranch *>(IngameDbMngr.getNodePtr()->getNode(0));
		if(nodeRoot)
		{
			_DBEntry = dynamic_cast<CCDBNodeBranch *>(nodeRoot->getNode(_Slot));
			if(_DBEntry == 0)
				pushDebugStr("Cannot get a pointer on the DB entry.");
		}
	}
	//
	initialize();

	initShape( sh->getShape() );
	// Entity Created.
	return true;
}// build //

//-----------------------------------------------
// initProperties :
// Initialize properties for an item.
//-----------------------------------------------
void CItemCL::initProperties()
{
	properties().liftable(true);
}// initProperties //

//-----------------------------------------------
// initShape :
//
//-----------------------------------------------
void CItemCL::initShape( const string &fileName )
{
	string filePath = CPath::lookup(fileName, false, false);
	if(!filePath.empty())
		_Instance = Scene->createInstance( filePath );
	else
		nlwarning("Item:initShape:%d: file '%s' not found.", _Slot, fileName.c_str());
}// initShape //

//-----------------------------------------------
// updateVisualProperty0 :
/// Update the item position.
//-----------------------------------------------
void CItemCL::updateVisualPropertyPos(const NLMISC::TGameCycle &/* gameCycle */, const sint64 &prop, const NLMISC::TGameCycle &/* pI */)
{
	// Check the DB entry (the warning is already done in the build method).
	if(_DBEntry == 0)
		return;
	// Get The property 'Y'.
	CCDBNodeLeaf *nodeY	= dynamic_cast<CCDBNodeLeaf *>(_DBEntry->getNode(CLFECOMMON::PROPERTY_POSY));
	if(nodeY == 0)
	{
		nlwarning("ITM:updtVPPos:%d: Cannot find the property 'PROPERTY_POSY(%d)'.", _Slot, CLFECOMMON::PROPERTY_POSY);
		return;
	}
	// Get The property 'Z'.
	CCDBNodeLeaf *nodeZ	= dynamic_cast<CCDBNodeLeaf *>(_DBEntry->getNode(CLFECOMMON::PROPERTY_POSZ));
	if(nodeZ == 0)
	{
		nlwarning("ITM:updtVPPos:%d: Cannot find the property 'PROPERTY_POSZ(%d)'.", _Slot, CLFECOMMON::PROPERTY_POSZ);
		return;
	}
	// Insert the primitive into the world.
	if(_Primitive)
		_Primitive->insertInWorldImage(dynamicWI);
	float x = (float)(prop)/1000.0f;
	float y = (float)(nodeY->getValue64())/1000.0f;
	float z = (float)(nodeZ->getValue64())/1000.0f;
	// Set the primitive position.
	pacsPos(CVectorD(x, y, z));
	// Snap the entity to the ground.
	snapToGround();
	// Change the instance position.
	if(!_Instance.empty())
	{
		_Instance.setPos(pos());
		_Instance.getShapeAABBox(_SelectBox);
		_SelectBox.setCenter(pos() + _SelectBox.getCenter());
		// Adjust the collision.
		if(_Primitive)
		{
			_Primitive->setRadius(std::min(std::max((_SelectBox.getHalfSize()).x, (_SelectBox.getHalfSize()).y), (float)(RYZOM_ENTITY_SIZE_MAX/2)));
			_Primitive->setHeight((_SelectBox.getHalfSize()).z);
		}
	}
}// updateVisualProperty0 //


//---------------------------------------------------
// drawBox :
// Draw the selection Box.
//---------------------------------------------------
void CItemCL::drawBox()	// virtual
{
	::drawBox(selectBox().getMin(), selectBox().getMax(), CRGBA(250,250,0));
}// drawBox //
