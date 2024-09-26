// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include "nel/3d/meshvp_per_pixel_light.h"
#include "nel/3d/mesh_base_instance.h"
#include "nel/3d/driver.h"
#include "nel/3d/scene.h"
#include "nel/3d/render_trav.h"
#include "nel/3d/render_trav.h"
#include "nel/3d/vertex_program_parse.h"



#include <string>


#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{

NLMISC::CSmartPtr<CVertexProgramPerPixelLight>	CMeshVPPerPixelLight::_VertexProgram[NumVp];

// ***************************************************************************
// Light VP fragment constants start at 24
static	const uint	VPLightConstantStart = 24;

// ***************************************************************************
// ***************************************************************************

/////////////////
// omni lights //
/////////////////

/** We store the first tangent vector of the tangent space in v[8].
  * The third vector can be computed using a cross product.
  * We assume that normal and tangent are normalized.
  * The position of light must be expressed in object space, and is stored in c[4]
  */
// omni light + specular, no normalization
static const char*	PPLightingVPCodeBegin =
"!!VP1.0																				\n\
#compute B = N ^ T																		\n\
MOV R6, v[2];																			\n\
MUL R1, R6.yzxw, v[9].zxyw;																\n\
MAD R1, v[9].yzxw, -R6.zxyw, R1;														\n\
#vector in tangent space = [ T B N ] * L												\n\
ADD R2, c[4], -v[0];			   # compute L											\n\
DP3 R3, R2, R2;					   # get L normalized									\n\
RSQ R3, R3.x;																			\n\
MUL R2, R3, R2;																			\n\
DP3 o[TEX0].x, v[9], R2;		   # get x of light vector in tangent space             \n\
DP3 o[TEX0].y, R1, R2;             # get y												\n\
DP3 o[TEX0].z, R6, R2;			   # get z												\n\
#specular part																			\n\
ADD R3, c[5], - v[0];			   # compute V (return to eye)							\n\
#compute inverse norm of V																\n\
DP3 R4, R3, R3;																			\n\
RSQ R4, R4.x;																			\n\
#we normalize V and add it to L															\n\
MAD R2, R4, R3, R2; #H in R1															\n\
																						\n\
#normalize H																			\n\
DP3 R3, R2, R2;																			\n\
RSQ R3, R3.x;																			\n\
MUL R2, R3, R2;																			\n\
#compute H in tangent space																\n\
DP3 o[TEX2].x, v[9], R2;																\n\
DP3 o[TEX2].y, R1, R2;																	\n\
DP3 o[TEX2].z, R6, R2;																	\n\
# Position in R5 for additionnal lighting                                               \n\
MOV R5, v[0];																			\n\
# Normal is in R6 for additionnal lighting												\n\
";

/// The same as above, but for skin / MRM : This normalize the tangent basis.
static const char*	PPLightingVPNormalizeCodeBegin =
"!!VP1.0																				\n\
#normalize the normal																	\n\
DP3 R1, v[2], v[2];																		\n\
RSQ R1, R1.x;																			\n\
MUL R6, v[2], R1;																			\n\
																						\n\
#normalize the second vector															\n\
DP3	R1, R6, v[9];																		\n\
MAD	R1, R6, -R1, v[9]; #subtract the normal component									\n\
DP3 R2, R1, R1;																			\n\
RSQ	R2, R2.x;																			\n\
MUL R5, R1, R2;	#second basis vector in R5												\n\
#compute B = N ^ T																		\n\
MUL R1, R6.yzxw, R5.zxyw;																\n\
MAD R1, R5.yzxw, -R6.zxyw, R1; #third basis vector in R1								\n\
																						\n\
#vector in tangent space = [ T B N ] * L												\n\
ADD R2, c[4], -v[0];			   # compute L											\n\
DP3 R3, R2, R2;					   # get L normalized									\n\
RSQ R3, R3.x;																			\n\
MUL R2, R3, R2;																			\n\
DP3 o[TEX0].x, R5, R2;			   # get x of light vector in tangent space				\n\
DP3 o[TEX0].y, R1, R2;             # get y												\n\
DP3 o[TEX0].z, R6, R2;			   # get z												\n\
																						\n\
#specular part																			\n\
ADD R3, c[5], - v[0];			   # compute V (return to eye)							\n\
#compute inverse norm of V																\n\
DP3 R4, R3, R3;																			\n\
RSQ R4, R4.x;																			\n\
#we normalize V and add it to L															\n\
MAD R2, R4, R3, R2; #H in R1															\n\
																						\n\
#normalize H																			\n\
DP3 R3, R2, R2;																			\n\
RSQ R3, R3.x;																			\n\
MUL R2, R3, R2;																			\n\
#compute H in tangent space																\n\
DP3 o[TEX2].x, R5, R2;																	\n\
DP3 o[TEX2].y, R1, R2;																	\n\
DP3 o[TEX2].z, R6, R2;																	\n\
# Position in R5 for additionnal lighting                                               \n\
MOV R5, v[0];																			\n\
# Normal is in R6 for additionnal lighting												\n\
";


