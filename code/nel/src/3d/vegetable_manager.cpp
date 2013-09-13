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


#include "nel/3d/vegetable_manager.h"
#include "nel/3d/driver.h"
#include "nel/3d/texture_file.h"
#include "nel/misc/fast_floor.h"
#include "nel/3d/vegetable_quadrant.h"
#include "nel/3d/dru.h"
#include "nel/3d/radix_sort.h"
#include "nel/3d/scene.h"
#include "nel/3d/vegetable_blend_layer_model.h"
#include "nel/3d/vegetable_light_ex.h"
#include "nel/misc/hierarchical_timer.h"

#include <algorithm>


using namespace std;
using namespace NLMISC;


namespace NL3D
{


#define	NL3D_VEGETABLE_CLIP_BLOCK_BLOCKSIZE			16
#define	NL3D_VEGETABLE_SORT_BLOCK_BLOCKSIZE			64
#define	NL3D_VEGETABLE_INSTANCE_GROUP_BLOCKSIZE		128


// ***************************************************************************
CVegetableManager::CVegetableManager(uint maxVertexVbHardUnlit, uint maxVertexVbHardLighted,
	uint nbBlendLayers, float blendLayerDistMax) :
	_ClipBlockMemory(NL3D_VEGETABLE_CLIP_BLOCK_BLOCKSIZE),
	_SortBlockMemory(NL3D_VEGETABLE_SORT_BLOCK_BLOCKSIZE),
	_InstanceGroupMemory(NL3D_VEGETABLE_INSTANCE_GROUP_BLOCKSIZE),
	_GlobalDensity(1.f),
	_NumZSortBlendLayers(nbBlendLayers), _ZSortLayerDistMax(blendLayerDistMax),
	_ZSortScene(NULL)
{
	uint	i;

	// Init all the allocators
	nlassert((uint)(CVegetableVBAllocator::VBTypeCount) == 2);
	_VBHardAllocator[CVegetableVBAllocator::VBTypeLighted].init( CVegetableVBAllocator::VBTypeLighted, maxVertexVbHardLighted );
	_VBHardAllocator[CVegetableVBAllocator::VBTypeUnlit].init( CVegetableVBAllocator::VBTypeUnlit, maxVertexVbHardUnlit );
	// Init soft one, with no vbHard vertices.
	_VBSoftAllocator[CVegetableVBAllocator::VBTypeLighted].init( CVegetableVBAllocator::VBTypeLighted, 0 );
	_VBSoftAllocator[CVegetableVBAllocator::VBTypeUnlit].init( CVegetableVBAllocator::VBTypeUnlit, 0 );

	// NB Vertex programs are initilized during the first call to update driver.

	// setup the material. Unlit (doesn't matter, lighting in VP) Alpha Test.
	_VegetableMaterial.initUnlit();
	_VegetableMaterial.setAlphaTest(true);
	_VegetableMaterial.setBlendFunc(CMaterial::srcalpha, CMaterial::invsrcalpha);

	// default light.
	_DirectionalLight= (CVector(0,1, -1)).normed();
	_GlobalAmbient.set(64, 64, 64, 255);
	_GlobalDiffuse.set(150, 150, 150, 255);

	// Wind.
	_WindDirection.set(1,0,0);
	_WindFrequency= 1;
	_WindPower= 1;
	_WindBendMin= 0;
	_Time= 0;
	_WindPrecRenderTime= 0;
	_WindAnimTime= 0;

	// Init CosTable.
	for(i=0; i<NL3D_VEGETABLE_VP_LUT_SIZE; i++)
	{
		_CosTable[i]= (float)cos( i*2*Pi / NL3D_VEGETABLE_VP_LUT_SIZE );
	}

	// init to NULL _ZSortModelLayers.
	_NumZSortBlendLayers= max(1U, _NumZSortBlendLayers);
	_ZSortModelLayers.resize(_NumZSortBlendLayers, NULL);
	_ZSortModelLayersUW.resize(_NumZSortBlendLayers, NULL);


	// UL
	_ULFrequency= 0;
	_ULNVerticesToUpdate=0;
	_ULNTotalVertices= 0;
	_ULRootIg= NULL;
	_ULCurrentIgRdrPass= 0;
	_ULCurrentIgInstance= 0;
	_ULPrecTime= 0;
	_ULPrecTimeInit= false;
	_ULTime= 0;

	// Misc.
	_NumVegetableFaceRendered= 0;

	for (uint k = 0; k < NL3D_VEGETABLE_NRDRPASS; ++k)
	{
		_VertexProgram[k][0] = NULL;
		_VertexProgram[k][1] = NULL;
	}
}


// ***************************************************************************
CVegetableManager::~CVegetableManager()
{
	// delete All VP
	for(sint i=0; i <NL3D_VEGETABLE_NRDRPASS; i++)
	{
		delete	_VertexProgram[i][0];
		delete	_VertexProgram[i][1];
		_VertexProgram[i][0] = NULL;
		_VertexProgram[i][1] = NULL;
	}

	// delete ZSort models.
	if(_ZSortScene)
	{
		// remove models from scene.
		for(uint i= 0; i<_NumZSortBlendLayers; i++)
		{
			_ZSortScene->deleteModel(_ZSortModelLayers[i]);
			_ZSortModelLayers[i]= NULL;
			_ZSortScene->deleteModel(_ZSortModelLayersUW[i]);
			_ZSortModelLayersUW[i]= NULL;
		}

		_ZSortScene= NULL;
	}
}


// ***************************************************************************
void		CVegetableManager::createVegetableBlendLayersModels(CScene *scene)
{
	// setup scene
	nlassert(scene);
	_ZSortScene= scene;

	// create the layers models.
	for(uint i=0;i<_NumZSortBlendLayers; i++)
	{
		// assert not already done.
		nlassert(_ZSortModelLayers[i]==NULL);
		nlassert(_ZSortModelLayersUW[i]==NULL);

		_ZSortModelLayers[i]= (CVegetableBlendLayerModel*)scene->createModel(VegetableBlendLayerModelId);
		_ZSortModelLayersUW[i]= (CVegetableBlendLayerModel*)scene->createModel(VegetableBlendLayerModelId);
		// init owner.
		_ZSortModelLayers[i]->VegetableManager= this;
		_ZSortModelLayersUW[i]->VegetableManager= this;

		// Set UnderWater layer for _ZSortModelLayersUW
		_ZSortModelLayersUW[i]->setOrderingLayer(2);
	}
}


// ***************************************************************************
CVegetableVBAllocator	&CVegetableManager::getVBAllocatorForRdrPassAndVBHardMode(uint rdrPass, uint vbHardMode)
{
	// If software VB
	if(vbHardMode==0)
	{
		if(rdrPass == NL3D_VEGETABLE_RDRPASS_LIGHTED)
			return _VBSoftAllocator[CVegetableVBAllocator::VBTypeLighted];
		if(rdrPass == NL3D_VEGETABLE_RDRPASS_LIGHTED_2SIDED)
			return _VBSoftAllocator[CVegetableVBAllocator::VBTypeLighted];
		if(rdrPass == NL3D_VEGETABLE_RDRPASS_UNLIT)
			return _VBSoftAllocator[CVegetableVBAllocator::VBTypeUnlit];
		if(rdrPass == NL3D_VEGETABLE_RDRPASS_UNLIT_2SIDED)
			return _VBSoftAllocator[CVegetableVBAllocator::VBTypeUnlit];
		if(rdrPass == NL3D_VEGETABLE_RDRPASS_UNLIT_2SIDED_ZSORT)
			return _VBSoftAllocator[CVegetableVBAllocator::VBTypeUnlit];
	}
	// If hard VB
	else
	{
		if(rdrPass == NL3D_VEGETABLE_RDRPASS_LIGHTED)
			return _VBHardAllocator[CVegetableVBAllocator::VBTypeLighted];
		if(rdrPass == NL3D_VEGETABLE_RDRPASS_LIGHTED_2SIDED)
			return _VBHardAllocator[CVegetableVBAllocator::VBTypeLighted];
		if(rdrPass == NL3D_VEGETABLE_RDRPASS_UNLIT)
			return _VBHardAllocator[CVegetableVBAllocator::VBTypeUnlit];
		if(rdrPass == NL3D_VEGETABLE_RDRPASS_UNLIT_2SIDED)
			return _VBHardAllocator[CVegetableVBAllocator::VBTypeUnlit];
		if(rdrPass == NL3D_VEGETABLE_RDRPASS_UNLIT_2SIDED_ZSORT)
			return _VBHardAllocator[CVegetableVBAllocator::VBTypeUnlit];
	}

	// abnormal case
	nlstop;
	// To avoid warning;
	return _VBSoftAllocator[0];
}



// ***************************************************************************
// ***************************************************************************
// Vertex Program.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
/*
	Vegetable, without bend for now.

	Inputs
	--------
	v[0]  == Pos to Center of the vegetable in world space.
	v[10] == Center of the vegetable in world space.
	v[2]  == Normal (present if lighted only)
	v[3]  == Color (if unlit) or DiffuseColor (if lighted)
	v[4]  == SecondaryColor (==ambient if Lighted, and use only Alpha part for DLM if Unlit)
	v[8]  == Tex0 (xy)
	v[9]  == BendInfo (xyz) = {BendWeight/2, BendPhase, BendFrequencyFactor}
		NB: /2 because compute a quaternion

	Changes: If unlit, then small changes:
	v[0]  == Pos to center, with v[0].w == BendWeight * v[0].norm()
	v[9]  == BendInfo/BlendInfo (xyzw) = {v[0].norm(), BendPhase, BendFrequencyFactor, BlendDist}

	NB: v[9].w. is used only in Unlit+2Sided+AlphaBlend. But prefer do this for gestion purpose:
		to have only one VBAllocator for all modes.

	NB: Color and Secondary color Alpha Part contains Dynamic LightMap UV, (in 8 bits).

	Constant:
	--------
	Setuped at beginning of CVegetableManager::render()
	c[0..3]= ModelViewProjection Matrix.
	c[6]= Fog vector.
	c[8]= {0, 1, 0.5, 2}
	c[9]= unit world space Directionnal light.
	c[10]= camera pos in world space.
	c[11]= {1/DistBlendTransition}
	NB: DiffuseColor and AmbientColor of vertex must have been pre-multiplied by lightColor

	// Bend:
	c[16]= quaternion axis. w==1, and z must be 0
	c[17]=	{ timeAnim , WindPower, WindPower*(1-WindBendMin)/2, 0 }
	c[18]=	High order Taylor cos coefficient: { -1/2, 1/24, -1/720, 1/40320 }
	c[19]=	Low order Taylor cos coefficient: { 1, -1/2, 1/24, -1/720 }
	c[20]=	Low order Taylor sin coefficient: { 1, -1/6, 1/120, -1/5040 }
	c[21]=	Special constant vector for quatToMatrix: { 0, 1, -1, 0 }
	c[22]=	{0.5, Pi, 2*Pi, 1/(2*Pi)}
	c[23]=	{64, 0, 0, 0}  (size of the LUT)

	// Bend Lut:
	c[32..95] 64 Lut entries for cos-like animation


	Fog Note:
	-----------
	Fog should be disabled, because not computed (for speed consideration and becasue micro-vegetation should never
	be in Fog).


	Speed Note:
	-----------
	Max program length (lighted/2Sided) is:
		29 (bend-quaternion) +
		16 (rotNormal + bend + lit 2Sided) +
		5  (proj + tex)
		2  (Dynamic lightmap copy)
		51

	Normal program length (unlit/2Sided/No Alpha Blend) is:
		12 (bend-delta) +
		1  (unlit) +
		5  (proj + tex)
		2  (Dynamic lightmap copy)
		20

	AlphaBlend program length (unlit/2Sided/Alpha Blend) is:
		12 (bend-delta) +
		1  (unlit) +
		5  (Alpha Blend)
		5  (proj + tex)
		2  (Dynamic lightmap copy)
		26

*/


// ***********************
/*
	Fast (but less accurate) Bend program:
		Result: bend pos into R5,
*/
// ***********************
const char* NL3D_FastBendProgram=
"!!VP1.0																				\n\
	# compute time of animation: time*freqfactor + phase.								\n\
	MAD	R0.x, c[17].x, v[9].z, v[9].y;	# R0.x= time of animation							\n\
																						\n\
	# animation: use the 64 LUT entries													\n\
	EXP	R0.y, R0.x;						# fract part=> R0.y= [0,1[						\n\
	MUL	R0,  R0.y, c[23].xyyy;			# R0.x= [0,64[									\n\
	ARL	A0.x, R0.x;						# A0.x= index in the LUT						\n\
	EXP	R0.y, R0.x;						# R0.y= R0.x-A0.x= fp (fract part)				\n\
	# lookup and lerp in one it: R0= value + fp * dv.									\n\
	MAD	R0.xy, R0.y, c[A0.x+32].zwww, c[A0.x+32].xyww;									\n\
																						\n\
	# The direction and power of the wind is encoded in the LUT.						\n\
	# Scale now by vertex BendFactor (stored in v[0].w)									\n\
	MAD	R5, R0, v[0].w, v[0].xyzw;													\n\
	# compute 1/norm, and multiply by original norm stored in v[9].x					\n\
	DP3	R0.x, R5, R5;																	\n\
	RSQ	R0.x, R0.x;																		\n\
	MUL	R0.x, R0.x, v[9].x;																\n\
	# mul by this factor, and add to center												\n\
	MAD	R5, R0.xxxw, R5, v[10];															\n\
																						\n\
	# make local to camera pos. Important for ZBuffer precision							\n\
	ADD R5, R5, -c[10];																	\n\
";


// Test
/*const char* NL3D_FastBendProgram=
"!!VP1.0																				\n\
	# compute time of animation: time + phase.											\n\
	ADD	R0.x, c[17].x, v[9].y;		# R0.x= time of animation							\n\
																						\n\
	# animation: f(x)= cos(x). compute a high precision cosinus							\n\
	EXP	R0.y, R0.x;						# fract part=> R0.y= [0,1] <=> [-Pi, Pi]		\n\
	MAD	R0.x, R0.y, c[22].z, -c[22].y;	# R0.x= a= [-Pi, Pi]							\n\
	# R0 must get a2, a4, a6, a8														\n\
	MUL	R0.x, R0.x, R0.x;				# R0.x= a2										\n\
	MUL	R0.y, R0.x, R0.x;				# R0= a2, a4									\n\
	MUL	R0.zw, R0.y, R0.xxxy;			# R0= a2, a4, a6, a8							\n\
	# Taylor serie: cos(x)= 1 - (1/2) a2 + (1/24) a4 - (1/720) a6 + (1/40320) a8.		\n\
	DP4	R0.x, R0, c[18];				# R0.x= cos(x) - 1.								\n\
																						\n\
																						\n\
	# original	norm																	\n\
	DP3	R2.x, v[0], v[0];																\n\
	RSQ	R2.y, R2.x;																		\n\
	MUL	R2.x, R2.x, R2.y;																\n\
	# norm, mul by factor, and add to relpos											\n\
	ADD	R1.x, R0.x, c[8].w;																	\n\
	MUL	R0.x, v[9].x, R2.x;															\n\
	MUL	R1, R1, R0.x;																	\n\
	ADD	R5.xyz, R1, v[0];																\n\
	# mod norm																			\n\
	DP3	R0.x, R5, R5;																	\n\
	RSQ	R0.x, R0.x;																		\n\
	MUL	R0.x, R0.x, R2.x;																\n\
	MAD	R5, R0.x, R5, v[10];															\n\
";*/



// ***********************
/*
	Bend start program:
		Result: bend pos into R5, and R7,R8,R9 is the rotation matrix for possible normal lighting.
*/
// ***********************
// Splitted in 2 parts because of the 2048 char limit
const char* NL3D_BendProgramP0=
"!!VP1.0																				\n\
	# compute time of animation: time*freqfactor + phase.								\n\
	MAD	R0.x, c[17].x, v[9].z, v[9].y;	# R0.x= time of animation							\n\
																						\n\
	# animation: f(x)= cos(x). compute a high precision cosinus							\n\
	EXP	R0.y, R0.x;						# fract part=> R0.y= [0,1] <=> [-Pi, Pi]		\n\
	MAD	R0.x, R0.y, c[22].z, -c[22].y;	# R0.x= a= [-Pi, Pi]							\n\
	# R0 must get a2, a4, a6, a8														\n\
	MUL	R0.x, R0.x, R0.x;				# R0.x= a2										\n\
	MUL	R0.y, R0.x, R0.x;				# R0= a2, a4									\n\
	MUL	R0.zw, R0.y, R0.xxxy;			# R0= a2, a4, a6, a8							\n\
	# Taylor serie: cos(x)= 1 - (1/2) a2 + (1/24) a4 - (1/720) a6 + (1/40320) a8.		\n\
	DP4	R0.x, R0, c[18];				# R0.x= cos(x) - 1.								\n\
																						\n\
	# R0.x= [-2, 0]. And we want a result in BendWeight/2*WindPower*[WindBendMin, 1]	\n\
	MAD	R0.x, R0.x, c[17].z, c[17].y;	# R0.x= WindPower*[WindBendMin, 1]				\n\
	MUL	R0.x, R0.x, v[9].x;				# R0.x= angle= BendWeight/2*WindPower*[WindBendMin, 1]	\n\
																						\n\
	# compute good precision sinus and cosinus, in R0.xy.								\n\
	# suppose that BendWeightMax/2== 2Pi/3 => do not need to fmod() nor					\n\
	# to have high order taylor serie													\n\
	DST	R1.xy, R0.x, R0.x;				# R1= 1, a2										\n\
	MUL	R1.z, R1.y, R1.y;				# R1= 1, a2, a4									\n\
	MUL	R1.w, R1.y, R1.z;				# R1= 1, a2, a4, a6 (cos serie)					\n\
	MUL	R2, R1, R0.x;					# R2= a, a3, a5, a7 (sin serie)					\n\
	DP4 R0.x, R1, c[19];				# R0.x= cos(a)									\n\
	DP4 R0.y, R2, c[20];				# R0.y= sin(a)									\n\
";
const char* NL3D_BendProgramP1=
"																						\n\
	# build our quaternion																\n\
	# multiply the angleAxis by sin(a) / cos(a), where a is actually a/2				\n\
	# remind: c[16].z== angleAxis.z== 0													\n\
	MUL	R0, c[16], R0.yyyx;				# R0= quaternion.xyzw							\n\
																						\n\
	# build	our matrix from this quaternion, into R7,R8,R9								\n\
	# Quaternion TO matrix 3x3 in 7 ope, with quat.z==0									\n\
	MUL	R1, R0, c[8].w;					# R1= quat2= 2*quat == 2*x, 2*y, 0, 2*w			\n\
	MUL R2, R1, R0.x;					# R2= quatX= xx, xy, 0, wx						\n\
	MUL R3, R1, R0.y;					# R3= quatY= xy, yy, 0, wy						\n\
	# NB: c[21]= {0, 1, -1, 0}															\n\
	# Write to w, then w = 0, this avoid an unitialized component                       \n\
	MAD	R7.xyzw, c[21].zyyw, R3.yxww, c[21].yxxw;										\n\
	# R7.x= a11 = 1.0f - (yy)															\n\
	# R7.y= a12 = xy																	\n\
	# R7.z= a13 = wy																	\n\
	# NB: c[21]= {0, 1, -1, 0}															\n\
	# Write to w, then w = 0, this avoid an unitialized component                       \n\
	MAD	R8.xyzw, c[21].yzzw, R2.yxww, c[21].xyxw;										\n\
	# R8.x= a21 = xy																	\n\
	# R8.y= a22 = 1.0f - (xx)															\n\
	# R8.z= a23 = - wx																	\n\
	# NB: c[21]= {0, 1, -1, 0}															\n\
	# Write to w, then w = 0, this avoid an unitialized component                       \n\
	ADD	R9.xyzw, R2.zwxw, R3.wzyw;		# a31= 0+wy, a32= wx+0, a33= xx + yy, because z==0	\n\
	MAD R9.xyzw, R9.xyzw, c[21].zyzw, c[21].xxyw;										\n\
	# R9.x= a31 = - wy																	\n\
	# R9.y= a32 = wx																	\n\
	# R9.z= a33 = 1.0f - (xx + yy)														\n\
	# transform pos																		\n\
	DP3	R5.x, R7, v[0];																	\n\
	DP3	R5.y, R8, v[0];																	\n\
	DP3	R5.z, R9, v[0];				# R5= bended relative pos to center.				\n\
	#temp, to optimize																	\n\
	MOV R5.w, c[21].w;																    \n\
	# add pos to center pos.															\n\
	ADD	R5, R5, v[10];				# R5= world pos. R5.w= R5.w+v[10].w= 0+1= 1			\n\
	# make local to camera pos. Important for ZBuffer precision							\n\
	ADD R5, R5, -c[10];																	\n\
";


// Concat the 2 strings
const string NL3D_BendProgram= string(NL3D_BendProgramP0) + string(NL3D_BendProgramP1);



// ***********************
/*
	Lighted start program:
		bend pos and normal, normalize and lit
*/
// ***********************
// Common start program.
const char* NL3D_LightedStartVegetableProgram=
"																						\n\
	# bend Pos into R5. Now do it for normal											\n\
	DP3	R0.x, R7, v[2];																	\n\
	DP3	R0.y, R8, v[2];																	\n\
	DP3	R0.z, R9, v[2];				# R0= matRot * normal.								\n\
	# Do the rot 2 times for normal (works fine)										\n\
	DP3	R6.x, R7, R0;																	\n\
	DP3	R6.y, R8, R0;																	\n\
	DP3	R6.z, R9, R0;				# R6= bended normal.								\n\
																						\n\
	# Normalize normal, and dot product, into R0.x										\n\
	# w hasn't been written                                                             \n\
	DP3	R0.x, R6.xyzz, R6.xyzz;		# R0.x= R6.sqrnorm()								\n\
	RSQ R0.x, R0.x;					# R0.x= 1/norm()									\n\
	MUL	R6, R6.xyzz, R0.x;				# R6= R6.normed()								\n\
	DP3	R0.x, R6, c[9];																	\n\
																						\n\
	#FrontFacing																		\n\
	MAX	R0.y, -R0.x, c[8].x;		# R0.y= diffFactor= max(0, -R6*LightDir)			\n\
	MUL	R1.xyz, R0.y, v[3];			# R7= diffFactor*DiffuseColor						\n\
	ADD	o[COL0].xyz, R1, v[4];		# col0.RGB= AmbientColor + diffFactor*DiffuseColor	\n\
	MOV o[COL0].w, c[8].y;																\n\
";


// ***********************
/*
	Unlit start program:
		bend pos into R5, and copy color(s)
*/
// ***********************


// Unlit no alpha blend.
const char* NL3D_UnlitVegetableProgram=
"	MOV o[COL0].xyz, v[3];			# col.RGBA= vertex color							\n\
																						\n\
	MOV o[COL0].w, c[8].y;																\n\
";


// Unlit with AlphaBlend.
const char* NL3D_UnlitAlphaBlendVegetableProgram=
"	MOV o[COL0].xyz, v[3];			# col.RGBA= vertex color							\n\
																						\n\
	#Blend transition. NB: in R5, we already have the position relative to the camera	\n\
	DP3	R0.x, R5, R5;				# R0.x= sqr(dist to viewer).						\n\
	RSQ R0.y, R0.x;																		\n\
	MUL R0.x, R0.x, R0.y;			# R0.x= dist to viewer								\n\
	# setup alpha Blending. Distance of appartition is encoded in the vertex.			\n\
	MAD o[COL0].w, R0.x, c[11].x, v[9].w;												\n\
";



// ***********************
/*
	Common end of program: project, texture. Take pos from R5
*/
// ***********************
const char* NL3D_CommonEndVegetableProgram=
"	# compute in Projection space														\n\
	DP4 o[HPOS].x, c[0], R5;															\n\
	DP4 o[HPOS].y, c[1], R5;															\n\
	DP4 o[HPOS].z, c[2], R5;															\n\
	DP4 o[HPOS].w, c[3], R5;															\n\
	# copy Dynamic lightmap UV in stage0, from colors Alpha part.						\n\
	MAD o[TEX0].xzw, v[3].w, c[8].yxxx, c[8].xxxy;										\n\
	MOV o[TEX0].y, v[4].w;																\n\
	# copy diffuse texture uv to stage 1.												\n\
	MOV o[TEX1], v[8];																\n\
";

// fogged version
const char* NL3D_VegetableProgramFog =
"	DP4	o[FOGC].x, c[6], R5;															\n\
";


// ***********************
/*
	Speed test VP, No bend,no lighting.
*/
// ***********************
const char* NL3D_SimpleStartVegetableProgram=
"!!VP1.0																				\n\
	# compute in Projection space														\n\
	MAD	R5, v[0], c[8].yyyx, c[8].xxxy;													\n\
	ADD	R5.xyz, R5, v[10];																\n\
	# make local to camera pos															\n\
	ADD R5, R5, -c[10];																	\n\
	MOV o[COL0].xyz, v[3];			# col.RGBA= vertex color							\n\
";

class CVertexProgramVeget : public CVertexProgram
{
public:
	struct CIdx
	{
		// 0-3 modelViewProjection
		// 4
		// 5
		// 6 fog
		// 7
		uint ProgramConstants0; // 8
		uint DirectionalLight; // 9
		uint ViewCenter; // 10
		uint NegInvTransDist; // 11
		// 12
		// 13
		// 14
		// 15
		uint AngleAxis; // 16
		uint Wind; // 17
		uint CosCoeff0; // 18
		uint CosCoeff1; // 19
		uint CosCoeff2; // 20
		uint QuatConstants; // 21
		uint PiConstants; // 22
		uint LUTSize; // 23 (value = 64)
		uint LUT[NL3D_VEGETABLE_VP_LUT_SIZE]; // 32+
	};
	CVertexProgramVeget(uint vpType, bool fogEnabled)
	{
		// nelvp
		{
			CSource *source = new CSource();
			source->Profile = nelvp;
			source->DisplayName = "nelvp/Veget";
			
			// Init the Vertex Program.
			string	vpgram;
			// start always with Bend.
			if( vpType==NL3D_VEGETABLE_RDRPASS_LIGHTED || vpType==NL3D_VEGETABLE_RDRPASS_LIGHTED_2SIDED )
			{
				source->DisplayName += "/Bend";
				vpgram= NL3D_BendProgram;
			}
			else
			{
				source->DisplayName += "/FastBend";
				vpgram= NL3D_FastBendProgram;
			}

			// combine the VP according to Type
			switch(vpType)
			{
			case NL3D_VEGETABLE_RDRPASS_LIGHTED:
			case NL3D_VEGETABLE_RDRPASS_LIGHTED_2SIDED:
				source->DisplayName += "/Lighted";
				vpgram+= string(NL3D_LightedStartVegetableProgram);
				break;
			case NL3D_VEGETABLE_RDRPASS_UNLIT:
			case NL3D_VEGETABLE_RDRPASS_UNLIT_2SIDED:
				source->DisplayName += "/Unlit";
				vpgram+= string(NL3D_UnlitVegetableProgram);
				break;
			case NL3D_VEGETABLE_RDRPASS_UNLIT_2SIDED_ZSORT:
				source->DisplayName += "/UnlitAlphaBlend";
				vpgram+= string(NL3D_UnlitAlphaBlendVegetableProgram);
				break;
			}

			// common end of VP
			vpgram+= string(NL3D_CommonEndVegetableProgram);

			if (fogEnabled)
			{
				source->DisplayName += "/Fog";
				vpgram+= string(NL3D_VegetableProgramFog);
			}

			vpgram+="\nEND\n";

			source->setSource(vpgram);

			source->ParamIndices["modelViewProjection"] = 0;
			source->ParamIndices["fog"] = 6;
			source->ParamIndices["programConstants0"] = 8;
			source->ParamIndices["directionalLight"] = 9;
			source->ParamIndices["viewCenter"] = 10;
			source->ParamIndices["negInvTransDist"] = 11;
			source->ParamIndices["angleAxis"] = 16;
			source->ParamIndices["wind"] = 17;
			source->ParamIndices["cosCoeff0"] = 18;
			source->ParamIndices["cosCoeff1"] = 19;
			source->ParamIndices["cosCoeff2"] = 20;
			source->ParamIndices["quatConstants"] = 21;
			source->ParamIndices["piConstants"] = 22;
			source->ParamIndices["lutSize"] = 23;
			for (uint i = 0; i < NL3D_VEGETABLE_VP_LUT_SIZE; ++i)
			{
				source->ParamIndices[NLMISC::toString("lut[%i]", i)] = 32 + i;
			}

			addSource(source);
		}
		// TODO_VP_GLSL
	}
	virtual ~CVertexProgramVeget()
	{

	}
	virtual void buildInfo()
	{
		m_Idx.ProgramConstants0 = getUniformIndex("programConstants0");
		nlassert(m_Idx.ProgramConstants0 != ~0);
		m_Idx.DirectionalLight = getUniformIndex("directionalLight");
		nlassert(m_Idx.DirectionalLight != ~0);
		m_Idx.ViewCenter = getUniformIndex("viewCenter");
		nlassert(m_Idx.ViewCenter != ~0);
		m_Idx.NegInvTransDist = getUniformIndex("negInvTransDist");
		nlassert(m_Idx.NegInvTransDist != ~0);
		m_Idx.AngleAxis = getUniformIndex("angleAxis");
		nlassert(m_Idx.AngleAxis != ~0);
		m_Idx.Wind = getUniformIndex("wind");
		nlassert(m_Idx.Wind != ~0);
		m_Idx.CosCoeff0 = getUniformIndex("cosCoeff0");
		nlassert(m_Idx.CosCoeff0 != ~0);
		m_Idx.CosCoeff1 = getUniformIndex("cosCoeff1");
		nlassert(m_Idx.CosCoeff1 != ~0);
		m_Idx.CosCoeff2 = getUniformIndex("cosCoeff2");
		nlassert(m_Idx.CosCoeff2 != ~0);
		m_Idx.QuatConstants = getUniformIndex("quatConstants");
		nlassert(m_Idx.QuatConstants != ~0);
		m_Idx.PiConstants = getUniformIndex("piConstants");
		nlassert(m_Idx.PiConstants != ~0);
		m_Idx.LUTSize = getUniformIndex("lutSize");
		nlassert(m_Idx.LUTSize != ~0);
		for (uint i = 0; i < NL3D_VEGETABLE_VP_LUT_SIZE; ++i)
		{
			m_Idx.LUT[i] = getUniformIndex(NLMISC::toString("lut[%i]", i));
			nlassert(m_Idx.LUT[i] != ~0);
		}
	}
	const CIdx &idx() const { return m_Idx; }
private:
	CIdx m_Idx;
};

// ***************************************************************************
void					CVegetableManager::initVertexProgram(uint vpType, bool fogEnabled)
{
	nlassert(_LastDriver); // update driver should have been called at least once !
	
	// create VP.
	_VertexProgram[vpType][fogEnabled ? 1 : 0] = new CVertexProgramVeget(vpType, fogEnabled);
}


// ***************************************************************************
// ***************************************************************************
// Instanciation
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CVegetableClipBlock			*CVegetableManager::createClipBlock()
{
	// create a clipblock
	CVegetableClipBlock	*ret;
	ret= _ClipBlockMemory.allocate();

	// append to list.
	_EmptyClipBlockList.append(ret);

	return ret;
}

// ***************************************************************************
void						CVegetableManager::deleteClipBlock(CVegetableClipBlock *clipBlock)
{
	if(!clipBlock)
		return;

	// verify no more sortBlocks in this clipblock
	nlassert(clipBlock->_SortBlockList.size() == 0);

	// unlink from _EmptyClipBlockList, because _InstanceGroupList.size() == 0 ...
	_EmptyClipBlockList.remove(clipBlock);

	// delete
	_ClipBlockMemory.free(clipBlock);
}


// ***************************************************************************
CVegetableSortBlock			*CVegetableManager::createSortBlock(CVegetableClipBlock *clipBlock, const CVector &center, float radius)
{
	nlassert(clipBlock);

	// create a clipblock
	CVegetableSortBlock	*ret;
	ret= _SortBlockMemory.allocate();
	ret->_Owner= clipBlock;
	ret->_Center= center;
	ret->_Radius= radius;

	// append to list.
	clipBlock->_SortBlockList.append(ret);

	return ret;
}

// ***************************************************************************
void						CVegetableManager::deleteSortBlock(CVegetableSortBlock *sortBlock)
{
	if(!sortBlock)
		return;

	// verify no more IGs in this sortblock
	nlassert(sortBlock->_InstanceGroupList.size() == 0);

	// unlink from clipBlock
	sortBlock->_Owner->_SortBlockList.remove(sortBlock);

	// delete
	_SortBlockMemory.free(sortBlock);
}


// ***************************************************************************
CVegetableInstanceGroup		*CVegetableManager::createIg(CVegetableSortBlock *sortBlock)
{
	nlassert(sortBlock);
	CVegetableClipBlock		*clipBlock= sortBlock->_Owner;


	// create an IG
	CVegetableInstanceGroup	*ret;
	ret= _InstanceGroupMemory.allocate();
	ret->_SortOwner= sortBlock;
	ret->_ClipOwner= clipBlock;

	// if the clipBlock is empty, change list, because won't be no more.
	if(clipBlock->_NumIgs==0)
	{
		// remove from empty list
		_EmptyClipBlockList.remove(clipBlock);
		// and append to not empty one.
		_ClipBlockList.append(clipBlock);
	}

	// inc the number of igs appended to the clipBlock.
	clipBlock->_NumIgs++;

	// link ig to sortBlock.
	sortBlock->_InstanceGroupList.append(ret);

	// Special Init: The ZSort rdrPass must start with the same HardMode than SortBlock.
	ret->_RdrPass[NL3D_VEGETABLE_RDRPASS_UNLIT_2SIDED_ZSORT].HardMode= sortBlock->ZSortHardMode;

	return ret;
}

// ***************************************************************************
void						CVegetableManager::deleteIg(CVegetableInstanceGroup *ig)
{
	if(!ig)
		return;

	// update lighting mgt: no more vertices.
	// -----------
	// If I delete the ig which is the current root
	if(_ULRootIg == ig)
	{
		// switch to next
		_ULRootIg= ig->_ULNext;
		// if still the same, it means that the circular list is now empty
		if(_ULRootIg == ig)
			_ULRootIg= NULL;
		// Reset UL instance info.
		_ULCurrentIgRdrPass= 0;
		_ULCurrentIgInstance= 0;
	}
	// remove UL vertex count of the deleted ig
	_ULNTotalVertices-= ig->_ULNumVertices;
	// unlink the ig for lighting update.
	ig->unlinkUL();


	// For all render pass of this instance, delete his vertices
	// -----------
	for(sint rdrPass=0; rdrPass < NL3D_VEGETABLE_NRDRPASS; rdrPass++)
	{
		// rdrPass
		CVegetableInstanceGroup::CVegetableRdrPass	&vegetRdrPass= ig->_RdrPass[rdrPass];
		// which allocator?
		CVegetableVBAllocator	&vbAllocator= getVBAllocatorForRdrPassAndVBHardMode(rdrPass, vegetRdrPass.HardMode);

		// For all vertices of this rdrPass, delete it
		sint	numVertices;
		numVertices= vegetRdrPass.Vertices.size();
		// all vertices must have been setuped.
		nlassert((uint)numVertices == vegetRdrPass.NVertices);
		for(sint i=0; i<numVertices;i++)
		{
			vbAllocator.deleteVertex(vegetRdrPass.Vertices[i]);
		}
		vegetRdrPass.Vertices.clear();
	}

	CVegetableClipBlock		*clipBlock= ig->_ClipOwner;
	CVegetableSortBlock		*sortBlock= ig->_SortOwner;

	// If I have got some faces in ZSort rdrPass
	if(ig->_HasZSortPassInstances)
		// after my deletion, the sortBlock must be updated.
		sortBlock->_Dirty= true;


	// unlink from sortBlock, and delete.
	sortBlock->_InstanceGroupList.remove(ig);
	_InstanceGroupMemory.free(ig);


	// decRef the clipBlock
	clipBlock->_NumIgs--;
	// if the clipBlock is now empty, change list
	if(clipBlock->_NumIgs==0)
	{
		// remove from normal list
		_ClipBlockList.remove(clipBlock);
		// and append to empty list.
		_EmptyClipBlockList.append(clipBlock);
	}

}


// ***************************************************************************
CVegetableShape				*CVegetableManager::getVegetableShape(const std::string &shape)
{
	ItShapeMap	it= _ShapeMap.find(shape);
	// if found
	if(it != _ShapeMap.end())
		return &it->second;
	// else insert
	{
		// insert.
		CVegetableShape		*ret;
		it= ( _ShapeMap.insert(make_pair(shape, CVegetableShape()) ) ).first;
		ret= &it->second;

		// fill.
		try
		{
			if( !ret->loadShape(shape) )
			{
				// Warning
				nlwarning ("CVegetableManager::getVegetableShape could not load shape file '%s'", shape.c_str ());

				// Remove from map
				_ShapeMap.erase (shape);

				// Return NULL
				ret = NULL;
			}
		}
		catch (const Exception &e)
		{
			// Warning
			nlwarning ("CVegetableManager::getVegetableShape error while loading shape file '%s' : '%s'", shape.c_str (), e.what ());

			// Remove from map
			_ShapeMap.erase (shape);

			// Return NULL
			ret = NULL;
		}

		return ret;
	}
}


// ***************************************************************************
uint			CVegetableManager::getRdrPassInfoForShape(CVegetableShape *shape, TVegetableWater vegetWaterState,
	bool &instanceLighted, bool &instanceDoubleSided, bool &instanceZSort,
	bool &destLighted, bool &precomputeLighting)
{
	instanceLighted= shape->Lighted;
	instanceDoubleSided= shape->DoubleSided;
	// Disable ZSorting when we intersect water.
	instanceZSort= shape->AlphaBlend && vegetWaterState!=IntersectWater;
	destLighted= instanceLighted && !shape->PreComputeLighting;
	precomputeLighting= instanceLighted && shape->PreComputeLighting;

	// get correct rdrPass
	uint	rdrPass;
	// get according to lighted / doubleSided state
	if(destLighted)
	{
		if(instanceDoubleSided)
			rdrPass= NL3D_VEGETABLE_RDRPASS_LIGHTED_2SIDED;
		else
			rdrPass= NL3D_VEGETABLE_RDRPASS_LIGHTED;
	}
	else
	{
		if(instanceDoubleSided)
		{
			if(instanceZSort)
				rdrPass= NL3D_VEGETABLE_RDRPASS_UNLIT_2SIDED_ZSORT;
			else
				rdrPass= NL3D_VEGETABLE_RDRPASS_UNLIT_2SIDED;
		}
		else
			rdrPass= NL3D_VEGETABLE_RDRPASS_UNLIT;
	}

	return rdrPass;
}


// ***************************************************************************
void			CVegetableManager::reserveIgAddInstances(CVegetableInstanceGroupReserve &vegetIgReserve, CVegetableShape *shape, TVegetableWater vegetWaterState, uint numInstances)
{
	bool	instanceLighted;
	bool	instanceDoubleSided;
	bool	instanceZSort;
	bool	destLighted;
	bool	precomputeLighting;

	// get correct rdrPass / info
	uint	rdrPass;
	rdrPass= getRdrPassInfoForShape(shape, vegetWaterState, instanceLighted, instanceDoubleSided,
		instanceZSort, destLighted, precomputeLighting);

	// veget rdrPass
	CVegetableInstanceGroupReserve::CVegetableRdrPass	&vegetRdrPass= vegetIgReserve._RdrPass[rdrPass];

	// Reserve space in the rdrPass.
	vegetRdrPass.NVertices+= numInstances * shape->VB.getNumVertices();
	vegetRdrPass.NTriangles+= numInstances * (uint)shape->TriangleIndices.size()/3;
	// if the instances are lighted, reserve space for lighting updates
	if(instanceLighted)
		vegetRdrPass.NLightedInstances+= numInstances;
}


// ***************************************************************************
void			CVegetableManager::reserveIgCompile(CVegetableInstanceGroup *ig, const CVegetableInstanceGroupReserve &vegetIgReserve)
{
	uint	rdrPass;


	// Check.
	//===========
	// For all rdrPass of the ig, check empty
	for(rdrPass= 0; rdrPass<NL3D_VEGETABLE_NRDRPASS; rdrPass++)
	{
		CVegetableInstanceGroup::CVegetableRdrPass	&vegetRdrPass= ig->_RdrPass[rdrPass];
		nlassert(vegetRdrPass.TriangleIndices.getNumIndexes()==0);
		nlassert(vegetRdrPass.TriangleLocalIndices.size()==0);
		nlassert(vegetRdrPass.Vertices.size()==0);
		nlassert(vegetRdrPass.LightedInstances.size()==0);
	}
	// Do the same for all quadrants of the zsort rdrPass.
	nlassert(ig->_TriangleQuadrantOrderArray.size()==0);
	nlassert(ig->_TriangleQuadrantOrderNumTriangles==0);


	// Reserve.
	//===========
	// For all rdrPass of the ig, reserve.
	for(rdrPass= 0; rdrPass<NL3D_VEGETABLE_NRDRPASS; rdrPass++)
	{
		CVegetableInstanceGroup::CVegetableRdrPass	&vegetRdrPass= ig->_RdrPass[rdrPass];
		uint	numVertices= vegetIgReserve._RdrPass[rdrPass].NVertices;
		uint	numTris= vegetIgReserve._RdrPass[rdrPass].NTriangles;
		uint	numLightedInstances= vegetIgReserve._RdrPass[rdrPass].NLightedInstances;
		// reserve triangles indices and vertices for this rdrPass.
		vegetRdrPass.TriangleIndices.setFormat(vegetRdrPass.HardMode ? CIndexBuffer::Indices16 : CIndexBuffer::Indices32);
		vegetRdrPass.TriangleIndices.setNumIndexes(numTris*3);
		vegetRdrPass.TriangleLocalIndices.resize(numTris*3);
		vegetRdrPass.Vertices.resize(numVertices);
		// reserve ligthedinstances space.
		vegetRdrPass.LightedInstances.resize(numLightedInstances);
	}

	// Reserve space for the zsort rdrPass sorting.
	uint	numZSortTris= vegetIgReserve._RdrPass[NL3D_VEGETABLE_RDRPASS_UNLIT_2SIDED_ZSORT].NTriangles;
	// allocate sufficient space for all quadrants (1 alloc for all quadrants).
	ig->_TriangleQuadrantOrderArray.resize(numZSortTris * NL3D_VEGETABLE_NUM_QUADRANT);

	// And init ptrs.
	if(numZSortTris>0)
	{
		sint16	*start= ig->_TriangleQuadrantOrderArray.getPtr();
		// init ptr to each qaudrant
		for(uint i=0; i<NL3D_VEGETABLE_NUM_QUADRANT; i++)
		{
			ig->_TriangleQuadrantOrders[i]= start + i*numZSortTris;
		}
	}
}


// ***************************************************************************
inline void		computeVegetVertexLighting(const CVector &rotNormal,
	const CVector &sunDir, CRGBA primaryRGBA, CRGBA secondaryRGBA,
	CVegetableLightEx &vegetLex, CRGBA diffusePL[2], CRGBA *dst)
{
	float	dpSun;
	float	dpPL[2];
	CRGBA	col;
	CRGBA	resColor;


	// compute front-facing coloring.
	{
		// Compute Sun Light.
		dpSun= rotNormal*sunDir;
		float	f= max(0.f, -dpSun);
		col.modulateFromuiRGBOnly(primaryRGBA, NLMISC::OptFastFloor(f*256));
		// Add it with ambient
		resColor.addRGBOnly(col, secondaryRGBA);

		// Add influence of 2 lights only. (unrolled for better BTB use)
		// Compute Light 0 ?
		if(vegetLex.NumLights>=1)
		{
			dpPL[0]= rotNormal*vegetLex.Direction[0];
			f= max(0.f, -dpPL[0]);
			col.modulateFromuiRGBOnly(diffusePL[0], NLMISC::OptFastFloor(f*256));
			resColor.addRGBOnly(col, resColor);
			// Compute Light 1 ?
			if(vegetLex.NumLights>=2)
			{
				dpPL[1]= rotNormal*vegetLex.Direction[1];
				f= max(0.f, -dpPL[1]);
				col.modulateFromuiRGBOnly(diffusePL[1], NLMISC::OptFastFloor(f*256));
				resColor.addRGBOnly(col, resColor);
			}
		}

		// Keep correct U of Dynamic Lightmap UV encoded in primaryRGBA Alpha part.
		resColor.A= primaryRGBA.A;

		// copy to dest
		*dst= resColor;
	}
}


// ***************************************************************************
inline void		computeVegetVertexLightingForceBestSided(const CVector &rotNormal,
	const CVector &sunDir, CRGBA primaryRGBA, CRGBA secondaryRGBA,
	CVegetableLightEx &vegetLex, CRGBA diffusePL[2], CRGBA *dst)
{
	float	dpSun;
	float	dpPL[2];
	CRGBA	col;
	CRGBA	resColor;


	// compute best-facing coloring.
	{
		// Compute Sun Light.
		dpSun= rotNormal*sunDir;
		// ForceBestSided: take the absolute value (max of -val,val)
		float	f= (float)fabs(dpSun);
		col.modulateFromuiRGBOnly(primaryRGBA, NLMISC::OptFastFloor(f*256));
		// Add it with ambient
		resColor.addRGBOnly(col, secondaryRGBA);

		// Add influence of 2 lights only. (unrolled for better BTB use)
		// Compute Light 0 ?
		if(vegetLex.NumLights>=1)
		{
			dpPL[0]= rotNormal*vegetLex.Direction[0];
			// ForceBestSided: take the absolute value (max of -val,val)
			f= (float)fabs(dpPL[0]);
			col.modulateFromuiRGBOnly(diffusePL[0], NLMISC::OptFastFloor(f*256));
			resColor.addRGBOnly(col, resColor);
			// Compute Light 1 ?
			if(vegetLex.NumLights>=2)
			{
				dpPL[1]= rotNormal*vegetLex.Direction[1];
				f= (float)fabs(dpPL[1]);
				col.modulateFromuiRGBOnly(diffusePL[1], NLMISC::OptFastFloor(f*256));
				resColor.addRGBOnly(col, resColor);
			}
		}

		// Keep correct U of Dynamic Lightmap UV encoded in primaryRGBA Alpha part.
		resColor.A= primaryRGBA.A;

		// copy to dest
		*dst= resColor;
	}

}


// ***************************************************************************
void			CVegetableManager::addInstance(CVegetableInstanceGroup *ig,
		CVegetableShape	*shape, const NLMISC::CMatrix &mat,
		const NLMISC::CRGBAF &ambientColor, const NLMISC::CRGBAF &diffuseColor,
		float	bendFactor, float bendPhase, float bendFreqFactor, float blendDistMax,
		TVegetableWater vegetWaterState, CVegetableUV8 dlmUV)
{
	sint	i;


	// Some setup.
	//--------------------
	bool	instanceLighted;
	bool	instanceDoubleSided;
	bool	instanceZSort;
	bool	destLighted;
	bool	precomputeLighting;

	// get correct rdrPass / info
	uint	rdrPass;
	rdrPass= getRdrPassInfoForShape(shape, vegetWaterState, instanceLighted, instanceDoubleSided,
		instanceZSort, destLighted, precomputeLighting);
	// bestSided Precompute lighting or not??
	bool	bestSidedPrecomputeLighting= precomputeLighting && shape->BestSidedPreComputeLighting;


	// veget rdrPass
	CVegetableInstanceGroup::CVegetableRdrPass	&vegetRdrPass= ig->_RdrPass[rdrPass];

	// color.
	// setup using OptFastFloor.
	CRGBA		ambientRGBA, diffuseRGBA;
	CRGBA		primaryRGBA, secondaryRGBA;
	// diffuseColor
	diffuseRGBA.R= (uint8)NLMISC::OptFastFloor(diffuseColor.R*255);
	diffuseRGBA.G= (uint8)NLMISC::OptFastFloor(diffuseColor.G*255);
	diffuseRGBA.B= (uint8)NLMISC::OptFastFloor(diffuseColor.B*255);
	diffuseRGBA.A= 255;
	// ambientColor
	ambientRGBA.R= (uint8)NLMISC::OptFastFloor(ambientColor.R*255);
	ambientRGBA.G= (uint8)NLMISC::OptFastFloor(ambientColor.G*255);
	ambientRGBA.B= (uint8)NLMISC::OptFastFloor(ambientColor.B*255);
	ambientRGBA.A= 255;

	// For Lighted, modulate with global light.
	if(instanceLighted)
	{
		primaryRGBA.modulateFromColorRGBOnly(diffuseRGBA, _GlobalDiffuse);
		secondaryRGBA.modulateFromColorRGBOnly(ambientRGBA, _GlobalAmbient);
	}
	// if the instance is not lighted, then don't take care of lighting
	else
	{
		primaryRGBA.R= diffuseRGBA.R;
		primaryRGBA.G= diffuseRGBA.G;
		primaryRGBA.B= diffuseRGBA.B;
		// may not be useful (2Sided lighting no more supported)
		secondaryRGBA= primaryRGBA;
	}

	// Copy Dynamic Lightmap UV in Alpha part (save memory for an extra cost of 1 VP instruction)
	primaryRGBA.A= dlmUV.U;
	secondaryRGBA.A= dlmUV.V;

	// get ref on the vegetLex.
	CVegetableLightEx	&vegetLex= ig->VegetableLightEx;
	// Color of pointLights modulated by diffuse.
	CRGBA	diffusePL[2];
	diffusePL[0] = CRGBA::Black;
	diffusePL[1] = CRGBA::Black;
	if(vegetLex.NumLights>=1)
	{
		diffusePL[0].modulateFromColorRGBOnly(diffuseRGBA, vegetLex.Color[0]);
		if(vegetLex.NumLights>=2)
		{
			diffusePL[1].modulateFromColorRGBOnly(diffuseRGBA, vegetLex.Color[1]);
		}
	}

	// normalize bendFreqFactor
	bendFreqFactor*= NL3D_VEGETABLE_FREQUENCY_FACTOR_PREC;
	bendFreqFactor= (float)floor(bendFreqFactor + 0.5f);
	bendFreqFactor/= NL3D_VEGETABLE_FREQUENCY_FACTOR_PREC;


	// Get allocator, and manage VBhard overriding.
	//--------------------
	CVegetableVBAllocator	*allocator;
	// if still in Sfot mode, keep it.
	if(!vegetRdrPass.HardMode)
	{
		// get the soft allocator.
		allocator= &getVBAllocatorForRdrPassAndVBHardMode(rdrPass, 0);
	}
	else
	{
		// Get VB allocator Hard for this rdrPass
		allocator= &getVBAllocatorForRdrPassAndVBHardMode(rdrPass, 1);
		// Test if the instance don't add too many vertices for this VBHard
		if(allocator->exceedMaxVertexInBufferHard(shape->VB.getNumVertices()))
		{
			// if exceed, then must pass ALL the IG in software mode. vertices/faces are correclty updated.
			// special: if rdrPass is the ZSort one,
			if(rdrPass == NL3D_VEGETABLE_RDRPASS_UNLIT_2SIDED_ZSORT)
			{
				nlassert(ig->_SortOwner->ZSortHardMode);

				// must do it on ALL igs of the sortBlock, for less VBuffer mode switching.
				CVegetableInstanceGroup		*pIg= ig->_SortOwner->_InstanceGroupList.begin();
				while(pIg)
				{
					// let's pass them in software mode.
					swapIgRdrPassHardMode(pIg, rdrPass);
					// next
					pIg= (CVegetableInstanceGroup*)pIg->Next;
				}

				// Then all The sortBlock is in SoftMode.
				ig->_SortOwner->ZSortHardMode= false;
			}
			else
			{
				// just do it on this Ig (can mix hardMode in a SortBlock for normal rdrPass)
				swapIgRdrPassHardMode(ig, rdrPass);
			}

			// now, we can use the software only Allocator to append our instance
			allocator= &getVBAllocatorForRdrPassAndVBHardMode(rdrPass, 0);
		}
	}


	// get correct dstVB
	const CVertexBuffer	&dstVBInfo= allocator->getSoftwareVertexBuffer();


	// Transform vertices to a vegetable instance, and enlarge clipBlock
	//--------------------
	// compute matrix to multiply normals, ie (M-1)t
	CMatrix		normalMat;
	// need just rotation scale matrix.
	normalMat.setRot(mat);
	normalMat.invert();
	normalMat.transpose();
	// compute Instance position
	CVector		instancePos;
	mat.getPos(instancePos);


	// At least, the bbox of the clipBlock must include the center of the shape.
	ig->_ClipOwner->extendSphere(instancePos);


	// Vertex/triangle Info.
	uint	numNewVertices= shape->VB.getNumVertices();
	uint	numNewTris= (uint)shape->TriangleIndices.size()/3;
	uint	numNewIndices= (uint)shape->TriangleIndices.size();

	// src info.
	uint	srcNormalOff= (instanceLighted? shape->VB.getNormalOff() : 0);
	uint	srcTex0Off= shape->VB.getTexCoordOff(0);
	uint	srcTex1Off= shape->VB.getTexCoordOff(1);

	// dst info
	uint	dstNormalOff= (destLighted? dstVBInfo.getValueOffEx(NL3D_VEGETABLE_VPPOS_NORMAL) : 0);
	uint	dstColor0Off= dstVBInfo.getValueOffEx(NL3D_VEGETABLE_VPPOS_COLOR0);
	uint	dstColor1Off= dstVBInfo.getValueOffEx(NL3D_VEGETABLE_VPPOS_COLOR1);
	uint	dstTex0Off= dstVBInfo.getValueOffEx(NL3D_VEGETABLE_VPPOS_TEX0);
	uint	dstBendOff= dstVBInfo.getValueOffEx(NL3D_VEGETABLE_VPPOS_BENDINFO);
	uint	dstCenterOff= dstVBInfo.getValueOffEx(NL3D_VEGETABLE_VPPOS_CENTER);

	// For D3D, If the VertexBuffer is in BGRA mode
	if(allocator->isBGRA())
	{
		// then swap only the B and R (no cpu cycle added per vertex)
		primaryRGBA.swapBR();
		secondaryRGBA.swapBR();
		diffusePL[0].swapBR();
		diffusePL[1].swapBR();
	}

	// Useful for !destLighted only.
	CVector		deltaPos;
	float		deltaPosNorm=0.0;


	// Useful for ZSORT rdrPass, the worldVertices.
	static	vector<CVector>		worldVertices;
	if(rdrPass == NL3D_VEGETABLE_RDRPASS_UNLIT_2SIDED_ZSORT)
	{
		worldVertices.resize(numNewVertices);
	}

	CVertexBufferRead vba;
	shape->VB.lock (vba);

	// For all vertices of shape, transform and store manager indices in temp shape.
	for(i=0; i<(sint)numNewVertices;i++)
	{
		// allocate a Vertex
		uint	vid= allocator->allocateVertex();

		CVertexBufferReadWrite vbaOut;
		allocator->getSoftwareVertexBuffer ().lock(vbaOut);

		// store in tmp shape.
		shape->InstanceVertices[i]= vid;

		// Fill this vertex.
		const uint8	*srcPtr= (uint8*)vba.getVertexCoordPointer(i);
		uint8	*dstPtr= (uint8*)vbaOut.getVertexCoordPointer(vid);

		// Get bendWeight for this vertex.
		float	vertexBendWeight= ((CUV*)(srcPtr + srcTex1Off))->U * bendFactor;

		// Pos.
		//-------
		// Separate Center and relative pos.
		CVector	relPos= mat.mulVector(*(CVector*)srcPtr);	// mulVector, because translation in v[center]
		// compute bendCenterPos
		CVector	bendCenterPos;
		if(shape->BendCenterMode == CVegetableShapeBuild::BendCenterNull)
			bendCenterPos= CVector::Null;
		else
		{
			CVector	v= *(CVector*)srcPtr;
			v.z= 0;
			bendCenterPos= mat.mulVector(v);				// mulVector, because translation in v[center]
		}
		// copy
		deltaPos= relPos-bendCenterPos;
		*(CVector*)dstPtr= deltaPos;
		*(CVector*)(dstPtr + dstCenterOff)= instancePos + bendCenterPos;
		// if !destLighted, then VP is different
		if(!destLighted)
		{
			deltaPosNorm= deltaPos.norm();
			// copy bendWeight in v.w
			CVectorH	*vh= (CVectorH*)dstPtr;
			// Mul by deltaPosNorm, to draw an arc circle.
			vh->w= vertexBendWeight * deltaPosNorm;
		}

		// Enlarge the clipBlock of the IG.
		// Since small shape, enlarge with each vertices. simpler and maybe faster.
		// TODO_VEGET: bend and clipping ...
		ig->_ClipOwner->extendBBoxOnly(instancePos + relPos);

		// prepare for ZSort
		if(rdrPass == NL3D_VEGETABLE_RDRPASS_UNLIT_2SIDED_ZSORT)
		{
			worldVertices[i]= instancePos + relPos;
		}


		// Color-ligthing.
		//-------
		if(!precomputeLighting)
		{
			// just copy the primary color (means diffuse part if lighted)
			*(CRGBA*)(dstPtr + dstColor0Off)= primaryRGBA;
			// normal and secondary color
			if(destLighted)
			{
				// normal
				*(CVector*)(dstPtr + dstNormalOff)= normalMat.mulVector( *(CVector*)(srcPtr + srcNormalOff) );
			}
			// If destLighted, secondaryRGBA is the ambient
			// else secondaryRGBA is used only for Alpha (DLM uv.v).
			*(CRGBA*)(dstPtr + dstColor1Off)= secondaryRGBA;
		}
		else
		{
			nlassert(!destLighted);

			// compute normal.
			CVector		rotNormal= normalMat.mulVector( *(CVector*)(srcPtr + srcNormalOff) );
			// must normalize() because scale is possible.
			rotNormal.normalize();

			// Do the compute.
			if(!bestSidedPrecomputeLighting)
			{
				computeVegetVertexLighting(rotNormal,
					_DirectionalLight, primaryRGBA, secondaryRGBA,
					vegetLex, diffusePL, (CRGBA*)(dstPtr + dstColor0Off) );
			}
			else
			{
				computeVegetVertexLightingForceBestSided(rotNormal,
					_DirectionalLight, primaryRGBA, secondaryRGBA,
					vegetLex, diffusePL, (CRGBA*)(dstPtr + dstColor0Off) );
			}

			// copy secondaryRGBA, used only for Alpha (DLM uv.v).
			*(CRGBA*)(dstPtr + dstColor1Off)= secondaryRGBA;
		}


		// Texture.
		//-------
		*(CUV*)(dstPtr + dstTex0Off)= *(CUV*)(srcPtr + srcTex0Off);

		// Bend.
		//-------
		CVector		*dstBendPtr= (CVector*)(dstPtr + dstBendOff);
		// setup bend Phase.
		dstBendPtr->y= bendPhase;
		// setup bend Weight.
		// if !destLighted, then VP is different, vertexBendWeight is stored in v[0].w
		if(destLighted)
			dstBendPtr->x= vertexBendWeight;
		else
			// the VP need the norm of relPos in v[9].x
			dstBendPtr->x= deltaPosNorm;
		// setup bendFreqFactor
		dstBendPtr->z= bendFreqFactor;
		/// If AlphaBlend / ZSort rdrPass, then setup AlphaBlend computing.
		if(rdrPass == NL3D_VEGETABLE_RDRPASS_UNLIT_2SIDED_ZSORT)
		{
			// get ptr on v[9].w NB: in Unlit mode, it has 4 components.
			CVectorH		*dstBendPtr= (CVectorH*)(dstPtr + dstBendOff);
			// setup the constant of linear formula:
			// Alpha= -1/blendTransDist * dist + blendDistMax/blendTransDist
			dstBendPtr->w= blendDistMax/NL3D_VEGETABLE_BLOCK_BLEND_TRANSITION_DIST;
		}


		// fill the vertex in AGP.
		//-------
		allocator->flushVertex(vid);
	}


	// must recompute the sphere according to the bbox.
	ig->_ClipOwner->updateSphere();


	// If ZSort, compute Triangle Centers and Orders for quadrant
	//--------------------
	if(rdrPass==NL3D_VEGETABLE_RDRPASS_UNLIT_2SIDED_ZSORT)
	{
		// inform the SB that it must be updated.
		ig->_SortOwner->_Dirty= true;
		// For deletion, inform the ig that it has instances which impact the SB.
		ig->_HasZSortPassInstances= true;

		// change UnderWater falg of the SB
		if(vegetWaterState == AboveWater)
			ig->_SortOwner->_UnderWater= false;
		else if(vegetWaterState == UnderWater)
			ig->_SortOwner->_UnderWater= true;

		// static to avoid reallocation
		static	vector<CVector>		triangleCenters;
		triangleCenters.resize(numNewTris);

		// compute triangle centers
		for(uint i=0; i<numNewTris; i++)
		{
			// get index in shape.
			uint	v0= shape->TriangleIndices[i*3+0];
			uint	v1= shape->TriangleIndices[i*3+1];
			uint	v2= shape->TriangleIndices[i*3+2];

			// get world coord.
			const CVector	&vert0= worldVertices[v0];
			const CVector	&vert1= worldVertices[v1];
			const CVector	&vert2= worldVertices[v2];

			// compute center
			triangleCenters[i]= (vert0 + vert1 + vert2) / 3;
			// relative to center of the sortBlock (for sint16 compression)
			triangleCenters[i]-= ig->_SortOwner->_Center;
		}


		// resize the array. Actually only modify the number of triangles really setuped.
		uint	offTri= ig->_TriangleQuadrantOrderNumTriangles;
		ig->_TriangleQuadrantOrderNumTriangles+= numNewTris;
		// verify user has correclty used reserveIg system.
		nlassert(ig->_TriangleQuadrantOrderNumTriangles * NL3D_VEGETABLE_NUM_QUADRANT <= ig->_TriangleQuadrantOrderArray.size());


		// compute distance for each quadrant. Since we are not sure of the sortBlockSize, mul with a (big: 16) security.
		// NB: for landscape practical usage, this left us with more than 1mm precision.
		float	distFactor=32768/(16*ig->_SortOwner->_Radius);
		for(uint quadId=0; quadId<NL3D_VEGETABLE_NUM_QUADRANT; quadId++)
		{
			const CVector		&quadDir= CVegetableQuadrant::Dirs[quadId];

			// For all tris.
			for(uint i=0; i<numNewTris; i++)
			{
				// compute the distance with orientation of the quadrant. (DotProduct)
				float	dist= triangleCenters[i] * quadDir;
				// compress to sint16.
				ig->_TriangleQuadrantOrders[quadId][offTri + i]= (sint16)NLMISC::OptFastFloor(dist*distFactor);
			}
		}
	}


	// Append list of indices and list of triangles to the IG
	//--------------------

	// TODO_VEGET_OPTIM: system reallocation of array is very bad...


	// compute dest start idx.
	uint	offVertex= vegetRdrPass.NVertices;
	uint	offTri= vegetRdrPass.NTriangles;
	uint	offTriIdx= offTri*3;

	// verify user has correclty used reserveIg system.
	nlassert(offVertex + numNewVertices <= vegetRdrPass.Vertices.size());
	nlassert(offTriIdx + numNewIndices <= vegetRdrPass.TriangleIndices.getNumIndexes());
	nlassert(offTriIdx + numNewIndices <= vegetRdrPass.TriangleLocalIndices.size());


	// insert list of vertices to delete in ig vertices.
	vegetRdrPass.Vertices.copy(offVertex, offVertex+numNewVertices, &shape->InstanceVertices[0]);

	// insert array of triangles in ig.
	// for all indices, fill IG
	CIndexBufferReadWrite ibaWrite;
	vegetRdrPass.TriangleIndices.lock (ibaWrite);
	if (vegetRdrPass.TriangleIndices.getFormat() == CIndexBuffer::Indices16)
	{
		uint16 *ptr = (uint16 *) ibaWrite.getPtr();
		for(i=0; i<(sint)numNewIndices; i++)
		{
			// get the index of the vertex in the shape
			uint	vid= shape->TriangleIndices[i];
			// re-direction, using InstanceVertices;
			#ifdef NL_DEBUG
				nlassert(shape->InstanceVertices[vid] <= 0xffff);
			#endif
			ptr[offTriIdx + i]= (uint16) shape->InstanceVertices[vid];
			// local re-direction: adding vertexOffset.
			vegetRdrPass.TriangleLocalIndices[offTriIdx + i]= offVertex + vid;
		}
	}
	else
	{
		uint32 *ptr = (uint32 *) ibaWrite.getPtr();
		for(i=0; i<(sint)numNewIndices; i++)
		{
			// get the index of the vertex in the shape
			uint	vid= shape->TriangleIndices[i];
			// re-direction, using InstanceVertices;
			ptr[offTriIdx + i]= shape->InstanceVertices[vid];
			// local re-direction: adding vertexOffset.
			vegetRdrPass.TriangleLocalIndices[offTriIdx + i]= offVertex + vid;
		}
	}

	// new triangle and vertex size.
	vegetRdrPass.NTriangles+= numNewTris;
	vegetRdrPass.NVertices+= numNewVertices;


	// if lighted, must add a lightedInstance for lighting update.
	//--------------------
	if(instanceLighted)
	{
		// first, update Ig.
		ig->_ULNumVertices+= numNewVertices;
		// and update the vegetable manager.
		_ULNTotalVertices+= numNewVertices;
		// link at the end of the circular list: link before the current root.
		if(_ULRootIg==NULL)
			_ULRootIg= ig;
		else
			ig->linkBeforeUL(_ULRootIg);

		// check good use of reserveIg.
		nlassert(vegetRdrPass.NLightedInstances < vegetRdrPass.LightedInstances.size());

		// Fill instance info
		CVegetableInstanceGroup::CVegetableLightedInstance	&vli=
			vegetRdrPass.LightedInstances[vegetRdrPass.NLightedInstances];
		vli.Shape= shape;
		vli.NormalMat= normalMat;
		// copy colors unmodulated by global light.
		vli.MatAmbient= ambientRGBA;
		vli.MatDiffuse= diffuseRGBA;
		// store dynamic lightmap UV
		vli.DlmUV= dlmUV;
		// where vertices of this instances are wrote in the VegetRdrPass
		vli.StartIdInRdrPass= offVertex;

		// Inc size setuped.
		vegetRdrPass.NLightedInstances++;
	}

}


// ***************************************************************************
void			CVegetableManager::swapIgRdrPassHardMode(CVegetableInstanceGroup *ig, uint rdrPass)
{
	CVegetableInstanceGroup::CVegetableRdrPass	&vegetRdrPass= ig->_RdrPass[rdrPass];

	// the allocator where vertices come from
	CVegetableVBAllocator	&srcAllocator= getVBAllocatorForRdrPassAndVBHardMode(rdrPass, vegetRdrPass.HardMode);
	// the allocator where vertices will go
	CVegetableVBAllocator	&dstAllocator= getVBAllocatorForRdrPassAndVBHardMode(rdrPass, !vegetRdrPass.HardMode);

	// vertex size
	uint	vbSize= srcAllocator.getSoftwareVertexBuffer().getVertexSize();
	nlassert(vbSize == dstAllocator.getSoftwareVertexBuffer().getVertexSize());

	CVertexBufferRead vbaIn;
	srcAllocator.getSoftwareVertexBuffer ().lock(vbaIn);

	// for all vertices of the IG, change of VBAllocator
	uint i;
	// Do it only for current Vertices setuped!!! because a swapIgRdrPassHardMode awlays arise when the ig is
	// in construcion.
	// Hence here, we may have vegetRdrPass.NVertices < vegetRdrPass.Vertices.size() !!!
	for(i=0;i<vegetRdrPass.NVertices;i++)
	{
		// get idx in src allocator.
		uint	srcId= vegetRdrPass.Vertices[i];
		// allocate a vertex in the dst allocator.
		uint	dstId= dstAllocator.allocateVertex();

		CVertexBufferReadWrite vbaOut;
		dstAllocator.getSoftwareVertexBuffer ().lock(vbaOut);

		// copy from VBsoft of src to dst.
		const void	*vbSrc= vbaIn.getVertexCoordPointer(srcId);
		void	*vbDst= vbaOut.getVertexCoordPointer(dstId);
		memcpy(vbDst, vbSrc, vbSize);
		// release src vertex.
		srcAllocator.deleteVertex(srcId);

		// and copy new dest id in Vertices array.
		vegetRdrPass.Vertices[i]= dstId;

		// and flush this vertex into VBHard (if dst is aVBHard).
		dstAllocator.flushVertex(dstId);
	}

	// For all triangles, bind correct triangles.
	nlassert(vegetRdrPass.TriangleIndices.getNumIndexes() == vegetRdrPass.TriangleLocalIndices.size());
	// Do it only for current Triangles setuped!!! same reason as vertices
	// For all setuped triangles indices
	CIndexBufferReadWrite ibaWrite;
	// For hard mode, uses faster 16 bit indices because the VB is not bigger than 65K
	vegetRdrPass.TriangleIndices.setFormat(vegetRdrPass.HardMode ? CIndexBuffer::Indices32 : CIndexBuffer::Indices16); // NB : this is not an error here : vegetRdrPass.HardMode has not been inverted yet
	vegetRdrPass.TriangleIndices.lock (ibaWrite);
	if (ibaWrite.getFormat() == CIndexBuffer::Indices16)
	{
		uint16 *ptr = (uint16 *) ibaWrite.getPtr();
		for(i=0;i<vegetRdrPass.NTriangles*3;i++)
		{
			// get the index in Vertices.
			uint	localVid= vegetRdrPass.TriangleLocalIndices[i];
			// get the index in new VBufffer (dstAllocator), and copy to TriangleIndices
			ptr[i]= (uint16) vegetRdrPass.Vertices[localVid];
		}
	}
	else
	{
		uint32 *ptr = (uint32 *) ibaWrite.getPtr();
		for(i=0;i<vegetRdrPass.NTriangles*3;i++)
		{
			// get the index in Vertices.
			uint	localVid= vegetRdrPass.TriangleLocalIndices[i];
			// get the index in new VBufffer (dstAllocator), and copy to TriangleIndices
			ptr[i]= (uint32) vegetRdrPass.Vertices[localVid];
		}
	}

	// Since change is made, flag the IG rdrpass
	vegetRdrPass.HardMode= !vegetRdrPass.HardMode;
}


// ***************************************************************************
void		CVegetableManager::setGlobalDensity(float density)
{
	clamp(density, 0.f, 1.f);
	_GlobalDensity= density;
}


// ***************************************************************************
// ***************************************************************************
// Render
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
bool			CVegetableManager::doubleSidedRdrPass(uint rdrPass)
{
	nlassert(rdrPass<NL3D_VEGETABLE_NRDRPASS);
	return (rdrPass == NL3D_VEGETABLE_RDRPASS_LIGHTED_2SIDED) ||
		(rdrPass == NL3D_VEGETABLE_RDRPASS_UNLIT_2SIDED) ||
		(rdrPass == NL3D_VEGETABLE_RDRPASS_UNLIT_2SIDED_ZSORT);
}

// ***************************************************************************
void			CVegetableManager::updateDriver(IDriver *driver)
{
	// update all driver
	uint i;
	for(i=0; i <CVegetableVBAllocator::VBTypeCount; i++)
	{
		_VBHardAllocator[i].updateDriver(driver);
		_VBSoftAllocator[i].updateDriver(driver);
	}

	// if driver changed, recreate vertex programs
	if (driver != _LastDriver)
	{
		_LastDriver = driver;
		for(i=0; i <NL3D_VEGETABLE_NRDRPASS; i++)
		{
			// both fog & no fog
			initVertexProgram(i, true);
			initVertexProgram(i, false);
		}
	}
}


// ***************************************************************************
void			CVegetableManager::loadTexture(const string &texName)
{
	// setup a CTextureFile (smartPtr-ized).
	ITexture	*tex= new CTextureFile(texName);
	loadTexture(tex);
	// setup good params.
	tex->setFilterMode(ITexture::Linear, ITexture::LinearMipMapLinear);
	tex->setWrapS(ITexture::Clamp);
	tex->setWrapT(ITexture::Clamp);
}

// ***************************************************************************
void			CVegetableManager::loadTexture(ITexture *itex)
{
	// setup a ITexture (smartPtr-ized).
	// Store in stage1, for dynamicLightmaping
	_VegetableMaterial.setTexture(1, itex);
}

// ***************************************************************************
void			CVegetableManager::setDirectionalLight(const CRGBA &ambient, const CRGBA &diffuse, const CVector &light)
{
	_DirectionalLight= light;
	_DirectionalLight.normalize();
	// Setup ambient/Diffuse.
	_GlobalAmbient= ambient;
	_GlobalDiffuse= diffuse;
}

// ***************************************************************************
void			CVegetableManager::lockBuffers()
{
	// lock all buffers
	for(uint i=0; i <CVegetableVBAllocator::VBTypeCount; i++)
	{
		_VBHardAllocator[i].lockBuffer();
		_VBSoftAllocator[i].lockBuffer();
	}
}

// ***************************************************************************
void			CVegetableManager::unlockBuffers()
{
	// unlock all buffers
	for(uint i=0; i <CVegetableVBAllocator::VBTypeCount; i++)
	{
		_VBHardAllocator[i].unlockBuffer();
		_VBSoftAllocator[i].unlockBuffer();
	}
}


// ***************************************************************************
class	CSortVSB
{
public:
	CVegetableSortBlock			*Sb;

