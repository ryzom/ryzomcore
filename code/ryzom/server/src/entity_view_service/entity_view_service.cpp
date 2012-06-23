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



// misc
#include "nel/misc/command.h"
#include "nel/misc/vector_2d.h"
#include "nel/misc/vectord.h"
#include "nel/misc/aabbox.h"
#include "nel/misc/hierarchical_timer.h"
#include "nel/misc/path.h"
#include "nel/misc/sheet_id.h"
//
#include "entity_view_service.h"
// game share
/*
#include "game_share/container_property_receiver.h"
#include "game_share/property_manager_template.h"
*/
#include "game_share/tick_event_handler.h"
#include "game_share/ryzom_entity_id.h"
#include "game_share/ryzom_version.h"

#include <nel/3d/u_camera.h>
#include <nel/3d/u_driver.h>
#include <nel/3d/u_text_context.h>
#include <nel/3d/u_scene.h>
#include <nel/3d/u_3d_mouse_listener.h>
#include <nel/3d/u_material.h>
#include <nel/3d/u_landscape.h>

using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace NL3D;
using namespace NLPACS;

#define BACKGROUND (CRGBA (64,64,64,0))
#define GROUND_COLOR (CRGBA (96,96,128,255))
#define CHAIN_COLOR (CRGBA (160,160,96,255))
#define DIRECTION_COLOR (CRGBA (255,255,255,255))


// driver
UDriver	*pDriver;

// Text context
UTextContext *textContext;

// Create a scene
UScene *pScene;

// Camera
UCamera	*pCam;

// Mouse listener
U3dMouseListener *plistener;

// Material for lines
UMaterial *lineMat;
UMaterial *directMat;
UMaterial *chainMat;
UMaterial *targetMat;

//
CEntityViewService		*pEVS;


//
vector< pair<CVector,CEntity*> >	Names;
vector< deque<CVectorD>* >			Pathes;

//
const uint	MaxObjects = 400;

//
CEntityId		Follow = CEntityId::Unknown;

//
CVector		MousePos;
bool		MouseClicked = false;
CEntityId	SelId = CEntityId::Unknown;


//
/*
TPropertySint32Manager	*XManager;
TPropertySint32Manager	*YManager;
TPropertySint32Manager	*ZManager;
TPropertySint32Manager	*LocalXManager;
TPropertySint32Manager	*LocalYManager;
TPropertySint32Manager	*LocalZManager;
TPropertyFloatManager	*ThetaManager;
TPropertySheetIdManager	*SheetIdManager;
TPropertyUint8Manager	*CombatStateManager;
*/

class CCommandLineListener : public IEventListener
{
public:

	/// Constructor
	CCommandLineListener()
	{
		_CommandLine = "";
		_InputState = false;
	}

	/// Destructor
	~CCommandLineListener()
	{
	}

	/** 
     * Register the listener to the server.
	 * \param server is the reference to the server
	 */
	void addToServer(CEventServer& server)
	{
		server.addListener(EventKeyDownId, this);
	}

	/** 
	 * Unregister the listener to the server.
	 * \param server is the reference to the server
	 */
	void removeFromServer(CEventServer& server)
	{
		server.removeListener(EventKeyDownId, this);
	}

	//
	bool	getInputState() { return _InputState; }

	//
	const string	&getCommandLine() { return _CommandLine; }

protected:
	/*
	 * Call back of the listener.
	 * \param event is the event send to the listener
	 */
	virtual void operator ()(const CEvent& event)
	{
		if (event == EventKeyDownId)
		{
			const CEventKeyDown	&ev = static_cast<const CEventKeyDown&>(event);

			LastKey = ev.Key;

			if (!_InputState)
			{
				if (ev.Key == KeyRETURN)
					_InputState = true;
			}
			else
			{
				switch (ev.Key)
				{
				case KeyRETURN:
					// send command
					ICommand::execute (_CommandLine, *InfoLog);
					_InputState = false;
					_CommandLine = "";
					break;
				case KeyBACK:
					if (_CommandLine.empty())
						break;
					_CommandLine.erase(_CommandLine.end()-1);
					break;
				case KeySPACE:	_CommandLine += ' '; break;
				case KeyTAB: ICommand::expand(_CommandLine); break;
				case KeyNUMPAD0: case KeyNUMPAD1: case KeyNUMPAD2: case KeyNUMPAD3: case KeyNUMPAD4:
				case KeyNUMPAD5: case KeyNUMPAD6: case KeyNUMPAD7: case KeyNUMPAD8: case KeyNUMPAD9:
					_CommandLine += ev.Key-KeyNUMPAD0+'0';
					break;
				case KeyMULTIPLY:	_CommandLine += '*'; break;
				case KeyADD:		_CommandLine += '+'; break;
				case KeySEPARATOR:	_CommandLine += '-'; break;
				case KeySUBTRACT:	_CommandLine += '-'; break;
				case KeyDECIMAL:	_CommandLine += '.'; break;
				case KeyDIVIDE:		_CommandLine += '/'; break;

				default:
					if (ev.Key >= '0' && ev.Key <= '9')
						_CommandLine += ev.Key;
					else if (ev.Key >= 'A' && ev.Key <= 'Z')
						_CommandLine += (ev.Key - 'A' + (ev.Button & shiftKeyButton ? 'A' : 'a'));
					break;
				}
			}
		}
	}

private:
	string	_CommandLine;
	bool	_InputState;
public:
	uint	LastKey;
};

