// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "std3d.h"

#include "nel/misc/debug.h"
#include "nel/3d/lod_character_builder.h"
#include "nel/3d/scene.h"
#include "nel/3d/skeleton_shape.h"
#include "nel/3d/mesh.h"
#include "nel/3d/skeleton_model.h"


using namespace std;
using namespace NLMISC;


namespace NL3D
{


// ***************************************************************************
CLodCharacterBuilder::CLodCharacterBuilder()
{
	_SkeletonShape= NULL;
	_LodBuild= NULL;
	_TmpScene= NULL;
}
// ***************************************************************************
CLodCharacterBuilder::~CLodCharacterBuilder()
{
	// release the scene
	if(_TmpScene)
	{
		_TmpScene->release();
		delete _TmpScene;
		_TmpScene= NULL;
	}
}

// ***************************************************************************
void			CLodCharacterBuilder::setShape(const std::string &name, CSkeletonShape *skeletonShape, CLodCharacterShapeBuild *lodBuild)
{
	nlassert(skeletonShape);
	nlassert(lodBuild);

	// SmartPtr the skeleton Shape (NB: important because skeletonModel use it)
	_SkeletonShape= skeletonShape;
	// a std ptr.
	_LodBuild= lodBuild;

	// Remap bone, with help of lodBuild and skeleton names.
	_BoneRemap.resize(lodBuild->BonesNames.size());
	for(uint i=0; i<_BoneRemap.size(); i++)
	{
		const std::string	&boneName= lodBuild->BonesNames[i];
		sint32	boneId= _SkeletonShape->getBoneIdByName(boneName);
		// If not found
		if(boneId<0)
		{
			nlwarning("Not found a bone in the skeleton Shape: %s", boneName.c_str());
			// use root bone.
			_BoneRemap[i]= 0;
		}
		else
			// remap
			_BoneRemap[i]= boneId;
	}

	// build basics
	_LodCharacterShape.buildMesh(name, *_LodBuild);

	// Build a scene, for addAnim purpose
	if(!_TmpScene)
	{
		_TmpScene= new CScene(false);
		// Must init Statics for scene (because use it in addAnim). NB: never mind if done twice.
		CScene::registerBasics();
		// init default Roots.
		_TmpScene->initDefaultRoots();
		// Don't Set driver/viewport
		// init QuadGridClipManager
		_TmpScene->initQuadGridClipManager ();
	}
}


// ***************************************************************************
void			CLodCharacterBuilder::addAnim(const char *animName, CAnimation *animation, float frameRate)
{
	nlassert(frameRate>0);
	nlassert(animation);

	/*	Create a Scene, a skeletonModel, an animation set, and a channel mixer to play the animation
		NB: no render is made and no driver is created. The scene is just here for correct creation of the skeleton
		Yoyo: This is a tricky way, but I found it the easier one...
	*/

	// Create Components necesssary to play the animation
	//==========================

	// create an animationSet, and a channelMixer.
	//--------------
	// build an animation set with the only one animation. This animation will be deleted with the animationSet
	CAnimationSet	*tmpAnimationSet= new CAnimationSet;
	tmpAnimationSet->addAnimation(animName, animation);
	tmpAnimationSet->build();
	// Build a channelMixer.
	CChannelMixer	*tmpChannelMixer= new CChannelMixer;
	tmpChannelMixer->setAnimationSet(tmpAnimationSet);


	// create a skeleton Model for animation
	//---------------
	CSkeletonModel	*skeleton= (CSkeletonModel*)_SkeletonShape->createInstance(*_TmpScene);
	// and skeleton it with animation
	skeleton->registerToChannelMixer(tmpChannelMixer, "");
	// activate the anim
	uint animID = tmpAnimationSet->getAnimationIdByName(animName);
	nlassert(animID != CAnimationSet::NotFound);
	tmpChannelMixer->setSlotAnimation(0, animID);


	// Build Dst Animation basics.
	//--------------
	CLodCharacterShape::CAnimBuild	dstAnim;
	dstAnim.Name= animName;
	dstAnim.AnimLength= animation->getEndTime();
	dstAnim.NumKeys= (uint)ceil(dstAnim.AnimLength * frameRate);
	dstAnim.NumKeys= max(1U, dstAnim.NumKeys);
	// resize array.
	dstAnim.Keys.resize(_LodCharacterShape.getNumVertices() * dstAnim.NumKeys);


	// Bake the animation
	//==========================
	double	time=0;
	double	dt= 1.0/(double)frameRate;
	uint64	evalDetaiDate= 0;
	for(uint i=0; i<dstAnim.NumKeys; i++, time+= dt)
	{
		// clamp the time
		time= min(time, (double)dstAnim.AnimLength);

		// setup the channelMixer time
		tmpChannelMixer->setSlotTime(0, (float)time);

		// Eval the channelMixer, both global and detail
		tmpChannelMixer->eval(false);
		tmpChannelMixer->eval(true, evalDetaiDate++);

		// Use the skeleton model to compute bone skin matrix, supposing an identity skeleton worldMatrix
		skeleton->computeAllBones(CMatrix::Identity);

		// apply the skinning from the current skeleton state
		applySkin(skeleton, &dstAnim.Keys[i*_LodCharacterShape.getNumVertices()]);
	}


	// Add the animation to the lod
	//==========================
	_LodCharacterShape.addAnim(dstAnim);


	// Delete
	//==========================
	// release the skeleton
	_TmpScene->deleteModel(skeleton);
	// delete the channelMixer
	delete tmpChannelMixer;
	// delete the animationSet
	delete tmpAnimationSet;
}


// ***************************************************************************
void			CLodCharacterBuilder::applySkin(CSkeletonModel *skeleton, CVector	*dstVertices)
{
	uint	numVerts= (uint)_LodBuild->Vertices.size();

	// for all vertices.
	for(uint i=0; i<numVerts; i++)
	{
		CMesh::CSkinWeight	&skinWgt= _LodBuild->SkinWeights[i];
		CVector				&srcVert= _LodBuild->Vertices[i];
		CVector				&dstVert= dstVertices[i];
		dstVert= CVector::Null;
		// parse all Weights, and add influence.
		for(uint j=0; j<NL3D_MESH_SKINNING_MAX_MATRIX; j++)
		{
			float	wgt= skinWgt.Weights[j];

			if(wgt==0)
			{
				// this should not happen, at least weight 0 should have an influence.
				if(j==0)
					dstVert= srcVert;
				// no more influence for this vertex.
				break;
			}
			else
			{
				// Get the skeleton bone to read.
				uint	boneId= _BoneRemap[skinWgt.MatrixId[j]];
				// Get the computed matrix from the skeleton.
				const	CMatrix	&boneMat= skeleton->Bones[boneId].getBoneSkinMatrix();
				// Add the influence of this bone.
				dstVert+= (boneMat * srcVert) * wgt;
			}
		}
	}
}


} // NL3D
