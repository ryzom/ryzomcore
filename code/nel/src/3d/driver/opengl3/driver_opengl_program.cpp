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

#include "driver_opengl.h"
#include "driver_opengl_program.h"
#include "driver_glsl_shader_generator.h"
#include "driver_opengl_vertex_buffer_hard.h"
#include "nel/3d/dynamic_material.h"
#include "nel/3d/usr_shader_manager.h"
#include "nel/3d/usr_shader_program.h"

namespace NL3D {

#ifdef NL_STATIC
namespace NLDRIVERGL3 {
#endif

const uint16 g_VertexFlags[CVertexBuffer::NumValue] = 
{
	CVertexBuffer::PositionFlag,
	CVertexBuffer::WeightFlag,
	CVertexBuffer::NormalFlag,
	CVertexBuffer::PrimaryColorFlag,
	CVertexBuffer::SecondaryColorFlag,
	CVertexBuffer::FogFlag,
	CVertexBuffer::PaletteSkinFlag,
	0,
	CVertexBuffer::TexCoord0Flag,
	CVertexBuffer::TexCoord1Flag,
	CVertexBuffer::TexCoord2Flag,
	CVertexBuffer::TexCoord3Flag,
	CVertexBuffer::TexCoord4Flag,
	CVertexBuffer::TexCoord5Flag,
	CVertexBuffer::TexCoord6Flag,
	CVertexBuffer::TexCoord7Flag
};

const char *g_AttribNames[CVertexBuffer::NumValue] =
{
	"position",
	"weight",
	"normal",
	"primaryColor",
	"secondaryColor",
	"fog",
	"paletteSkin",
	"none",
	"texCoord0",
	"texCoord1",
	"texCoord2",
	"texCoord3",
	"texCoord4",
	"texCoord5",
	"texCoord6",
	"texCoord7"
};

const char *g_TexelNames[IDRV_MAT_MAXTEXTURES] =
{
	"texel0",
	"texel1",
	"texel2",
	"texel3"
};

const char *g_ConstantNames[4] =
{
	"constant0",
	"constant1",
	"constant2",
	"constant3"
};

bool CDriverGL3::supportVertexProgram(CVertexProgram::TProfile profile) const
{
	return (profile == IProgram::glsl330v);
}

bool CDriverGL3::compileVertexProgram(CVertexProgram *program)
{
	if (program->m_DrvInfo != NULL)
		return false;

	IProgram::CSource *src = NULL;
	for (int i = 0; i < program->getSourceNb(); i++)
	{
		src = program->getSource(i);
		if (src->Profile == IProgram::glsl330v)
			break;
		
		src = NULL;
	}
	if (src == NULL)
		return false;

	const char *s = src->SourcePtr;
	glGetError();
	unsigned int id = nglCreateShaderProgramv(GL_VERTEX_SHADER, 1, &s);

	if (id == 0)
		return false;

	GLint ok;
	nglGetProgramiv(id, GL_LINK_STATUS, &ok);
	if (ok == 0)
	{
		char errorLog[ 1024 ];
		nglGetProgramInfoLog(id, 1024, NULL, errorLog);
		nlwarning("GL3: %s", errorLog);
		std::vector<std::string> lines;
		NLMISC::explode(std::string(src->SourcePtr), std::string("\n"), lines);
		for (std::vector<std::string>::size_type i = 0; i < lines.size(); ++i)
		{
			nldebug("GL3: %i: %s", i, lines[i].c_str());
		}
		return false;
	}
	else // debug
	{
		std::vector<std::string> lines;
		NLMISC::explode(std::string(src->SourcePtr), std::string("\n"), lines);
		for (std::vector<std::string>::size_type i = 0; i < lines.size(); ++i)
		{
			nldebug("GL3: %i: %s", i, lines[i].c_str());
		}
	}

	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
		return false;

	ItGPUPrgDrvInfoPtrList it = _GPUPrgDrvInfos.insert(_GPUPrgDrvInfos.end(),(NL3D::IProgramDrvInfos*)NULL);
	CProgramDrvInfosGL3 *drvInfo = new CProgramDrvInfosGL3(this, it);
	*it = drvInfo;
	program->m_DrvInfo = drvInfo;
	drvInfo->setProgramId(id);

	program->buildInfo(src);

	setupInitialUniforms(program);

	return true;
}

bool CDriverGL3::activeVertexProgram(CVertexProgram *program)
{
	return activeVertexProgram(program, false);
}

bool CDriverGL3::activeVertexProgram(CVertexProgram *program, bool driver)
{
	if (driver) nlassert(m_UserVertexProgram == NULL);

	if (m_DriverVertexProgram == program)
		return true;

	if (program == NULL)
	{
		nglUseProgramStages(ppoId, GL_VERTEX_SHADER_BIT, 0);
		m_UserVertexProgram = NULL;
		m_DriverVertexProgram = NULL;
		return true;
	}

	IProgramDrvInfos *di = program->m_DrvInfo;
	CProgramDrvInfosGL3 *drvInfo = dynamic_cast< CProgramDrvInfosGL3* >(di);
	if (drvInfo == NULL)
	{
		m_UserVertexProgram = NULL;
		m_DriverVertexProgram = NULL;
		return false;
	}
	glGetError();

	nglUseProgramStages(ppoId, GL_VERTEX_SHADER_BIT, drvInfo->getProgramId());
	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
	{
		m_UserVertexProgram = NULL;
		m_DriverVertexProgram = NULL;
		return false;
	}

	if (!driver) m_UserVertexProgram = program;
	m_DriverVertexProgram = program;
	return true;
}

bool CDriverGL3::supportPixelProgram(IProgram::TProfile profile) const
{
	if (profile == IProgram::glsl330f)
		return true;
	else
		return false;
}

bool CDriverGL3::compilePixelProgram(CPixelProgram *program)
{
	if (program->m_DrvInfo != NULL)
		return false;

	IProgram::CSource *src = NULL;

	for (int i = 0; i < program->getSourceNb(); i++)
	{
		src = program->getSource(i);
		if (src->Profile == IProgram::glsl330f)
			break;

		src = NULL;
	}

	if (src == NULL)
		return false;

	const char *s = src->SourcePtr;
	glGetError();
	unsigned int id = nglCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &s);
	if (id == 0)
		return false;

