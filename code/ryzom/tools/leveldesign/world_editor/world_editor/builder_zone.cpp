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

#include "stdafx.h"
#include "nel/misc/object_vector.h"
#include "nel/misc/path.h"

#include "world_editor.h"
#include "world_editor_doc.h"
#include "builder_zone.h"
#include "main_frm.h"
#include "display.h"
#include "tools_zone.h"

extern bool	DontUse3D;

using namespace std;
using namespace NLLIGO;

// Because we cant use the namespace NLMISC or NL3D in global due to some collision (CBitmap,...)
using NLMISC::CIFile;
using NLMISC::Exception;
using NLMISC::CVector;
using NLMISC::CUV;
using NLMISC::CMatrix;
using NLMISC::CSmartPtr;
using NLMISC::CIFile;
using NLMISC::COFile;

using NL3D::ITexture;
using NL3D::CTextureMem;
using NL3D::CTextureFile;
using NL3D::CVertexBuffer;
using NL3D::CIndexBuffer;
using NL3D::CMaterial;
using NL3D::CViewport;
using NL3D::CNELU;
using NL3D::CVertexBufferReadWrite;
using NL3D::CIndexBufferReadWrite;

// ***************************************************************************
// CDataBase::SCacheTexture
// ***************************************************************************

// ---------------------------------------------------------------------------
CDataBase::SCacheTexture::SCacheTexture()
{
	Enabled = false;
	Texture = NULL;
}

// ---------------------------------------------------------------------------
bool CDataBase::SCacheTexture::isFull()
{
	if (!Enabled)
		return false;
	for (uint32 i = 0; i < FreePlace.size(); ++i)
		if (FreePlace[i])
			return false;
	return true;
}

// ***************************************************************************
// CDataBase
// ***************************************************************************

// ---------------------------------------------------------------------------
CDataBase::CDataBase (uint bitmapSize)
{
	_RefCacheTextureSizeX = _RefCacheTextureSizeY = 1024; // Size of the texture cache
	_RefSizeX = _RefSizeY = bitmapSize; // Size of a zone in pixel
	_RefCacheTextureNbEltX = _RefCacheTextureSizeX / _RefSizeX;
	_RefCacheTextureNbEltY = _RefCacheTextureSizeY / _RefSizeY;
	for (uint32 i = 0; i < 64; ++i)
		_CacheTexture[i].Enabled = false;
}

// ---------------------------------------------------------------------------
CDataBase::~CDataBase ()
{
	reset ();
}

// ---------------------------------------------------------------------------
void CDataBase::reset ()
{
	for (uint32 i = 0; i < 64; ++i)
	{
		_CacheTexture[i].Enabled = false;
		_CacheTexture[i].Texture = NULL; // Because its a smart ptr
		_CacheTexture[i].PtrMem.resize (0);
		_CacheTexture[i].FreePlace.resize(0);
	}
	
	map<string,SElement>::iterator it = _ZoneDBmap.begin ();
	while (it != _ZoneDBmap.end())
	{
		string sName = it->first;
		SElement &rElt = it->second;
		delete rElt.WinBitmap;
		rElt.WinBitmap = NULL;
		++it;
	}
	_ZoneDBmap.clear ();
	_UnusedTexture = NULL;
}

// ---------------------------------------------------------------------------
bool CDataBase::initFromPath (const string &Path)
{
	return false;
}

// ---------------------------------------------------------------------------
bool CDataBase::init (const string &Path, CZoneBank &zb)
{
	string sDirBackup = NLMISC::CPath::getCurrentPath();

	// "Path" can be relative to the doc path so we have to be first in the doc path
	string s2 = NLMISC::CFile::getPath ((LPCTSTR)getMainFrame()->getDocument()->GetPathName());
	NLMISC::CPath::setCurrentPath(s2.c_str());
	string ss = NLMISC::CPath::getFullPath(Path);
	NLMISC::CPath::setCurrentPath (ss.c_str());

	uint32 i, m, n, o, p;
	uint8 k, l;

	vector<string> ZoneNames;
	zb.getCategoryValues ("zone", ZoneNames);
	for (i = 0; i < ZoneNames.size(); ++i)
	{
		// Progress
		getMainFrame ()->progressLoadingDialog ((float)i / (float)ZoneNames.size());

		SElement zdbTmp;
		CZoneBankElement *pZBE = zb.getElementByZoneName (ZoneNames[i]);
		// Read the texture file
		string zdbTmpName = ZoneNames[i];
		zdbTmp.SizeX = pZBE->getSizeX ();
		zdbTmp.SizeY = pZBE->getSizeY ();
		const vector<bool> &rMask = pZBE->getMask();

		NLMISC::CBitmap *pBitmap = loadBitmap (getTextureFile(zdbTmpName));

		// Should not return NULL !
		nlassert (pBitmap);

		// Wanted zone size
		uint width = _RefSizeX * zdbTmp.SizeX;
		uint height = _RefSizeY * zdbTmp.SizeY;

		// Good size ?
		if ((pBitmap->getWidth () != width) || (pBitmap->getHeight () != height))
		{
			// Resize it
			pBitmap->resample (width, height);
		}

		zdbTmp.WinBitmap = convertToWin (pBitmap);
		pBitmap->flipV ();

		for (l = 0; l < zdbTmp.SizeY; ++l)
		for (k = 0; k < zdbTmp.SizeX; ++k)
		if (rMask[k+l*zdbTmp.SizeX])
		{
			SCacheZone czTmp;

			czTmp.PosX = k;
			czTmp.PosY = l;

			// Found first non full texture cache
			for (m = 0; m < 64; ++m)
			if (_CacheTexture[m].Enabled == false)
			{
				// Create the texture
				_CacheTexture[m].FreePlace.resize (_RefCacheTextureNbEltX*_RefCacheTextureNbEltY, true);
				_CacheTexture[m].Texture = new CTextureMem();
				_CacheTexture[m].Texture->setAllowDegradation (true);
				_CacheTexture[m].PtrMem.resize (4*_RefCacheTextureSizeX*_RefCacheTextureSizeY);
				_CacheTexture[m].Texture->resize (_RefCacheTextureSizeX, _RefCacheTextureSizeY);
				_CacheTexture[m].Texture->setPointer (&_CacheTexture[m].PtrMem[0], 4*_RefCacheTextureSizeX*_RefCacheTextureSizeY,
											false, false, _RefCacheTextureSizeX, _RefCacheTextureSizeY);

				_CacheTexture[m].Enabled = true;
				break;
			}
			else
			{
				if (!_CacheTexture[m].isFull())
					break;
			}

			nlassert (m<64);

			// Found first place in this texture

			for (n = 0; n < _CacheTexture[m].FreePlace.size(); ++n)
			if (_CacheTexture[m].FreePlace[n])
			{
				sint32 xSrc = k*_RefSizeX;
				sint32 ySrc = l*_RefSizeY;
				sint32 xDst = (n%_RefCacheTextureNbEltX)*_RefSizeX;
				sint32 yDst = (n/_RefCacheTextureNbEltX)*_RefSizeY;
				uint8 *pSrc = &pBitmap->getPixels()[(xSrc+ySrc*pBitmap->getWidth())*4];
				uint8 *pDst = &_CacheTexture[m].PtrMem[(xDst+yDst*_RefCacheTextureSizeX)*4];
				// Copy part of the bitmap into cache texture
				for (p = 0; p < _RefSizeY; ++p)
				for (o = 0; o < _RefSizeX; ++o)
				{
					pDst[(o+p*_RefCacheTextureSizeX)*4+0] = pSrc[(o+p*pBitmap->getWidth())*4+0];
					pDst[(o+p*_RefCacheTextureSizeX)*4+1] = pSrc[(o+p*pBitmap->getWidth())*4+1];
					pDst[(o+p*_RefCacheTextureSizeX)*4+2] = pSrc[(o+p*pBitmap->getWidth())*4+2];
					pDst[(o+p*_RefCacheTextureSizeX)*4+3] = pSrc[(o+p*pBitmap->getWidth())*4+3];
				}
				czTmp.PosUV.U = ((float)xDst) / ((float)_RefCacheTextureSizeX);
				czTmp.PosUV.V = ((float)yDst) / ((float)_RefCacheTextureSizeY);
				czTmp.CacheTexture = _CacheTexture[m].Texture;
				_CacheTexture[m].FreePlace[n] = false;
				break;
			}
			//nlassert (m<_CacheTexture[m].FreePlace.size());
			zdbTmp.ZonePieces.push_back (czTmp);
		}
		// Add the entry in the DataBase
		_ZoneDBmap.insert (pair<string,SElement>(zdbTmpName, zdbTmp));
		delete pBitmap;
	}

	// Upload all textures in VRAM
	for (m = 0; m < 64; ++m)
	if (_CacheTexture[m].Enabled)
		_CacheTexture[m].Texture->touch ();

	_UnusedTexture = loadTexture (getTextureFile("_UNUSED_"));

	NLMISC::CPath::setCurrentPath(sDirBackup);
	return true;
}

