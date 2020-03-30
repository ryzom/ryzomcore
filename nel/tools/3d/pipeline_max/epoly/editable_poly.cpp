/**
 * \file editable_poly.cpp
 * \brief CEditablePoly
 * \date 2012-08-26 12:02GMT
 * \author Jan Boon (Kaetemi)
 * CEditablePoly
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
#include "editable_poly.h"

// STL includes
#include <iomanip>

// NeL includes
// #include <nel/misc/debug.h>

// Project includes
#include "epoly.h"

using namespace std;
// using namespace NLMISC;

namespace PIPELINE {
namespace MAX {
namespace EPOLY {

CEditablePoly::CEditablePoly(CScene *scene) : CPolyObject(scene)
{

}

CEditablePoly::~CEditablePoly()
{
	if (!m_ChunksOwnsPointers)
	{
		for (TStorageObjectContainer::iterator it = m_EditablePolyUnknown.begin(), end = m_EditablePolyUnknown.end(); it != end; ++it)
			delete it->second;
		m_EditablePolyUnknown.clear();
	}
}

const ucstring CEditablePoly::DisplayName = ucstring("EditablePoly");
const char *CEditablePoly::InternalName = "EditablePoly";
const NLMISC::CClassId CEditablePoly::ClassId = NLMISC::CClassId(0x1bf8338d, 0x192f6098);
const TSClassId CEditablePoly::SuperClassId = CPolyObject::SuperClassId;
const CEditablePolyClassDesc EditablePolyClassDesc(&DllPluginDescEPoly);

void CEditablePoly::parse(uint16 version, uint filter)
{
	CPolyObject::parse(version);

	IStorageObject *so;
	so = getChunk(0x4039);
	if (so) m_EditablePolyUnknown.push_back(TStorageObjectWithId(0x4039, so));
	so = getChunk(0x403a);
	if (so) m_EditablePolyUnknown.push_back(TStorageObjectWithId(0x403a, so));
	for (; ; )
	{ // note: also in editable mesh, copy paste or related somehow? / use a common parser class inbetween?
		so = getChunk(0x3003);
		if (so) m_EditablePolyUnknown.push_back(TStorageObjectWithId(0x3003, so));
		else break;
		so = getChunk(0x3004);
		if (so) m_EditablePolyUnknown.push_back(TStorageObjectWithId(0x3004, so));
	}
	so = getChunk(0x3002);
	if (so) m_EditablePolyUnknown.push_back(TStorageObjectWithId(0x3002, so));
	so = getChunk(0x4038);
	if (so) m_EditablePolyUnknown.push_back(TStorageObjectWithId(0x4038, so));

	CPolyObject::parse(version, PMB_POLY_OBJECT_PARSE_FILTER);
}

void CEditablePoly::clean()
{
	CPolyObject::clean();
}

void CEditablePoly::build(uint16 version, uint filter)
{
	CPolyObject::build(version);

	for (TStorageObjectContainer::iterator it = m_EditablePolyUnknown.begin(), end = m_EditablePolyUnknown.end(); it != end; ++it)
		putChunk(it->first, it->second);

	CPolyObject::build(version, PMB_POLY_OBJECT_PARSE_FILTER);
}

void CEditablePoly::disown()
{
	m_EditablePolyUnknown.clear();
	CPolyObject::disown();
}

void CEditablePoly::init()
{
	CPolyObject::init();
}

bool CEditablePoly::inherits(const NLMISC::CClassId classId) const
{
	if (classId == classDesc()->classId()) return true;
	return CPolyObject::inherits(classId);
}

const ISceneClassDesc *CEditablePoly::classDesc() const
{
	return &EditablePolyClassDesc;
}

void CEditablePoly::toStringLocal(std::ostream &ostream, const std::string &pad, uint filter) const
{
	CPolyObject::toStringLocal(ostream, pad);

	std::string padpad = pad + "\t";
	sint i = 0;
	for (TStorageObjectContainer::const_iterator it = m_EditablePolyUnknown.begin(), end = m_EditablePolyUnknown.end(); it != end; ++it)
	{
		std::stringstream ss;
		ss << std::hex << std::setfill('0');
		ss << std::setw(4) << it->first;
		ostream << "\n" << pad << "EditablePolyUnkown[" << i << "] 0x" << ss.str() << ": ";
		it->second->toString(ostream, padpad);
		++i;
	}

	CPolyObject::toStringLocal(ostream, pad, PMB_POLY_OBJECT_PARSE_FILTER);
}

IStorageObject *CEditablePoly::createChunkById(uint16 id, bool container)
{
	return CPolyObject::createChunkById(id, container);
}

} /* namespace EPOLY */
} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