// Omni light, no specular
static const char*	PPLightingNoSpecVPCodeBegin =
"!!VP1.0																				\n\
#compute B = N ^ T																		\n\
MOV R6, v[2];																			\n\
MUL R1, R6.yzxw, v[9].zxyw;																\n\
MAD R1, v[9].yzxw, -R6.zxyw, R1;														\n\
																						\n\
#vector in tangent space = [ T B N ] * L												\n\
ADD R2, c[4], -v[0];			   # compute L											\n\
DP3 R3, R2, R2;					   # get L normalized									\n\
RSQ R3, R3.x;																			\n\
MUL R2, R3, R2;																			\n\
DP3 o[TEX0].x, v[9], R2;		   # get x of light vector in tangent space             \n\
DP3 o[TEX0].y, R1, R2;             # get y												\n\
DP3 o[TEX0].z, R6, R2;			   # get z												\n\
																						\n\
																						\n\
# Normal is in R6 for additionnal lighting												\n\
# Position in R5 for additionnal lighting                                               \n\
MOV R5, v[0];																			\n\
";

/// Omni light with normalization and no specular
static const char*	PPLightingVPNormalizeNoSpecCodeBegin =
"!!VP1.0																				\n\
#normalize the normal																	\n\
DP3 R1, v[2], v[2];																		\n\
RSQ R1, R1.x;																			\n\
MUL R6, v[2], R1;																			\n\
																						\n\
#normalize the second vector															\n\
DP3	R1, R6, v[9];																		\n\
MAD	R1, R6, -R1, v[9]; #subtract the normal component									\n\
DP3 R2, R1, R1;																			\n\
RSQ	R2, R2.x;																			\n\
MUL R5, R1, R2;	#second basis vector in R5												\n\
#compute B = N ^ T																		\n\
MUL R1, R6.yzxw, R5.zxyw;																\n\
MAD R1, R5.yzxw, -R6.zxyw, R1; #third basis vector in R1								\n\
																						\n\
#vector in tangent space = [ T B N ] * L												\n\
ADD R2, c[4], -v[0];			   # compute L											\n\
DP3 R3, R2, R2;					   # get L normalized									\n\
RSQ R3, R3.x;																			\n\
MUL R2, R3, R2;																			\n\
DP3 o[TEX0].x, R5, R2;			   # get x of light vector in tangent space				\n\
DP3 o[TEX0].y, R1, R2;             # get y												\n\
DP3 o[TEX0].z, R6, R2;			   # get z												\n\
																						\n\
# Normal is in R6 for additionnal lighting												\n\
# Position in R5 for additionnal lighting                                               \n\
MOV R5, v[0];																			\n\
";


/////////////////////////
// directionnal lights //
/////////////////////////

/// We store the direction of the light rather than its position
/// The direction must be normalized and expressed in model space.