//
class CMousePosListener : public IEventListener
{
public:

	/// Constructor
	CMousePosListener()
	{
	}

	/// Destructor
	~CMousePosListener()
	{
	}

	/** 
     * Register the listener to the server.
	 * \param server is the reference to the server
	 */
	void addToServer(CEventServer& server)
	{
		server.addListener(EventMouseMoveId, this);
		server.addListener(EventMouseDownId, this);
		server.addListener(EventMouseUpId, this);
	}

	/** 
	 * Unregister the listener to the server.
	 * \param server is the reference to the server
	 */
	void removeFromServer(CEventServer& server)
	{
		server.removeListener(EventMouseMoveId, this);
		server.removeListener(EventMouseDownId, this);
		server.removeListener(EventMouseUpId, this);
	}

protected:
	/*
	 * Call back of the listener.
	 * \param event is the event send to the listener
	 */
	virtual void operator ()(const CEvent& event)
	{
		if (event == EventMouseMoveId)
		{
			const CEventMouseMove	&ev = static_cast<const CEventMouseMove&>(event);
			MousePos = CVector(ev.X, ev.Y, 0.0f);
		}
		else if (event == EventMouseDownId)
		{
			const CEventMouseDown	&ev = static_cast<const CEventMouseDown&>(event);
			MousePos = CVector(ev.X, ev.Y, 0.0f);
			if ((ev.Button & rightButton) != 0)
				MouseClicked = true;
		}
		else if (event == EventMouseUpId)
		{
			const CEventMouseUp		&ev = static_cast<const CEventMouseUp&>(event);
			MousePos = CVector(ev.X, ev.Y, 0.0f);
			if ((ev.Button & rightButton) != 0)
				MouseClicked = false;
		}
	}
};



//
bool	GPMSUp = false;

void cbGpmsUp( const std::string &serviceName, uint16 serviceId, void *arg )
{
	GPMSUp = true;
}

void cbGpmsDown( const std::string &serviceName, uint16 serviceId, void *arg )
{
	GPMSUp = false;
}

bool	CMSUp = false;

void cbCmsUp( const std::string &serviceName, uint16 serviceId, void *arg )
{
	CMSUp = true;
}

void cbCmsDown( const std::string &serviceName, uint16 serviceId, void *arg )
{
	CMSUp = false;
}
//
CCommandLineListener	CommandLine;
CMousePosListener		MousePosListener;

/*
 * Initialisation
 */
void	cbMirrorIsReady( CMirror *mirror )
{
	pEVS->initMirror();
}

//
void	CEntity::setup(const TDataSetRow& entityIndex)
{
	EntityIndex = entityIndex;

	X.init(TheDataset, Id, "X");
	Y.init(TheDataset, Id, "Y");
	Z.init(TheDataset, Id, "Z");
	Theta.init(TheDataset, Id, "Theta");
	Sheet.init(TheDataset, Id, "Sheet");
	CombatState.init(TheDataset, Id, "CombatState");
}

//
void	CEntity::setSheet(const CSheetId &sheetId)
{
	SheetId = sheetId;
	SheetName = sheetId.toString();

	const CSheets::CSheet	*pSheet = CSheets::lookup(SheetId);
	if (pSheet != NULL)
	{
		Radius = pSheet->BoundingRadius;
		Height = pSheet->Height;
	}
}