// ---------------------------------------------------------------------------
CBitmap *CDataBase::getBitmap (const string &ZoneName)
{
	map<string,SElement>::iterator it = _ZoneDBmap.find (ZoneName);
	if (it != _ZoneDBmap.end())
		return (it->second.WinBitmap);
	else
		return NULL;
}

// ---------------------------------------------------------------------------
ITexture* CDataBase::getTexture (const string &ZoneName, sint32 nPosX, sint32 nPosY, CUV &retUVmin, CUV &retUVmax)
{
	if (ZoneName == STRING_UNUSED)
	{
		retUVmin.U = 0.0f;
		retUVmin.V = 1.0f - 0.0f;
		retUVmax.U = 1.0f;
		retUVmax.V = 1.0f - 1.0f;
		return _UnusedTexture;
	}

	map<string,SElement>::iterator it = _ZoneDBmap.find (ZoneName);
	if (it != _ZoneDBmap.end())
	{
		SElement &rElt = it->second;
		for (uint32 j = 0; j < rElt.ZonePieces.size(); ++j)
		if ((rElt.ZonePieces[j].PosX == nPosX) && (rElt.ZonePieces[j].PosY == nPosY))
		{
			retUVmin = rElt.ZonePieces[j].PosUV;
			retUVmin.U += 0.5f / ((float)_RefCacheTextureSizeX);
			retUVmin.V += 0.5f / ((float)_RefCacheTextureSizeY);
			retUVmax = retUVmin;
			retUVmax.U += ((float)_RefSizeX-1) / ((float)_RefCacheTextureSizeX);
			retUVmax.V += ((float)_RefSizeY-1) / ((float)_RefCacheTextureSizeY);
			return rElt.ZonePieces[j].CacheTexture;
		}
	}

	return NULL;
}

// ---------------------------------------------------------------------------
// Private
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Divisor to initialize windows bitmap
#define WIDTH_DIVISOR	2
#define HEIGHT_DIVISOR	2

CBitmap *CDataBase::convertToWin (NLMISC::CBitmap *pBitmap)
{
	CBitmap *pWinBitmap = new CBitmap;
	NLMISC::CObjectVector<uint8> &rPixel = pBitmap->getPixels();
	uint32 nNewWidth = pBitmap->getWidth()/WIDTH_DIVISOR;
	uint32 nNewHeight = pBitmap->getHeight()/HEIGHT_DIVISOR;
	uint8 *pNewPixel = new uint8[nNewHeight*nNewWidth*4];
	uint32 i, j;
	for (j = 0; j < nNewHeight; ++j)
	for (i = 0; i < nNewWidth; ++i)
	{
		pNewPixel[(i+j*nNewWidth)*4+0] = rPixel[(i*WIDTH_DIVISOR+j*HEIGHT_DIVISOR*pBitmap->getWidth())*4+2];
		pNewPixel[(i+j*nNewWidth)*4+1] = rPixel[(i*WIDTH_DIVISOR+j*HEIGHT_DIVISOR*pBitmap->getWidth())*4+1];
		pNewPixel[(i+j*nNewWidth)*4+2] = rPixel[(i*WIDTH_DIVISOR+j*HEIGHT_DIVISOR*pBitmap->getWidth())*4+0];
		pNewPixel[(i+j*nNewWidth)*4+3] = rPixel[(i*WIDTH_DIVISOR+j*HEIGHT_DIVISOR*pBitmap->getWidth())*4+3];
	}
	pWinBitmap->CreateBitmap (nNewWidth, nNewHeight, 1, 32, pNewPixel);	
	if (pNewPixel == NULL)
	{
		delete pWinBitmap;
		return NULL;
	}
	else
		return pWinBitmap;
}

// ---------------------------------------------------------------------------
CTextureFile *CDataBase::loadTexture (const std::string &fileName)
{
	CTextureFile *pTexture = new CTextureFile;
	pTexture->setFileName (fileName);
	pTexture->setReleasable (false);
	pTexture->generate ();
	return pTexture;
}

// ---------------------------------------------------------------------------
NLMISC::CBitmap *CDataBase::loadBitmap (const std::string &fileName)
{
	NLMISC::CBitmap *pBitmap = new NLMISC::CBitmap();

	try
	{
		CIFile fileIn;
		if (fileIn.open (fileName))
		{
			pBitmap->load (fileIn);
		}
		else
		{
			pBitmap->makeDummy();
			nlwarning ("Bitmap not found : %s", fileName.c_str());
		}
	}
	catch (Exception& e)
	{
		pBitmap->makeDummy();
		theApp.errorMessage ("Error while loading bitmap %s : %s", fileName.c_str(), e.what());
	}

	return pBitmap;
}

// ***************************************************************************
// CBuilderZone
// ***************************************************************************
// ---------------------------------------------------------------------------
// PRIVATE
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
void CBuilderZone::calcMask()
{
	sint32 i;
	sint32 x, y;

	_MinY = _MinX = 1000000;
	_MaxY = _MaxX = -1000000;

	if (_ZoneRegions.size() == 0)
		return;

	CWorldEditorDoc *doc = getDocument ();

	for (i = 0; i < (sint32)_ZoneRegions.size(); ++i)
	{
		const CZoneRegion *pBZR = &(doc->getZoneRegion (i));

		if (_MinX > pBZR->getMinX())
			_MinX = pBZR->getMinX();
		if (_MinY > pBZR->getMinY())
			_MinY = pBZR->getMinY();
		if (_MaxX < pBZR->getMaxX())
			_MaxX = pBZR->getMaxX();
		if (_MaxY < pBZR->getMaxY())
			_MaxY = pBZR->getMaxY();
	}
	
	_ZoneMask.resize ((1+_MaxX-_MinX)*(1+_MaxY-_MinY));
	sint32 stride = (1+_MaxX-_MinX);
	for (y = _MinY; y <= _MaxY; ++y)
	for (x = _MinX; x <= _MaxX; ++x)
	{
		_ZoneMask[x-_MinX+(y-_MinY)*stride] = true;

		for (i = 0; i < (sint32)_ZoneRegions.size(); ++i)
		if (i != _ZoneRegionSelected)
		{
			const CZoneRegion *pBZR = &(doc->getZoneRegion (i));

			const string &rSZone = pBZR->getName (x, y);
			if ((rSZone != STRING_OUT_OF_BOUND) && (rSZone != STRING_UNUSED))
			{
				_ZoneMask[x-_MinX+(y-_MinY)*stride] = false;
			}
		}
	}
}

// ---------------------------------------------------------------------------
// PUBLIC
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
CBuilderZone::CBuilderZone (uint bitmapSize) : _DataBase (bitmapSize)
{
	// Set Current Filter
	_FilterType1 = STRING_UNUSED; _FilterValue1 = "";
	_FilterType2 = STRING_UNUSED; _FilterValue2 = "";
	_FilterType3 = STRING_UNUSED; _FilterValue3 = "";
	_FilterType4 = STRING_UNUSED; _FilterValue4 = "";
	_FilterOperator2 = 0;
	_FilterOperator3 = 0;
	_FilterOperator4 = 0;
	_RandomSelection = false;
	_CurSelectedZone = -1;
	_ZoneRegionSelected = -1;
	_ApplyRot = 0;
	_ApplyRotType = 0;
	_ApplyFlip = 0;
	_ApplyFlipType = 0;

	_Display = NULL;
	_ApplyRotCycle = 0;
	_ApplyFlipCycle = 0;
	_CycleSelection = false;
	_ApplyCycleSelection = 0;
	_NotPropagate = false;
	_Force = false;
	_LastPathName = "";
	_ToolsZone = NULL;
}

