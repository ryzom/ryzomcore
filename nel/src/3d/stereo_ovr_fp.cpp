/************************************************************************************

Filename    :   stereo_ovf_fp.cpp
Content     :   Barrel fragment program compiled to a blob of assembly
Created     :   July 01, 2013
Modified by :   Jan Boon (Kaetemi)

Copyright   :   Copyright 2012 Oculus VR, Inc. All Rights reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

************************************************************************************/

#include "std3d.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D {
const char *g_StereoOVR_fp40 = 
	"!!ARBfp1.0\n"
	"OPTION NV_fragment_program2;\n"
	//# cgc version 3.1.0013, build date Apr 18 2012
	//# command line args: -profile fp40
	//# source file: pp_oculus_vr.cg
	//#vendor NVIDIA Corporation
	//#version 3.1.0.13
	//#profile fp40
	//#program pp_oculus_vr
	//#semantic pp_oculus_vr.cLensCenter
	//#semantic pp_oculus_vr.cScreenCenter
	//#semantic pp_oculus_vr.cScale
	//#semantic pp_oculus_vr.cScaleIn
	//#semantic pp_oculus_vr.cHmdWarpParam
	//#semantic pp_oculus_vr.cTex0 : TEX0
	//#var float2 texCoord : $vin.TEXCOORD0 : TEX0 : 0 : 1
	//#var float2 cLensCenter :  : c[0] : 1 : 1
	//#var float2 cScreenCenter :  : c[1] : 2 : 1
	//#var float2 cScale :  : c[2] : 3 : 1
	//#var float2 cScaleIn :  : c[3] : 4 : 1
	//#var float4 cHmdWarpParam :  : c[4] : 5 : 1
	//#var sampler2D nlTex0 : TEX0 : texunit 0 : 6 : 1
	//#var float4 oCol : $vout.COLOR : COL : 7 : 1
	//#const c[5] = 0.25 0.5 0
	"PARAM c[6] = { program.env[0..4],\n" // program.local->program.env!
	"				{ 0.25, 0.5, 0 } };\n"
	"TEMP R0;\n"
	"TEMP R1;\n"
	"SHORT TEMP H0;\n"
	"TEMP RC;\n"
	"TEMP HC;\n"
	"OUTPUT oCol = result.color;\n"
	"ADDR  R0.xy, fragment.texcoord[0], -c[0];\n"
	"MULR  R0.xy, R0, c[3];\n"
	"MULR  R0.z, R0.y, R0.y;\n"
	"MADR  R1.x, R0, R0, R0.z;\n"
	"MULR  R0.zw, R1.x, c[4].xywz;\n"
	"MADR  R1.y, R1.x, c[4], c[4].x;\n"
	"MADR  R0.w, R0, R1.x, R1.y;\n"
	"MULR  R0.z, R0, R1.x;\n"
	"MADR  R0.z, R0, R1.x, R0.w;\n"
	"MULR  R1.xy, R0, R0.z;\n"
	"MOVR  R0.xy, c[5];\n"
	"ADDR  R1.zw, R0.xyxy, c[1].xyxy;\n"
	"MOVR  R0.zw, c[0].xyxy;\n"
	"MADR  R0.zw, R1.xyxy, c[2].xyxy, R0;\n"
	"MINR  R1.xy, R0.zwzw, R1.zwzw;\n"
	"ADDR  R0.xy, -R0, c[1];\n"
	"MAXR  R0.xy, R0, R1;\n"
	"SEQR  H0.xy, R0, R0.zwzw;\n"
	"MULXC HC.x, H0, H0.y;\n"
	"IF    EQ.x;\n"
	"MOVR  oCol, c[5].z;\n"
	"ELSE;\n"
	"TEX   oCol, R0.zwzw, texture[0], 2D;\n"
	"ENDIF;\n"
	"END\n";
	//# 24 instructions, 2 R-regs, 1 H-regs

