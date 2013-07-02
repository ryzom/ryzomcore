/************************************************************************************

Filename    :   stereo_ovf_fp.cpp
Content     :   Barrel fragment program compiled to a blob of assembly
Created     :   July 01, 2013

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

namespace NL3D {
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
	//#var sampler2D cTex0 : TEX0 : texunit 0 : 6 : 1
	//#var float4 oCol : $vout.COLOR : COL : 7 : 1
	//#const c[5] = 0.25 0.5 0 1
	"PARAM c[6] = { program.local[0..4],\n"
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
const char *g_StereoOVR_ps_2_0 =
	//# 28 instructions, 2 R-regs
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
	//var sampler2D cTex0 : TEX0 : texunit 0 : 6 : 1
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
}