// ---------------------------------------------------------------------------
bool CBuilderZone::init (const string &sPathName, bool makeAZone, CDisplay *display)
{
	// Set the display
	_Display = display;
	
	bool bRet = true;
	if (sPathName != _LastPathName)
	{
		_LastPathName = sPathName;
		string sZoneBankPath = sPathName;
		sZoneBankPath += "\\zoneligos\\";
		// Init the ZoneBank
		_ZoneBank.reset ();
		if (!initZoneBank (sZoneBankPath))
		{
			_ZoneBank.reset ();
			return false;
		}

		// Construct the DataBase from the ZoneBank
		string sZoneBitmapPath = sPathName;
		sZoneBitmapPath += "\\zonebitmaps\\";
		_DataBase.reset ();
		if (!_DataBase.init (sZoneBitmapPath.c_str(), _ZoneBank))
		{
			_DataBase.reset ();
			_ZoneBank.reset ();
			return false;
		}
	}

	if ((makeAZone) && (bRet))
		newZone();
	
	return bRet;
}

// ---------------------------------------------------------------------------
void CBuilderZone::setToolsZone (CToolsZone *pTool)
{
	_ToolsZone = pTool;
}

// ---------------------------------------------------------------------------
void CBuilderZone::updateToolsZone ()
{
	uint32 i;
	
	if (_ToolsZone == NULL)
		return;

	// Execute the filter
	_ZoneBank.resetSelection ();
	if(_FilterType1 != STRING_UNUSED)
		_ZoneBank.addOrSwitch (_FilterType1, _FilterValue1);

	if(_FilterType2 != STRING_UNUSED)
	{
		if (_FilterOperator2 == 0) // AND switch wanted
			_ZoneBank.addAndSwitch (_FilterType2, _FilterValue2);
		else // OR switch wanted
			_ZoneBank.addOrSwitch (_FilterType2, _FilterValue2);
	}

	if(_FilterType3 != STRING_UNUSED)
	{
		if (_FilterOperator3 == 0) // AND switch wanted
			_ZoneBank.addAndSwitch (_FilterType3, _FilterValue3);
		else // OR switch wanted
			_ZoneBank.addOrSwitch (_FilterType3, _FilterValue3);
	}

	if(_FilterType4 != STRING_UNUSED)
	{
		if (_FilterOperator4 == 0) // AND switch wanted
			_ZoneBank.addAndSwitch (_FilterType4, _FilterValue4);
		else // OR switch wanted
			_ZoneBank.addOrSwitch (_FilterType4, _FilterValue4);
	}

	_ZoneBank.getSelection (_CurrentSelection);

	// Create the corresponding image list from selected item using DataBase

	vector<CBitmap*> vIL;
	vIL.resize (_CurrentSelection.size());
	for (i = 0; i < _CurrentSelection.size(); ++i)
	{
		CZoneBankElement *pElt = _CurrentSelection[i];
		// Get bitmap from DataBase
		vIL[i] = _DataBase.getBitmap (pElt->getName());
	}

	// Construct the tree add first items then the images!
	_ToolsZone->getListCtrl()->reset ();
	for (i = 0; i < _CurrentSelection.size(); ++i)
	{
		CZoneBankElement *pElt = _CurrentSelection[i];
		_ToolsZone->getListCtrl()->addItem (pElt->getName());
	}
	_ToolsZone->getListCtrl()->setImages (vIL);
	if (_CurrentSelection.size() > 0)
	{
		_ToolsZone->getListCtrl()->SetCurSel (0);
		_CurSelectedZone = 0;
	}
	else
		_CurSelectedZone = -1;
	_ApplyCycleSelection = 0;


	_ToolsZone->Invalidate();
	_ToolsZone->UpdateWindow();
}

// ---------------------------------------------------------------------------
bool CBuilderZone::refresh ()
{
	// Reset all
	_ZoneRegions.clear ();
	_ZoneRegionSelected = -1;

	CWorldEditorDoc *doc  = getDocument ();
	for (uint j=0; j<doc->getNumDatabaseElement (); j++)
	{
		// Is it a landscape ?
		if (doc->isLandscape (j))
		{
			_ZoneRegionSelected++;

			newZone (false);

			// Check if we can load this zone
			const CZoneRegion *pBZR = &(getDocument ()->getZoneRegion (_ZoneRegionSelected));
			for (sint32 y = pBZR->getMinY(); y <= pBZR->getMaxY(); ++y)
			for (sint32 x = pBZR->getMinX(); x <= pBZR->getMaxX(); ++x)
			{
				const string &refSZone = pBZR->getName (x, y);
				if (refSZone != STRING_UNUSED)
				{
					sint zoneRegionSelected2 = -1;
					CWorldEditorDoc *doc  = getDocument ();
					for (uint i=0; i<doc->getNumDatabaseElement (); i++)
					{
						// Is it a landscape ?
						if (doc->isLandscape (i))
						{
							zoneRegionSelected2++;
							if (i!=j)
							{
								const CZoneRegion *pBZR2 = &(getDocument ()->getZoneRegion (zoneRegionSelected2));
								const string &sZone = pBZR2->getName (x, y);
								if ((sZone != STRING_UNUSED)&&(sZone != STRING_OUT_OF_BOUND))
								{
									unload (_ZoneRegionSelected);
									MessageBox (NULL, "Cannot add this zone because it overlaps existing ones", 
												"Error", MB_ICONERROR|MB_OK);
									return false;
								}
							}
						}
					}
				}
			}

			string error;
			if (!_ZoneRegions[_ZoneRegionSelected]->init (&_ZoneBank, this, error))
			{
				unload (_ZoneRegionSelected);
				MessageBox (NULL, ("Cannot add this zone :\n"+error).c_str(), 
							"Error", MB_ICONERROR|MB_OK);
				return false;
			}

			_Display->_Offset.x = _Display->_CellSize*(pBZR->getMinX() + 
														pBZR->getMaxX()) / 2.0f;
			_Display->_Offset.y = _Display->_CellSize*(pBZR->getMinY() + 
														pBZR->getMaxY()) / 2.0f;
			calcMask ();
		}
	}
	return true;
}

// ---------------------------------------------------------------------------

void CBuilderZone::newZone (bool bDisplay)
{
	_ZoneRegions.push_back (new CBuilderZoneRegion (_ZoneRegions.size()));
	_ZoneRegionSelected = _ZoneRegions.size() - 1;
	// Select starting point for the moment 0,0
	sint32 i;
	sint32 x = 0, y = 0;
	// If there are some zone already present increase x until free
	for (i = 0; i < (sint32)_ZoneRegions.size(); ++i)
	{
		if (getDocument ()->isLandscape (i))
		{
			const CZoneRegion *pBZR = &(getDocument ()->getZoneRegion (i));
			const string &rsZone = pBZR->getName (x, y);
			if ((rsZone != STRING_OUT_OF_BOUND) && (rsZone != STRING_UNUSED))
			{
				++x; i = -1;
			}
		}
	}
	// _ZoneRegions[_ZoneRegionSelected]->setStart (x,y);
	calcMask ();
}

// ---------------------------------------------------------------------------
void CBuilderZone::unload (uint32 pos)
{
	uint32 i = 0;
	if (_ZoneRegions.size() == 0)
		return;

	delete _ZoneRegions[pos];
	for (i = pos; i < (_ZoneRegions.size()-1); ++i)
	{
		_ZoneRegions[i] = _ZoneRegions[i+1];
	}
	_ZoneRegions.resize (_ZoneRegions.size()-1);
	if (_ZoneRegionSelected == (sint32)_ZoneRegions.size())
		_ZoneRegionSelected = _ZoneRegions.size()-1;
	calcMask ();
}

// ---------------------------------------------------------------------------
void CBuilderZone::move (sint32 x, sint32 y)
{
	if (_ZoneRegions.size() == 0)
		return;
	_ZoneRegions[_ZoneRegionSelected]->move(x, y);
}

// ---------------------------------------------------------------------------
uint32 CBuilderZone::countZones ()
{
	if (_ZoneRegions.size() == 0)
		return 0;
	return _ZoneRegions[_ZoneRegionSelected]->countZones ();
}