/****************************************************************\
							init() 
\****************************************************************/
// init the service
void CEntityViewService::init()
{   
	setVersion (RYZOM_VERSION);

	// set update time out
	setUpdateTimeout(30);

	pEVS = this;

	CSheetId::init();

	//
	CUnifiedNetwork::getInstance()->setServiceUpCallback( "GPMS", cbGpmsUp, 0);
	CUnifiedNetwork::getInstance()->setServiceDownCallback( "GPMS", cbGpmsDown, 0);
	CUnifiedNetwork::getInstance()->setServiceUpCallback( "CMS", cbCmsUp, 0);
	CUnifiedNetwork::getInstance()->setServiceDownCallback( "CMS", cbCmsDown, 0);

	// Create a driver
	pDriver=UDriver::createDriver(0);

	// Text context
	pDriver->setDisplay (UDriver::CMode(640, 480, 0));
	pDriver->setFontManagerMaxMemory (2000000);
	textContext=pDriver->createTextContext ("R:\\code\\ryzom\\data\\3d\\common\\fonts\\arialuni.ttf");
	textContext->setHotSpot (UTextContext::TopLeft);
	textContext->setColor (CRGBA (255,255,255));
	textContext->setFontSize (12);

	//
	CommandLine.addToServer(pDriver->EventServer);
	MousePosListener.addToServer(pDriver->EventServer);

	// Create a scene
	pScene=pDriver->createScene();


	// Camera
	pCam=pScene->getCam();
	pCam->setTransformMode (UTransformable::DirectMatrix);
	pCam->setPerspective ((float)Pi/2.f, 1.33f, 0.1f, 1000);

	// Mouse listener
	plistener=pDriver->create3dMouseListener ();
	plistener->setHotSpot (CVectorD (0,0,0));
	plistener->setFrustrum (pCam->getFrustum());
	plistener->setMatrix (pCam->getMatrix());
	plistener->setMouseMode (U3dMouseListener::firstPerson);

	// Add mouse listener to event server
//	plistener->addToServer(CNELU::EventServer);

	// Material for lines
	lineMat=pDriver->createMaterial ();
	lineMat->initUnlit ();
	lineMat->setColor (GROUND_COLOR);

	directMat=pDriver->createMaterial ();
	directMat->initUnlit ();
	directMat->setColor (DIRECTION_COLOR);

	chainMat=pDriver->createMaterial ();
	chainMat->initUnlit ();
	chainMat->setColor (CHAIN_COLOR);

	targetMat=pDriver->createMaterial ();
	targetMat->initUnlit ();
	targetMat->setColor (CHAIN_COLOR);

	// Init the mirror system
	vector<string> datasetNames;
	datasetNames.push_back( "fe_temp" );
	Mirror.init( datasetNames, cbMirrorIsReady );

	uint	i;

	for (i=0; i<MaxObjects; ++i)
	{
		UInstance	*instance = pScene->createInstance ("box_op.shape");
		_OBoxes.push_back(instance);

		instance->setScale (CVectorD (1.0, 1.0, 1.0));
		instance->setRotQuat (CQuat (CVectorD (0, 0, 1), (float)0.0));
		instance->setPos(CVectorD(0.0, 0.0, 0.0));
		instance->hide();
	}

	for (i=0; i<MaxObjects; ++i)
	{
		UInstance	*instance = pScene->createInstance ("box_tr.shape");
		_TBoxes.push_back(instance);

		instance->setScale (CVectorD (1.0, 1.0, 1.0));
		instance->setRotQuat (CQuat (CVectorD (0, 0, 1), (float)0.0));
		instance->setPos(CVectorD(0.0, 0.0, 0.0));
		instance->hide();
	}

	for (i=0; i<MaxObjects; ++i)
	{
		UInstance	*instance = pScene->createInstance ("cylinder_op.shape");
		_OCylinders.push_back(instance);

		instance->setScale (CVectorD (1.0, 1.0, 1.0));
		instance->setRotQuat (CQuat (CVectorD (0, 0, 1), (float)0.0));
		instance->setPos(CVectorD(0.0, 0.0, 0.0));
		instance->hide();
	}

	for (i=0; i<MaxObjects; ++i)
	{
		UInstance	*instance = pScene->createInstance ("cylinder_tr.shape");
		_TCylinders.push_back(instance);

		instance->setScale (CVectorD (1.0, 1.0, 1.0));
		instance->setRotQuat (CQuat (CVectorD (0, 0, 1), (float)0.0));
		instance->setPos(CVectorD(0.0, 0.0, 0.0));
		instance->hide();
	}

	vector<string>	chainfiles;
	CPath::getPathContent("data_shard/ochains/", true, false, true, chainfiles);

	nlinfo("Loading %d ochain files", chainfiles.size());

	for (i=0; i<chainfiles.size(); ++i)
	{
		// open zone ochain file
		CIFile	f(CPath::lookup(chainfiles[i]));

		// load chains
		vector<COrderedChain3f>	chains;
		f.serialCont(chains);

		// convert zonename into x y coordinates
		string		filename = CFile::getFilename(chainfiles[i]);
		const char	*zone = filename.c_str();
		char		ax[16];

		sint	x, y;
		sscanf(zone, "%d_%2s", &y, ax);

		--y;
		x = (ax[0]-'A')*26+(ax[1]-'A');

		// get zone center for translation
		CVector	center(160.0f*((float)x+0.5f), -160.0f*((float)y+0.5f), 0.0f);

		// translate all chains
		uint	j;
		for (j=0; j<chains.size(); ++j)
			chains[j].translate(center);

		// add chains to common chains
		_Chains.insert(_Chains.end(), chains.begin(), chains.end());
	}

	nlinfo("Loaded %d chains, inserting into chain grid...", _Chains.size());

	for (i=0; i<_Chains.size(); ++i)
	{
		const vector<CVector>	&verts = _Chains[i].getVertices();
		CAABBox					bbox;

		if (verts.empty())
			continue;

		bbox.setCenter(verts[0]);
		bbox.setHalfSize(CVector::Null);

		uint	j;
		for (j=1; j<verts.size(); ++j)
			bbox.extend(verts[j]);

		_ChainGrid.insert(&(_Chains[i]), bbox.getCenter());
	}

	nlinfo("All chains inserted");

	CSheetId::init();
	CSheets::init();

} // init //


