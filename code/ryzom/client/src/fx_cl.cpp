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

#include "fx_cl.h"
#include "fx_manager.h"
#include "ingame_database_manager.h"
#include "pacs_client.h"
#include "misc.h"
#include "debug_client.h"
#include "client_sheets/fx_sheet.h"

#include "nel/3d/u_driver.h"
#include "nel/3d/u_scene.h"
#include "nel/misc/path.h"

using namespace NL3D;
using namespace NLMISC;


////////////
// EXTERN //
////////////
extern UDriver			*Driver;
extern UScene			*Scene;

/*
 * Constructor
 */
CFxCL::CFxCL() : CEntityCL(), _FXSheet(NULL), _BadBuild(false)
{
	init();
}




/*
 * init :
 * Initialize the Object with this function for all constructors.
 */
void CFxCL::init()
{
	//_CrtCheckMemory();
	CEntityCL::init();
	//_CrtCheckMemory();
}


/*
 * Build the entity from a sheet.
 */
bool CFxCL::build( const CEntitySheet *sheet )
{
	//_CrtCheckMemory();

	// Get the FX filename and user params from the sheet (only the first one of PSList)
	_FXSheet = dynamic_cast<const CFXSheet*>(sheet);
	if ( (! _FXSheet) || (_FXSheet->PSList.empty()) )
	{
		_BadBuild = true;
		nlwarning( "Bad sheet %s for fx", sheet->Id.toString().c_str() );
		return false;
	}

	// Base class init
	initialize();
	Type = Entity;
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

	//_CrtCheckMemory();
	return true;
}


/*
 * setFx :
 * Initialize properties of the entity (according to the class).
 */
bool CFxCL::setFx( const std::string &fileName )
{
	//_CrtCheckMemory();
	// Delete the old _Instance if needed
	if(!_Instance.empty())
	{
		if ( Scene )
			Scene->deleteInstance(_Instance);
		_Instance = NULL;
	}

	// Create the FX object and insert it into the scene
	_Instance = Scene->createInstance( fileName );
	UParticleSystemInstance fxInst;
	fxInst.cast (_Instance);
	if (fxInst.empty())
	{
		Scene->deleteInstance( _Instance );
		_Instance = NULL;
		nlwarning( "FX file '%s' not found.", fileName.c_str()) ;
		return false;
	}
	//_CrtCheckMemory();
	return true;
}


/*
 * Update the item position.
 */
void CFxCL::updateVisualPropertyPos(const NLMISC::TGameCycle &/* gameCycle */, const sint64 &prop, const NLMISC::TGameCycle &/* pI */)
{
	//_CrtCheckMemory();
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

	// Set the primitive position and snap the entity to the ground.
	CVectorD requestedPos( (float)(prop)/1000.0f,
						   (float)(nodeY->getValue64())/1000.0f,
						   (float)(nodeZ->getValue64())/1000.0f );
	pacsPos( requestedPos );
	snapToGround();

	// Set normal mode and unflag _FirsPos to prevent the fx to be clipped out
	_Mode = MBEHAV::NORMAL;
	_First_Pos = false;

	if (_BadBuild) return;

	if (_Instance.empty())
	{
		if ( ! setFx( _FXSheet->PSList[0].PSName ) )
		{
			_BadBuild = true;
			return;
		}
		UParticleSystemInstance fxInst;
		fxInst.cast (_Instance);
		fxInst.setUserParam( 0, _FXSheet->PSList[0].Power[CFXSheet::CPSStruct::StandardIndex].UserParam0 );
		fxInst.setUserParam( 1, _FXSheet->PSList[0].Power[CFXSheet::CPSStruct::StandardIndex].UserParam1 );


	}

	// Change the instance position.
	if (!_Instance.empty())
	{
		CVector prevPos;
		_Instance.getPos(prevPos);
		_Instance.setPos( pos() );
		UParticleSystemInstance fxInst;
		fxInst.cast (_Instance);
		fxInst.getShapeAABBox( _SelectBox );
		_SelectBox.setCenter( pos() + _SelectBox.getCenter() );
		//nldebug( "SelectBox: %s %s", _SelectBox.getCenter().asString().c_str(), _SelectBox.getHalfSize().asString().c_str() );

		// Adjust the collision.
		if( _Primitive )
		{
			_Primitive->setRadius(std::min(std::max((_SelectBox.getHalfSize()).x, (_SelectBox.getHalfSize()).y), (float)(RYZOM_ENTITY_SIZE_MAX/2)));
			_Primitive->setHeight((_SelectBox.getHalfSize()).z);
		}

		if (_Instance.getPos() != prevPos)
		{
			// a move or init occured -> must update cluster system
			UGlobalPosition gPos;
			if( _Primitive )
			{
				_Primitive->getGlobalPosition(gPos, dynamicWI);
			}
			else
			{
				GR->retrievePosition(pos());
			}
			UInstanceGroup *clusterSystem = getCluster(gPos);
			_Instance.setClusterSystem(clusterSystem);
		}

	}


	//_CrtCheckMemory();
}


/*
 * Update the position of the entity after the motion.
 */
/*void CFxCL::updatePos( const NLMISC::TTime &time, CEntityCL *target )
{
	CEntityCL::updatePos( time, target );
}*/
/*
 * updateDisplay : get the entity position and set all visual stuff with it.
 */
/*void CFxCL::updateVisible( const NLMISC::TTime &time, CEntityCL *target )
{
	CEntityCL::updateVisible( time, target );
}*/


/*
 * Draw the selection Box.
 */
void CFxCL::drawBox()
{
	::drawBox(selectBox().getMin(), selectBox().getMax(), CRGBA(250,250,0));
}// drawBox //


/*
 * Destructor
 */
CFxCL::~CFxCL()
{
	//_CrtCheckMemory();
	if (_Instance.empty())
		return;

	// Stop emitters
	UParticleSystemInstance fxInst;
	fxInst.cast (_Instance);
	if (!fxInst.removeByID(NELID("STOP")) && !fxInst.removeByID(NELID("main")))
	{
		fxInst.activateEmitters( false );
	}

	// Delegate the clean removal of the fx to the fx manager
	FXMngr.fx2remove( fxInst );
	// The fx manager now has ownership on the fx, so set the pointer to NULL,
	// (otherwise it is delete in CentityCL dtor)
	_Instance = NULL;
}