// ---------------------------------------------------------------------------
void CBuilderZone::snapshot (const char *fileName, uint sizeSource, bool grayscale)
{
	const CZoneRegion *pBZR = &(getDocument ()->getZoneRegion (_ZoneRegionSelected));
	sint32 nMinX = pBZR->getMinX();
	sint32 nMaxX = pBZR->getMaxX();
	sint32 nMinY = pBZR->getMinY();
	sint32 nMaxY = pBZR->getMaxY();

	uint width = (nMaxX - nMinX + 1)*sizeSource;
	uint height = (nMaxY - nMinY + 1)*sizeSource;
	snapshotCustom (fileName, width, height, false, sizeSource, grayscale);
}

// ---------------------------------------------------------------------------
void CBuilderZone::snapshotCustom (const char *fileName, uint width, uint height, bool keepRatio, uint sizeSource, bool grayscale)
{
	if (_ZoneRegions.size() == 0)
		return;

	// Some bitmaps
	NLMISC::CBitmap bitmapTmp;
	NLMISC::CBitmap bitmapDest;

	const CZoneRegion *pBZR = &(getDocument ()->getZoneRegion (_ZoneRegionSelected));
	sint32 nMinX = pBZR->getMinX();
	sint32 nMaxX = pBZR->getMaxX();
	sint32 nMinY = pBZR->getMinY();
	sint32 nMaxY = pBZR->getMaxY();

	uint nSizeX = (nMaxX - nMinX + 1)*sizeSource;
	uint nSizeY = (nMaxY - nMinY + 1)*sizeSource;
	sint x, y, j;

	// Keep ratio ?
	if (keepRatio)
		height = (width * nSizeY) / nSizeX;

	// Resize the bitmaps
	bitmapDest.resize (nSizeX, nSizeY, NLMISC::CBitmap::RGBA);

	// white all
	NLMISC::CObjectVector<uint8> &rPixels = bitmapDest.getPixels();
	memset (&rPixels[0], 0xff, rPixels.size ());
	
	// Copy ZoneBitmaps in the bitmap
	CUV uvMin, uvMax;
	ITexture *pTexture;
	CZoneBankElement *pZBE;
	uint8 nRot, nFlip;

	// For each tiles
	for (y = nMinY; y <= nMaxY; ++y)
	for (x = nMinX; x <= nMaxX; ++x)
	{
		const string &rsZoneName = pBZR->getName (x, y);
		if ((rsZoneName == STRING_OUT_OF_BOUND) && (rsZoneName == STRING_UNUSED))
			continue;

		pZBE = _ZoneBank.getElementByZoneName (rsZoneName);
		if (pZBE == NULL)
			continue;

		// Get the texture
		pTexture = _DataBase.getTexture (rsZoneName, pBZR->getPosX(x, y), 
										pBZR->getPosY(x, y), uvMin, uvMax);

		// Generate it
		pTexture->generate ();

		// Be sure it is tga
		pTexture->convertToType (NLMISC::CBitmap::RGBA);

		// Get rot
		nRot = pBZR->getRot(x, y);

		// Get flip
		nFlip = pBZR->getFlip(x, y);

		// Copy the texture

		// Dest bitmap size
		uint destWidth = 1+(uint)((float)pTexture->getWidth() * (uvMax.U - uvMin.U));
		uint destHeight = 1+(uint)((float)pTexture->getHeight() * (uvMax.V - uvMin.V));
		bitmapTmp.resize (destWidth, destHeight, NLMISC::CBitmap::RGBA);

		// Source bitmap size and position
		uint u = (uint)((float)pTexture->getWidth() * uvMin.U);
		uint v = (uint)((float)pTexture->getHeight() * uvMin.V);
		uint sourceWidth = pTexture->getWidth();
		uint sourceHeight = pTexture->getHeight();

		// Source pointer
		uint8 *srcPixels = &(pTexture->getPixels ()[0]);

		// Destination pointer
		uint8 *destPixels = &(bitmapTmp.getPixels ()[0]);

		// Copy the temp bitmap
		for (j = 0; j < (sint)destHeight; ++j)
			// Copy the line
			memcpy (destPixels+(4*j*destWidth), srcPixels + 4 * ( (v + j) * sourceWidth + u), destWidth*4);

		// Flip ?
		if (nFlip)
			bitmapTmp.flipH();

		// Rot ?
		while (nRot)
		{
			bitmapTmp.rot90CW ();
			nRot--;
		}

		// Resize the bitmap to normal size
		if ( (bitmapTmp.getWidth () != sizeSource) || (bitmapTmp.getHeight () != sizeSource) )
			bitmapTmp.resample (sizeSource, sizeSource);

		// Copy it in the map
		bitmapDest.blit (&bitmapTmp, (x-nMinX)*sizeSource, (y-nMinY)*sizeSource);

		pTexture->release ();
	}

	// Resample the final bitmap
	bitmapDest.resample (width, height);
	bitmapDest.flipV ();

	COFile f(fileName, false, false, true);

	if (grayscale)
	{
		bitmapDest.convertToType (NLMISC::CBitmap::Luminance);
		if (bitmapDest.writeTGA (f, 8))
			f.close();
	}
	else
	{
		if (bitmapDest.writeTGA (f, 32))
			f.close();
	}
}

// ---------------------------------------------------------------------------
CBuilderZone::SCacheRender::SCacheRender ()
{
	Used = false;
	Mat.initUnlit ();
	Mat.setBlend (false);
	VB.setVertexFormat (CVertexBuffer::PositionFlag|CVertexBuffer::TexCoord0Flag|CVertexBuffer::PrimaryColorFlag);
}

