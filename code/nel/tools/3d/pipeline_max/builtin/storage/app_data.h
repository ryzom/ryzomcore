/**
 * \file app_data.h
 * \brief CAppData
 * \date 2012-08-21 11:47GMT
 * \author Jan Boon (Kaetemi)
 * CAppData
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

#ifndef PIPELINE_APP_DATA_H
#define PIPELINE_APP_DATA_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes
#include <nel/misc/class_id.h>
#include <nel/misc/mem_stream.h>

// Project includes
#include "../../typedefs.h"
#include "../../storage_object.h"
#include "../../storage_value.h"

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {
namespace STORAGE {

#define PMBS_APP_DATA_CHUNK_ID 0x2150
#define PMBS_APP_DATA_ENTRY_VALUE_CHUNK_ID 0x0130

class CAppDataEntry;

/**
 * \brief CAppData
 * \date 2012-08-21 11:47GMT
 * \author Jan Boon (Kaetemi)
 * This implements the AppData chunk in the storage
 */
class CAppData : public CStorageContainer
{
public:
	struct TKey
	{
		TKey(NLMISC::CClassId classId, TSClassId superClassId, uint32 subId);
		NLMISC::CClassId ClassId;
		TSClassId SuperClassId;
		uint32 SubId;
		bool operator<(const TKey &right) const;
		bool operator>(const TKey &right) const;
		bool operator==(const TKey &right) const;
	};
	typedef std::map<TKey, CAppDataEntry *> TMap;

public:
	CAppData();
	virtual ~CAppData();

	// inherited
	virtual std::string className() const;
	virtual void toString(std::ostream &ostream, const std::string &pad = "") const;
	virtual void parse(uint16 version, uint filter = 0);
	virtual void clean();
	virtual void build(uint16 version, uint filter = 0);
	virtual void disown();

	// init
	/// Initialize a new instance of this chunk
	void init();

	// public // TODO: Simplify using templates and returning a specialized storage object, auto-converted to the requested type.
	/*/// Gets a pointer to an appdata chunk buffer. Returns NULL if it does not exist. Size is returned in the size parameter.
	const uint8 *read(NLMISC::CClassId classId, TSClassId superClassId, uint32 subId, uint32 &size) const;
	/// Locks a pointer to an appdata chunk buffer for writing to with specified capacity. May return NULL if this chunk is unparsable or no memory can be allocated.
	uint8 *lock(NLMISC::CClassId classId, TSClassId superClassId, uint32 subId, uint32 capacity);
	/// Unlocks a pointer to an appdata chunk buffer, setting the final written size.
	void unlock(NLMISC::CClassId classId, TSClassId superClassId, uint32 subId, uint32 size);
	/// Fills an appdata chunk buffer with specified data, which will be copied.
	void fill(NLMISC::CClassId classId, TSClassId superClassId, uint32 subId, uint8 *buffer, uint32 size);
	/// Erases an appdata chunk.
	void erase(NLMISC::CClassId classId, TSClassId superClassId, uint32 subId);*/

	// public
	/// Gets an appdata chunk storage object, returns NULL if it does not exist
	template <typename T>
	T *get(NLMISC::CClassId classId, TSClassId superClassId, uint32 subId);
	/// Gets or creates a chunk storage object
	template <typename T>
	T *getOrCreate(NLMISC::CClassId classId, TSClassId superClassId, uint32 subId);
	/// Erases an appdata chunk.
	void erase(NLMISC::CClassId classId, TSClassId superClassId, uint32 subId);

	// read access
	/// Return the entries map, do not modify directly
	inline const TMap &entries() const { return m_Entries; }

protected:
	virtual IStorageObject *createChunkById(uint16 id, bool container);

private:
	TMap m_Entries;

}; /* class CAppData */

/**
 * \brief CAppDataEntryKey
 * \date 2012-08-18 18:01GMT
 * \author Jan Boon (Kaetemi)
 * CAppDataEntryKey
 */
