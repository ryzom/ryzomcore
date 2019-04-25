/**
 * \file root_node.cpp
 * \brief CRootNode
 * \date 2012-08-22 19:45GMT
 * \author Jan Boon (Kaetemi)
 * CRootNode
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
#include "root_node.h"

// STL includes

// NeL includes
// #include <nel/misc/debug.h>

// Project includes

using namespace std;
// using namespace NLMISC;

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {

CRootNode::CRootNode(CScene *scene) : INode(scene)
{

}

CRootNode::~CRootNode()
{

}

const ucstring CRootNode::DisplayName = ucstring("RootNode");
const char *CRootNode::InternalName = "RootNode";
const NLMISC::CClassId CRootNode::ClassId = NLMISC::CClassId(0x00000002, 0x00000000);
const TSClassId CRootNode::SuperClassId = INode::SuperClassId;
const CRootNodeClassDesc RootNodeClassDesc(&DllPluginDescBuiltin);

void CRootNode::parse(uint16 version, uint filter)
{
	INode::parse(version);
}

void CRootNode::clean()
{
	INode::clean();
}

void CRootNode::build(uint16 version, uint filter)
{
	INode::build(version);
}

void CRootNode::disown()
{
	INode::disown();
}

void CRootNode::init()
{
	INode::init();
}

bool CRootNode::inherits(const NLMISC::CClassId classId) const
{
	if (classId == classDesc()->classId()) return true;
	return INode::inherits(classId);
}

const ISceneClassDesc *CRootNode::classDesc() const
{
	return &RootNodeClassDesc;
}

void CRootNode::toStringLocal(std::ostream &ostream, const std::string &pad, uint filter) const
{
	INode::toStringLocal(ostream, pad);
}

const ucstring &CRootNode::userName() const
{
	static const ucstring v = ucstring("Root Node");
	return v;
}

IStorageObject *CRootNode::createChunkById(uint16 id, bool container)
{
	return INode::createChunkById(id, container);
}

} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