// ---------------------------------------------------------------------------
void CBuilderZone::render (const NLMISC::CVector &viewMin, const NLMISC::CVector &viewMax)
{
	sint32 i, zoneSelected;

	DWORD backgroundColor = GetSysColor (COLOR_WINDOW);
 
	// Reset the cache
	for (i = 0; i < (CACHE_RENDER_SIZE); ++i)
	{
		_CacheRender[i].VB.setNumVertices (0);
		_CacheRender[i].PB.setNumIndexes (0);
		_CacheRender[i].Used = false;
	}

	// Select all blocks visible
	float minx = floorf(viewMin.x/_Display->_CellSize)*_Display->_CellSize;
	float miny = floorf(viewMin.y/_Display->_CellSize)*_Display->_CellSize;
	float maxx = ceilf(viewMax.x/_Display->_CellSize)*_Display->_CellSize;
	float maxy = ceilf(viewMax.y/_Display->_CellSize)*_Display->_CellSize;
	CVector pos1, pos2, pos3, pos4;
	CUV uvMin, uvMax;
	sint32 x, y;
	ITexture *pTexture;
	
	while (minx < maxx)
	{
		miny = floorf(viewMin.y/_Display->_CellSize)*_Display->_CellSize;
		while (miny < maxy)
		{
			x = (sint32)floor(minx / _Display->_CellSize);
			y = (sint32)floor(miny / _Display->_CellSize);

			i = 0;
			string sZone = STRING_OUT_OF_BOUND;
			zoneSelected = 0;
			for (i = 0; i < (sint32)_ZoneRegions.size(); ++i)
			{
				const string &rSZone = getDocument ()->getZoneRegion (i).getName (x, y);
				if ((sZone == STRING_OUT_OF_BOUND) && (rSZone == STRING_UNUSED))
				{
					sZone = STRING_UNUSED;
					zoneSelected = i;
				}
				if ((rSZone != STRING_OUT_OF_BOUND) && (rSZone != STRING_UNUSED))
				{
					sZone = rSZone;
					zoneSelected = i;
				}
			}
			CZoneBankElement *pZBE = _ZoneBank.getElementByZoneName (sZone);

			if (pZBE == NULL)
				pTexture = _DataBase.getTexture (sZone, 0, 0, uvMin, uvMax);
			else
				pTexture = _DataBase.getTexture (sZone, getDocument ()->getZoneRegion (zoneSelected).getPosX(x, y), 
												getDocument ()->getZoneRegion (zoneSelected).getPosY(x, y), uvMin, uvMax);

			// Look if already existing texture exists in the cache
			for (i = 0; i < (CACHE_RENDER_SIZE); ++i)
			if (_CacheRender[i].Used)
				if (_CacheRender[i].Mat.getTexture(0) == pTexture)
					break;

			if (i == (CACHE_RENDER_SIZE))
			{
				// Use a new CacheRender slot
				for (i = 0; i < (CACHE_RENDER_SIZE); ++i)
					if (!_CacheRender[i].Used)
						break;
				nlassert(i<(CACHE_RENDER_SIZE));
				_CacheRender[i].Used = true;
				_CacheRender[i].Mat.setTexture (0, pTexture);
			}

			pos1.x = (minx-viewMin.x)/(viewMax.x-viewMin.x);
			pos1.y = 0.0f;
			pos1.z = (miny-viewMin.y)/(viewMax.y-viewMin.y);

			pos2.x = (_Display->_CellSize+minx-viewMin.x)/(viewMax.x-viewMin.x);
			pos2.y = 0.0f;
			pos2.z = (miny-viewMin.y)/(viewMax.y-viewMin.y);

			pos3.x = (_Display->_CellSize+minx-viewMin.x)/(viewMax.x-viewMin.x);
			pos3.y = 0.0f;
			pos3.z = (_Display->_CellSize+miny-viewMin.y)/(viewMax.y-viewMin.y);

			pos4.x = (minx-viewMin.x)/(viewMax.x-viewMin.x);
			pos4.y = 0.0f;
			pos4.z = (_Display->_CellSize+miny-viewMin.y)/(viewMax.y-viewMin.y);

			uint32 nBasePt = _CacheRender[i].VB.getNumVertices();
			_CacheRender[i].VB.setNumVertices (nBasePt+4);
			CVertexBufferReadWrite vba;
			_CacheRender[i].VB.lock (vba);
			vba.setVertexCoord (nBasePt+0, pos1);
			vba.setVertexCoord (nBasePt+1, pos2);
			vba.setVertexCoord (nBasePt+2, pos3);
			vba.setVertexCoord (nBasePt+3, pos4);


			uint32 nBaseTri = _CacheRender[i].PB.getNumIndexes ();
			_CacheRender[i].PB.setNumIndexes (nBaseTri+6);
			CIndexBufferReadWrite iba;
			_CacheRender[i].PB.lock (iba);
			iba.setTri (nBaseTri+0, nBasePt+0, nBasePt+1, nBasePt+2);
			iba.setTri (nBaseTri+3, nBasePt+0, nBasePt+2, nBasePt+3);

			if ((zoneSelected>=0)&&(zoneSelected<(sint32)_ZoneRegions.size()))
			{
				const CZoneRegion *pBZR = &(getDocument ()->getZoneRegion (zoneSelected));
				if (pBZR->getFlip (x, y) == 1)
				{
					float rTmp = uvMin.U;
					uvMin.U = uvMax.U;
					uvMax.U = rTmp;
				}

				vba.setTexCoord (nBasePt+(pBZR->getRot (x, y)+0)%4, 0, CUV(uvMin.U, uvMin.V));
				vba.setTexCoord (nBasePt+(pBZR->getRot (x, y)+1)%4, 0, CUV(uvMax.U, uvMin.V));
				vba.setTexCoord (nBasePt+(pBZR->getRot (x, y)+2)%4, 0, CUV(uvMax.U, uvMax.V));
				vba.setTexCoord (nBasePt+(pBZR->getRot (x, y)+3)%4, 0, CUV(uvMin.U, uvMax.V));
			}
			else
			{
				vba.setTexCoord (nBasePt+0, 0, CUV(uvMin.U, uvMin.V));
				vba.setTexCoord (nBasePt+1, 0, CUV(uvMax.U, uvMin.V));
				vba.setTexCoord (nBasePt+2, 0, CUV(uvMax.U, uvMax.V));
				vba.setTexCoord (nBasePt+3, 0, CUV(uvMin.U, uvMax.V));
			}

			NLMISC::CRGBA color;

			if ((zoneSelected>=0)&&(zoneSelected<(sint32)_ZoneRegions.size()))
			{
				if (getZoneMask(x,y))
					color = NLMISC::CRGBA(255, 255, 255, 255);
				else
					color = NLMISC::CRGBA(127, 127, 127, 255);
			}
			else
				color = NLMISC::CRGBA(255, 255, 255, 255);

			if (pTexture == NULL)
				color = _Display->_BackgroundColor;

			vba.setColor (nBasePt+0, color);
			vba.setColor (nBasePt+1, color);
			vba.setColor (nBasePt+2, color);
			vba.setColor (nBasePt+3, color);

			miny += _Display->_CellSize;
		}
		minx += _Display->_CellSize;
	}

	//MessageBox(NULL, "CBuilderZone::render", "3", MB_OK);

	if (DontUse3D)
		return;

	// Flush the cache to the screen
	CMatrix mtx;
	mtx.identity();
	CNELU::Driver->setupViewport (CViewport());
	CNELU::Driver->setupViewMatrix (mtx);
	CNELU::Driver->setupModelMatrix (mtx);
	CNELU::Driver->setFrustum (0.f, 1.f, 0.f, 1.f, -1.f, 1.f, false);

	//MessageBox(NULL, "CBuilderZone::render", "4", MB_OK);

	for (i = 0; i < (CACHE_RENDER_SIZE); ++i)
	if (_CacheRender[i].Used)
	{
		// Render with driver
		CNELU::Driver->activeVertexBuffer (_CacheRender[i].VB);
		CNELU::Driver->activeIndexBuffer (_CacheRender[i].PB);
		CNELU::Driver->renderTriangles (_CacheRender[i].Mat, 0, _CacheRender[i].PB.getNumIndexes()/3);
	}
}

