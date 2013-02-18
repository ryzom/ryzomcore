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
#include "interface_v3/interface_manager.h"
#include "forage_source_cl.h"
#include "ingame_database_manager.h"
#include "debug_client.h"
#include "game_share/rm_family.h"
#include "client_sheets/forage_source_sheet.h"
#include "interface_v3/group_in_scene_user_info.h"
#include "fx_manager.h"
#include "string_manager_client.h"
#include "sheet_manager.h"
#include "entities.h"
#include "view.h"
#include "time_client.h"
#include "game_share/constants.h"
#include <nel/misc/i18n.h>

using namespace NL3D;
using namespace NLMISC;

extern class CView View;
extern UCamera MainCam;

const uint8 BarNotInit = 255;

CRGBA CForageSourceCL::SafeSourceColor(64, 192, 255);

/*
 * Constructor
 */
CForageSourceCL::CForageSourceCL() :
	CFxCL(),
	_InSceneUserInterface( NULL ),
	_IconFilename( NULL ),
	_IsExtractionInProgress( false ),
	_SafeSource( false ),
	_KnowledgeLevel( 0 ),
	_LastExplosionSwitch( 0 ),
	_ProspectorSlot( 255 ),
	_ExtraTime(0),
	_InclBonusExtraTime(0),
	_InitialQuantity(BarNotInit),
	_CurrentQuantity(0)
{
	for ( uint i=0; i!=NbFSBarIndices-1; ++i )
	{
		_BarDestValues[i] = BarNotInit;
		_BarCurrentValues[i] = BarNotInit;
	}
	_BarDestValues[NbFSBarIndices-1] = BarNotInit;

	// init to 0 per default (first frames...)
	_TimeBar= 0;
	_QuantityBar= 0;
	_DBar= 0;
	_EBar= 0;
}


/*
 * Initialize properties of the entity (according to the class).
 */
void CForageSourceCL::initProperties()
{
	CFxCL::initProperties();

	properties().selectable( true );
}



/*
 * Build the entity from a sheet.
 */
bool CForageSourceCL::build( const CEntitySheet *sheet )
{
	//_CrtCheckMemory();

	// Get FX filename and info from the sheet
	const CForageSourceSheet *forageSourceSheet = dynamic_cast<const CForageSourceSheet*>(sheet);
	if ( ! forageSourceSheet )
	{
		nlwarning( "Bad sheet %s for forage source", sheet->Id.toString().c_str() );
		return false;
	}
	if ( ! setFx( forageSourceSheet->FxFilename ) )
		return false;
	_KnowledgeLevel = forageSourceSheet->Knowledge;
	if ( _KnowledgeLevel != 0 )
		_KnowledgeLevel |= 0x80; // we don't know the group or family yet (visual FX not received)

	// Base class init
	initialize();
	Type = ForageSource;
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

	// Set default name (with no knowledge)
	_EntityName = CI18N::get( "mpSource" );

	// Build hud interface
	buildInSceneInterface();

	// Init user params
	UParticleSystemInstance fxInst;
	fxInst.cast (_Instance);
	nlassert(!fxInst.empty());
	fxInst.setUserParam( 0, 0.0f ); // inversed opacity
	fxInst.setUserParam( 1, 0.0f ); // glow
	fxInst.setUserParam( 2, 0.0f ); // particle quantity

	//_CrtCheckMemory();
	return true;
}


void CForageSourceCL::resetVP()
{
	// Init user params
	UParticleSystemInstance fxInst;
	fxInst.cast (_Instance);
	nlassert(!fxInst.empty());
	fxInst.setUserParam( 0, 0.0f ); // inversed opacity
	fxInst.setUserParam( 1, 0.0f ); // glow
	fxInst.setUserParam( 2, 0.0f ); // particle quantity

	nlinfo( "FG: Source: %s FX: %s", _Position.asVector().asString().c_str(), fxInst.getPos().toString().c_str() );
}


/*
 * Rebuild in scene interfaces
 */
void CForageSourceCL::buildInSceneInterface()
{
	// Delete previous interface
	releaseInSceneInterfaces();

	_InSceneUserInterface = CGroupInSceneUserInfo::build( this );
	_InSceneUserInterface->setUserScale( true );

	// parent
	CFxCL::buildInSceneInterface();
}


/*
 * Destroy inscene interfaces
 */