// directionnal, no normalization
static const char*	PPLightingDirectionnalVPCodeBegin =
"!!VP1.0																				\n\
#compute B = N ^ T																		\n\
MOV R6, v[2];																			\n\
MUL R1, R6.yzxw, v[9].zxyw;																\n\
MAD R1, v[9].yzxw, -R6.zxyw, R1;														\n\
																						\n\
#vector in tangent space = [ T B N ] * L												\n\
DP3 o[TEX0].x, v[9], -c[4];		   # get x of light vector in tangent space             \n\
DP3 o[TEX0].y, R1, -c[4];           # get y												\n\
DP3 o[TEX0].z, R6, -c[4];		   # get z												\n\
																						\n\
#specular part																			\n\
ADD R3, c[5], - v[0];			   # compute V (return to eye)							\n\
#compute inverse norm of V																\n\
DP3 R4, R3, R3;																			\n\
RSQ R4, R4.x;																			\n\
#we normalize V and add it to L. It gives H unnormalized								\n\
MAD R2, R4, R3, -c[4]; #H in R1															\n\
																						\n\
#normalize H																			\n\
DP3 R3, R2, R2;																			\n\
RSQ R3, R3.x;																			\n\
MUL R2, R3, R2;																			\n\
#compute H in tangent space																\n\
DP3 o[TEX2].x, v[9], R2;																\n\
DP3 o[TEX2].y, R1, R2;																	\n\
DP3 o[TEX2].z, R6, R2;																	\n\
																						\n\
# Normal is in R6 for additionnal lighting												\n\
# Position in R5 for additionnal lighting                                               \n\
MOV R5, v[0];																			\n\
";

// directionnal + normalization
static const char*	PPLightingDirectionnalVPNormalizeCodeBegin =
"!!VP1.0																				\n\
#normalize the normal																	\n\
DP3 R1, v[2], v[2];																		\n\
RSQ R1, R1.x;																			\n\
MUL R6, v[2], R1;																		\n\
																						\n\
#normalize the second vector															\n\
DP3	R1, R6, v[9];																		\n\
MAD	R1, R6, -R1, v[9]; #subtract the normal component									\n\
DP3 R2, R1, R1;																			\n\
RSQ	R2, R2.x;																			\n\
MUL R5, R1, R2;	#second basis vector in R5												\n\
#compute B = N ^ T																		\n\
MUL R1, R6.yzxw, R5.zxyw;																\n\
MAD R1, R5.yzxw, -R6.zxyw, R1; #third basis vector in R1								\n\
																						\n\
#vector in tangent space = [ T B N ] * L												\n\
DP3 o[TEX0].x, R5, -c[4];			   # get x of light vector in tangent space			\n\
DP3 o[TEX0].y, R1, -c[4];               # get y											\n\
DP3 o[TEX0].z, R6, -c[4];			   # get z											\n\
																						\n\
#specular part																			\n\
ADD R3, c[5], - v[0];			   # compute V (return to eye)							\n\
#compute inverse norm of V																\n\
DP3 R4, R3, R3;																			\n\
RSQ R4, R4.x;																			\n\
#we normalize V and add it to L															\n\
MAD R2, R4, R3, -c[4]; #H in R1															\n\
																						\n\
#normalize H																			\n\
DP3 R3, R2, R2;																			\n\
RSQ R3, R3.x;																			\n\
MUL R2, R3, R2;																			\n\
#compute H in tangent space																\n\
DP3 o[TEX2].x, R5, R2;																	\n\
DP3 o[TEX2].y, R1, R2;																	\n\
DP3 o[TEX2].z, R6, R2;																	\n\
# Normal is in R6 for additionnal lighting												\n\
# Position in R5 for additionnal lighting                                               \n\
MOV R5, v[0];																			\n\
";


// directionnal, no normalization, no specular
static const char*	PPLightingDirectionnalNoSpecVPCodeBegin =
"!!VP1.0																				\n\
#compute B = N ^ T																		\n\
MOV R6, v[2];																			\n\
MUL R1, R6.yzxw, v[9].zxyw;																\n\
MAD R1, v[9].yzxw, -R6.zxyw, R1;														\n\
																						\n\
#vector in tangent space = [ T B N ] * L												\n\
DP3 o[TEX0].x, v[9], -c[4];		   # get x of light vector in tangent space             \n\
DP3 o[TEX0].y, R1, -c[4];           # get y												\n\
DP3 o[TEX0].z, R6, -c[4];		   # get z												\n\
# Normal is in R6 for additionnal lighting												\n\
# Position in R5 for additionnal lighting                                               \n\
MOV R5, v[0];																			\n\
";

