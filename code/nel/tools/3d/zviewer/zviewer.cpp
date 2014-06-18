// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#include <string>
#include <deque>

#include <nel/misc/types_nl.h>
#include <nel/misc/config_file.h>
#include <nel/misc/debug.h>
#include <nel/misc/path.h>
#include <nel/misc/i18n.h>

#include <nel/3d/driver.h>
#include <nel/3d/camera.h>
#include <nel/3d/landscape_model.h>
#include <nel/3d/landscape.h>
#include <nel/3d/text_context.h>
#include <nel/3d/mini_col.h>
#include <nel/3d/nelu.h>
#include <nel/3d/scene_group.h>
#include <nel/3d/texture_file.h>

//#include "nel/net/local_entity.h"

#include "move_listener.h"

// Tempyoyo.
#include <nel/3d/height_map.h>

#ifdef NL_OS_WINDOWS
#	ifndef NL_COMP_MINGW
#		define NOMINMAX
#	endif
#	include <windows.h>
#endif // NL_OS_WINDOWS

using namespace std;
using namespace NLMISC;
using namespace NL3D;


#define BANK_PAH_RELATIVE

#ifndef NL_ZVIEWER_CFG
#define NL_ZVIEWER_CFG "."
#endif // NL_ZVIEWER_CFG

/**
 * CViewerConfig
 */
struct CViewerConfig
{
	bool			Windowed;
	uint			Width;
	uint			Height;
	uint			Depth;
	CVector			Position;
	CVector			Heading;
	CVector			EyesHeight;
	CRGBA			Background;

	// Landscape
	bool			AutoLight;
	CVector			LightDir;
	string			ZonesPath;
	string			BanksPath;
	string			TilesPath;
	bool			UseDDS;
	bool			AllPathRelative;

	string			IgPath;
	string			ShapePath;
	string			MapsPath;
	string			Bank;
	string			FontPath;
	CTextContext	TextContext;
	CFontManager	FontManager;
	float			ZFar;
	float			LandscapeTileNear;
	float			LandscapeThreshold;
	bool			LandscapeNoise;
	vector<string>	Zones;
	vector<string>	Igs;

	// HeightField.
	string			HeightFieldName;
	float			HeightFieldMaxZ;
	float			HeightFieldOriginX;
	float			HeightFieldOriginY;
	float			HeightFieldSizeX;
	float			HeightFieldSizeY;

	// StaticLight
	CRGBA			LandAmbient;
	CRGBA			LandDiffuse;

	CViewerConfig()
	{
		Windowed = true;
		Width = 800;
		Height = 600;
		Depth = 32;
		Position = CVector( 1088.987793f, -925.732178f, 0.0f );
		Heading = CVector(0,1,0);
		EyesHeight = CVector(0,0,1.8f);
		Background = CRGBA(100,100,255);
		AutoLight = false;
		LightDir = CVector (1, 0, 0);
		ZonesPath = "./";
		BanksPath = "./";
		TilesPath = "./";
		UseDDS = false;
		AllPathRelative = false;
		IgPath = "./";
		ShapePath = "./";
		MapsPath = "./";
		Bank = "bank.bank";
		FontPath = "\\\\server\\code\\fonts\\arialuni.ttf";
		ZFar = 1000;
		LandscapeTileNear = 50.0f;
		LandscapeThreshold = 0.001f;
		LandscapeNoise = true;

		HeightFieldName= "";
		HeightFieldMaxZ= 100;
		HeightFieldOriginX= 16000;
		HeightFieldOriginY= -24000;
		HeightFieldSizeX= 160;
		HeightFieldSizeY= 160;

		CRGBA diffuse (241, 226, 244);
		CRGBA ambiant  (17, 54, 100);
		LandDiffuse= diffuse;
		LandAmbient= ambiant;

	}
};

CViewerConfig			ViewerCfg;



CLandscapeModel			*Landscape = NULL;
CMoveListener			MoveListener;
CMiniCol				CollisionManager;











/*******************************************************************\
						getZoneNameByCoord()
\*******************************************************************/
string getZoneNameByCoord(float x, float y)
{
	const float zoneDim = 160.0f;

	float xcount = x/zoneDim;
	float ycount = -y/zoneDim + 1;

	string zoneName;
	char ych[32];
	sprintf(ych,"%d",(sint)ycount);
	sint sz = (sint)strlen(ych);
	for(sint i = 0; i<sz; i++)
	{
		zoneName += ych[i];
	}
	zoneName += '_';
	zoneName += 'A' + (sint)xcount/26;
	zoneName += 'A' + (sint)xcount%26;

	return zoneName;
}