void CForageSourceCL::releaseInSceneInterfaces()
{
	if (_InSceneUserInterface)
	{
		CWidgetManager::getInstance()->unMakeWindow(_InSceneUserInterface);
		if (_InSceneUserInterface->getParent())
		{
			_InSceneUserInterface->getParent()->delGroup(_InSceneUserInterface);
		}
		else
		{
			delete _InSceneUserInterface;
		}

		_InSceneUserInterface = NULL;
	}
}


/*
 * Called when clipped out
 */
void CForageSourceCL::updateClipped (const NLMISC::TTime &currentTimeInMs, CEntityCL *target)
{
	// hide the scene interface
	if (_InSceneUserInterface)
	{
		if (_InSceneUserInterface->getActive())
			_InSceneUserInterface->setActive (false);
	}

	// parent
	CFxCL::updateClipped(currentTimeInMs, target);
}


/*
 * Method called each frame to manage the entity after the clipping test if the primitive is visible.
 */
void CForageSourceCL::updateVisible(const NLMISC::TTime &time, CEntityCL *target)
{
	// Update Modifiers
	if(!_HPModifiers.empty())
	{
		HPMD mod;
		mod.CHPModifier::operator= (*_HPModifiers.begin());
		mod.Time = TimeInSec;
		_HPDisplayed.push_back(mod);
		_HPModifiers.erase(_HPModifiers.begin());
	}

	// parent
	CFxCL::updateVisible(time, target);
}


/*
 * Helper for updateVisualPropertyBars()
 */
inline void setBarValue( float& currentValue, uint8& displayedValue, uint8& newValue )
{
	displayedValue = newValue;
	currentValue = (float)displayedValue;
}


/*
 * Helper for updateVisiblePostPos()
 */
inline void updateBarValueTime( uint8& destValue, float& currentValue, uint8& displayedValue )
{
	sint8 diff = (sint8)(destValue - displayedValue);
	if ( diff < 0 )
	{
		currentValue = std::max( (float)destValue, currentValue - DeltaTimeBarPerSec*DT );
		displayedValue = (uint8)currentValue;
	}
	else if ( diff > 0 )
	{
		currentValue = (float)destValue;
		displayedValue = destValue;
	}
}


/*
 * Helper for updateVisiblePostPos()
 */
inline void updateBarValue( uint8& destValue, float& currentValue, uint8& displayedValue )
{
	sint8 diff = (sint8)(destValue - displayedValue);
	if ( diff < 0 )
	{
		currentValue = std::max( (float)destValue, currentValue - DeltaMoveBarPerSec*DT );
		displayedValue = (uint8)currentValue;
	}
	else if ( diff > 0 )
	{
		float speed = (destValue == 127) ? DeltaResetBarPerSec : DeltaMoveBarPerSec;
		currentValue = std::min( (float)destValue, currentValue + speed*DT );
		displayedValue = (uint8)currentValue;
	}
}


/*
 * Update the entity after all positions done.
 */
void CForageSourceCL::updateVisiblePostPos(const NLMISC::TTime &time, CEntityCL *target)
{
	// Update in scene interface
	if( _InSceneUserInterface )
	{
		float dist = (_Position.asVector() - View.currentViewPos()/*UserEntity->pos()*/).norm();

		// Draw the interface is bars received
		bool showIS = (dist < (CLFECOMMON::THRESHOLD_BARS/1000 - 5.0f) ); //mustShowInsceneInterface( true );

		// Activate
		if ( _InSceneUserInterface->getActive() != showIS )
			_InSceneUserInterface->setActive( showIS );

		if ( showIS )
		{
			// Scale it
			const float ClampDist = 8.0f; // > 0
			if ( dist < ClampDist )
				_InSceneUserInterface->Scale = 1.0f;
			else
				_InSceneUserInterface->Scale = ClampDist / dist;

			// Update dynamic data
			_InSceneUserInterface->updateDynamicData();

			// Update position
			NLMISC::CVectorD pos;
			pos = (box().getMin() + box().getMax())/2;
			pos.z = box().getMax().z; // * 0.7f; // not as high as the top of the box
			nlassert(isValidDouble(pos.x) && isValidDouble(pos.y) && isValidDouble(pos.z));
			_InSceneUserInterface->Position = pos;
		}
	}

	// Update bar delayed movement
	updateBarValueTime( _BarDestValues[FSBTime],    _BarCurrentValues[FSBTime],    _TimeBar );     // Time (slower transition)
	updateBarValue(     _BarDestValues[FSBQuantiy], _BarCurrentValues[FSBQuantiy], _QuantityBar ); // Qtty
	updateBarValue(     _BarDestValues[FSBD],       _BarCurrentValues[FSBD],       _DBar );        // D
	updateBarValue(     _BarDestValues[FSBE],       _BarCurrentValues[FSBE],       _EBar );        // E

	// Parent
	CFxCL::updateVisiblePostPos(time, target);
}