/*--------------------------------------------------------------*\
						cbSync()  
\*--------------------------------------------------------------*/
void cbSync()
{
} // cbSync //

void CEntityViewService::initMirror()
{
	// Allow to add a few entities manually (using the command addEntity)
//	Mirror.declareEntityTypeOwner( RYZOMID::player, 10 );
//	Mirror.declareEntityTypeOwner( RYZOMID::npc, 10 );

	DataSet = &(Mirror.getDataSet("fe_temp"));
	DataSet->declareProperty( "X", PSOReadOnly | PSONotifyChanges, "X");		// group notification on X
	DataSet->declareProperty( "Y", PSOReadOnly | PSONotifyChanges, "X");		// group notification on X
	DataSet->declareProperty( "Z", PSOReadOnly | PSONotifyChanges, "X");		// group notification on X
	DataSet->declareProperty( "Theta", PSOReadOnly | PSONotifyChanges, "Theta");
	DataSet->declareProperty( "Sheet", PSOReadOnly | PSONotifyChanges, "Sheet");
	DataSet->declareProperty( "CombatState", PSOReadOnly);

	initRyzomVisualPropertyIndices( *DataSet );
}


//
void	CEntityViewService::updateEntities()
{
	if ( ! Mirror.mirrorIsReady() )
		return;

	// Process entities added to mirror
	TheDataset.beginAddedEntities();
	entityIndex = TheDataset.getNextAddedEntity();
	while ( entityIndex != LAST_CHANGED )
	{
		TEntityMap::iterator	it = createEntity(TheDataset.getEntityId(entityIndex));
		(*it).second.setup( entityIndex );
		entityIndex = TheDataset.getNextAddedEntity();
	}
	TheDataset.endAddedEntities();

	// Process entities removed from mirror
	TheDataset.beginRemovedEntities();
	CEntityId *id;
	TDataSetRow entityIndex = TheDataset.getNextRemovedEntity( &id );
	while ( entityIndex != LAST_CHANGED )
	{
		removeEntity( *id );
		entityIndex = TheDataset.getNextRemovedEntity( &id );
	}
	TheDataset.endRemovedEntities();

	// Process properties changed and notified in the mirror
	TPropertyIndex propIndex;
	TheDataset.beginChangedValues();
	TheDataset.getNextChangedValue( entityIndex, propIndex );
	while ( entityIndex != LAST_CHANGED )
	{
		const CEntityId& entityId = TheDataset.getEntityId( entityIndex );
		CEntity	*entity = getEntity(entityId);

		// TEMP: while we don't handle all by entity indices, we need to test if the entityId has been notified (added)
		if ( entity != NULL )
		{
			if (propIndex == DSPropertyPOSX)
			{
				entity->Position = CVectorD(entity->X()*0.001, entity->Y()*0.001, entity->Z()*0.001);
				_EntityGrid.move(entity->Iterator, entity->Position);

				entity->Path.push_back(entity->Position);
				while (entity->Path.size() > 100)
					entity->Path.pop_front();
			}
			else if (propIndex == DSPropertyORIENTATION)
			{
				entity->Orientation = entity->Theta();
			}
			else if (propIndex == DSPropertySHEET)
			{
				uint32				sheetId;
				TheDataset.getValue( entityIndex, DSPropertySHEET, sheetId);
				entity->setSheet(CSheetId(sheetId));
			}
		}

		//nldebug( "Pos changed from mirror E%d", entityIndex  );
		TheDataset.getNextChangedValue( entityIndex, propIndex );
	}
	TheDataset.endChangedValues();

	Pathes.clear();

	uint	usedOCylinders = 0;
	uint	usedTCylinders = 0;

	CAABBox	bbox;

	bbox.setCenter(pCam->getMatrix().getPos());
	bbox.setHalfSize(CVector(32.0f, 32.0f, 10.0f));

	_EntityGrid.clearSelection();
	_EntityGrid.select(bbox);

	//
	TEntityGrid::CIterator	its;

	CMatrix	matrix = pCam->getMatrix();
	matrix.invert();

	uint32	w, h;
	pDriver->getWindowSize(w, h);

	float		minDist = 64;
	SelId = CEntityId::Unknown;
	for (its=_EntityGrid.begin(); its!=_EntityGrid.end(); ++its)
	{
		CEntity	&entity = *(*its);

		CVector	np = matrix.mulPoint(entity.Position);

		if (np.y < 0.0)
			continue;

		CVector	d = pCam->getFrustum().project(np)-MousePos;
		float	px = d.x*w, py = d.y*h;

		float	dist = (float)sqrt(px*px+py*py);
		if (dist < minDist)
		{
			minDist = dist;
			SelId = entity.Id;
		}
	}

	Names.clear();

	for (its=_EntityGrid.begin(); its!=_EntityGrid.end(); ++its)
	{
		CEntity	&entity = *(*its);

		switch(entity.Id.getType())
		{
		case RYZOMID::player:
		case RYZOMID::npc:
		case RYZOMID::creature:
			if (usedOCylinders < _OCylinders.size())
			{
				CVector	np = matrix.mulPoint(entity.Position);

				if (np.y < 0.0)
					break;

				if (entity.Id == SelId)
				{
					_TCylinders[usedTCylinders]->setRotQuat(CQuat (CVectorD (0, 0, 1), entity.Orientation));
					_TCylinders[usedTCylinders]->setPos(entity.Position);
					_TCylinders[usedTCylinders]->setScale(CVectorD (entity.Radius, entity.Radius, entity.Height));
					if (_TCylinders[usedTCylinders]->getVisibility() == UTransform::Hide)
						_TCylinders[usedTCylinders]->show();
					++usedTCylinders;

					Names.push_back(make_pair<CVector,CEntity*>(pCam->getFrustum().project(np), &entity));
				}
				else
				{
					_OCylinders[usedOCylinders]->setRotQuat(CQuat (CVectorD (0, 0, 1), entity.Orientation));
					_OCylinders[usedOCylinders]->setPos(entity.Position);
					_OCylinders[usedOCylinders]->setScale(CVectorD (entity.Radius, entity.Radius, entity.Height));
					if (_OCylinders[usedOCylinders]->getVisibility() == UTransform::Hide)
						_OCylinders[usedOCylinders]->show();
					++usedOCylinders;
				}

				TEntityMap::iterator	it;
				if (entity.Target != CEntityId::Unknown && (it= _Entities.find(entity.Target))!=_Entities.end())
				{
					CLineColor	line;
					line = CLine(entity.Position+CVector(0.0f, 0.0f, 1.0f), (*it).second.Position+CVector(0.0f, 0.0f, 1.0f));
					line.Color0 = CRGBA(255, 128, 128, 255);
					line.Color1 = CRGBA(160, 160,   0, 128);
					_Targets.push_back(line);
				}

				Pathes.push_back(&(entity.Path));

				_Directions.push_back(CLine(entity.Position+CVector(entity.Radius*(float)cos(entity.Orientation), entity.Radius*(float)sin(entity.Orientation), 1.0f),
											entity.Position+CVector((entity.Radius+1.0f)*(float)cos(entity.Orientation), (entity.Radius+1.0f)*(float)sin(entity.Orientation), 1.0f)));
			}
			break;
		case RYZOMID::trigger:
			if (usedTCylinders < _TCylinders.size())
			{
				_TCylinders[usedTCylinders]->setRotQuat(CQuat (CVectorD (0, 0, 1), entity.Orientation));
				_TCylinders[usedTCylinders]->setPos(entity.Position);
				_TCylinders[usedTCylinders]->setScale(CVectorD (1.0, 1.0, 10.0));
				if (_TCylinders[usedTCylinders]->getVisibility() == UTransform::Hide)
					_TCylinders[usedTCylinders]->show();
				++usedTCylinders;
			}
			break;
		default:
			break;
		}
	}

	_EntityGrid.clearSelection();

	for (; usedOCylinders<_OCylinders.size(); ++usedOCylinders)
		_OCylinders[usedOCylinders]->hide();

	for (; usedTCylinders<_TCylinders.size(); ++usedTCylinders)
		_TCylinders[usedTCylinders]->hide();
}