// ---------------------------------------------------------------------------
void CBuilderZone::renderTransition (const NLMISC::CVector &viewMin, const NLMISC::CVector &viewMax)
{
	// Selected ?
	if (_ZoneRegionSelected != -1)
	{
		// Select all blocks visible
		float rMinX = floorf (viewMin.x / _Display->_CellSize)*_Display->_CellSize;
		float rMinY = floorf (viewMin.y / _Display->_CellSize)*_Display->_CellSize;
		float rMaxX = ceilf  (viewMax.x / _Display->_CellSize)*_Display->_CellSize;
		float rMaxY = ceilf  (viewMax.y / _Display->_CellSize)*_Display->_CellSize;

		sint32 nMinX = (sint32)floor (rMinX / _Display->_CellSize);
		sint32 nMinY = (sint32)floor (rMinY / _Display->_CellSize);
		sint32 nMaxX = (sint32)floor (rMaxX / _Display->_CellSize);
		sint32 nMaxY = (sint32)floor (rMaxY / _Display->_CellSize);

		CVertexBuffer VB , VBSel;
		CIndexBuffer PB, PBSel;
		CMaterial Mat, MatSel;

		Mat.initUnlit ();
		Mat.setBlend (false);
		MatSel.initUnlit ();
		MatSel.setBlend (false);
		MatSel.setColor(NLMISC::CRGBA(255,0,0,0));
		VB.setVertexFormat (CVertexBuffer::PositionFlag);
		VB.reserve ((nMaxX-nMinX+1)*(nMaxY-nMinY+1)*4*2);
		VBSel.setVertexFormat (CVertexBuffer::PositionFlag);
		VBSel.reserve ((nMaxX-nMinX+1)*(nMaxY-nMinY+1)*4*2);

		CVertexBufferReadWrite vba;
		VB.lock (vba);
		CVertexBufferReadWrite vbaSel;
		VBSel.lock (vbaSel);
		
		sint32 nVertCount = 0;
		sint32 nVertCountSel = 0;
		sint32 x, y, k;
		CVector worldPos, screenPos;
		for (y = nMinY; y <= nMaxY; ++y)
		for (x = nMinX; x <= nMaxX; ++x)
		{
			const CZoneRegion *pBZR = &(getDocument ()->getZoneRegion (_ZoneRegionSelected));
			uint8 ceUp = pBZR->getCutEdge (x, y, 0);
			uint8 ceLeft = pBZR->getCutEdge (x, y, 2);

			if ((ceUp > 0) && (ceUp < 3))
			for (k = 0; k < 2; ++k)
			{
				if (ceUp == 1)
					worldPos.x = x * _Display->_CellSize + 3.0f * _Display->_CellSize / 12.0f;
				else
					worldPos.x = x * _Display->_CellSize + 7.0f * _Display->_CellSize / 12.0f;
				worldPos.y = (y+1)*_Display->_CellSize + _Display->_CellSize / 12.0f;
				worldPos.z = 0;
				// World -> Screen conversion
				screenPos.x = (worldPos.x - viewMin.x) / (viewMax.x - viewMin.x);
				screenPos.y = 0.0f;
				screenPos.z = (worldPos.y - viewMin.y) / (viewMax.y - viewMin.y);
				if (k == 0)
				{
					vba.setVertexCoord (nVertCount, screenPos);
					++nVertCount;
				}
				else
				{
					vbaSel.setVertexCoord (nVertCountSel, screenPos);
					++nVertCountSel;
				}

				worldPos.y = (y+1)*_Display->_CellSize - _Display->_CellSize / 12.0f;
				screenPos.z = (worldPos.y - viewMin.y) / (viewMax.y - viewMin.y);
				if (k == 0)
				{
					vba.setVertexCoord (nVertCount, screenPos);
					++nVertCount;
				}
				else
				{
					vbaSel.setVertexCoord (nVertCountSel, screenPos);
					++nVertCountSel;
				}

				if (ceUp == 1)
					worldPos.x = x * _Display->_CellSize + 5.0f * _Display->_CellSize / 12.0f;
				else
					worldPos.x = x * _Display->_CellSize + 9.0f * _Display->_CellSize / 12.0f;
				screenPos.x = (worldPos.x - viewMin.x) / (viewMax.x - viewMin.x);
				if (k == 0)
				{
					vba.setVertexCoord (nVertCount, screenPos);
					++nVertCount;
				}
				else
				{
					vbaSel.setVertexCoord (nVertCountSel, screenPos);
					++nVertCountSel;
				}

				worldPos.y = (y+1)*_Display->_CellSize + _Display->_CellSize / 12.0f;
				screenPos.z = (worldPos.y - viewMin.y) / (viewMax.y - viewMin.y);
				if (k == 0)
				{
					vba.setVertexCoord (nVertCount, screenPos);
					++nVertCount;
				}
				else
				{
					vbaSel.setVertexCoord (nVertCountSel, screenPos);
					++nVertCountSel;
				}
				ceUp = 3 - ceUp;
			}

			if ((ceLeft > 0) && (ceLeft < 3))
			for (k = 0; k < 2; ++k)
			{
				worldPos.x = x * _Display->_CellSize - _Display->_CellSize / 12.0f;
				if (ceLeft == 1)
					worldPos.y = y * _Display->_CellSize + 3.0f * _Display->_CellSize / 12.0f;
				else
					worldPos.y = y * _Display->_CellSize + 7.0f * _Display->_CellSize / 12.0f;
				worldPos.z = 0;
				// World -> Screen conversion
				screenPos.x = (worldPos.x - viewMin.x) / (viewMax.x - viewMin.x);
				screenPos.y = 0.0f;
				screenPos.z = (worldPos.y - viewMin.y) / (viewMax.y - viewMin.y);
				if (k == 0)
				{
					vba.setVertexCoord (nVertCount, screenPos);
					++nVertCount;
				}
				else
				{
					vbaSel.setVertexCoord (nVertCountSel, screenPos);
					++nVertCountSel;
				}

				worldPos.x = x * _Display->_CellSize + _Display->_CellSize / 12.0f;
				screenPos.x = (worldPos.x - viewMin.x) / (viewMax.x - viewMin.x);
				if (k == 0)
				{
					vba.setVertexCoord (nVertCount, screenPos);
					++nVertCount;
				}
				else
				{
					vbaSel.setVertexCoord (nVertCountSel, screenPos);
					++nVertCountSel;
				}

				if (ceLeft == 1)
					worldPos.y = y * _Display->_CellSize + 5.0f * _Display->_CellSize / 12.0f;
				else
					worldPos.y = y * _Display->_CellSize + 9.0f * _Display->_CellSize / 12.0f;
				screenPos.z = (worldPos.y - viewMin.y) / (viewMax.y - viewMin.y);
				if (k == 0)
				{
					vba.setVertexCoord (nVertCount, screenPos);
					++nVertCount;
				}
				else
				{
					vbaSel.setVertexCoord (nVertCountSel, screenPos);
					++nVertCountSel;
				}

				worldPos.x = x * _Display->_CellSize - _Display->_CellSize / 12.0f;
				screenPos.x = (worldPos.x - viewMin.x) / (viewMax.x - viewMin.x);
				if (k == 0)
				{
					vba.setVertexCoord (nVertCount, screenPos);
					++nVertCount;
				}
				else
				{
					vbaSel.setVertexCoord (nVertCountSel, screenPos);
					++nVertCountSel;
				}
				ceLeft = 3 - ceLeft;
			}
		}

		vba.unlock();
		VB.setNumVertices (nVertCount);
		PB.setNumIndexes (nVertCount*2);
		CIndexBufferReadWrite iba;
		PB.lock (iba);
		for (x = 0; x < (nVertCount/4); ++x)
		{
			iba.setLine (x*4+0, x*4+0, x*4+1);
			iba.setLine (x*4+1, x*4+1, x*4+2);
			iba.setLine (x*4+2, x*4+2, x*4+3);
			iba.setLine (x*4+3, x*4+3, x*4+0);
		}
		iba.unlock();

		vbaSel.unlock();
		VBSel.setNumVertices (nVertCountSel);
		PBSel.setNumIndexes (nVertCountSel*2);
		CIndexBufferReadWrite ibaSel;
		PBSel.lock (ibaSel);
		for (x = 0; x < (nVertCountSel/4); ++x)
		{
			ibaSel.setLine (x*4+0, x*4+0, x*4+1);
			ibaSel.setLine (x*4+1, x*4+1, x*4+2);
			ibaSel.setLine (x*4+2, x*4+2, x*4+3);
			ibaSel.setLine (x*4+3, x*4+3, x*4+0);
		}
		ibaSel.unlock();

		if (DontUse3D)
			return;

		// Render
		CMatrix mtx;
		mtx.identity();
		CNELU::Driver->setupViewport (CViewport());
		CNELU::Driver->setupViewMatrix (mtx);
		CNELU::Driver->setupModelMatrix (mtx);
		CNELU::Driver->setFrustum (0.f, 1.f, 0.f, 1.f, -1.f, 1.f, false);
		CNELU::Driver->activeVertexBuffer (VB);
		CNELU::Driver->activeIndexBuffer (PB);
		CNELU::Driver->renderLines (Mat, 0, PB.getNumIndexes()/2);
		CNELU::Driver->activeVertexBuffer (VBSel);
		CNELU::Driver->activeIndexBuffer (PBSel);
		CNELU::Driver->renderLines (MatSel, 0, PBSel.getNumIndexes()/2);
	}
}