void CForageSourceCL::displayInscenePos()
{
	nlinfo( "FG: Source: %s Inscene: %s % slot: %u", _Position.asVector().asString().c_str(), _InSceneUserInterface->Position.asString().c_str(), (slot() == UserEntity->selection()) ? "SELECTED":"Unselected", slot() );
}


/*
 * Update Entity Visual Property B
 */
/*void CForageSourceCL::updateVisualPropertyVpb(const NLMISC::TGameCycle &gameCycle, const sint64 &prop)
{

}*/


bool ForageSourceUseUP;
float ForageSourceUP0;
float ForageSourceUP1;
float ForageSourceUP2;
float ForageSourceUP3;
NLMISC_VARIABLE( bool, ForageSourceUseUP, "Use debug user param" );
NLMISC_VARIABLE( float, ForageSourceUP0, "" );
NLMISC_VARIABLE( float, ForageSourceUP1, "" );
NLMISC_VARIABLE( float, ForageSourceUP2, "" );
NLMISC_VARIABLE( float, ForageSourceUP3, "" );


/*
 * Update Entity Bars
 */
void CForageSourceCL::updateVisualPropertyBars(const NLMISC::TGameCycle &/* gameCycle */, const sint64 &prop)
{
	// NB: forage don't use CBarManager for 2 reasons: useless (forage bars exist only through VP),
	// and complicated since updated at each frame on client (because of smooth transition code below)

	bool setBarsNow = (_BarDestValues[0] == BarNotInit);
	_BarDestValues[FSBTime] = (uint8)(prop&0x7f);		  // Time to live
	_BarDestValues[FSBD]    = (uint8)((prop>>14)&0x7f); // D
	_BarDestValues[FSBE]    = (uint8)((prop>>21)&0x7f); // E
	// Deal with safe sources
	if (_BarDestValues[FSBE]==0) // Safe source
	{
		_BarDestValues[FSBE] = 127;
		/* // This is disabled coz setFx don't work after entity creation (:TODO: try to see why)
		if (!_SafeSource)
		{
			CEntitySheet *entitySheet = SheetMngr.get(sheetId());
			const CForageSourceSheet *forageSourceSheet = dynamic_cast<const CForageSourceSheet*>(entitySheet);
			if ( forageSourceSheet )
			{
				if ( ! setFx( forageSourceSheet->FxSafeFilename ) )
					setFx( forageSourceSheet->FxFilename );
			}
		}
		*/
		_SafeSource = true;
	}
	else
	{
		/* // This is disabled coz setFx don't work after entity creation (:TODO: try to see why)
		if (_SafeSource)
		{
			CEntitySheet *entitySheet = SheetMngr.get(sheetId());
			const CForageSourceSheet *forageSourceSheet = dynamic_cast<const CForageSourceSheet*>(entitySheet);
			if ( forageSourceSheet )
				setFx( forageSourceSheet->FxFilename );
		}
		*/
		_SafeSource = false;
	}
	if ( setBarsNow )
	{
		setBarValue( _BarCurrentValues[FSBTime], _TimeBar, _BarDestValues[FSBTime] );
		_InitialQuantity = ((uint8)((prop>>7)&0x7f)); // Quantity
		if ( _InitialQuantity != 0 )
		{
			_CurrentQuantity = _InitialQuantity;
			_BarDestValues[FSBQuantiy] = 127;
		}
		else
		{
			_CurrentQuantity = 0;
			_BarDestValues[FSBQuantiy] = 0;
		}
		setBarValue( _BarCurrentValues[FSBQuantiy], _QuantityBar, _BarDestValues[FSBQuantiy] );
		setBarValue( _BarCurrentValues[FSBD], _DBar, _BarDestValues[FSBD] );
		setBarValue( _BarCurrentValues[FSBE], _EBar, _BarDestValues[FSBE] );
	}
	else
	{
		_CurrentQuantity = (uint8)((prop>>7)&0x7f);
		_BarDestValues[1] = (_InitialQuantity != 0) ? (_CurrentQuantity * 127 / _InitialQuantity) : 0; // Quantity
	}

	if ( ! _IsExtractionInProgress )
	{
		_IsExtractionInProgress = (bool)((prop>>28)&1);
		if ( _IsExtractionInProgress )
			buildInSceneInterface(); // Rebuild hud interface
	}

	if ( !_Instance.empty() )
	{
		UParticleSystemInstance fxInst;
		fxInst.cast (_Instance);
		nlassert(!fxInst.empty());
		if ( ForageSourceUseUP )
		{
			fxInst.setUserParam( 0, ForageSourceUP0 );
			fxInst.setUserParam( 1, ForageSourceUP1 );
			fxInst.setUserParam( 2, ForageSourceUP2 );
			fxInst.setUserParam( 3, ForageSourceUP3 );
		}
		else
		{
			// Link Time to live to user param 1 (127-> 0=bright; 0-> 0.9=dark&transparent)
			fxInst.setUserParam( 0, 1.0f - (((float)_BarDestValues[0])*(0.9f / 127.0f) + 0.1f) );

			// Link E to user param 1 (glow)
			fxInst.setUserParam( 1, ((float)(127-_BarDestValues[3])) / 127.0f );
			// :TODO: Find a way to change fx or fx color. setUserColor does a modulate, so it's not suitable.
		//	if (_SafeSource)
		//		fxInst.setUserColor(CRGBA(255,0,0));

			// Link Quantity to user param 2 (particle quantity)
			fxInst.setUserParam( 2, std::min( 1.0f, ((float)_CurrentQuantity) / 50.0f) ); // map 100% to quantity 50
		}
	}
}