	GLint ok;
	nglGetProgramiv(id, GL_LINK_STATUS, &ok);
	if (ok == 0)
	{
		char errorLog[ 1024 ];
		nglGetProgramInfoLog(id, 1024, NULL, errorLog);
		nlwarning("GL3: %s", errorLog);
		std::vector<std::string> lines;
		NLMISC::explode(std::string(src->SourcePtr), std::string("\n"), lines);
		for (std::vector<std::string>::size_type i = 0; i < lines.size(); ++i)
		{
			nldebug("GL3: %i: %s", i, lines[i].c_str());
		}
		return false;
	}
	else // debug
	{
		std::vector<std::string> lines;
		NLMISC::explode(std::string(src->SourcePtr), std::string("\n"), lines);
		for (std::vector<std::string>::size_type i = 0; i < lines.size(); ++i)
		{
			nldebug("GL3: %i: %s", i, lines[i].c_str());
		}
	}

	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
		return false;

	ItGPUPrgDrvInfoPtrList it = _GPUPrgDrvInfos.insert(_GPUPrgDrvInfos.end(), (NL3D::IProgramDrvInfos*)NULL);
	CProgramDrvInfosGL3 *drvInfo = new CProgramDrvInfosGL3(this, it);
	*it = drvInfo;
	drvInfo->setProgramId(id);
	program->m_DrvInfo = drvInfo;

	program->buildInfo(src);

	setupInitialUniforms(program);

