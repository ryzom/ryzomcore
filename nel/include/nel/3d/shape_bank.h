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

#ifndef NL_SHAPE_BANK_H
#define NL_SHAPE_BANK_H

#include "nel/3d/shape.h"
#include "nel/misc/smart_ptr.h"
#include <map>
#include <list>

#include "nel/3d/async_file_manager_3d.h"

namespace NLMISC
{
	class IProgressCallback;
}

namespace NL3D
{

class IDriver;
class ITexture;

// ***************************************************************************
/**
 * A CShapeBank handle all the instance of the shapes and the cache management
 * system.
 * There is a default cache. If the shape is not linked explicitly to any cache
 * it is linked to the default cache. The comportement of this cache is to not
 * do any cache. When the release is called on the last reference to a shape
 * linked to this cache, the shape is removed instantly. This is the behavior
 * of all newly created cache before we call the setShapeCacheSize method.
 *
 * NB: ShapeCacheName is case-sensitive but shapeName are not (all entry are lwrcased)
 *
 * \author Matthieu Besson
 * \author Nevrax France
 * \date 2000
 */
class CShapeBank
{
public:

	/// \name State of a shape
	//@{
	/** NotPresent : Not present in the bank
	  * Present : Present in the bank and ready to be used
	  * AsyncLoad_Error : Asynchronous loading failed
	  * AsyncLoad_Shape : Asynchronous loading is currently loading the .shape file, textures and lightmaps
	  * AsyncLoad_Texture : Asynchronous loading is currently uploading textures and lightmaps to VRAM
	  */
	enum TShapeState {	NotPresent, Present,
						AsyncLoad_Error, AsyncLoad_Shape, AsyncLoad_Texture, AsyncLoad_Ready, AsyncLoad_Delete };
	//@}

	CShapeBank();
	~CShapeBank();

	/// \name Instance Management
	//@{
	/// Add a reference to a shape and return the instance created. NB: getPresentState()==Present must be tested first.
	IShape*			addRef (const std::string &shapeName);

	/**
	  * Release a reference to a shape by its instance. If the shape has no more reference it is added to
	  * its own shape cache. When the shape cache is full the last entry is deleted.
	  */
	void			release (IShape* pShp);

	/// Return the shape state. Process the waiting shapes.
	TShapeState		getPresentState (const std::string &shapeName);

	/** Return the IShape from the bank. Unlike addRef, no reference is added.
	 *	Thus the returning shape sould be used temporarily
	 *	\return NULL if shape not found or not loaded (if being async loaded still return NULL)
	 */
	IShape			*getShape (const std::string &shapeName);

	/// Load the corresponding file from disk and add it to the bank.
	void			load (const std::string &shapeName);

	/** Load the corresponding file from disk asynchronously and add it to the bank.
	 * The driver passed to this function is used to know if we have to load the textures.
	 */
	void			loadAsync (const std::string &shapeName, IDriver *pDriver, const NLMISC::CVector &position, bool *bSignal, uint selectedTexture);
	void			cancelLoadAsync (const std::string &shapeName);
	bool			isShapeWaiting ();
	/// processWaitingShapes must be done one time per frame
	void			processWaitingShapes ();
	/// Setup the maximum number of bytes to upload for a frame (texture upload from RAM to VRAM)
	void			setMaxBytesToUpload (uint32 MaxUploadPerFrame);

	/// Add directly a shape to the bank. If the shape name is already used do nothing.
	void			add (const std::string &shapeName, IShape* shape);
	//@}

	/// \name Shape cache management
	//@{
	/// Add a new ShapeCache. If already exist do nothing.
	void			addShapeCache (const std::string &shapeCacheName);

	/**
	  * Remove a ShapeCache. All shapes in the shape cache are deleted. All links are redirected to
	  * the default ShapeCache
	  */
	void			removeShapeCache (const std::string &shapeCacheName);

	/// true if the shape cache exist
	bool			isShapeCache(const std::string &shapeCacheName) const;

	/**
	  * Remove all ShapeCache and suppress all links (even the link to the default cache are removed)
	  */
	void			reset ();

	/// Set the shapeCache shapeCacheName the new size.(delete shapes if maxsize<shapeCacheSize).
	void			setShapeCacheSize (const std::string &shapeCacheName, sint32 maxSize);

	/// return free cache space (maxSize-nbCurrentInCache)
	sint			getShapeCacheFreeSpace(const std::string &shapeCacheName) const;

