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
#include <iomanip>

// NeL includes
// #include <nel/misc/debug.h>

// Project includes

using namespace std;
// using namespace NLMISC;

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {

INode::INode(CScene *scene) : CReferenceTarget(scene)
{

}

INode::~INode()
{

}

const ucstring INode::DisplayName = ucstring("Node Interface");
const char *INode::InternalName = "Node";
const char *INode::InternalNameUnknown = "NodeUnknown";
const NLMISC::CClassId INode::ClassId = NLMISC::CClassId(0x8f5b13, 0x624d477d); /* Not official, please correct */
const TSClassId INode::SuperClassId = 0x00000001;
const CNodeClassDesc NodeClassDesc(&DllPluginDescBuiltin);
const CNodeSuperClassDesc NodeSuperClassDesc(&NodeClassDesc);

void INode::parse(uint16 version, uint filter)
{
	CReferenceTarget::parse(version);
}

void INode::clean()
{
	CReferenceTarget::clean();
}

void INode::build(uint16 version, uint filter)
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

void INode::toStringLocal(std::ostream &ostream, const std::string &pad, uint filter) const
{
	CReferenceTarget::toStringLocal(ostream, pad);
	// Print the implied connected children
	ostream << "\n" << pad << "Children: IMPLICIT { ";
	uint i = 0;
	for (std::set<NLMISC::CRefPtr<INode> >::iterator it = m_Children.begin(), end = m_Children.end(); it != end; ++it)
	{
		INode *node = (*it);
		nlassert(node);
		if (node)
		{
			ostream << "\n" << pad << "\t" << i << ": <ptr=0x";
			{
				std::stringstream ss;
				ss << std::hex << std::setfill('0');
				ss << std::setw(16) << (uint64)(void *)node;
				ostream << ss.str();
			}
			ostream << "> ";
			ostream << "(" << ucstring(node->classDesc()->displayName()).toUtf8() << ", " << node->classDesc()->classId().toString() << ") ";
			ostream << node->userName().toUtf8() << " ";
		}
		else
		{
			ostream << "\n" << pad << "\t" << i << ": NULL ";
		}
		++i;
	}
	ostream << "} ";
}

INode *INode::parent()
{
	nlerror("Unkown node class, cannot get parent node");
	return NULL;
}

void INode::setParent(INode *node)
{
	nlerror("Unkown node class, cannot set parent node");
}

void INode::addChild(INode *node)
{
	m_Children.insert(node);
}

void INode::removeChild(INode *node)
{
	m_Children.erase(node);
}

const ucstring &INode::userName() const
{
	static const ucstring v = ucstring("Invalid INode");
	return v;
}

INode *INode::find(const ucstring &userName) const
{
	ucstring unl = NLMISC::toLower(userName);
	for (std::set<NLMISC::CRefPtr<INode> >::iterator it = m_Children.begin(), end = m_Children.end(); it != end; ++it)
	{
		INode *node = (*it);
		nlassert(node);
		if (NLMISC::toLower(node->userName()) == unl)
			return node;
	}
	return NULL;
}

void INode::dumpNodes(std::ostream &ostream, const std::string &pad) const
{
	ostream << "<ptr=0x";
	{
		std::stringstream ss;
		ss << std::hex << std::setfill('0');
		ss << std::setw(16) << (uint64)(void *)this;
		ostream << ss.str();
	}
	ostream << "> " << userName().toUtf8() << " [" << m_Children.size() << "] { ";
	CReferenceMaker *object = getReference(1);
	if (object) // TODO: Implement!
	{
		ostream << "\n" << pad << "Object: ";
		ostream << "<ptr=0x";
		{
			std::stringstream ss;
			ss << std::hex << std::setfill('0');
			ss << std::setw(16) << (uint64)(void *)object;
			ostream << ss.str();
		}
		ostream << "> ";
		ostream << ucstring(object->classDesc()->displayName()).toUtf8() << " ";
	}
	uint i = 0 ;
	std::string padpad = pad + "\t";
	for (std::set<NLMISC::CRefPtr<INode> >::iterator it = m_Children.begin(), end = m_Children.end(); it != end; ++it)
	{
		INode *node = (*it);
		nlassert(node);
		ostream << "\n" << pad << i << ": ";
		node->dumpNodes(ostream, padpad);
		++i;
	}
	ostream << "} ";
}

IStorageObject *INode::createChunkById(uint16 id, bool container)
{
	return CReferenceTarget::createChunkById(id, container);
}

} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
