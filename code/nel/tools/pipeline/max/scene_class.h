/**
 * \file scene_class.h
 * \brief CSceneClass
 * \date 2012-08-20 09:07GMT
 * \author Jan Boon (Kaetemi)
 * CSceneClass
 */

/*
 * Copyright (C) 2012  by authors
 *
 * This file is part of RYZOM CORE PIPELINE.
 * RYZOM CORE PIPELINE is free software: you can redistribute it
 * and/or modify it under the terms of the GNU Affero General Public
 * License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * RYZOM CORE PIPELINE is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with RYZOM CORE PIPELINE.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef PIPELINE_SCENE_CLASS_H
#define PIPELINE_SCENE_CLASS_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes
#include <nel/misc/class_id.h>
#include <nel/misc/smart_ptr.h>

// Project includes
#include "typedefs.h"
#include "storage_object.h"
#include "storage_value.h"
#include "dll_plugin_desc.h"
#include "scene.h"

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {

class CSceneImpl;

}

class ISceneClassDesc;

/**
 * \brief CSceneClass
 * \date 2012-08-19 19:25GMT
 * \author Jan Boon (Kaetemi)
 * It is recommended to use CRefPtr<T> to refer to any pointers to
 * classes inherited from this class.
 * NOTE: CRefPtr<T> does not delete the class when references go to
 * zero. When you remove a class from the scene, the class will be
 * deleted if the reference count is zero. Otherwise, you are
 * responsible for deleting it (for example, if you keep the class
 * backed up in an undo stack for undeletion). You may use CSmartPtr<T>
 * when the class is no longer owned by the scene container.
 * CRefPtr<T> is a safe handle, which you can use to verify if the class
 * has been deleted or not, similar to AnimHandle in max.
 */
class CSceneClass : public CStorageContainer, public NLMISC::CVirtualRefCount
{
public:
	CSceneClass(CScene *scene);
	virtual ~CSceneClass();

	//! \name Inherited functions that are implemented by wrapping around other virtual functions in this class
	//@{
	virtual std::string className() const; // do not override, implemented using classDesc
	virtual void toString(std::ostream &ostream, const std::string &pad = "") const; // do not override, implemented using toStringLocal
	//@}

	//! \name Inherited functions called through the storage loading and saving system
	//@{
	/// Override this to read a chunk from the chunks stream. Call the parent's parse function first. Get the necessary chunks implemented by your class using getChunk. See the getChunk function for further information. In case of failure, call disown. If m_ChunksOwnsPointers is true, the parsing has failed, warnings have already been printed, and nothing should happen. Chunks are already parsed when you get them from getChunk
	virtual void parse(uint16 version, uint filter = 0);
	/// Override this if the chunks you are using are not needed after they have been parsed. You may delete any chunks you own. Call the parent's clean first. You must call clean on any chunks you own if they implement this function
	virtual void clean();
	/// Override this to write a chunk to the chunks stream. If you create a new chunk, the creating class owns it, and should delete it when clean is called. You no longer own any created chunks when disown has been called. Use putChunk to add the chunk. Call the parent's build first. The putChunk function calls build on added chunks
	virtual void build(uint16 version, uint filter = 0);
	/// Remove any references to chunks that are owned by the inheriting class (those that were acquired using getChunk during parse or created using putChunk during build). You no longer have ownership over these chunks, and should not use them anymore. This function may be called after build to disable any functionality from the loaded storage. Call the parent's disown after disowning your own chunks. Disown is called on the disowned child chunks for you
	virtual void disown();
	//@}

	//! \name Virtual functionality for inheriting classes to implement
	//@{
	/// Initialize this class from scratch, call the parent first
	virtual void init();
	//@}

	//! \name Static const variables for the class description
	//@{
	static const ucstring DisplayName;
	static const char *InternalName;
	static const NLMISC::CClassId ClassId;
	static const TSClassId SuperClassId;
	//@}

	//! \name More virtual functionality for inheriting classes to implement
	//@{
	/// Returns whether the class inherits from a class with given class id
	virtual bool inherits(const NLMISC::CClassId classId) const;
	/// Return the class description of the inheriting class
	virtual const ISceneClassDesc *classDesc() const;
	/// Create a readable representation of this class
	virtual void toStringLocal(std::ostream &ostream, const std::string &pad = "", uint filter = 0) const;
	//@}

public:
	//! \name Read access to internal data, provided for browsing trough the file from code
	//@{
	inline const TStorageObjectContainer &orphanedChunks() const { return m_OrphanedChunks; }
	//@}