/****************************************************************\
							update() 
\****************************************************************/
// main loop
bool CEntityViewService::update()
{
	// Main loop
	if (!pDriver->isActive() || pDriver->AsyncListener.isKeyPushed (KeyESCAPE))
		return false;

	// Get time
	static TTime	lastTime = CTime::getLocalTime ();

	TTime			newTime = CTime::getLocalTime ();
	double			deltaTime = (double)(uint32)(newTime-lastTime)/1000.0;
	lastTime=newTime;

	static bool		dispChains = true;
	static bool		dispPath = false;

	updateEntities();

	if (pDriver->AsyncListener.isKeyPushed (KeyF1))
		dispChains = !dispChains;

	if (pDriver->AsyncListener.isKeyPushed (KeyF2))
		dispPath = !dispPath;


	// manual engagement
	static bool			inCommand = false;

	static bool			selAssail = false;
	static bool			selTarget = false;
	static CEntityId	assailId;
	static CEntityId	targetId;

	static bool			selD1 = false;
	static bool			selD2 = false;
	static CEntityId	d1Id;
	static CEntityId	d2Id;

	static TTime		displayTime = 0;
	static TTime		displayDistance = 0;
	static char			displayString[1024];

	CMatrix	matrix = pCam->getMatrix();
	matrix.invert();

	if (!CommandLine.getInputState())
	{
		// engage
		if (pDriver->AsyncListener.isKeyPushed(KeyE) && !inCommand)
		{
			selAssail = true;
			sprintf(displayString, "Engage: select assailant");
			displayTime = CTime::getLocalTime()+5000;
			inCommand = true;
		}

		if (selAssail && MouseClicked)
		{
			assailId = SelId;
			MouseClicked = false;
			selAssail = false;
			selTarget = true;
			sprintf(displayString, "Engage %s: select target", assailId.toString().c_str());
			displayTime = CTime::getLocalTime()+5000;
		}

		if (selTarget && MouseClicked)
		{
			targetId = SelId;
			MouseClicked = false;
			selAssail = false;
			selTarget = false;

			CMessage	msg("FORCE_ENGAGE");
			msg.serial(assailId, targetId);
			CUnifiedNetwork::getInstance()->send("AIS", msg);

			sprintf(displayString, "Engaged %s with %s", assailId.toString().c_str(), targetId.toString().c_str());
			displayTime = CTime::getLocalTime()+5000;
			inCommand = false;
		}

		// distance
		if (pDriver->AsyncListener.isKeyPushed(KeyD) && !inCommand)
		{
			selD1 = true;
			sprintf(displayString, "Distance: select entity");
			displayTime = CTime::getLocalTime()+5000;
			inCommand = true;
		}

		if (selD1 && MouseClicked)
		{
			d1Id = SelId;
			MouseClicked = false;
			selD1 = false;
			selD2 = true;
			sprintf(displayString, "Distance %s: select entity", d1Id.toString().c_str());
			displayTime = CTime::getLocalTime()+5000;
		}

		if (selD2 && MouseClicked)
		{
			d2Id = SelId;
			MouseClicked = false;
			selD1 = false;
			selD2 = false;

			TEntityMap::iterator	it1, it2;
			it1 = _Entities.find(d1Id);
			it2 = _Entities.find(d2Id);
			if (it1 == _Entities.end())
			{
				sprintf(displayString, "Distance: can't find entity %s", d1Id.toString().c_str());
				displayTime = CTime::getLocalTime()+2000;
			}
			else if (it2 == _Entities.end())
			{
				sprintf(displayString, "Distance: can't find entity %s", d2Id.toString().c_str());
				displayTime = CTime::getLocalTime()+2000;
			}
			else
			{
				CEntity	&e1 = (*it1).second,
						&e2 = (*it2).second;

				CVectorD	d1 = e1.Position-e2.Position;
				CVectorD	d2 = d1;
				d1.z = 0.0;

				sprintf(displayString, "Distance: |%s-%s| = %f (%f using z)", d1Id.toString().c_str(), d2Id.toString().c_str(), d1.norm(), d2.norm());
				displayTime = CTime::getLocalTime()+8000;
				displayDistance = CTime::getLocalTime()+8000;
			}

			inCommand = false;
		}
	}

	// Matrix
	pCam->setMatrix(plistener->getViewMatrix());

	// Clear
	pDriver->clearBuffers (BACKGROUND);

	// Render
	pScene->render ();

	// Draw some lines
	{
		pDriver->setMatrixMode3D(*pCam);
		float	x, y;

		CVector	pos = pCam->getMatrix().getPos();

		const float	GridStep = 16.0f;
		const float	GridWidth = 48.0f;

		pos.x = pos.x-(float)fmod(pos.x, GridStep);
		pos.y = pos.y-(float)fmod(pos.y, GridStep);
		pos.z = 0.0f;

		for (x=-GridWidth; x<=+GridWidth; x+=GridStep)
		{
			CLine	line(pos+CVector(x, -GridWidth, 0.0f), pos+CVector(x, +GridWidth, 0.0f));
			pDriver->drawLine(line, *lineMat);
		}

		for (y=-GridWidth; y<=+GridWidth; y+=GridStep)
		{
			CLine	line(pos+CVector(-GridWidth, y, 0.0f), pos+CVector(+GridWidth, y, 0.0f));
			pDriver->drawLine(line, *lineMat);
		}
	}

	uint	i;
	for (i=0; i<_Directions.size(); ++i)
		pDriver->drawLine(_Directions[i], *directMat);

	for (i=0; i<_Targets.size(); ++i)
		pDriver->drawLine(_Targets[i], *targetMat);

	CVector	dp1, dp2, dp, ddpos;
	if (displayDistance>CTime::getLocalTime())
	{
		TEntityMap::iterator	it1 = _Entities.find(d1Id),
								it2 = _Entities.find(d2Id);

		if (it1 == _Entities.end() || it2 == _Entities.end())
		{
			displayDistance = 0;
		}
		else
		{
			dp1 = (*it1).second.Position;
			dp2 = (*it2).second.Position;
			dp = dp1-dp2;
			dp.z = 0.0f;

			CLine	line(dp1, dp2);
			pDriver->drawLine(line, *lineMat);

			ddpos = pCam->getFrustum().project(matrix.mulPoint((dp1+dp2)*0.5f));
		}
	}

	_Directions.clear();
	_Targets.clear();

	if (dispPath)
	{
		for (i=0; i<Pathes.size(); ++i)
		{
			uint			j;
			CLine			line;
			deque<CVectorD>	&path = *(Pathes[i]);

			for (j=0; j<path.size()-1; ++j)
			{
				line.V0 = path[j];
				line.V1 = path[j+1];
				pDriver->drawLine(line, *directMat);
			}
		}
	}

	if (dispChains)
	{
		CAABBox	bbox;

		bbox.setCenter(pCam->getMatrix().getPos());
		bbox.setHalfSize(CVector(128.0f, 128.0f, 10.0f));

		_ChainGrid.clearSelection();
		_ChainGrid.select(bbox);

		//
		TChainGrid::CIterator	itc;
		for (itc=_ChainGrid.begin(); itc!=_ChainGrid.end(); ++itc)
		{
			const COrderedChain3f	&chain = *(*itc);
			const vector<CVector>	&verts = chain.getVertices();

			for (i=0; (sint)i<(sint)(verts.size()-1); ++i)
				pDriver->drawLine(CLine(verts[i], verts[i+1]), *chainMat);
		}
	}

	{
		textContext->setHotSpot(UTextContext::TopLeft);
		for (i=0; i<Names.size(); ++i)
		{
			textContext->printfAt(Names[i].first.x, Names[i].first.y, "%s", Names[i].second->Id.toString().c_str());
			textContext->printfAt(Names[i].first.x, Names[i].first.y-0.02f, "(%.2f,%.2f,%.2f) : %.3f", Names[i].second->Position.x, Names[i].second->Position.y, Names[i].second->Position.z, Names[i].second->Orientation);
			textContext->printfAt(Names[i].first.x, Names[i].first.y-0.04f, "Target: %s", Names[i].second->Target == CEntityId::Unknown ? "<None>" : Names[i].second->Target.toString().c_str());
			textContext->printfAt(Names[i].first.x, Names[i].first.y-0.06f, "SheetId: %d %s", Names[i].second->SheetId.asInt(), Names[i].second->SheetName.c_str());
			textContext->printfAt(Names[i].first.x, Names[i].first.y-0.08f, "Radius: %.2f Height: %.2f", Names[i].second->Radius, Names[i].second->Height);

			static char		*combatStates[] =
			{
				"NotEngaged",
				"MovingTowardTarget",
				"TargetUnreachable",
				"Engaged",
				"TargetLost"
			};
			textContext->printfAt(Names[i].first.x, Names[i].first.y-0.10f, "CombatState: %s", combatStates[Names[i].second->CombatState]);
		}

		CVector	posl = plistener->getViewMatrix().getPos();
		CVector	posc = pCam->getMatrix().getPos();

		textContext->printfAt (0.0f ,1.0f, "pos listener: %.1f,%.1f,%.1f - pos cam: %.1f,%.1f,%.1f", posl.x, posl.y, posl.z, posc.x, posc.y, posc.z);

		if (CommandLine.getInputState())
			textContext->printfAt (0.0f ,0.98f,"Command: %s_", CommandLine.getCommandLine().c_str());

		if (displayTime>CTime::getLocalTime())
			textContext->printfAt(0.0f, 0.96f, "%s", displayString);

		textContext->setHotSpot(UTextContext::MiddleMiddle);
		if (displayTime>CTime::getLocalTime())
			textContext->printfAt(ddpos.x, ddpos.y, "%.3f", dp.norm());
	}

	// Swap
	pDriver->swapBuffers ();

	// Pump messages
	pDriver->EventServer.pump(true);

	return true;
} // update //



