/**
 * \file builtin.cpp
 * \brief CBuiltin
 * \date 2012-08-22 09:42GMT
 * \author Jan Boon (Kaetemi)
 * CBuiltin
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
#include "builtin.h"

// STL includes

// NeL includes
// #include <nel/misc/debug.h>

// Project includes
#include "../scene_class_registry.h"
#include "animatable.h"
#include "reference_maker.h"
#include "reference_target.h"

// using namespace std;
// using namespace NLMISC;

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {

namespace {

// 0x9003 bezier float control, subclass under control???; control is under reftarget
typedef CSuperClassDescUnknown<CReferenceTarget, 0x00009003> CControlFloatSuperClassDesc;
const CControlFloatSuperClassDesc ControlFloatSuperClassDesc(&ReferenceTargetClassDesc);

// 0x8 param block, under reftarget directly
typedef CSuperClassDescUnknown<CReferenceTarget, 0x00000008> CParamBlockSuperClassDesc;
const CParamBlockSuperClassDesc ParamBlockSuperClassDesc(&ReferenceTargetClassDesc);

// 0xc20 uv gen, sub of mtlbase
typedef CSuperClassDescUnknown<CReferenceTarget, 0x00000c20> CUVGenSuperClassDesc;
const CUVGenSuperClassDesc UVGenSuperClassDesc(&ReferenceTargetClassDesc);

// 0x82 param block 2, under reftarget directly
typedef CSuperClassDescUnknown<CReferenceTarget, 0x00000082> CParamBlock2SuperClassDesc;
const CParamBlock2SuperClassDesc ParamBlock2SuperClassDesc(&ReferenceTargetClassDesc);

// 0xc40 output, textureoutput???, under mtlbase
typedef CSuperClassDescUnknown<CReferenceTarget, 0x00000c40> CTextureOutputSuperClassDesc;
const CTextureOutputSuperClassDesc TextureOutputSuperClassDesc(&ReferenceTargetClassDesc);

// 0xc10 texmap, under mtlbase
typedef CSuperClassDescUnknown<CReferenceTarget, 0x00000c10> CTexmapSuperClassDesc;
const CTexmapSuperClassDesc TexmapSuperClassDesc(&ReferenceTargetClassDesc);

// 0x1080 texmap_container, 'Texmaps' under reftarget directly
typedef CSuperClassDescUnknown<CReferenceTarget, 0x00001080> CTexmapContainerSuperClassDesc;
const CTexmapContainerSuperClassDesc TexmapContainerSuperClassDesc(&ReferenceTargetClassDesc);

// 0x10b0, shader, under baseshader, under special_Fx
typedef CSuperClassDescUnknown<CReferenceTarget, 0x000010b0> CShaderSuperClassDesc;
const CShaderSuperClassDesc ShaderSuperClassDesc(&ReferenceTargetClassDesc);

// 0x1110, sampler, under special_fx
typedef CSuperClassDescUnknown<CReferenceTarget, 0x00001110> CSamplerSuperClassDesc;
const CSamplerSuperClassDesc SamplerSuperClassDesc(&ReferenceTargetClassDesc);

// 0xc00, mtl 'materials', under mtlbase
typedef CSuperClassDescUnknown<CReferenceTarget, 0x00000c00> CMtlSuperClassDesc;
const CMtlSuperClassDesc MtlSuperClassDesc(&ReferenceTargetClassDesc);

// 0xd00, soundobj, under reftarget directly
typedef CSuperClassDescUnknown<CReferenceTarget, 0x00000d00> CSoundObjSuperClassDesc;
const CSoundObjSuperClassDesc SoundObjSuperClassDesc(&ReferenceTargetClassDesc);

// 0x1, node; under reftarget directly; classid 1 is node, 2 is rootnode
typedef CSuperClassDescUnknown<CReferenceTarget, 0x00000001> CNodeSuperClassDesc;
const CNodeSuperClassDesc NodeSuperClassDesc(&ReferenceTargetClassDesc);

// 0x900b, controlposition, under control???
typedef CSuperClassDescUnknown<CReferenceTarget, 0x0000900b> CControlPositionSuperClassDesc;
const CControlPositionSuperClassDesc ControlPositionSuperClassDesc(&ReferenceTargetClassDesc);

// 0x900c, controlrotation, under control???
typedef CSuperClassDescUnknown<CReferenceTarget, 0x0000900c> CControlRotationSuperClassDesc;
const CControlRotationSuperClassDesc ControlRotationSuperClassDesc(&ReferenceTargetClassDesc);

// 0x900d, control_scale, under control???
typedef CSuperClassDescUnknown<CReferenceTarget, 0x0000900d> CControlScaleSuperClassDesc;
const CControlScaleSuperClassDesc ControlScaleSuperClassDesc(&ReferenceTargetClassDesc);

// 0x9008, pos/rot/scale; controltransform; matrix3; under control???
typedef CSuperClassDescUnknown<CReferenceTarget, 0x00009008> CControlTransformSuperClassDesc;
const CControlTransformSuperClassDesc ControlTransformSuperClassDesc(&ReferenceTargetClassDesc);

// 0x810 - osmodifier, under modifier (physique etc, necessary for skinning)
typedef CSuperClassDescUnknown<CReferenceTarget, 0x00000810> COSModifierSuperClassDesc;
const COSModifierSuperClassDesc OSModifierSuperClassDesc(&ReferenceTargetClassDesc);

// 0x9010 - master point controller
typedef CSuperClassDescUnknown<CReferenceTarget, 0x00009010> CMasterPointControllerSuperClassDesc;
const CMasterPointControllerSuperClassDesc MasterPointControllerSuperClassDesc(&ReferenceTargetClassDesc);

// 0x10 - geom object
typedef CSuperClassDescUnknown<CReferenceTarget, 0x00000010> CGeomObjectSuperClassDesc;
const CGeomObjectSuperClassDesc GeomObjectSuperClassDesc(&ReferenceTargetClassDesc);

// 0x10f0 - layer, under reftarget directly
typedef CSuperClassDescUnknown<CReferenceTarget, 0x000010f0> CLayerSuperClassDesc;
const CLayerSuperClassDesc LayerSuperClassDesc(&ReferenceTargetClassDesc);

// 0x60 - object???, under base object
typedef CSuperClassDescUnknown<CReferenceTarget, 0x00000060> CObjectSuperClassDesc;
const CObjectSuperClassDesc ObjectSuperClassDesc(&ReferenceTargetClassDesc);

// 0x50 helperobject, under object...
typedef CSuperClassDescUnknown<CReferenceTarget, 0x00000050> CHelperObjectSuperClassDesc;
const CHelperObjectSuperClassDesc HelperObjectSuperClassDesc(&ReferenceTargetClassDesc);

} /* anonymous namespace */

