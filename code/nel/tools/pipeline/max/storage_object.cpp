/**
 * \file storage_object.cpp
 * \brief CStorageObject
 * \date 2012-08-18 09:02GMT
 * \author Jan Boon (Kaetemi)
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

#include <nel/misc/types_nl.h>
#include "storage_object.h"

// STL includes
#include <iomanip>

// NeL includes
// #include <nel/misc/debug.h>

// Project includes
#include "storage_stream.h"
#include "storage_chunks.h"

// using namespace std;
// using namespace NLMISC;

// #define NL_DEBUG_STORAGE

namespace PIPELINE {
namespace MAX {

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

IStorageObject::IStorageObject()
{

}

IStorageObject::~IStorageObject()
{

}

std::string IStorageObject::toString()
{
	std::stringstream ss;
	toString(ss);
	return ss.str();
}

void IStorageObject::setSize(sint32 size)
{
	// ignore
}

bool IStorageObject::getSize(sint32 &size) const
{
	return false;
}

bool IStorageObject::isContainer() const
{
	return false;
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

CStorageContainer::CStorageContainer() : ChunksOwnsPointers(true)
{

}

CStorageContainer::~CStorageContainer()
{
	if (ChunksOwnsPointers)
	{
		for (TStorageObjectContainer::iterator it = Chunks.begin(), end = Chunks.end(); it != end; ++it)
		{
			delete it->second;
		}
	}
	Chunks.clear();
}

std::string CStorageContainer::getClassName() // why is this not const in IClassable?
{
	return "StorageContainer";
}

void CStorageContainer::serial(NLMISC::IStream &stream)
{
	if (stream.isReading())
	{
		nlassert(ChunksOwnsPointers);
		nlassert(Chunks.empty());
	}
	if (stream.getPos() == 0)
	{
		CStorageStream *storageStream = dynamic_cast<CStorageStream *>(&stream);
		if (storageStream)
		{
#ifdef NL_DEBUG_STORAGE
			nldebug("Implicitly assume the entire stream is the container");
#endif
			CStorageChunks chunks(stream, stream.isReading() ? storageStream->size() : 0);
			serial(chunks);
			return;
		}
	}
#ifdef NL_DEBUG_STORAGE
	nldebug("Wrapping the container inside a stream with necessary size markers");
#endif
	{
		const uint32 magic = 0xC0C01473;
		stream.serialCheck(magic);
		uint version = stream.serialVersion(1);
		sint32 sizePos;
		bool reading = stream.isReading();
		if (!reading)
		{
			sizePos = stream.getPos();
		}
		sint64 size = 0;
		stream.serial(size);
		CStorageChunks chunks(stream, size);
		serial(chunks);
		if (!reading)
		{
			sint32 returnPos = stream.getPos();
#ifdef NL_DEBUG_STORAGE
			nldebug("current (return) pos is %i", stream.getPos());
#endif
			size = returnPos - sizePos - 8;
			stream.seek(sizePos, NLMISC::IStream::begin);
#ifdef NL_DEBUG_STORAGE
			nldebug("current (size) pos is %i", stream.getPos());
#endif
			stream.serial(size);
			stream.seek(returnPos, NLMISC::IStream::begin);
#ifdef NL_DEBUG_STORAGE
			nldebug("sizePos is %i", sizePos);
			nldebug("returnPos is %i", returnPos);
			nldebug("current (return) pos is %i", stream.getPos());
#endif
		}
#ifdef NL_DEBUG_STORAGE
		nldebug("Chunk container wrapper size is %i", size);
#endif
		return;
	}
}

void CStorageContainer::toString(std::ostream &ostream, const std::string &pad)
{
	// note: only use pad when multi-lining
	// like Blah: (Something) "SingleValue"
	//      Blahblah: (Container) {
	//          Moo: (Foo) "What" }
	// only increase pad when multi-lining sub-items
	nlassert(ChunksOwnsPointers);
	ostream << "(" << getClassName() << ") [" << Chunks.size() << "] { ";
	std::string padpad = pad + "\t";
	sint i = 0;
	for (TStorageObjectContainer::const_iterator it = Chunks.begin(), end = Chunks.end(); it != end; ++it)
	{
		std::stringstream ss;
		ss << std::hex << std::setfill('0');
		ss << std::setw(4) << it->first;
		ostream << "\n" << pad << i << " 0x" << ss.str() << ": ";
		it->second->toString(ostream, padpad);
		++i;
	}
	ostream << "} ";
}

void CStorageContainer::parse(uint16 version, TParseLevel level)
{
	for (TStorageObjectContainer::const_iterator it = Chunks.begin(), end = Chunks.end(); it != end; ++it)
	{
		if (it->second->isContainer())
		{
			static_cast<CStorageContainer *>(it->second)->parse(version, level);
		}
	}
}

void CStorageContainer::clean()
{
	nlassert(ChunksOwnsPointers); // Can only use the default when Chunks retains ownership.
	for (TStorageObjectContainer::const_iterator it = Chunks.begin(), end = Chunks.end(); it != end; ++it)
	{
		if (it->second->isContainer())
		{
			static_cast<CStorageContainer *>(it->second)->clean();
		}
	}
}

void CStorageContainer::build(uint16 version)
{
	for (TStorageObjectContainer::const_iterator it = Chunks.begin(), end = Chunks.end(); it != end; ++it)
	{
		if (it->second->isContainer())
		{
			static_cast<CStorageContainer *>(it->second)->build(version);
		}
	}
}

void CStorageContainer::disown()
{
	for (TStorageObjectContainer::const_iterator it = Chunks.begin(), end = Chunks.end(); it != end; ++it)
	{
		if (it->second->isContainer())
		{
			static_cast<CStorageContainer *>(it->second)->disown();
		}
	}
}

bool CStorageContainer::isContainer() const
{
	return true;
}

void CStorageContainer::serial(CStorageChunks &chunks)
{
	if (chunks.stream().isReading())
	{
#ifdef NL_DEBUG_STORAGE
		nldebug("Reading container chunk");
#endif
		nlassert(ChunksOwnsPointers);
		nlassert(Chunks.empty());
		while (chunks.enterChunk())
		{
			uint16 id = chunks.getChunkId();
			bool cont = chunks.isChunkContainer();
			IStorageObject *storageObject = createChunkById(id, cont);
			storageObject->setSize(chunks.getChunkSize());
			if (storageObject->isContainer()) static_cast<CStorageContainer *>(storageObject)->serial(chunks);
			else storageObject->serial(chunks.stream());
			Chunks.push_back(TStorageObjectWithId(id, storageObject));
			if (chunks.leaveChunk()) // bytes were skipped while reading
				throw EStorage();
			/*TStorageObjectContainer::iterator soit = Chunks.end();
			--soit;
			serialized(soit, cont);*/
		}
	}
	else
	{
#ifdef NL_DEBUG_STORAGE
		nldebug("Writing container chunk");
#endif
		for (TStorageObjectContainer::iterator it = Chunks.begin(), end = Chunks.end(); it != end; ++it)
		{
			chunks.enterChunk(it->first, it->second->isContainer());
			IStorageObject *storageObject = it->second;
			if (storageObject->isContainer()) static_cast<CStorageContainer *>(storageObject)->serial(chunks);
			else storageObject->serial(chunks.stream());
			chunks.leaveChunk();
		}
	}
}