/****************************************************************\
							release() 
\****************************************************************/
void CEntityViewService::release()
{
	// Mouse listener
	pDriver->delete3dMouseListener (plistener);

}// release //



/****************************************************************\
 ************** callback table for input message ****************
\****************************************************************/
void cbSetTarget(CMessage& msgin, const string &serviceName, uint16 serviceId)
{
	CEntityId	ide, idt;
	msgin.serial(ide, idt);
	((CEntityViewService*)IService::getInstance())->setTarget(ide, idt);
}

void cbSetPos(CMessage& msgin, const string &serviceName, uint16 serviceId)
{
/*
	CEntityId	id;
	sint		x, y, z;
	msgin.serial(id, x, y, z);

	((CEntityViewService*)IService::getInstance())->setPos(id, x, y, z);
*/
}


TUnifiedCallbackItem CbArray[] = 
{
	{ "TARGET", cbSetTarget },
	{ "EVSPOS", cbSetPos },
};

NLNET_SERVICE_MAIN( CEntityViewService, "EVS", "entity_view_service", 0, CbArray, "", "" );



NLMISC_COMMAND(follow, "follow an entity", "entityId")
{
	if (args.size() < 1)
		return false;

	CEntityId	sid;

	uint64		id;
	uint		type;
	uint		creatorId;
	uint		dynamicId;

	if (sscanf(args[0].c_str(), "(%"NL_I64"x:%x:%x:%x)", &id, &type, &creatorId, &dynamicId) != 4)
		return false;

	sid.setShortId( id );
	sid.setType( type );
	sid.setCreatorId( creatorId );
	sid.setDynamicId( dynamicId );

	Follow = sid;

	return true;
}

NLMISC_COMMAND(goTo, "goto a position", "x y z")
{
	if (args.size() < 3)
		return false;

	sint32	x = atoi(args[0].c_str());
	sint32	y = atoi(args[1].c_str());
	sint32	z = atoi(args[2].c_str());

	CMatrix	mat = pCam->getMatrix();
	mat.setPos(CVector((float)x, (float)y, (float)z));
	pCam->setMatrix(mat);

	plistener->setMatrix(pCam->getMatrix());

	return true;
}

NLMISC_COMMAND(displayEntities, "display entities positions", "")
{
	static_cast<CEntityViewService*>(IService::getInstance())->displayEntities();

	return true;
}

NLMISC_COMMAND(setUpdateTimeout, "set update timeout of service", "ms")
{
	if (args.size() != 1)
		return false;

	IService::getInstance()->setUpdateTimeout(atoi(args[0].c_str()));

	return true;
}