/*
 * Update Entity Orientation.
 * Used to carry the kami anger bar (does not change often)
 */
void CForageSourceCL::updateVisualPropertyOrient(const NLMISC::TGameCycle &/* gameCycle */, const sint64 &prop)
{
	uint u = (uint)prop;
	_BarDestValues[4] = (uint8)(u&0x7f);
	_ExtraTime = (uint8)((u>>7)&0x7f);
	_InclBonusExtraTime = (uint8)((u>>14)&0x7f);
}


/*
 * Update Visual FX.
 * Contains group or family (if knowledge is 1 or 2-3), and explosion state.
 */
void CForageSourceCL::updateVisualPropertyVisualFX(const NLMISC::TGameCycle &/* gameCycle */, const sint64 &prop)
{
	// Display explosion FX if the switch flag tells us to do it
	uint8 receivedExplosionSwitch = (uint8)((prop & 0x400) >> 10);
	if ( receivedExplosionSwitch && (receivedExplosionSwitch != _LastExplosionSwitch) ) // bit 10
	{
		UParticleSystemInstance fxNewInst = FXMngr.instantFX( "FOR_explosion.ps" );
		if ( !fxNewInst.empty() )
			fxNewInst.setPos( pos() );
	}
	_LastExplosionSwitch = receivedExplosionSwitch;

	// Set family or group knowledge info
	if ( (_KnowledgeLevel & 0x80) != 0 )
	{
		uint32 index = (uint32)(prop&0x3ff); // 10 bits
		_KnowledgeLevel &= 0x7F;
		switch ( _KnowledgeLevel )
		{
		//case 0: default name unchanged
		case 1: _EntityName = RM_GROUP::toLocalString( (RM_GROUP::TRMGroup)index ); break; // display group as title
		case 2: _EntityName = RM_FAMILY::toLocalString( (RM_FAMILY::TRMFamily)index ); break; // display family as title
		// case 3: received by property Name (see below)
		}
		if ( (_KnowledgeLevel<=2) && (_ProspectorSlot != 255) )
		{
			CEntityCL *prospector = EntitiesMngr.entities()[_ProspectorSlot];
			if (prospector != NULL)
			{
				ucstring prospectorName = prospector->getDisplayName();
				if ( ! prospectorName.empty() )
					_EntityName += ucstring(" [") + prospectorName + ucstring("]");
			}
		}

		// Set icon (2 and 3: the family index is transmitted (for knowledge 3, used only as icon index))
		CEntitySheet *sheet = SheetMngr.get( sheetId() );
		const CForageSourceSheet *forageSourceSheet = dynamic_cast<const CForageSourceSheet*>(sheet);
		if ( forageSourceSheet && (index < forageSourceSheet->Icons.size()) )
		{
			_IconFilename = &(forageSourceSheet->Icons[index]);
		}

		// Rebuild inscene interface
		buildInSceneInterface();
	}
}


/*
 * Update Entity Name.
 * Interpret the property Name as the sheet id (it's not the usual string id!) of
 * the raw material, when the knowledge is 3.
 */
