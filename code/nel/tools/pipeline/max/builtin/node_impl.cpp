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

// NeL includes
// #include <nel/misc/debug.h>

// Project includes

using namespace std;
// using namespace NLMISC;

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {

CNodeImpl::CNodeImpl(PIPELINE::MAX::CScene *scene) : INode(scene)
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

void CNodeImpl::parse(uint16 version, TParseLevel level)
{
	INode::parse(version, level);
}

void CNodeImpl::clean()
{
	INode::clean();
}

void CNodeImpl::build(uint16 version)
{
	INode::build(version);
}

void CNodeImpl::disown()
{
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

void CNodeImpl::toStringLocal(std::ostream &ostream, const std::string &pad) const
{
	INode::toStringLocal(ostream, pad);
}

IStorageObject *CNodeImpl::createChunkById(uint16 id, bool container)
{
	return INode::createChunkById(id, container);
}

} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
