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

#include "scene_impl.h"

#include "i_node.h"
#include "node_impl.h"
#include "root_node.h"

#include "track_view_node.h"

#include "base_object.h"
#include "object.h"
#include "geom_object.h"
#include "tri_object.h"
#include "poly_object.h"
#include "patch_object.h"
#include "editable_patch.h"

// using namespace std;
// using namespace NLMISC;

namespace PIPELINE {
namespace MAX {
namespace BUILTIN {

namespace {

// 0x0 - invalid, default to reftarget
typedef CSuperClassDescUnknown<CReferenceTarget, 0x00000000> CNullSuperClassDesc;
const CNullSuperClassDesc NullSuperClassDesc(&ReferenceTargetClassDesc, "NullSuperClassUnknown");

// 0x9003 bezier float control, subclass under control???; control is under reftarget
typedef CSuperClassDescUnknown<CReferenceTarget, 0x00009003> CControlFloatSuperClassDesc;
const CControlFloatSuperClassDesc ControlFloatSuperClassDesc(&ReferenceTargetClassDesc, "ControlFloatSuperClassUnknown");

// 0x8 param block, under reftarget directly
typedef CSuperClassDescUnknown<CReferenceTarget, 0x00000008> CParamBlockSuperClassDesc;
const CParamBlockSuperClassDesc ParamBlockSuperClassDesc(&ReferenceTargetClassDesc, "ParamBlockSuperClassUnknown");

// 0xc20 uv gen, sub of mtlbase
typedef CSuperClassDescUnknown<CReferenceTarget, 0x00000c20> CUVGenSuperClassDesc;
const CUVGenSuperClassDesc UVGenSuperClassDesc(&ReferenceTargetClassDesc, "UVGenSuperClassUnknown");

// 0x82 param block 2, under reftarget directly
typedef CSuperClassDescUnknown<CReferenceTarget, 0x00000082> CParamBlock2SuperClassDesc;
const CParamBlock2SuperClassDesc ParamBlock2SuperClassDesc(&ReferenceTargetClassDesc, "ParamBlock2SuperClassUnknown");

// 0xc40 output, textureoutput???, under mtlbase
typedef CSuperClassDescUnknown<CReferenceTarget, 0x00000c40> CTextureOutputSuperClassDesc;
const CTextureOutputSuperClassDesc TextureOutputSuperClassDesc(&ReferenceTargetClassDesc, "TextureOutputSuperClassUnknown");

// 0xc10 texmap, under mtlbase
typedef CSuperClassDescUnknown<CReferenceTarget, 0x00000c10> CTexmapSuperClassDesc;
const CTexmapSuperClassDesc TexmapSuperClassDesc(&ReferenceTargetClassDesc, "TexmapSuperClassUnknown");

// 0x1080 texmap_container, 'Texmaps' under reftarget directly
typedef CSuperClassDescUnknown<CReferenceTarget, 0x00001080> CTexmapContainerSuperClassDesc;
const CTexmapContainerSuperClassDesc TexmapContainerSuperClassDesc(&ReferenceTargetClassDesc, "TexmapContainerSuperClassUnknown");

// 0x10b0, shader, under baseshader, under special_Fx
typedef CSuperClassDescUnknown<CReferenceTarget, 0x000010b0> CShaderSuperClassDesc;
const CShaderSuperClassDesc ShaderSuperClassDesc(&ReferenceTargetClassDesc, "ShaderSuperClassUnknown");

// 0x1110, sampler, under special_fx
typedef CSuperClassDescUnknown<CReferenceTarget, 0x00001110> CSamplerSuperClassDesc;
const CSamplerSuperClassDesc SamplerSuperClassDesc(&ReferenceTargetClassDesc, "SamplerSuperClassUnknown");

// 0xc00, mtl 'materials', under mtlbase
typedef CSuperClassDescUnknown<CReferenceTarget, 0x00000c00> CMtlSuperClassDesc;
const CMtlSuperClassDesc MtlSuperClassDesc(&ReferenceTargetClassDesc, "MtlSuperClassUnknown");

// 0xd00, soundobj, under reftarget directly
typedef CSuperClassDescUnknown<CReferenceTarget, 0x00000d00> CSoundObjSuperClassDesc;
const CSoundObjSuperClassDesc SoundObjSuperClassDesc(&ReferenceTargetClassDesc, "SoundObjSuperClassUnknown");
/*
// 0x1, node; under reftarget directly; classid 1 is node, 2 is rootnode
typedef CSuperClassDescUnknown<CReferenceTarget, > CNodeSuperClassDesc;
const CNodeSuperClassDesc NodeSuperClassDesc(&ReferenceTargetClassDesc, "NodeSuperClassUnknown");
*/
// 0x900b, controlposition, under control???
typedef CSuperClassDescUnknown<CReferenceTarget, 0x0000900b> CControlPositionSuperClassDesc;
const CControlPositionSuperClassDesc ControlPositionSuperClassDesc(&ReferenceTargetClassDesc, "ControlPositionSuperClassUnknown");

// 0x900c, controlrotation, under control???
typedef CSuperClassDescUnknown<CReferenceTarget, 0x0000900c> CControlRotationSuperClassDesc;
const CControlRotationSuperClassDesc ControlRotationSuperClassDesc(&ReferenceTargetClassDesc, "ControlRotationSuperClassUnknown");

// 0x900d, control_scale, under control???
typedef CSuperClassDescUnknown<CReferenceTarget, 0x0000900d> CControlScaleSuperClassDesc;
const CControlScaleSuperClassDesc ControlScaleSuperClassDesc(&ReferenceTargetClassDesc, "ControlScaleSuperClassUnknown");

// 0x9008, pos/rot/scale; controltransform; matrix3; under control???
typedef CSuperClassDescUnknown<CReferenceTarget, 0x00009008> CControlTransformSuperClassDesc;
const CControlTransformSuperClassDesc ControlTransformSuperClassDesc(&ReferenceTargetClassDesc, "ControlTransformSuperClassUnknown");

// 0x810 - osmodifier, under modifier (physique etc, necessary for skinning)
typedef CSuperClassDescUnknown<CReferenceTarget, 0x00000810> COSModifierSuperClassDesc;
const COSModifierSuperClassDesc OSModifierSuperClassDesc(&ReferenceTargetClassDesc, "OSModifierSuperClassUnknown");

// 0x9010 - master point controller
typedef CSuperClassDescUnknown<CReferenceTarget, 0x00009010> CMasterPointControllerSuperClassDesc;
const CMasterPointControllerSuperClassDesc MasterPointControllerSuperClassDesc(&ReferenceTargetClassDesc, "MasterPointControllerSuperClassUnknown");
/*
// 0x10 - geom object
typedef CSuperClassDescUnknown<CReferenceTarget, 0x00000010> CGeomObjectSuperClassDesc;
const CGeomObjectSuperClassDesc GeomObjectSuperClassDesc(&ReferenceTargetClassDesc, "GeomObjectSuperClassUnknown");
*/
// 0x10f0 - layer, under reftarget directly
typedef CSuperClassDescUnknown<CReferenceTarget, 0x000010f0> CLayerSuperClassDesc;
const CLayerSuperClassDesc LayerSuperClassDesc(&ReferenceTargetClassDesc, "LayerSuperClassUnknown");
/*
// 0x60 - object???, under base object
typedef CSuperClassDescUnknown<CReferenceTarget, 0x00000060> CObjectSuperClassDesc;
const CObjectSuperClassDesc ObjectSuperClassDesc(&ReferenceTargetClassDesc, "ObjectSuperClassUnknown");
*/
// 0x50 helperobject, under object...
typedef CSuperClassDescUnknown<CReferenceTarget, 0x00000050> CHelperObjectSuperClassDesc;
const CHelperObjectSuperClassDesc HelperObjectSuperClassDesc(&ReferenceTargetClassDesc, "HelperObjectSuperClassUnknown");

// 0x10a0 filterkernel, under specialfx (example: area filter)
typedef CSuperClassDescUnknown<CReferenceTarget, 0x000010a0> CFilterKernelSuperClassDesc;
const CFilterKernelSuperClassDesc FilterKernelSuperClassDesc(&ReferenceTargetClassDesc, "FilterKernelSuperClassUnknown");

// 0xf00 - renderer ,direct sub of reftarget
typedef CSuperClassDescUnknown<CReferenceTarget, 0x00000f00> CRendererSuperClassDesc;
const CRendererSuperClassDesc RendererSuperClassDesc(&ReferenceTargetClassDesc, "RendererSuperClassUnknown");

// 0x9005 - control point3 (also color), under control???
typedef CSuperClassDescUnknown<CReferenceTarget, 0x00009005> CControlPoint3SuperClassDesc;
const CControlPoint3SuperClassDesc ControlPoint3SuperClassDesc(&ReferenceTargetClassDesc, "ControlPoint3SuperClassUnknown");

// 0x1010 - atmospheric, under special effects
typedef CSuperClassDescUnknown<CReferenceTarget, 0x00001010> CAtmosphericSuperClassDesc;
const CAtmosphericSuperClassDesc AtmosphericSuperClassDesc(&ReferenceTargetClassDesc, "AtmosphericSuperClassUnknown");

// 0x9011 - control master block 'block control', under control???
typedef CSuperClassDescUnknown<CReferenceTarget, 0x00009011> CControlMasterBlockSuperClassDesc;
const CControlMasterBlockSuperClassDesc ControlMasterBlockSuperClassDesc(&ReferenceTargetClassDesc, "ControlMasterBlockSuperClassUnknown");

// 0xfffffe00 - grid reference, not sure where, probably directly under reftarget
typedef CSuperClassDescUnknown<CReferenceTarget, 0xfffffe00> CGridReferenceSuperClassDesc;
const CGridReferenceSuperClassDesc GridReferenceSuperClassDesc(&ReferenceTargetClassDesc, "GridReferenceSuperClassUnknown");

// 0x1090 - render effect, possibly under special fx
typedef CSuperClassDescUnknown<CReferenceTarget, 0x00001090> CRenderEffectSuperClassDesc;
const CRenderEffectSuperClassDesc RenderEffectSuperClassDesc(&ReferenceTargetClassDesc, "RenderEffectSuperClassUnknown");

// 0x10d0 - shadow type, directly under ref target
typedef CSuperClassDescUnknown<CReferenceTarget, 0x000010d0> CShadowTypeSuperClassDesc;
const CShadowTypeSuperClassDesc ShadowTypeSuperClassDesc(&ReferenceTargetClassDesc, "ShadowTypeSuperClassUnknown");

// 0x1160 - CustAttrib, directly under ref target
typedef CSuperClassDescUnknown<CReferenceTarget, 0x00001160> CCustAttribSuperClassDesc;
const CCustAttribSuperClassDesc CustAttribSuperClassDesc(&ReferenceTargetClassDesc, "CustAttribSuperClassUnknown");

// 0x9012 - point4list, controlpoint4, also rgba, under controll???
typedef CSuperClassDescUnknown<CReferenceTarget, 0x00009012> CControlPoint4SuperClassDesc;
const CControlPoint4SuperClassDesc ControlPoint4SuperClassDesc(&ReferenceTargetClassDesc, "ControlPoint4SuperClassUnknown");

// 0xb60 - userdatatype, deprecated, don't care
typedef CSuperClassDescUnknown<CReferenceTarget, 0x00000b60> CUserDataTypeSuperClassDesc;
const CUserDataTypeSuperClassDesc UserDataTypeSuperClassDesc(&ReferenceTargetClassDesc, "UserDataTypeSuperClassUnknown");

// 0x900f - usertype, don't care
typedef CSuperClassDescUnknown<CReferenceTarget, 0x0000900f> CUserTypeSuperClassDesc;
const CUserTypeSuperClassDesc UserTypeSuperClassDesc(&ReferenceTargetClassDesc, "UserTypeSuperClassUnknown");

// 0x40 - shape object (text, ...)
typedef CSuperClassDescUnknown<CReferenceTarget, 0x00000040> CShapeObjectSuperClassDesc;
const CShapeObjectSuperClassDesc ShapeObjectSuperClassDesc(&GeomObjectClassDesc, "ShapeObjectSuperClassUnknown");

// 0x30 - light object (omni, ...)
typedef CSuperClassDescUnknown<CReferenceTarget, 0x00000030> CLightObjectSuperClassDesc;
const CLightObjectSuperClassDesc LightObjectSuperClassDesc(&ObjectClassDesc, "LightObjectSuperClassUnknown");

// 0x20 camera - (target, ...)
typedef CSuperClassDescUnknown<CReferenceTarget, 0x00000020> CCameraObjectSuperClassDesc;
const CCameraObjectSuperClassDesc CameraObjectSuperClassDesc(&ObjectClassDesc, "CameraObjectSuperClassUnknown");

// Creating superclass 0x820 (FFD Binding) (0xd6636ea2, 0x9aa42bf3) that does not exist = WSM, sub of modifier (under baseobj)
typedef CSuperClassDescUnknown<CReferenceTarget, 0x00000820> CWSModifierSuperClassDesc;
const CWSModifierSuperClassDesc WSModifierSuperClassDesc(&BaseObjectClassDesc, "WSModifierSuperClassUnknown");

// Creating superclass 0x830 (FFD(Cyl)) (0xfa4700be, 0xbbe85051) that does not exist = WSMObject, sub of object
typedef CSuperClassDescUnknown<CReferenceTarget, 0x00000830> CWSMObjectSuperClassDesc;
const CWSMObjectSuperClassDesc WSMObjectSuperClassDesc(&ObjectClassDesc, "WSMObjectSuperClassUnknown");

// Creating superclass 0xc30 (Placement) (0x00000100, 0x00000000) that does not exist | xyzgen, sub of mtlbase
typedef CSuperClassDescUnknown<CReferenceTarget, 0x00000c30> CXYZGenSuperClassDesc;
const CXYZGenSuperClassDesc XYZGenSuperClassDesc(&ReferenceTargetClassDesc, "XYZGenSuperClassUnknown");

} /* anonymous namespace */

CBuiltin::CBuiltin()
{

}

CBuiltin::~CBuiltin()
{

}


void CBuiltin::registerClasses(CSceneClassRegistry *registry)
{
	// invalid
	registry->add(&NullSuperClassDesc);

	// available
	registry->add(&AnimatableClassDesc);
	registry->add(&AnimatableSuperClassDesc);
	registry->add(&ReferenceMakerClassDesc);
	registry->add(&ReferenceMakerSuperClassDesc);
	registry->add(&ReferenceTargetClassDesc);
	registry->add(&ReferenceTargetSuperClassDesc);

	// scene (inh ReferenceMaker)
	registry->add(&SceneImplClassDesc);

	// node (inh ReferenceTarget)
	registry->add(&NodeSuperClassDesc);
	registry->add(&NodeClassDesc);
	{
		registry->add(&NodeImplClassDesc);
		registry->add(&RootNodeClassDesc);
	}

	// tvnode (inh ReferenceTarget)
	registry->add(&TrackViewNodeClassDesc);

	// object (inh ReferenceMaker)
	registry->add(&BaseObjectClassDesc);
	{
		registry->add(&ObjectSuperClassDesc);
		registry->add(&ObjectClassDesc);
		{
			registry->add(&GeomObjectSuperClassDesc);
			registry->add(&GeomObjectClassDesc);
			{
				registry->add(&TriObjectClassDesc);
				registry->add(&PolyObjectClassDesc);
				registry->add(&PatchObjectClassDesc);
				{
					registry->add(&EditablePatchClassDesc);
				}
			}
		}
	}

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
	// registry->add(&NodeSuperClassDesc);
	registry->add(&ControlPositionSuperClassDesc);
	registry->add(&ControlRotationSuperClassDesc);
	registry->add(&ControlScaleSuperClassDesc);
	registry->add(&ControlTransformSuperClassDesc);
	registry->add(&OSModifierSuperClassDesc);
	registry->add(&MasterPointControllerSuperClassDesc);
	//registry->add(&GeomObjectSuperClassDesc);
	registry->add(&LayerSuperClassDesc);
	//registry->add(&ObjectSuperClassDesc);
	registry->add(&HelperObjectSuperClassDesc);
	registry->add(&FilterKernelSuperClassDesc);
	registry->add(&RendererSuperClassDesc);
	registry->add(&ControlPoint3SuperClassDesc);
	registry->add(&AtmosphericSuperClassDesc);
	registry->add(&ControlMasterBlockSuperClassDesc);
	registry->add(&GridReferenceSuperClassDesc);
	registry->add(&RenderEffectSuperClassDesc);
	registry->add(&ShadowTypeSuperClassDesc);
	registry->add(&CustAttribSuperClassDesc);
	registry->add(&ControlPoint4SuperClassDesc);
	registry->add(&UserDataTypeSuperClassDesc);
	registry->add(&UserTypeSuperClassDesc);
	registry->add(&ShapeObjectSuperClassDesc);
	registry->add(&LightObjectSuperClassDesc);
	registry->add(&CameraObjectSuperClassDesc);
	registry->add(&WSModifierSuperClassDesc);
	registry->add(&WSMObjectSuperClassDesc);
	registry->add(&XYZGenSuperClassDesc);
}

} /* namespace BUILTIN */
} /* namespace MAX */
} /* namespace PIPELINE */

/* end of file */
