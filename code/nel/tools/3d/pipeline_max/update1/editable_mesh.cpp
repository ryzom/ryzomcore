/**
 * \file editable_mesh.cpp
 * \brief CEditableMesh
 * \date 2012-08-26 12:11GMT
 * \author Jan Boon (Kaetemi)
 * CEditableMesh
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
#include "editable_mesh.h"

// STL includes
#include <iomanip>

// NeL includes
// #include <nel/misc/debug.h>

// Project includes
#include "update1.h"

using namespace std;
// using namespace NLMISC;

namespace PIPELINE {
namespace MAX {
namespace UPDATE1 {

CEditableMesh::CEditableMesh(CScene *scene) : CTriObject(scene)
{

}

CEditableMesh::~CEditableMesh()
{
	if (!m_ChunksOwnsPointers)
	{
		for (TStorageObjectContainer::iterator it = m_EditableMeshUnknown.begin(), end = m_EditableMeshUnknown.end(); it != end; ++it)
			delete it->second;
		m_EditableMeshUnknown.clear();
	}
}

const ucstring CEditableMesh::DisplayName = ucstring("EditableMesh");
const char *CEditableMesh::InternalName = "EditableMesh";
const NLMISC::CClassId CEditableMesh::ClassId = NLMISC::CClassId(0xe44f10b3, 0x00000000);
const TSClassId CEditableMesh::SuperClassId = CTriObject::SuperClassId;
const CEditableMeshClassDesc EditableMeshClassDesc(&DllPluginDescUpdate1);

void CEditableMesh::parse(uint16 version, uint filter)
{
	CTriObject::parse(version);

	IStorageObject *so;
	so = getChunk(0x3001);
	if (so)
	{
		m_EditableMeshUnknown.push_back(TStorageObjectWithId(0x3001, so));
		for (; ; )
		{
			if (peekChunk() == 0x2845)
			{
				so = getChunk(0x2845);
				m_EditableMeshUnknown.push_back(TStorageObjectWithId(0x2845, so));
			}
			else if (peekChunk() == 0x2846)
			{
				so = getChunk(0x2846);
				m_EditableMeshUnknown.push_back(TStorageObjectWithId(0x2846, so));
			}
			else if (peekChunk() == 0x2847)
			{
				so = getChunk(0x2847);
				m_EditableMeshUnknown.push_back(TStorageObjectWithId(0x2847, so));
			}
			else break;
		}
	}
	for (; ; )
	{
		so = getChunk(0x3003);
		if (so) m_EditableMeshUnknown.push_back(TStorageObjectWithId(0x3003, so));
		else break;
		so = getChunk(0x3004);
		if (so) m_EditableMeshUnknown.push_back(TStorageObjectWithId(0x3004, so));
	}
	so = getChunk(0x3002);
	if (so) m_EditableMeshUnknown.push_back(TStorageObjectWithId(0x3002, so));
	so = getChunk(0x4020);
	if (so) m_EditableMeshUnknown.push_back(TStorageObjectWithId(0x4020, so));
	so = getChunk(0x4024);
	if (so) m_EditableMeshUnknown.push_back(TStorageObjectWithId(0x4024, so));
	so = getChunk(0x4025);
	if (so) m_EditableMeshUnknown.push_back(TStorageObjectWithId(0x4025, so));
	so = getChunk(0x4026);
	if (so) m_EditableMeshUnknown.push_back(TStorageObjectWithId(0x4026, so));
	so = getChunk(0x402c);
	if (so) m_EditableMeshUnknown.push_back(TStorageObjectWithId(0x402c, so));
	so = getChunk(0x402d);
	if (so) m_EditableMeshUnknown.push_back(TStorageObjectWithId(0x402d, so));
	so = getChunk(0x4030);
	if (so) m_EditableMeshUnknown.push_back(TStorageObjectWithId(0x4030, so));
	so = getChunk(0x4034);
	if (so) m_EditableMeshUnknown.push_back(TStorageObjectWithId(0x4034, so));
	so = getChunk(0x4038);
	if (so) m_EditableMeshUnknown.push_back(TStorageObjectWithId(0x4038, so));
	so = getChunk(0x403b);
	if (so) m_EditableMeshUnknown.push_back(TStorageObjectWithId(0x403b, so));

	CTriObject::parse(version, PMB_TRI_OBJECT_PARSE_FILTER);
}

void CEditableMesh::clean()
{
	CTriObject::clean();
}

void CEditableMesh::build(uint16 version, uint filter)
{
	CTriObject::build(version);

	for (TStorageObjectContainer::iterator it = m_EditableMeshUnknown.begin(), end = m_EditableMeshUnknown.end(); it != end; ++it)
		putChunk(it->first, it->second);

	CTriObject::build(version, PMB_TRI_OBJECT_PARSE_FILTER);
}

void CEditableMesh::disown()
{
	m_EditableMeshUnknown.clear();
	CTriObject::disown();
}

void CEditableMesh::init()
{
	CTriObject::init();
}

bool CEditableMesh::inherits(const NLMISC::CClassId classId) const
{
	if (classId == classDesc()->classId()) return true;
	return CTriObject::inherits(classId);
}

const ISceneClassDesc *CEditableMesh::classDesc() const
{
	return &EditableMeshClassDesc;
}

void CEditableMesh::toStringLocal(std::ostream &ostream, const std::string &pad, uint filter) const
{
	CTriObject::toStringLocal(ostream, pad);

	std::string padpad = pad + "\t";
	sint i = 0;
	for (TStorageObjectContainer::const_iterator it = m_EditableMeshUnknown.begin(), end = m_EditableMeshUnknown.end(); it != end; ++it)
	{
		std::stringstream ss;
		ss << std::hex << std::setfill('0');
		ss << std::setw(4) << it->first;
		ostream << "\n" << pad << "EditableMeshUnkown[" << i << "] 0x" << ss.str() << ": ";
		it->second->toString(ostream, padpad);
		++i;
	}

	CTriObject::toStringLocal(ostream, pad, PMB_TRI_OBJECT_PARSE_FILTER);
}

IStorageObject *CEditableMesh::createChunkById(uint16 id, bool container)
{
	return CTriObject::createChunkById(id, container);
}

} /* namespace UPDATE1 */
} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