// directionnal + normalization, no specular
static const char*	PPLightingDirectionnalNoSpecVPNormalizeCodeBegin =
"!!VP1.0																				\n\
#normalize the normal																	\n\
DP3 R1, v[2], v[2];																		\n\
RSQ R1, R1.x;																			\n\
MUL R6, v[2], R1;																		\n\
																						\n\
#normalize the second vector															\n\
DP3	R1, R6, v[9];																		\n\
MAD	R1, R6, -R1, v[9]; #subtract the normal component									\n\
DP3 R2, R1, R1;																			\n\
RSQ	R2, R2.x;																			\n\
MUL R5, R1, R2;	#second basis vector in R5												\n\
#compute B = N ^ T																		\n\
MUL R1, R6.yzxw, R5.zxyw;																\n\
MAD R1, R5.yzxw, -R6.zxyw, R1; #third basis vector in R1								\n\
																						\n\
#vector in tangent space = [ T B N ] * L												\n\
DP3 o[TEX0].x, R5, -c[4];			   # get x of light vector in tangent space			\n\
DP3 o[TEX0].y, R1, -c[4];               # get y											\n\
DP3 o[TEX0].z, R6, -c[4];			   # get z											\n\
																						\n\
# Normal is in R6 for additionnal lighting												\n\
# Position in R5 for additionnal lighting                                               \n\
MOV R5, v[0];																			\n\
";



// End of per pixel lighting code : compute pos and setup texture
static const char*	PPLightingVPCodeEnd=
"	# compute in Projection space														\n\
	DP4 o[HPOS].x, c[0], v[0];															\n\
	DP4 o[HPOS].y, c[1], v[0];															\n\
	DP4 o[HPOS].z, c[2], v[0];															\n\
	DP4 o[HPOS].w, c[3], v[0];															\n\
	MOV o[TEX1], v[8];																	\n\
	END																					\n\
";

/***************************************************************/
/******************* THE FOLLOWING CODE IS COMMENTED OUT *******/
/***************************************************************

// Test code : map the tangent space vector as the diffuse color
static const char*	PPLightingVPCodeTest =
"!!VP1.0																			\n\
 # compute in Projection space														\n\
 DP4 o[HPOS].x, c[0], v[0];															\n\
 DP4 o[HPOS].y, c[1], v[0];															\n\
 DP4 o[HPOS].z, c[2], v[0];															\n\
 DP4 o[HPOS].w, c[3], v[0];															\n\
 MOV o[COL0], v[9];																	\n\
 END																				\n\
";
***************************************************************/

class CVertexProgramPerPixelLight : public CVertexProgramLighted
{
public:
	struct CIdx
	{
		/// Position or direction of strongest light
		uint StrongestLight;
		/// Viewer position
		uint ViewerPos;
	};
	CVertexProgramPerPixelLight(uint vp);
	virtual ~CVertexProgramPerPixelLight() { };
	virtual void buildInfo();
	const CIdx &idx() const { return m_Idx; }

private:
	CIdx m_Idx;

};