/*********************************************************\
					displayOrientation()
\*********************************************************/
void displayOrientation()
{
	float x = 0.9f*4.f/3.f;
	float y = 0.1f;
	float radius = 0.015f;

	// Triangle
	CMaterial mat;
	mat.initUnlit();
	mat.setSrcBlend(CMaterial::srcalpha);
	mat.setDstBlend(CMaterial::invsrcalpha);
	mat.setBlend(true);
	
	CVertexBuffer vb;
	vb.setVertexFormat (CVertexBuffer::PositionFlag);
	vb.setNumVertices (7);
	{
		CVertexBufferReadWrite vba;
		vb.lock(vba);
		
		// tri
		vba.setVertexCoord (0, CVector (-radius, 0, 0));
		vba.setVertexCoord (1, CVector (radius, 0, 0));
		vba.setVertexCoord (2, CVector (0, 0, 3*radius));

		// quad
		vba.setVertexCoord (3, CVector (-radius, 0, -radius));
		vba.setVertexCoord (4, CVector (radius, 0, -radius));
		vba.setVertexCoord (5, CVector (radius, 0, radius));
		vba.setVertexCoord (6, CVector (-radius, 0, radius));
	}
	
	CNELU::Driver->activeVertexBuffer(vb);

	CIndexBuffer pbTri;
	pbTri.setNumIndexes (3);
	{
		CIndexBufferReadWrite iba;
		pbTri.lock (iba);
		iba.setTri (0, 0, 1, 2);
	}
	
	CIndexBuffer pbQuad;
	pbQuad.setNumIndexes (6);
	{
		CIndexBufferReadWrite iba;
		pbQuad.lock(iba);
		iba.setTri (0, 3, 4, 5);
		iba.setTri (3, 5, 6, 3);
	}
	
	CNELU::Driver->setFrustum (0.f, 4.f/3.f, 0.f, 1.f, -1.f, 1.f, false);
	CMatrix mtx;
	mtx.identity();
	CNELU::Driver->setupViewMatrix (mtx);

	mat.setColor(CRGBA(50,255,255,150));

	// up
	mtx.identity();
	mtx.translate(CVector(x,0,y));
	mtx.rotateY(MoveListener.getRotZ() );
	mtx.translate(CVector(0,0,radius));
	CNELU::Driver->setupModelMatrix (mtx);
	CNELU::Driver->activeVertexBuffer(vb);
	CNELU::Driver->activeIndexBuffer(pbTri);
	CNELU::Driver->renderTriangles(mat, 0, pbTri.getNumIndexes()/3);

	mat.setColor(CRGBA(50,50,255,150));

	// down
	mtx.identity();
	mtx.translate(CVector(x,0,y));
	mtx.rotateY(MoveListener.getRotZ() + (float)Pi);
	mtx.translate(CVector(0,0,radius));
	CNELU::Driver->setupModelMatrix (mtx);
	CNELU::Driver->renderTriangles(mat, 0, pbTri.getNumIndexes()/3);

	// left
	mtx.identity();
	mtx.translate(CVector(x,0,y));
	mtx.rotateY(MoveListener.getRotZ() - (float)Pi/2);
	mtx.translate(CVector(0,0,radius));
	CNELU::Driver->setupModelMatrix (mtx);
	CNELU::Driver->renderTriangles(mat, 0, pbTri.getNumIndexes()/3);

	// right
	mtx.identity();
	mtx.translate(CVector(x,0,y));
	mtx.rotateY(MoveListener.getRotZ() + (float)Pi/2);
	mtx.translate(CVector(0,0,radius));
	CNELU::Driver->setupModelMatrix (mtx);
	CNELU::Driver->renderTriangles(mat, 0, pbTri.getNumIndexes()/3);
	
	// center
	mtx.identity();
	mtx.translate(CVector(x,0,y));
	mtx.rotateY(MoveListener.getRotZ());
	CNELU::Driver->setupModelMatrix (mtx);
	CNELU::Driver->activeIndexBuffer(pbQuad);
	CNELU::Driver->renderTriangles(mat, 0, pbQuad.getNumIndexes()/3);	
}





