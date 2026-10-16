/**
 * \file node_impl.cpp
 * \brief CNodeImpl
 * \date 2012-08-22 20:01GMT
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 * CNodeImpl
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
#include "node_impl.h"

// STL includes
#include <cstring>
#include <iomanip>

// NeL includes
// #include <nel/misc/debug.h>

// Project includes

using namespace std;
// using namespace NLMISC;

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {

#define PMB_NODE_VERSION_CHUNK_ID 0x09ce
#define PMB_NODE_PARENT_CHUNK_ID 0x0960
#define PMB_NODE_NAME_CHUNK_ID 0x0962

// Optional node state chunks, decoded as a typed overlay over the orphans (never claimed —
// they re-emit verbatim in position among the other orphans; see the header note).
#define PMB_NODE_FLAGS_CHUNK_ID 0x0963
#define PMB_NODE_RENDERFLAGS_CHUNK_ID 0x099c
#define PMB_NODE_OFFSET_POS_CHUNK_ID 0x096a
#define PMB_NODE_OFFSET_ROT_CHUNK_ID 0x096b
#define PMB_NODE_OFFSET_SCALE_CHUNK_ID 0x096c

CNodeImpl::CNodeImpl(CScene *scene) : INode(scene), m_NodeVersion(0), m_ParentFlags(0), m_UserName(ucstring("Untitled Node")),
	m_HasNodeFlags(false), m_NodeFlags(0), m_HasRenderFlags(false), m_RenderFlags(0),
	m_HasObjOffsetPos(false), m_HasObjOffsetRot(false), m_HasObjOffsetScaleVec(false), m_HasObjOffsetScale(false)
{
	m_ObjOffsetPos[0] = m_ObjOffsetPos[1] = m_ObjOffsetPos[2] = 0.0f;
	m_ObjOffsetRot[0] = m_ObjOffsetRot[1] = m_ObjOffsetRot[2] = 0.0f; m_ObjOffsetRot[3] = 1.0f;
	m_ObjOffsetScaleS[0] = m_ObjOffsetScaleS[1] = m_ObjOffsetScaleS[2] = 1.0f;
	m_ObjOffsetScaleQ[0] = m_ObjOffsetScaleQ[1] = m_ObjOffsetScaleQ[2] = 0.0f; m_ObjOffsetScaleQ[3] = 1.0f;
}

CNodeImpl::~CNodeImpl()
{

}

const ucstring CNodeImpl::DisplayName = ucstring("Node");
const char *CNodeImpl::InternalName = "NodeImpl";
const NLMISC::CClassId CNodeImpl::ClassId = NLMISC::CClassId(0x00000001, 0x00000000);
const TSClassId CNodeImpl::SuperClassId = 0x00000001; // Node; literal to avoid cross-TU static-init-order dependency
const CNodeImplClassDesc NodeImplClassDesc(&DllPluginDescBuiltin);

void CNodeImpl::parse(uint16 version, uint filter)
{
	INode::parse(version);
	if (!m_ChunksOwnsPointers)
	{
		m_NodeVersion = getChunkValue<uint32>(PMB_NODE_VERSION_CHUNK_ID);

		CStorageArray<uint32> *parent = static_cast<CStorageArray<uint32> *>(getChunk(PMB_NODE_PARENT_CHUNK_ID));
		nlassert(parent);
		nlassert(parent->Value.size() == 2);
		setParent(dynamic_cast<INode *>(container()->getByStorageIndex((sint32)parent->Value[0])));
		nlassert(m_Parent);
		m_ParentFlags = parent->Value[1];
		m_ArchivedChunks.push_back(parent);

		m_UserName = getChunkValue<ucstring>(PMB_NODE_NAME_CHUNK_ID);

		decodeStateChunks();
	}
}

// Decode the optional node state chunks from the orphans WITHOUT moving them (raw stays
// authoritative; first match wins, matching the historical findRawChunk readers).
void CNodeImpl::decodeStateChunks()
{
	const TStorageObjectContainer &orphans = orphanedChunks();
	for (TStorageObjectConstIt it = orphans.begin(); it != orphans.end(); ++it)
	{
		CStorageRaw *raw = dynamic_cast<CStorageRaw *>(it->second);
		if (!raw) continue;
		switch (it->first)
		{
		case PMB_NODE_FLAGS_CHUNK_ID:
			if (!m_HasNodeFlags && raw->Value.size() >= 4)
			{
				memcpy(&m_NodeFlags, nlVectorData(raw->Value), 4);
				m_HasNodeFlags = true;
			}
			break;
		case PMB_NODE_RENDERFLAGS_CHUNK_ID:
			if (!m_HasRenderFlags && raw->Value.size() >= 4)
			{
				memcpy(&m_RenderFlags, nlVectorData(raw->Value), 4);
				m_HasRenderFlags = true;
			}
			break;
		case PMB_NODE_OFFSET_POS_CHUNK_ID:
			if (!m_HasObjOffsetPos && raw->Value.size() >= 12)
			{
				memcpy(m_ObjOffsetPos, nlVectorData(raw->Value), 12);
				m_HasObjOffsetPos = true;
			}
			break;
		case PMB_NODE_OFFSET_ROT_CHUNK_ID:
			if (!m_HasObjOffsetRot && raw->Value.size() >= 16)
			{
				memcpy(m_ObjOffsetRot, nlVectorData(raw->Value), 16);
				m_HasObjOffsetRot = true;
			}
			break;
		case PMB_NODE_OFFSET_SCALE_CHUNK_ID:
			if (!m_HasObjOffsetScaleVec && raw->Value.size() >= 12)
			{
				memcpy(m_ObjOffsetScaleS, nlVectorData(raw->Value), 12);
				m_HasObjOffsetScaleVec = true;
				if (raw->Value.size() >= 28)
				{
					memcpy(m_ObjOffsetScaleQ, (const uint8 *)nlVectorData(raw->Value) + 12, 16);
					m_HasObjOffsetScale = true;
				}
			}
			break;
		}
	}
}

bool CNodeImpl::nodeFlags(uint32 &flags) const
{
	if (!m_HasNodeFlags) return false;
	flags = m_NodeFlags;
	return true;
}

bool CNodeImpl::isHidden() const
{
	return m_HasNodeFlags && (m_NodeFlags & 0x40) != 0;
}

bool CNodeImpl::renderFlags(uint32 &flags) const
{
	if (!m_HasRenderFlags) return false;
	flags = m_RenderFlags;
	return true;
}

bool CNodeImpl::objectOffsetPos(float pos[3]) const
{
	if (!m_HasObjOffsetPos) return false;
	pos[0] = m_ObjOffsetPos[0]; pos[1] = m_ObjOffsetPos[1]; pos[2] = m_ObjOffsetPos[2];
	return true;
}

bool CNodeImpl::objectOffsetRot(float rot[4]) const
{
	if (!m_HasObjOffsetRot) return false;
	rot[0] = m_ObjOffsetRot[0]; rot[1] = m_ObjOffsetRot[1]; rot[2] = m_ObjOffsetRot[2]; rot[3] = m_ObjOffsetRot[3];
	return true;
}

bool CNodeImpl::objectOffsetScale(float s[3], float q[4]) const
{
	if (!m_HasObjOffsetScale) return false;
	s[0] = m_ObjOffsetScaleS[0]; s[1] = m_ObjOffsetScaleS[1]; s[2] = m_ObjOffsetScaleS[2];
	q[0] = m_ObjOffsetScaleQ[0]; q[1] = m_ObjOffsetScaleQ[1]; q[2] = m_ObjOffsetScaleQ[2]; q[3] = m_ObjOffsetScaleQ[3];
	return true;
}

bool CNodeImpl::objectOffsetScaleVec(float s[3]) const
{
	if (!m_HasObjOffsetScaleVec) return false;
	s[0] = m_ObjOffsetScaleS[0]; s[1] = m_ObjOffsetScaleS[1]; s[2] = m_ObjOffsetScaleS[2];
	return true;
}

void CNodeImpl::clean()
{
	INode::clean();
}

void CNodeImpl::build(uint16 version, uint filter)
{
	INode::build(version);

	putChunkValue(PMB_NODE_VERSION_CHUNK_ID, m_NodeVersion);

	CStorageArray<uint32> *parent = new CStorageArray<uint32>();
	parent->Value.resize(2);
	parent->Value[0] = container()->getOrCreateStorageIndex(m_Parent);
	parent->Value[1] = m_ParentFlags;
	m_ArchivedChunks.push_back(parent);
	putChunk(PMB_NODE_PARENT_CHUNK_ID, parent);

	putChunkValue(PMB_NODE_NAME_CHUNK_ID, m_UserName);
}

void CNodeImpl::disown()
{
	m_NodeVersion = 0;
	setParent(NULL);
	m_ParentFlags = 0;
	m_UserName = ucstring("Untitled Node");

	m_HasNodeFlags = false;
	m_NodeFlags = 0;
	m_HasRenderFlags = false;
	m_RenderFlags = 0;
	m_HasObjOffsetPos = false;
	m_ObjOffsetPos[0] = m_ObjOffsetPos[1] = m_ObjOffsetPos[2] = 0.0f;
	m_HasObjOffsetRot = false;
	m_ObjOffsetRot[0] = m_ObjOffsetRot[1] = m_ObjOffsetRot[2] = 0.0f; m_ObjOffsetRot[3] = 1.0f;
	m_HasObjOffsetScaleVec = false;
	m_HasObjOffsetScale = false;
	m_ObjOffsetScaleS[0] = m_ObjOffsetScaleS[1] = m_ObjOffsetScaleS[2] = 1.0f;
	m_ObjOffsetScaleQ[0] = m_ObjOffsetScaleQ[1] = m_ObjOffsetScaleQ[2] = 0.0f; m_ObjOffsetScaleQ[3] = 1.0f;

	INode::disown();
}

void CNodeImpl::init()
{
	INode::init();
}

bool CNodeImpl::inherits(const NLMISC::CClassId classId) const
{
	if (classId == classDesc()->classId()) return true;
	return INode::inherits(classId);
}

const ISceneClassDesc *CNodeImpl::classDesc() const
{
	return &NodeImplClassDesc;
}

void CNodeImpl::toStringLocal(std::ostream &ostream, const std::string &pad, uint filter) const
{
	INode::toStringLocal(ostream, pad);
	ostream << "\n" << pad << "NodeVersion: " << m_NodeVersion;
	ostream << "\n" << pad << "Parent: ";
	INode *parent = m_Parent;
	nlassert(parent);
	if (parent)
	{
		ostream << "<ptr=0x";
		{
			std::stringstream ss;
			ss << std::hex << std::setfill('0');
			ss << std::setw(16) << (uint64)(void *)parent;
			ostream << ss.str();
		}
		ostream << "> ";
		ostream << "(" << ucstring(parent->classDesc()->displayName()).toUtf8() << ", " << parent->classDesc()->classId().toString() << ") ";
		ostream << parent->userName().toUtf8();
	}
	else
	{
		ostream << "NULL";
	}
	ostream << "\n" << pad << "ParentFlags: " << m_ParentFlags;
	ostream << "\n" << pad << "UserName: " << m_UserName.toUtf8() << " ";
	if (m_HasNodeFlags)
		ostream << "\n" << pad << "NodeFlags: 0x" << std::hex << m_NodeFlags << std::dec << (isHidden() ? " (hidden)" : "");
	if (m_HasRenderFlags)
		ostream << "\n" << pad << "RenderFlags: 0x" << std::hex << m_RenderFlags << std::dec;
	if (m_HasObjOffsetPos)
		ostream << "\n" << pad << "ObjOffsetPos: " << m_ObjOffsetPos[0] << " " << m_ObjOffsetPos[1] << " " << m_ObjOffsetPos[2];
	if (m_HasObjOffsetRot)
		ostream << "\n" << pad << "ObjOffsetRot: " << m_ObjOffsetRot[0] << " " << m_ObjOffsetRot[1] << " " << m_ObjOffsetRot[2] << " " << m_ObjOffsetRot[3];
	if (m_HasObjOffsetScaleVec)
		ostream << "\n" << pad << "ObjOffsetScale: " << m_ObjOffsetScaleS[0] << " " << m_ObjOffsetScaleS[1] << " " << m_ObjOffsetScaleS[2] << (m_HasObjOffsetScale ? "" : " (12-byte)");
}

INode *CNodeImpl::parent()
{
	return m_Parent;
}

void CNodeImpl::setParent(INode *node)
{
	if (m_Parent) m_Parent->removeChild(this);
	m_Parent = node;
	if (node) node->addChild(this);
}

const ucstring &CNodeImpl::userName() const
{
	return m_UserName;
}

IStorageObject *CNodeImpl::createChunkById(uint16 id, bool container)
{
	switch (id)
	{
	case PMB_NODE_VERSION_CHUNK_ID:
		return new CStorageValue<uint32>();
	case PMB_NODE_PARENT_CHUNK_ID:
		return new CStorageArray<uint32>();
	case PMB_NODE_NAME_CHUNK_ID:
		return new CStorageValue<ucstring>();
	}
	return INode::createChunkById(id, container);
}

} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
