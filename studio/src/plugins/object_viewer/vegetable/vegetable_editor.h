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

#ifndef VEGETABLE_EDITOR_H
#define VEGETABLE_EDITOR_H

// STL includes
#include <string>
#include <vector>

// NeL includes
#include <nel/misc/types_nl.h>
#include <nel/misc/rgba.h>
#include <nel/misc/vector.h>

// Project includes
#include "vegetable_node.h"

// Defaults Sliders ranges.
#define NL_VEGETABLE_EDIT_DEFAULT_MAX_DENSITY 50.f

// General Frequence
#define	NL_VEGETABLE_FREQ_RANGE_MIN	0.0001f
#define	NL_VEGETABLE_FREQ_RANGE_MAX	1.f
#define	NL_VEGETABLE_FREQ_DEFAULT	0.1f

// Density
#define	NL_VEGETABLE_DENSITY_ABS_RANGE_MIN	-10.f
#define	NL_VEGETABLE_DENSITY_ABS_RANGE_MAX	10.f
#define	NL_VEGETABLE_DENSITY_RAND_RANGE_MIN	0.f
#define	NL_VEGETABLE_DENSITY_RAND_RANGE_MAX	10.f
#define	NL_VEGETABLE_DENSITY_ABS_DEFAULT	0.f
#define	NL_VEGETABLE_DENSITY_RAND_DEFAULT	0.25f

// BendPhase
#define	NL_VEGETABLE_BENDPHASE_RANGE_MIN	0.f
#define	NL_VEGETABLE_BENDPHASE_RANGE_MAX	2.f
#define	NL_VEGETABLE_BENDPHASE_ABS_DEFAULT	0.f
#define	NL_VEGETABLE_BENDPHASE_RAND_DEFAULT	2.f

// BendFactor
#define	NL_VEGETABLE_BENDFACTOR_RANGE_MIN	0.f
#define	NL_VEGETABLE_BENDFACTOR_RANGE_MAX	1.f
#define	NL_VEGETABLE_BENDFACTOR_ABS_DEFAULT	0.5f
#define	NL_VEGETABLE_BENDFACTOR_RAND_DEFAULT	0.5f

// ColorNoise
#define	NL_VEGETABLE_COLOR_RANGE_MIN		-1.f
#define	NL_VEGETABLE_COLOR_RANGE_MAX		3.f
#define	NL_VEGETABLE_COLOR_ABS_DEFAULT		-1.f
#define	NL_VEGETABLE_COLOR_RAND_DEFAULT		3.f

// Scale
#define	NL_VEGETABLE_SCALE_RANGE_MIN		0.f
#define	NL_VEGETABLE_SCALE_RANGE_MAX		1.f
#define	NL_VEGETABLE_SCALE_ABS_DEFAULT		0.5f
#define	NL_VEGETABLE_SCALE_RAND_DEFAULT		0.5f

// Rotate
#define	NL_VEGETABLE_ROTATE_RANGE_MIN		-90.f
#define	NL_VEGETABLE_ROTATE_RANGE_MAX		90.f
#define	NL_VEGETABLE_ROTATEX_ABS_DEFAULT	-20.f
#define	NL_VEGETABLE_ROTATEX_RAND_DEFAULT	40.f
#define	NL_VEGETABLE_ROTATEY_ABS_DEFAULT	0.f
#define	NL_VEGETABLE_ROTATEY_RAND_DEFAULT	0.f
#define	NL_VEGETABLE_ROTATEZ_ABS_DEFAULT	0.f
#define	NL_VEGETABLE_ROTATEZ_RAND_DEFAULT	3000.f
#define	NL_VEGETABLE_ROTATEZ_FREQ_DEFAULT	10.f

// BendFreq
#define	NL_VEGETABLE_BENDFREQ_RANGE_MIN		0.f
#define	NL_VEGETABLE_BENDFREQ_RANGE_MAX		4.f

namespace NL3D
{
class CTileVegetableDesc;
class CLandscapeModel;
class CVisualCollisionManager;
class CVisualCollisionEntity;
class CScene;
class IDriver;
}


namespace NLQT
{
/**
@class CVegetableEditor
@brief Loading and viewing .zonel, .vegetset files.
Dynamic control of parameters of the landscape (wind/colors) and
list micro-vegetation node (load/save veget, create/add/ins/del list micro-vegetation node).
*/
class CVegetableEditor
{
public:
	CVegetableEditor(void);
	~CVegetableEditor(void);

	void init();
	void release();

	/// @name Landscape control
	//@{

	/// @return true if landscape is created
	bool isVegetableLandscapeCreated() const
	{
		return _VegetableLandscape != NULL;
	}

	/// Load the landscape with help of setup in object_viewer_qt.cfg. return true if OK.
	bool createVegetableLandscape();