	/// Link a shape to a ShapeCache. The ShapeCache must exist and must not contains the shape.
	void			linkShapeToShapeCache (const std::string &shapeName, const std::string &shapeCacheName);
	//@}


	/// \name Tools
	// @{
	/** PreLoad all shapes (.shape, .ps, .skel...) files from a list of files
	 *	Shapes are Loaded if not present, assigned to the given cache, and fit in the cache Size as max possible.
	 *	NB: crash if you try to load a non shape file (eg: a .dds etc...)
	 *	\param shapeCacheName name of a shapeCache created with addShapeCache()/setShapeCacheSize(). no-op if don't exist
	 *	\param fileList a list of file names. NB: CPath is used to load the shapes.
	 *	\param wildcard a filter string like: "*.shape", "??_HOM*.shape". NB: toLower-ed internally
	 *  \param flushTextures if true, then textures are flushed in the driver drv
	 */
	void			preLoadShapes(const std::string &shapeCacheName,
		const std::vector<std::string> &listFile, const std::string &wildCardNotLwr, NLMISC::IProgressCallback *progress = NULL, bool flushTextures = false, IDriver *drv = NULL);

	/** Mark this shape name as needing a buildSystemGeometry() (typically used for selection)
	 *	NB: this will take effect only for shapes on a subsequent add(), load() or loadAsync()
	 */
	void			buildSystemGeometryForshape(const std::string &shapeName);
	// @}

	// get the shape Name from the shape Ptr. return NULL if not found
	const std::string*	getShapeNameFromShapePtr(IShape* pShp) const;

private:
	/// \name Shape/Instances.
	//@{
	typedef		NLMISC::CSmartPtr<IShape>		PShape;
	typedef		std::map<std::string, PShape>	TShapeMap;
	TShapeMap	ShapeMap;

	struct CWaitingShape
	{
		IShape *ShapePtr;	// Do not work with this value that is shared between threads
		uint32 RefCnt;		// Counter if multiple instance wants the same shape
		TShapeState State;	// State of the waiting shape (shape in loading mode)
		std::set<bool *>	Signal;		// To signal when all is done
		// Upload piece by piece part
		uint32 UpTextProgress;	// Upload Texture progress current texture or lightmap
		uint8  UpTextMipMap;	// Upload Texture progress current mipmap
		uint32 UpTextLine;		// Upload Texture progress current line in mipmap
		// ---------------------------------
		CWaitingShape ()
		{
			State = AsyncLoad_Shape;
			RefCnt = 0;
			ShapePtr = NULL;
			UpTextProgress = 0;
			UpTextMipMap = 0;
			UpTextLine = 0;
		}
	};
	typedef		std::map< std::string, CWaitingShape > TWaitingShapesMap;
	TWaitingShapesMap	WaitingShapes;
	uint32 _MaxUploadPerFrame;

private:

	/// return true if the texture is entirely uploaded
	bool processWSUploadTexture (CWaitingShape &rWS, uint32 &nTotalUploaded, ITexture *pText);

private:

	IDriver *_pDriver;
	//@}

	/// \name Shape/Caches.
	//@{
	struct CShapeCache
	{
		std::list<IShape*> Elements;
		sint32 MaxSize;
		CShapeCache() { MaxSize = 0; }
	};

	struct CShapeInfo
	{
		CShapeCache*	pShpCache;
		std::string		sShpName;
		bool			isAdded;
		CShapeInfo() { isAdded = false; pShpCache = NULL; }
	};

private:
	IShape*			getShapePtrFromShapeName(const std::string &pShpName);
	CShapeCache*	getShapeCachePtrFromShapePtr(IShape* pShp);
	CShapeCache*	getShapeCachePtrFromShapeCacheName(const std::string &shapeCacheName);
	CShapeCache*	getShapeCachePtrFromShapeName(const std::string &shapeName);
	void			checkShapeCache(CShapeCache* pShpCache);

	typedef		std::map<std::string,std::string>	TShapeCacheNameMap;
	typedef		std::map<std::string,CShapeCache>	TShapeCacheMap;
	typedef		std::map<IShape*,CShapeInfo>		TShapeInfoMap;
	TShapeCacheNameMap		ShapeNameToShapeCacheName;
	TShapeCacheMap			ShapeCacheNameToShapeCache;
	TShapeInfoMap			ShapePtrToShapeInfo;

	// Special for buildSystemGeometryForshape
	std::set<std::string>	_ShapeNeedingSystemGeometryCopy;
	//@}
};

}

#endif // NL_SHAPE_BANK_H

/* End of shape_bank.h */