CVertexProgramPerPixelLight::CVertexProgramPerPixelLight(uint vp)
{
	// lighted settings
	m_FeaturesLighted.SupportSpecular = (vp & 2) != 0;
	m_FeaturesLighted.NumActivePointLights = MaxLight - 1;
	m_FeaturesLighted.Normalize = false;
	m_FeaturesLighted.CtStartNeLVP = VPLightConstantStart;

	// nelvp
	{
		// Gives each vp name
		// Bit 0 : 1 when it is a directionnal light
		// Bit 1 : 1 when specular is needed
		// Bit 2 : 1 when normalization of the tangent space is needed
		static const char *vpName[] =
		{
			//  no spec
			PPLightingDirectionnalNoSpecVPCodeBegin,
			PPLightingNoSpecVPCodeBegin,
			// specular
			PPLightingDirectionnalVPCodeBegin,
			PPLightingVPCodeBegin,
			/////////////// normalized versions
			// no spec
			PPLightingDirectionnalNoSpecVPNormalizeCodeBegin,
			PPLightingVPNormalizeNoSpecCodeBegin,
			// spec
			PPLightingDirectionnalVPNormalizeCodeBegin,
			PPLightingVPNormalizeCodeBegin,
		};

		uint numvp  = sizeof(vpName) / sizeof(const char *);
		nlassert(CMeshVPPerPixelLight::NumVp == numvp); // make sure that it is in sync with header..todo : compile time assert :)

		// \todo yoyo TODO_OPTIM Manage different number of pointLights
		// NB: never call getLightVPFragmentNeLVP() with normalize, because already done by PerPixel fragment before.
		std::string vpCode	= std::string(vpName[vp])
							  + std::string("# ***************") // temp for debug
							  + CRenderTrav::getLightVPFragmentNeLVP(
									m_FeaturesLighted.NumActivePointLights, 
									m_FeaturesLighted.CtStartNeLVP, 
									m_FeaturesLighted.SupportSpecular,  
									m_FeaturesLighted.Normalize)
							  + std::string("# ***************") // temp for debug
							  + std::string(PPLightingVPCodeEnd);
		#ifdef NL_DEBUG
			/** For test : parse those programs before they are used.
			  * As a matter of fact some program will works with the NV_VERTEX_PROGRAM extension,
			  * but won't with EXT_vertex_shader, because there are some limitations (can't read a temp
			  * register that hasn't been written before..)
			  */
			CVPParser			vpParser;
			CVPParser::TProgram result;
			std::string          parseOutput;
			if (!vpParser.parse(vpCode.c_str(), result, parseOutput))
			{
				nlwarning(parseOutput.c_str());
				nlassert(0);
			}
		#endif

		CSource *source = new CSource();
		source->DisplayName = NLMISC::toString("nelvp/MeshVPPerPixel/%i", vp);
		source->Profile = CVertexProgram::nelvp;
		source->setSource(vpCode);
		source->ParamIndices["modelViewProjection"] = 0;
		addSource(source);
	}

	// glsl
	{
		// TODO_VP_GLSL
	}
}

void CVertexProgramPerPixelLight::buildInfo()
{
	CVertexProgramLighted::buildInfo();
	if (profile() == nelvp)
	{
		m_Idx.StrongestLight = 4;
		if (m_FeaturesLighted.SupportSpecular)
		{
			m_Idx.ViewerPos = 5;
		}
		else
		{
			m_Idx.ViewerPos = std::numeric_limits<uint>::max();
		}
	}
	else
	{
		// TODO_VP_GLSL
	}
	nlassert(m_Idx.StrongestLight != std::numeric_limits<uint>::max());
	if (m_FeaturesLighted.SupportSpecular)
	{
		nlassert(m_Idx.ViewerPos !=std::numeric_limits<uint>::max());
	}
}


//=================================================================================
void	CMeshVPPerPixelLight::initInstance(CMeshBaseInstance *mbi)
{
	// init the vertexProgram code.
	static	bool	vpCreated= false;
	if (!vpCreated)
	{
		vpCreated = true;
		
		for (uint vp = 0; vp < NumVp; ++vp)
		{
			_VertexProgram[vp] = new CVertexProgramPerPixelLight(vp);
		}
	}
}

