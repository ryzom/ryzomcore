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
#include "driver_glsl_shader_generator.h"
#include "driver_opengl_vertex_buffer_hard.h"
#include "nel/3d/dynamic_material.h"
#include "nel/3d/usr_shader_manager.h"
#include "nel/3d/usr_shader_program.h"

namespace
{
	const char *constNames[ NL3D::IDRV_MAT_MAXTEXTURES ] =
	{
		"constant0",
		"constant1",
		"constant2",
		"constant3"
	};


	uint16 vertexFlags[ NL3D::CVertexBuffer::NumValue ] = 
	{
		NL3D::CVertexBuffer::PositionFlag,
		NL3D::CVertexBuffer::WeightFlag,
		NL3D::CVertexBuffer::NormalFlag,
		NL3D::CVertexBuffer::PrimaryColorFlag,
		NL3D::CVertexBuffer::SecondaryColorFlag,
		NL3D::CVertexBuffer::FogFlag,
		NL3D::CVertexBuffer::PaletteSkinFlag,
		0,
		NL3D::CVertexBuffer::TexCoord0Flag,
		NL3D::CVertexBuffer::TexCoord1Flag,
		NL3D::CVertexBuffer::TexCoord2Flag,
		NL3D::CVertexBuffer::TexCoord3Flag,
		NL3D::CVertexBuffer::TexCoord4Flag,
		NL3D::CVertexBuffer::TexCoord5Flag,
		NL3D::CVertexBuffer::TexCoord6Flag,
		NL3D::CVertexBuffer::TexCoord7Flag
	};

	enum AttribOffset
	{
		Position,
		Weight,
		Normal,
		PrimaryColor,
		SecondaryColor,
		Fog,
		PaletteSkin,
		Empty,
		TexCoord0,
		TexCoord1,
		TexCoord2,
		TexCoord3,
		TexCoord4,
		TexCoord5,
		TexCoord6,
		TexCoord7,
		NumOffsets
	};
}

namespace NL3D
{
	bool CDriverGL3::supportVertexProgram(CVertexProgram::TProfile profile) const
	{
		if (profile == IProgram::glsl330v)
			return true;
		else
			return false;
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


		GLenum error = glGetError();
		if (error != GL_NO_ERROR)
			return false;

		ItGPUPrgDrvInfoPtrList it = _GPUPrgDrvInfos.insert(_GPUPrgDrvInfos.end(),(NL3D::IProgramDrvInfos*)NULL);
		CVertexProgramDrvInfosGL3 *drvInfo = new CVertexProgramDrvInfosGL3(this, it);
		*it = drvInfo;

		program->m_DrvInfo = drvInfo;

		drvInfo->setProgramId(id);

		program->buildInfo(src);

		return true;
	}

	bool CDriverGL3::activeVertexProgram(CVertexProgram *program)
	{
		if (program == NULL)
			return true;

		IProgramDrvInfos *di = program->m_DrvInfo;
		CVertexProgramDrvInfosGL3 *drvInfo = dynamic_cast< CVertexProgramDrvInfosGL3* >(di);
		if (drvInfo == NULL)
			return false;

		glGetError();

		nglUseProgramStages(ppoId, GL_VERTEX_SHADER_BIT, drvInfo->getProgramId());

		GLenum error = glGetError();
		if (error != GL_NO_ERROR)
		{
			
			return false;
		}

		currentProgram.vp = program;

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

		GLenum error = glGetError();
		if (error != GL_NO_ERROR)
			return false;

		ItGPUPrgDrvInfoPtrList it = _GPUPrgDrvInfos.insert(_GPUPrgDrvInfos.end(), (NL3D::IProgramDrvInfos*)NULL);
		CPixelProgramDrvInfosGL3 *drvInfo = new CPixelProgramDrvInfosGL3(this, it);
		*it = drvInfo;
		drvInfo->setProgramId(id);
		program->m_DrvInfo = drvInfo;

		program->buildInfo(src);

		return true;
	}

	bool CDriverGL3::activePixelProgram(CPixelProgram *program)
	{
		if (program == NULL)
			return true;

		if (program->m_DrvInfo == NULL)
			return false;
		
		IProgramDrvInfos *di = program->m_DrvInfo;
		CPixelProgramDrvInfosGL3 *drvInfo = dynamic_cast< CPixelProgramDrvInfosGL3* >(di);
		if (drvInfo == NULL)
			return false;

		glGetError();

		nglUseProgramStages(ppoId, GL_FRAGMENT_SHADER_BIT, drvInfo->getProgramId());

		GLenum error = glGetError();
		if (error != GL_NO_ERROR)
		{
			return false;
		}

		currentProgram.pp = program;

		return true;
	}