// ---------------------------------------------------------------------------
void CBuilderZone::displayGrid (const NLMISC::CVector &viewMin, const NLMISC::CVector &viewMax)
{
	// Select all blocks visible
	float rMinX = floorf (viewMin.x / _Display->_CellSize)*_Display->_CellSize;
	float rMinY = floorf (viewMin.y / _Display->_CellSize)*_Display->_CellSize;
	float rMaxX = ceilf  (viewMax.x / _Display->_CellSize)*_Display->_CellSize;
	float rMaxY = ceilf  (viewMax.y / _Display->_CellSize)*_Display->_CellSize;

	sint32 nMinX = (sint32)floor (rMinX / _Display->_CellSize);
	sint32 nMinY = (sint32)floor (rMinY / _Display->_CellSize);
	sint32 nMaxX = (sint32)floor (rMaxX / _Display->_CellSize);
	sint32 nMaxY = (sint32)floor (rMaxY / _Display->_CellSize);

	static vector<uint8> vBars;
	sint32 nBarsW = (nMaxX-nMinX)+1;
	sint32 nBarsH = (nMaxY-nMinY)+1;
	vBars.resize (nBarsW*nBarsH);
	sint32 x, y, i, j, zoneSelected;
	for (i = 0; i < nBarsW*nBarsH; ++i)
		vBars[i] = 0;


	for (y = nMinY; y <= nMaxY; ++y)
	for (x = nMinX; x <= nMaxX; ++x)
	{

		string sZone = STRING_OUT_OF_BOUND;
		zoneSelected = 0;
		for (i = 0; i < (sint32)_ZoneRegions.size(); ++i)
		{
			const string &rSZone = getDocument ()->getZoneRegion (i).getName (x, y);
			if ((sZone == STRING_OUT_OF_BOUND) && (rSZone == STRING_UNUSED))
			{
				sZone = STRING_UNUSED;
				zoneSelected = i;
			}
			if ((rSZone != STRING_OUT_OF_BOUND) && (rSZone != STRING_UNUSED))
			{
				sZone = rSZone;
				zoneSelected = i;
			}
		}


		//const string &sZone = _ZoneRegion.getName (x, y);
		CZoneBankElement *pZBE = _ZoneBank.getElementByZoneName (sZone);
		if (pZBE != NULL)
		if ((pZBE->getSizeX() > 1) || (pZBE->getSizeY() > 1))
		{
			const CZoneRegion *pSelected = &(getDocument ()->getZoneRegion (zoneSelected));
			sint32 sizeX = pZBE->getSizeX(), sizeY = pZBE->getSizeY();
			sint32 posX = pSelected->getPosX (x, y), posY = pSelected->getPosY (x, y);
			uint8 rot = pSelected->getRot (x, y);
			uint8 flip = pSelected->getFlip (x, y);
			sint32 deltaX, deltaY;
		
			if (flip == 0)
			{
				switch (rot)
				{
					case 0: deltaX = -posX; deltaY = -posY; break;
					case 1: deltaX = -(sizeY-1-posY); deltaY = -posX; break;
					case 2: deltaX = -(sizeX-1-posX); deltaY = -(sizeY-1-posY); break;
					case 3: deltaX = -posY; deltaY = -(sizeX-1-posX); break;
				}
			}
			else
			{
				switch (rot)
				{
					case 0: deltaX = -(sizeX-1-posX); deltaY = -posY; break;
					case 1: deltaX = -(sizeY-1-posY); deltaY = -(sizeX-1-posX); break;
					case 2: deltaX = -posX; deltaY = -(sizeY-1-posY); break;
					case 3: deltaX = -posY; deltaY = -posX; break;
				}
			}

			static SPiece sMask;
			sMask.Tab.resize (sizeX*sizeY);
			for(i = 0; i < sizeX*sizeY; ++i)
				sMask.Tab[i] = pZBE->getMask()[i];
			sMask.w = sizeX;
			sMask.h = sizeY;
			sMask.rotFlip (rot, flip);

			for (j = 0; j < sMask.h; ++j)
			for (i = 0; i < sMask.w; ++i)
			if (sMask.Tab[i+j*sMask.w])
			{
				if (((x+deltaX+i-nMinX)>=0) && ((x+deltaX+i-nMinX)<nBarsW) &&
					((y+deltaY+j-nMinY)>=0) && ((y+deltaY+j-nMinY)<nBarsH))
				{
					if ((i > 0) && (sMask.Tab[i-1+j*sMask.w]))
						vBars[x+deltaX+i-nMinX + (y+deltaY+j-nMinY)*nBarsW] |= 1;

					if ((j > 0) && (sMask.Tab[i+(j-1)*sMask.w]))
						vBars[x+deltaX+i-nMinX + (y+deltaY+j-nMinY)*nBarsW] |= 2;
				}
			}
		}
	}

	CVertexBuffer VB;
	CIndexBuffer PB;
	CMaterial Mat;

	Mat.initUnlit ();
	Mat.setBlend (false);
	VB.setVertexFormat (CVertexBuffer::PositionFlag);
	VB.setNumVertices ((nBarsW+1)*(nBarsH+1));
	CVertexBufferReadWrite vba;
	VB.lock (vba);
	
	for (y = nMinY; y <= nMaxY+1; ++y)
	for (x = nMinX; x <= nMaxX+1; ++x)
	{
		CVector pos;

		pos.x = (x*_Display->_CellSize - viewMin.x)/(viewMax.x-viewMin.x);
		pos.y = 0.0f;
		pos.z = (y*_Display->_CellSize - viewMin.y)/(viewMax.y-viewMin.y);
		vba.setVertexCoord (x-nMinX+(y-nMinY)*(nBarsW+1), pos);
	}

	PB.setNumIndexes (nBarsW*nBarsH*2*2);
	CIndexBufferReadWrite iba;
	PB.lock (iba);
	uint32 nNbLine = 0;
	for (y = 0; y < nBarsH; ++y)
	for (x = 0; x < nBarsW; ++x)
	{
		// Vertical Line ?
		if ((vBars[x+y*nBarsW] & 1) == 0)
		{
			iba.setLine (nNbLine*2, x+y*(nBarsW+1), x+(y+1)*(nBarsW+1));
			++nNbLine;
		}

		// Horizontal Line ?
		if ((vBars[x+y*nBarsW] & 2) == 0)
		{
			iba.setLine (nNbLine*2, x+y*(nBarsW+1), (x+1)+y*(nBarsW+1));
			++nNbLine;
		}
	}
	iba.unlock();
	PB.setNumIndexes (nNbLine*2);

	if (DontUse3D)
		return;

	// Render with driver
	CMatrix mtx;
	mtx.identity();
	vba.unlock();
	CNELU::Driver->setupViewport (CViewport());
	CNELU::Driver->setupViewMatrix (mtx);
	CNELU::Driver->setupModelMatrix (mtx);
	CNELU::Driver->setFrustum (0.f, 1.f, 0.f, 1.f, -1.f, 1.f, false);
	CNELU::Driver->activeVertexBuffer (VB);
	CNELU::Driver->activeIndexBuffer (PB);
	CNELU::Driver->renderLines (Mat, 0, PB.getNumIndexes()/2);
	
}

// ---------------------------------------------------------------------------
void CBuilderZone::add (const CVector &worldPos)
{
	sint32 x = (sint32)floor (worldPos.x / _Display->_CellSize);
	sint32 y = (sint32)floor (worldPos.y / _Display->_CellSize);
	uint8 rot, flip;

	if (_ZoneRegions.size() == 0)
		return;

	if (_RandomSelection)
	{
		if (_CurrentSelection.size() > 0)
		{
			uint32 nSel = (uint32)(NLMISC::frand (1.0) * _CurrentSelection.size());
			NLMISC::clamp (nSel, (uint32)0, (uint32)(_CurrentSelection.size()-1));
			_CurSelectedZone = nSel;
		}
	}
	if (_CycleSelection)
	{
		_CurSelectedZone = _ApplyCycleSelection;
		_ApplyCycleSelection++;
		_ApplyCycleSelection = _ApplyCycleSelection%_CurrentSelection.size();
	}

	if (_ApplyRotType == 1)
	{
		uint32 nSel = (uint32)(NLMISC::frand (1.0) * 4);
		NLMISC::clamp (nSel, (uint32)0, (uint32)3);
		rot = (uint8)nSel;
	}
	else if (_ApplyRotType == 0)
	{
		rot = _ApplyRot;
	}
	else if (_ApplyRotType == 2)
	{
		rot = _ApplyRotCycle;
		_ApplyRotCycle++;
		_ApplyRotCycle = _ApplyRotCycle%4;
	}


	if (_ApplyFlipType == 1)
	{
		uint32 nSel = (uint32)(NLMISC::frand (1.0) * 2);
		NLMISC::clamp (nSel, (uint32)0, (uint32)1);
		flip = (uint8)nSel;
	}
	else if (_ApplyFlipType == 0)
	{
		flip = _ApplyFlip;
	}
	else if (_ApplyFlipType == 2)
	{
		flip = _ApplyFlipCycle;
		_ApplyFlipCycle++;
		_ApplyFlipCycle = _ApplyFlipCycle%2;
	}

	if ((_CurSelectedZone >= 0)&&(_CurSelectedZone <= ((sint32)_CurrentSelection.size()-1)))
	{
		CZoneBankElement *pZBE = _CurrentSelection[_CurSelectedZone];
		string error;
		_ZoneRegions[_ZoneRegionSelected]->init (&_ZoneBank, this, error);

		if (_Force)
		{
			_ZoneRegions[_ZoneRegionSelected]->addForce (x, y, rot, flip, pZBE);
		}
		else
		{
			if (_NotPropagate)
				_ZoneRegions[_ZoneRegionSelected]->addNotPropagate (x, y, rot, flip, pZBE);
			else
				_ZoneRegions[_ZoneRegionSelected]->add (x, y, rot, flip, pZBE);
		}
	}
}