/*********************************************************\
					displayZone()
\*********************************************************/
void displayZones()
{
	const float zFarStep = 5.0f;
	const float	tileNearStep = 10.0f;
	const float thresholdStep = 0.005f;

	ViewerCfg.TextContext.setHotSpot(CComputedString::MiddleMiddle);
	ViewerCfg.TextContext.setColor(CRGBA(255,255,255));
	ViewerCfg.TextContext.setFontSize(20);
	
	CNELU::clearBuffers(CRGBA(0,0,0));
	CNELU::swapBuffers();



	// Create landscape
	CNELU::clearBuffers(CRGBA(0,0,0));
	ViewerCfg.TextContext.printfAt(0.5f,0.5f,"Creating landscape...");
	CNELU::swapBuffers();
	
	Landscape = (CLandscapeModel*)CNELU::Scene->createModel(LandscapeModelId);
	Landscape->Landscape.setNoiseMode (ViewerCfg.LandscapeNoise);
	Landscape->Landscape.setTileNear(ViewerCfg.LandscapeTileNear);
	Landscape->Landscape.setThreshold(ViewerCfg.LandscapeThreshold);

	Landscape->Landscape.enableAutomaticLighting (ViewerCfg.AutoLight);
	Landscape->Landscape.setupAutomaticLightDir (ViewerCfg.LightDir);

	// Enable Additive Tiles.
	Landscape->enableAdditive(true);

	// HeightField.
	CBitmap		heightBitmap;
	CIFile file(ViewerCfg.HeightFieldName);

	if( ViewerCfg.HeightFieldName!="" && heightBitmap.load(file) )
	{
		CHeightMap	heightMap;
		heightMap.buildFromBitmap(heightBitmap);
		heightMap.MaxZ= ViewerCfg.HeightFieldMaxZ;
		heightMap.OriginX= ViewerCfg.HeightFieldOriginX;
		heightMap.OriginY= ViewerCfg.HeightFieldOriginY;
		heightMap.SizeX = ViewerCfg.HeightFieldSizeX;
		heightMap.SizeY = ViewerCfg.HeightFieldSizeY;
		Landscape->Landscape.setHeightField(heightMap);
	}

	
	// Init TileBank.
	CNELU::clearBuffers(CRGBA(0,0,0));
	ViewerCfg.TextContext.printfAt(0.5f,0.5f,"Initializing TileBanks...");
	CNELU::swapBuffers();

	try 
	{
		CIFile bankFile (ViewerCfg.BanksPath + "/" + ViewerCfg.Bank);
		Landscape->Landscape.TileBank.serial(bankFile);
	}
	catch(const Exception &)
	{
		string tmp = string("Cant load bankfile ")+ViewerCfg.BanksPath + "/" + ViewerCfg.Bank;
		nlerror (tmp.c_str());
	}

	if ((Landscape->Landscape.TileBank.getAbsPath ()=="")&&(ViewerCfg.TilesPath!=""))
		Landscape->Landscape.TileBank.setAbsPath (ViewerCfg.TilesPath + "/");

	if (ViewerCfg.UseDDS)
	{
		Landscape->Landscape.TileBank.makeAllExtensionDDS();
	}

	if (ViewerCfg.AllPathRelative)
		Landscape->Landscape.TileBank.makeAllPathRelative();

	string::size_type idx = ViewerCfg.Bank.find(".");
	string farBank = ViewerCfg.Bank.substr(0,idx);
	farBank += ".farbank";

	try
	{
		CIFile farbankFile(ViewerCfg.BanksPath + "/" + farBank);
		Landscape->Landscape.TileFarBank.serial(farbankFile);
	}
	catch(const Exception &)
	{
		string tmp = string("Cant load bankfile ")+ViewerCfg.BanksPath + "/" + farBank;
		nlerror (tmp.c_str());
	}
	
	if ( ! Landscape->Landscape.initTileBanks() )
	{
		nlwarning( "You need to recompute bank.farbank for the far textures" );
	}
	
	// Init light color
	CNELU::clearBuffers(CRGBA(0,0,0));
	ViewerCfg.TextContext.printfAt(0.5f,0.5f,"Initializing Light...");
	CNELU::swapBuffers();
	
	Landscape->Landscape.setupStaticLight (ViewerCfg.LandDiffuse, ViewerCfg.LandAmbient, 1.1f);

	// Init collision manager
	CollisionManager.init( &(Landscape->Landscape), 200);
	

	// Preload of TileBank
	CNELU::clearBuffers(CRGBA(0,0,0));
	ViewerCfg.TextContext.printfAt(0.5f,0.5f,"Loading TileBank...");
	CNELU::swapBuffers();
	
	for (int ts=0; ts<Landscape->Landscape.TileBank.getTileSetCount (); ts++)
	{
		CTileSet *tileSet=Landscape->Landscape.TileBank.getTileSet (ts);
		sint tl;
		for (tl=0; tl<tileSet->getNumTile128(); tl++)
			Landscape->Landscape.flushTiles (CNELU::Scene->getDriver(), (uint16)tileSet->getTile128(tl), 1);
		for (tl=0; tl<tileSet->getNumTile256(); tl++)
			Landscape->Landscape.flushTiles (CNELU::Scene->getDriver(), (uint16)tileSet->getTile256(tl), 1);
		for (tl=0; tl<CTileSet::count; tl++)
			Landscape->Landscape.flushTiles (CNELU::Scene->getDriver(), (uint16)tileSet->getTransition(tl)->getTile (), 1);
	}
		

	// Build zones.
	CNELU::clearBuffers(CRGBA(0,0,0));
	ViewerCfg.TextContext.printfAt(0.5f,0.5f,"Loading zones...");
	CNELU::swapBuffers();
	uint32 i;
	for(i =0; i<ViewerCfg.Zones.size(); i++)
	{
		CZone zone;
		try
		{
			CIFile file(CPath::lookup(ViewerCfg.Zones[i]));
			zone.serial(file);
			file.close();

			// Add it to landscape.
			Landscape->Landscape.addZone(zone);

			// Add it to collision manager.
			CollisionManager.addZone(zone.getZoneId());
		}
		catch(const Exception &e)
		{
			printf("%s\n", e.what ());
		}		
	}

	// Load instance group.
	CNELU::clearBuffers(CRGBA(0,0,0));
	ViewerCfg.TextContext.printfAt(0.5f,0.5f,"Loading objects...");
	CNELU::swapBuffers();
	for(i =0; i<ViewerCfg.Igs.size(); i++)
	{
		CInstanceGroup *group = new CInstanceGroup;
		try
		{
			CIFile file(CPath::lookup(ViewerCfg.Igs[i]));
			group->serial(file);
			file.close();

			// Add it to the scene.
			group->addToScene (*CNELU::Scene);
		}
		catch(const Exception &e)
		{
			printf("%s\n", e.what ());
		}		
	}
	
	// Init collision Manager.
	CNELU::clearBuffers(CRGBA(0,0,0));
	ViewerCfg.TextContext.printfAt(0.5f,0.5f,"Initializing collision manager...");
	CNELU::swapBuffers();
	
	
	CollisionManager.setCenter(ViewerCfg.Position);
	ViewerCfg.Position.z = 0.0f;
	CollisionManager.snapToGround( ViewerCfg.Position, 1000.0f );

	

	// hide mouse cursor
	CNELU::Driver->showCursor(false);
#ifdef NL_RELEASE
	CNELU::Driver->setCapture(true);
#endif

	
		
	// Events management
	CNELU::EventServer.addEmitter(CNELU::Driver->getEventEmitter());
	CNELU::AsyncListener.addToServer(CNELU::EventServer);
	
	MoveListener.init(CNELU::Scene, ViewerCfg.Width, ViewerCfg.Height, *CNELU::Camera);
	MoveListener.addToServer(CNELU::EventServer);
	MoveListener.setPos( ViewerCfg.Position );	

	CNELU::Camera->setPerspective (float(80.0*Pi/180.0), 1.33f, 0.1f, 1000.0f);
	
	bool showInfos = true;


	// initializing Z-Clip Far
	float left;
	float right;
	float bottom;
	float top; 
	float znear;
	float zfar;
	CNELU::Camera->getFrustum(left, right, bottom, top, znear, zfar);
	zfar = ViewerCfg.ZFar;
	CNELU::Camera->setFrustum(left, right, bottom, top, znear, zfar);
	
	
	do
	{
		// Time mgt.
		//==========
		static sint64 t0 = (sint64)CTime::getLocalTime();
		static sint64 t1 = (sint64)CTime::getLocalTime();
		static sint64 ts = 0;

		t0 = t1;
		t1 = (sint64)CTime::getLocalTime();
		sint64 dt64 = t1-t0;
		ts += dt64;
		float	dt= ((float)dt64)*0.001f;

	
		CNELU::EventServer.pump();

		
		// Manage movement and collision
		MoveListener.setLocalTime(CTime::getLocalTime());
		CVector oldpos = MoveListener.getPos();
		MoveListener.changeViewMatrix();
		if(MoveListener.getMode()==CMoveListener::WALK)
		{
			CVector pos = MoveListener.getPos();
			CollisionManager.snapToGround( pos , 1000.0f );
			MoveListener.setPos( pos );
		}
		CollisionManager.setCenter(MoveListener.getPos()); 

				
		// Change move mode
		if(CNELU::AsyncListener.isKeyPushed(KeySPACE))
		{
			MoveListener.swapMode();	
		}
		
		
		// Change displaying infos state
		if(CNELU::AsyncListener.isKeyPushed(KeyF1))
		{
			showInfos = !showInfos;
		}


		// Change eyes height
		float eh = MoveListener.getEyesHeight();
		if(CNELU::AsyncListener.isKeyPushed(KeyADD))
		{
			ViewerCfg.EyesHeight.z += 0.1f;
			eh += 0.1f;
		}
		if(CNELU::AsyncListener.isKeyPushed(KeySUBTRACT))
		{
			ViewerCfg.EyesHeight.z -= 0.1f;
			eh -= 0.1f;
		}
		if(ViewerCfg.EyesHeight.z<0.1f) ViewerCfg.EyesHeight.z = 0.1f;
		if(eh<0.1f) eh = 0.1f;
		MoveListener.setEyesHeight(eh);


		// Change TileNear
		float tileNear = Landscape->Landscape.getTileNear();
		if(CNELU::AsyncListener.isKeyPushed(KeyHOME))
			tileNear += tileNearStep;
		if(CNELU::AsyncListener.isKeyPushed(KeyEND))
			tileNear -= tileNearStep;
		if(tileNear<0) tileNear = 0;
		Landscape->Landscape.setTileNear(tileNear);


		// Change Z-Far
		CNELU::Camera->getFrustum(left, right, bottom, top, znear, zfar);
		if(CNELU::AsyncListener.isKeyDown(KeyPRIOR))
			zfar += zFarStep;
		if(CNELU::AsyncListener.isKeyDown(KeyNEXT))
			zfar -= zFarStep;
		if(zfar<0) zfar = 0;
		CNELU::Camera->setFrustum(left, right, bottom, top, znear, zfar);


		// Change Threshold
		float threshold = Landscape->Landscape.getThreshold();
		if(CNELU::AsyncListener.isKeyPushed(KeyINSERT))
			threshold += thresholdStep;
		if(CNELU::AsyncListener.isKeyPushed(KeyDELETE))
			threshold -= thresholdStep;
		if(threshold<0.001f) threshold = 0.001f;
		if(threshold>0.1f) threshold = 0.1f;
		Landscape->Landscape.setThreshold(threshold);


		// Switch between wired and filled scene display
		if(CNELU::AsyncListener.isKeyPushed(KeyF3))
		{
			if (CNELU::Driver->getPolygonMode ()==IDriver::Filled)
				CNELU::Driver->setPolygonMode (IDriver::Line);
			else
				CNELU::Driver->setPolygonMode (IDriver::Filled);
		}

		
		// Switch between mouse move and keyboard-only move
		if(CNELU::AsyncListener.isKeyPushed(KeyRETURN))
		{
			MoveListener.changeControlMode();
		}
		

		
		// Render
		//=======
		CNELU::clearBuffers(ViewerCfg.Background);
		CNELU::Driver->clearZBuffer();
		CNELU::Scene->render();

				
		if(showInfos)
		{
			
			// black top quad
			CDRU::drawQuad(0,0.97f,1.0f,1.0f,*CNELU::Driver,CRGBA(0,0,0),CNELU::Scene->getViewport());

			// black bottom quad
			CDRU::drawQuad(0,0,1.0f,0.03f,*CNELU::Driver,CRGBA(0,0,0),CNELU::Scene->getViewport());

			
			ViewerCfg.TextContext.setFontSize(12);
			ViewerCfg.TextContext.setColor(CRGBA(255,255,255));

			// Display fps.
			ViewerCfg.TextContext.printfAt(0.05f,0.98f,"%.1f fps",1/dt);

			// Display ms
			ViewerCfg.TextContext.printfAt(0.12f,0.98f,"%d ms",dt64);

			// Display Tile Near
			ViewerCfg.TextContext.printfAt(0.75f,0.98f,"Tile Near : %.1f",tileNear);

			//Display moving mode
			ViewerCfg.TextContext.setColor(CRGBA(255,0,0));
			switch(MoveListener.getMode())
			{
				case CMoveListener::WALK :
					ViewerCfg.TextContext.printfAt(0.5f,0.98f,"Walk Mode");
					break;
				case CMoveListener::FREE :
					ViewerCfg.TextContext.printfAt(0.5f,0.98f,"Free-Look Mode");
					break;
				default:
					break;
			}
			ViewerCfg.TextContext.setColor(CRGBA(255,255,255));

			// Display Threshold
			ViewerCfg.TextContext.printfAt(0.3f,0.98f,"Threshold : %.3f",threshold);

			// Display Clip Far
			ViewerCfg.TextContext.printfAt(0.92f,0.98f,"Clip Far : %.1f",zfar);

			
			ViewerCfg.TextContext.setHotSpot(CComputedString::MiddleBottom);
			
			// Display current zone name
			CVector pos = MoveListener.getPos();
			string zoneName = getZoneNameByCoord(pos.x, pos.y);
			ViewerCfg.TextContext.printfAt(0.3f,0.01f,"Zone : %s",zoneName.c_str());

			// Position
			ViewerCfg.TextContext.printfAt(0.1f,0.01f,"Position : %d %d %d",(sint)pos.x,(sint)pos.y,(sint)pos.z);

			// Eyes height
			ViewerCfg.TextContext.printfAt(0.7f,0.01f,"Eyes : %.2f m",ViewerCfg.EyesHeight.z);

			// Display speed in km/h
			ViewerCfg.TextContext.setColor(CRGBA(255,0,0));
			ViewerCfg.TextContext.printfAt(0.5f,0.01f,"Speed : %d km/h",(sint)(MoveListener.getSpeed()*3.6f));
			ViewerCfg.TextContext.setColor(CRGBA(255,255,255));

			// Heading
			sint heading = -(sint)(MoveListener.getRotZ()*180/Pi)%360;
			if(heading<0) heading += 360;
			ViewerCfg.TextContext.printfAt(0.9f,0.01f,"Heading : %d degrees",heading);

			// Display the cool compass.
			displayOrientation();
		}

		
		CNELU::swapBuffers();
		CNELU::screenshot();
	}
	while(!CNELU::AsyncListener.isKeyPushed(KeyESCAPE));





	ViewerCfg.Position = MoveListener.getPos();
	
	CNELU::AsyncListener.removeFromServer(CNELU::EventServer);
	MoveListener.removeFromServer(CNELU::EventServer);

	CNELU::Driver->showCursor(true);
#ifdef NL_RELEASE
	CNELU::Driver->setCapture(false);
#endif
}