	return true;
}

bool CDriverGL3::activePixelProgram(CPixelProgram *program)
{
	return activePixelProgram(program, false);
}

bool CDriverGL3::activePixelProgram(CPixelProgram *program, bool driver)
{
	if (driver) nlassert(m_UserPixelProgram == NULL);

	if (m_DriverPixelProgram == program)
		return true;

	if (program == NULL)
	{
		nglUseProgramStages(ppoId, GL_FRAGMENT_SHADER_BIT, 0);
		m_UserPixelProgram = NULL;
		m_DriverPixelProgram = NULL;
		return true;
	}

	if (program->m_DrvInfo == NULL)
	{
		m_UserPixelProgram = NULL;
		m_DriverPixelProgram = NULL;
		return false;
	}
	
	IProgramDrvInfos *di = program->m_DrvInfo;
	CProgramDrvInfosGL3 *drvInfo = dynamic_cast< CProgramDrvInfosGL3* >(di);
	if (drvInfo == NULL)
	{
		m_UserPixelProgram = NULL;
		m_DriverPixelProgram = NULL;
		return false;
	}
	glGetError();

	nglUseProgramStages(ppoId, GL_FRAGMENT_SHADER_BIT, drvInfo->getProgramId());
	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
	{
		m_UserPixelProgram = NULL;
		m_DriverPixelProgram = NULL;
		return false;
	}

	if (!driver) m_UserPixelProgram = program;
	m_DriverPixelProgram = program;
	return true;
}


uint32 CDriverGL3::getProgramId(TProgram program) const
{
	uint32 id = 0;

	switch(program)
	{
	case IDriver::VertexProgram:
		if (m_DriverVertexProgram)
		{
			IProgramDrvInfos *di = m_DriverVertexProgram->m_DrvInfo;
			CProgramDrvInfosGL3 *drvInfo = dynamic_cast< CProgramDrvInfosGL3* >(di);
			if (drvInfo != NULL)
				id = drvInfo->getProgramId();
		}
		break;

	case IDriver::PixelProgram:
		if (m_DriverPixelProgram)
		{
			IProgramDrvInfos *di = m_DriverPixelProgram->m_DrvInfo;
			CProgramDrvInfosGL3 *drvInfo = dynamic_cast< CProgramDrvInfosGL3* >(di);
			if (drvInfo != NULL)
				id = drvInfo->getProgramId();
		}
		break;

	case IDriver::GeometryProgram:
		break;
	}

	return id;
}

IProgram* CDriverGL3::getProgram(TProgram program) const
{
	switch(program)
	{
	case IDriver::VertexProgram:
		return m_DriverVertexProgram;
	case IDriver::PixelProgram:
		return m_DriverPixelProgram;
	case IDriver::GeometryProgram:
		return m_DriverGeometryProgram;
	}

	return NULL;
}

int CDriverGL3::getUniformLocation(TProgram program, const char *name)
{
	uint32 id = getProgramId(program);
	return nglGetUniformLocation(id, name);
}

void CDriverGL3::setUniform1f(TProgram program, uint index, float f0)
{
	uint32 id = getProgramId(program);
	nglProgramUniform1f(id, index, f0);
}

void CDriverGL3::setUniform2f(TProgram program, uint index, float f0, float f1)
{
	uint32 id = getProgramId(program);
	nglProgramUniform2f(id, index, f0, f1);
}

void CDriverGL3::setUniform3f(TProgram program, uint index, float f0, float f1, float f2)
{
	uint32 id = getProgramId(program);
	nglProgramUniform3f(id, index, f0, f1, f2);
}

void CDriverGL3::setUniform4f(TProgram program, uint index, float f0, float f1, float f2, float f3)
{
	uint32 id = getProgramId(program);
	nglProgramUniform4f(id, index, f0, f1, f2, f3);
}

void CDriverGL3::setUniform1i(TProgram program, uint index, sint32 i0)
{
	uint32 id = getProgramId(program);
	nglProgramUniform1i(id, index, i0);
}

void CDriverGL3::setUniform2i(TProgram program, uint index, sint32 i0, sint32 i1)
{
	uint32 id = getProgramId(program);
	nglProgramUniform2i(id, index, i0, i1);
}

void CDriverGL3::setUniform3i(TProgram program, uint index, sint32 i0, sint32 i1, sint32 i2)
{
	uint32 id = getProgramId(program);
	nglProgramUniform3i(id, index, i0, i1, i2);
}

void CDriverGL3::setUniform4i(TProgram program, uint index, sint32 i0, sint32 i1, sint32 i2, sint32 i3)
{
	uint32 id = getProgramId(program);
	nglProgramUniform4i(id, index, i0, i1, i2, i3);
}

void CDriverGL3::setUniform1ui(TProgram program, uint index, uint32 ui0)
{
	uint32 id = getProgramId(program);
	nglProgramUniform1ui(id, index, ui0);
}

void CDriverGL3::setUniform2ui(TProgram program, uint index, uint32 ui0, uint32 ui1)
{
	uint32 id = getProgramId(program);
	nglProgramUniform2ui(id, index, ui0, ui1);
}

void CDriverGL3::setUniform3ui(TProgram program, uint index, uint32 ui0, uint32 ui1, uint32 ui2)
{
	uint32 id = getProgramId(program);
	nglProgramUniform3ui(id, index, ui0, ui1, ui2);
}

void CDriverGL3::setUniform4ui(TProgram program, uint index, uint32 ui0, uint32 ui1, uint32 ui2, uint32 ui3)
{
	uint32 id = getProgramId(program);
	nglProgramUniform4ui(id, index, ui0, ui1, ui2, ui3);
}

void CDriverGL3::setUniform3f(TProgram program, uint index, const CVector &v)
{
	uint32 id = getProgramId(program);
	nglProgramUniform3f(id, index, v.x, v.y, v.z);
}

void CDriverGL3::setUniform4f(TProgram program, uint index, const CVector &v, float f3)
{
	uint32 id = getProgramId(program);
	nglProgramUniform4f(id, index, v.x, v.y, v.z, f3);
}

void CDriverGL3::setUniform4f(TProgram program, uint index, const NLMISC::CRGBAF& rgba)
{
	uint32 id = getProgramId(program);
	nglProgramUniform4f(id, index, rgba.R, rgba.G, rgba.B, rgba.A);
}

void CDriverGL3::setUniform3x3f(TProgram program, uint index, const float *src)
{
	uint32 id = getProgramId(program);
	nglProgramUniformMatrix3fv(id, index, 1, false, src);
}

void CDriverGL3::setUniform4x4f(TProgram program, uint index, const CMatrix &m)
{
	uint32 id = getProgramId(program);
	nglProgramUniformMatrix4fv(id, index, 1, false, m.get());
}

void CDriverGL3::setUniform4x4f(TProgram program, uint index, const float *src)
{
	uint32 id = getProgramId(program);
	nglProgramUniformMatrix4fv(id, index, 1, false, src);
}

void CDriverGL3::setUniform4fv(TProgram program, uint index, size_t num, const float *src)
{
	uint32 id = getProgramId(program);
	nglProgramUniform4fv(id, index, num, src);
}

void CDriverGL3::setUniform4iv(TProgram program, uint index, size_t num, const sint32 *src)
{
	uint32 id = getProgramId(program);
	nglProgramUniform4iv(id, index, num, src);
}

void CDriverGL3::setUniform4uiv(TProgram program, uint index, size_t num, const uint32 *src)
{
	uint32 id = getProgramId(program);
	nglProgramUniform4uiv(id, index, num, src);
}

void CDriverGL3::setUniformMatrix(TProgram program, uint index, TMatrix matrix, TTransform transform)
{
	uint32 id = getProgramId(program);
	CMatrix mat;
	

	switch(matrix)
	{
	case IDriver::ModelView:
		mat = _ModelViewMatrix;
		break;
	case IDriver::Projection:
		mat = _GLProjMat;
		break;
	case IDriver::ModelViewProjection:
		mat = _ModelViewMatrix * _GLProjMat;
		break;
	}

	switch(transform)
	{
	case IDriver::Inverse:
		mat.invert();
		break;
	case IDriver::Transpose:
		mat.transpose();
		break;
	case IDriver::InverseTranspose:
		mat.transpose();
		mat.invert();
		break;
	}


	nglProgramUniformMatrix4fv(id, index, 1, false, mat.get());
}

void CDriverGL3::setUniformFog(TProgram program, uint index)
{
	uint32 id = getProgramId(program);
	const float *v = _ModelViewMatrix.get();
	nglProgramUniform4f(id, index, -v[ 2 ], -v[ 6 ], -v[ 10 ], -v[ 4 ]);
}

void CDriverGL3::generateShaderDesc(CShaderDesc &desc, CMaterial &mat)
{
	desc.setShaderType(mat.getShader());
	uint16 vbFlags = _CurrentVertexBufferHard->VB->getVertexFormat();
	for (sint i = 0; i < IDRV_MAT_MAXTEXTURES; ++i)
	{
		if (m_VPBuiltinCurrent.TexGenMode[i] >= 0)
		{
			//nldebug("texgen hack for keeping pp simpler, stage %i, tex %s valid", i, mat.getTexture(i) ? "IS" : "NOT");
			vbFlags |= g_VertexFlags[TexCoord0 + i];
		}
	}
	desc.setVBFlags(vbFlags);
	
	if (mat.getShader() == CMaterial::LightMap)
		desc.setNLightMaps(mat._LightMaps.size());
	
	//int i = 0;

	if (mat.getShader() == CMaterial::Normal
		|| mat.getShader() == CMaterial::UserColor
		|| mat.getShader() == CMaterial::Specular)
	{
		bool useTextures = false;

		int maxTextures = std::min(int(IDRV_MAT_MAXTEXTURES), int(IDRV_MAT_MAXTEXTURES));

		if (mat.getShader() == CMaterial::Normal)
		{
			for (int i = 0; i < maxTextures; i++)
			{
				desc.setTexEnvMode(i, mat.getTexEnvMode(i));
			} // todo specular env mode..
		}

		for (int i = 0; i < maxTextures; i++)
		{
			// GL3 TEX COORD
			if (mat.getTexture(i) != NULL 
				&& (desc.hasVBFlags(g_VertexFlags[TexCoord0]) || desc.hasVBFlags(g_VertexFlags[TexCoord0 + i]))
				)
			{
				//nldebug("use stage %i", i);

				desc.setUseTexStage(i, true);
				useTextures = true;

				desc.textureSamplerMode[i] = mat.getTexture(i)->isTextureCube() ? SamplerCube : Sampler2D; // Driver state
			}
			else
			{
				/*nldebug("stage fail %i, tex %s, tc0 %s, tci, %s", i, mat.getTexture(i) ? "VALID" : "NO", 
					desc.hasVBFlags(g_VertexFlags[TexCoord0]) ? "YES" : "NO",
					desc.hasVBFlags(g_VertexFlags[TexCoord0 + i]) ? "YES" : "NO");*/
			}
		}

		if (!useTextures)
		{
			desc.setNoTextures(true);
		}
	}

	if (mat.getAlphaTest())
	{
		desc.setAlphaTest(true);
		desc.setAlphaTestThreshold(mat.getAlphaTestThreshold());
	}

	if (m_VPBuiltinCurrent.Fog) // Driver state
	{
		desc.setFog(true);
		desc.setFogMode(CShaderDesc::Linear);
	}

	//bool enableLights = false;
	for (int i = 0; i < MaxLight; i++)
	{
		if (!_UserLightEnable[i])
		{
			desc.setLight(i, CShaderDesc::Nolight);
			continue;
		}
		//if (!_UserLightEnable[ i ])
		//	continue;

		//enableLights = true;
		
		switch(_LightMode[ i ])
		{
		case CLight::DirectionalLight:
			desc.setLight(i, CShaderDesc::Directional);
			break;
		
		case CLight::PointLight:
			desc.setLight(i, CShaderDesc::Point);
			break;
		
		case CLight::SpotLight:
			desc.setLight(i, CShaderDesc::Spot);
			break;
		}
	
	}

	desc.setLighting(/*enableLights && mat.isLighted() &&*/ m_VPBuiltinCurrent.Lighting);			
}

bool CDriverGL3::setupBuiltinPrograms(CMaterial &mat)
{
	return setupBuiltinVertexProgram()
		&& setupBuiltinPixelProgram(mat)
		&& setupUniforms();
}

bool CDriverGL3::setupBuiltinVertexProgram()
{
	if (m_UserVertexProgram) return true;

	touchVertexFormatVP(); // TODO

	if (m_VPBuiltinTouched)
	{
		generateBuiltinVertexProgram();
		nlassert(m_VPBuiltinCurrent.VertexProgram);
		m_VPBuiltinTouched = false;
	}

	if (!activeVertexProgram(m_VPBuiltinCurrent.VertexProgram, true))
		return false;

	// GL3 TODO: Here we set the uniforms of the vertex program!

	return true;
}

bool CDriverGL3::setupBuiltinPixelProgram(CMaterial &mat)
{
	if (m_UserPixelProgram) return true;

	CMaterialDrvInfosGL3 *matDrv = static_cast<CMaterialDrvInfosGL3 *>((IMaterialDrvInfos *)(mat._MatDrvInfo));
	nlassert(matDrv);

	matDrv->PPBuiltin.checkDriverStateTouched(this);
	matDrv->PPBuiltin.checkMaterialStateTouched(mat);

	if (matDrv->PPBuiltin.Touched)
	{
		generateBuiltinPixelProgram(mat);
		nlassert(matDrv->PPBuiltin.PixelProgram);
		matDrv->PPBuiltin.Touched = false;
	}

	if (!activePixelProgram(matDrv->PPBuiltin.PixelProgram, true))
		return false;

	// GL3 TODO: Here we set the uniforms of the vertex program!

	return true;


#if 0
	// nlassert(!m_UserVertexProgram); // TEMP
	// nlassert(!m_UserPixelProgram); // TEMP

	if (mat.getDynMat() != NULL)
		return true;
	
	// CVertexProgram *vp = NULL; // REMOVED
	CPixelProgram *pp = NULL;
	SShaderPair sp;

	CShaderDesc desc;

	generateShaderDesc(desc, mat);

	// See if we've already generated and compiled this shader
	sp = shaderCache.findShader(desc);

	// Yes we have!
	if (!sp.empty())
	{
		/*if (m_UserVertexProgram == NULL)
		{
			if (!activeVertexProgram(sp.vp, true))
				return false;
		}*/

		if (m_UserPixelProgram == NULL)
		{
			if (!activePixelProgram(sp.pp, true))
				return false;
		}
	}
	// No we need to generate it now
	else
	{
		// std::string vs;
		std::string ps;
		bool cacheShaders = true;

		shaderGenerator->reset();
		shaderGenerator->setMaterial(&mat);
		shaderGenerator->setShaderDesc(&desc);
		
		// If we don't already have a vertex program attached, we'll generate it now
		/* if (m_UserVertexProgram == NULL)
		{
			shaderGenerator->generateVS(vs);
			vp = new CVertexProgram();
			{
				IProgram::CSource *src = new IProgram::CSource();
				src->Profile = IProgram::glsl330v;
				src->DisplayName = "";
				src->setSource(vs);
				vp->addSource(src);
			}

			if (!compileVertexProgram(vp))
			{
				delete vp;
				vp = NULL;
				return false;
			}

			if (!activeVertexProgram(vp, true))
			{
				delete vp;
				vp = NULL;
				return false;
			}
		}
		else
			cacheShaders = false;
		*/
	
		// If we don't already have a pixel program attached, we'll generate it now
		if (m_UserPixelProgram == NULL)
		{
			shaderGenerator->generatePS(ps);
			pp = new CPixelProgram();
			{
				IProgram::CSource *src = new IProgram::CSource();
				src->Profile = IProgram::glsl330f;
				src->DisplayName = "";
				src->setSource(ps);
				pp->addSource(src);
			}
		
			if (!compilePixelProgram(pp))
			{
				// delete vp;
				// vp = NULL;
				delete pp;
				pp = NULL;
				return false;
			}

			if (!activePixelProgram(pp, true))
			{
				// delete vp;
				// vp = NULL;
				delete pp;
				pp = NULL;
				return false;
			}
		}
		else
			cacheShaders = false;

	
		// If we already have a shader attached we won't cache this shaderpair, since we didn't generate it
		if (cacheShaders)
		{
			sp.vp = NULL;
			sp.pp = pp;
			desc.setShaders(sp);
			shaderCache.cacheShader(desc);
		}
	}

	return true;
#endif
}

bool CDriverGL3::setupDynMatProgram(CMaterial& mat, uint pass)
{
	/*
	if ((currentProgram.vp != NULL) && (currentProgram.pp != NULL))
		return true;

	CDynMaterial *m = mat.getDynMat();
	const SRenderPass *rp = m->getPass(pass);
	std::string shaderRef;
	rp->getShaderRef(shaderRef);

	NL3D::CUsrShaderProgram prg;

	if (!usrShaderManager->getShader(shaderRef, &prg))
		return false;
	
	std::string shaderSource;
	std::string log;
	std::string name;

	if (currentProgram.vp == NULL)
	{
		prg.getVP(shaderSource);
		prg.getName(name);

		CVertexProgram *vp = new CVertexProgram();		
		{
			IProgram::CSource *src = new IProgram::CSource();
			src->Profile = IProgram::glsl330v;			
			src->DisplayName = name;
			src->setSource(shaderSource.c_str());
			vp->addSource(src);
		}
	
		if (!compileVertexProgram(vp))
		{
			delete vp;
			return false;
		}

		if (!activeVertexProgram(vp))
		{
			delete vp;
			return false;
		}

		if (currentProgram.dynmatVP != NULL)
			delete currentProgram.dynmatVP;
		currentProgram.dynmatVP = vp;

	}

	if (currentProgram.pp == NULL)
	{
	
		CPixelProgram *pp = new CPixelProgram();

		prg.getFP(shaderSource);
		{
			IProgram::CSource *src = new IProgram::CSource();
			src->Profile = IProgram::glsl330f;			
			src->DisplayName = name;
			src->setSource(shaderSource.c_str());
			pp->addSource(src);
		}
	
		if (!compilePixelProgram(pp))
		{
			delete pp;
			return false;
		}

		if (!activePixelProgram(pp))
		{
			delete pp;
			return false;
		}

		if (currentProgram.dynmatPP != NULL)
			delete currentProgram.dynmatPP;
		currentProgram.dynmatPP = pp;

	}

	return true;
	*/
	return false;
}

bool CDriverGL3::setupUniforms()
{
	setupUniforms(IDriver::VertexProgram);
	setupUniforms(IDriver::PixelProgram);
	return true;
}

void CDriverGL3::setupUniforms(TProgram program)
{
	CMaterial &mat = *_CurrentMaterial;
	IProgram *p = getProgram(program);
	CProgramDrvInfosGL3 *drvInfo = dynamic_cast<CProgramDrvInfosGL3 *>(&*p->m_DrvInfo);
	if (drvInfo == NULL) return;
	GLuint progId = drvInfo->getProgramId();

	uint mvpIndex = p->getUniformIndex(CProgramIndex::ModelViewProjection);
	if (mvpIndex != ~0)
	{
		CMatrix mvp = _GLProjMat * _ModelViewMatrix;
		setUniform4x4f(program, mvpIndex, mvp);
	}

	uint vmIndex = p->getUniformIndex(CProgramIndex::ViewMatrix);
	if (vmIndex != ~0)
	{
		setUniform4x4f(program, vmIndex, _ViewMtx);
	}

	uint mvIndex = p->getUniformIndex(CProgramIndex::ModelView);
	if (mvIndex != ~0)
	{
		setUniform4x4f(program, mvIndex, _ModelViewMatrix);
	}

	uint nmIdx = p->getUniformIndex(CProgramIndex::NormalMatrix);
	if (nmIdx != ~0)
	{
		// normal matrix is the inverse-transpose of the rotation part of the modelview matrix
		// Inverse-transpose of the rotation matrix, is itself
		const float *mv = _ModelViewMatrix.get();
		float nm[ 3 * 3 ];
		nm[ 0 ] = mv[ 0 ];
		nm[ 1 ] = mv[ 1 ];
		nm[ 2 ] = mv[ 2 ];
		nm[ 3 ] = mv[ 4 ];
		nm[ 4 ] = mv[ 5 ];
		nm[ 5 ] = mv[ 6 ];
		nm[ 6 ] = mv[ 8 ];
		nm[ 7 ] = mv[ 9 ];
		nm[ 8 ] = mv[ 10 ];

		setUniform3x3f(program, nmIdx, nm);
	}

	uint fogStartIdx = p->getUniformIndex(CProgramIndex::FogStart);
	if (fogStartIdx != ~0)
	{
		nglProgramUniform1f(progId, fogStartIdx, _FogStart);
	}

	uint fogEndIdx = p->getUniformIndex(CProgramIndex::FogEnd);
	if (fogEndIdx != ~0)
	{
		nglProgramUniform1f(progId, fogEndIdx, _FogEnd);
	}

	uint fogColorIdx = p->getUniformIndex(CProgramIndex::FogColor);
	if (fogColorIdx != ~0)
	{
		GLfloat glCol[ 4 ];
		CRGBA col = getFogColor();
		glCol[ 0 ] = col.R / 255.0f;
		glCol[ 1 ] = col.G / 255.0f;
		glCol[ 2 ] = col.B / 255.0f;
		glCol[ 3 ] = col.A / 255.0f;
		nglProgramUniform4f(progId, fogColorIdx, glCol[0], glCol[1], glCol[2], glCol[3]);
	}

	uint colorIndex = p->getUniformIndex(CProgramIndex::Color);
	if (colorIndex != ~0)
	{
		GLfloat glCol[ 4 ];
		CRGBA col = mat.getColor();
		glCol[ 0 ] = col.R / 255.0f;
		glCol[ 1 ] = col.G / 255.0f;
		glCol[ 2 ] = col.B / 255.0f;
		glCol[ 3 ] = col.A / 255.0f;

		nglProgramUniform4f(progId, colorIndex, glCol[ 0 ], glCol[ 1 ], glCol[ 2 ], glCol[ 3 ]);
	}

	uint diffuseIndex = p->getUniformIndex(CProgramIndex::DiffuseColor);
	if (diffuseIndex != ~0)
	{
		/*GLfloat glCol[ 4 ];
		CRGBA col = mat.getDiffuse();
		glCol[ 0 ] = col.R / 255.0f;
		glCol[ 1 ] = col.G / 255.0f;
		glCol[ 2 ] = col.B / 255.0f;
		glCol[ 3 ] = col.A / 255.0f;*/

		nglProgramUniform4f(progId, diffuseIndex, 1.0f, 1.0f, 1.0f, 0.0f);
	}

	NLMISC::CRGBAF selfIllumination = NLMISC::CRGBAF(0.0f, 0.0f, 0.0f);
	NLMISC::CRGBAF matDiffuse = NLMISC::CRGBAF(mat.getDiffuse());
	NLMISC::CRGBAF matSpecular = NLMISC::CRGBAF(mat.getSpecular());

	for (uint i = 0; i < NL_OPENGL3_MAX_LIGHT; ++i)
	{
		if (!_UserLightEnable[i])
			continue;

		selfIllumination += NLMISC::CRGBAF(_UserLight[i].getAmbiant());
		
		////////////////// Temporary insanity  ///////////////////////////////
		if ((_LightMode[i] != CLight::DirectionalLight) && (_LightMode[i] != CLight::PointLight))
			continue;
		//////////////////////////////////////////////////////////////////////
		
		if (_LightMode[i] == CLight::DirectionalLight)
		{
			uint ld = p->getUniformIndex(CProgramIndex::TName(CProgramIndex::Light0DirOrPos + i));
			if (ld != ~0)
			{
				CVector v = -1 * _UserLight[i].getDirection();
				nglProgramUniform4f(progId, ld, v.x, v.y, v.z, 0.0f);
			}
		}
		else
		{
			uint lp = p->getUniformIndex(CProgramIndex::TName(CProgramIndex::Light0DirOrPos + i));
			if (lp != ~0)
			{
				CVector v = _UserLight[i].getPosition() - _PZBCameraPos;
				nglProgramUniform4f(progId, lp, v.x, v.y, v.z, 0.0f);
			}
		}

		uint ldc = p->getUniformIndex(CProgramIndex::TName(CProgramIndex::Light0ColDiff + i));
		if (ldc != ~0)
		{
			NLMISC::CRGBAF diffuse = NLMISC::CRGBAF(_UserLight[i].getDiffuse()) * matDiffuse;
			nglProgramUniform4f(progId, ldc, diffuse.R, diffuse.G, diffuse.B, 0.0f); // 1.0f?
		}

		uint lsc = p->getUniformIndex(CProgramIndex::TName(CProgramIndex::Light0ColSpec + i));
		if (lsc != ~0)
		{
			NLMISC::CRGBAF specular = NLMISC::CRGBAF(_UserLight[i].getSpecular()) * matSpecular;
			nglProgramUniform4f(progId, lsc, specular.R, specular.G, specular.B, 0.0f); // 1.0f?
		}

		uint shl = p->getUniformIndex(CProgramIndex::TName(CProgramIndex::Light0Shininess + i));
		if (shl != ~0)
		{
			nglProgramUniform1f(progId, shl, mat.getShininess());
		}

		/*int lac = p->getUniformIndex(CProgramIndex::TName(CProgramIndex::Light0ColAmb + i));
		if (lac >= 0)
		{
			GLfloat glCol[ 4 ];
			CRGBA col;
			if (mat.getShader() == CMaterial::LightMap)
				col = _UserLight[ i ].getAmbiant();
			else
				col.add(_UserLight[ i ].getAmbiant(), mat.getEmissive());

			glCol[ 0 ] = col.R / 255.0f;
			glCol[ 1 ] = col.G / 255.0f;
			glCol[ 2 ] = col.B / 255.0f;
			glCol[ 3 ] = col.A / 255.0f;
			setUniform4f(program, lac, glCol[ 0 ], glCol[ 1 ], glCol[ 2 ], glCol[ 3 ]);
		}*/

		uint lca = p->getUniformIndex(CProgramIndex::TName(CProgramIndex::Light0ConstAttn + i));
		if (lca != ~0)
		{
			nglProgramUniform1f(progId, lca, _UserLight[ i ].getConstantAttenuation());
		}

		uint lla = p->getUniformIndex(CProgramIndex::TName(CProgramIndex::Light0LinAttn + i));
		if (lla != ~0)
		{
			nglProgramUniform1f(progId, lla, _UserLight[ i ].getLinearAttenuation());
		}

		uint lqa = p->getUniformIndex(CProgramIndex::TName(CProgramIndex::Light0QuadAttn + i));
		if (lqa != ~0)
		{
			nglProgramUniform1f(progId, lqa, _UserLight[ i ].getQuadraticAttenuation());
		}
	}

	selfIllumination *= NLMISC::CRGBAF(mat.getAmbient());
	if (mat.getShader() != CMaterial::LightMap) // Really?
		selfIllumination += NLMISC::CRGBAF(mat.getEmissive());
	int selfIlluminationId = p->getUniformIndex(CProgramIndex::TName(CProgramIndex::SelfIllumination));
	if (selfIlluminationId != -1)
	{
		nglProgramUniform4f(progId, selfIlluminationId, selfIllumination.R, selfIllumination.G, selfIllumination.B, 0.0f);
	}

	// Lightmaps have special constants
	if (mat.getShader() == CMaterial::Normal
		|| mat.getShader() == CMaterial::UserColor)
	{
		for (uint stage = 0; stage < IDRV_MAT_MAXTEXTURES; ++stage)
		{
			int cl = p->getUniformIndex(CProgramIndex::TName(CProgramIndex::Constant0 + stage));
			if (cl != -1)
			{
				CRGBA col = mat._TexEnvs[stage].ConstantColor;
				GLfloat glCol[4];
				glCol[0] = col.R / 255.0f;
				glCol[1] = col.G / 255.0f;
				glCol[2] = col.B / 255.0f;
				glCol[3] = col.A / 255.0f;

				nglProgramUniform4f(progId, cl, glCol[0], glCol[1], glCol[2], glCol[3]);
			}
		}
	}
}

void CDriverGL3::setupInitialUniforms(IProgram *program)
{
	IProgramDrvInfos *di = program->m_DrvInfo;
	CProgramDrvInfosGL3 *drvInfo = dynamic_cast<CProgramDrvInfosGL3 *>(di);
	if (drvInfo != NULL)
	{
		GLuint id = drvInfo->getProgramId();
	
		for (uint i = 0; i < IDRV_MAT_MAXTEXTURES; ++i)
		{
			uint samplerIdx = program->getUniformIndex((CProgramIndex::TName)(CProgramIndex::Sampler0 + i));
			if (samplerIdx >= 0)
				nglProgramUniform1i(id, samplerIdx, i);
		}
	}
}

bool CDriverGL3::initProgramPipeline()
{
	ppoId = 0;

	nglGenProgramPipelines(1, &ppoId);
	if (ppoId == 0)
		return false;

	nglBindProgramPipeline(ppoId);

	return true;
}

#ifdef NL_STATIC
} // NLDRIVERGL3
#endif

} // NL3D

/* end of file */