class CAppDataEntryKey : public IStorageObject
{
public:
	CAppDataEntryKey();
	virtual ~CAppDataEntryKey();

	// public data
	NLMISC::CClassId ClassId;
	TSClassId SuperClassId;
	uint32 SubId;
	uint32 Size;

	// inherited
	virtual std::string className() const;
	virtual void serial(NLMISC::IStream &stream);
	virtual void toString(std::ostream &ostream, const std::string &pad = "") const;

}; /* class CAppDataEntryKey */

/**
 * \brief CAppDataEntry
 * \date 2012-08-21 11:47GMT
 * \author Jan Boon (Kaetemi)
 * This implements an entry in the AppData chunk in the storage
 */
class CAppDataEntry : public CStorageContainer
{
public:
	friend class CAppData;

public:
	CAppDataEntry();
	virtual ~CAppDataEntry();

	// inherited
	virtual std::string className() const;
	virtual void toString(std::ostream &ostream, const std::string &pad = "") const;
	virtual void parse(uint16 version, uint filter = 0);
	virtual void clean();
	virtual void build(uint16 version, uint filter = 0);
	virtual void disown();

	// public
	// Initializes a new entry
	void init();
	// Returns the key
	inline CAppDataEntryKey *key() { return m_Key; }
	// Returns the value
	template <typename T>
	T *value();

protected:
	virtual IStorageObject *createChunkById(uint16 id, bool container);

private:
	CAppDataEntryKey *m_Key;
	CStorageRaw *m_Raw;
	IStorageObject *m_Value;

}; /* class CAppDataEntry */

template <typename T>
T *CAppDataEntry::value()
{
	if (m_Value)
	{
		T *result = dynamic_cast<T *>(m_Value);
		if (result)
		{
			return result;
		}
		else
		{
			nlwarning("AppData value has already been cast to another type, recasting, the previous chunk becomes invalid, this may not be intended");
			NLMISC::CMemStream mem;
			m_Value->serial(mem);
			result = new T();
			result->setSize(mem.getPos());
			mem.invert();
			delete m_Value;
			m_Value = result;
			result->serial(mem);
			return result;
		}
	}
	else
	{
		nlassert(!m_Value);
		nlassert(m_Raw);
		// nldebug("Casting raw to user type");
		NLMISC::CMemStream mem;
		m_Raw->serial(mem);
		T *result = new T();
		result->setSize(mem.getPos());
		// nldebug("Read %i", mem.getPos());
		mem.invert();
		m_Raw->Value.resize(0);
		m_Key->Size = 0xFFFFFFFF;
		m_Value = result;
		result->serial(mem);
		// nldebug("Wrote %i", mem.getPos());
		return result;
	}
}

template <typename T>
T *CAppData::get(NLMISC::CClassId classId, TSClassId superClassId, uint32 subId)
{
	if (m_ChunksOwnsPointers) { nlwarning("Not parsed"); return NULL; }
	TKey key(classId, superClassId, subId);
	TMap::const_iterator it = m_Entries.find(key);
	if (it == m_Entries.end()) { nldebug("Trying to read non-existant key, this is allowed, returning NULL"); return NULL; }
	return it->second->value<T>();
}

template <typename T>
T *CAppData::getOrCreate(NLMISC::CClassId classId, TSClassId superClassId, uint32 subId)
{
	if (m_ChunksOwnsPointers) { nlwarning("Not parsed"); return NULL; }
	TKey key(classId, superClassId, subId);
	TMap::const_iterator it = m_Entries.find(key);
	CAppDataEntry *appDataEntry;
	if (it == m_Entries.end())
	{
		appDataEntry = new CAppDataEntry();
		appDataEntry->init();
		m_Entries[key] = appDataEntry;
		appDataEntry->key()->ClassId = classId;
		appDataEntry->key()->SuperClassId = superClassId;
		appDataEntry->key()->SubId = subId;
	}
	else
	{
		appDataEntry = it->second;
	}
	return appDataEntry->value<T>();
}

} /* namespace STORAGE */
} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_APP_DATA_H */

/* end of file */