void CForageSourceCL::updateVisualPropertyName(const NLMISC::TGameCycle &/* gameCycle */, const sint64 &prop)
{
	CSheetId rmSheetId( (const uint32&)prop );
	const ucchar *name = STRING_MANAGER::CStringManagerClient::getItemLocalizedName( rmSheetId );
	if ( name )
	{
		_EntityName = name;
		if ( _ProspectorSlot != 255 )
		{
			CEntityCL *prospector = EntitiesMngr.entities()[_ProspectorSlot];
			if (prospector != NULL)
			{
				ucstring prospectorName = prospector->getDisplayName();
				if ( ! prospectorName.empty() )
					_EntityName += ucstring(" [") + prospectorName + ucstring("]");
			}
		}
		// Rebuild inscene interface
		buildInSceneInterface();
	}
}


/*
 * Update Entity Target.
 */
void CForageSourceCL::updateVisualPropertyTarget(const NLMISC::TGameCycle &/* gameCycle */, const sint64 &prop)
{
	sint slot = (sint)prop;
	if ( slot != CLFECOMMON::INVALID_SLOT )
	{
		_ProspectorSlot = slot;
		CEntityCL *prospector = EntitiesMngr.entities()[_ProspectorSlot]; // NULL if entity not received
		if (prospector != NULL)
		{
			ucstring prospectorName = prospector->getDisplayName();
			if ( ! prospectorName.empty() )
				_EntityName = _EntityName + ucstring(" [") + prospectorName + ucstring("]");
		}

		// Rebuild inscene interface
		buildInSceneInterface();
	}
}


/*
 * Display the modifiers
 */
void CForageSourceCL::displayModifiers()
{
	// if none, no op
	if(	_HPDisplayed.empty())
		return;

	// **** get the name pos
	NLMISC::CVector namePos = pos() + CVector(0.f, 0.f, 0.8f);

	// **** compute the scale
	float	dist = (MainCam.getPos()-pos()).norm();
	float	scale= 1.f;
	if(dist > ClientCfg.MaxNameDist)
		return;
	if ( dist < ClientCfg.ConstNameSizeDist )
		scale = 1.0f;
	else
		scale = ClientCfg.ConstNameSizeDist / dist;


	// **** Display HP modifiers.
	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	std::list<HPMD>::iterator itTmp;
	std::list<HPMD>::iterator it = _HPDisplayed.begin();
	while(it != _HPDisplayed.end())
	{
		HPMD &mod = *it;
		//
		const	float totalDuration= 3.f;
		const	float noFadeDuration= 1.f;
		const	float fadeDuration= totalDuration-noFadeDuration;
		if(TimeInSec > (mod.Time+totalDuration))
		{
			itTmp = it;
			++it;
			_HPDisplayed.erase(itTmp);
		}
		else
		{
			uint16 qttyDelta = ((uint16)mod.Value) & 0xFF;
			uint16 qlty = ((uint16)mod.Value) >> 8;
			ucstring hpModifier = ucstring(toString("%u ", qttyDelta) + CI18N::get("uittQualityAbbrev") + toString(" %u", qlty));
			double t = TimeInSec-mod.Time;
			// Compute the position for the Modifier.
			CVector		pos= namePos + CVector(0.0f, 0.0f, 0.3f+(float)t*1.0f/totalDuration);
			// get the color
			CRGBA color;
			if(mod.Value < 0)
				color = CRGBA(220,0,0);
			else
				color = CRGBA(0,220,0);
			// fade
			if(t<noFadeDuration)
				color.A= 255;
			else
				color.A= 255-(uint8)((t-noFadeDuration)*255.0/fadeDuration);

			// Display the name
			pIM->FlyingTextManager.addFlyingText(&mod, hpModifier, pos, color, scale);

			// Next
			++it;
		}
	}
}


/*
 * Destructor
 */
CForageSourceCL::~CForageSourceCL()
{
	releaseInSceneInterfaces();
}


/*NLMISC_COMMAND( resetSourceVP, "", "" )
{
	CLFECOMMON::TCLEntityId slot = UserEntity->selection();
	CEntityCL *selection = EntitiesMngr.entities()[slot];
	if ( selection && selection->isForageSource() )
	{
		((CForageSourceCL*)selection)->resetVP();
	}
	return true;
}

CForageSourceCL *ViewedSource = NULL;

NLMISC_COMMAND( viewSourcePos, "", "" )
{
	CLFECOMMON::TCLEntityId slot = UserEntity->selection();
	CEntityCL *selection = EntitiesMngr.entities()[slot];
	if ( selection && selection->isForageSource() )
	{
		ViewedSource = ((CForageSourceCL*)selection);
		ViewedSource->displayInscenePos();
	}
	else
	{
		// Use with caution
		ViewedSource->displayInscenePos();
	}
	return true;
}*/