/****************************************************************\
							writeConfigFile
\****************************************************************/
void writeConfigFile(const char * configFileName)
{
	FILE * f = fopen(configFileName,"wt");

	if(f==NULL)
	{
		nlerror("can't open file '%s'\n",configFileName);
	}

	fprintf(f,"FullScreen = %d;\n",ViewerCfg.Windowed?0:1);
	fprintf(f,"Width = %d;\n",ViewerCfg.Width);
	fprintf(f,"Height = %d;\n",ViewerCfg.Height);
	fprintf(f,"Depth = %d;\n",ViewerCfg.Depth);
	fprintf(f,"Position = { %f, %f, %f };\n", ViewerCfg.Position.x,ViewerCfg.Position.y,ViewerCfg.Position.z);
	fprintf(f,"EyesHeight = %f;\n", ViewerCfg.EyesHeight.z);
	fprintf(f,"Background = { %d, %d, %d };\n", ViewerCfg.Background.R,ViewerCfg.Background.G,ViewerCfg.Background.B);
	fprintf(f,"ZFar = %f;\n", ViewerCfg.ZFar);

	fprintf(f,"AutoLight = %d;\n", ViewerCfg.AutoLight?1:0);
	fprintf(f,"LightDir = { %f, %f, %f };\n", ViewerCfg.LightDir.x, ViewerCfg.LightDir.y, ViewerCfg.LightDir.z);
	fprintf(f,"LandscapeTileNear = %f;\n", ViewerCfg.LandscapeTileNear);
	fprintf(f,"LandscapeThreshold = %f;\n", ViewerCfg.LandscapeThreshold);
	fprintf(f,"LandscapeNoise = %d;\n", (int)ViewerCfg.LandscapeNoise);
	fprintf(f,"BanksPath = \"%s\";\n",ViewerCfg.BanksPath.c_str());
	fprintf(f,"TilesPath = \"%s\";\n",ViewerCfg.TilesPath.c_str());
	fprintf(f,"UseDDS = \"%d\";\n",ViewerCfg.UseDDS?1:0);
	fprintf(f,"AllPathRelative = \"%d\";\n",ViewerCfg.AllPathRelative?1:0);
	fprintf(f,"Bank = \"%s\";\n",ViewerCfg.Bank.c_str());
	fprintf(f,"ZonesPath = \"%s\";\n",ViewerCfg.ZonesPath.c_str());
	fprintf(f,"IgPath = \"%s\";\n",ViewerCfg.IgPath.c_str());
	fprintf(f,"ShapePath = \"%s\";\n",ViewerCfg.ShapePath.c_str());
	fprintf(f,"MapsPath = \"%s\";\n",ViewerCfg.MapsPath.c_str());
	fprintf(f,"FontPath = \"%s\";\n",ViewerCfg.FontPath.c_str());

	fprintf(f,"HeightFieldName = \"%s\";\n", ViewerCfg.HeightFieldName.c_str());
	fprintf(f,"HeightFieldMaxZ = %f;\n", ViewerCfg.HeightFieldMaxZ);
	fprintf(f,"HeightFieldOriginX = %f;\n", ViewerCfg.HeightFieldOriginX);
	fprintf(f,"HeightFieldOriginY = %f;\n", ViewerCfg.HeightFieldOriginY);
	fprintf(f,"HeightFieldSizeX = %f;\n", ViewerCfg.HeightFieldSizeX);
	fprintf(f,"HeightFieldSizeY = %f;\n", ViewerCfg.HeightFieldSizeY);

	fprintf(f,"LandAmbient = { %d, %d, %d };\n", ViewerCfg.LandAmbient.R,ViewerCfg.LandAmbient.G,ViewerCfg.LandAmbient.B);
	fprintf(f,"LandDiffuse = { %d, %d, %d };\n", ViewerCfg.LandDiffuse.R,ViewerCfg.LandDiffuse.G,ViewerCfg.LandDiffuse.B);

	fprintf(f,"Zones = {\n");
	fprintf(f,"};\n");

	fprintf(f,"Ig = {\n");
	fprintf(f,"};\n");

	fclose(f);
}



