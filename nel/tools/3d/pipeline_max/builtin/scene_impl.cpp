/**
 * \file scene_impl.cpp
 * \brief CSceneImpl
 * \date 2012-08-24 12:33GMT
 * \author Jan Boon (Kaetemi)
 * CSceneImpl
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
#include "scene_impl.h"

// STL includes

// NeL includes
// #include <nel/misc/debug.h>

// Project includes

using namespace std;
// using namespace NLMISC;

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {

CSceneImpl::CSceneImpl(CScene *scene) : CReferenceMaker(scene)
{

}

CSceneImpl::~CSceneImpl()
{

}

const ucstring CSceneImpl::DisplayName = ucstring("Scene");
const char *CSceneImpl::InternalName = "SceneImpl";
const NLMISC::CClassId CSceneImpl::ClassId = NLMISC::CClassId(0x00002222, 0x00000000);
const TSClassId CSceneImpl::SuperClassId = CReferenceMaker::SuperClassId;
const CSceneImplClassDesc SceneImplClassDesc(&DllPluginDescBuiltin);

void CSceneImpl::parse(uint16 version, uint filter)
{
	CReferenceMaker::parse(version);
	nlassert(m_MaterialEditor);
	nlassert(m_MtlBaseLib);
	nlassert(m_Sound);
	nlassert(m_RootNode);
	nlassert(m_RenderEnvironment);
	nlassert(m_NamedSelSetList);
	nlassert(m_TrackViewNode);
	nlassert(m_GridReference);
	nlassert(m_RenderEffects);
	nlassert(m_ShadowMap);
	nlassert(m_LayerManager);
	if (version > Version3) nlassert(m_TrackSetList);
}

void CSceneImpl::clean()
{
	CReferenceMaker::clean();
}

void CSceneImpl::build(uint16 version, uint filter)
{
	CReferenceMaker::build(version);
}

void CSceneImpl::disown()
{
	CReferenceMaker::disown();
}

void CSceneImpl::init()
{
	CReferenceMaker::init();
}

bool CSceneImpl::inherits(const NLMISC::CClassId classId) const
{
	if (classId == classDesc()->classId()) return true;
	return CReferenceMaker::inherits(classId);
}

const ISceneClassDesc *CSceneImpl::classDesc() const
{
	return &SceneImplClassDesc;
}

void CSceneImpl::toStringLocal(std::ostream &ostream, const std::string &pad, uint filter) const
{
	CReferenceMaker::toStringLocal(ostream, pad);
}

CReferenceMaker *CSceneImpl::getReference(uint index) const
{
	switch (index)
	{
	case 0:
		return m_MaterialEditor;
	case 1:
		return m_MtlBaseLib;
	case 2:
		return m_Sound;
	case 3:
		return m_RootNode;
	case 4:
		return m_RenderEnvironment;
	case 5:
		return m_NamedSelSetList;
	case 6:
		return m_TrackViewNode;
	case 7:
		return m_GridReference;
	case 8:
		return m_RenderEffects;
	case 9:
		return m_ShadowMap;
	case 10:
		return m_LayerManager;
	case 11:
		return m_TrackSetList;
	default:
		if (index > 0)
			nlerror("Invalid index %i", index);
		return NULL;
	}
}

void CSceneImpl::setReference(uint index, CReferenceMaker *reference)
{
	switch (index)
	{
	case 0:
		m_MaterialEditor = reference;
		break;
	case 1:
		m_MtlBaseLib = reference;
		break;
	case 2:
		m_Sound = reference;
		break;
	case 3:
		m_RootNode = dynamic_cast<CRootNode *>(reference);
		break;
	case 4:
		m_RenderEnvironment = reference;
		break;
	case 5:
		m_NamedSelSetList = reference;
		break;
	case 6:
		m_TrackViewNode = dynamic_cast<CTrackViewNode *>(reference);
		break;
	case 7:
		m_GridReference = reference;
		break;
	case 8:
		m_RenderEffects = reference;
		break;
	case 9:
		m_ShadowMap = reference;
		break;
	case 10:
		m_LayerManager = reference;
		break;
	case 11:
		m_TrackSetList = reference;
		break;
	default:
		nlerror("Unknown reference index %i entry <ptr=0x%x> (%s, 0x%x)", index, (uint32)(uint64)(void *)reference, reference->classDesc()->classId().toString().c_str(), reference->classDesc()->superClassId());
		break;
	}
}

uint CSceneImpl::nbReferences() const
{
	return m_TrackSetList ? 12 : 11;
}

IStorageObject *CSceneImpl::createChunkById(uint16 id, bool container)
{
	return CReferenceMaker::createChunkById(id, container);
}

} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
