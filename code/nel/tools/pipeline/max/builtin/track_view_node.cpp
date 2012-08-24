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
#define PMB_TVNODE_INTEGER0130_CHUNK_ID 0x0130 /* type? */

CTrackViewNode::CTrackViewNode(CScene *scene) : CReferenceTarget(scene)
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

void CTrackViewNode::parse(uint16 version)
{
	CReferenceTarget::parse(version);
}

void CTrackViewNode::clean()
{
	CReferenceTarget::clean();
}

void CTrackViewNode::build(uint16 version)
{
	CReferenceTarget::build(version);
}

void CTrackViewNode::disown()
{
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

void CTrackViewNode::toStringLocal(std::ostream &ostream, const std::string &pad) const
{
	CReferenceTarget::toStringLocal(ostream, pad);
}
/*
CReferenceMaker *CTrackViewNode::getReference(uint index) const
{
	if (m_References.size() <= index) return NULL;
	return m_References[index];
}

void CTrackViewNode::setReference(uint index, CReferenceMaker *reference)
{
	if (m_References.size() <= index) m_References.resize(index + 1);
	m_References[index] = reference;
}

uint CTrackViewNode::nbReferences() const
{
	return m_References.size();
}
*/
IStorageObject *CTrackViewNode::createChunkById(uint16 id, bool container)
{
	switch (id)
	{
	case PMB_TVNODE_DISPLAYNAME_CHUNK_ID:
		return new CStorageValue<ucstring>();
	case PMB_TVNODE_IDENTIFIER_CHUNK_ID:
		return new CStorageValue<NLMISC::CClassId>();
	case PMB_TVNODE_INTEGER0130_CHUNK_ID:
		return new CStorageValue<sint32>();
	}
	return CReferenceTarget::createChunkById(id, container);
}

} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
