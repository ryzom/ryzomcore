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

#include "std3d.h"

#include "nel/3d/shape_bank.h"
#include "nel/3d/mesh_base.h"
#include "nel/misc/file.h"
#include "nel/misc/path.h"
#include "nel/misc/rect.h"
#include "nel/misc/algo.h"
#include "nel/misc/progress_callback.h"

using namespace std;
using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{

// ***************************************************************************

CShapeBank::CShapeBank()
{
	// Default cache creation
	addShapeCache( "default" );
	_MaxUploadPerFrame = 16*1024;
}

// ***************************************************************************

CShapeBank::~CShapeBank()
{
}

// ***************************************************************************

IShape*CShapeBank::addRef(const string &shapeNameNotLwr)
{
	string	shapeName= toLower(shapeNameNotLwr);

	// get the shape info (must succeed)
	TShapeInfoMap::iterator scfpmIt = ShapePtrToShapeInfo.find( getShapePtrFromShapeName( shapeName ) );
	nlassert( scfpmIt != ShapePtrToShapeInfo.end() );

	// If The shape is not inserted into a cache, just return it
	if( !scfpmIt->second.isAdded )
		return getShapePtrFromShapeName( shapeName );

	// else If the shape is inserted in a shape cache remove it
	scfpmIt->second.isAdded = false;
	CShapeCache *pShpCache = scfpmIt->second.pShpCache;
	nlassert( pShpCache != NULL );
	// Search the shape cache for the shape we want to remove
	list<IShape*>::iterator lsIt = pShpCache->Elements.begin();
	while(lsIt != pShpCache->Elements.end())
	{
		const string *sTemp = getShapeNameFromShapePtr(*lsIt);
		if( *sTemp == shapeName )
		{
			// Ok the shape cache contains the shape remove it and return
			pShpCache->Elements.erase( lsIt );
			return getShapePtrFromShapeName( shapeName );
		}
		++lsIt;
	}
	nlassert( false );
	return getShapePtrFromShapeName( shapeName );
}

// ***************************************************************************

void CShapeBank::release(IShape* pShp)
{
	// Do we have the last smartPtr on the shape ?
	const string* str = getShapeNameFromShapePtr( pShp );

	if (str == NULL)
	{
		nlwarning ("Trying to release a mesh that have not be added to the shape bank");
	}
	else
	{
		TShapeMap::iterator smIt = ShapeMap.find( *str );
		if( smIt != ShapeMap.end() )
		{
			if( smIt->second.getNbRef() == 1 )
			{
				// Yes -> add the shape to its shapeCache
				CShapeCache *pShpCache = getShapeCachePtrFromShapePtr( pShp );
				pShpCache->Elements.push_front( pShp );

				TShapeInfoMap::iterator scfpmIt = ShapePtrToShapeInfo.find( pShp );
				if( scfpmIt != ShapePtrToShapeInfo.end() )
				{
					scfpmIt->second.isAdded = true;
				}

				// check the shape cache
				checkShapeCache(getShapeCachePtrFromShapePtr(pShp));
			}
		}
		else
		{
			nlassert( false );
		}
	}
}

// ***************************************************************************

void CShapeBank::processWaitingShapes ()
{
	uint32 nTotalUploaded = 0;
	TWaitingShapesMap::iterator wsmmIt = WaitingShapes.begin();
	while( wsmmIt != WaitingShapes.end() )
	{
		// Backup next iterator
		TWaitingShapesMap::iterator wsmmItNext = wsmmIt;
		wsmmItNext++;

		const string &shapeName = wsmmIt->first;
		CWaitingShape &rWS = wsmmIt->second;
		IShape *pShp = rWS.ShapePtr; // Take care this value is shared between thread so copy it in a local variable first

		switch (rWS.State)
		{
			case AsyncLoad_Shape: // Check if we can pass to the AsyncLoad_Texture state
				if (pShp != NULL)
				{
					if (pShp == (IShape*)-1)
						rWS.State = AsyncLoad_Error;
					else
						rWS.State = AsyncLoad_Texture;
				}
			break;

			case AsyncLoad_Texture:
			{
				// Setup all textures and lightmaps of the shape
				if (nTotalUploaded > _MaxUploadPerFrame)
					break;

				CMeshBase *pMesh = dynamic_cast<CMeshBase*>(pShp);
				if( pMesh != NULL )
				{
					uint8 j;
					uint32 i, CurrentProgress = 0;
					uint32 nNbMat = pMesh->getNbMaterial();

					for (i = 0; i < nNbMat; ++i)
					{
						const CMaterial &rMat = pMesh->getMaterial(i);
						// Parse all textures from this material and setup
						for (j = 0; j < IDRV_MAT_MAXTEXTURES; ++j)
						{
							if (CurrentProgress >= rWS.UpTextProgress)
							{
								if (rMat.texturePresent(j))
								{
									if ((!_pDriver->isTextureExist(*rMat.getTexture(j))) ||
										(rWS.UpTextLine > 0) || (rWS.UpTextMipMap > 0))
									{
										//_pDriver->setupTexture (*rMat.getTexture(j));

										if (!processWSUploadTexture (rWS, nTotalUploaded, rMat.getTexture(j)))
											break;
									}
									else
									{
										/*
											Must release texture data because of the following scenario (common!)
											- An IG with 2 different meshs is async loaded
											- the 2 meshs access the same texture "pipo.tga"
											- in CAsyncFileManager3D::loadMesh(), if this case arise, the
												textures are loaded twice in RAM!!!! (is managed per mesh, but not
												thourhg meshs)
											- hence the first mesh setup and upload above the texture in VRAM
												but the second still have data in RAM!! (because 2nd will
												say that isTextureExist() ....

											Note: you could say that the cool stuff would be to not load the 2 :)
												but it appears that this case arise rarely because of the following statement:
												- while the second mesh is async loaded, the first upload in VRAM
													so there is good change that the second don't load the texture
													(isTextureExist() return true in the thread)
										*/
										ITexture	*tex= rMat.getTexture(j);
										if(tex->getReleasable())
											tex->release();
									}
								}
								++rWS.UpTextProgress;
							}
							++CurrentProgress;
							if (nTotalUploaded > _MaxUploadPerFrame)
								break;
						}

						if (nTotalUploaded > _MaxUploadPerFrame)
							break;

						// Do the same with lightmaps
						if (rMat.getShader() == CMaterial::LightMap)
						{
							uint j = 0; ITexture *pText = rMat.getLightMap (j);
							while (pText != NULL)
							{
								if (CurrentProgress >= rWS.UpTextProgress)
								{
									if ((!_pDriver->isTextureExist(*pText)) ||
										(rWS.UpTextLine > 0) || (rWS.UpTextMipMap > 0))
									{
										//_pDriver->setupTexture (*pText);

										if (!processWSUploadTexture (rWS, nTotalUploaded, pText))
											break;
									}
									else
									{
										// see above for explanation
										if(pText->getReleasable())
											pText->release();
									}

									++rWS.UpTextProgress;
								}
								++CurrentProgress;
								++j; pText = rMat.getLightMap (j);
								if (nTotalUploaded > _MaxUploadPerFrame)
									break;
							}
						}
						if (nTotalUploaded > _MaxUploadPerFrame)
							break;
					}
				}
				if (nTotalUploaded > _MaxUploadPerFrame)
					break;

				rWS.State = AsyncLoad_Ready;
			}
			break;

			case AsyncLoad_Ready:
				add (wsmmIt->first, pShp);
				rWS.State = AsyncLoad_Delete;
			break;

			// The delete operation can take several frames to complete but this is not a problem

			// For error do the same as delete but let the flag to error if a shape is asked just after
			// the error was found

			case AsyncLoad_Error:
			case AsyncLoad_Delete:
				rWS.RefCnt -= 1;
				if (rWS.RefCnt == 0)
				{
					// We have to signal if we are the last
					std::set<bool *>::iterator ite = rWS.Signal.begin();
					while (ite != rWS.Signal.end())
					{
						bool *bSignal = *ite;
						if (bSignal != NULL)
						{
							bool bFound = false;
							TWaitingShapesMap::iterator wsmmIt2 = WaitingShapes.begin();
							while (wsmmIt2 != WaitingShapes.end())
							{
								const string &shapeName2 = wsmmIt2->first;
								if (shapeName2 != shapeName)
								{
									std::set<bool *>::iterator ite2 = wsmmIt2->second.Signal.begin();
									while (ite2 != wsmmIt2->second.Signal.end())
									{
										if (*ite2 == bSignal)
										{
											bFound = true;
											break;
										}
										ite2++;
									}
									if (ite2 != wsmmIt2->second.Signal.end())
										break;
								}
								++wsmmIt2;
							}
							if (!bFound)
								*bSignal = true;
						}

						// Next iterator
						ite++;
					}
					WaitingShapes.erase (wsmmIt);
				}
			break;

			default:
				nlstop; // This must never happen
			break;
		}

		wsmmIt = wsmmItNext;
	}
}

// ***************************************************************************
void CShapeBank::setMaxBytesToUpload (uint32 MaxUploadPerFrame)
{
	_MaxUploadPerFrame = MaxUploadPerFrame;
}

// ***************************************************************************
bool CShapeBank::processWSUploadTexture (CWaitingShape &rWS, uint32 &nTotalUploaded, ITexture *pText)
{
	CRect zeRect;
	uint32 nFace, nWeight = 0, nMipMap;

	if ((rWS.UpTextMipMap == 0) && (rWS.UpTextLine == 0))
	{
		// Create the texture only and do not upload anything
		bool isRel = pText->getReleasable ();
		pText->setReleasable (false);
		bool isAllUploaded = false;
		_pDriver->setupTextureEx (*pText, false, isAllUploaded);
		pText->setReleasable (isRel);
		if (isAllUploaded)
			return true;
	}

	if (pText->mipMapOn())
		nMipMap = pText->getMipMapCount();
	else
		nMipMap = 1;

	// Upload all mipmaps
	for (; rWS.UpTextMipMap < nMipMap;)
	{
		uint32 nMM = rWS.UpTextMipMap;
		// What is left to upload ?
		nWeight = pText->getSize (nMM) - rWS.UpTextLine*pText->getWidth(nMM);
		// Yoyo: important: in case of DXTC1, must mul before div
		nWeight= (nWeight*CBitmap::bitPerPixels[pText->getPixelFormat()]) / 8;
		if (pText->isTextureCube())
			nWeight *= 6;

		// NB: nWeight can be 0 in case of 1x1 in DXTC1. Estimate 1 byte
		if(nWeight==0)
			nWeight= 1;

		// Setup rectangle
		if ((nTotalUploaded + nWeight) > _MaxUploadPerFrame)
		{
			// We cannot upload the whole mipmap -> we have to cut it
			uint32 nSizeToUpload = _MaxUploadPerFrame - nTotalUploaded;
			uint32 nLineWeight = (pText->getWidth(nMM)*CBitmap::bitPerPixels[pText->getPixelFormat()]) / 8;
			// NB: nLineWeight can be 0 in case of 1x1 in DXTC1. Estimate 1 byte (avoid divide by zero)
			if(nLineWeight==0)
				nLineWeight= 1;
			if (pText->isTextureCube())
				nLineWeight *= 6;
			// compute the number of lines we'll upload
			uint32 nNbLineToUpload = nSizeToUpload / nLineWeight;
			nNbLineToUpload = nNbLineToUpload / 4;
			if (nNbLineToUpload == 0)
				nNbLineToUpload = 1;
			nNbLineToUpload *= 4; // Upload 4 line by 4 line
			// setup the rect
			uint32 nNewLine = rWS.UpTextLine + nNbLineToUpload;
			if (nNewLine > pText->getHeight(nMM))
				nNewLine = pText->getHeight(nMM);
			zeRect.set (0, rWS.UpTextLine, pText->getWidth(nMM), nNewLine);
			rWS.UpTextLine = nNewLine;
			if (rWS.UpTextLine == pText->getHeight(nMM))
			{
				rWS.UpTextLine = 0;
				rWS.UpTextMipMap += 1;
			}
		}
		else
		{
			// We can upload the whole mipmap (or the whole rest of the mipmap)
			zeRect.set (0, rWS.UpTextLine, pText->getWidth(nMM), pText->getHeight(nMM));
			rWS.UpTextLine = 0;
			rWS.UpTextMipMap += 1;
		}

		// Upload !
		if (pText->isTextureCube())
		{
			for (nFace = 0; nFace < 6; ++nFace)
				_pDriver->uploadTextureCube (*pText, zeRect, (uint8)nMM, (uint8)nFace);
		}
		else
		{
			_pDriver->uploadTexture (*pText, zeRect, (uint8)nMM);
		}

		nTotalUploaded += nWeight;
		if (nTotalUploaded > _MaxUploadPerFrame)
			return false;
	}

	if (pText->getReleasable())
		pText->release();

	rWS.UpTextMipMap = 0;
	rWS.UpTextLine = 0;
	return true;
}

// ***************************************************************************

CShapeBank::TShapeState CShapeBank::getPresentState (const string &shapeNameNotLwr)
{
	string	shapeName= toLower(shapeNameNotLwr);

	// Is the shape is found in the shape map so return Present
	TShapeMap::iterator smIt = ShapeMap.find (shapeName);
	if( smIt != ShapeMap.end() )
		return Present;
	// Look in the waiting shapes
	TWaitingShapesMap::iterator wsmmIt = WaitingShapes.find (shapeName);
	if (wsmmIt != WaitingShapes.end())
		return wsmmIt->second.State; // AsyncLoad_*
	return NotPresent;
}

// ***************************************************************************
IShape	*CShapeBank::getShape (const std::string &shapeNameNotLwr)
{
	string	shapeName= toLower(shapeNameNotLwr);

	// Is the shape is found in the shape map so return Present
	TShapeMap::iterator smIt = ShapeMap.find (shapeName);
	if( smIt != ShapeMap.end() )
		return smIt->second;

	return NULL;
}

// ***************************************************************************

void CShapeBank::load (const string &shapeNameNotLwr)
{
	string	shapeName= toLower(shapeNameNotLwr);

	TShapeMap::iterator smIt = ShapeMap.find(shapeName);
	if( smIt == ShapeMap.end() )
	{
		// If we are loading it asynchronously so we do not have to try to load it in sync mode
		TWaitingShapesMap::iterator wsmmIt = WaitingShapes.find (shapeName);
		if (wsmmIt != WaitingShapes.end())
			return;

		CShapeStream mesh;
		CIFile meshfile;
		if (meshfile.open(CPath::lookup(shapeName, false)))
		{
			meshfile.serial( mesh );
			meshfile.close();
		}
		else
		{
			nlwarning ("CShapeBank::load() : Can't open file %s", shapeName.c_str());
		}

		if (mesh.getShapePointer() != NULL)
		{
			// Add the shape to the map.
			add( shapeName, mesh.getShapePointer() );
		}
	}
}

// ***************************************************************************

void CShapeBank::loadAsync (const std::string &shapeNameNotLwr, IDriver *pDriver, const CVector &position, bool *bSignal, uint selectedTexture)
{
	string	shapeName= toLower(shapeNameNotLwr);

	TShapeMap::iterator smIt = ShapeMap.find(shapeName);
	if (smIt != ShapeMap.end())
		return;
	_pDriver = pDriver; // Backup the pointer to the driver for later use
	TWaitingShapesMap::iterator wsmmIt = WaitingShapes.find (shapeName);

	// First time this shape is loaded ?
	bool firstTime = wsmmIt == WaitingShapes.end();

	if (firstTime)
		wsmmIt = WaitingShapes.insert (TWaitingShapesMap::value_type(shapeName, CWaitingShape())).first;

	// Add a reference to it
	CWaitingShape &rWS = wsmmIt->second;

	// Insert a new signal pointer
	rWS.Signal.insert (bSignal);

	// Add a signal
	rWS.RefCnt += 1;

	// Launch an async mesh loader the first time
	if (firstTime)
		CAsyncFileManager3D::getInstance().loadMesh (shapeName, &(wsmmIt->second.ShapePtr), pDriver, position, selectedTexture);
}

// ***************************************************************************

void CShapeBank::cancelLoadAsync (const std::string &shapeNameNotLwr)
{
	string	shapeName= toLower(shapeNameNotLwr);

	TWaitingShapesMap::iterator wsmmIt = WaitingShapes.find(shapeName);
	if (wsmmIt != WaitingShapes.end())
	{
		wsmmIt->second.RefCnt -= 1;
		if (wsmmIt->second.RefCnt == 0)
		{
			// nlinfo("unloadasync %s", shapeName);
			CAsyncFileManager3D::getInstance().cancelLoadMesh (shapeName);

			// If the state is not Delete, or Error, then the mesh has not been added to the map.
			if(wsmmIt->second.State!=AsyncLoad_Delete && wsmmIt->second.State!=AsyncLoad_Error)
			{
				/* but it can still have been loaded:
					- just ended in above cancelLoadMesh() because it was the current task
					- ended in async far before this cancelLoadAsync() call, but processWaintingShape() still
						not called on it
				   In this case we have to delete this shape, or even cancel texture upload!
				   NB: don't forget that the load can still be a fail too....
				*/
				IShape	*shape= wsmmIt->second.ShapePtr;
				if(shape!=NULL && shape!=(IShape*)-1)
				{
					// this ptr should not be added to the map
					nlassert(ShapePtrToShapeInfo.find(shape)==ShapePtrToShapeInfo.end());

					// Before deleting this shape, we must ensure it is not currently uploading texture
					if(wsmmIt->second.State==AsyncLoad_Texture)
					{
						/* if it was uploading a texture, then force him to end, else may have a bug
							in this scenario (very improbable, but possible I think):
								- a mesh is loaded asynchronously, and reference a texture "pipo.tga"
								- mesh load async is ended, and the texture is not found in the driver
									=> texture generate()-d too
								- mesh state is AsyncLoad_Texture, and begin (but doesn't end) to upload the texture
								- another mesh is created syncrhonously using also this texture (thus
									found in driver, and so just referencing it, no generate)
								- the async mesh is then canceled, while the texture has not end to load!
								- the texture is still in memory (the sync mesh still point to it), but with
									partialy uploaded data!
						*/
						// \todo yoyo: should be very rare, and don't know if really happens. must do tests
						//forceEndUpLoadTexture(wsmmIt->second);
					}

					// then delete this shape
					delete shape;
					wsmmIt->second.ShapePtr= NULL;
				}
			}

			// erase this waiting shape
			WaitingShapes.erase (wsmmIt); // Delete the waiting shape
		}
	}
}

// ***************************************************************************

bool CShapeBank::isShapeWaiting ()
{
	return !WaitingShapes.empty();
}

// ***************************************************************************

void CShapeBank::add (const string &shapeNameNotLwr, IShape* pShp)
{
	nlassert(pShp);
	string	shapeName= toLower(shapeNameNotLwr);

	// request a system mem geometry copy?
	if(pShp && _ShapeNeedingSystemGeometryCopy.find(shapeName)!=_ShapeNeedingSystemGeometryCopy.end())
	{
		// make a copy of the geometry, in RAM
		pShp->buildSystemGeometry();
	}

	// Is the shape name already used ?
	TShapeMap::iterator smIt = ShapeMap.find( shapeName );
	if( smIt == ShapeMap.end() )
	{
		// No ok so lets add the smart pointer
		CSmartPtr<IShape> spShape = pShp;
		ShapeMap[shapeName] = spShape;

		// create the shape info
		CShapeInfo siTemp;
		siTemp.sShpName = shapeName;
		siTemp.pShpCache = getShapeCachePtrFromShapeName( shapeName );
		// Is the shape has a valid shape cache ?
		if( siTemp.pShpCache == NULL )
		{
			// No -> link to default (which do the UpdateShapeInfo)
			siTemp.pShpCache = getShapeCachePtrFromShapeCacheName( "default" );
			// Add the shape to the default shape cache
			ShapePtrToShapeInfo[pShp]= siTemp;
			ShapeNameToShapeCacheName[shapeName]= "default";
		}
		else
		{
			// Yes -> add or replace the shape info
			ShapePtrToShapeInfo[pShp] = siTemp;
		}
	}
}

// ***************************************************************************

void CShapeBank::addShapeCache(const string &shapeCacheName)
{
	TShapeCacheMap::iterator scmIt = ShapeCacheNameToShapeCache.find( shapeCacheName );
	if( scmIt == ShapeCacheNameToShapeCache.end() )
	{
		// Not found so add it
		ShapeCacheNameToShapeCache.insert(TShapeCacheMap::value_type(shapeCacheName,CShapeCache()));
	}
}

// ***************************************************************************

void CShapeBank::removeShapeCache(const std::string &shapeCacheName)
{
	if( shapeCacheName == "default" )
		return;

	// Free the shape cache
	CShapeCache *pShpCache = getShapeCachePtrFromShapeCacheName( shapeCacheName );
	if( pShpCache == NULL )
		return;
	pShpCache->MaxSize = 0;
	checkShapeCache( pShpCache );

	// Remove it
	ShapeCacheNameToShapeCache.erase( shapeCacheName );

	// All links are redirected to the default cache
	TShapeCacheNameMap::iterator scnIt = ShapeNameToShapeCacheName.begin();
	while( scnIt != ShapeNameToShapeCacheName.end() )
	{
		if( scnIt->second == shapeCacheName )
			scnIt->second = "default";
		++scnIt;
	}
}

// ***************************************************************************

void CShapeBank::reset()
{
	// Parse map ShapeCacheNameToShapeCache to delete all caches
	TShapeCacheMap::iterator scmIt = ShapeCacheNameToShapeCache.begin();
	while( scmIt != ShapeCacheNameToShapeCache.end() )
	{
		CShapeCache *pShpCache = getShapeCachePtrFromShapeCacheName( scmIt->first );
		if( pShpCache == NULL )
			nlstop; // Should never happen
		pShpCache->MaxSize = 0;
		checkShapeCache( pShpCache );

		++scmIt;
	}
	ShapeNameToShapeCacheName.clear();
	ShapeCacheNameToShapeCache.clear();
	addShapeCache( "default" );
}

// ***************************************************************************

void CShapeBank::setShapeCacheSize(const string &shapeCacheName, sint32 maxSize)
{
	TShapeCacheMap::iterator scmIt = ShapeCacheNameToShapeCache.find( shapeCacheName );
	if( scmIt != ShapeCacheNameToShapeCache.end() )
	{
		scmIt->second.MaxSize = maxSize;
		checkShapeCache(getShapeCachePtrFromShapeCacheName(shapeCacheName));
	}
}

// ***************************************************************************
sint CShapeBank::getShapeCacheFreeSpace(const std::string &shapeCacheName) const
{
	TShapeCacheMap::const_iterator scmIt = ShapeCacheNameToShapeCache.find( shapeCacheName );
	if( scmIt != ShapeCacheNameToShapeCache.end() )
	{
		return scmIt->second.MaxSize - (sint)scmIt->second.Elements.size();
	}
	return 0;
}

// ***************************************************************************

void CShapeBank::linkShapeToShapeCache(const string &shapeNameNotLwr, const string &shapeCacheName)
{
	string	shapeName= toLower(shapeNameNotLwr);

	for(;;)
	{
		// Shape exist?
		IShape	*shapePtr= getShapePtrFromShapeName(shapeName);
		if(shapePtr == NULL)
			// No, but still link the shape name to the shapeCache name.
			break;
		// Is the shape cache exist ?
		CShapeCache *shapeCachePtr = getShapeCachePtrFromShapeCacheName( shapeCacheName );
		if( shapeCachePtr == NULL )
			// abort, since cannot correctly link to a valid shapeCache
			return;

		// Try to set to the same shape Cache as before?
		CShapeInfo	&shapeInfo= ShapePtrToShapeInfo[shapePtr];
		if( shapeCachePtr ==  shapeInfo.pShpCache)
			// abort, since same cache name / cache ptr
			return;

		// If The shape is In the cache of another Shape Cache, abort.
		if( shapeInfo.isAdded )
			// Abort, because impossible.
			return;

		// Is the shape is present ?
		// Yes -> Update the ShapeInfo
		shapeInfo.pShpCache= shapeCachePtr;

		break;
	}

	// change the cache name of the shape
	ShapeNameToShapeCacheName[shapeName] = shapeCacheName;
}

// ***************************************************************************

CShapeBank::CShapeCache* CShapeBank::getShapeCachePtrFromShapePtr(IShape* pShp)
{
	TShapeInfoMap::iterator scfpmIt = ShapePtrToShapeInfo.find( pShp );
	if( scfpmIt != ShapePtrToShapeInfo.end() )
	{
		return scfpmIt->second.pShpCache;
	}
	return NULL;
}

// ***************************************************************************

IShape* CShapeBank::getShapePtrFromShapeName(const std::string &pShpName)
{
	TShapeMap::iterator smIt = ShapeMap.find(pShpName);
	if( smIt != ShapeMap.end() )
	{
		// TMP
		IShape *ptr = (IShape*)(smIt->second);
		return ptr;
	}
	return NULL;
}

// ***************************************************************************

CShapeBank::CShapeCache* CShapeBank::getShapeCachePtrFromShapeCacheName(const string &shapeCacheName)
{
	TShapeCacheMap::iterator scmIt = ShapeCacheNameToShapeCache.find( shapeCacheName );
	if( scmIt != ShapeCacheNameToShapeCache.end())
	{
		return &(scmIt->second);
	}
	return NULL;
}

// ***************************************************************************

const string* CShapeBank::getShapeNameFromShapePtr(IShape* pShp) const
{
	TShapeInfoMap::const_iterator scfpmIt = ShapePtrToShapeInfo.find( pShp );
	if( scfpmIt != ShapePtrToShapeInfo.end() )
	{
		return &(scfpmIt->second.sShpName);
	}
	return NULL;
}

// ***************************************************************************

CShapeBank::CShapeCache* CShapeBank::getShapeCachePtrFromShapeName(const std::string &shapeName)
{
	TShapeCacheNameMap::iterator scnIt = ShapeNameToShapeCacheName.find( shapeName );
	if( scnIt != ShapeNameToShapeCacheName.end() )
	{
		return getShapeCachePtrFromShapeCacheName(scnIt->second);
	}
	return NULL;
}

// ***************************************************************************

void CShapeBank::checkShapeCache(CShapeCache* pShpCache)
{
	if( pShpCache != NULL )
	while( (sint)pShpCache->Elements.size() > pShpCache->MaxSize )
	{
		// Suppress the last shape of the cache
		IShape *pShp = pShpCache->Elements.back();
		// Physical suppression because we own the last smart pointer on the shape
		ShapeMap.erase(*getShapeNameFromShapePtr(pShp));
		// delete information associated with the shape
		ShapePtrToShapeInfo.erase( pShp );
		// remove from queue
		pShpCache->Elements.pop_back();
	}
}


// ***************************************************************************
bool CShapeBank::isShapeCache(const std::string &shapeCacheName) const
{
	return ShapeCacheNameToShapeCache.find(shapeCacheName) != ShapeCacheNameToShapeCache.end();
}

// ***************************************************************************
void CShapeBank::preLoadShapes(const std::string &shapeCacheName,
	const std::vector<std::string> &listFile, const std::string &wildCardNotLwr, NLMISC::IProgressCallback *progress, bool flushTextures /*= false*/, IDriver *drv /*= NULL*/)
{
	// Abort if cache don't exist.
	if(!isShapeCache(shapeCacheName))
		return;

	// lower case
	string wildCard= toLower(wildCardNotLwr);

	// For all files
	for(uint i=0;i<listFile.size();i++)
	{
		// Progress bar
		if (progress)
			progress->progress ((float)i/(float)listFile.size ());

		string	fileName= toLower(CFile::getFilename(listFile[i]));
		// if the file is ok for the wildCard, process it
		if( testWildCard(fileName.c_str(), wildCard.c_str()) )
		{
			// link the shape to the shapeCache
			linkShapeToShapeCache(fileName, shapeCacheName);

			// If !present in the shapeBank
			if( getPresentState(fileName)==CShapeBank::NotPresent )
			{
				// Don't load it if no more space in the cache
				if( getShapeCacheFreeSpace(shapeCacheName)>0 )
				{
					// load it.
					load(fileName);

					// If success
					if( getPresentState(fileName)==CShapeBank::Present )
					{
						// When a shape is first added to the bank, it is not in the cache.
						// add it and release it to force it to be in the cache.
						IShape	*shp= addRef(fileName);
						if(shp)
						{
							//nlinfo("Loading %s", CPath::lookup(fileName.c_str(), false, false).c_str());
							if (flushTextures && drv)
							{
								shp->flushTextures(*drv, 0);
							}
							release(shp);
						}
					}
				}
			}
		}
	}

}

// ***************************************************************************
void CShapeBank::buildSystemGeometryForshape(const std::string &shapeName)
{
	_ShapeNeedingSystemGeometryCopy.insert(toLower(shapeName));
}


}