const char *g_StereoOVR_arbfp1 =
	"!!ARBfp1.0\n"
	//# cgc version 3.1.0013, build date Apr 18 2012
	//# command line args: -profile arbfp1
	//# source file: pp_oculus_vr.cg
	//#vendor NVIDIA Corporation
	//#version 3.1.0.13
	//#profile arbfp1
	//#program pp_oculus_vr
	//#semantic pp_oculus_vr.cLensCenter
	//#semantic pp_oculus_vr.cScreenCenter
	//#semantic pp_oculus_vr.cScale
	//#semantic pp_oculus_vr.cScaleIn
	//#semantic pp_oculus_vr.cHmdWarpParam
	//#semantic pp_oculus_vr.cTex0 : TEX0
	//#var float2 texCoord : $vin.TEXCOORD0 : TEX0 : 0 : 1
	//#var float2 cLensCenter :  : c[0] : 1 : 1
	//#var float2 cScreenCenter :  : c[1] : 2 : 1
	//#var float2 cScale :  : c[2] : 3 : 1
	//#var float2 cScaleIn :  : c[3] : 4 : 1
	//#var float4 cHmdWarpParam :  : c[4] : 5 : 1
	//#var sampler2D nlTex0 : TEX0 : texunit 0 : 6 : 1
	//#var float4 oCol : $vout.COLOR : COL : 7 : 1
	//#const c[5] = 0.25 0.5 0 1
	"PARAM c[6] = { program.env[0..4],\n"
	"				{ 0.25, 0.5, 0, 1 } };\n"
	"TEMP R0;\n"
	"TEMP R1;\n"
	"ADD R0.xy, fragment.texcoord[0], -c[0];\n"
	"MUL R0.xy, R0, c[3];\n"
	"MUL R0.z, R0.y, R0.y;\n"
	"MAD R0.z, R0.x, R0.x, R0;\n"
	"MUL R0.w, R0.z, c[4];\n"
	"MUL R0.w, R0, R0.z;\n"
	"MAD R1.y, R0.z, c[4], c[4].x;\n"
	"MUL R1.x, R0.z, c[4].z;\n"
	"MAD R1.x, R0.z, R1, R1.y;\n"
	"MAD R0.z, R0.w, R0, R1.x;\n"
	"MUL R0.xy, R0, R0.z;\n"
	"MOV R0.zw, c[5].xyxy;\n"
	"ADD R1.xy, R0.zwzw, c[1];\n"
	"MUL R0.xy, R0, c[2];\n"
	"ADD R0.xy, R0, c[0];\n"
	"MIN R1.xy, R1, R0;\n"
	"ADD R0.zw, -R0, c[1].xyxy;\n"
	"MAX R0.zw, R0, R1.xyxy;\n"
	"ADD R0.zw, R0, -R0.xyxy;\n"
	"ABS R0.zw, R0;\n"
	"CMP R0.zw, -R0, c[5].z, c[5].w;\n"
	"MUL R0.z, R0, R0.w;\n"
	"ABS R0.z, R0;\n"
	"CMP R0.z, -R0, c[5], c[5].w;\n"
	"ABS R1.x, R0.z;\n"
	"TEX R0, R0, texture[0], 2D;\n"
	"CMP R1.x, -R1, c[5].z, c[5].w;\n"
	"CMP result.color, -R1.x, R0, c[5].z;\n"
	"END\n";
	//# 28 instructions, 2 R-regs

