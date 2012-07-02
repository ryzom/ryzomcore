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

#include "nel/3d/async_texture_manager.h"
#include "nel/3d/async_file_manager_3d.h"
#include "nel/3d/mesh_base_instance.h"
#include "nel/3d/driver.h"


using	namespace std;
using	namespace NLMISC;

namespace NL3D
{

// ***************************************************************************
#define	NL3D_ATM_MIN_DISTANCE		1.0f

// ***************************************************************************
CAsyncTextureManager::CTextureEntry::CTextureEntry()
{
	IsTextureEntry= true;

	Loaded= false;
	UpLoaded= false;
	CanHaveLOD= false;
	BuildFromHLSManager= false;
	HLSManagerTextId= -1;
	BaseSize= 0;
	TotalTextureSizeAsked= 0;
}


// ***************************************************************************
void		CAsyncTextureManager::CTextureEntry::createCoarseBitmap()
{
	// the texture must exist.
	nlassert(Texture);
	nlassert(Texture->getSize()>0);

	// copy the bitmap.
	CoarseBitmap= *Texture;
	// remove all mipmaps, and convert to DXTC1 (if possible, ie if was DXTC5 or DXTC3 as example)
	CoarseBitmap.releaseMipMaps();
	// TODODO: conversion to DXTC1
	CoarseBitmap.convertToType(CBitmap::DXTC1);
}


// ***************************************************************************
CAsyncTextureManager::CTextureLod::CTextureLod()
{
	IsTextureEntry= false;

	TextureEntry= NULL;
	Weight= 0;
	Level= 0;
	Loaded= false;
	UpLoaded= false;
	ExtraSize= 0;
}


// ***************************************************************************
CAsyncTextureManager::~CAsyncTextureManager()
{
	// For all remaining textures, delete them.
	for(uint i=0;i<_TextureEntries.size();i++)
	{
		if(_TextureEntries[i])
			deleteTexture(i);
	}

	// there must be no waitting textures, nor map, nor current upload texture
	nlassert(_WaitingTextures.empty() && _TextureEntryMap.empty() && _CurrentUploadTexture==NULL
		&& _CurrentTextureLodLoaded==NULL);
}

// ***************************************************************************
CAsyncTextureManager::CAsyncTextureManager()
{
	_BaseLodLevel= 3;
	_MaxLodLevel= 1;
	_MaxUploadPerFrame= 65536;
	_MaxHLSColoringPerFrame= 20*1024;
	_CurrentUploadTexture= NULL;
	_MaxTotalTextureSize= 10*1024*1024;
	_TotalTextureSizeAsked= 0;
	_LastTextureSizeGot= 0;

	// Do not share this texture, to force uploading of the lods.
	_CurrentTextureLodLoaded= NULL;

	// For Texture profiling
	_TextureCategory= new ITexture::CTextureCategory("ASYNC ENTITY MANAGER");
}


// ***************************************************************************
void			CAsyncTextureManager::setupLod(uint baseLevel, uint maxLevel)
{
	nlassert(baseLevel>=maxLevel);
	_BaseLodLevel= baseLevel;
	_MaxLodLevel= maxLevel;
}


// ***************************************************************************
void			CAsyncTextureManager::setupMaxUploadPerFrame(uint maxup)
{
	if(maxup>0)
		_MaxUploadPerFrame= maxup;
}

// ***************************************************************************
void			CAsyncTextureManager::setupMaxHLSColoringPerFrame(uint maxCol)
{
	if(maxCol>0)
		_MaxHLSColoringPerFrame= maxCol;
}

// ***************************************************************************
void			CAsyncTextureManager::setupMaxTotalTextureSize(uint maxText)
{
	_MaxTotalTextureSize= maxText;
}


// ***************************************************************************
uint			CAsyncTextureManager::addTextureRef(const string &textNameNotLwr, CMeshBaseInstance *instance, const NLMISC::CVector &position)
{
	uint	ret;

	// lower case name
	string	textName = toLower(textNameNotLwr);

	// find the texture in map
	ItTextureEntryMap	it;
	it= _TextureEntryMap.find(textName);

	// not found, create.
	if(it==_TextureEntryMap.end())
	{
		// search a free id.
		uint	i= (uint)_TextureEntries.size();
		if(!_FreeTextureIds.empty())
		{
			i= _FreeTextureIds.back();
			_FreeTextureIds.pop_back();
		}
		// resize if needed.
		if(i>=_TextureEntries.size())
		{
			_TextureEntries.push_back(NULL);
			_FreeTextureIds.reserve(_TextureEntries.capacity());
		}

		// alloc new.
		CTextureEntry	*text= new CTextureEntry();
		_TextureEntries[i]= text;
		text->Texture= new CTextureFile;
		// Do not allow degradation.
		text->Texture->setAllowDegradation(false);
		// For Profiling
		text->Texture->setTextureCategory(_TextureCategory);

		// add to map.
		it= _TextureEntryMap.insert(make_pair(textName, i)).first;
		// bkup the it for deletion
		text->ItMap= it;

		// Start Color or Async loading.
		text->Texture->setFileName(textName);
		// First try with the HLSManager
		sint	colorTextId= HLSManager.findTexture(textName);
		// If found
		if(colorTextId!=-1)
		{
			// Mark the texture as Loaded, and ready to colorize (done in update()).
			text->Loaded= true;
			text->BuildFromHLSManager= true;
			text->HLSManagerTextId= colorTextId;
		}
		// else must async load it.
		else
		{
			// start to load a small DDS version if possible
			text->Texture->setMipMapSkipAtLoad(_BaseLodLevel);
			// load it async.
			CAsyncFileManager3D::getInstance().loadTexture(text->Texture, &text->Loaded, position);
		}
		// Add to a list so we can check each frame if it has ended.
		_WaitingTextures.push_back(i);
	}

	// get the id of the text entry.
	ret= it->second;

	// add this instance to the list of ones which use this texture.
	CTextureEntry	*text= _TextureEntries[ret];
	text->Instances.push_back(instance);

	// if the texture is not yet ready, must increment the instance refCount.
	if(!text->UpLoaded)
		instance->_AsyncTextureToLoadRefCount++;

	return ret;
}


// ***************************************************************************
void			CAsyncTextureManager::deleteTexture(uint id)
{
	CTextureEntry	*text= _TextureEntries[id];


	// **** Stop AsyncLoading/UpLoading of main texture.

	// stop async loading if not ended
	if(!text->Loaded)
	{
		CAsyncFileManager3D::getInstance().cancelLoadTexture(text->Texture);
	}

	// remove map entry
	_TextureEntryMap.erase(text->ItMap);

	// remove in list of waiting textures
	vector<uint>::iterator	itWait= find(_WaitingTextures.begin(),_WaitingTextures.end(), id);
	if(itWait!=_WaitingTextures.end())
		_WaitingTextures.erase(itWait);

	// If it was the currently uploaded one, abort
	if(_CurrentUploadTexture==text)
	{
		_CurrentUploadTexture= NULL;
	}

	// If not uploaded.
	if(!text->UpLoaded)
	{
		// For all its remainding instances, dec refcount
		for(uint i=0;i<text->Instances.size();i++)
		{
			text->Instances[i]->_AsyncTextureToLoadRefCount--;
		}
	}

	// remove from bench
	_TotalTextureSizeAsked-= text->TotalTextureSizeAsked;


	// **** Stop AsyncLoading/UpLoading of HDLod 's texture.

	// Check if must stop TextureLod loading/uploading.
	CTextureLod		*textLod= &text->HDLod;
	if(textLod==_CurrentTextureLodLoaded)
	{
		// stop the async loading if not ended.
		if(!textLod->Loaded)
		{
			CAsyncFileManager3D::getInstance().cancelLoadTexture(textLod->Texture);
		}
		// stop uploading if was me
		if(_CurrentUploadTexture==textLod)
		{
			_CurrentUploadTexture= NULL;
		}
		// stop loading me.
		_CurrentTextureLodLoaded= NULL;
	}

	// At last delete texture entry.
	delete text;
	_TextureEntries[id]= NULL;
	// add a new free id.
	_FreeTextureIds.push_back(id);
}


// ***************************************************************************
void			CAsyncTextureManager::releaseTexture(uint id, CMeshBaseInstance *instance)
{
	nlassert(id<_TextureEntries.size());
	nlassert(_TextureEntries[id]);

	// find an instance in this texture an remove it.
	CTextureEntry	*text= _TextureEntries[id];
	uint			instSize= (uint)text->Instances.size();
	for(uint i=0;i<instSize;i++)
	{
		if(text->Instances[i]== instance)
		{
			// Must first release the refCount if the texture is not uploaded
			if(!text->UpLoaded)
				text->Instances[i]->_AsyncTextureToLoadRefCount--;
			// remove it by swapping with last texture
			text->Instances[i]= text->Instances[instSize-1];
			text->Instances.pop_back();
			// must stop: remove only the first occurence of instance.
			break;
		}
	}

	// if no more instance occurence, the texture is no more used => release it.
	if(text->Instances.empty())
	{
		// do all the good stuff
		deleteTexture(id);
	}
}

// ***************************************************************************
bool			CAsyncTextureManager::isTextureUpLoaded(uint id) const
{
	nlassert(id<_TextureEntries.size());
	nlassert(_TextureEntries[id]);
	return _TextureEntries[id]->UpLoaded;
}


// ***************************************************************************
const NLMISC::CBitmap	*CAsyncTextureManager::getCoarseBitmap(uint id) const
{
	if(id>=_TextureEntries.size())
		return NULL;
	CTextureEntry	*textEntry= _TextureEntries[id];
	if(!textEntry)
		return NULL;

	// if the textEntry not uploaded, return NULL
	if(!textEntry->UpLoaded)
		return NULL;

	// ok return the CoarseBitmap
	return &textEntry->CoarseBitmap;
}


// ***************************************************************************
void			CAsyncTextureManager::update(IDriver *pDriver)
{
	uint	nTotalUploaded = 0;
	uint	nTotalColored = 0;

	// if no texture to upload, get the next one
	if(_CurrentUploadTexture==NULL)
		getNextTextureToUpLoad(nTotalColored, pDriver);

	// while some texture to upload
	while(_CurrentUploadTexture)
	{
		ITexture	*pText= _CurrentUploadTexture->Texture;
		if(uploadTexturePart(pText, pDriver, nTotalUploaded))
		{
			// Stuff for TextureEntry
			if(_CurrentUploadTexture->isTextureEntry())
			{
				uint	i;
				CTextureEntry	*textEntry= static_cast<CTextureEntry*>(_CurrentUploadTexture);
				// If we are here, the texture is finally entirely uploaded. Compile it!
				textEntry->UpLoaded= true;
				// Can Have lod if texture is DXTC and have mipMaps! Also disalbe if system disable it
				textEntry->CanHaveLOD= validDXTCMipMap(pText) && _BaseLodLevel>_MaxLodLevel;
				// compute the size it takes in VRAM
				uint	baseMipMapSize= pText->getSize(0)*CBitmap::bitPerPixels[pText->getPixelFormat()]/8;
				// full size with mipmap
				textEntry->BaseSize= (uint)(baseMipMapSize*1.33f);
				// UpLoaded !! => signal all instances.
				for(i=0;i<textEntry->Instances.size();i++)
				{
					textEntry->Instances[i]->_AsyncTextureToLoadRefCount--;
				}

				// Create the coarse bitmap with the text (NB: still in memory here)
				textEntry->createCoarseBitmap();

				// If CanHaveLOD, create now the lods entries.
				if(textEntry->CanHaveLOD)
				{
					/* Allow only the MaxLod to be loaded async
						This is supposed to be faster since a fseek is much longer than a texture Read.
						Then it is more intelligent to read only One texture (the High Def), than to try to
						read intermediate ones (512, 256, 128) because this made 3 more fseek.
					*/
					// create only the MaxLod possible entry
					CTextureLod		&textLod= textEntry->HDLod;
					// fill textLod
					textLod.TextureEntry= textEntry;
					textLod.Level= _MaxLodLevel;
					// extra size of the lod only (important for LoadBalacing in updateTextureLodSystem())
					textLod.ExtraSize= textEntry->BaseSize*(1<<(2*(_BaseLodLevel-_MaxLodLevel))) - textEntry->BaseSize;
					// not yet loaded/upLoaded
					textLod.Loaded= false;
					textLod.UpLoaded= false;
				}

				// compute texture size for bench
				textEntry->TotalTextureSizeAsked= textEntry->BaseSize + textEntry->HDLod.ExtraSize;

				// Add texture size to global texture size
				_TotalTextureSizeAsked+= textEntry->TotalTextureSizeAsked;
			}
			// else, stuff for textureLod.
			else
			{
				CTextureLod		*textLod= static_cast<CTextureLod*>(_CurrentUploadTexture);
				// Swap the uploaded Driver Handle with the Main texture.
				pDriver->swapTextureHandle(*textLod->Texture, *textLod->TextureEntry->Texture);
				// Flag the Lod.
				textLod->UpLoaded= true;
				// Ok, ended to completly load this textureLod.
				_CurrentTextureLodLoaded= NULL;
			}

			// finally uploaded in VRAM, can release the RAM texture memory
			pText->release();

			// if not break because can't upload all parts, get next texture to upload
			_CurrentUploadTexture= NULL;
			getNextTextureToUpLoad(nTotalColored, pDriver);
		}
		else
			// Fail to upload all, abort.
			return;
	}
}


// ***************************************************************************
bool			CAsyncTextureManager::uploadTexturePart(ITexture *pText, IDriver *pDriver, uint &nTotalUploaded)
{
	uint		nMipMap;
	nMipMap = pText->getMipMapCount();


	// If this is the start of uploading, setup the texture in driver.
	if(_CurrentUploadTextureMipMap==0 && _CurrentUploadTextureLine==0)
	{
		// If the texture is not a valid DXTC with mipmap
		if(!validDXTCMipMap(pText))
		{
			/* For now, prefer do nothing, because this may be an error (texture not found)
				and the texture may not be used at all, so don't take VRAM for nothing.
				=> if the texture is used, it will be loaded synchronously by the caller later in the process
				=> frame freeze.
			*/
			/*
			// upload All now.
			// MipMap generation and compression may be done here => Maybe Big Freeze.
			// approximate*2 instead of *1.33 for mipmaps.
			uint	nWeight = pText->getSize (0) * 2;
			nWeight= (nWeight*CBitmap::bitPerPixels[pText->getPixelFormat()])/8;
			nTotalUploaded+= nWeight;
			pDriver->setupTexture(*pText);
			return true;*/
			return true;
		}
		else
		{
			// Create the texture only and do not upload anything
			bool isRel = pText->getReleasable ();
			pText->setReleasable (false);
			bool isAllUploaded = false;
			/* Even if the shared texture is still referenced and so still exist in driver, we MUST recreate with good size
				the texture. This is important for Texture Memory Load Balancing
				(this may means that is used elsewhere than in the CAsyncTextureManager)
				Hence: bMustRecreateSharedTexture==true
			*/
			pDriver->setupTextureEx (*pText, false, isAllUploaded, true);
			pText->setReleasable (isRel);
			// if the texture is already uploaded, abort partial uploading.
			if (isAllUploaded)
				return true;
		}
	}


	// try to upload all mipmap
	for(; _CurrentUploadTextureMipMap<nMipMap; _CurrentUploadTextureMipMap++)
	{
		CRect zeRect;
		uint nMM= _CurrentUploadTextureMipMap;

		// What is left to upload ?
		uint	nWeight = pText->getSize (nMM) - _CurrentUploadTextureLine*pText->getWidth(nMM);
		nWeight= (nWeight*CBitmap::bitPerPixels[pText->getPixelFormat()])/8;

		if ((nTotalUploaded  + nWeight) > _MaxUploadPerFrame)
		{
			// We cannot upload the whole mipmap -> we have to cut it
			uint nSizeToUpload = _MaxUploadPerFrame - nTotalUploaded ;
			// DXTC => min block of 4x4
			uint nLineWeight = (max(pText->getWidth(nMM), (uint32)4)*CBitmap::bitPerPixels[pText->getPixelFormat()])/8;
			uint nNbLineToUpload = nSizeToUpload / nLineWeight;
			// Upload 4 line by 4 line, and upload at leat one 4*line.
			nNbLineToUpload = nNbLineToUpload / 4;
			nNbLineToUpload= max(nNbLineToUpload, 1U);
			nNbLineToUpload *= 4;
			// comput rect to upload
			uint32 nNewLine = _CurrentUploadTextureLine + nNbLineToUpload;
			nNewLine= min(nNewLine, pText->getHeight(nMM));
			zeRect.set (0, _CurrentUploadTextureLine, pText->getWidth(nMM), nNewLine);
			_CurrentUploadTextureLine = nNewLine;
			// if fill all the mipmap, must go to next
			if (_CurrentUploadTextureLine == pText->getHeight(nMM))
			{
				_CurrentUploadTextureLine = 0;
				_CurrentUploadTextureMipMap++;
			}
		}
		else
		{
			// We can upload the whole mipmap (or the whole rest of the mipmap)
			zeRect.set (0, _CurrentUploadTextureLine, pText->getWidth(nMM), pText->getHeight(nMM));
			_CurrentUploadTextureLine= 0;
		}

		// upload the texture
		pDriver->uploadTexture (*pText, zeRect, (uint8)nMM);

		nTotalUploaded += nWeight;
		// If outpass max allocated upload, abort.
		if (nTotalUploaded > _MaxUploadPerFrame)
			return false;
	}

	return true;
}


// ***************************************************************************
void			CAsyncTextureManager::getNextTextureToUpLoad(uint &nTotalColored, IDriver *pDriver)
{
	// Reset texture uploading
	_CurrentUploadTexture= NULL;
	_CurrentUploadTextureMipMap= 0;
	_CurrentUploadTextureLine= 0;

	// Search in WaitingTextures if one has ended async loading
	vector<uint>::iterator	it;
	for(it=_WaitingTextures.begin();it!=_WaitingTextures.end();it++)
	{
		CTextureEntry	*text= _TextureEntries[*it];
		// If Async loading done.
		if(text->Loaded)
		{
			// Is it a "texture to color" with HLSManager? yes=> color it now.
			if(text->BuildFromHLSManager)
			{
				// If not beyond the max coloring texture
				if(nTotalColored<_MaxHLSColoringPerFrame)
				{
					// Build the texture directly in the TextureFile.
					nlverify(HLSManager.buildTexture(text->HLSManagerTextId, *text->Texture));
					// Must validate the textureFile generation. NB: little weird since this is not really a textureFile.
					// But it is the easier way to do it.
					text->Texture->validateGenerateFlag();
					// compute the texture size (approx). NB: DXTC5 means 1 pixel==1 byte.
					uint	size= (uint)(text->Texture->getSize(0)*1.33);
					// Add it to the num of colorised texture done in current update().
					nTotalColored+= size;
				}
				// Else must quit and don't update any more texture this frame (_CurrentUploadTexture==NULL)
				else
					return;
			}

			// upload this one
			_CurrentUploadTexture= text;
			// remove it from list of waiting textures
			_WaitingTextures.erase(it);
			// found => end.
			return;
		}
	}

	// If here, and if no more waiting textures, update the Lod system.
	if(_WaitingTextures.empty())
	{
		// if end to load the current lod.
		if(_CurrentTextureLodLoaded && _CurrentTextureLodLoaded->Loaded)
		{
			// upload this one
			_CurrentUploadTexture= _CurrentTextureLodLoaded;
			return;
		}

		// if no Lod texture currently loading, try to load/unload one
		if(_CurrentTextureLodLoaded == NULL)
		{
			updateTextureLodSystem(pDriver);
		}
	}
}


// ***************************************************************************
bool			CAsyncTextureManager::validDXTCMipMap(ITexture *pText)
{
	return pText->getMipMapCount()>1 && (
		pText->getPixelFormat() == CBitmap::DXTC1 ||
		pText->getPixelFormat() == CBitmap::DXTC1Alpha ||
		pText->getPixelFormat() == CBitmap::DXTC3 ||
		pText->getPixelFormat() == CBitmap::DXTC5 );
}

// ***************************************************************************
void			CAsyncTextureManager::updateTextureLodSystem(IDriver *pDriver)
{
	sint	i;

	// the array to sort
	static	vector<CTextureLodToSort>	lodArray;
	lodArray.clear();
	uint	reserveSize= 0;

	// for each texture entry compute min distance of use
	//=============
	uint	currentBaseSize= 0;
	for(i=0;i<(sint)_TextureEntries.size();i++)
	{
		if(!_TextureEntries[i])
			continue;
		CTextureEntry	&text= *_TextureEntries[i];
		// do it only for Lodable textures
		if(text.CanHaveLOD)
		{
			text.MinDistance= FLT_MAX;
			// for all instances.
			for(uint j=0;j<text.Instances.size();j++)
			{
				float	instDist= text.Instances[j]->getAsyncTextureDistance();

				if (instDist<text.MinDistance)
				{
					text.MinPosition = text.Instances[j]->getPos();
					text.MinDistance = instDist;
				}
			}

			// avoid /0
			text.MinDistance= max(NL3D_ATM_MIN_DISTANCE, text.MinDistance);

			// how many textLods to add
			reserveSize++;

			// the minimum mem size the system take with base lod.
			currentBaseSize+= text.BaseSize;
		}
	}
	// reserve space
	lodArray.reserve(reserveSize);


	// for each texture lod compute weight, and append
	//=============
	for(i=0;i<(sint)_TextureEntries.size();i++)
	{
		if(!_TextureEntries[i])
			continue;
		CTextureEntry	&text= *_TextureEntries[i];
		// do it only for Lodable textures
		if(text.CanHaveLOD)
		{
			// This Weight is actually a screen Pixel Ratio! (divide by distance)
			CTextureLod	*textLod= &text.HDLod;
			textLod->Weight= (1<<textLod->Level) / text.MinDistance;
			// add to array
			CTextureLodToSort toSort;
			toSort.Lod = textLod;
			toSort.Position = text.MinPosition;
			lodArray.push_back(toSort);
		}
	}


	// sort
	//=============
	sort(lodArray.begin(), lodArray.end());


	// Compute lod to load/unload
	//=============
	// Compute Pivot, ie what lods have to be loaded, and what lods do not
	uint	pivot= 0;
	uint	currentWantedSize= currentBaseSize;
	uint	currentLoadedSize= currentBaseSize;
	for(i=(sint)lodArray.size()-1;i>=0;i--)
	{
		uint	lodSize= lodArray[i].Lod->ExtraSize;
		currentWantedSize+= lodSize;
		if(lodArray[i].Lod->UpLoaded)
			currentLoadedSize+= lodSize;
		// if > max allowed, stop the pivot here. NB: the pivot is included in the "must load them" part.
		if(currentWantedSize > _MaxTotalTextureSize)
		{
			pivot= i;
			break;
		}
	}
	// continue to count currentLoadedSize
	for(;i>=0;i--)
	{
		if(lodArray[i].Lod->UpLoaded)
			currentLoadedSize+= lodArray[i].Lod->ExtraSize;
	}
	// save bench.
	_LastTextureSizeGot= currentLoadedSize;


	// if the loadedSize is inferior to the wanted size, we can load a new LOD
	CTextureLodToSort	*textLod= NULL;
	bool			unload;
	if(currentLoadedSize<currentWantedSize)
	{
		unload= false;
		// search from end of the list to pivot (included), the first LOD (ie the most important) to load.
		for(i=(sint)lodArray.size()-1;i>=(sint)pivot;i--)
		{
			if(!lodArray[i].Lod->UpLoaded)
			{
				textLod= &(lodArray[i]);
				break;
			}
		}
		// One must have been found, since currentLoadedSize<currentWantedSize
		nlassert(textLod);
	}
	else
	{
		unload= true;
		// search from start to pivot (exclued), the first LOD (ie the less important) to unload.
		for(i=0;i<(sint)pivot;i++)
		{
			if(lodArray[i].Lod->UpLoaded)
			{
				textLod= &(lodArray[i]);
				break;
			}
		}
		// it is possible that not found here. It means that All is Ok!!
		if(textLod==NULL)
			// no-op.
			return;
	}


	// load/unload
	//=============
	if(!unload)
	{
		// create a new TextureFile, with no sharing system.
		nlassert(textLod->Lod->Texture==NULL);
		textLod->Lod->Texture= new CTextureFile;
		// Do not allow degradation.
		textLod->Lod->Texture->setAllowDegradation(false);
		textLod->Lod->Texture->enableSharing(false);
		textLod->Lod->Texture->setFileName(textLod->Lod->TextureEntry->Texture->getFileName());
		textLod->Lod->Texture->setMipMapSkipAtLoad(textLod->Lod->Level);
		// For Profiling
		textLod->Lod->Texture->setTextureCategory(_TextureCategory);
		// setup async loading
		_CurrentTextureLodLoaded= textLod->Lod;
		// load it async.
		CAsyncFileManager3D::getInstance().loadTexture(textLod->Lod->Texture, &textLod->Lod->Loaded, textLod->Position);
	}
	else
	{
		// Swap now the lod.
		nlassert(textLod->Lod->Texture!=NULL);
		// Swap the uploaded Driver Handle with the Main texture (ot get the Ugly one)
		pDriver->swapTextureHandle(*textLod->Lod->Texture, *textLod->Lod->TextureEntry->Texture);
		// Flag the Lod.
		textLod->Lod->UpLoaded= false;
		textLod->Lod->Loaded= false;
		// Release completly the texture in driver. (SmartPtr delete)
		textLod->Lod->Texture= NULL;
	}

}



} // NL3D