/****************************************************************\
						init()
\****************************************************************/
void initViewerConfig(const char * configFileName)
{
	FILE * f = fopen(configFileName,"rt");
	if(f==NULL)
	{
		nlwarning("'%s' not found, default values used", configFileName);
		writeConfigFile(configFileName);
	}
	else fclose (f);
	
	try
	{
		CConfigFile cf;
	
		cf.load(configFileName);
	
		CConfigFile::CVar &cvFullScreen = cf.getVar("FullScreen");
		ViewerCfg.Windowed = cvFullScreen.asInt() ? false : true;
		
		CConfigFile::CVar &cvWidth = cf.getVar("Width");
		ViewerCfg.Width = cvWidth.asInt();

		CConfigFile::CVar &cvHeight = cf.getVar("Height");
		ViewerCfg.Height = cvHeight.asInt();

		CConfigFile::CVar &cvDepth = cf.getVar("Depth");
		ViewerCfg.Depth = cvDepth.asInt();

		CConfigFile::CVar &cvPosition = cf.getVar("Position");
		nlassert(cvPosition.size()==3);
		ViewerCfg.Position.x = cvPosition.asFloat(0);
		ViewerCfg.Position.y = cvPosition.asFloat(1);
		ViewerCfg.Position.z = cvPosition.asFloat(2);

		CConfigFile::CVar &cvEyesHeight = cf.getVar("EyesHeight");
		ViewerCfg.EyesHeight = CVector(0,0,cvEyesHeight.asFloat());

		CConfigFile::CVar &cvBackColor = cf.getVar("Background");
		nlassert(cvBackColor.size()==3);
		ViewerCfg.Background.R = cvBackColor.asInt(0);
		ViewerCfg.Background.G = cvBackColor.asInt(1);
		ViewerCfg.Background.B = cvBackColor.asInt(2);

		CConfigFile::CVar &cvZFar = cf.getVar("ZFar");
		ViewerCfg.ZFar = cvZFar.asFloat();

		CConfigFile::CVar &cvAutoLight = cf.getVar("AutoLight");
		ViewerCfg.AutoLight = cvAutoLight.asInt() ? true : false;

		CConfigFile::CVar &cvLightDir = cf.getVar("LightDir");
		nlassert(cvLightDir.size()==3);
		ViewerCfg.LightDir.x = cvLightDir.asFloat(0);
		ViewerCfg.LightDir.y = cvLightDir.asFloat(1);
		ViewerCfg.LightDir.z = cvLightDir.asFloat(2);

		CConfigFile::CVar &cvLandscapeTileNear = cf.getVar("LandscapeTileNear");
		ViewerCfg.LandscapeTileNear = cvLandscapeTileNear.asFloat();

		CConfigFile::CVar &cvLandscapeThreshold = cf.getVar("LandscapeThreshold");
		ViewerCfg.LandscapeThreshold = cvLandscapeThreshold.asFloat();

		CConfigFile::CVar &cvLandscapeNoise = cf.getVar("LandscapeNoise");
		ViewerCfg.LandscapeNoise = cvLandscapeNoise.asInt() != 0;

		CConfigFile::CVar &cvBanksPath = cf.getVar("BanksPath");
		ViewerCfg.BanksPath = cvBanksPath.asString();

		CConfigFile::CVar &cvTilesPath = cf.getVar("TilesPath");
		ViewerCfg.TilesPath = cvTilesPath.asString();

		CConfigFile::CVar &cvUseDDS = cf.getVar("UseDDS");
		ViewerCfg.UseDDS = cvUseDDS.asInt() ? true : false;

		CConfigFile::CVar &cvAllPathRelative = cf.getVar("AllPathRelative");
		ViewerCfg.AllPathRelative = cvAllPathRelative.asInt() ? true : false;

		CConfigFile::CVar &cvBank = cf.getVar("Bank");
		ViewerCfg.Bank = cvBank.asString();
		
		CConfigFile::CVar &cvZonesPath = cf.getVar("ZonesPath");
		ViewerCfg.ZonesPath = cvZonesPath.asString();
		CPath::addSearchPath(cvZonesPath.asString());

		CConfigFile::CVar &cvIgPath = cf.getVar("IgPath");
		ViewerCfg.IgPath = cvIgPath.asString();
		CPath::addSearchPath(cvIgPath.asString());

		CConfigFile::CVar &cvShapePath = cf.getVar("ShapePath");
		ViewerCfg.ShapePath = cvShapePath.asString();
		CPath::addSearchPath(cvShapePath.asString());

		CConfigFile::CVar &cvMapsPath = cf.getVar("MapsPath");
		ViewerCfg.MapsPath = cvMapsPath.asString();
		CPath::addSearchPath(cvMapsPath.asString());

		CConfigFile::CVar &cvFontPath = cf.getVar("FontPath");
		ViewerCfg.FontPath = cvFontPath.asString();

		CConfigFile::CVar &cvHeightFieldName = cf.getVar("HeightFieldName");
		ViewerCfg.HeightFieldName = cvHeightFieldName.asString();
		
		CConfigFile::CVar &cvHeightFieldMaxZ = cf.getVar("HeightFieldMaxZ");
		ViewerCfg.HeightFieldMaxZ = cvHeightFieldMaxZ.asFloat();

		CConfigFile::CVar &cvHeightFieldOriginX = cf.getVar("HeightFieldOriginX");
		ViewerCfg.HeightFieldOriginX = cvHeightFieldOriginX.asFloat();

		CConfigFile::CVar &cvHeightFieldOriginY = cf.getVar("HeightFieldOriginY");
		ViewerCfg.HeightFieldOriginY = cvHeightFieldOriginY.asFloat();

		CConfigFile::CVar &cvHeightFieldSizeX = cf.getVar("HeightFieldSizeX");
		ViewerCfg.HeightFieldSizeX = cvHeightFieldSizeX.asFloat();

		CConfigFile::CVar &cvHeightFieldSizeY = cf.getVar("HeightFieldSizeY");
		ViewerCfg.HeightFieldSizeY = cvHeightFieldSizeY.asFloat();


		CConfigFile::CVar &cvLandAmb = cf.getVar("LandAmbient");
		nlassert(cvLandAmb.size()==3);
		ViewerCfg.LandAmbient.R = cvLandAmb.asInt(0);
		ViewerCfg.LandAmbient.G = cvLandAmb.asInt(1);
		ViewerCfg.LandAmbient.B = cvLandAmb.asInt(2);

		CConfigFile::CVar &cvLandDiff = cf.getVar("LandDiffuse");
		nlassert(cvLandDiff.size()==3);
		ViewerCfg.LandDiffuse.R = cvLandDiff.asInt(0);
		ViewerCfg.LandDiffuse.G = cvLandDiff.asInt(1);
		ViewerCfg.LandDiffuse.B = cvLandDiff.asInt(2);


		CConfigFile::CVar &cvZones = cf.getVar("Zones");
		for(uint i=0; i<cvZones.size(); i++)
		{
			ViewerCfg.Zones.push_back(cvZones.asString(i));
		}

		CConfigFile::CVar &cvIgs = cf.getVar("Ig");
		for(uint i=0; i<cvIgs.size(); i++)
		{
			ViewerCfg.Igs.push_back(cvIgs.asString(i));
		}

	}
	catch (const EConfigFile &e)
	{
		nlerror("Problem in config file : %s\n", e.what ());
	}

}

