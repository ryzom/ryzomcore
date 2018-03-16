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

#ifndef NL_ASYNC_TEXTURE_MANAGER_H
#define NL_ASYNC_TEXTURE_MANAGER_H


#include "nel/misc/types_nl.h"
#include "nel/3d/texture_file.h"
#include "nel/3d/hls_texture_manager.h"
#include <vector>
#include "nel/misc/bitmap.h"


namespace NL3D
{


class	CMeshBaseInstance;


// ***************************************************************************
/**
 * Async Loader of textures and Texture Load Balancer.
 *	Additionaly, store in RAM for each texture load a very low, DXTC1 compressed version of the texture.
 *	Used for some Lod systems.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2002
 */
class CAsyncTextureManager
{
public:
	/// User is free to add bank to this manager. Other methods are used by the async manager
	CHLSTextureManager		HLSManager;

public:

	/// Constructor
	CAsyncTextureManager();
	~CAsyncTextureManager();

	/** setup the mipMap levels.
	 *	\baseLevel When the texture is first added, it is loaded skipping the baseLevel
	 *	first mipmap
	 *	\maxLevel During time, further mipmap are loaded, according to instance position etc... maxLevel
	 *	tells where to stop. If 0, the texture will finally be entirely uploaded.
	 *	Default is 3,1.
	 */
	void			setupLod(uint baseLevel, uint maxLevel);
	/// Setup max texture upload in driver per update() call (in bytes). Default to 64K
	void			setupMaxUploadPerFrame(uint maxup);
	/// Setup max texture HLS Coloring per update() call (in bytes). Default to 20K.
	void			setupMaxHLSColoringPerFrame(uint maxCol);
	/// Setup max total texture size allowed. Default is 10Mo
	void			setupMaxTotalTextureSize(uint maxText);

	/** Add a reference to a texture owned by an instance.
	 *	If the texture still exists, only the refcount is incremented
	 *	Else if texture is found in the HLSTextureManager, it is builded (async) from it, else Begin Async loading
	 *
	 *	Therefore, only CTextureFile are possible. Also note that the texture is uploaded with mipmap by default, and
	 *	UpLoadFormat is also default (say ITexture::Auto)
	 *
	 *	If the texture file is not a DDS with mipmap, this is an error. But the system doesn't fail and
	 *	the file is entirely loaded and uploaded. The problem is that upload is not cut according to maxUpLoadPerFrame, so
	 *	some freeze may occur.
	 */
	uint			addTextureRef(const std::string &textName, CMeshBaseInstance *instance, const NLMISC::CVector &position);

	/// release a texture-instance tuple. The texture is released if no more instance use it.
	void			releaseTexture(uint id, CMeshBaseInstance *instance);

	/// tells if a texture is loaded in the driver (ie. ready to use)
	bool			isTextureUpLoaded(uint id) const;

	/** get the RAM LowDef version of a texture. Used For CLodCharacters
	 *	return NULL if bad Id or if the texture is still not loaded.
	 *	The bitmap returned has no mipmaps and should be in DXTC1 (not guaranteed).
	 */
	const NLMISC::CBitmap	*getCoarseBitmap(uint id) const;


	/** update the manager. New loaded textures are uploaded. Instances are updated to know if all their
	 *	pending textures have been uploaded.
	 */
	void			update(IDriver *pDriver);


	/// get the async texture size asked (ie. maybe bigger than MaxTotalTextureSize).
	uint			getTotalTextureSizeAsked() const {return _TotalTextureSizeAsked;}
	/// get what the system really allows
	uint			getLastTextureSizeGot() const {return _LastTextureSizeGot;}


// ***************************************************************************
private:

	typedef	std::map<std::string, uint>	TTextureEntryMap;
	typedef	TTextureEntryMap::iterator	ItTextureEntryMap;


	// A base texture uploadable.
	class	CTextureBase
	{
	public:
		// the texture currently loaded / uploaded.
		NLMISC::CSmartPtr<CTextureFile>		Texture;