	/// If created, show the landscape
	void showVegetableLandscape();

	/// If created, hide the landscape
	void hideVegetableLandscape();

	/// Display vegetable with landscape
	void enableLandscapeVegetable(bool enable);

	/// Refresh the vegetables in landscape, with the new vegetableSet
	void refreshVegetableLandscape(const NL3D::CTileVegetableDesc &tvdesc);

	/// Get vegetable Wind wetup.
	double getVegetableWindPower() const
	{
		return _VegetableWindPower;
	}
	double getVegetableWindBendStart() const
	{
		return _VegetableWindBendMin;
	}
	double getVegetableWindFrequency() const
	{
		return _VegetableWindFreq;
	}

	/// Set vegetable Wind wetup (updat view if possible)
	void setVegetableWindPower(double w);
	void setVegetableWindBendStart(double w);
	void setVegetableWindFrequency(double w);

	void setVegetableAmbientLight(const NLMISC::CRGBA &ambient);
	void setVegetableDiffuseLight(const NLMISC::CRGBA &diffuse);

	NLMISC::CRGBA getVegetableAmbientLight() const
	{
		return _VegetableLandscapeAmbient;
	}
	NLMISC::CRGBA getVegetableDiffuseLight() const
	{
		return _VegetableLandscapeDiffuse;
	}

	/// If enable, snap the camera to the ground of the landscape.
	void snapToGroundVegetableLandscape(bool enable);
	//@}

	/// @name Vegetable control
	//@{

	/// Refresh vegetable display even if box unchecked.
	void refreshVegetableDisplay();

	/// Load a vegetSet
	bool loadVegetableSet(NL3D::CTileVegetableDesc &vegetSet, std::string fileName);

	/// Build the vegetSet from the current _Vegetables
	/// NB: transform Rotate Angle in Radians.
	/// @param keepDefaultShapeName - if true, then vegetables with a ShapeName=="" are kept.
	/// @param keepHiden - if true, then vegetables maked as hiden in ObjectViewer are kept.
	void buildVegetableSet(NL3D::CTileVegetableDesc &vegetSet, bool keepDefaultShapeName = true, bool keepHiden = true );

	/// Append the vegetSet to the current _Vegetables
	/// NB: transform Rotate Angle in Degrees.
	void appendVegetableSet(NL3D::CTileVegetableDesc &vegetSet);

	/// Clear all vegetables.
	void clearVegetables();

	/// Get full list vegetables from the landscape
	/// @param listVeget - ref of return list vegetables
	void getListVegetables(std::vector<std::string> &listVeget);

	/// Create and add veget node to list
	uint addVegetDesc(const NL3D::CVegetable &vegetable);

	/// Create empty veget node and insert to list
	void insEmptyVegetDesc(uint row);

	/// Remove veget node from the list
	void delVegetDesc(uint id);

	CVegetableNode *getVegetable(sint id);
	//@}

	/// Update snap the camera to the ground of the landscape (if enabled snapToGroundVegetableLandscape()).
	void update();

	NL3D::IDriver *getDriver() const
	{
		return _Driver;
	}

	NL3D::CScene *getScene() const
	{
		return _Scene;
	}

private:
	void loadConfig();
	void loadLandscapeSetup();
	void saveConfig();

	NL3D::CLandscapeModel *_VegetableLandscape;

	// File info to build it
	std::string	_VegetableLandscapeTileBank;
	std::string	_VegetableLandscapeTileFarBank;
	std::vector<std::string> _VegetableLandscapeZoneNames;
	std::string	_CoarseMeshTexture;

	// Misc.
	double _VegetableLandscapeThreshold;
	double _VegetableLandscapeTileNear;
	double _VegetableLandscapeMultiply;
	NLMISC::CRGBA _VegetableLandscapeAmbient;
	NLMISC::CRGBA _VegetableLandscapeDiffuse;
	std::string	_VegetableTexture;
	NLMISC::CRGBA _VegetableAmbient;
	NLMISC::CRGBA _VegetableDiffuse;
	NLMISC::CVector	_VegetableLightDir;
	// Vegetable wind.
	NLMISC::CVector	_VegetableWindDir;
	double _VegetableWindFreq;
	double _VegetableWindPower;
	double _VegetableWindBendMin;

	bool _VegetableEnabled;

	// Collision
	bool _VegetableSnapToGround;
	float _VegetableSnapHeight;
	NL3D::CVisualCollisionManager *_VegetableCollisionManager;
	NL3D::CVisualCollisionEntity *_VegetableCollisionEntity;

	// The vegetable List.
	std::vector<CVegetableNode>	_Vegetables;

	NL3D::IDriver *_Driver;
	NL3D::CScene *_Scene;

}; /* class CVegetableEditor */

} /* namespace NLQT */

#endif // VEGETABLE_EDITOR_H