const char *g_StereoOVR_ps_2_0 =
	"ps_2_0\n"
	// cgc version 3.1.0013, build date Apr 18 2012
	// command line args: -profile ps_2_0
	// source file: pp_oculus_vr.cg
	//vendor NVIDIA Corporation
	//version 3.1.0.13
	//profile ps_2_0
	//program pp_oculus_vr
	//semantic pp_oculus_vr.cLensCenter
	//semantic pp_oculus_vr.cScreenCenter
	//semantic pp_oculus_vr.cScale
	//semantic pp_oculus_vr.cScaleIn
	//semantic pp_oculus_vr.cHmdWarpParam
	//semantic pp_oculus_vr.cTex0 : TEX0
	//var float2 texCoord : $vin.TEXCOORD0 : TEX0 : 0 : 1
	//var float2 cLensCenter :  : c[0] : 1 : 1
	//var float2 cScreenCenter :  : c[1] : 2 : 1
	//var float2 cScale :  : c[2] : 3 : 1
	//var float2 cScaleIn :  : c[3] : 4 : 1
	//var float4 cHmdWarpParam :  : c[4] : 5 : 1
	//var sampler2D nlTex0 : TEX0 : texunit 0 : 6 : 1
	//var float4 oCol : $vout.COLOR : COL : 7 : 1
	//const c[5] = -0.25 -0.5 0.25 0.5
	//const c[6] = 1 0
	"dcl_2d s0\n"
	"def c5, -0.25000000, -0.50000000, 0.25000000, 0.50000000\n"
	"def c6, 1.00000000, 0.00000000, 0, 0\n"
	"dcl t0.xy\n"
	"add r0.xy, t0, -c0\n"
	"mul r4.xy, r0, c3\n"
	"mul r0.x, r4.y, r4.y\n"
	"mad r0.x, r4, r4, r0\n"
	"mul r1.x, r0, c4.w\n"
	"mul r1.x, r1, r0\n"
	"mad r3.x, r0, c4.y, c4\n"
	"mul r2.x, r0, c4.z\n"
	"mad r2.x, r0, r2, r3\n"
	"mad r0.x, r1, r0, r2\n"
	"mul r0.xy, r4, r0.x\n"
	"mul r0.xy, r0, c2\n"
	"add r3.xy, r0, c0\n"
	"mov r1.x, c5.z\n"
	"mov r1.y, c5.w\n"
	"mov r2.xy, c1\n"
	"add r2.xy, r1, r2\n"
	"mov r1.xy, c1\n"
	"min r2.xy, r2, r3\n"
	"add r1.xy, c5, r1\n"
	"max r1.xy, r1, r2\n"
	"add r1.xy, r1, -r3\n"
	"abs r1.xy, r1\n"
	"cmp r1.xy, -r1, c6.x, c6.y\n"
	"mul_pp r1.x, r1, r1.y\n"
	"abs_pp r1.x, r1\n"
	"cmp_pp r1.x, -r1, c6, c6.y\n"
	"abs_pp r1.x, r1\n"
	"texld r0, r3, s0\n"
	"cmp r0, -r1.x, r0, c6.y\n"
	"mov oC0, r0\n";

const char *g_StereoOVR_glsl330f = 
	"#version 330\n"
	"\n"
	"bool _TMP2;\n"
	"bvec2 _TMP1;\n"
	"vec2 _TMP3;\n"
	"uniform vec2 cLensCenter;\n"
	"uniform vec2 cScreenCenter;\n"
	"uniform vec2 cScale;\n"
	"uniform vec2 cScaleIn;\n"
	"uniform vec4 cHmdWarpParam;\n"
	"uniform sampler2D nlTex0;\n"
	"vec2 _TMP10;\n"
	"vec2 _b0011;\n"
	"vec2 _a0011;\n"
	"in vec4 nlTexCoord0;\n"
	"out vec4 nlCol;\n"
	"\n"
	"void main()\n"
	"{\n"
	"	vec2 _theta;\n"
	"	float _rSq;\n"
	"	vec2 _theta1;\n"
	"	vec2 _tc;\n"
	"\n"
	"	_theta = (nlTexCoord0.xy - cLensCenter)*cScaleIn;\n"
	"	_rSq = _theta.x*_theta.x + _theta.y*_theta.y;\n"
	"	_theta1 = _theta*(cHmdWarpParam.x + cHmdWarpParam.y*_rSq + cHmdWarpParam.z*_rSq*_rSq + cHmdWarpParam.w*_rSq*_rSq*_rSq);\n"
	"	_tc = cLensCenter + cScale*_theta1;\n"
	"	_a0011 = cScreenCenter - vec2( 0.25, 0.5);\n"
	"	_b0011 = cScreenCenter + vec2( 0.25, 0.5);\n"
	"	_TMP3 = min(_b0011, _tc);\n"
	"	_TMP10 = max(_a0011, _TMP3);\n"
	"	_TMP1 = bvec2(_TMP10.x == _tc.x, _TMP10.y == _tc.y);\n"
	"	_TMP2 = _TMP1.x && _TMP1.y;\n"
	"	if (!_TMP2) {\n"
	"		nlCol = vec4(0, 0, 0, 0);\n"
	"	} else {\n"
	"		nlCol = texture(nlTex0, _tc);\n"
	"	}\n"
	"}\n";

}

/* end of file */