//=================================================================================
bool	CMeshVPPerPixelLight::begin(IDriver *drv,
									CScene *scene, CMeshBaseInstance *mbi,
									const NLMISC::CMatrix &invertedModelMat,
									const NLMISC::CVector &viewerPos)
{
	// test if supported by driver
	if (drv->isVertexProgramEmulated()
		|| !drv->supportPerPixelLighting(SpecularLighting))
	{
		return false;
	}
	//
	enable(true, drv); // must enable the vertex program before the vb is activated
	CVertexProgramPerPixelLight *program = _ActiveVertexProgram;
	if (!program)
	{
		// failed to compile vertex program
		return false;
	}
	//
	CRenderTrav		*renderTrav= &scene->getRenderTrav();
	/// Setup for gouraud lighting
	renderTrav->prepareVPLightSetup();
	renderTrav->beginVPLightSetup(program, invertedModelMat);
	//
	sint strongestLightIndex = renderTrav->getStrongestLightIndex();
	if (strongestLightIndex == -1) return false; // if no strongest light, disable this vertex program
	// setup the strongest light
	///\todo disabling of specular lighting with this shader
	const CLight &strongestLight  = renderTrav->getDriverLight(strongestLightIndex);

	switch (strongestLight.getMode())
	{
		case CLight::DirectionalLight:
		{
			// put light direction in object space
			NLMISC::CVector lPos = invertedModelMat.mulVector(strongestLight.getDirection());
			drv->setUniform3f(IDriver::VertexProgram, program->idx().StrongestLight, lPos);
			_IsPointLight = false;
		}
		break;
		case CLight::PointLight:
		{
			// put light in object space
			NLMISC::CVector lPos = invertedModelMat * strongestLight.getPosition();
			drv->setUniform3f(IDriver::VertexProgram, program->idx().StrongestLight, lPos);
			_IsPointLight = true;
		}
		break;
		default:
			return false;
		break;
	}


	if (SpecularLighting)
	{
		// viewer pos in object space
		NLMISC::CVector vPos = invertedModelMat * viewerPos;
		drv->setUniform3f(IDriver::VertexProgram, program->idx().ViewerPos, vPos);
	}

	// c[0..3] take the ModelViewProjection Matrix. After setupModelMatrix();
	drv->setUniformMatrix(IDriver::VertexProgram, program->getUniformIndex(CProgramIndex::ModelViewProjection), IDriver::ModelViewProjection, IDriver::Identity);

	return true;
}


//=================================================================================
void	CMeshVPPerPixelLight::end(IDriver *drv)
{
	enable(false, drv);
}


//=================================================================================
void	CMeshVPPerPixelLight::serial(NLMISC::IStream &f)
{
	(void)f.serialVersion(0);
	f.serial(SpecularLighting);
}


//=================================================================================
void	CMeshVPPerPixelLight::enable(bool enabled, IDriver *drv)
{
	if (enabled != _Enabled)
	{
		nlassert(drv);
		if (enabled)
		{
			/* for (uint k = 0; k < NumVp; ++k)
			{
				nlinfo("test vp %d", k);
				drv->activeVertexProgram(_VertexProgram[k].get());
			} */
			uint	idVP =   (drv->isForceNormalize() ? 4 : 0)
						   | (SpecularLighting	      ? 2 : 0)
						   | (_IsPointLight		      ? 1 : 0);
			//
			if (drv->activeVertexProgram((CVertexProgramPerPixelLight *)_VertexProgram[idVP]))
			{
				_ActiveVertexProgram = _VertexProgram[idVP];
			}
			else
			{
				_ActiveVertexProgram = NULL;
			}
		}
		else
		{
			drv->activeVertexProgram(NULL);
			_ActiveVertexProgram = NULL;
		}
		_Enabled = enabled;
	}
}

//=================================================================================
bool  CMeshVPPerPixelLight::setupForMaterial(const CMaterial &mat,
											 IDriver *drv,
											 CScene *scene
											)
{
	bool enabled = (mat.getShader() == CMaterial::PerPixelLighting || mat.getShader() == CMaterial::PerPixelLightingNoSpec);
	bool change = (enabled != _Enabled);
	enable(enabled, drv); // enable disable the vertex program (for material that don't have the right shader)
	if (enabled)
	{
		CRenderTrav		*renderTrav= &scene->getRenderTrav();
		renderTrav->changeVPLightSetupMaterial(mat, true /* exclude strongest*/);

		NLMISC::CRGBA pplDiffuse, pplSpecular;
		renderTrav->getStrongestLightColors(pplDiffuse, pplSpecular);
		drv->setPerPixelLightingLight(pplDiffuse, pplSpecular, mat.getShininess());
	}
	return change;
}
//=================================================================================
void	CMeshVPPerPixelLight::setupForMaterial(const CMaterial &mat,
											   IDriver *drv,
											   CScene *scene,
											   CVertexBuffer *vb)
{

	if (setupForMaterial(mat, drv, scene)) // a switch from v.p enabled / disabled force to reactivate the vertex buffer.
	{
		drv->activeVertexBuffer(*vb);
	}
}


} // NL3D
