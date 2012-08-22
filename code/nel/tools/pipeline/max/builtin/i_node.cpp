/**
 * \file i_node.cpp
 * \brief INode
 * \date 2012-08-22 19:45GMT
 * \author Jan Boon (Kaetemi)
 * INode
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
#include "i_node.h"

// STL includes

// NeL includes
// #include <nel/misc/debug.h>

// Project includes

using namespace std;
// using namespace NLMISC;

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {

INode::INode()
{

}

INode::~INode()
{

}

const ucchar *INode::DisplayName = ucstring("Node").c_str();
const char *INode::InternalName = "Node";
const char *INode::InternalNameUnknown = "NodeUnknown";
const NLMISC::CClassId INode::ClassId = NLMISC::CClassId(0x8f5b13, 0x624d477d); /* Not official, please correct */
const TSClassId INode::SuperClassId = 0x00000001;
const CNodeClassDesc NodeClassDesc(&DllPluginDescBuiltin);
const CNodeSuperClassDesc NodeSuperClassDesc(&NodeClassDesc);

void INode::parse(uint16 version, TParseLevel level)
{
	CReferenceTarget::parse(version, level);
}

void INode::clean()
{
	CReferenceTarget::clean();
}

void INode::build(uint16 version)
{
	CReferenceTarget::build(version);
}

void INode::disown()
{
	CReferenceTarget::disown();
}

void INode::init()
{
	CReferenceTarget::init();
}

bool INode::inherits(const NLMISC::CClassId classId) const
{
	if (classId == classDesc()->classId()) return true;
	return CReferenceTarget::inherits(classId);
}

const ISceneClassDesc *INode::classDesc() const
{
	return &NodeClassDesc;
}

void INode::toStringLocal(std::ostream &ostream, const std::string &pad) const
{
	CReferenceTarget::toStringLocal(ostream, pad);
}

IStorageObject *INode::createChunkById(uint16 id, bool container)
{
	return CReferenceTarget::createChunkById(id, container);
}

} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