/****************************************************************\
							MAIN
\****************************************************************/
#ifdef NL_OS_WINDOWS
int WINAPI WinMain(HINSTANCE /* hInstance */, HINSTANCE /* hPrevInstance */, LPSTR cmdline, int /* nCmdShow */)
{
#else
int main(int /* argc */, char ** /* argv */)
{
#endif
	try
	{
		NLMISC::CApplicationContext myApplicationContext;

#ifdef NL_OS_UNIX
		NLMISC::CPath::addSearchPath(NLMISC::CPath::getApplicationDirectory("NeL"));
#endif // NL_OS_UNIX

		NLMISC::CPath::addSearchPath(NL_ZVIEWER_CFG);

		initViewerConfig("zviewer.cfg");

		// Init NELU
		NL3D::CNELU::init(ViewerCfg.Width, ViewerCfg.Height, CViewport(), ViewerCfg.Depth, ViewerCfg.Windowed, EmptyWindow, false, false);
		NL3D::CNELU::Driver->setWindowTitle(ucstring("NeL ZViewer"));
		NL3D::CNELU::Camera->setTransformMode(ITransformable::DirectMatrix);

		// Init the font manager
		ViewerCfg.TextContext.init (CNELU::Driver, &ViewerCfg.FontManager);
		ViewerCfg.TextContext.setFontGenerator(ViewerCfg.FontPath);
		ViewerCfg.TextContext.setFontSize(12);
		ViewerCfg.FontManager.setMaxMemory(2000000);

		displayZones();
			
		// release nelu
		NL3D::CNELU::release();
	}
	catch (const Exception &e)
	{
		nlerror("main trapped an exception: '%s'", e.what ());
	}

	return 0;
}
