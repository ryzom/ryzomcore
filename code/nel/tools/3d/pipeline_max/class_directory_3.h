/**
 * \file class_directory_3.h
 * \brief CClassDirectory3
 * \date 2012-08-18 18:01GMT
 * \author Jan Boon (Kaetemi)
 * CClassDirectory3
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

#ifndef PIPELINE_CLASS_DIRECTORY_3_H
#define PIPELINE_CLASS_DIRECTORY_3_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes
#include <nel/misc/class_id.h>
#include <nel/misc/ucstring.h>

// Project includes
#include "storage_object.h"
#include "storage_value.h"
#include "scene_class.h"

namespace PIPELINE {
namespace MAX {

class CDllDirectory;

class CClassEntry;

/**
 * \brief CClassDirectory3
 * \date 2012-08-18 18:01GMT
 * \author Jan Boon (Kaetemi)
 * This class is used for parsing the local class indices in a max file,
 * both reading and writing is supported. It is not used at runtime,
 * and should be reset when no longer necessary. Plugins have their own
 * static copy of the actual ISceneClassDesc. This class only contains
 * CClassEntry instances, which should not be used directly.
 */
class CClassDirectory3 : public CStorageContainer
{
public:
	CClassDirectory3(CDllDirectory *dllDirectory);
	virtual ~CClassDirectory3();

	// inherited
	virtual std::string className() const;
	virtual void toString(std::ostream &ostream, const std::string &pad = "") const;
	virtual void parse(uint16 version, uint filter = 0);
	virtual void clean();
	virtual void build(uint16 version, uint filter = 0);
	virtual void disown();

	// public
	// Get a class entry corresponding to a chunk index, pointers become invalid after reset
	const CClassEntry *get(uint16 index) const;
	// Reset the class directory, all class entry pointers become invalid, use class id and scene class registry
	void reset();
	// Get or create the chunk index for a class by class description
	uint16 getOrCreateIndex(const ISceneClassDesc *sceneClassDesc);

protected:
	virtual IStorageObject *createChunkById(uint16 id, bool container);

private:
	TStorageObjectContainer m_ChunkCache;
	std::vector<CClassEntry *> m_Entries;
	std::map<NLMISC::CClassId, uint16> m_ClassIdToIndex;

	CDllDirectory *m_DllDirectory;

}; /* class CClassDirectory3 */

/**
 * \brief CClassEntryHeader
 * \date 2012-08-18 18:01GMT
 * \author Jan Boon (Kaetemi)
 * CClassEntryHeader
 */
class CClassEntryHeader : public IStorageObject
{
public:
	CClassEntryHeader();
	virtual ~CClassEntryHeader();

	// public data
	sint32 DllIndex;
	NLMISC::CClassId ClassId;
	uint32 SuperClassId;

	// inherited
	virtual std::string className() const;
	virtual void serial(NLMISC::IStream &stream);
	virtual void toString(std::ostream &ostream, const std::string &pad = "") const;

}; /* class CClassEntryHeader */

/**
 * \brief CClassEntry
 * \date 2012-08-18 18:01GMT
 * \author Jan Boon (Kaetemi)
 * CClassEntry
 */
class CClassEntry : public CStorageContainer
{
public:
	CClassEntry();
	CClassEntry(CDllDirectory *dllDirectory, const ISceneClassDesc *sceneClassDesc);
	virtual ~CClassEntry();

	// inherited
	virtual std::string className() const;
	virtual void toString(std::ostream &ostream, const std::string &pad = "") const;
	virtual void parse(uint16 version, uint filter = 0);
	virtual void clean();
	virtual void build(uint16 version, uint filter = 0);
	virtual void disown();

	// read access
	const ucstring &displayName() const { return m_Name->Value; }
	sint32 dllIndex() const { return m_Header->DllIndex; }
	NLMISC::CClassId classId() const { return m_Header->ClassId; }
	uint32 superClassId() const { return m_Header->SuperClassId; }

	// debug
	void overrideClassId(NLMISC::CClassId classId) { m_Header->ClassId = classId; }

protected:
	virtual IStorageObject *createChunkById(uint16 id, bool container);
	CClassEntryHeader *m_Header;
	CStorageValue<ucstring> *m_Name;

}; /* class CClassEntry */

} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_CLASS_DIRECTORY_3_H */

/* end of file */
