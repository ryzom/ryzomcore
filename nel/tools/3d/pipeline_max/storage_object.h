/**
 * \file storage_object.h
 * \brief CStorageObject
 * \date 2012-08-18 09:02GMT
 * \author Jan Boon (Kaetemi)
 * \author Claude Opus 4.7
 * \author Claude Sonnet 5
 * CStorageObject
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

#ifndef PIPELINE_STORAGE_OBJECT_H
#define PIPELINE_STORAGE_OBJECT_H
#include <nel/misc/types_nl.h>

// STL includes
#include <sstream>
#include <vector>

// NeL includes
#include <nel/misc/stream.h>

// Project includes

namespace PIPELINE {
namespace MAX {

class CStorageChunks;

// Portable replacement for std::vector<T>::data() — absent in MSVC 9.0 (VS2008, used for the
// Max-2010-matching x87 reference build). Null-safe like data(), so an empty vector yields NULL
// rather than the undefined &v[0]. Kept as a plain helper (not the C++11 member) so the whole
// library compiles under both the modern x64 toolchain and VS2008.
template <typename T> inline T *nlVectorData(std::vector<T> &v) { return v.empty() ? (T *)0 : &v[0]; }
template <typename T> inline const T *nlVectorData(const std::vector<T> &v) { return v.empty() ? (const T *)0 : &v[0]; }

struct EStorage : public NLMISC::Exception
{
	EStorage() : NLMISC::Exception("PIPELINE::MAX::EStorage") { }
	EStorage(const char *msg) : NLMISC::Exception(msg) { }
	virtual ~EStorage() throw() NL_OVERRIDE { }
};

struct EStorageParse : public EStorage
{
	EStorageParse() : EStorage("PIPELINE::MAX::EStorageParse") { }
	EStorageParse(const char *msg) : EStorage(msg) { }
	virtual ~EStorageParse() throw() NL_OVERRIDE { }
};

enum TParseLevel
{
	PARSE_INTERNAL = 0x00000001, // Directly parse basic class formats
	PARSE_BUILTIN = 0x00000002, // Parse all builtin classes
	// PARSE_NELDATA = 0x00000004, // Parse all structures related to nel specific data (nel material, node properties, etcetera)
	// PARSE_NEL3D = 0x00000008, // Parse classes to initialize their nel3d equivalent classes
};

// IStorageObject : exposes serial(CStorageStream &stream) and dump(const std::string &pad)
class IStorageObject : public NLMISC::IStreamable
{
public:
	IStorageObject();
	virtual ~IStorageObject() NL_OVERRIDE;

	virtual std::string getClassName() NL_OVERRIDE { return className(); } // inherited from NLMISC::IClassable through NLMISC::IStreamable
	virtual std::string className() const = 0; // renamed for constness
	// virtual void serial(NLMISC::IStream &stream); // inherited from NLMISC::IStreamable
	std::string toString();
	virtual void toString(std::ostream &ostream, const std::string &pad = "") const = 0;

public: // should be protected but that doesn't compile, nice c++!
	// Sets size when reading
	virtual void setSize(sint32 size);
	// Gets the size when writing, return false if unknown
	virtual bool getSize(sint32 &size) const;
	// Only true when inherting from CStorageContainer
	virtual bool isContainer() const;
	// Which chunk-header container bit to write for this object. Defaults to isContainer(), but
	// e.g. CSceneClass overrides this to reproduce a chunk that was read with the container bit
	// unset even though the object type is always a CStorageContainer (see CSceneClass::m_ReadAsLeaf).
	virtual bool writeAsContainer() const { return isContainer(); }

	// Whether THIS specific chunk was read with a 64-bit header (6- vs 14-byte chunk header; see
	// CStorageChunks). Set by CStorageContainer::serial(CStorageChunks&) right after the chunk is
	// entered, consulted on write so a stream with a genuine mix of 32-bit and 64-bit chunks
	// round-trips each chunk at its original width instead of the writer upgrading everything to
	// whatever width the outermost chunk happened to use (see pipeline_max_design.md defect list).
	// Tri-state (known/unknown, not just true/false): typed classes commonly rebuild their own
	// sub-chunks as brand new objects during build() (putChunk/putChunkValue), which never went
	// through a read, so they have no original width of their own — wasWas64BitChunkKnown() is
	// false for those, and the container's own build() falls back to its aggregate m_Was64Bit
	// default for them instead of silently treating "never read" as "was 32-bit".
	inline void setWas64BitChunk(bool was64Bit) { m_Was64BitChunk = was64Bit; m_Was64BitChunkKnown = true; }
	inline bool wasRead64BitChunk() const { return m_Was64BitChunk; }
	inline bool wasRead64BitChunkKnown() const { return m_Was64BitChunkKnown; }

private:
	bool m_Was64BitChunk;
	bool m_Was64BitChunkKnown;
};

// CStorageContainer : serializes a container chunk
class CStorageContainer : public IStorageObject
{
public:
	// public data
	typedef std::pair<uint16, IStorageObject *> TStorageObjectWithId;
	typedef std::list<TStorageObjectWithId> TStorageObjectContainer;
	typedef TStorageObjectContainer::iterator TStorageObjectIterator;
	typedef TStorageObjectContainer::const_iterator TStorageObjectConstIt;

protected:
	// protected data
	TStorageObjectContainer m_Chunks;
	bool m_ChunksOwnsPointers;
	// True if the source stream used 64-bit chunk headers (any chunk with size32==0 marker).
	// Set on read; consumed on write to preserve per-stream byte identity. Nested containers
	// share the top-level's CStorageChunks so this flag is only load-bearing on the top-level
	// container passed to serial(stream, size).
	bool m_Was64Bit;

public:
	CStorageContainer();
	virtual ~CStorageContainer() NL_OVERRIDE;

	// inherited
	virtual std::string className() const NL_OVERRIDE;
	virtual void serial(NLMISC::IStream &stream) NL_OVERRIDE; // only used to wrap a container inside another stream
	virtual void toString(std::ostream &ostream, const std::string &pad = "") const NL_OVERRIDE;

	// utility
	void serial(NLMISC::IStream &stream, uint size); // without wrapping, known size

	// virtual
	// Parse this class with given version and parse level filter
	virtual void parse(uint16 version, uint filter = 0);
	// Clean up built data or duplicate unparsed source data, call after serializing build and after parse
	virtual void clean();
	// Build the storage structure needed to store the parsed data back
	virtual void build(uint16 version, uint filter = 0);
	// Give ownership of the chunks back to the m_Chunks, must call build first, call instead of clean, reduces the parse level back to 0
	virtual void disown();

public:
	// read access
	inline const TStorageObjectContainer &chunks() const { return m_Chunks; }
	IStorageObject *findStorageObject(uint16 id, uint nb = 0) const; // find storage object with given id, nb count in case there are more
	IStorageObject *findLastStorageObject(uint16 id) const;

public: // should be protected but that doesn't compile, nice c++!
	// inherited
	virtual bool isContainer() const NL_OVERRIDE;

protected:
	// override in subclasses, default to parent if not handled
	virtual void serial(CStorageChunks &chunks);
	// Create a storage object by id, override to provide custom serialization
	virtual IStorageObject *createChunkById(uint16 id, bool container);

};

// CStorageRaw : serializes raw data, use for unknown data
class CStorageRaw : public IStorageObject
{
public:
	// public data
	typedef std::vector<uint8> TType;
	TType Value;

public:
	CStorageRaw();
	virtual ~CStorageRaw() NL_OVERRIDE;

	// inherited
	virtual std::string className() const NL_OVERRIDE;
	virtual void serial(NLMISC::IStream &stream) NL_OVERRIDE;
	virtual void toString(std::ostream &ostream, const std::string &pad = "") const NL_OVERRIDE;

public: // should be protected but that doesn't compile, nice c++!
	// Sets size when reading
	virtual void setSize(sint32 size) NL_OVERRIDE;
	// Gets the size when writing, return false if unknown
	virtual bool getSize(sint32 &size) const NL_OVERRIDE;

};

} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_STORAGE_OBJECT_H */

/* end of file */
