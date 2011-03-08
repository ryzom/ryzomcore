/*
    Object Viewer Qt
    Copyright (C) 2010 Dzmitry Kamiahin <dnk-88@tut.by>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "stdpch.h"
#include "vegetable_editor.h"

// Qt includes
#include <QtGui/QProgressDialog>
#include <QtGui/QMessageBox>

// NeL includes
#include <nel/3d/landscape_model.h>
#include <nel/3d/visual_collision_manager.h>
#include "nel/misc/file.h"
#include <nel/3d/driver.h>
#include <nel/3d/scene_user.h>
#include <nel/3d/scene.h>

//#include <nel/3d/u_landscape.h>

// Project includes
#include "modules.h"
#include <nel/3d/u_camera.h>

namespace NLQT
{

CVegetableEditor::CVegetableEditor(void)
	: _VegetableLandscape(NULL),
	  _VegetableCollisionManager(NULL),
	  _VegetableCollisionEntity(NULL),
	  _Driver(NULL), _Scene(NULL)
{
	loadConfig();
}

CVegetableEditor::~CVegetableEditor(void)
{
	saveConfig();
}

void CVegetableEditor::init()
{
	//H_AUTO2
	nldebug("CVegetableEditor::init");

	NL3D::CDriverUser *driver = dynamic_cast<NL3D::CDriverUser *>(Modules::objView().getDriver());
	_Driver = driver->getDriver();

	NL3D::CSceneUser *scene = dynamic_cast<NL3D::CSceneUser *>(Modules::objView().getScene());
	_Scene = &scene->getScene();
}

void CVegetableEditor::release()
{
	//H_AUTO2
	nldebug("CVegetableEditor::release");

	if(_VegetableCollisionEntity)
	{
		_VegetableCollisionManager->deleteEntity(_VegetableCollisionEntity);
		_VegetableCollisionEntity= NULL;
	}
	if(_VegetableCollisionManager)
	{
		delete _VegetableCollisionManager;
		_VegetableCollisionManager= NULL;
	}

	// delete Landscape
	if(_VegetableLandscape)
	{
		Modules::veget().getScene()->deleteModel(_VegetableLandscape);
		_VegetableLandscape= NULL;
	}
}

bool CVegetableEditor::createVegetableLandscape()
{
	/// TODO: switch from C to U classes, if possible
	// If not already done.
	if(!_VegetableLandscape)
	{
		// load general landscape param
		loadLandscapeSetup();

		// create the landscape.
		_VegetableLandscape= static_cast<NL3D::CLandscapeModel *>(Modules::veget().getScene()->createModel(NL3D::LandscapeModelId));

		// create progress dialog
		QProgressDialog progress("Loading TileBanks....", "Cancel", 0, 100);
		progress.show();
		progress.setWindowModality(Qt::WindowModal);
		progress.setValue(0);
		try
		{
			if((_VegetableLandscapeTileBank.empty()) || (_VegetableLandscapeTileFarBank.empty()) || (_VegetableLandscapeZoneNames.empty()))
			{
				throw NLMISC::Exception("Landscape section object_viewer_qt.cfg not fully defined");
			}

			// Load The Bank files (copied from CLandscapeUser :) ).
			// ================
			// load
			NLMISC::CIFile bankFile(NLMISC::CPath::lookup(_VegetableLandscapeTileBank));
			_VegetableLandscape->Landscape.TileBank.serial(bankFile);
			_VegetableLandscape->Landscape.TileBank.makeAllPathRelative();
			_VegetableLandscape->Landscape.TileBank.makeAllExtensionDDS();
			_VegetableLandscape->Landscape.TileBank.setAbsPath ("");

			// load
			NLMISC::CIFile farbankFile(NLMISC::CPath::lookup(_VegetableLandscapeTileFarBank));
			_VegetableLandscape->Landscape.TileFarBank.serial(farbankFile);
			if ( ! _VegetableLandscape->Landscape.initTileBanks() )
			{
				nlwarning( "You need to recompute bank.farbank for the far textures" );
			}
			bankFile.close();
			farbankFile.close();

			// flushTiles.
			// ================
			if(Modules::veget().getDriver())
			{
				// progress
				progress.setLabelText("Start loading Tiles...");
				// count nbText to load.
				sint	ts;
				sint	nbTextTotal = 0;
				for (ts=0; ts < _VegetableLandscape->Landscape.TileBank.getTileSetCount (); ++ts)
				{
					NL3D::CTileSet *tileSet =_VegetableLandscape->Landscape.TileBank.getTileSet (ts);
					nbTextTotal += tileSet->getNumTile128();
					nbTextTotal += tileSet->getNumTile256();
					nbTextTotal += NL3D::CTileSet::count;
				}

				// load.
				sint nbTextDone= 0;
				for (ts=0; ts < _VegetableLandscape->Landscape.TileBank.getTileSetCount (); ++ts)
				{
					NL3D::CTileSet *tileSet=_VegetableLandscape->Landscape.TileBank.getTileSet (ts);
					sint tl;
					for (tl=0; tl<tileSet->getNumTile128(); tl++, nbTextDone++)
					{
						_VegetableLandscape->Landscape.flushTiles (Modules::veget().getDriver(), (uint16)tileSet->getTile128(tl), 1);
						progress.setValue(nbTextDone * 100 / nbTextTotal);
					}
					for (tl=0; tl<tileSet->getNumTile256(); tl++, nbTextDone++)
					{
						_VegetableLandscape->Landscape.flushTiles (Modules::veget().getDriver(), (uint16)tileSet->getTile256(tl), 1);
						progress.setValue(nbTextDone * 100 / nbTextTotal);
					}
					for (tl=0; tl < NL3D::CTileSet::count; tl++, nbTextDone++)
					{
						_VegetableLandscape->Landscape.flushTiles (Modules::veget().getDriver(), (uint16)tileSet->getTransition(tl)->getTile (), 1);
						progress.setValue(nbTextDone * 100 / nbTextTotal);
					}
				}
				progress.setValue(100);
			}

			// misc setup.
			// ================
			_VegetableLandscape->Landscape.setThreshold(_VegetableLandscapeThreshold);
			_VegetableLandscape->Landscape.setTileNear(_VegetableLandscapeTileNear);
			_VegetableLandscape->Landscape.setupStaticLight(_VegetableLandscapeDiffuse, _VegetableLandscapeAmbient, 1);
			_VegetableLandscape->Landscape.loadVegetableTexture(_VegetableTexture);
			_VegetableLandscape->Landscape.setupVegetableLighting(_VegetableAmbient, _VegetableDiffuse, _VegetableLightDir);
			_VegetableLandscape->Landscape.setVegetableWind(_VegetableWindDir, _VegetableWindFreq, _VegetableWindPower, _VegetableWindBendMin);
			_VegetableLandscape->Landscape.setUpdateLightingFrequency(1);
			_VegetableLandscape->enableAdditive(true);
			// Load the zones.
			// ================
			// landscape recentering.
			bool zoneLoaded = false;
			NL3D::CAABBox landscapeBBox;
			// progress
			progress.setLabelText("Start loading Zones...");
			progress.setValue(0);
			uint	nbZones= (uint)_VegetableLandscapeZoneNames.size();
			for(uint i = 0; i < nbZones; i++)
			{
				// open the file
				NLMISC::CIFile zoneFile(NLMISC::CPath::lookup(_VegetableLandscapeZoneNames[i]));
				NL3D::CZone zone;
				// load
				zoneFile.serial(zone);
				// append to landscape
				_VegetableLandscape->Landscape.addZone(zone);
				// progress
				progress.setValue(i * 100 / nbZones);
				// Add to the bbox.
				if(!zoneLoaded)
				{
					zoneLoaded= true;
					landscapeBBox.setCenter(zone.getZoneBB().getCenter());
				}
				else
					landscapeBBox.extend(zone.getZoneBB().getCenter());
			}

			// After All zone loaded, recenter the mouse listener on the landscape.
			if(zoneLoaded)
			{
				NL3D::CMatrix matrix;
				Modules::objView().get3dMouseListener()->setHotSpot(landscapeBBox.getCenter());
				matrix.setPos(landscapeBBox.getCenter());
				matrix.rotateX(-(float)NLMISC::Pi / 4);
				matrix.translate(NLMISC::CVector(0, -1000, 0));
				Modules::objView().get3dMouseListener()->setMatrix(matrix);
			}

			// Create collisions objects.
			_VegetableCollisionManager= new NL3D::CVisualCollisionManager;
			_VegetableCollisionManager->setLandscape(&_VegetableLandscape->Landscape);
			_VegetableCollisionEntity= _VegetableCollisionManager->createEntity();
			progress.setValue(100);
		}
		catch (NLMISC::Exception &e)
		{
			// remove first possibly created collisions objects.
			if(_VegetableCollisionEntity)
			{
				_VegetableCollisionManager->deleteEntity(_VegetableCollisionEntity);
				_VegetableCollisionEntity= NULL;
			}

			if(_VegetableCollisionManager)
			{
				delete _VegetableCollisionManager;
				_VegetableCollisionManager= NULL;
			}

			// remove the landscape
			Modules::veget().getScene()->deleteModel(_VegetableLandscape);

			_VegetableLandscape= NULL;

			QMessageBox::critical(&Modules::mainWin(), "Failed to Load landscape", QString(e.what()), QMessageBox::Ok);

			return false;
		}
	}

	return true;
}

void CVegetableEditor::showVegetableLandscape()
{
	if(_VegetableLandscape)
	{
		_VegetableLandscape->show();
	}
}

void CVegetableEditor::hideVegetableLandscape()
{
	if(_VegetableLandscape)
	{
		_VegetableLandscape->hide();
	}
}

void CVegetableEditor::enableLandscapeVegetable(bool enable)
{
	// update
	_VegetableEnabled = enable;

	// update view.
	if(_VegetableLandscape)
	{
		_VegetableLandscape->Landscape.enableVegetable(_VegetableEnabled);
	}
}

void CVegetableEditor::refreshVegetableLandscape(const NL3D::CTileVegetableDesc &tvdesc)
{
	// if landscape is displayed.
	if(_VegetableLandscape)
	{
		// first disable the vegetable, to delete any vegetation
		_VegetableLandscape->Landscape.enableVegetable(false);

		// Then change all the tileSet of all the TileBanks.
		for (sint ts=0; ts<_VegetableLandscape->Landscape.TileBank.getTileSetCount (); ++ts)
		{
			NL3D::CTileSet *tileSet=_VegetableLandscape->Landscape.TileBank.getTileSet (ts);
			// change the vegetableTileDesc of this tileSet.
			tileSet->setTileVegetableDesc(tvdesc);
		}

		// re-Enable the vegetable (if wanted).
		_VegetableLandscape->Landscape.enableVegetable(_VegetableEnabled);
	}
}

void CVegetableEditor::setVegetableWindPower(float w)
{
	_VegetableWindPower= w;
	if(_VegetableLandscape)
		_VegetableLandscape->Landscape.setVegetableWind(_VegetableWindDir,
				_VegetableWindFreq,
				_VegetableWindPower,
				_VegetableWindBendMin);
}

void CVegetableEditor::setVegetableWindBendStart(float w)
{
	_VegetableWindBendMin= w;
	if(_VegetableLandscape)
		_VegetableLandscape->Landscape.setVegetableWind(_VegetableWindDir,
				_VegetableWindFreq,
				_VegetableWindPower,
				_VegetableWindBendMin);
}

void CVegetableEditor::setVegetableWindFrequency(float w)
{
	_VegetableWindFreq= w;
	if(_VegetableLandscape)
		_VegetableLandscape->Landscape.setVegetableWind(_VegetableWindDir,
				_VegetableWindFreq,
				_VegetableWindPower,
				_VegetableWindBendMin);
}

void CVegetableEditor::snapToGroundVegetableLandscape(bool enable)
{
	// update
	if(_VegetableLandscape)
		_VegetableSnapToGround = enable;
}

void CVegetableEditor::setVegetableAmbientLight(const NLMISC::CRGBA &ambient)
{
	_VegetableLandscapeAmbient = ambient;
	if(_VegetableLandscape)
		_VegetableLandscape->Landscape.setupStaticLight(_VegetableLandscapeDiffuse, _VegetableLandscapeAmbient, 1);
}

void CVegetableEditor::setVegetableDiffuseLight(const NLMISC::CRGBA &diffuse)
{
	_VegetableLandscapeDiffuse = diffuse;
	if(_VegetableLandscape)
		_VegetableLandscape->Landscape.setupStaticLight(_VegetableLandscapeDiffuse, _VegetableLandscapeAmbient, 1);
}

void CVegetableEditor::update()
{
	// Vegetable: manage collision snapping if wanted and possible
	if(_VegetableSnapToGround && _VegetableLandscape)
	{
		// get matrix from camera.
		NLMISC::CMatrix	matrix = Modules::objView().getScene()->getCam().getMatrix();

		// snap To ground.
		NLMISC::CVector pos = matrix.getPos();
		// if succes to snap to ground
		if(_VegetableCollisionEntity->snapToGround(pos))
		{
			pos.z+= _VegetableSnapHeight;
			matrix.setPos(pos);
			// reset the moveListener and the camera.
			Modules::objView().get3dMouseListener()->setMatrix(matrix);
			Modules::objView().getScene()->getCam().setMatrix(matrix);
		}
	}
}

void CVegetableEditor::getListVegetables(std::vector<std::string> &listVeget)
{
	listVeget.clear();
	for (size_t i = 0; i < _Vegetables.size(); i++)
		listVeget.push_back(_Vegetables[i]._vegetableName);
}

bool CVegetableEditor::loadVegetableSet(NL3D::CTileVegetableDesc &vegetSet, std::string fileName)
{
	bool ok = true;
	NLMISC::CIFile f;

	if( f.open(fileName) )
	{
		try
		{
			// read the vegetable
			f.serial(vegetSet);
		}
		catch(NLMISC::EStream &)
		{
			ok = false;
			QMessageBox::critical(&Modules::mainWin(), "NeL Vegetable editor", QString("Failed to load file!"), QMessageBox::Ok);
		}
	}
	else
	{
		ok = false;
		QMessageBox::critical(&Modules::mainWin(), "NeL Vegetable editor", QString("Failed to load file!"), QMessageBox::Ok);
	}

	return ok;
}

void CVegetableEditor::buildVegetableSet(NL3D::CTileVegetableDesc &vegetSet, bool keepDefaultShapeName, bool keepHiden )
{
	vegetSet.clear();
	float degToRad= (float)(NLMISC::Pi / 180.f);

	// build the list.
	std::vector<NL3D::CVegetable> vegetables;
	for(uint i = 0; i < _Vegetables.size(); ++i)
	{
		// if don't want to keep <default> ShapeNames, skip them.
		if(!keepDefaultShapeName && _Vegetables[i]._vegetable->ShapeName == "")
			continue;
		// if don't want to keep hiden vegetables, skip them.
		if(!keepHiden && !_Vegetables[i]._visible)
			continue;

		vegetables.push_back(*_Vegetables[i]._vegetable);
		// get dst index.
		uint	dstId= (uint)vegetables.size()-1;
		// transform degrees in radians.
		vegetables[dstId].Rx.Abs *= degToRad;
		vegetables[dstId].Rx.Rand *= degToRad;
		vegetables[dstId].Ry.Abs *= degToRad;
		vegetables[dstId].Ry.Rand *= degToRad;
		vegetables[dstId].Rz.Abs *= degToRad;
		vegetables[dstId].Rz.Rand *= degToRad;
	}

	// build the set.
	vegetSet.build(vegetables);
}

void CVegetableEditor::appendVegetableSet(NL3D::CTileVegetableDesc &vegetSet)
{
	float	radToDeg = (float)(180.f / NLMISC::Pi);

	// for all distances Types.
	for(uint distType = 0; distType < NL3D_VEGETABLE_BLOCK_NUMDIST; ++distType)
	{
		// retrieve list of vegetable
		const std::vector<NL3D::CVegetable> &vegetList = vegetSet.getVegetableList(distType);

		// for all of them
		for(uint i = 0; i < vegetList.size(); ++i)
		{
			// append the vegetable to the list.
			NL3D::CVegetable veget = vegetList[i];

			// transform radians into degrees.
			veget.Rx.Abs *= radToDeg;
			veget.Rx.Rand *= radToDeg;
			veget.Ry.Abs *= radToDeg;
			veget.Ry.Rand *= radToDeg;
			veget.Rz.Abs *= radToDeg;
			veget.Rz.Rand *= radToDeg;

			// Add a new vegetable to the list.
			_Vegetables.push_back( CVegetableNode());
			uint	id= (uint)_Vegetables.size()-1;
			_Vegetables[id].initVegetable(veget);
		}
	}
}

void CVegetableEditor::clearVegetables()
{
	// delete all vegetables.
	for(uint i=0; i<_Vegetables.size(); i++)
	{
		_Vegetables[i].deleteVegetable();
	}
	_Vegetables.clear();
}

CVegetableNode *CVegetableEditor::getVegetable(sint id)
{
	if(id == -1)
		return NULL;
	else
		return &_Vegetables[id];
}

uint CVegetableEditor::addVegetDesc(const NL3D::CVegetable &vegetable)
{
	// Add a new vegetable to the list.
	_Vegetables.push_back(CVegetableNode ());
	uint id = (uint)_Vegetables.size()-1;
	_Vegetables[id].initVegetable(vegetable);
	return id;
}

void CVegetableEditor::insEmptyVegetDesc(uint row)
{
	_Vegetables.insert(_Vegetables.begin() + row, CVegetableNode());
	_Vegetables[row].initDefaultVegetable();
}

void CVegetableEditor::delVegetDesc(uint id)
{
	// erase the vegetable from the list.
	_Vegetables[id].deleteVegetable();
	_Vegetables.erase(_Vegetables.begin()+id);
}

void CVegetableEditor::refreshVegetableDisplay()
{
	NL3D::CTileVegetableDesc	vegetSet;

	// first build the vegetSet, but don't keep <default> shapeName. and skip Hiden vegetables too
	buildVegetableSet(vegetSet, false, false);

	// then refresh window.
	Modules::veget().refreshVegetableLandscape(vegetSet);
}

void CVegetableEditor::loadConfig()
{
	// vegetable display is true by default.
	_VegetableEnabled = true;
	_VegetableSnapToGround = false;

	// Load Landscape params.
	// --------------
	// threshold
	try
	{
		_VegetableLandscapeThreshold = Modules::config().getConfigFile().getVar("VegetLandscapeThreshold").asFloat();
		// clamp to avoid divide/0.
		_VegetableLandscapeThreshold = std::max(_VegetableLandscapeThreshold, 0.001f);
	}
	catch (NLMISC::EUnknownVar &)
	{
		_VegetableLandscapeThreshold = 0.003f;
	}
	// tilenear
	try
	{
		_VegetableLandscapeTileNear = Modules::config().getConfigFile().getVar("VegetLandscapeTileNear").asFloat();
	}
	catch (NLMISC::EUnknownVar &)
	{
		_VegetableLandscapeTileNear= 50;
	}
	// ambient
	try
	{
		NLMISC::CConfigFile::CVar &color = Modules::config().getConfigFile().getVar("VegetLandscapeAmbient");
		_VegetableLandscapeAmbient.R = color.asInt(0);
		_VegetableLandscapeAmbient.G = color.asInt(1);
		_VegetableLandscapeAmbient.B = color.asInt(2);
		_VegetableLandscapeAmbient.A = color.asInt(3);
	}
	catch (NLMISC::EUnknownVar &)
	{
		_VegetableLandscapeAmbient.set(80, 80, 80, 255);
	}
	// diffuse
	try
	{
		NLMISC::CConfigFile::CVar &color= Modules::config().getConfigFile().getVar("VegetLandscapeDiffuse");
		_VegetableLandscapeDiffuse.R = color.asInt(0);
		_VegetableLandscapeDiffuse.G = color.asInt(1);
		_VegetableLandscapeDiffuse.B = color.asInt(2);
		_VegetableLandscapeDiffuse.A = color.asInt(3);
	}
	catch (NLMISC::EUnknownVar &)
	{
		_VegetableLandscapeDiffuse.set(255, 255, 255, 255);
	}
	// Snapping
	try
	{
		_VegetableSnapHeight = Modules::config().getConfigFile().getVar("VegetLandscapeSnapHeight").asFloat();
	}
	catch (NLMISC::EUnknownVar &)
	{
		_VegetableSnapHeight = 1.70f;
	}


	// Load Vegetable params.
	// --------------

	// vegetable ambient
	try
	{
		NLMISC::CConfigFile::CVar &color = Modules::config().getConfigFile().getVar("VegetAmbient");
		_VegetableAmbient.R = color.asInt(0);
		_VegetableAmbient.G = color.asInt(1);
		_VegetableAmbient.B = color.asInt(2);
	}
	catch (NLMISC::EUnknownVar &)
	{
		_VegetableAmbient.set(80, 80, 80, 255);
	}
	// vegetable diffuse
	try
	{
		NLMISC::CConfigFile::CVar &color = Modules::config().getConfigFile().getVar("VegetDiffuse");
		// setup to behave correclty ie as maxLightFactor:
		sint R = color.asInt(0) - _VegetableAmbient.R;
		NLMISC::clamp(R, 0, 255);
		_VegetableDiffuse.R = R;
		sint G = color.asInt(1) - _VegetableAmbient.G;
		NLMISC::clamp(G, 0, 255);
		_VegetableDiffuse.G = G;
		sint B = color.asInt(2) - _VegetableAmbient.B;
		NLMISC::clamp(B, 0, 255);
		_VegetableDiffuse.B = B;
	}
	catch (NLMISC::EUnknownVar &)
	{
		sint R = 255 - _VegetableAmbient.R;
		NLMISC::clamp(R, 0, 255);
		_VegetableDiffuse.R = R;
		sint G = 255 - _VegetableAmbient.G;
		NLMISC::clamp(G, 0, 255);
		_VegetableDiffuse.G = G;
		sint B = 255 - _VegetableAmbient.B;
		NLMISC::clamp(B, 0, 255);
		_VegetableDiffuse.B = B;
	}
	// vegetable lightDir
	try
	{
		NLMISC::CConfigFile::CVar &var = Modules::config().getConfigFile().getVar("VegetLightDir");
		_VegetableLightDir.x = var.asFloat(0);
		_VegetableLightDir.y = var.asFloat(1);
		_VegetableLightDir.z = var.asFloat(2);
		_VegetableLightDir.normalize();
	}
	catch (NLMISC::EUnknownVar &)
	{
		_VegetableLightDir.set(0, 1, -1);
		_VegetableLightDir.normalize();
	}

	// windDir
	try
	{
		NLMISC::CConfigFile::CVar &var = Modules::config().getConfigFile().getVar("VegetWindDir");
		_VegetableWindDir.x = var.asFloat(0);
		_VegetableWindDir.y = var.asFloat(1);
		_VegetableWindDir.z = var.asFloat(2);
	}
	catch (NLMISC::EUnknownVar &)
	{
		_VegetableWindDir.x = 0.5f;
		_VegetableWindDir.y = 0.5f;
		_VegetableWindDir.z = 0;
	}
	// windFreq
	try
	{
		_VegetableWindFreq = Modules::config().getConfigFile().getVar("VegetWindFreq").asFloat();
	}
	catch (NLMISC::EUnknownVar &)
	{
		_VegetableWindFreq= 0.5;
	}
	// windPower
	try
	{
		_VegetableWindPower = Modules::config().getConfigFile().getVar("VegetWindPower").asFloat();
	}
	catch (NLMISC::EUnknownVar &)
	{
		_VegetableWindPower= 1;
	}
	// windBendMin
	try
	{
		_VegetableWindBendMin = Modules::config().getConfigFile().getVar("VegetWindBendMin").asFloat();
	}
	catch (NLMISC::EUnknownVar &)
	{
		_VegetableWindBendMin= 0;
	}
}

void CVegetableEditor::loadLandscapeSetup()
{
	// Load landscape setup
	// --------------
	try
	{
		// tileBank setup.
		_VegetableLandscapeTileBank = Modules::config().getConfigFile().getVar("VegetTileBank").asString();
		_VegetableLandscapeTileFarBank = Modules::config().getConfigFile().getVar("VegetTileFarBank").asString();
		// zone list.
		_VegetableLandscapeZoneNames.clear();
		NLMISC::CConfigFile::CVar &zones = Modules::config().getConfigFile().getVar("VegetLandscapeZones");
		for (uint i=0; i<(uint)zones.size(); i++)
			_VegetableLandscapeZoneNames.push_back(zones.asString(i).c_str());
	}
	catch (NLMISC::EUnknownVar &)
	{
		_VegetableLandscapeTileBank.clear();
		_VegetableLandscapeTileFarBank.clear();
		_VegetableLandscapeZoneNames.clear();
	}

	// Load Vegetable params.
	// --------------

	// vegetable texture
	try
	{
		_VegetableTexture = Modules::config().getConfigFile().getVar("VegetTexture").asString();
	}
	catch (NLMISC::EUnknownVar &)
	{
		_VegetableTexture= "";
	}
}

void CVegetableEditor::saveConfig()
{
	Modules::config().getConfigFile().getVar("VegetLandscapeThreshold").setAsFloat(_VegetableLandscapeThreshold);
	Modules::config().getConfigFile().getVar("VegetLandscapeTileNear").setAsFloat(_VegetableLandscapeTileNear);

	Modules::config().getConfigFile().getVar("VegetLandscapeAmbient").setAsInt(_VegetableLandscapeAmbient.R, 0);
	Modules::config().getConfigFile().getVar("VegetLandscapeAmbient").setAsInt(_VegetableLandscapeAmbient.G, 1);
	Modules::config().getConfigFile().getVar("VegetLandscapeAmbient").setAsInt(_VegetableLandscapeAmbient.B, 2);
	Modules::config().getConfigFile().getVar("VegetLandscapeAmbient").setAsInt(_VegetableLandscapeAmbient.A, 3);

	Modules::config().getConfigFile().getVar("VegetLandscapeDiffuse").setAsInt(_VegetableLandscapeDiffuse.R, 0);
	Modules::config().getConfigFile().getVar("VegetLandscapeDiffuse").setAsInt(_VegetableLandscapeDiffuse.G, 1);
	Modules::config().getConfigFile().getVar("VegetLandscapeDiffuse").setAsInt(_VegetableLandscapeDiffuse.B, 2);
	Modules::config().getConfigFile().getVar("VegetLandscapeAmbient").setAsInt(_VegetableLandscapeAmbient.A, 3);

	Modules::config().getConfigFile().getVar("VegetWindDir").setAsFloat(_VegetableWindDir.x, 0);
	Modules::config().getConfigFile().getVar("VegetWindDir").setAsFloat(_VegetableWindDir.y, 1);
	Modules::config().getConfigFile().getVar("VegetWindDir").setAsFloat(_VegetableWindDir.z, 2);

	Modules::config().getConfigFile().getVar("VegetWindPower").setAsFloat(_VegetableWindPower);
	Modules::config().getConfigFile().getVar("VegetWindFreq").setAsFloat(_VegetableWindFreq);
	Modules::config().getConfigFile().getVar("VegetWindBendMin").setAsFloat(_VegetableWindBendMin);
}

} /* namespace NLQT */