	CSortVSB() : Sb(NULL) {}
	CSortVSB(CVegetableSortBlock *sb) : Sb(sb) {}


	// for sort()
	bool	operator<(const CSortVSB &o) const
	{
		return Sb->_SortKey>o.Sb->_SortKey;
	}

};


// ***************************************************************************
void			CVegetableManager::setupVertexProgramConstants(IDriver *driver, bool fogEnabled)
{
	nlassert(_ActiveVertexProgram);
	

	// Standard
	// setup VertexProgram constants.
	// c[0..3] take the ModelViewProjection Matrix. After setupModelMatrix();
	driver->setUniformMatrix(IDriver::VertexProgram, _ActiveVertexProgram->getUniformIndex(CGPUProgramIndex::ModelViewProjection), IDriver::ModelViewProjection, IDriver::Identity);
	// c[6] take the Fog vector. After setupModelMatrix();
	if (fogEnabled)
	{
		driver->setUniformFog(IDriver::VertexProgram, _ActiveVertexProgram->getUniformIndex(CGPUProgramIndex::Fog));
	}
	// c[8] take useful constants.
	driver->setUniform4f(IDriver::VertexProgram, _ActiveVertexProgram->idx().ProgramConstants0, 0, 1, 0.5f, 2);
	// c[9] take normalized directional light
	driver->setUniform3f(IDriver::VertexProgram, _ActiveVertexProgram->idx().DirectionalLight, _DirectionalLight);
	// c[10] take pos of camera
	driver->setUniform3f(IDriver::VertexProgram, _ActiveVertexProgram->idx().ViewCenter, _ViewCenter);
	// c[11] take factor for Blend formula
	driver->setUniform1f(IDriver::VertexProgram, _ActiveVertexProgram->idx().NegInvTransDist, -1.f/NL3D_VEGETABLE_BLOCK_BLEND_TRANSITION_DIST);



	// Bend.
	// c[16]= quaternion axis. w==1, and z must be 0
	driver->setUniform4f(IDriver::VertexProgram, _ActiveVertexProgram->idx().AngleAxis, _AngleAxis, 1);
	// c[17]=	{timeAnim, WindPower, WindPower*(1-WindBendMin)/2, 0)}
	driver->setUniform3f(IDriver::VertexProgram, _ActiveVertexProgram->idx().Wind, (float)_WindAnimTime, _WindPower, _WindPower * (1 - _WindBendMin) / 2);
	// c[18]=	High order Taylor cos coefficient: { -1/2, 1/24, -1/720, 1/40320 }
	driver->setUniform4f(IDriver::VertexProgram, _ActiveVertexProgram->idx().CosCoeff0, -1/2.f, 1/24.f, -1/720.f, 1/40320.f );
	// c[19]=	Low order Taylor cos coefficient: { 1, -1/2, 1/24, -1/720 }
	driver->setUniform4f(IDriver::VertexProgram, _ActiveVertexProgram->idx().CosCoeff1, 1, -1/2.f, 1/24.f, -1/720.f );
	// c[20]=	Low order Taylor sin coefficient: { 1, -1/6, 1/120, -1/5040 }
	driver->setUniform4f(IDriver::VertexProgram, _ActiveVertexProgram->idx().CosCoeff2, 1, -1/6.f, 1/120.f, -1/5040.f );
	// c[21]=	Special constant vector for quatToMatrix: { 0, 1, -1, 0 }
	driver->setUniform4f(IDriver::VertexProgram, _ActiveVertexProgram->idx().QuatConstants, 0.f, 1.f, -1.f, 0.f);
	// c[22]=	{0.5f, Pi, 2*Pi, 1/(2*Pi)}
	driver->setUniform4f(IDriver::VertexProgram, _ActiveVertexProgram->idx().PiConstants, 0.5f, (float)Pi, (float)(2*Pi), (float)(1/(2*Pi)));
	// c[23]=	{NL3D_VEGETABLE_VP_LUT_SIZE, 0, 0, 0}. NL3D_VEGETABLE_VP_LUT_SIZE==64.
	driver->setUniform1f(IDriver::VertexProgram, _ActiveVertexProgram->idx().LUTSize, NL3D_VEGETABLE_VP_LUT_SIZE);


	// Fill constant. Start at 32.
	for(uint i=0; i<NL3D_VEGETABLE_VP_LUT_SIZE; i++)
	{
		CVector2f		cur= _WindTable[i];
		CVector2f		delta= _WindDeltaTable[i];
		driver->setUniform4f(IDriver::VertexProgram, _ActiveVertexProgram->idx().LUT[i], cur.x, cur.y, delta.x, delta.y);
	}
}


// ***************************************************************************
void			CVegetableManager::render(const CVector &viewCenter, const CVector &frontVector, const std::vector<CPlane> &pyramid,
	ITexture *textureDLM, IDriver *driver)
{
	H_AUTO( NL3D_Vegetable_Render );

	CVegetableClipBlock		*rootToRender= NULL;

	// get normalized front vector.
	CVector		frontVectorNormed= frontVector.normed();

	// For Speed debug only.
	/*extern	bool	YOYO_ATTest;
	if(YOYO_ATTest)
		return;
	*/

	// Clip.
	//--------------------
	// For all current not empty clipBlocks, clip against pyramid, and insert visibles in list.
	CVegetableClipBlock		*ptrClipBlock= _ClipBlockList.begin();
	while(ptrClipBlock)
	{
		// if the clipBlock is visible and not empty
		if(ptrClipBlock->clip(pyramid))
		{
			// insert into visible list.
			ptrClipBlock->_RenderNext= rootToRender;
			rootToRender= ptrClipBlock;
		}

		// next
		ptrClipBlock= (CVegetableClipBlock*)ptrClipBlock->Next;
	}


	// If no clip block visible, just skip!!
	if(rootToRender==NULL)
		return;


	// Prepare Render
	//--------------------

	// profile.
	CPrimitiveProfile	ppIn, ppOut;
	driver->profileRenderedPrimitives(ppIn, ppOut);
	uint	precNTriRdr= ppOut.NTriangles;


	// Disable Fog.
	bool	bkupFog;
	bkupFog= driver->fogEnabled();

	bool fogged = bkupFog && driver->getFogStart() < _ZSortLayerDistMax;


	driver->enableFog(fogged);


	// Used by setupVertexProgramConstants(). The center of camera.
	// Used for AlphaBlending, and for ZBuffer precision problems.
	_ViewCenter= viewCenter;


	// The manager is identity in essence. But for ZBuffer improvements, must set it as close
	// to the camera. In the VertexProgram, _ViewCenter is substracted from bent vertex pos. So take it as position.
	_ManagerMatrix.identity();
	_ManagerMatrix.setPos(_ViewCenter);


	// set model matrix to the manager matrix.
	driver->setupModelMatrix(_ManagerMatrix);


	// set the driver for all allocators
	updateDriver(driver);


	// Compute Bend Anim.

	// AnimFrequency factor.
	// Doing it incrementally allow change of of frequency each frame with good results.
	_WindAnimTime+= (_Time - _WindPrecRenderTime)*_WindFrequency;
	_WindAnimTime= fmod((float)_WindAnimTime, (float)NL3D_VEGETABLE_FREQUENCY_FACTOR_PREC);
	// NB: Leave timeBend (_WindAnimTime) as a time (ie [0..1]), because VP do a "EXP time".
	// For incremental computing.
	_WindPrecRenderTime= _Time;


	// compute the angleAxis corresponding to direction
	// perform a 90deg rotation to get correct angleAxis
	_AngleAxis.set(-_WindDirection.y,_WindDirection.x,0);


	// Fill LUT WindTable.
	uint	i;
	for(i=0; i<NL3D_VEGETABLE_VP_LUT_SIZE; i++)
	{
		/* NB: this formula works quite well, because vertex BendFactor is expressed in Radian/2.
			And since animFactor==(_CosTable[i] + 1) E [0..2], we have here an arc-circle computing:
			dmove= Radius * AngleRadian/2 *  animFactor. So at max of animFactor (ie 2), we have:
			dmove= Radius * AngleRadian, which is by definition an arc-circle computing...
			And so this approximate the Bend-quaternion Vertex Program.
		*/
		float	windForce= (_CosTable[(i+32)%64] + 1);
		// Modify with _WindPower / _WindBendMin.
		windForce= _WindBendMin*2 + windForce * (1-_WindBendMin);
		windForce*= _WindPower;
		// Compute direction of the wind, and multiply by windForce.
		_WindTable[i]= CVector2f(_WindDirection.x, _WindDirection.y) * windForce;
	}
	// compute delta
	for(i=0; i<NL3D_VEGETABLE_VP_LUT_SIZE; i++)
	{
		CVector2f		cur= _WindTable[i];
		CVector2f		delta= _WindTable[ (i+1)%NL3D_VEGETABLE_VP_LUT_SIZE ] - cur;
		_WindDeltaTable[i]= delta;
	}


	// Setup TexEnvs for Dynamic lightmapping
	//--------------------
	// if the dynamic lightmap is provided
	if(textureDLM)
	{
		// stage0 RGB is Diffuse + DLM.
		_VegetableMaterial.setTexture(0, textureDLM);
		_VegetableMaterial.texEnvOpRGB(0, CMaterial::Add);
		_VegetableMaterial.texEnvArg0RGB(0, CMaterial::Texture, CMaterial::SrcColor);
		_VegetableMaterial.texEnvArg1RGB(0, CMaterial::Diffuse, CMaterial::SrcColor);
		// stage1 RGB is Previous * Texture
		_VegetableMaterial.texEnvOpRGB(1, CMaterial::Modulate);
		_VegetableMaterial.texEnvArg0RGB(1, CMaterial::Texture, CMaterial::SrcColor);
		_VegetableMaterial.texEnvArg1RGB(1, CMaterial::Previous, CMaterial::SrcColor);
	}
	else
	{
		// reset stage0 (to skip it)
		_VegetableMaterial.setTexture(0, NULL);
		// stage1 RGB is Diffuse * Texture
		_VegetableMaterial.texEnvOpRGB(1, CMaterial::Modulate);
		_VegetableMaterial.texEnvArg0RGB(1, CMaterial::Texture, CMaterial::SrcColor);
		_VegetableMaterial.texEnvArg1RGB(1, CMaterial::Diffuse, CMaterial::SrcColor);
	}
	// stage1 Alpha is always "Modulate texture with diffuse Alpha"
	_VegetableMaterial.texEnvOpAlpha(1, CMaterial::Modulate);
	_VegetableMaterial.texEnvArg0Alpha(1, CMaterial::Texture, CMaterial::SrcAlpha);
	_VegetableMaterial.texEnvArg1Alpha(1, CMaterial::Diffuse, CMaterial::SrcAlpha);



	// Render !ZSORT pass
	//--------------------

	// setup material (may have change because of ZSORT / alphaBlend pass)
	_VegetableMaterial.setBlend(false);
	_VegetableMaterial.setZWrite(true);
	_VegetableMaterial.setAlphaTestThreshold(0.5f);

	bool uprogst = driver->isUniformProgramState();
	bool progstateset[NL3D_VEGETABLE_NRDRPASS];
	for (sint rdrPass = 0; rdrPass < NL3D_VEGETABLE_NRDRPASS; ++rdrPass)
	{
		progstateset[rdrPass] = false;
	}

	/*
		Prefer sort with Soft / Hard first.
		Also, Prefer do VBsoft last, for better GPU //ism with Landscape.
	*/
	// For both allocators: Hard(1) then Soft(0)
	for(sint vbHardMode= 1; vbHardMode>=0; vbHardMode--)
	{
		// For all renderPass.
		for(sint rdrPass=0; rdrPass < NL3D_VEGETABLE_NRDRPASS; rdrPass++)
		{
			// skip ZSORT rdrPass, done after.
			if(rdrPass == NL3D_VEGETABLE_RDRPASS_UNLIT_2SIDED_ZSORT)
				continue;

			// which allocator?
			CVegetableVBAllocator	&vbAllocator= getVBAllocatorForRdrPassAndVBHardMode(rdrPass, vbHardMode);


			// Do the pass only if there is some vertices to draw.
			if(vbAllocator.getNumUserVerticesAllocated()>0)
			{
				// additional setup to the material
				bool	doubleSided= doubleSidedRdrPass(rdrPass);
				// set the 2Sided flag in the material
				_VegetableMaterial.setDoubleSided( doubleSided );

				// activate Vertex program first.
				//nlinfo("\nSTARTVP\n%s\nENDVP\n", _VertexProgram[rdrPass]->getProgram().c_str());

				_ActiveVertexProgram = _VertexProgram[rdrPass][fogged ? 1 : 0];
				nlverify(driver->activeVertexProgram(_ActiveVertexProgram));

				// Set VP constants
				if (!progstateset[uprogst ? rdrPass : 0])
				{
					setupVertexProgramConstants(driver, uprogst ? fogged : true);
				}

				// Activate the unique material.
				driver->setupMaterial(_VegetableMaterial);

				// Activate the good VBuffer
				vbAllocator.activate();

				// For all visibles clipBlock, render their instance groups.
				ptrClipBlock= rootToRender;
				while(ptrClipBlock)
				{
					// For all sortBlock of the clipBlock
					CVegetableSortBlock	*ptrSortBlock= ptrClipBlock->_SortBlockList.begin();
					while(ptrSortBlock)
					{
						// For all igs of the sortBlock
						CVegetableInstanceGroup		*ptrIg= ptrSortBlock->_InstanceGroupList.begin();
						while(ptrIg)
						{
							// rdrPass
							CVegetableInstanceGroup::CVegetableRdrPass	&vegetRdrPass= ptrIg->_RdrPass[rdrPass];

							// if this rdrPass is in same HardMode as we process now.
							if( (vegetRdrPass.HardMode && vbHardMode==1) || (!vegetRdrPass.HardMode && vbHardMode==0) )
							{
								// Ok, Render the faces.
								if(vegetRdrPass.NTriangles)
								{
									driver->activeIndexBuffer(vegetRdrPass.TriangleIndices);
									#ifdef NL_DEBUG
										if (vegetRdrPass.HardMode)
										{
											nlassert(vegetRdrPass.TriangleIndices.getFormat() == CIndexBuffer::Indices16);
										}
										else
										{
											nlassert(vegetRdrPass.TriangleIndices.getFormat() == CIndexBuffer::Indices32);
										}
									#endif
									driver->renderSimpleTriangles(0,
										vegetRdrPass.NTriangles);
								}
							}

							// next ig.
							ptrIg= (CVegetableInstanceGroup*)ptrIg->Next;
						}

						// next sortBlock
						ptrSortBlock= (CVegetableSortBlock	*)(ptrSortBlock->Next);
					}

					// next clipBlock to render
					ptrClipBlock= ptrClipBlock->_RenderNext;
				}
			}

		}

	}

	// Render ZSort pass.
	//--------------------

	// Debug Quadrants.
	/*static vector<CVector>		p0DebugLines;
	static vector<CVector>		p1DebugLines;
	p0DebugLines.clear();
	p1DebugLines.clear();*/

	// For all Blend model Layers, clear Sort Block list and setup.
	for(i=0; i<_NumZSortBlendLayers;i++)
	{
		// must have been created.
		nlassert(_ZSortModelLayers[i]);
		nlassert(_ZSortModelLayersUW[i]);
		// NB: don't refresh list, it is done in CVegetableBlendLayerModel.
		// We must do it here, because if vegetableManger::render() is no more called (eg: disabled),
		// then the models must do nothing.

		// To get layers correclty sorted from fornt to back, must init their pos
		// because it is the renderTraversal which sort them.
		// compute distance to camera of this layer.
		float	layerZ= i * _ZSortLayerDistMax / _NumZSortBlendLayers;
		// compute position of this layer.
		CVector		pos= viewCenter + frontVector * layerZ;
		// special setup in the layer.
		_ZSortModelLayers[i]->setWorldPos(pos);
		_ZSortModelLayersUW[i]->setWorldPos(pos);
	}

	// If some vertices in arrays for ZSort rdrPass
	if( getVBAllocatorForRdrPassAndVBHardMode(NL3D_VEGETABLE_RDRPASS_UNLIT_2SIDED_ZSORT, 0).getNumUserVerticesAllocated()>0 ||
		getVBAllocatorForRdrPassAndVBHardMode(NL3D_VEGETABLE_RDRPASS_UNLIT_2SIDED_ZSORT, 1).getNumUserVerticesAllocated()>0 )
	{
		uint	rdrPass= NL3D_VEGETABLE_RDRPASS_UNLIT_2SIDED_ZSORT;

		// sort
		//-------------
		// Array for sorting. (static to avoid reallocation)
		static	vector<CSortVSB>		sortVegetSbs;
		sortVegetSbs.clear();

		// For all visibles clipBlock
		ptrClipBlock= rootToRender;
		while(ptrClipBlock)
		{
			// For all sortBlock, prepare to sort them
			CVegetableSortBlock	*ptrSortBlock= ptrClipBlock->_SortBlockList.begin();
			while(ptrSortBlock)
			{
				// if the sortBlock has some sorted faces to render
				if(ptrSortBlock->_NTriangles != 0)
				{
					// Compute Distance to Viewer.
					/* NB: compute radial distance (with norm()) instead of linear distance
						(DotProduct with front vector) get less "ZSort poping".
					*/
					CVector		dirToSb= ptrSortBlock->_Center - viewCenter;
					float		distToViewer= dirToSb.norm();
					// SortKey change if the center is behind the camera.
					if(dirToSb * frontVectorNormed<0)
					{
						ptrSortBlock->_SortKey= - distToViewer;
					}
					else
					{
						ptrSortBlock->_SortKey= distToViewer;
					}

					// Choose the quadrant for this sortBlock
					sint		bestDirIdx= 0;
					float		bestDirVal= -FLT_MAX;
					// If too near, must take the frontVector as key, to get better sort.
					// use ptrSortBlock->_SortKey to get correct negative values.
					if(ptrSortBlock->_SortKey < ptrSortBlock->_Radius)
					{
						dirToSb= frontVectorNormed;
					}

					// NB: no need to normalize dirToSb, because need only to sort with DP
					// choose the good list of triangles according to quadrant.
					for(uint dirIdx=0; dirIdx<NL3D_VEGETABLE_NUM_QUADRANT; dirIdx++)
					{
						float	dirVal= CVegetableQuadrant::Dirs[dirIdx] * dirToSb;
						if(dirVal>bestDirVal)
						{
							bestDirVal= dirVal;
							bestDirIdx= dirIdx;
						}
					}

					// set the result.
					ptrSortBlock->_QuadrantId= bestDirIdx;

					// insert in list to sort.
					sortVegetSbs.push_back(CSortVSB(ptrSortBlock));

					// Debug Quadrants
					/*p0DebugLines.push_back(ptrSortBlock->_Center);
					p1DebugLines.push_back(ptrSortBlock->_Center + CVegetableQuadrant::Dirs[bestDirIdx]);*/
				}

				// next sortBlock
				ptrSortBlock= (CVegetableSortBlock	*)(ptrSortBlock->Next);
			}

			// next clipBlock to render
			ptrClipBlock= ptrClipBlock->_RenderNext;
		}

		// sort!
		// QSort. (I tried, better than radix sort, guckk!!)
		sort(sortVegetSbs.begin(), sortVegetSbs.end());


		// setup material for this rdrPass. NB: rendered after (in LayerModels).
		//-------------
		bool	doubleSided= doubleSidedRdrPass(rdrPass);
		// set the 2Sided flag in the material
		_VegetableMaterial.setDoubleSided( doubleSided );

		// setup the unique material.
		_VegetableMaterial.setBlend(true);
		_VegetableMaterial.setZWrite(false);
		// leave AlphaTest but still kick low alpha values (for fillRate performance)
		_VegetableMaterial.setAlphaTestThreshold(0.1f);



		// order them in Layers.
		//-------------

		// render from back to front, to keep correct Z order in a single layer.
		for(uint i=0; i<sortVegetSbs.size();i++)
		{
			CVegetableSortBlock	*ptrSortBlock= sortVegetSbs[i].Sb;

			float	z= ptrSortBlock->_SortKey;
			// compute in which layer must store this SB.
			z= z*_NumZSortBlendLayers / _ZSortLayerDistMax;
			// Avoid a floor(), using an OptFastFloor, but without the OptFastFloorBegin() End() group.
			// => avoid the imprecision with such a trick; *256, then divide the integer by 256.
			sint	layer= NLMISC::OptFastFloor(z*256) >> 8;
			clamp(layer, 0, (sint)_NumZSortBlendLayers-1);

			// Range in correct layer, according to water ordering
			if(ptrSortBlock->_UnderWater)
				// range in the correct layermodel (NB: keep the same layer internal order).
				_ZSortModelLayersUW[layer]->SortBlocks.push_back(ptrSortBlock);
			else
				_ZSortModelLayers[layer]->SortBlocks.push_back(ptrSortBlock);
		}

	}


	// Quit
	//--------------------

	// disable VertexProgram.
	driver->activeVertexProgram(NULL);
	_ActiveVertexProgram = NULL;


	// restore Fog.
	driver->enableFog(bkupFog);


	// Debug Quadrants
	/*for(uint l=0; l<p0DebugLines.size();l++)
	{
		CVector	dv= CVector::K;
		CDRU::drawLine(p0DebugLines[l]+dv, p1DebugLines[l]+dv, CRGBA(255,0,0), *driver);
	}*/

	// profile: compute number of triangles rendered with vegetable manager.
	driver->profileRenderedPrimitives(ppIn, ppOut);
	_NumVegetableFaceRendered= ppOut.NTriangles-precNTriRdr;

}






// ***************************************************************************
void		CVegetableManager::setupRenderStateForBlendLayerModel(IDriver *driver)
{
	// Setup Global.
	//=============

	// disable fog, for faster VP.
	_BkupFog= driver->fogEnabled();
	static volatile bool testDist = true;
	bool fogged = _BkupFog && driver->getFogStart() < _ZSortLayerDistMax;
	driver->enableFog(fogged);

	// set model matrix to the manager matrix.
	driver->setupModelMatrix(_ManagerMatrix);

	// Setup RdrPass.
	//=============
	uint	rdrPass= NL3D_VEGETABLE_RDRPASS_UNLIT_2SIDED_ZSORT;

	// activate Vertex program first.
	//nlinfo("\nSTARTVP\n%s\nENDVP\n", _VertexProgram[rdrPass]->getProgram().c_str());
	_ActiveVertexProgram = _VertexProgram[rdrPass][fogged ? 1 : 0];
	nlverify(driver->activeVertexProgram(_ActiveVertexProgram));

	// setup VP constants.
	setupVertexProgramConstants(driver, fogged);

	/*if (fogged) // duplicate
	{
		driver->setConstantFog(6);
	}*/

	// Activate the unique material (correclty setuped for AlphaBlend in render()).
	driver->setupMaterial(_VegetableMaterial);
}


// ***************************************************************************
void		CVegetableManager::resetNumVegetableFaceRendered()
{
	_NumVegetableFaceRendered= 0;
}


// ***************************************************************************
uint		CVegetableManager::getNumVegetableFaceRendered() const
{
	return _NumVegetableFaceRendered;
}


// ***************************************************************************
void		CVegetableManager::exitRenderStateForBlendLayerModel(IDriver *driver)
{
	// disable VertexProgram.
	driver->activeVertexProgram(NULL);
	_ActiveVertexProgram = NULL;

	// restore Fog.
	driver->enableFog(_BkupFog);
}



// ***************************************************************************
void		CVegetableManager::setWind(const CVector &windDir, float windFreq, float windPower, float windBendMin)
{
	// Keep only XY component of the Wind direction (because VP only support z==0 quaternions).
	_WindDirection= windDir;
	_WindDirection.z= 0;
	_WindDirection.normalize();
	// copy setup
	_WindFrequency= windFreq;
	_WindPower= windPower;
	_WindBendMin= windBendMin;
	clamp(_WindBendMin, 0, 1);
}

// ***************************************************************************
void		CVegetableManager::setTime(double time)
{
	// copy time
	_Time= time;
}


// ***************************************************************************
// ***************************************************************************
// Lighting part.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void		CVegetableManager::setUpdateLightingTime(double time)
{
	_ULTime= time;
}


// ***************************************************************************
void		CVegetableManager::updateLighting()
{
	// first time in this method??
	if(!_ULPrecTimeInit)
	{
		_ULPrecTimeInit= true;
		_ULPrecTime= _ULTime;
	}
	// compute delta time from last update.
	float dt= float(_ULTime - _ULPrecTime);
	_ULPrecTime= _ULTime;

	// compute number of vertices to update.
	_ULNVerticesToUpdate+= dt*_ULFrequency * _ULNTotalVertices;
	// maximize, so at max, it computes all Igs, just one time.
	_ULNVerticesToUpdate= min(_ULNVerticesToUpdate, (float)_ULNTotalVertices);

	// go.
	doUpdateLighting();
}


// ***************************************************************************
void		CVegetableManager::updateLightingAll()
{
	// maximize, so at max, it computes all Igs
	_ULNVerticesToUpdate= (float)_ULNTotalVertices;

	// go.
	doUpdateLighting();
}


// ***************************************************************************
void		CVegetableManager::doUpdateLighting()
{
	// while there is still some vertices to update.
	while(_ULNVerticesToUpdate > 0 && _ULRootIg)
	{
		// update the current ig. if all updated, skip to next one.
		if(updateLightingIGPart())
		{
			// next
			_ULRootIg= _ULRootIg->_ULNext;
		}
	}

	// Now, _ULNVerticesToUpdate should be <=0. (most of the time < 0)
}


// ***************************************************************************
void		CVegetableManager::setUpdateLightingFrequency(float freq)
{
	freq= max(freq, 0.f);
	_ULFrequency= freq;
}


// ***************************************************************************
bool		CVegetableManager::updateLightingIGPart()
{
	nlassert(_ULRootIg);


	// First, update lighting info global to the ig, ie update current
	// colros of the PointLights which influence the ig.
	_ULRootIg->VegetableLightEx.computeCurrentColors();

	// while there is some vertices to update
	while(_ULNVerticesToUpdate>0)
	{
		// if all rdrPass of the ig are processed.
		if(_ULCurrentIgRdrPass>= NL3D_VEGETABLE_NRDRPASS)
		{
			// All this Ig is updated.
			_ULCurrentIgRdrPass= 0;
			_ULCurrentIgInstance= 0;
			// skip to next Ig.
			return true;
		}
		CVegetableInstanceGroup::CVegetableRdrPass	&vegetRdrPass= _ULRootIg->_RdrPass[_ULCurrentIgRdrPass];

		// if all instances are processed for this pass (especially if size()==0 !!)
		if(_ULCurrentIgInstance>= vegetRdrPass.LightedInstances.size())
		{
			// skip to the next rdrPass.
			_ULCurrentIgRdrPass++;
			_ULCurrentIgInstance= 0;
			continue;
		}

		// Process this instance.
		_ULNVerticesToUpdate-= updateInstanceLighting(_ULRootIg, _ULCurrentIgRdrPass, _ULCurrentIgInstance);

		// next instance.
		_ULCurrentIgInstance++;

		// if all instances are processed for this pass
		if(_ULCurrentIgInstance>= vegetRdrPass.LightedInstances.size())
		{
			// skip to the next rdrPass.
			_ULCurrentIgRdrPass++;
			_ULCurrentIgInstance= 0;
		}
	}

	// If all rdrPass of the ig are processed.
	if(_ULCurrentIgRdrPass>= NL3D_VEGETABLE_NRDRPASS)
	{
		// All this Ig is updated.
		_ULCurrentIgRdrPass= 0;
		_ULCurrentIgInstance= 0;
		// skip to next Ig.
		return true;
	}
	else
	{
		// The Ig is not entirely updated.
		return false;
	}

}


// ***************************************************************************
uint		CVegetableManager::updateInstanceLighting(CVegetableInstanceGroup *ig, uint rdrPassId, uint instanceId)
{
	nlassert(ig);
	// get the rdrPass.
	nlassert(rdrPassId<NL3D_VEGETABLE_NRDRPASS);
	CVegetableInstanceGroup::CVegetableRdrPass	&vegetRdrPass= ig->_RdrPass[rdrPassId];
	// get the lighted instance.
	nlassert(instanceId<vegetRdrPass.LightedInstances.size());
	CVegetableInstanceGroup::CVegetableLightedInstance	&vegetLI= vegetRdrPass.LightedInstances[instanceId];

	// get the shape
	CVegetableShape		*shape= vegetLI.Shape;
	// it must be lighted.
	nlassert(shape->Lighted);
	bool	instanceLighted= true;


	// get ref on the vegetLex.
	CVegetableLightEx	&vegetLex= ig->VegetableLightEx;
	// Color of pointLights modulated by diffuse.
	CRGBA	diffusePL[2];
	diffusePL[0] = CRGBA::Black;
	diffusePL[1] = CRGBA::Black;
	if(vegetLex.NumLights>=1)
	{
		diffusePL[0].modulateFromColorRGBOnly(vegetLI.MatDiffuse, vegetLex.Color[0]);
		if(vegetLex.NumLights>=2)
		{
			diffusePL[1].modulateFromColorRGBOnly(vegetLI.MatDiffuse, vegetLex.Color[1]);
		}
	}

	// Recompute lighting
	//===========

	// setup for this instance.
	//---------
	// Precompute lighting or not??
	bool	precomputeLighting= instanceLighted && shape->PreComputeLighting;
	// bestSided Precompute lighting or not??
	bool	bestSidedPrecomputeLighting= precomputeLighting && shape->BestSidedPreComputeLighting;
	// destLighted?
	bool	destLighted= instanceLighted && !shape->PreComputeLighting;
	// Diffuse and ambient, modulated by current GlobalAmbient and GlobalDiffuse.
	CRGBA	primaryRGBA, secondaryRGBA;
	primaryRGBA.modulateFromColorRGBOnly(vegetLI.MatDiffuse, _GlobalDiffuse);
	secondaryRGBA.modulateFromColorRGBOnly(vegetLI.MatAmbient, _GlobalAmbient);
	// get normal matrix
	CMatrix		&normalMat= vegetLI.NormalMat;
	// array of vertex id to update
	uint32		*ptrVid= vegetRdrPass.Vertices.getPtr() + vegetLI.StartIdInRdrPass;
	uint		numVertices= (uint)shape->InstanceVertices.size();

	// Copy Dynamic Lightmap UV in Alpha part (save memory for an extra cost of 1 VP instruction)
	primaryRGBA.A= vegetLI.DlmUV.U;
	secondaryRGBA.A= vegetLI.DlmUV.V;


	// get VertexBuffer info.
	CVegetableVBAllocator	*allocator;
	allocator= &getVBAllocatorForRdrPassAndVBHardMode(rdrPassId, vegetRdrPass.HardMode);
	const CVertexBuffer	&dstVBInfo= allocator->getSoftwareVertexBuffer();

	uint	srcNormalOff= (instanceLighted? shape->VB.getNormalOff() : 0);
	uint	dstColor0Off= dstVBInfo.getValueOffEx(NL3D_VEGETABLE_VPPOS_COLOR0);
	uint	dstColor1Off= dstVBInfo.getValueOffEx(NL3D_VEGETABLE_VPPOS_COLOR1);

	// For D3D, If the VertexBuffer is in BGRA mode
	if(allocator->isBGRA())
	{
		// then swap only the B and R (no cpu cycle added per vertex)
		primaryRGBA.swapBR();
		secondaryRGBA.swapBR();
		diffusePL[0].swapBR();
		diffusePL[1].swapBR();
	}

	CVertexBufferRead vba;
	shape->VB.lock (vba);
	CVertexBufferReadWrite vbaOut;
	allocator->getSoftwareVertexBuffer ().lock(vbaOut);

	// For all vertices, recompute lighting.
	//---------
	for(sint i=0; i<(sint)numVertices;i++)
	{
		// get the Vertex in the VB.
		uint	vid= ptrVid[i];
		// store in tmp shape.
		shape->InstanceVertices[i]= vid;

		// Fill this vertex.
		const uint8	*srcPtr= (const uint8*)vba.getVertexCoordPointer(i);
		uint8	*dstPtr= (uint8*)vbaOut.getVertexCoordPointer(vid);


		// if !precomputeLighting (means destLighted...)
		if(!precomputeLighting)
		{
			// just copy the primary and secondary color
			*(CRGBA*)(dstPtr + dstColor0Off)= primaryRGBA;
			*(CRGBA*)(dstPtr + dstColor1Off)= secondaryRGBA;
		}
		else
		{
			nlassert(!destLighted);

			// compute normal.
			CVector		rotNormal= normalMat.mulVector( *(CVector*)(srcPtr + srcNormalOff) );
			// must normalize() because scale is possible.
			rotNormal.normalize();

			// Do the compute.
			if(!bestSidedPrecomputeLighting)
			{
				computeVegetVertexLighting(rotNormal,
					_DirectionalLight, primaryRGBA, secondaryRGBA,
					vegetLex, diffusePL, (CRGBA*)(dstPtr + dstColor0Off) );
			}
			else
			{
				computeVegetVertexLightingForceBestSided(rotNormal,
					_DirectionalLight, primaryRGBA, secondaryRGBA,
					vegetLex, diffusePL, (CRGBA*)(dstPtr + dstColor0Off) );
			}

		}

		// flust the vertex in AGP.
		allocator->flushVertex(vid);
	}


	// numVertices vertices are updated
	return numVertices;
}


} // NL3D
