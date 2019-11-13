/**
 * \file track_view_node.cpp
 * \brief CTrackViewNode
 * \date 2012-08-24 09:44GMT
 * \author Jan Boon (Kaetemi)
 * CTrackViewNode
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
#include "track_view_node.h"

// STL includes
#include <iomanip>

// NeL includes
// #include <nel/misc/debug.h>

// Project includes

using namespace std;
// using namespace NLMISC;

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {

#define PMB_TVNODE_EMPTY0140_CHUNK_ID 0x0140
#define PMB_TVNODE_EMPTY0150_CHUNK_ID 0x0150

#define PMB_TVNODE_DISPLAYNAME_CHUNK_ID 0x0110
#define PMB_TVNODE_IDENTIFIER_CHUNK_ID 0x0120
#define PMB_TVNODE_ISNOTNODE_CHUNK_ID 0x0130

CTrackViewNode::CTrackViewNode(CScene *scene) : CReferenceTarget(scene), m_Empty0140(NULL), m_Empty0150(NULL)
{

}

CTrackViewNode::~CTrackViewNode()
{

}

const ucstring CTrackViewNode::DisplayName = ucstring("TVNode");
const char *CTrackViewNode::InternalName = "TrackViewNode";
const NLMISC::CClassId CTrackViewNode::ClassId = NLMISC::CClassId(0x8d73b8aa, 0x90f2ee71);
const TSClassId CTrackViewNode::SuperClassId = CReferenceTarget::SuperClassId;
const CTrackViewNodeClassDesc TrackViewNodeClassDesc(&DllPluginDescBuiltin);

void CTrackViewNode::parse(uint16 version, uint filter)
{
	CReferenceTarget::parse(version);
	if (!m_ChunksOwnsPointers)
	{
		// Read unknown chunks
		m_Empty0140 = static_cast<CStorageRaw *>(getChunk(PMB_TVNODE_EMPTY0140_CHUNK_ID));
		if (m_Empty0140) nlassert(m_Empty0140->Value.empty());
		m_Empty0150 = static_cast<CStorageRaw *>(getChunk(PMB_TVNODE_EMPTY0150_CHUNK_ID));
		if (m_Empty0150) nlassert(m_Empty0140->Value.empty());

		// Read child nodes
		for (std::vector<TChild>::size_type i = 0; i < m_Children.size(); ++i)
		{
			m_Children[i].DisplayName = getChunkValue<ucstring>(PMB_TVNODE_DISPLAYNAME_CHUNK_ID);
			m_Children[i].Identifier = getChunkValue<NLMISC::CClassId>(PMB_TVNODE_IDENTIFIER_CHUNK_ID);
			m_Children[i].IsNotAnotherNode = getChunkValue<sint32>(PMB_TVNODE_ISNOTNODE_CHUNK_ID);
		}
	}
}

void CTrackViewNode::clean()
{
	CReferenceTarget::clean();
}

void CTrackViewNode::build(uint16 version, uint filter)
{
	CReferenceTarget::build(version);

	// Write unknown chunks
	if (m_Empty0140) putChunk(PMB_TVNODE_EMPTY0140_CHUNK_ID, m_Empty0140);
	if (m_Empty0150) putChunk(PMB_TVNODE_EMPTY0150_CHUNK_ID, m_Empty0150);

	// Write child nodes
	for (std::vector<TChild>::size_type i = 0; i < m_Children.size(); ++i)
	{
		putChunkValue(PMB_TVNODE_DISPLAYNAME_CHUNK_ID, m_Children[i].DisplayName);
		putChunkValue(PMB_TVNODE_IDENTIFIER_CHUNK_ID, m_Children[i].Identifier);
		putChunkValue(PMB_TVNODE_ISNOTNODE_CHUNK_ID, m_Children[i].IsNotAnotherNode);
	}
}

void CTrackViewNode::disown()
{
	m_Children.clear();
	CReferenceTarget::disown();
}

void CTrackViewNode::init()
{
	CReferenceTarget::init();
}

bool CTrackViewNode::inherits(const NLMISC::CClassId classId) const
{
	if (classId == classDesc()->classId()) return true;
	return CReferenceTarget::inherits(classId);
}

const ISceneClassDesc *CTrackViewNode::classDesc() const
{
	return &TrackViewNodeClassDesc;
}

void CTrackViewNode::toStringLocal(std::ostream &ostream, const std::string &pad, uint filter) const
{
	CReferenceTarget::toStringLocal(ostream, pad);
	if (m_Empty0140) ostream << "\n" << pad << "Empty 0x0140 ";
	if (m_Empty0150) ostream << "\n" << pad << "Empty 0x0150 ";
	// std::string padpad = pad + "\t";
	for (std::vector<TChild>::size_type i = 0; i < m_Children.size(); ++i)
	{
		CReferenceMaker *referenceMaker = m_Children[i].Reference;
		nlassert(referenceMaker);
		ostream << "\n" << pad << i << ": <ptr=0x";
		{
			std::stringstream ss;
			ss << std::hex << std::setfill('0');
			ss << std::setw(16) << (uint64)(void *)referenceMaker;
			ostream << ss.str();
		}
		ostream << "> ";
		ostream << "(" << ucstring(referenceMaker->classDesc()->displayName()).toUtf8() << ", " << referenceMaker->classDesc()->classId().toString() << ") ";
		ostream << "(" << m_Children[i].DisplayName.toUtf8() << ", " << m_Children[i].Identifier.toString() << ", " << (m_Children[i].IsNotAnotherNode ? "ENTRY" : "TVNODE") << ") ";
	}
}

CReferenceMaker *CTrackViewNode::getReference(uint index) const
{
	if (m_Children.size() <= index) return NULL;
	return m_Children[index].Reference;
}

void CTrackViewNode::setReference(uint index, CReferenceMaker *reference)
{
	if (m_Children.size() <= index) m_Children.resize(index + 1);
	m_Children[index].Reference = reference;
}

uint CTrackViewNode::nbReferences() const
{
	return m_Children.size();
}

IStorageObject *CTrackViewNode::createChunkById(uint16 id, bool container)
{
	switch (id)
	{
	case PMB_TVNODE_DISPLAYNAME_CHUNK_ID:
		return new CStorageValue<ucstring>();
	case PMB_TVNODE_IDENTIFIER_CHUNK_ID:
		return new CStorageValue<NLMISC::CClassId>();
	case PMB_TVNODE_ISNOTNODE_CHUNK_ID:
		return new CStorageValue<sint32>();
	}
	return CReferenceTarget::createChunkById(id, container);
}

} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
