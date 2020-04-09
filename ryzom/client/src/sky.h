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
#ifndef CL_SKY_H
#define CL_SKY_H

#include "sky_object.h"
#include "time_client.h"
//
#include <vector>


namespace NL3D
{
	class UDriver;
	class UAnimationSet;
	class UPlayListManager;
	class UPlayList;
}

class CSkySheet;



class CSky
{
public:
	// ctor
	CSky();
	// dtor
	~CSky();
	/** init from a sky sheet and a scene
	  * \param unsupportedObjects If not NULL, will be filled with the names of the shapes that the driver cannot render
	  */
	void init(NL3D::UDriver *drv, const CSkySheet &sheet, bool forceFallbaclVersion = false, float numHourInDay = 24.f, std::vector<std::string> *unsupportedObjects = NULL);
	// release all datas, including the sky scene
	void release();
	/** Setup the sky
	  * \param skyScene the sky is rendered fof
	  * \return the number of visible objects
	  */
	uint setup(const CClientDate &date, const CClientDate &animationDate, float weatherLevel, CRGBA fogColor, const NLMISC::CVector &sunLightDir, bool envMapScene);
	// Get the sky scene
	NL3D::UScene	*getScene() { return _Scene; }
	// Get number of objects in the sky
	uint			 getNumObjects() const { return uint(_Objects.size()); }
	const CSkyObject &getObject(uint index) const { return _Objects[index]; }
	// test if the sky has a fog color
	bool			  hasFogColor() const { return _FogColor != NULL; }
	// compute fog color
	NLMISC::CRGBA	 computeFogColor(const CClientDate &date, float weatherLevel) const;
	float			 getWaterEnvMapCameraHeight() const { return _WaterEnvMapCameraHeight; }
	uint8			 getWaterEnvMapAlpha() const { return _WaterEnvMapAlpha; }
private:
	std::vector<CSkyObject> _Objects; // all the object in the sky
	std::vector<NLMISC::CBitmap *>	_Bitmaps; // all the bitmaps for the color lookups
	NL3D::UScene			*_Scene;
	NL3D::UDriver			*_Driver;
	NL3D::UInstanceGroup	*_IG;
	double					 _AnimLengthInSeconds;
	float					 _NumHourInDay;
	NL3D::UAnimationSet		*_AnimationSet;
	NL3D::UPlayListManager	*_PlayListManager;
	NL3D::UPlayList			*_PlayList;
	// Bitmap that gives the lighting of the sky scene depending on weather & hour
	NLMISC::CBitmap			*_AmbientSunLight;
	NLMISC::CBitmap			*_DiffuseSunLight;
	NLMISC::CBitmap			*_FogColor;
	float					_WaterEnvMapCameraHeight;
	uint8					_WaterEnvMapAlpha;
};


// retrieve a bitmap from its name, find it in the map if it has already been built, or load it and add it otherwise
NLMISC::CBitmap *buildSharedBitmap(const std::string &filename,
						   std::map<std::string, NLMISC::CBitmap *> &bitmapByName,
						   std::vector<NLMISC::CBitmap *> &builtBitmaps,
						   bool &alreadyBuilt
						  );

#endif