	//! \name Scene utility access
	//@{
	/// Return the scene scene class
	inline BUILTIN::CSceneImpl *scene() const { return m_Scene->container()->scene(); }
	/// Return the scene version
	inline uint16 version() const { return m_Scene->version(); }
	/// Return the scene container
	inline CSceneClassContainer *container() const { return m_Scene->container(); }
	//@}

protected:
	//! \name Methods used by inheriting classes to read and write to the storage safely
	//@{
	/// Use during parsing. Gets the chunk with specified id. Warnings when chunks are skipped may be elevated to errors. Remaining orphaned chunks will be appended after chunks that are written by the classes. Returns NULL when the chunk does not exist. Empty chunks are often not written by classes. You have ownership over the chunk until it is disowned. In case that the chunk cannot be parsed, call disown and abort parsing, or you may keep the chunk as is without modifications and put it back as is during build with putChunk. If this function returns NULL it is also possible that the parsing has been aborted when m_ChunksOwnsPointers is true
	IStorageObject *getChunk(uint16 id);
	/// Use during file build. Adds a chunk to the chunks that will be written to the file. Build is called when a chunk is passed through
	void putChunk(uint16 id, IStorageObject *storageObject);
	/// Same as getChunk but for lazy programmers, must use together with putChunkValue
	template <typename T>
	const T &getChunkValue(uint16 id);
	/// Same as putChunk but for lazy programmers, must use together with getChunkValue
	template <typename T>
	void putChunkValue(uint16 id, const T &value);
	/// See the next chunk id
	uint16 peekChunk();
	//@}

protected:
	virtual IStorageObject *createChunkById(uint16 id, bool container);

	/// Chunks which have been parsed, kept unmodified, and which are no longer necessary, should be placed here
	std::vector<IStorageObject *> m_ArchivedChunks;

private:
	TStorageObjectContainer m_OrphanedChunks;
	TStorageObjectIterator m_PutChunkInsert;

	CScene *m_Scene;

}; /* class CSceneClass */

template <typename T>
const T &CSceneClass::getChunkValue(uint16 id)
{
	CStorageValue<T> *chunk = static_cast<CStorageValue<T> *>(getChunk(id));
	if (!chunk) { nlerror("Try to get required chunk value 0x%x but it does not exist, bad file format"); }
	m_ArchivedChunks.push_back(chunk);
	return chunk->Value;
}

template <typename T>
void CSceneClass::putChunkValue(uint16 id, const T &value)
{
	CStorageValue<T> *chunk = new CStorageValue<T>();
	chunk->Value = value;
	m_ArchivedChunks.push_back(chunk);
	putChunk(id, chunk);
}

/**
 * \brief ISceneClassDesc
 * \date 2012-08-19 19:25GMT
 * \author Jan Boon (Kaetemi)
 * ISceneClassDesc
 */
class ISceneClassDesc
{
public:
	virtual CSceneClass *create(CScene *scene) const = 0;
	virtual void destroy(CSceneClass *sc) const = 0;
	virtual const ucchar *displayName() const = 0;
	virtual const char *internalName() const = 0;
	virtual NLMISC::CClassId classId() const = 0;
	virtual TSClassId superClassId() const = 0;
	virtual const IDllPluginDescInternal *dllPluginDesc() const = 0;

}; /* class ISceneClassDesc */

/**
 * \brief CSceneClassDesc
 * \date 2012-08-19 19:25GMT
 * \author Jan Boon (Kaetemi)
 * CSceneClassDesc
 * Use in a cpp when registering the CClassId.
 */
template <typename T>
class CSceneClassDesc : public ISceneClassDesc
{
public:
	CSceneClassDesc(const IDllPluginDescInternal *dllPluginDesc) : m_DllPluginDesc(dllPluginDesc) { }
	virtual CSceneClass *create(CScene *scene) const { return static_cast<CSceneClass *>(new T(scene)); }
	virtual void destroy(CSceneClass *sc) const { delete static_cast<T *>(sc); }
	virtual const ucchar *displayName() const { return T::DisplayName.c_str(); }
	virtual const char *internalName() const { return T::InternalName; }
	virtual NLMISC::CClassId classId() const { return T::ClassId; }
	virtual TSClassId superClassId() const { return T::SuperClassId; }
	virtual const IDllPluginDescInternal *dllPluginDesc() const { return m_DllPluginDesc; }
private:
	const IDllPluginDescInternal *m_DllPluginDesc;

}; /* class CSceneClassDesc */

} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_SCENE_CLASS_H */

/* end of file */