CBuiltin::CBuiltin()
{

}

CBuiltin::~CBuiltin()
{

}


void CBuiltin::registerClasses(CSceneClassRegistry *registry)
{
	registry->add(&AnimatableClassDesc);
	registry->add(&AnimatableSuperClassDesc);
	registry->add(&ReferenceMakerClassDesc);
	registry->add(&ReferenceMakerSuperClassDesc);
	registry->add(&ReferenceTargetClassDesc);
	registry->add(&ReferenceTargetSuperClassDesc);

	// unimplemented
	registry->add(&ControlFloatSuperClassDesc);
	registry->add(&ParamBlockSuperClassDesc);
	registry->add(&UVGenSuperClassDesc);
	registry->add(&ParamBlock2SuperClassDesc);
	registry->add(&TextureOutputSuperClassDesc);
	registry->add(&TexmapSuperClassDesc);
	registry->add(&TexmapContainerSuperClassDesc);
	registry->add(&ShaderSuperClassDesc);
	registry->add(&SamplerSuperClassDesc);
	registry->add(&MtlSuperClassDesc);
	registry->add(&SoundObjSuperClassDesc);
	registry->add(&NodeSuperClassDesc);
	registry->add(&ControlPositionSuperClassDesc);
	registry->add(&ControlRotationSuperClassDesc);
	registry->add(&ControlScaleSuperClassDesc);
	registry->add(&ControlTransformSuperClassDesc);
	registry->add(&OSModifierSuperClassDesc);
	registry->add(&MasterPointControllerSuperClassDesc);
	registry->add(&GeomObjectSuperClassDesc);
	registry->add(&LayerSuperClassDesc);
	registry->add(&ObjectSuperClassDesc);
	registry->add(&HelperObjectSuperClassDesc);
}

} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