// ---------------------------------------------------------------------------
void CBuilderZone::addTransition (const NLMISC::CVector &worldPos)
{
	sint32 x = (sint32)floor (worldPos.x / _Display->_CellSize);
	sint32 y = (sint32)floor (worldPos.y / _Display->_CellSize);
	sint32 k;

	if (_ZoneRegions.size() == 0)
		return;

	// Detect if we are in a transition square to switch
	CBuilderZoneRegion *pBZR2 = _ZoneRegions[_ZoneRegionSelected];
	const CZoneRegion *pBZR = &(getDocument ()->getZoneRegion (_ZoneRegionSelected));
	bool bCutEdgeTouched = false;
	for (uint8 transPos = 0; transPos < 4; ++transPos)
	{
		uint ce = pBZR->getCutEdge (x, y, transPos);

		if ((ce > 0) && (ce < 3))
		for (k = 0; k < 2; ++k)
		{
			float xTrans, yTrans;

			if ((transPos == 0) || (transPos == 1))
			{
				if (ce == 1)
					xTrans = _Display->_CellSize / 3.0f;
				else
					xTrans = 2.0f * _Display->_CellSize / 3.0f;
			}
			else
			{
				if (transPos == 2)
					xTrans = 0;
				else
					xTrans = _Display->_CellSize;
			}
			xTrans += x * _Display->_CellSize;

			if ((transPos == 2) || (transPos == 3))
			{
				if (ce == 1)
					yTrans = _Display->_CellSize / 3.0f;
				else
					yTrans = 2.0f * _Display->_CellSize / 3.0f;
			}
			else
			{
				if (transPos == 1)
					yTrans = 0;
				else
					yTrans = _Display->_CellSize;
			}
			yTrans += y * _Display->_CellSize;

			if ((worldPos.x >= (xTrans-_Display->_CellSize/12.0f)) && (worldPos.x <= (xTrans+_Display->_CellSize/12.0f)) &&
				(worldPos.y >= (yTrans-_Display->_CellSize/12.0f)) && (worldPos.y <= (yTrans+_Display->_CellSize/12.0f)))
			{
				pBZR2->invertCutEdge (x, y, transPos);
				bCutEdgeTouched = true;
			}
			ce = 3 - ce;
		}
	}

	// If not clicked to change the cutEdge so the user want to change the transition
	if (!bCutEdgeTouched)
	{
		pBZR2->cycleTransition (x, y);
	}
}

// ---------------------------------------------------------------------------
void CBuilderZone::del (const CVector &worldPos)
{
	sint32 x = (sint32)floor (worldPos.x / _Display->_CellSize);
	sint32 y = (sint32)floor (worldPos.y / _Display->_CellSize);

	if (_ZoneRegions.size() == 0)
		return;

	CBuilderZoneRegion *pBZR = _ZoneRegions[_ZoneRegionSelected];
	string error;
	pBZR->init (&_ZoneBank, this, error);
	pBZR->del (x, y);
}

// ---------------------------------------------------------------------------
bool CBuilderZone::initZoneBank (const string &sPathName)
{
	char sDirBackup[512];
	GetCurrentDirectory (512, sDirBackup);
	SetCurrentDirectory (sPathName.c_str());
	WIN32_FIND_DATA findData;
	HANDLE hFind;
	hFind = FindFirstFile ("*.ligozone", &findData);
	
	while (hFind != INVALID_HANDLE_VALUE)
	{
		// If the name of the file is not . or .. then its a valid entry in the DataBase
		if (!((strcmp (findData.cFileName, ".") == 0) || (strcmp (findData.cFileName, "..") == 0)))
		{
			string error;
			if (!_ZoneBank.addElement (findData.cFileName, error))
				theApp.errorMessage (error.c_str());
		}
		if (FindNextFile (hFind, &findData) == 0)
			break;
	}
	SetCurrentDirectory (sDirBackup);
	return true;
}

// ---------------------------------------------------------------------------

uint32 CBuilderZone::getNbZoneRegion ()
{
	return _ZoneRegions.size ();
}

// ---------------------------------------------------------------------------

string CBuilderZone::getZoneName (sint32 x, sint32 y)
{
	string sRet = STRING_UNUSED;
	uint32 nNbRegion = _ZoneRegions.size();
	for (uint32 i = 0; i < nNbRegion; ++i)
	{
		const CZoneRegion *pBZR = &(getDocument ()->getZoneRegion (i));
		if ((x < pBZR->getMinX())||(x > pBZR->getMaxX())||(y < pBZR->getMinY())||(y > pBZR->getMaxY()))
			continue;
		if (pBZR->getName(x, y) != STRING_UNUSED)
		{
			sRet = pBZR->getName(x, y);
			return sRet;
		}
	}
	sRet = "";
	return sRet;
}

// ---------------------------------------------------------------------------
uint8 CBuilderZone::getRot (sint32 x, sint32 y)
{
	uint8 sRet = 0;
	uint32 nNbRegion = _ZoneRegions.size();
	for (uint32 i = 0; i < nNbRegion; ++i)
	{
		const CZoneRegion *pBZR = &(getDocument ()->getZoneRegion (i));
		if ((x < pBZR->getMinX())||(x > pBZR->getMaxX())||(y < pBZR->getMinY())||(y > pBZR->getMaxY()))
			continue;
		if (pBZR->getName(x, y) != STRING_UNUSED)
		{
			sRet = pBZR->getRot(x, y);
			return sRet;
		}
	}
	sRet = 0;
	return sRet;
}

// ---------------------------------------------------------------------------
uint8 CBuilderZone::getFlip (sint32 x, sint32 y)
{
	uint8 sRet = 0;
	uint32 nNbRegion = _ZoneRegions.size();
	for (uint32 i = 0; i < nNbRegion; ++i)
	{
		const CZoneRegion *pBZR = &(getDocument ()->getZoneRegion (i));
		if ((x < pBZR->getMinX())||(x > pBZR->getMaxX())||(y < pBZR->getMinY())||(y > pBZR->getMaxY()))
			continue;
		if (pBZR->getName(x, y) != STRING_UNUSED)
		{
			sRet = pBZR->getFlip(x, y);
			return sRet;
		}
	}
	sRet = 0;
	return sRet;
}

// ---------------------------------------------------------------------------
CBuilderZoneRegion*	CBuilderZone::getPtrCurZoneRegion ()
{
	if (_ZoneRegions.size() == 0)
		return NULL;
	return _ZoneRegions[_ZoneRegionSelected];
}

// ---------------------------------------------------------------------------
uint32 CBuilderZone::getCurZoneRegion ()
{
	return _ZoneRegionSelected;
}

// ---------------------------------------------------------------------------
void CBuilderZone::setCurZoneRegion (uint32 sel)
{
	_ZoneRegionSelected = sel;
	calcMask();
}

// ---------------------------------------------------------------------------
bool CBuilderZone::getZoneMask (sint32 x, sint32 y)
{
	if ((x < _MinX) || (x > _MaxX) ||
		(y < _MinY) || (y > _MaxY))
	{
		return true;
	}
	else
	{
		return _ZoneMask[(x-_MinX)+(y-_MinY)*(1+_MaxX-_MinX)];
	}
}

// ---------------------------------------------------------------------------
/*
void CBuilderZone::generate (sint32 nMinX, sint32 nMinY, sint32 nMaxX, sint32 nMaxY, 
							sint32 nZoneBaseX, sint32 nZoneBaseY, const char *MaterialString)
{
	if ((nMinX > nMaxX)	|| (nMinY > nMaxY))
		return;

	for (sint32 j = nMinY; j <= nMaxY; ++j)
	for (sint32 i = nMinX; i <= nMaxX; ++i)
	{
		// Generate zone name
		string ZoneName = "converted-"+CExport::getZoneNameFromXY (i, j);
		ZoneName = NLMISC::strlwr (ZoneName);
		CZoneBankElement *pZBE = _ZoneBank.getElementByZoneName (ZoneName);
		if (pZBE != NULL)
		{
			_ZoneRegions[_ZoneRegionSelected]->init (&_ZoneBank, this);
			_ZoneRegions[_ZoneRegionSelected]->add (i, j, 0, 0, pZBE);
		}
	}
}
*/