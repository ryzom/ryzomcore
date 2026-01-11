/**
 * \file node_impl.cpp
 * \brief CNodeImpl
 * \date 2012-08-22 20:01GMT
 * \author Jan Boon (Kaetemi)
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

CNodeImpl::CNodeImpl(CScene *scene) : INode(scene), m_NodeVersion(0), m_ParentFlags(0), m_UserName(ucstring("Untitled Node"))
{

}

CNodeImpl::~CNodeImpl()
{

}

const ucstring CNodeImpl::DisplayName = ucstring("Node");
const char *CNodeImpl::InternalName = "NodeImpl";
const NLMISC::CClassId CNodeImpl::ClassId = NLMISC::CClassId(0x00000001, 0x00000000);
const TSClassId CNodeImpl::SuperClassId = INode::SuperClassId;
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
	}
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
