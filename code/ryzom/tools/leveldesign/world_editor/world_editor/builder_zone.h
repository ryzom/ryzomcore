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

// This class is interface between all that is displayed in
// display/tools view and the game core

#ifndef BUILDERZONE_H
#define BUILDERZONE_H

// ***************************************************************************

#include "builder_zone_region.h"

#include <string>
#include <vector>

// ***************************************************************************

class CDisplay;
class CToolsZone;

namespace NL3D
{
	class CVertexBuffer;
	class CIndexBuffer;
	class CTextureFile;
	class CTextureMem;
	class ITexture;
}

#define CACHE_RENDER_SIZE (64+2)

// ***************************************************************************
// CDataBase contains the image database for Nel and Windows
// A big texture (called CacheTexture) contains all zone (called SCacheZone)
// There are 64 cache texture of 1024x1024 (it should be enough)
// An element is composed of zones (the number of zones is equal to the number
// of true in the mask (the mask is in the zone bank))

class CDataBase
{
	struct SCacheTexture
	{
		bool									Enabled;
		NLMISC::CSmartPtr<NL3D::CTextureMem>	Texture;
		std::vector<bool>						FreePlace;
		std::vector<uint8>						PtrMem;

		SCacheTexture();
		bool isFull();
	};

	struct SCacheZone
	{
		NL3D::CTextureMem						*CacheTexture;
		NLMISC::CUV								PosUV;
		uint8									PosX, PosY;
	};

	struct SElement
	{
		uint8									SizeX, SizeY;
		std::vector<SCacheZone>					ZonePieces;
		CBitmap									*WinBitmap;
	};

private:

	CBitmap				*convertToWin (NLMISC::CBitmap *pBitmap);
	NL3D::CTextureFile	*loadTexture (const std::string &fileName);
	NLMISC::CBitmap		*loadBitmap (const std::string &fileName);

private:

	SCacheTexture							_CacheTexture[64];
	std::map<std::string, SElement>			_ZoneDBmap;
	NLMISC::CSmartPtr<NL3D::CTextureFile>	_UnusedTexture;

	uint32									_RefSizeX, _RefSizeY;
	uint32									_RefCacheTextureSizeX, _RefCacheTextureSizeY;
	uint32									_RefCacheTextureNbEltX, _RefCacheTextureNbEltY;

public:

	CDataBase (uint bitmapSize);
	~CDataBase();

	bool				initFromPath (const std::string &Path);
	void				reset ();
	bool				init (const std::string &Path, NLLIGO::CZoneBank &zb);

	CBitmap				*getBitmap (const std::string &ZoneName);
	NL3D::ITexture		*getTexture (const std::string &ZoneName, sint32 nPosX, sint32 nPosY, 
									NLMISC::CUV &retUVmin, NLMISC::CUV &retUVmax);
};

// ***************************************************************************
// CBuilderZone contains all the shared data between the tools and the engine
// ZoneBank contains the macro zones that is composed of several zones plus a mask
// DataBase contains the graphics for the zones
class CBuilderZone
{
public:
	std::string					_LastPathName;

	NLLIGO::CZoneBank			_ZoneBank;

	CDataBase					_DataBase;

	std::vector<CBuilderZoneRegion*>	_ZoneRegions;

	sint32								_ZoneRegionSelected;

	sint32						_MinX, _MaxX, _MinY, _MaxY;
	std::vector<bool>			_ZoneMask;

	CDisplay					*_Display;
	CToolsZone					*_ToolsZone;

public:

	std::string _FilterType1, _FilterValue1;
	std::string _FilterType2, _FilterValue2;
	uint8		_FilterOperator2;				// 0 -> AND, 1 -> OR
	std::string _FilterType3, _FilterValue3;
	uint8		_FilterOperator3;				// 0 -> AND, 1 -> OR
	std::string _FilterType4, _FilterValue4;
	uint8		_FilterOperator4;				// 0 -> AND, 1 -> OR

	bool		_RandomSelection;
	bool		_CycleSelection;
	uint8		_ApplyCycleSelection;
	sint32		_CurSelectedZone;

	bool		_NotPropagate;
	bool		_Force;

	uint8		_ApplyRot;
	uint8		_ApplyRotType;	// (0-Normal)(1-Random)(2-Cycle)
	uint8		_ApplyRotCycle;

	uint8		_ApplyFlip;
	uint8		_ApplyFlipType;	// (0-Normal)(1-Random)(2-Cycle)
	uint8		_ApplyFlipCycle;

	std::vector<NLLIGO::CZoneBankElement*> _CurrentSelection;

private:

	void				calcMask();
	bool				initZoneBank (const std::string &Path);

public:

	CBuilderZone		(uint bitmapSize);

	bool				init (const std::string &sPath, bool bMakeAZone, CDisplay *display);

	void				setToolsZone (CToolsZone *pTool);
	void				updateToolsZone ();
	bool				refresh (); // Full name
	void				newZone (bool bDisplay=true);
	void				unload (uint32 i);
	void				move (sint32 x, sint32 y);
	uint32				countZones ();
	void				snapshot (const char *fileName, uint sizeSource, bool grayscale);
	void				snapshotCustom (const char *fileName, uint width, uint height, bool keepRatio, uint sizeSource, bool grayscale);

	void				add (const NLMISC::CVector &worldPos);
	void				addTransition (const NLMISC::CVector &worldPos);
	void				del (const NLMISC::CVector &worldPos);

	// Accessors
	NLLIGO::CZoneBank	&getZoneBank () { return _ZoneBank; }

	void				render (const NLMISC::CVector &viewMin, const NLMISC::CVector &viewMax);
	void				renderTransition (const NLMISC::CVector &viewMin, const NLMISC::CVector &viewMax);
	void				displayGrid (const NLMISC::CVector &viewMin, const NLMISC::CVector &viewMax);

	uint32				getNbZoneRegion ();
	std::string			getZoneName (sint32 x, sint32 y);

	uint8				getRot (sint32 x, sint32 y);
	uint8				getFlip (sint32 x, sint32 y);
	CBuilderZoneRegion*	getPtrCurZoneRegion ();
	uint32				getCurZoneRegion ();
	void				setCurZoneRegion (uint32 i);
	bool				getZoneMask (sint32 x, sint32 y);

	/* void				generate (sint32 nMinX, sint32 nMinY, sint32 nMaxX, sint32 nMaxY, 
								sint32 nZoneBaseX, sint32 nZoneBaseY, const char *MaterialString); */

private:

	// SCacheRender is a simple structure to store triangles for each texture in the scene
	struct SCacheRender
	{
		bool					Used;
		NL3D::CVertexBuffer		VB;
		NL3D::CIndexBuffer		PB;
		NL3D::CMaterial			Mat;

		SCacheRender();
	};

	// There are one CacheRender per cacheTexture + texture unused and NULL (no texture)
	SCacheRender _CacheRender[CACHE_RENDER_SIZE];

};

// ***************************************************************************

#endif // BUILDERZONE_H