		bool	isTextureEntry() const {return IsTextureEntry;}
	protected:
		bool	IsTextureEntry;
	};


	class	CTextureEntry;

	// A Lod version of a texture entry.
	class	CTextureLod : public CTextureBase
	{
	public:
		CTextureLod();

		// A Ptr on the real texture used.
		CTextureEntry						*TextureEntry;
		// Weight of the lod, according to distance and level.
		float								Weight;
		// the level of this Lod. 0 means full original texture resolution.
		uint8								Level;
		// True if loading has ended
		bool								Loaded;
		// True if TextureEntry has at least this lod in VRAM
		bool								UpLoaded;
		// The size that this lod takes in VRAM (minus TextureEntry->BaseSize)
		uint								ExtraSize;
	};


	struct CTextureLodToSort
	{
		CTextureLod		*Lod;
		NLMISC::CVector	Position;
		bool			operator<(const CTextureLodToSort &other) const
		{
			return Lod->Weight<other.Lod->Weight;
		}
	};

	// A texture entry
	class	CTextureEntry : public CTextureBase
	{
	public:
		// The it in the map.
		ItTextureEntryMap					ItMap;
		// true if async loading has ended
		bool								Loaded;
		// true if the texture is loaded in the driver (at least the coarsest level).
		bool								UpLoaded;
		// true if first loading ended, and if DXTC with mipmap
		bool								CanHaveLOD;
		// true if this texture must be built from the HLSManager (at first load)
		bool								BuildFromHLSManager;
		// if BuildFromHLSManager, gives the text id in the manager
		sint								HLSManagerTextId;

		// Base Size of the texture, without HDLod
		uint								BaseSize;
		// list of instances currently using this texture.
		std::vector<CMeshBaseInstance*>		Instances;
		// min distance of all Instances.
		float								MinDistance;
		// min position.
		NLMISC::CVector						MinPosition;
		// with all mipmaps loaded, what place this takes.
		uint								TotalTextureSizeAsked;

		// The High Def Lod.
		CTextureLod							HDLod;

		// The Coarse Bitmap stored in RAM for CLod
		NLMISC::CBitmap						CoarseBitmap;

	public:
		CTextureEntry();

		void		createCoarseBitmap();
	};


private:
	uint								_BaseLodLevel, _MaxLodLevel;
	uint								_MaxUploadPerFrame;
	uint								_MaxHLSColoringPerFrame;
	uint								_MaxTotalTextureSize;
	uint								_TotalTextureSizeAsked;
	uint								_LastTextureSizeGot;

	// Textures Entries.
	std::vector<CTextureEntry*>			_TextureEntries;
	std::vector<uint>					_FreeTextureIds;
	TTextureEntryMap					_TextureEntryMap;
	std::vector<uint>					_WaitingTextures;

	// Upload of texture piece by piece.
	CTextureBase						*_CurrentUploadTexture;
	uint								_CurrentUploadTextureMipMap;
	uint								_CurrentUploadTextureLine;

	// The current HDLod async loaded (NB: loaded / or upLoaded)
	CTextureLod							*_CurrentTextureLodLoaded;

	// For texture profiling
	NLMISC::CSmartPtr<ITexture::CTextureCategory>	_TextureCategory;

private:

	static bool		validDXTCMipMap(ITexture *pText);

	// delete the texture and all references in map/array, instance refcount etc...
	void			deleteTexture(uint id);

	// Fill _CurrentUploadTexture with next texture to upload, or set NULL if none
	void			getNextTextureToUpLoad(uint &nTotalColored, IDriver *pDriver);
	bool			uploadTexturePart(ITexture *pText, IDriver *pDriver, uint &nTotalUpload);

	// update list of texture lods.
	void			updateTextureLodSystem(IDriver *pDriver);

};


} // NL3D


#endif // NL_ASYNC_TEXTURE_MANAGER_H

/* End of async_texture_manager.h */