IStorageObject *CStorageContainer::createChunkById(uint16 id, bool container)
{
	if (container)
	{
		return new CStorageContainer();
	}
	else
	{
		return new CStorageRaw();
	}
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

CStorageRaw::CStorageRaw()
{

}

CStorageRaw::~CStorageRaw()
{

}

std::string CStorageRaw::getClassName()
{
	return "StorageRaw";
}

void CStorageRaw::serial(NLMISC::IStream &stream)
{
	stream.serialBuffer(&Value[0], Value.size());
}

void CStorageRaw::toString(std::ostream &ostream, const std::string &pad)
{
	// note: only use pad when multi-lining
	// like Blah: (Something) "SingleValue"
	//      Blahblah: (Container) {
	//          Moo: (Foo) "What" }
	// only increase pad when multi-lining sub-items
	ostream << "(" << getClassName() << ") { ";
	ostream << "\n" << pad << "Size: " << Value.size();
	bool isString = true;
	ostream << "\n" << pad << "String: ";
	for (TType::size_type i = 0; i < Value.size(); ++i)
	{
		char c = Value[i];
		if (c == 0 || c == 123 || c == 125) ostream << ".";
		else if (c >= 32 && c <= 126) ostream << c;
		else
		{
			ostream << ".";
			isString = false;
		}
	}
	ostream << " ";
	if (!isString || Value.size() <= 4)
	{
		ostream << "\n" << pad << "Hex: ";
		for (TType::size_type i = 0; i < Value.size(); ++i)
		{
			std::stringstream ss;
			ss << std::hex << std::setfill('0') << std::setw(2) << (int)Value[i];
			ostream << ss.str() << " ";
		}
	}
	ostream << "} ";
}

void CStorageRaw::setSize(sint32 size)
{
	Value.resize(size);
}

bool CStorageRaw::getSize(sint32 &size) const
{
	size = Value.size();
	return true;
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