	uint32 CDriverGL3::getProgramId(TProgram program) const
	{
		uint32 id = 0;

		switch(program)
		{
		case IDriver::VertexProgram:
			{
				if (currentProgram.vp != NULL)
				{
					IProgramDrvInfos *di = currentProgram.vp->m_DrvInfo;
					CVertexProgramDrvInfosGL3 *drvInfo = dynamic_cast< CVertexProgramDrvInfosGL3* >(di);
					if (drvInfo != NULL)
						id = drvInfo->getProgramId();
				}
			}
			break;

		case IDriver::PixelProgram:
			if (currentProgram.pp != NULL)
			{
				IProgramDrvInfos *di = currentProgram.pp->m_DrvInfo;
				CPixelProgramDrvInfosGL3 *drvInfo = dynamic_cast< CPixelProgramDrvInfosGL3* >(di);
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
		IProgram *p = NULL;

		switch(program)
		{
		case IDriver::VertexProgram:
			p = currentProgram.vp;
			break;

		case IDriver::PixelProgram:
			p = currentProgram.pp;
			break;

		case IDriver::GeometryProgram:
			p = currentProgram.gp;
			break;
		}

		return p;
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
		desc.setVBFlags(_CurrentVertexBufferHard->VB->getVertexFormat());
		
		if (mat.getShader() == CMaterial::LightMap)
			desc.setNLightMaps(mat._LightMaps.size());
		
		//int i = 0;

		if (mat.getShader() == CMaterial::Normal)
		{
			bool useTextures = false;

			int maxTextures = std::min(int(SHADER_MAX_TEXTURES), int(IDRV_MAT_MAXTEXTURES));
			for (int i = 0; i < maxTextures; i++)
			{
				desc.setTexEnvMode(i, mat.getTexEnvMode(i));
			}

			for (int i = 0; i < maxTextures; i++)
			{
				if (desc.hasVBFlags(vertexFlags[ TexCoord0 + i ]))
				{
					desc.setUseTexStage(i, true);
					useTextures = true;
				}
			}

			if (useTextures && !desc.getUseTexStage(1))
			{
				for (int i = 1; i < maxTextures; i++)
				{
					if (mat.getTexture(i) != NULL)
					{
						desc.setUseTexStage(i, true);
						desc.setUseFirstTexCoords(true);
					}
				}
			}
			else
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

		if (fogEnabled())
		{
			desc.setFog(true);
			desc.setFogMode(CShaderDesc::Linear);
		}

		int maxLights = std::min(int(SHADER_MAX_LIGHTS), int(MaxLight));
		bool enableLights = false;
		for (int i = 0; i < maxLights; i++)
		{
			if (!_UserLightEnable[ i ])
				continue;

			enableLights = true;
			
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

		desc.setLighting(enableLights);			
	}


	bool CDriverGL3::setupProgram(CMaterial &mat)
	{
		if (mat.getDynMat() != NULL)
			return true;
		
		CVertexProgram *vp = NULL;
		CPixelProgram *pp = NULL;
		SShaderPair sp;

		CShaderDesc desc;

		generateShaderDesc(desc, mat);

		// See if we've already generated and compiled this shader
		sp = shaderCache.findShader(desc);

		// Yes we have!
		if (!sp.empty())
		{
			if (currentProgram.vp == NULL)
			{
				if (!activeVertexProgram(sp.vp))
					return false;
			}

			if (currentProgram.pp == NULL)
			{
				if (!activePixelProgram(sp.pp))
					return false;
			}
		}
		// No we need to generate it now
		else
		{
			std::string vs;
			std::string ps;
			bool cacheShaders = true;

			shaderGenerator->reset();
			shaderGenerator->setMaterial(&mat);
			shaderGenerator->setVBFormat(_CurrentVertexBufferHard->VB->getVertexFormat());
			shaderGenerator->setShaderDesc(&desc);
			
			// If we don't already have a vertex program attached, we'll generate it now
			if (currentProgram.vp == NULL)
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

				if (!activeVertexProgram(vp))
				{
					delete vp;
					vp = NULL;
					return false;
				}
			}
			else
				cacheShaders = false;
		
			// If we don't already have a pixel program attached, we'll generate it now
			if (currentProgram.pp == NULL)
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
					delete vp;
					vp = NULL;
					delete pp;
					pp = NULL;
					return false;
				}

				if (!activePixelProgram(pp))
				{
					delete vp;
					vp = NULL;
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
				sp.vp = vp;
				sp.pp = pp;
				desc.setShaders(sp);
				shaderCache.cacheShader(desc);
			}
		}

		setupUniforms();

		return true;
	}

	bool CDriverGL3::setupDynMatProgram(CMaterial& mat, uint pass)
	{
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
	}


	void CDriverGL3::setupUniforms()
	{
		setupUniforms(IDriver::VertexProgram);
		setupUniforms(IDriver::PixelProgram);
	}

	void CDriverGL3::setupUniforms(TProgram program)
	{
		CMaterial &mat = *_CurrentMaterial;
		IProgram *p = getProgram(program);

		int mvpIndex = p->getUniformIndex(CProgramIndex::ModelViewProjection);
		if (mvpIndex != -1)
		{
			CMatrix mvp = _GLProjMat * _ModelViewMatrix;
			setUniform4x4f(program, mvpIndex, mvp);
		}

		int vmIndex = p->getUniformIndex(CProgramIndex::ViewMatrix);
		if (vmIndex != -1)
		{
			setUniform4x4f(program, vmIndex, _ViewMtx);
		}

		int mvIndex = p->getUniformIndex(CProgramIndex::ModelView);
		if (mvIndex != -1)
		{
			setUniform4x4f(program, mvIndex, _ModelViewMatrix);
		}

		int nmIdx = p->getUniformIndex(CProgramIndex::NormalMatrix);
		if (nmIdx != -1)
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

		int fogStartIdx = p->getUniformIndex(CProgramIndex::FogStart);
		if (fogStartIdx != -1)
		{
			setUniform1f(program, fogStartIdx, getFogStart());
		}

		int fogEndIdx = p->getUniformIndex(CProgramIndex::FogEnd);
		if (fogEndIdx != -1)
		{
			setUniform1f(program, fogEndIdx, getFogEnd());
		}

		int fogColorIdx = p->getUniformIndex(CProgramIndex::FogColor);
		if (fogColorIdx != -1)
		{
			GLfloat glCol[ 4 ];
			CRGBA col = getFogColor();
			glCol[ 0 ] = col.R / 255.0f;
			glCol[ 1 ] = col.G / 255.0f;
			glCol[ 2 ] = col.B / 255.0f;
			glCol[ 3 ] = col.A / 255.0f;
			setUniform4f(program, fogColorIdx, glCol[ 0 ], glCol[ 1 ], glCol[ 2 ], glCol[ 3 ]);
		}

		int colorIndex = p->getUniformIndex(CProgramIndex::Color);
		if (colorIndex != -1)
		{
			GLfloat glCol[ 4 ];
			CRGBA col = mat.getColor();
			glCol[ 0 ] = col.R / 255.0f;
			glCol[ 1 ] = col.G / 255.0f;
			glCol[ 2 ] = col.B / 255.0f;
			glCol[ 3 ] = col.A / 255.0f;

			setUniform4f(program, colorIndex, glCol[ 0 ], glCol[ 1 ], glCol[ 2 ], glCol[ 3 ]);
		}

		int diffuseIndex = p->getUniformIndex(CProgramIndex::DiffuseColor);
		if (diffuseIndex != -1)
		{
			GLfloat glCol[ 4 ];
			CRGBA col = mat.getDiffuse();
			glCol[ 0 ] = col.R / 255.0f;
			glCol[ 1 ] = col.G / 255.0f;
			glCol[ 2 ] = col.B / 255.0f;
			glCol[ 3 ] = col.A / 255.0f;

			setUniform4f(program, diffuseIndex, glCol[ 0 ], glCol[ 1 ], glCol[ 2 ], glCol[ 3 ]);
		}


		int maxLights = std::min(int(MaxLight), int(SHADER_MAX_LIGHTS));
		for (int i = 0; i < maxLights; i++)
		{
			if (!_UserLightEnable[ i ])
				continue;
			
			////////////////// Temporary insanity  ///////////////////////////////
			if ((_LightMode[ i ] != CLight::DirectionalLight) && (_LightMode[ i ] != CLight::PointLight))
				continue;
			//////////////////////////////////////////////////////////////////////
			
			if (_LightMode[ i ] == CLight::DirectionalLight)
			{
				int ld = p->getUniformIndex(CProgramIndex::TName(CProgramIndex::Light0DirOrPos + i));
				if (ld != -1)
				{
					CVector v = -1 * _UserLight[ i ].getDirection();
					setUniform3f(program, ld, v.x, v.y, v.z);
				}
			}
			else
			{
				int lp = p->getUniformIndex(CProgramIndex::TName(CProgramIndex::Light0DirOrPos + i));
				if (lp != -1)
				{
					CVector v = _UserLight[ i ].getPosition();
					float pos[ 3 ];
					pos[ 0 ] = v.x - _PZBCameraPos.x;
					pos[ 1 ] = v.y - _PZBCameraPos.y;
					pos[ 2 ] = v.z - _PZBCameraPos.z;
					setUniform3f(program, lp, pos[ 0 ], pos[ 1 ], pos[ 2 ]);
				}
			}

			int ldc = p->getUniformIndex(CProgramIndex::TName(CProgramIndex::Light0ColDiff + i));
			if (ldc != -1)
			{
				GLfloat glCol[ 4 ];
				CRGBA col = _UserLight[ i ].getDiffuse();
				glCol[ 0 ] = col.R / 255.0f;
				glCol[ 1 ] = col.G / 255.0f;
				glCol[ 2 ] = col.B / 255.0f;
				glCol[ 3 ] = col.A / 255.0f;
				setUniform4f(program, ldc, glCol[ 0 ], glCol[ 1 ], glCol[ 2 ], glCol[ 3 ]);
			}

			int lsc = p->getUniformIndex(CProgramIndex::TName(CProgramIndex::Light0ColSpec + i));
			if (lsc != -1)
			{
				GLfloat glCol[ 4 ];
				CRGBA col = _UserLight[ i ].getSpecular();
				glCol[ 0 ] = col.R / 255.0f;
				glCol[ 1 ] = col.G / 255.0f;
				glCol[ 2 ] = col.B / 255.0f;
				glCol[ 3 ] = col.A / 255.0f;
				setUniform4f(program, lsc, glCol[ 0 ], glCol[ 1 ], glCol[ 2 ], glCol[ 3 ]);
			}

			int shl = p->getUniformIndex(CProgramIndex::TName(CProgramIndex::Light0Shininess + i));
			if (shl != -1)
			{
				setUniform1f(program, shl, mat.getShininess());
			}

			int lac = p->getUniformIndex(CProgramIndex::TName(CProgramIndex::Light0ColAmb + i));
			if (lac != -1)
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
			}

			int lca = p->getUniformIndex(CProgramIndex::TName(CProgramIndex::Light0ConstAttn + i));
			if (lca != -1)
			{
				setUniform1f(program, lca, _UserLight[ i ].getConstantAttenuation());
			}

			int lla = p->getUniformIndex(CProgramIndex::TName(CProgramIndex::Light0LinAttn + i));
			if (lla != -1)
			{
				setUniform1f(program, lla, _UserLight[ i ].getLinearAttenuation());
			}

			int lqa = p->getUniformIndex(CProgramIndex::TName(CProgramIndex::Light0QuadAttn + i));
			if (lqa != -1)
			{
				setUniform1f(program, lqa, _UserLight[ i ].getQuadraticAttenuation());
			}
		}


		// Lightmaps have special constants
		if (mat.getShader() != CMaterial::LightMap)
		{
		
			for (int i = 0; i < IDRV_MAT_MAXTEXTURES; i++)
			{
				int cl = p->getUniformIndex(CProgramIndex::TName(CProgramIndex::Constant0 + i));
				if (cl != -1)
				{
					CRGBA col = mat._TexEnvs[ i ].ConstantColor;
					GLfloat glCol[ 4 ];
					glCol[ 0 ] = col.R / 255.0f;
					glCol[ 1 ] = col.G / 255.0f;
					glCol[ 2 ] = col.B / 255.0f;
					glCol[ 3 ] = col.A / 255.0f;

					setUniform4f(program, cl, glCol[ 0 ], glCol[ 1 ], glCol[ 2 ], glCol[ 3 ]);
				}
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
}


