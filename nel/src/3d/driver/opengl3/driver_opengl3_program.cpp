// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2014-2015  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include "stdopengl3.h"
#include "driver_opengl3.h"
#include "driver_opengl3_program.h"
#include "driver_opengl3_vertex_buffer.h"
#include "driver_opengl3_uniform_buffer.h"

namespace NL3D {
namespace NLDRIVERGL3 {

// Insert builtin UBO headers after leading preprocessor and precision lines
static std::string insertBuiltinHeaders(const char *source, bool lightTable, bool camera, bool object, bool material,
	const std::map<sint, NLMISC::CSmartPtr<CUniformBufferFormat> > &userUBOs, sint maxLightTableSize)
{
	const char *p = source;

	// Scan past #preprocessor, //-comment, precision, and blank lines
	for (;;)
	{
		if (*p == '#' || *p == '/' || *p == '\n'
			|| strncmp(p, "precision", 9) == 0)
		{
			while (*p && *p != '\n')
				++p;
			if (*p == '\n')
				++p;
		}
		else
			break;
	}

	std::string result;
	result.append(source, p - source);
	if (camera)
		result.append(GLSLCameraHeader);
	if (lightTable)
		result.append(buildGLSLLightTableHeader(maxLightTableSize));
	if (object)
		result.append(GLSLObjectHeader);
	if (material)
		result.append(GLSLMaterialHeader);

	// User UBO declarations — map key is TUBBinding enum (0=VP, 1=PP),
	// translate to GL binding points for fallback block name generation
	if (!userUBOs.empty())
	{
		static const sint s_UBBindingToGLSL[] = {
			NL_USER_VERTEX_PROGRAM_BINDING,  // UBBindingVertexProgram
			NL_USER_PIXEL_PROGRAM_BINDING,   // UBBindingPixelProgram
			NL_USER_SKELETON_BINDING,        // UBBindingSkeleton
		};
		std::stringstream ss;
		for (std::map<sint, NLMISC::CSmartPtr<CUniformBufferFormat> >::const_iterator it = userUBOs.begin(); it != userUBOs.end(); ++it)
		{
			nlassert(it->first >= 0 && it->first < UBBindingCount);
			generateUniformBufferGLSL(ss, *it->second, s_UBBindingToGLSL[it->first]);
		}
		result.append(ss.str());
	}

	result.append(p);
	return result;
}

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
	"tangent",
	"texCoord0",
	"texCoord1",
	"texCoord2",
	"texCoord3",
	"texCoord4",
	"texCoord5",
	"texCoord6",
	"texCoord7"
};

const char *g_TexelNames[IDRV_PROGRAM_MAXSAMPLERS] =
{
	"texel0",
	"texel1",
	"texel2",
	"texel3",
	"texel4",
	"texel5",
	"texel6",
	"texel7",
};

const char *g_ConstantNames[IDRV_PROGRAM_MAXSAMPLERS] =
{
	"constant0",
	"constant1",
	"constant2",
	"constant3",
	"constant4",
	"constant5",
	"constant6",
	"constant7",
};

/// Returns true if the GL uniform type is an opaque type (sampler, image)
/// that must reside in the default block regardless of UBO usage.
static bool isSamplerUniformType(GLenum type)
{
	switch (type)
	{
	case GL_SAMPLER_1D: case GL_SAMPLER_2D: case GL_SAMPLER_3D: case GL_SAMPLER_CUBE:
	case GL_SAMPLER_1D_SHADOW: case GL_SAMPLER_2D_SHADOW: case GL_SAMPLER_CUBE_SHADOW:
	case GL_SAMPLER_1D_ARRAY: case GL_SAMPLER_2D_ARRAY:
	case GL_SAMPLER_1D_ARRAY_SHADOW: case GL_SAMPLER_2D_ARRAY_SHADOW:
	case GL_SAMPLER_2D_RECT: case GL_SAMPLER_2D_RECT_SHADOW:
	case GL_SAMPLER_BUFFER:
	case GL_INT_SAMPLER_1D: case GL_INT_SAMPLER_2D: case GL_INT_SAMPLER_3D: case GL_INT_SAMPLER_CUBE:
	case GL_INT_SAMPLER_1D_ARRAY: case GL_INT_SAMPLER_2D_ARRAY:
	case GL_INT_SAMPLER_2D_RECT: case GL_INT_SAMPLER_BUFFER:
	case GL_UNSIGNED_INT_SAMPLER_1D: case GL_UNSIGNED_INT_SAMPLER_2D:
	case GL_UNSIGNED_INT_SAMPLER_3D: case GL_UNSIGNED_INT_SAMPLER_CUBE:
	case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY: case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
	case GL_UNSIGNED_INT_SAMPLER_2D_RECT:
		return true;
	default:
		return false;
	}
}

/// Query a compiled/linked GL program for non-UBO, non-sampler uniforms.
/// Returns true if the program has default-block uniforms that would need
/// per-draw glProgramUniform* calls (i.e., OnlyUBOs should be false).
static bool programHasNonUBOUniforms(GLuint programId)
{
	GLint numUniforms = 0;
	nglGetProgramiv(programId, GL_ACTIVE_UNIFORMS, &numUniforms);
	for (GLuint i = 0; i < (GLuint)numUniforms; i++)
	{
		GLint blockIndex = 0;
		nglGetActiveUniformsiv(programId, 1, &i, GL_UNIFORM_BLOCK_INDEX, &blockIndex);
		if (blockIndex == -1) // Default block (not in any UBO)
		{
			GLenum type = GL_FLOAT;
			GLint size = 0;
			char name[256];
			nglGetActiveUniform(programId, i, 256, NULL, &size, &type, name);
			if (!isSamplerUniformType(type))
				return true;
		}
	}
	return false;
}

bool CDriverGL3::supportVertexProgram(CVertexProgram::TProfile profile) const
{
	return (profile == IProgram::glsl330v) || (profile == IProgram::glsl300esv) || (profile == IProgram::nelvp) || (profile == IProgram::glsl3vi);
}

bool CDriverGL3::compileProgram(IProgram *program, GLenum shaderType,
	IProgram::TProfile linkedProfile, IProgram::TProfile ssoProfile,
	const char *stageName)
{
	if (program->m_DrvInfo != NULL)
		return true;

	if (program->m_CompileFailed)
		return false;

	IProgram::CSource *src = NULL;
	if (m_LinkedMegaShaders)
	{
		for (int i = 0; i < program->getSourceNb(); i++)
		{
			IProgram::CSource *s = program->getSource(i);
			if (s->Profile == linkedProfile)
			{ src = s; break; }
		}
	}
	if (!src && m_SupportSSO)
	{
		for (int i = 0; i < program->getSourceNb(); i++)
		{
			IProgram::CSource *s = program->getSource(i);
			if (s->Profile == ssoProfile)
			{ src = s; break; }
		}
	}
	if (src == NULL)
	{
		program->m_CompileFailed = true;
		return false;
	}

	// Object UBO implies light table UBO and camera UBO — write back to features
	if (src->Features.UsesObjectUBO)
	{
		src->Features.UsesLightTableUBO = true;
		src->Features.UsesCameraUBO = true;
	}

	std::string fullSource;
	const char *s;
	bool hasBuiltinUBO = src->Features.UsesLightTableUBO || src->Features.UsesCameraUBO
	                   || src->Features.UsesObjectUBO || src->Features.UsesMaterialUBO;
	bool hasUserUBO = !src->UniformBufferFormats.empty();
	if (hasBuiltinUBO || hasUserUBO)
	{
		fullSource = insertBuiltinHeaders(src->SourcePtr,
			src->Features.UsesLightTableUBO, src->Features.UsesCameraUBO,
			src->Features.UsesObjectUBO, src->Features.UsesMaterialUBO,
			src->UniformBufferFormats, _MaxLightTableSize);
		s = fullSource.c_str();
	}
	else
	{
		s = src->SourcePtr;
	}
	unsigned int id;
	if (src->Profile == linkedProfile)
	{
		// Pipeline stage: compile as shader object, create single-stage program,
		// keep shader attached for later extraction into a combined linked program.
		// Linked profiles are always compiled as non-separable pipeline stages.
		GLuint shader = nglCreateShader(shaderType);
		if (shader == 0)
		{
			program->m_CompileFailed = true;
			return false;
		}
		nglShaderSource(shader, 1, &s, NULL);
		nglCompileShader(shader);

		GLint compileOk;
		nglGetShaderiv(shader, GL_COMPILE_STATUS, &compileOk);
		if (compileOk == 0)
		{
			char errorLog[1024];
			nglGetShaderInfoLog(shader, 1024, NULL, errorLog);
			nlwarning("GL3: %s compile failed (pipeline stage): %s", stageName, errorLog);
			std::vector<std::string> lines;
			NLMISC::explode(std::string(s), std::string("\n"), lines);
			for (std::vector<std::string>::size_type i = 0; i < lines.size(); ++i)
				nldebug("GL3: %i: %s", (int)i, lines[i].c_str());
			nglDeleteShader(shader);
			program->m_CompileFailed = true;
#ifdef NL_DEBUG
			nlerror("GL3: %s program compilation failed (pipeline stage)", stageName);
#endif
			return false;
		}

		id = nglCreateProgram();
		nglAttachShader(id, shader);

		// Under GL ES 3.0 / WebGL 2.0, single-stage programs cannot be linked
		// (both vertex and fragment shaders are required). Skip the link here;
		// the final link happens in linkPrograms() which combines VP + PP.
		// Under desktop GL 3.3, we can link single-stage programs and use the
		// result for uniform introspection.
#ifndef USE_OPENGLES3
		if (m_SupportSSO)
		{
			nglLinkProgram(id);

			GLint linkOk;
			nglGetProgramiv(id, GL_LINK_STATUS, &linkOk);
			if (linkOk == 0)
			{
				char errorLog[1024];
				nglGetProgramInfoLog(id, 1024, NULL, errorLog);
				nlwarning("GL3: %s link failed (pipeline stage): %s", stageName, errorLog);
				nglDeleteShader(shader);
				nglDeleteProgram(id);
				program->m_CompileFailed = true;
#ifdef NL_DEBUG
				nlerror("GL3: %s program link failed (pipeline stage)", stageName);
#endif
				return false;
			}
		}
#endif
		// NOTE: do NOT detach/delete the shader — keep it attached for later extraction
	}
	else
	{
		// SSO: create separable shader program in one step
		id = nglCreateShaderProgramv(shaderType, 1, &s);
		if (id == 0)
		{
			program->m_CompileFailed = true;
			return false;
		}

		GLint ok;
		nglGetProgramiv(id, GL_LINK_STATUS, &ok);
		if (ok == 0)
		{
			char errorLog[1024];
			nglGetProgramInfoLog(id, 1024, NULL, errorLog);
			nlwarning("GL3: %s compile failed: %s", stageName, errorLog);
			std::vector<std::string> lines;
			NLMISC::explode(std::string(s), std::string("\n"), lines);
			for (std::vector<std::string>::size_type i = 0; i < lines.size(); ++i)
				nldebug("GL3: %i: %s", (int)i, lines[i].c_str());
			program->m_CompileFailed = true;
#ifdef NL_DEBUG
			nlerror("GL3: %s program compilation failed", stageName);
#endif
			return false;
		}
	}

	// Detect nelvp-converted source early (before OnlyUBOs override).
	// Nelvp-converted programs have no GL uniforms but callers still use
	// setUniform4f/setUniformMatrix, so OnlyUBOs must stay false on the source.
	bool isNelvp = false;
	for (std::map<sint, NLMISC::CSmartPtr<CUniformBufferFormat> >::const_iterator
		ubIt = src->UniformBufferFormats.begin(); ubIt != src->UniformBufferFormats.end(); ++ubIt)
	{
		if (ubIt->second && ubIt->second->Name == "NlNelvpConstants")
		{ isNelvp = true; break; }
	}

	// Override OnlyUBOs based on actual program introspection.
	// On GLES3, linked-profile programs aren't linked yet at this point
	// (single-stage link is skipped at line ~301); they are combined and
	// linked later in linkPrograms(). programHasNonUBOUniforms returns
	// false trivially for unlinked programs — correct since the linked
	// path only uses UBOs.
	bool hasNonUBO = programHasNonUBOUniforms(id);
	if (src->Features.OnlyUBOs && hasNonUBO)
	{
		nlwarning("GL3: %s '%s' claims OnlyUBOs but has non-UBO uniforms", stageName, src->DisplayName.c_str());
		src->Features.OnlyUBOs = false;
	}
	else if (!isNelvp && !src->Features.OnlyUBOs && !hasNonUBO)
	{
		src->Features.OnlyUBOs = true;
	}
	// Linked profiles only support UBOs; non-UBO uniforms won't be set by
	// the engine, so reject the program.
	if (src->Profile == linkedProfile && hasNonUBO)
	{
		nlwarning("GL3: %s '%s' has non-UBO uniforms (linked path requires UBOs only)", stageName, src->DisplayName.c_str());
		nglDeleteProgram(id);
		program->m_CompileFailed = true;
#ifdef NL_DEBUG
		nlerror("GL3: %s program has non-UBO uniforms (linked path)", stageName);
#endif
		return false;
	}

	ItGPUPrgDrvInfoPtrList it = _GPUPrgDrvInfos.insert(_GPUPrgDrvInfos.end(), (NL3D::IProgramDrvInfos*)NULL);
	CProgramDrvInfosGL3 *drvInfo = new CProgramDrvInfosGL3(this, it);
	*it = drvInfo;
	program->m_DrvInfo = drvInfo;
	drvInfo->setProgramId(id);

	// Set nelvp-converted flag (detected earlier) so getUniformIndex returns
	// constant register indices instead of querying GL uniform locations.
	if (isNelvp)
	{
		drvInfo->isNelvpConverted = true;
		drvInfo->NelvpParamIndices = src->ParamIndices;
	}

	// buildInfo resolves and caches uniform locations via getUniformIndex.
	// Under GL ES 3.0, single-stage pipeline programs aren't linked yet
	// (linking happens in linkPrograms()), so uniform queries would fail.
	// Defer uniform resolution to linkPrograms() for these programs, but
	// still associate the source so features()/source() are available.
	if (src->Profile != linkedProfile || m_SupportSSO)
		program->buildInfo(src);
	else
		program->setBuildSrc(src);

	// Setup initial uniforms (sampler bindings, UBO block bindings).
	// Under GL ES 3.0 pipeline stages, the single-stage program isn't linked,
	// so uniform setup is deferred to linkPrograms().
	if (src->Profile != linkedProfile || m_SupportSSO)
		setupInitialUniforms(program);

	return true;
}

bool CDriverGL3::compileInsertVertexProgram(CVertexProgram *program)
{
	if (program->m_DrvInfo != NULL)
		return true;

	if (program->m_CompileFailed)
		return false;

	// Find the glsl3vi source
	IProgram::CSource *insertSrc = NULL;
	for (int i = 0; i < program->getSourceNb(); i++)
	{
		IProgram::CSource *s = program->getSource(i);
		if (s->Profile == IProgram::glsl3vi)
		{ insertSrc = s; break; }
	}
	if (!insertSrc)
	{
		program->m_CompileFailed = true;
		return false;
	}

	// Create drvInfo for the insert program (no GL program of its own)
	ItGPUPrgDrvInfoPtrList it = _GPUPrgDrvInfos.insert(_GPUPrgDrvInfos.end(), (NL3D::IProgramDrvInfos*)NULL);
	CProgramDrvInfosGL3 *drvInfo = new CProgramDrvInfosGL3(this, it);
	*it = drvInfo;
	program->m_DrvInfo = drvInfo;
	drvInfo->isInsertProgram = true;
	drvInfo->InsertSource.assign(insertSrc->SourcePtr, insertSrc->SourceLen);

	// Build info from insert source (for features, UBO formats)
	program->buildInfo(insertSrc);

	// Compile mega VP variants with this insert spliced in
	// Always all-UBO for insert programs
	for (int linked = 0; linked < 2; ++linked)
	{
		if (!linked && !m_SupportSSO) continue;
		if (linked && !m_LinkedMegaShaders) continue;

		for (int fogOrPpl = 0; fogOrPpl < 2; ++fogOrPpl)
		{
			for (int hwClip = 0; hwClip < 2; ++hwClip)
			{
				// Skip hwClip=1 when PP handles clip planes
				if (hwClip && m_PPClipPlanes) continue;
				// Skip linked non-all-UBO (insert is always all-UBO)
				// Skip SSO variants that won't be selected (insert always all-UBO)
				if (!linked && !m_BuildUnusedPrograms)
				{
					// Only build the currently active UBO config
					if (!m_UseMegaObjectUBO) continue; // Insert requires all-UBO
				}

				std::string result;
				megaVPGenerate(result, fogOrPpl != 0, hwClip != 0,
					/*tableUBO=*/true, /*cameraUBO=*/true, /*objectUBO=*/true, /*materialUBO=*/true,
					linked != 0, drvInfo->InsertSource.c_str());

				CVertexProgram *innerVP = new CVertexProgram();
				IProgram::CSource *src = new IProgram::CSource();
				src->Profile = linked ? IProgram::glsl300esv : IProgram::glsl330v;
				src->DisplayName = NLMISC::toString("Insert Mega VP (linked=%d, fogOrPpl=%d, hwClip=%d)", linked, fogOrPpl, hwClip);
				src->Features.UsesLightTableUBO = true;
				src->Features.UsesCameraUBO = true;
				src->Features.UsesObjectUBO = true;
				src->Features.UsesMaterialUBO = true;
				src->Features.OnlyUBOs = true;
				// Copy insert's UBO formats so the user VP UBO gets declared
				src->UniformBufferFormats = insertSrc->UniformBufferFormats;
				src->setSource(result);
				innerVP->addSource(src);

				nldebug("GL3: Compile '%s'", src->DisplayName.c_str());

				if (!compileProgram(innerVP, GL_VERTEX_SHADER,
					IProgram::glsl300esv, IProgram::glsl330v, "VP"))
				{
					nlwarning("GL3: Insert mega VP compilation failed (%s)", src->DisplayName.c_str());
					delete innerVP;
					// Don't fail the whole insert — skip this variant
					continue;
				}

				drvInfo->InsertMegaVP[linked][fogOrPpl][hwClip] = innerVP;
			}
		}
	}

	return true;
}

bool CDriverGL3::compileVertexProgram(CVertexProgram *program)
{
	// Check for VP insert profile (glsl3vi)
	for (int i = 0; i < program->getSourceNb(); i++)
	{
		IProgram::CSource *s = program->getSource(i);
		if (s->Profile == IProgram::glsl3vi)
			return compileInsertVertexProgram(program);
	}

	// If the program has nelvp source but no GLSL source, convert nelvp to GLSL
	bool hasGLSL = false;
	bool hasNelvp = false;
	for (int i = 0; i < program->getSourceNb(); i++)
	{
		IProgram::CSource *s = program->getSource(i);
		if (s->Profile == IProgram::glsl330v || s->Profile == IProgram::glsl300esv)
			hasGLSL = true;
		if (s->Profile == IProgram::nelvp)
			hasNelvp = true;
	}
	if (hasNelvp && !hasGLSL)
	{
		bool linked = m_LinkedMegaShaders;
		if (!convertNelvpToGLSL(program, linked))
		{
			nlwarning("GL3: Failed to convert nelvp to GLSL");
			program->m_CompileFailed = true;
			return false;
		}
	}

	if (!compileProgram(program, GL_VERTEX_SHADER,
		IProgram::glsl300esv, IProgram::glsl330v, "VP"))
		return false;

	// Post-compilation setup for nelvp-converted programs
	if (hasNelvp && !hasGLSL)
	{
		CProgramDrvInfosGL3 *drvInfo = static_cast<CProgramDrvInfosGL3 *>(
			(IProgramDrvInfos *)program->m_DrvInfo);
		if (drvInfo && drvInfo->isNelvpConverted)
		{
			// Create the UBO for constant registers, sized by NelvpRegisterCount
			uint16 regCount = program->features().NelvpRegisterCount;
			nlassert(regCount > 0);
			drvInfo->NelvpConstantUB = new CUniformBuffer();
			drvInfo->NelvpConstantUB->Format.Name = "NlNelvpConstants";
			drvInfo->NelvpConstantUB->Format.push("c", CUniformBufferFormat::FloatVec4, regCount);
			drvInfo->NelvpConstantUB->UsageHint = CUniformBuffer::StreamDraw;
			// Initialize UBO host memory by locking/unlocking
			void *p = drvInfo->NelvpConstantUB->lock();
			memset(p, 0, regCount * 16);
			drvInfo->NelvpConstantUB->unlock();
		}
	}

	return true;
}

bool CDriverGL3::activeVertexProgram(CVertexProgram *program)
{
	return activeVertexProgram(program, false);
}

bool CDriverGL3::activeVertexProgram(CVertexProgram *program, bool driver)
{
	// When the driver activates an inner VP (driver=true), the user VP must be
	// either NULL (normal mega VP path) or an insert program whose inner variant
	// is being materialized.
	if (driver)
	{
		nlassert(m_UserVertexProgram == NULL
			|| (m_UserVertexProgram->m_DrvInfo
				&& static_cast<CProgramDrvInfosGL3 *>((IProgramDrvInfos *)m_UserVertexProgram->m_DrvInfo)->isInsertProgram));
	}

	if (m_DriverVertexProgram == program)
	{
		// Still update user program tracking even on early return
		if (!driver) m_UserVertexProgram = program;
		return true;
	}

	// Restore PPO if transitioning back from linked program
	if (!m_PPOBound)
	{
		_DriverGLStates.forceUseProgram(0);
		_DriverGLStates.forceBindProgramPipeline(ppoId);
		m_PPOBound = true;
		m_DriverShaderProgram = NULL;
	}

	// TODO: If !m_SupportSSO
	// -> defer until actual setup; direct GL Uniforms not supported in that case, 
	// or nelvp Uniforms go through the UBO staging area so not a problem

	if (program == NULL)
	{
		nglUseProgramStages(ppoId, GL_VERTEX_SHADER_BIT, 0);
		m_UserVertexProgram = NULL;
		m_DriverVertexProgram = NULL;
		m_NelvpActiveUB = NULL;
		return true;
	}

	IProgramDrvInfos *di = program->m_DrvInfo;
	if (di == NULL)
	{
		if (!compileVertexProgram(program))
		{
			m_UserVertexProgram = NULL;
			m_DriverVertexProgram = NULL;
			m_NelvpActiveUB = NULL;
			return false;
		}
		di = program->m_DrvInfo;
	}
	CProgramDrvInfosGL3 *drvInfo = static_cast<CProgramDrvInfosGL3 *>(di);

	// Insert programs have no GL program of their own — the inner variant will be
	// activated by setupBuiltinVertexProgram. Skip the PPO stage bind here.
	if (!drvInfo->isInsertProgram)
		nglUseProgramStages(ppoId, GL_VERTEX_SHADER_BIT, drvInfo->getProgramId());

	if (!driver) m_UserVertexProgram = program;
	m_DriverVertexProgram = program;

	// Make nelvp UBO immediately available for setUniform* calls
	if (drvInfo->isNelvpConverted && drvInfo->NelvpConstantUB)
		m_NelvpActiveUB = drvInfo->NelvpConstantUB;
	else
		m_NelvpActiveUB = NULL;

	return true;
}

bool CDriverGL3::supportPixelProgram(IProgram::TProfile profile) const
{
	return (profile == IProgram::glsl330f) || (profile == IProgram::glsl300esf);
}

bool CDriverGL3::compilePixelProgram(CPixelProgram *program)
{
	return compileProgram(program, GL_FRAGMENT_SHADER,
		IProgram::glsl300esf, IProgram::glsl330f, "PP");
}

bool CDriverGL3::activePixelProgram(CPixelProgram *program)
{
	return activePixelProgram(program, false);
}

bool CDriverGL3::activePixelProgram(CPixelProgram *program, bool driver)
{
	if (driver) nlassert(m_UserPixelProgram == NULL);

	if (m_DriverPixelProgram == program)
	{
		// Still update user program tracking even on early return
		if (!driver) m_UserPixelProgram = program;
		return true;
	}

	// Restore PPO if transitioning back from linked program
	if (!m_PPOBound)
	{
		_DriverGLStates.forceUseProgram(0);
		_DriverGLStates.forceBindProgramPipeline(ppoId);
		m_PPOBound = true;
		m_DriverShaderProgram = NULL;
	}

	if (program == NULL)
	{
		nglUseProgramStages(ppoId, GL_FRAGMENT_SHADER_BIT, 0);
		m_UserPixelProgram = NULL;
		m_DriverPixelProgram = NULL;
		m_ProgramNoUniforms[PixelProgram] = false;
		m_ProgramNoBuiltinUniforms[PixelProgram] = false;
		m_ProgramOnlyUBOs[PixelProgram] = false;
		m_ProgramUsesLightTableUBO[PixelProgram] = false;
		m_ProgramUsesCameraUBO[PixelProgram] = false;
		m_ProgramUsesObjectUBO[PixelProgram] = false;
		m_ProgramUsesMaterialUBO[PixelProgram] = false;
		return true;
	}

	IProgramDrvInfos *di = program->m_DrvInfo;
	if (di == NULL)
	{
		if (!compilePixelProgram(program))
		{
			m_UserPixelProgram = NULL;
			m_DriverPixelProgram = NULL;
			m_ProgramNoUniforms[PixelProgram] = false;
			m_ProgramNoBuiltinUniforms[PixelProgram] = false;
			m_ProgramOnlyUBOs[PixelProgram] = false;
			m_ProgramUsesLightTableUBO[PixelProgram] = false;
			m_ProgramUsesCameraUBO[PixelProgram] = false;
			m_ProgramUsesObjectUBO[PixelProgram] = false;
			m_ProgramUsesMaterialUBO[PixelProgram] = false;
			return false;
		}
		di = program->m_DrvInfo;
	}
	CProgramDrvInfosGL3 *drvInfo = static_cast<CProgramDrvInfosGL3 *>(di);

	nglUseProgramStages(ppoId, GL_FRAGMENT_SHADER_BIT, drvInfo->getProgramId());

	if (!driver) m_UserPixelProgram = program;
	m_DriverPixelProgram = program;

	// Set per-program flags from the activated PP's features
	m_ProgramNoUniforms[PixelProgram] = program->features().NoUniforms;
	m_ProgramNoBuiltinUniforms[PixelProgram] = program->features().NoBuiltinUniforms;
	m_ProgramOnlyUBOs[PixelProgram] = program->features().OnlyUBOs;
	m_ProgramUsesLightTableUBO[PixelProgram] = program->features().UsesLightTableUBO;
	m_ProgramUsesCameraUBO[PixelProgram] = program->features().UsesCameraUBO;
	m_ProgramUsesObjectUBO[PixelProgram] = program->features().UsesObjectUBO;
	m_ProgramUsesMaterialUBO[PixelProgram] = program->features().UsesMaterialUBO;
	return true;
}


uint32 CDriverGL3::getProgramId(TProgram program) const
{
	// When a linked shader program is active, both VP and PP target the same GL program
	if (m_DriverShaderProgram)
	{
		IProgramDrvInfos *di = m_DriverShaderProgram->m_DrvInfo;
		if (di == NULL)
			return 0;
		CProgramDrvInfosGL3 *drvInfo = static_cast<CProgramDrvInfosGL3 *>(di);
		return drvInfo->getProgramId();
	}

	IProgramDrvInfos *di;
	switch(program)
	{
	case IDriver::VertexProgram:
		if (m_DriverVertexProgram)
			di = m_DriverVertexProgram->m_DrvInfo;
		else
			di = NULL;
		break;
	case IDriver::PixelProgram:
		if (m_DriverPixelProgram)
			di = m_DriverPixelProgram->m_DrvInfo;
		else
			di = NULL;
		break;
	default:
		di = NULL;
		break;
	}

	if (di == NULL)
		return 0;

	CProgramDrvInfosGL3 *drvInfo = static_cast<CProgramDrvInfosGL3 *>(di);
	return drvInfo->getProgramId();
}

// Check if a nelvp-converted UBO should intercept this setUniform call.
// Returns the NelvpConstantUB if so, NULL otherwise.
// m_NelvpActiveUB is set immediately in activeVertexProgram, so it's
// available for setUniform* calls before any render happens.
CUniformBuffer *CDriverGL3::getNelvpUB(TProgram program) const
{
	if (program != VertexProgram)
		return NULL;
	return m_NelvpActiveUB;
}

void CDriverGL3::flushNelvpUserVP()
{
	if (!m_UserVertexProgram)
		return;

	CProgramDrvInfosGL3 *di = static_cast<CProgramDrvInfosGL3 *>(
		(IProgramDrvInfos *)m_UserVertexProgram->m_DrvInfo);
	if (!di || !di->isNelvpConverted || !di->NelvpConstantUB)
		return;

	CUniformBuffer *ub = di->NelvpConstantUB;

	// Ensure UBO is bound to the VP slot (state manager deduplicates)
	bindUniformBuffer(UBBindingVertexProgram, ub);
	m_NelvpActiveUB = ub;

	// Upload dirty data to GL and bind to GL binding point
	flushUserUBOs();
}

IProgram* CDriverGL3::getProgram(TProgram program) const
{
	// When a linked shader program is active, both VP and PP resolve against the same program
	if (m_DriverShaderProgram)
		return m_DriverShaderProgram;

	switch(program)
	{
	case IDriver::VertexProgram:
		return m_DriverVertexProgram;
	case IDriver::PixelProgram:
		return m_DriverPixelProgram;
	// case IDriver::GeometryProgram:
	// 	return m_DriverGeometryProgram;
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
	if (index == ~0u) return;
	CUniformBuffer *ub = getNelvpUB(program);
	if (ub) { ub->lock(); ub->set(index * 16, f0, 0.0f, 0.0f, 0.0f); ub->unlock(); return; }
	uint32 id = getProgramId(program);
	nglProgramUniform1f(id, index, f0);
}

void CDriverGL3::setUniform2f(TProgram program, uint index, float f0, float f1)
{
	if (index == ~0u) return;
	CUniformBuffer *ub = getNelvpUB(program);
	if (ub) { ub->lock(); ub->set(index * 16, f0, f1, 0.0f, 0.0f); ub->unlock(); return; }
	uint32 id = getProgramId(program);
	nglProgramUniform2f(id, index, f0, f1);
}

void CDriverGL3::setUniform3f(TProgram program, uint index, float f0, float f1, float f2)
{
	if (index == ~0u) return;
	CUniformBuffer *ub = getNelvpUB(program);
	if (ub) { ub->lock(); ub->set(index * 16, f0, f1, f2, 0.0f); ub->unlock(); return; }
	uint32 id = getProgramId(program);
	nglProgramUniform3f(id, index, f0, f1, f2);
}

void CDriverGL3::setUniform4f(TProgram program, uint index, float f0, float f1, float f2, float f3)
{
	if (index == ~0u) return;
	CUniformBuffer *ub = getNelvpUB(program);
	if (ub) { ub->lock(); ub->set(index * 16, f0, f1, f2, f3); ub->unlock(); return; }
	uint32 id = getProgramId(program);
	nglProgramUniform4f(id, index, f0, f1, f2, f3);
}

void CDriverGL3::setUniform1i(TProgram program, uint index, sint32 i0)
{
	if (index == ~0u) return;
	uint32 id = getProgramId(program);
	nglProgramUniform1i(id, index, i0);
}

void CDriverGL3::setUniform2i(TProgram program, uint index, sint32 i0, sint32 i1)
{
	if (index == ~0u) return;
	uint32 id = getProgramId(program);
	nglProgramUniform2i(id, index, i0, i1);
}

void CDriverGL3::setUniform3i(TProgram program, uint index, sint32 i0, sint32 i1, sint32 i2)
{
	if (index == ~0u) return;
	uint32 id = getProgramId(program);
	nglProgramUniform3i(id, index, i0, i1, i2);
}

void CDriverGL3::setUniform4i(TProgram program, uint index, sint32 i0, sint32 i1, sint32 i2, sint32 i3)
{
	if (index == ~0u) return;
	uint32 id = getProgramId(program);
	nglProgramUniform4i(id, index, i0, i1, i2, i3);
}

void CDriverGL3::setUniform1ui(TProgram program, uint index, uint32 ui0)
{
	if (index == ~0u) return;
	uint32 id = getProgramId(program);
	nglProgramUniform1ui(id, index, ui0);
}

void CDriverGL3::setUniform2ui(TProgram program, uint index, uint32 ui0, uint32 ui1)
{
	if (index == ~0u) return;
	uint32 id = getProgramId(program);
	nglProgramUniform2ui(id, index, ui0, ui1);
}

void CDriverGL3::setUniform3ui(TProgram program, uint index, uint32 ui0, uint32 ui1, uint32 ui2)
{
	if (index == ~0u) return;
	uint32 id = getProgramId(program);
	nglProgramUniform3ui(id, index, ui0, ui1, ui2);
}

void CDriverGL3::setUniform4ui(TProgram program, uint index, uint32 ui0, uint32 ui1, uint32 ui2, uint32 ui3)
{
	if (index == ~0u) return;
	uint32 id = getProgramId(program);
	nglProgramUniform4ui(id, index, ui0, ui1, ui2, ui3);
}

void CDriverGL3::setUniform3f(TProgram program, uint index, const CVector &v)
{
	if (index == ~0u) return;
	CUniformBuffer *ub = getNelvpUB(program);
	if (ub) { ub->lock(); ub->set(index * 16, v.x, v.y, v.z, 0.0f); ub->unlock(); return; }
	uint32 id = getProgramId(program);
	nglProgramUniform3f(id, index, v.x, v.y, v.z);
}

void CDriverGL3::setUniform4f(TProgram program, uint index, const CVector &v, float f3)
{
	if (index == ~0u) return;
	CUniformBuffer *ub = getNelvpUB(program);
	if (ub) { ub->lock(); ub->set(index * 16, v.x, v.y, v.z, f3); ub->unlock(); return; }
	uint32 id = getProgramId(program);
	nglProgramUniform4f(id, index, v.x, v.y, v.z, f3);
}

void CDriverGL3::setUniform4f(TProgram program, uint index, const NLMISC::CRGBAF& rgba)
{
	if (index == ~0u) return;
	CUniformBuffer *ub = getNelvpUB(program);
	if (ub) { ub->lock(); ub->set(index * 16, rgba.R, rgba.G, rgba.B, rgba.A); ub->unlock(); return; }
	uint32 id = getProgramId(program);
	nglProgramUniform4f(id, index, rgba.R, rgba.G, rgba.B, rgba.A);
}

void CDriverGL3::setUniform3x3f(TProgram program, uint index, const float *src)
{
	if (index == ~0u) return;
	// nelvp: 3x3 matrix not used by nelvp programs, no interception needed
	uint32 id = getProgramId(program);
	nglProgramUniformMatrix3fv(id, index, 1, false, src);
}

void CDriverGL3::setUniform4x4f(TProgram program, uint index, const CMatrix &m)
{
	if (index == ~0u) return;
	CUniformBuffer *ub = getNelvpUB(program);
	if (ub)
	{
		// nelvp: rows in consecutive registers, CMatrix is column-major → transpose
		CMatrix mt = m;
		mt.transpose();
		ub->lock();
		ub->set(index * 16, mt);
		ub->unlock();
		return;
	}
	uint32 id = getProgramId(program);
	nglProgramUniformMatrix4fv(id, index, 1, false, m.get());
}

void CDriverGL3::setUniform4x4f(TProgram program, uint index, const float *src)
{
	if (index == ~0u) return;
	CUniformBuffer *ub = getNelvpUB(program);
	if (ub)
	{
		// nelvp: rows in consecutive registers, input is column-major → transpose
		CMatrix mt;
		mt.set(src);
		mt.transpose();
		ub->lock();
		ub->set(index * 16, mt);
		ub->unlock();
		return;
	}
	uint32 id = getProgramId(program);
	nglProgramUniformMatrix4fv(id, index, 1, false, src);
}

void CDriverGL3::setUniform4fv(TProgram program, uint index, size_t num, const float *src)
{
	if (index == ~0u) return;
	CUniformBuffer *ub = getNelvpUB(program);
	if (ub)
	{
		ub->lock();
		for (size_t i = 0; i < num; i++)
			ub->set((int)((index + i) * 16), src[i * 4], src[i * 4 + 1], src[i * 4 + 2], src[i * 4 + 3]);
		ub->unlock();
		return;
	}
	uint32 id = getProgramId(program);
	nglProgramUniform4fv(id, index, num, src);
}

void CDriverGL3::setUniform4iv(TProgram program, uint index, size_t num, const sint32 *src)
{
	if (index == ~0u) return;
	uint32 id = getProgramId(program);
	nglProgramUniform4iv(id, index, num, src);
}

void CDriverGL3::setUniform4uiv(TProgram program, uint index, size_t num, const uint32 *src)
{
	if (index == ~0u) return;
	uint32 id = getProgramId(program);
	nglProgramUniform4uiv(id, index, num, src);
}

void CDriverGL3::setUniformMatrix(TProgram program, uint index, TMatrix matrix, TTransform transform)
{
	if (index == ~0u) return;
	CUniformBuffer *ub = getNelvpUB(program);
	CMatrix mat;

	if (ub)
	{
		// nelvp programs expect old-GL-style matrices where _ModelViewMatrix
		// already includes the basis change (NeL space → GL eye space).
		// In GL3, _ModelViewMatrix is pure NeL space, so prepend _ChangeBasis.
		switch(matrix)
		{
		case IDriver::ModelView:
			mat = _ChangeBasis * _ModelViewMatrix;
			break;
		case IDriver::Projection:
			mat = _GLProjMat; // No basis here; MV already has it
			break;
		case IDriver::ModelViewProjection:
			mat = _GLProjMat * _ChangeBasis * _ModelViewMatrix;
			break;
		}
	}
	else
	{
		switch(matrix)
		{
		case IDriver::ModelView:
			mat = _ModelViewMatrix;
			break;
		case IDriver::Projection:
			mat = _GLProjMat * _ChangeBasis;
			break;
		case IDriver::ModelViewProjection:
			mat = _GLProjMat * _ChangeBasis * _ModelViewMatrix;
			break;
		}
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

	if (ub)
	{
		// nelvp stores matrix rows in consecutive constant registers (c[i]..c[i+3]).
		// CMatrix is column-major, so transpose to get rows as consecutive vec4s.
		mat.transpose();
		ub->lock();
		ub->set(index * 16, mat);
		ub->unlock();
		return;
	}

	uint32 id = getProgramId(program);
	nglProgramUniformMatrix4fv(id, index, 1, false, mat.get());
}

void CDriverGL3::setUniformFog(TProgram program, uint index)
{
	if (index == ~0u) return;
	CUniformBuffer *ub = getNelvpUB(program);
	if (ub)
	{
		// nelvp fog needs basis-changed ModelView (GL eye space)
		CMatrix nelvpMV = _ChangeBasis * _ModelViewMatrix;
		const float *v = nelvpMV.get();
		ub->lock();
		ub->set(index * 16, -v[2], -v[6], -v[10], -v[4]);
		ub->unlock();
		return;
	}
	uint32 id = getProgramId(program);
	const float *v = _ModelViewMatrix.get();
	nglProgramUniform4f(id, index, -v[ 2 ], -v[ 6 ], -v[ 10 ], -v[ 4 ]);
}

/*
void CDriverGL3::generateShaderDesc(CShaderDesc &desc, CMaterial &mat)
{
	desc.setShaderType(mat.getShader());
	uint16 vbFlags = _CurrentVertexBufferGL->VB->getVertexFormat();
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
					desc.hasVBFlags(g_VertexFlags[TexCoord0 + i]) ? "YES" : "NO");* /
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
		if (!_LightEnable[i])
		{
			desc.setLight(i, CShaderDesc::Nolight);
			continue;
		}
		//if (!_LightEnable[ i ])
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

	desc.setLighting(/*enableLights && mat.isLighted() &&* / m_VPBuiltinCurrent.Lighting);			
}*/

bool CDriverGL3::setupBuiltinPrograms()
{
	// Bind and flush the nelvp constant UBO if the active VP is a converted nelvp program.
	flushNelvpUserVP();

	// Effective programs: user > material > NULL
	CVertexProgram *effectiveVP = m_UserVertexProgram ? m_UserVertexProgram : m_MaterialVertexProgram;
	CPixelProgram *effectivePP = m_UserPixelProgram ? m_UserPixelProgram : m_MaterialPixelProgram;

	// Ensure effective programs are compiled so that features()/source() are available
	// to downstream functions. User programs should already be compiled via
	// activeVertexProgram/activePixelProgram, and material programs should be compiled
	// at creation time. If we reach here uncompiled, something is wrong upstream.
	if (effectiveVP && !effectiveVP->m_DrvInfo)
	{
		nlwarning("GL3: VP not compiled before setupBuiltinPrograms");
		if (!compileVertexProgram(effectiveVP))
			return false;
	}
	if (effectivePP && !effectivePP->m_DrvInfo)
	{
		nlwarning("GL3: PP not compiled before setupBuiltinPrograms");
		if (!compilePixelProgram(effectivePP))
			return false;
	}

	// Pure mega linked path (no user/material programs)
	if (m_LinkedMegaShaders && m_UseMegaShaders && !effectiveVP && !effectivePP)
	{
		m_NelvpActiveUB = NULL;
		return setupMegaLinkedPrograms()
			&& setupUniforms();
	}

	// Try user/material linked path when at least one non-builtin program is set
	if (m_LinkedMegaShaders && m_UseMegaShaders)
	{
		if (setupUserLinkedPrograms(effectiveVP, effectivePP))
			return setupUniforms();
		// Fall through to SSO if linking not possible
#ifdef USE_OPENGLES3
		nlwarning("GL3: GLES3 unreachable: setupUserLinkedPrograms failed, no SSO fallback available");
#endif
	}

	return setupBuiltinVertexProgram(effectiveVP, effectivePP)
		&& setupBuiltinPixelProgram(effectivePP)
		&& setupUniforms();
}

bool CDriverGL3::setupBuiltinVertexProgram(CVertexProgram *effectiveVP, CPixelProgram *effectivePP)
{
#ifdef USE_OPENGLES3
	// Builtin non-mega shaders are not supported under GLES 3.0;
	// they generate #version 330 which cannot compile on this target.
	// This path should be unreachable when m_LinkedMegaShaders is true.
	nlwarning("GL3: GLES3 unreachable: setupBuiltinVertexProgram called (effectiveVP=%p, effectivePP=%p)", (void *)effectiveVP, (void *)effectivePP);
	return false;
#endif
	touchVertexFormatVP(); // Always update — PP builtin depends on vertex format

	// Resolve PPL support: both VP and PP must support it
	m_ProgramSupportsPPL = effectivePP ? effectivePP->features().SupportPPL : true;
	if (effectiveVP)
		m_ProgramSupportsPPL = m_ProgramSupportsPPL && effectiveVP->features().SupportPPL;

	// Set canonical PPL count — single source of truth for all paths
	{
		uint8 numPpl = (uint8)(m_ProgramSupportsPPL ? _NumPerPixelLights : 0);
		if (m_VPBuiltinCurrent.NumPerPixelLights != numPpl)
		{
			m_VPBuiltinCurrent.NumPerPixelLights = numPpl;
			if (m_VPBuiltinCurrent.Lighting)
				m_VPBuiltinTouched = true;
		}
	}

	if (effectiveVP)
	{
		// Check if this is a VP insert program — route to mega VP variant selection
		CProgramDrvInfosGL3 *vpDrvInfo = static_cast<CProgramDrvInfosGL3 *>(
			(IProgramDrvInfos *)effectiveVP->m_DrvInfo);
		if (vpDrvInfo && vpDrvInfo->isInsertProgram)
		{
			// Insert VP: select appropriate inner mega VP variant with insert spliced in
			m_VPSpecularOutput = true; // Inner mega VP always outputs specularColor
			m_VPNormalOutput = false;
			m_VPWorldSpacePositionOutput = false;
			m_NelvpActiveUB = NULL;

			// Determine PPL activation (same logic as setupMegaVertexProgram)
			bool pplActive = false;
			if (m_VPBuiltinCurrent.NumPerPixelLights > 0)
			{
				if (effectivePP)
				{
					if (effectivePP->features().UsesObjectUBO)
						pplActive = true;
					else if (effectivePP->features().InputsWorldSpacePosition
					      && effectivePP->features().InputsWorldSpaceNormal)
						pplActive = true;
				}
				else
					pplActive = true; // Mega PP always supports PPL
			}
			if (pplActive)
			{
				m_VPWorldSpacePositionOutput = true;
				m_VPNormalOutput = true;
			}
			if (effectivePP)
			{
				if (effectivePP->features().InputsWorldSpaceNormal)
					m_VPNormalOutput = true;
				if (effectivePP->features().InputsWorldSpacePosition)
					m_VPWorldSpacePositionOutput = true;
			}

			int fogOrPpl = (m_VPBuiltinCurrent.Fog || pplActive || m_VPBuiltinCurrent.PPClipPlane) ? 1 : 0;
			int hwClip = (m_VPBuiltinCurrent.ClipPlaneMask != 0) ? 1 : 0;
			int linked = 0; // SSO path; linked handled by setupUserLinkedPrograms

			CVertexProgram *innerVP = vpDrvInfo->InsertMegaVP[linked][fogOrPpl][hwClip];
			if (!innerVP)
			{
				nlwarning("GL3: Insert mega VP variant [%d][%d][%d] not available", linked, fogOrPpl, hwClip);
				return false;
			}

			// Set program flags (insert mega VPs are always all-UBO)
			m_ProgramNoUniforms[VertexProgram] = false;
			m_ProgramNoBuiltinUniforms[VertexProgram] = false;
			m_ProgramOnlyUBOs[VertexProgram] = true;
			m_ProgramUsesLightTableUBO[VertexProgram] = true;
			m_ProgramUsesCameraUBO[VertexProgram] = true;
			m_ProgramUsesObjectUBO[VertexProgram] = true;
			m_ProgramUsesMaterialUBO[VertexProgram] = true;

			if (!activeVertexProgram(innerVP, true))
				return false;

			return true;
		}

		// Regular (non-insert) user/material VP
		// If this is a material VP (not already activated externally as user VP), activate on PPO
		if (!m_UserVertexProgram && m_MaterialVertexProgram)
		{
			if (!activeVertexProgram(effectiveVP, true))
				return false;
		}
		m_VPSpecularOutput = effectiveVP->features().OutputsSpecularColor;
		m_VPWorldSpacePositionOutput = effectiveVP->features().OutputsWorldSpacePosition;
		m_VPNormalOutput = false;
		m_ProgramNoUniforms[VertexProgram] = effectiveVP->features().NoUniforms;
		m_ProgramNoBuiltinUniforms[VertexProgram] = effectiveVP->features().NoBuiltinUniforms;
		m_ProgramOnlyUBOs[VertexProgram] = effectiveVP->features().OnlyUBOs;
		m_ProgramUsesLightTableUBO[VertexProgram] = effectiveVP->features().UsesLightTableUBO || effectiveVP->features().UsesObjectUBO;
		m_ProgramUsesCameraUBO[VertexProgram] = effectiveVP->features().UsesCameraUBO || effectiveVP->features().UsesObjectUBO;
		m_ProgramUsesObjectUBO[VertexProgram] = effectiveVP->features().UsesObjectUBO; // Object UBO implies table and camera UBO
		m_ProgramUsesMaterialUBO[VertexProgram] = effectiveVP->features().UsesMaterialUBO;
		// If PPL requested and user VP has object UBO, force world-space outputs
		// (UBO programs support PPL dynamically via nlWorldSpacePosition/Normal uniforms)
		if (m_VPBuiltinCurrent.NumPerPixelLights > 0 && m_ProgramUsesObjectUBO[VertexProgram])
		{
			m_VPWorldSpacePositionOutput = true;
			m_VPNormalOutput = true;
		}

		// Bind nelvp constant UBO if this is a converted nelvp program
		m_NelvpActiveUB = NULL;
		{
			CProgramDrvInfosGL3 *di = static_cast<CProgramDrvInfosGL3 *>(
				(IProgramDrvInfos *)effectiveVP->m_DrvInfo);
			if (di && di->isNelvpConverted && di->NelvpConstantUB)
			{
				bindUniformBuffer(UBBindingVertexProgram, di->NelvpConstantUB);
				m_NelvpActiveUB = di->NelvpConstantUB;
				// Nelvp-converted programs have no GL uniforms (all go through UBO),
				// but OnlyUBOs is kept false on the source so callers use setUniform*.
				m_ProgramOnlyUBOs[VertexProgram] = true;
			}
		}

		return true;
	}

	m_NelvpActiveUB = NULL;
	if (m_UseMegaShaders) return setupMegaVertexProgram();

	if (!m_SupportSSO) return false;

	// Check if PP needs world-space normal varying
	bool needNormal = false;
	if (effectivePP)
		needNormal = effectivePP->features().InputsWorldSpaceNormal;

	// Check if PP needs world-space position varying
	bool needWorldPos = false;
	if (effectivePP)
		needWorldPos = effectivePP->features().InputsWorldSpacePosition;

	// Force world-space outputs when per-pixel lighting is active
	if (m_VPBuiltinCurrent.NumPerPixelLights > 0)
	{
		needNormal = true;
		needWorldPos = true;
	}

	setWorldSpaceNormalVP(needNormal);
	setWorldSpacePositionVP(needWorldPos);

	if (m_VPBuiltinTouched)
	{
		generateBuiltinVertexProgram();
		nlassert(m_VPBuiltinCurrent.VertexProgram);
		m_VPBuiltinTouched = false;
	}

	m_VPSpecularOutput = m_VPBuiltinCurrent.Lighting
		|| (m_VPBuiltinCurrent.VertexFormat & g_VertexFlags[SecondaryColor]);
	m_VPNormalOutput = m_VPBuiltinCurrent.WorldSpaceNormal
		&& (m_VPBuiltinCurrent.VertexFormat & g_VertexFlags[Normal]);
	m_VPWorldSpacePositionOutput = m_VPBuiltinCurrent.WorldSpacePosition;
	m_ProgramNoUniforms[VertexProgram] = false; // Builtin non-mega VP does not use UBOs
	m_ProgramNoBuiltinUniforms[VertexProgram] = false;
	m_ProgramOnlyUBOs[VertexProgram] = false;
	m_ProgramUsesLightTableUBO[VertexProgram] = false;
	m_ProgramUsesCameraUBO[VertexProgram] = false;
	m_ProgramUsesObjectUBO[VertexProgram] = false;
	m_ProgramUsesMaterialUBO[VertexProgram] = false;

	if (!activeVertexProgram(m_VPBuiltinCurrent.VertexProgram, true))
		return false;

	return true;
}

bool CDriverGL3::setupBuiltinPixelProgram(CPixelProgram *effectivePP)
{
	if (effectivePP)
	{
		// If this is a material PP (not already activated externally as user PP), activate on PPO
		if (!m_UserPixelProgram && m_MaterialPixelProgram)
		{
			if (!activePixelProgram(effectivePP, true))
				return false;
		}
		m_ProgramNoUniforms[PixelProgram] = effectivePP->features().NoUniforms;
		m_ProgramNoBuiltinUniforms[PixelProgram] = effectivePP->features().NoBuiltinUniforms;
		m_ProgramOnlyUBOs[PixelProgram] = effectivePP->features().OnlyUBOs;
		m_ProgramUsesLightTableUBO[PixelProgram] = effectivePP->features().UsesLightTableUBO || effectivePP->features().UsesObjectUBO;
		m_ProgramUsesCameraUBO[PixelProgram] = effectivePP->features().UsesCameraUBO || effectivePP->features().UsesObjectUBO;
		m_ProgramUsesObjectUBO[PixelProgram] = effectivePP->features().UsesObjectUBO;
		m_ProgramUsesMaterialUBO[PixelProgram] = effectivePP->features().UsesMaterialUBO;
		return true;
	}

	if (m_UseMegaShaders) return setupMegaPixelProgram();

	if (!m_SupportSSO) return false;

	nlassert(_CurrentMaterial);
	CMaterial &mat = *_CurrentMaterial;
	CMaterialDrvInfosGL3 *matDrv = static_cast<CMaterialDrvInfosGL3 *>((IMaterialDrvInfos *)(mat._MatDrvInfo));
	nlassert(matDrv);

	// Material-derived PPBuiltin state is now pushed from setupMaterial.
	// LightMap texture state from setupLightmapPass.
	matDrv->PPBuiltin.checkDriverStateTouched(this);

	if (matDrv->PPBuiltin.Touched)
	{
		generateBuiltinPixelProgram(mat);
		nlassert(matDrv->PPBuiltin.PixelProgram);
		matDrv->PPBuiltin.Touched = false;
	}

	m_ProgramNoUniforms[PixelProgram] = false; // Builtin non-mega PP does not use UBOs
	m_ProgramNoBuiltinUniforms[PixelProgram] = false;
	m_ProgramOnlyUBOs[PixelProgram] = false;
	m_ProgramUsesLightTableUBO[PixelProgram] = false;
	m_ProgramUsesCameraUBO[PixelProgram] = false;
	m_ProgramUsesObjectUBO[PixelProgram] = false;
	m_ProgramUsesMaterialUBO[PixelProgram] = false;

	if (!activePixelProgram(matDrv->PPBuiltin.PixelProgram, true))
		return false;

	return true;
}

bool CDriverGL3::setupUniforms()
{
	// Stage camera data (fog, view matrix, clip planes) when dirty.
	// Both UBO upload and SSO individual uniforms read from _CameraUBOData.
	if (_CameraUBODirty)
		stageCameraUBO();

	if (m_DriverShaderProgram)
	{
		// Linked shader program path: only UBOs, no individual uniforms.
		bool vpSkipBuiltin = m_ProgramNoUniforms[VertexProgram] || m_ProgramNoBuiltinUniforms[VertexProgram];
		bool ppSkipBuiltin = m_ProgramNoUniforms[PixelProgram] || m_ProgramNoBuiltinUniforms[PixelProgram];

		// Upload builtin UBOs based on union of VP and PP stage needs.
		if (!vpSkipBuiltin || !ppSkipBuiltin)
		{
			if (m_ProgramUsesObjectUBO[VertexProgram] || m_ProgramUsesObjectUBO[PixelProgram])
				uploadObjectUBO();
			if (m_ProgramUsesMaterialUBO[VertexProgram] || m_ProgramUsesMaterialUBO[PixelProgram])
				uploadMaterialUBO();
			if (m_ProgramUsesCameraUBO[VertexProgram] || m_ProgramUsesCameraUBO[PixelProgram])
				uploadCameraUBO();
			if (m_ProgramUsesLightTableUBO[VertexProgram] || m_ProgramUsesLightTableUBO[PixelProgram])
				uploadLightTableUBO();
		}

		// Flush user-bound UBOs — skip only when truly no uniforms at all on both stages
		if (!m_ProgramNoUniforms[VertexProgram] || !m_ProgramNoUniforms[PixelProgram])
			flushUserUBOs();

		return true;
	}

	// SSO path: separate VP and PP programs
	bool vpSkipBuiltin = m_ProgramNoUniforms[VertexProgram] || m_ProgramNoBuiltinUniforms[VertexProgram];
	bool ppSkipBuiltin = m_ProgramNoUniforms[PixelProgram] || m_ProgramNoBuiltinUniforms[PixelProgram];

	// Upload builtin UBOs based on union of all active programs' needs
	if (!vpSkipBuiltin || !ppSkipBuiltin)
	{
		if (m_ProgramUsesObjectUBO[VertexProgram] || m_ProgramUsesObjectUBO[PixelProgram])
			uploadObjectUBO();
		if (m_ProgramUsesMaterialUBO[VertexProgram] || m_ProgramUsesMaterialUBO[PixelProgram])
			uploadMaterialUBO();
		if (m_ProgramUsesCameraUBO[VertexProgram] || m_ProgramUsesCameraUBO[PixelProgram])
			uploadCameraUBO();
		if (m_ProgramUsesLightTableUBO[VertexProgram] || m_ProgramUsesLightTableUBO[PixelProgram])
			uploadLightTableUBO();
	}

	// Flush user-bound UBOs (upload if dirty, bind to GL points) — only skip when truly no uniforms at all
	if (!m_ProgramNoUniforms[VertexProgram] || !m_ProgramNoUniforms[PixelProgram])
		flushUserUBOs();

	// Skip per-program uniform setup when no builtin uniforms or OnlyUBOs is set
	if (!vpSkipBuiltin && !m_ProgramOnlyUBOs[VertexProgram])
		setupUniforms(IDriver::VertexProgram);
	if (!ppSkipBuiltin && !m_ProgramOnlyUBOs[PixelProgram])
		setupUniforms(IDriver::PixelProgram);
	return true;
}



void CDriverGL3::setupUniforms(TProgram program)
{
	CMaterial &mat = *_CurrentMaterial;
	IProgram *p = getProgram(program);
	if (p == NULL) return;
	IProgramDrvInfos *di = p->m_DrvInfo;
	if (di == NULL) return;

	CProgramDrvInfosGL3 *drvInfo = static_cast<CProgramDrvInfosGL3 *>(di);
	GLuint progId = drvInfo->getProgramId();

	// FIXME
	if (!progId)
		return;

	if (!m_ProgramUsesObjectUBO[program])
	{
		uint mvpIndex = p->getUniformIndex(CProgramIndex::ModelViewProjection);
		if (mvpIndex != ~0)
		{
			CMatrix mvp = _GLProjMat * _ChangeBasis * _ModelViewMatrix;
			setUniform4x4f(program, mvpIndex, mvp);
		}

		uint mvIndex = p->getUniformIndex(CProgramIndex::ModelView);
		if (mvIndex != ~0)
		{
			setUniform4x4f(program, mvIndex, _ModelViewMatrix);
		}

		uint nmIdx = p->getUniformIndex(CProgramIndex::NormalMatrix);
		if (nmIdx != ~0)
		{
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
	}

	if (!m_ProgramUsesCameraUBO[program])
	{
		uint vmIndex = p->getUniformIndex(CProgramIndex::ViewMatrix);
		if (vmIndex != ~0)
			setUniform4x4f(program, vmIndex, _ViewMtx);

		// Specular texture matrix (inverse view rotation for cubemap reflection lookup)
		uint stmIdx = p->getUniformIndex(CProgramIndex::SpecularTexMtx);
		if (stmIdx != ~0)
			nglProgramUniformMatrix4fv(progId, stmIdx, 1, false, _CameraUBOData.specularTexMtx);
	}

	if (!m_ProgramUsesCameraUBO[program])
	{
		// Fog — read from _CameraUBOData (staged by stageCameraUBO(), includes override)
		uint fogParamsIdx = p->getUniformIndex(CProgramIndex::FogParams);
		if (fogParamsIdx != ~0)
			nglProgramUniform2fv(progId, fogParamsIdx, 1, _CameraUBOData.fogParams);

		uint fogColorIdx = p->getUniformIndex(CProgramIndex::FogColor);
		if (fogColorIdx != ~0)
			nglProgramUniform4fv(progId, fogColorIdx, 1, _CameraUBOData.fogColor);

		uint fogDensityIdx = p->getUniformIndex(CProgramIndex::FogDensity);
		if (fogDensityIdx != ~0)
			nglProgramUniform1f(progId, fogDensityIdx, _CameraUBOData.fogDensity);
	}

	if (!m_ProgramUsesMaterialUBO[program])
	{
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

		uint alphaRefIdx = p->getUniformIndex(CProgramIndex::AlphaRef);
		if (alphaRefIdx != ~0)
			nglProgramUniform1f(progId, alphaRefIdx, mat.getAlphaTestThreshold());

		// Shader type, texture active, TexEnv modes, alpha test (PP megashader control)
		CMaterialDrvInfosGL3 *matDrv = static_cast<CMaterialDrvInfosGL3 *>((IMaterialDrvInfos *)(mat._MatDrvInfo));
		if (matDrv)
		{
			uint shIdx = p->getUniformIndex(CProgramIndex::NlShader);
			if (shIdx != ~0u)
				nglProgramUniform1i(progId, shIdx, (sint32)matDrv->PPBuiltin.Shader);

			uint taIdx = p->getUniformIndex(CProgramIndex::NlTextureActive);
			if (taIdx != ~0u)
				nglProgramUniform1i(progId, taIdx, (sint32)matDrv->PPBuiltin.TextureActive);

			for (uint i = 0; i < IDRV_MAT_MAXTEXTURES; ++i)
			{
				uint teIdx = p->getUniformIndex((CProgramIndex::TName)(CProgramIndex::NlTexEnvMode0 + i));
				if (teIdx != ~0u)
					nglProgramUniform1ui(progId, teIdx, matDrv->PPBuiltin.TexEnvMode[i]);
			}

			uint atIdx = p->getUniformIndex(CProgramIndex::NlAlphaTest);
			if (atIdx != ~0u)
				nglProgramUniform1i(progId, atIdx, (matDrv->PPBuiltin.Flags & IDRV_MAT_ALPHA_TEST) ? 1 : 0);
		}

		uint lmsIdx = p->getUniformIndex(CProgramIndex::NlLightMapScale);
		if (lmsIdx != ~0u)
			nglProgramUniform1f(progId, lmsIdx, matDrv ? matDrv->MaterialUBO[matDrv->MaterialUBOCurrent].nlLightMapScale : 1.0f);

		// TexEnv constant colors, EMBM matrices, texture matrices
		// (staged in active MaterialUBO slot by setupNormalPass/setupLightMapPass)
		if (matDrv)
		{
			const CMaterialUBOData &matUBO = matDrv->MaterialUBO[matDrv->MaterialUBOCurrent];

			// Constants 0-7 (TexEnv constants for stages 0-3, lightmap factors for 0-7)
			for (uint i = 0; i < IDRV_PROGRAM_MAXSAMPLERS; ++i)
			{
				uint cIdx = p->getUniformIndex(CProgramIndex::TName(CProgramIndex::Constant0 + i));
				if (cIdx != ~0u)
					nglProgramUniform4fv(progId, cIdx, 1, matUBO.constant[i]);
			}

			// EMBM matrices (from constant[4-7]) and texture matrices (stages 0-3 only)
			for (uint i = 0; i < IDRV_MAT_MAXTEXTURES; ++i)
			{
				uint eIdx = p->getUniformIndex(CProgramIndex::TName(CProgramIndex::EmbmMatrix0 + i));
				if (eIdx != ~0u)
					nglProgramUniform4fv(progId, eIdx, 1, matUBO.constant[IDRV_MAT_MAXTEXTURES + i]);

				uint tIdx = p->getUniformIndex(CProgramIndex::TName(CProgramIndex::TexMatrix0 + i));
				if (tIdx != ~0u)
					setUniform4x4f(program, tIdx, matUBO.texMatrix[i]);
			}
		}
	}

	if (!m_ProgramUsesObjectUBO[program])
	{
		// selfIllumination: when lightmap override is active, use the pre-computed
		// LMC ambient from the staging area instead of computing from scratch.
		float si[4];
		if (_LightMapUBOOverride.Active)
		{
			memcpy(si, _LightMapUBOOverride.SelfIllumination, sizeof(si));
		}
		else
		{
			NLMISC::CRGBAF selfIllumination = NLMISC::CRGBAF(_AmbientGlobal);
			for (uint i = 0; i < NL_OPENGL3_MAX_LIGHT; ++i)
			{
				if (_LightEnable[i])
					selfIllumination += NLMISC::CRGBAF(_UserLight[i].getAmbiant());
			}
			CMaterialDrvInfosGL3 *siMatDrv = static_cast<CMaterialDrvInfosGL3 *>((IMaterialDrvInfos *)(mat._MatDrvInfo));
			selfIllumination *= siMatDrv->Ambient;
			if (siMatDrv->PPBuiltin.Shader != CMaterial::LightMap)
				selfIllumination += siMatDrv->Emissive;
			si[0] = selfIllumination.R;
			si[1] = selfIllumination.G;
			si[2] = selfIllumination.B;
			si[3] = 0.0f;
		}
		int selfIlluminationId = p->getUniformIndex(CProgramIndex::TName(CProgramIndex::SelfIllumination));
		if (selfIlluminationId != -1)
		{
			nglProgramUniform4fv(progId, selfIlluminationId, 1, si);
		}

		// World-space output flags
		uint wsnIdx = p->getUniformIndex(CProgramIndex::NlWorldSpaceNormal);
		if (wsnIdx != ~0u)
			nglProgramUniform1i(progId, wsnIdx, m_VPNormalOutput ? 1 : 0);

		uint wspIdx = p->getUniformIndex(CProgramIndex::NlWorldSpacePosition);
		if (wspIdx != ~0u)
			nglProgramUniform1i(progId, wspIdx, m_VPWorldSpacePositionOutput ? 1 : 0);

		// Per-pixel lighting count
		uint pplIdx = p->getUniformIndex(CProgramIndex::NlNumPerPixelLights);
		if (pplIdx != ~0u)
			nglProgramUniform1i(progId, pplIdx, (sint32)m_VPBuiltinCurrent.NumPerPixelLights);

		// Lighting mode
		uint nlIdx = p->getUniformIndex(CProgramIndex::NlLighting);
		if (nlIdx != ~0u)
			nglProgramUniform1i(progId, nlIdx, m_VPBuiltinCurrent.Lighting ? 1 : 0);

		// Per-light modes (non-table variant — table reads from UBO)
		if (!m_ProgramUsesLightTableUBO[program])
		{
			if (program == VertexProgram)
			{
				// VP lights are shifted: driver slot [m_VPBuiltinCurrent.NumPerPixelLights..7] → VP slot [0..7-N]
				for (uint i = 0; i < NL_OPENGL3_MAX_LIGHT; ++i)
				{
					uint lmIdx = p->getUniformIndex((CProgramIndex::TName)(CProgramIndex::NlLightMode0 + i));
					if (lmIdx != ~0u)
					{
						uint src = i + m_VPBuiltinCurrent.NumPerPixelLights;
						sint mode = (src < NL_OPENGL3_MAX_LIGHT && _LightEnable[src]) ? _LightMode[src] : -1;
						nglProgramUniform1i(progId, lmIdx, mode);
					}
				}
			}
			else
			{
				// PP lights are direct: driver slot [0..N-1] → PP slot [0..N-1]
				for (uint i = 0; i < NL_OPENGL3_MAX_LIGHT; ++i)
				{
					uint lmIdx = p->getUniformIndex((CProgramIndex::TName)(CProgramIndex::NlPpLightMode0 + i));
					if (lmIdx != ~0u)
					{
						sint mode = (_LightEnable[i]) ? _LightMode[i] : -1;
						nglProgramUniform1i(progId, lmIdx, mode);
					}
				}
			}
		}

		// TexGen modes
		for (uint i = 0; i < IDRV_MAT_MAXTEXTURES; ++i)
		{
			uint tgIdx = p->getUniformIndex((CProgramIndex::TName)(CProgramIndex::NlTexGenMode0 + i));
			if (tgIdx != ~0u)
				nglProgramUniform1i(progId, tgIdx, m_VPBuiltinCurrent.TexGenMode[i]);
		}

		// Vertex color lighted
		uint vclIdx = p->getUniformIndex(CProgramIndex::NlVertexColorLighted);
		if (vclIdx != ~0u)
			nglProgramUniform1i(progId, vclIdx, m_VPBuiltinCurrent.VertexColorLighted ? 1 : 0);

		// NlVertexFormat: VP and PP have different sources for this uniform.
		// VP uses m_VPBuiltinCurrent.VertexFormat (vertex buffer format).
		// PP uses matDrv->PPBuiltin.VertexFormat (includes texgen-driven texcoord flags).
		{
			uint vfIdx = p->getUniformIndex(CProgramIndex::NlVertexFormat);
			if (vfIdx != ~0u)
			{
				sint32 vertexFormat;
				if (program == VertexProgram)
				{
					vertexFormat = (sint32)m_VPBuiltinCurrent.VertexFormat;
				}
				else
				{
					CMaterialDrvInfosGL3 *matDrv = static_cast<CMaterialDrvInfosGL3 *>((IMaterialDrvInfos *)(mat._MatDrvInfo));
					vertexFormat = matDrv ? (sint32)matDrv->PPBuiltin.VertexFormat : (sint32)m_VPBuiltinCurrent.VertexFormat;
				}
				nglProgramUniform1i(progId, vfIdx, vertexFormat);
			}
		}
	}

	if (!m_ProgramUsesCameraUBO[program])
	{
		// Fog mode
		uint fmIdx = p->getUniformIndex(CProgramIndex::NlFogMode);
		if (fmIdx != ~0u)
			nglProgramUniform1i(progId, fmIdx, (int)_FogMode);

		// Clip plane mask (compute from _ClipPlaneEnabled[], not from VP descriptor
		// which may be zeroed when PP clip planes are active)
		uint cpmIdx = p->getUniformIndex(CProgramIndex::NlClipPlaneMask);
		if (cpmIdx != ~0u)
		{
			sint32 clipMask = 0;
			for (uint ci = 0; ci < MaxClipPlanes; ++ci)
				if (_ClipPlaneEnabled[ci]) clipMask |= (1 << ci);
			nglProgramUniform1i(progId, cpmIdx, clipMask);
		}
	}

	// nlFogEnabled: per-object fog enable state (in NlModel UBO when objectUBO is active)
	if (!m_ProgramUsesObjectUBO[program])
	{
		uint fogEnIdx = p->getUniformIndex(CProgramIndex::NlFogEnabled);
		if (fogEnIdx != ~0u)
			nglProgramUniform1i(progId, fogEnIdx, m_VPBuiltinCurrent.Fog ? 1 : 0);

		// UV routing (from vertex buffer)
		uint uvrIdx = p->getUniformIndex(CProgramIndex::NlUVRouting);
		if (uvrIdx != ~0u)
		{
			sint32 uvr[4];
			if (_CurrentVertexBufferGL)
			{
				const uint8 *vbUvr = _CurrentVertexBufferGL->VB->getUVRouting();
				for (uint i = 0; i < IDRV_MAT_MAXTEXTURES; ++i)
					uvr[i] = (sint32)vbUvr[i];
			}
			else
				for (uint i = 0; i < IDRV_MAT_MAXTEXTURES; ++i)
					uvr[i] = (sint32)i;
			nglProgramUniform4iv(progId, uvrIdx, 1, uvr);
		}
	}

	if (m_ProgramUsesObjectUBO[program])
	{
		// Object UBO handles selfIllumination, light indices/factors, matrices — skip individual uploads

		// Material properties as individual uniforms when material UBO is not active
		// (e.g. wind tree VP uses object UBO but not material UBO).
		// Read from staged MaterialUBO so lightmap per-pass overrides apply automatically.
		if (!m_ProgramUsesMaterialUBO[program])
		{
			CMaterialDrvInfosGL3 *matDrv2 = static_cast<CMaterialDrvInfosGL3 *>((IMaterialDrvInfos *)(mat._MatDrvInfo));
			const CMaterialUBOData &matUBO = matDrv2->MaterialUBO[matDrv2->MaterialUBOCurrent];
			NLMISC::CRGBAF matDiffuse = mat.isLightedVertexColor()
				? NLMISC::CRGBAF(1.0f, 1.0f, 1.0f, 1.0f)
				: NLMISC::CRGBAF(matUBO.materialDiffuse[0], matUBO.materialDiffuse[1],
				                  matUBO.materialDiffuse[2], matUBO.materialDiffuse[3]);
			NLMISC::CRGBAF matSpecular(matUBO.materialSpecular[0], matUBO.materialSpecular[1],
			                            matUBO.materialSpecular[2], matUBO.materialSpecular[3]);

			uint mdIdx = p->getUniformIndex(CProgramIndex::NlMaterialDiffuse);
			if (mdIdx != ~0u)
				nglProgramUniform4f(progId, mdIdx, matDiffuse.R, matDiffuse.G, matDiffuse.B, matDiffuse.A);

			uint msIdx = p->getUniformIndex(CProgramIndex::NlMaterialSpecular);
			if (msIdx != ~0u)
				nglProgramUniform4f(progId, msIdx, matSpecular.R, matSpecular.G, matSpecular.B, matSpecular.A);

			uint mshIdx = p->getUniformIndex(CProgramIndex::NlMaterialShininess);
			if (mshIdx != ~0u)
				nglProgramUniform1f(progId, mshIdx, mat.getShininess());
		}
	}
	else if (m_ProgramUsesLightTableUBO[program])
	{
		// Light table UBO path: per-object indices/factors/material
		// VP lights are shifted: driver slot [m_VPBuiltinCurrent.NumPerPixelLights..7] → VP slot [0..7-N]
		// PP lights are not shifted (PP reads from its own uniform namespace)
		uint lightShift = (program == VertexProgram) ? m_VPBuiltinCurrent.NumPerPixelLights : 0;

		// Per-object light indices and factors
		for (uint i = 0; i < NL_OPENGL3_MAX_LIGHT; ++i)
		{
			uint src = i + lightShift;
			sint32 lightIdx;
			float lightFactor;
			if (src >= NL_OPENGL3_MAX_LIGHT)
			{
				lightIdx = -1;
				lightFactor = 0.0f;
			}
			else if (_LightTableMode)
			{
				// Table mode: indices and factors from setLights()
				lightIdx = (sint32)_LightTableObjIndices[src];
				lightFactor = _LightTableObjFactors[src];
			}
			else
			{
				// Non-table mode: trivial mapping, slot src → UBO entry src
				lightIdx = _LightEnable[src] ? (sint32)src : -1;
				lightFactor = 1.0f;
			}

			// Lightmap pass > 0: zero all light factors (light added only on pass 0)
			if (_LightMapUBOOverride.Active && _LightMapUBOOverride.ZeroLightFactors)
				lightFactor = 0.0f;

			uint idxIdx = p->getUniformIndex(CProgramIndex::TName(CProgramIndex::NlLightIndex0 + i));
			if (idxIdx != ~0u)
				nglProgramUniform1i(progId, idxIdx, lightIdx);

			uint factIdx = p->getUniformIndex(CProgramIndex::TName(CProgramIndex::NlLightFactor0 + i));
			if (factIdx != ~0u)
				nglProgramUniform1f(progId, factIdx, lightFactor);
		}

		if (!m_ProgramUsesMaterialUBO[program])
		{
			// Material properties for GPU-side light×material multiply.
			// Read from staged MaterialUBO so lightmap per-pass overrides apply automatically.
			CMaterialDrvInfosGL3 *matDrv2 = static_cast<CMaterialDrvInfosGL3 *>((IMaterialDrvInfos *)(mat._MatDrvInfo));
			const CMaterialUBOData &matUBO = matDrv2->MaterialUBO[matDrv2->MaterialUBOCurrent];
			NLMISC::CRGBAF matDiffuse = mat.isLightedVertexColor()
				? NLMISC::CRGBAF(1.0f, 1.0f, 1.0f, 1.0f)
				: NLMISC::CRGBAF(matUBO.materialDiffuse[0], matUBO.materialDiffuse[1],
				                  matUBO.materialDiffuse[2], matUBO.materialDiffuse[3]);
			NLMISC::CRGBAF matSpecular(matUBO.materialSpecular[0], matUBO.materialSpecular[1],
			                            matUBO.materialSpecular[2], matUBO.materialSpecular[3]);

			uint mdIdx = p->getUniformIndex(CProgramIndex::NlMaterialDiffuse);
			if (mdIdx != ~0u)
				nglProgramUniform4f(progId, mdIdx, matDiffuse.R, matDiffuse.G, matDiffuse.B, matDiffuse.A);

			uint msIdx = p->getUniformIndex(CProgramIndex::NlMaterialSpecular);
			if (msIdx != ~0u)
				nglProgramUniform4f(progId, msIdx, matSpecular.R, matSpecular.G, matSpecular.B, matSpecular.A);

			uint mshIdx = p->getUniformIndex(CProgramIndex::NlMaterialShininess);
			if (mshIdx != ~0u)
				nglProgramUniform1f(progId, mshIdx, mat.getShininess());
		}

		if (!m_ProgramUsesCameraUBO[program])
		{
			uint pzbIdx = p->getUniformIndex(CProgramIndex::PzbCameraPos);
			if (pzbIdx != ~0u)
				nglProgramUniform3f(progId, pzbIdx, _PZBCameraPos.x, _PZBCameraPos.y, _PZBCameraPos.z);

			uint cwpIdx = p->getUniformIndex(CProgramIndex::CameraWorldPos);
			if (cwpIdx != ~0u)
			{
				// Camera world position: inverse view matrix translation
				CMatrix invView = _ViewMtx;
				invView.invert();
				CVector cwp = invView.getPos();
				nglProgramUniform3f(progId, cwpIdx, cwp.x, cwp.y, cwp.z);
			}
		}
	}
	else if (program == PixelProgram && m_VPBuiltinCurrent.NumPerPixelLights > 0)
	{
		// PP non-table PPL: raw per-light values via pp-prefixed uniforms
		// (computeLightPP handles direction negate and PZB-relative position internally)
		for (uint i = 0; i < NL_OPENGL3_MAX_LIGHT; ++i)
		{
			if (!_LightEnable[i])
				continue;

			// Raw direction or position
			uint ld = p->getUniformIndex(CProgramIndex::TName(CProgramIndex::PpLight0DirOrPos + i));
			if (ld != ~0u)
			{
				CVector v = (_LightMode[i] == CLight::DirectionalLight)
					? _UserLight[i].getDirection()
					: _UserLight[i].getPosition();
				nglProgramUniform3f(progId, ld, v.x, v.y, v.z);
			}

			// Raw diffuse
			uint ldc = p->getUniformIndex(CProgramIndex::TName(CProgramIndex::PpLight0ColDiff + i));
			if (ldc != ~0u)
			{
				NLMISC::CRGBAF diffuse = NLMISC::CRGBAF(_UserLight[i].getDiffuse());
				nglProgramUniform4f(progId, ldc, diffuse.R, diffuse.G, diffuse.B, 0.0f);
			}

			// Raw specular
			uint lsc = p->getUniformIndex(CProgramIndex::TName(CProgramIndex::PpLight0ColSpec + i));
			if (lsc != ~0u)
			{
				NLMISC::CRGBAF specular = NLMISC::CRGBAF(_UserLight[i].getSpecular());
				nglProgramUniform4f(progId, lsc, specular.R, specular.G, specular.B, 0.0f);
			}

			// Attenuation (same raw values)
			uint lca = p->getUniformIndex(CProgramIndex::TName(CProgramIndex::PpLight0ConstAttn + i));
			if (lca != ~0u)
				nglProgramUniform1f(progId, lca, _UserLight[i].getConstantAttenuation());
			uint lla = p->getUniformIndex(CProgramIndex::TName(CProgramIndex::PpLight0LinAttn + i));
			if (lla != ~0u)
				nglProgramUniform1f(progId, lla, _UserLight[i].getLinearAttenuation());
			uint lqa = p->getUniformIndex(CProgramIndex::TName(CProgramIndex::PpLight0QuadAttn + i));
			if (lqa != ~0u)
				nglProgramUniform1f(progId, lqa, _UserLight[i].getQuadraticAttenuation());

			// Spot params
			if (_LightMode[i] == CLight::SpotLight)
			{
				uint lsd = p->getUniformIndex(CProgramIndex::TName(CProgramIndex::PpLight0SpotDir + i));
				if (lsd != ~0u)
				{
					CVector d = _UserLight[i].getDirection();
					nglProgramUniform3f(progId, lsd, d.x, d.y, d.z);
				}
				uint lsc2 = p->getUniformIndex(CProgramIndex::TName(CProgramIndex::PpLight0SpotCutoff + i));
				if (lsc2 != ~0u)
					nglProgramUniform1f(progId, lsc2, cosf(_UserLight[i].getCutoff()));
				uint lse = p->getUniformIndex(CProgramIndex::TName(CProgramIndex::PpLight0SpotExp + i));
				if (lse != ~0u)
					nglProgramUniform1f(progId, lse, _UserLight[i].getExponent());
			}
		}

		// Material properties (when !materialUBO).
		// Read from staged MaterialUBO so lightmap per-pass overrides apply automatically.
		if (!m_ProgramUsesMaterialUBO[program])
		{
			CMaterialDrvInfosGL3 *matDrv2 = static_cast<CMaterialDrvInfosGL3 *>((IMaterialDrvInfos *)(mat._MatDrvInfo));
			const CMaterialUBOData &matUBO = matDrv2->MaterialUBO[matDrv2->MaterialUBOCurrent];
			NLMISC::CRGBAF matDiffuse = mat.isLightedVertexColor()
				? NLMISC::CRGBAF(1.0f, 1.0f, 1.0f, 1.0f)
				: NLMISC::CRGBAF(matUBO.materialDiffuse[0], matUBO.materialDiffuse[1],
				                  matUBO.materialDiffuse[2], matUBO.materialDiffuse[3]);
			NLMISC::CRGBAF matSpecular(matUBO.materialSpecular[0], matUBO.materialSpecular[1],
			                            matUBO.materialSpecular[2], matUBO.materialSpecular[3]);

			uint mdIdx = p->getUniformIndex(CProgramIndex::NlMaterialDiffuse);
			if (mdIdx != ~0u)
				nglProgramUniform4f(progId, mdIdx, matDiffuse.R, matDiffuse.G, matDiffuse.B, matDiffuse.A);

			uint msIdx = p->getUniformIndex(CProgramIndex::NlMaterialSpecular);
			if (msIdx != ~0u)
				nglProgramUniform4f(progId, msIdx, matSpecular.R, matSpecular.G, matSpecular.B, matSpecular.A);

			uint mshIdx = p->getUniformIndex(CProgramIndex::NlMaterialShininess);
			if (mshIdx != ~0u)
				nglProgramUniform1f(progId, mshIdx, mat.getShininess());
		}

		// pzbCameraPos (when !cameraUBO)
		if (!m_ProgramUsesCameraUBO[program])
		{
			uint pzbIdx = p->getUniformIndex(CProgramIndex::PzbCameraPos);
			if (pzbIdx != ~0u)
				nglProgramUniform3f(progId, pzbIdx, _PZBCameraPos.x, _PZBCameraPos.y, _PZBCameraPos.z);

			uint cwpIdx = p->getUniformIndex(CProgramIndex::CameraWorldPos);
			if (cwpIdx != ~0u)
			{
				CMatrix invView = _ViewMtx;
				invView.invert();
				CVector cwp = invView.getPos();
				nglProgramUniform3f(progId, cwpIdx, cwp.x, cwp.y, cwp.z);
			}
		}
	}
	else if (program == VertexProgram)
	{
		// Legacy per-light uniform path (VP-only: pre-multiplied light×material)
		// VP lights are shifted: driver slot [m_VPBuiltinCurrent.NumPerPixelLights..7] → VP slot [0..7-N]
		// Read from staged MaterialUBO so lightmap per-pass overrides apply automatically.
		CMaterialDrvInfosGL3 *matDrv2 = static_cast<CMaterialDrvInfosGL3 *>((IMaterialDrvInfos *)(mat._MatDrvInfo));
		const CMaterialUBOData &matUBO = matDrv2->MaterialUBO[matDrv2->MaterialUBOCurrent];
		NLMISC::CRGBAF matDiffuse = mat.isLightedVertexColor()
			? NLMISC::CRGBAF(1.0f, 1.0f, 1.0f, 1.0f)
			: NLMISC::CRGBAF(matUBO.materialDiffuse[0], matUBO.materialDiffuse[1],
			                  matUBO.materialDiffuse[2], matUBO.materialDiffuse[3]);
		NLMISC::CRGBAF matSpecular(matUBO.materialSpecular[0], matUBO.materialSpecular[1],
		                            matUBO.materialSpecular[2], matUBO.materialSpecular[3]);

		for (uint i = 0; i < NL_OPENGL3_MAX_LIGHT; ++i)
		{
			uint src = i + m_VPBuiltinCurrent.NumPerPixelLights;
			if (src >= NL_OPENGL3_MAX_LIGHT || !_LightEnable[src])
				continue;

			if (_LightMode[src] == CLight::DirectionalLight)
			{
				uint ld = p->getUniformIndex(CProgramIndex::TName(CProgramIndex::Light0DirOrPos + i));
				if (ld != ~0)
				{
					CVector v = -1 * _UserLight[src].getDirection();
					nglProgramUniform3f(progId, ld, v.x, v.y, v.z);
				}
			}
			else if (_LightMode[src] == CLight::PointLight || _LightMode[src] == CLight::SpotLight)
			{
				uint lp = p->getUniformIndex(CProgramIndex::TName(CProgramIndex::Light0DirOrPos + i));
				if (lp != ~0)
				{
					CVector v = _UserLight[src].getPosition() - _PZBCameraPos;
					nglProgramUniform3f(progId, lp, v.x, v.y, v.z);
				}
			}
			else
			{
				continue; // Unknown light mode
			}

			uint ldc = p->getUniformIndex(CProgramIndex::TName(CProgramIndex::Light0ColDiff + i));
			if (ldc != ~0)
			{
				NLMISC::CRGBAF diffuse = NLMISC::CRGBAF(_UserLight[src].getDiffuse()) * matDiffuse;
				nglProgramUniform4f(progId, ldc, diffuse.R, diffuse.G, diffuse.B, 0.0f); // 1.0f?
			}

			uint lsc = p->getUniformIndex(CProgramIndex::TName(CProgramIndex::Light0ColSpec + i));
			if (lsc != ~0)
			{
				NLMISC::CRGBAF specular = NLMISC::CRGBAF(_UserLight[src].getSpecular()) * matSpecular;
				nglProgramUniform4f(progId, lsc, specular.R, specular.G, specular.B, 0.0f); // 1.0f?
			}

			uint shl = p->getUniformIndex(CProgramIndex::TName(CProgramIndex::Light0Shininess + i));
			if (shl != ~0)
			{
				nglProgramUniform1f(progId, shl, mat.getShininess());
			}

			uint lca = p->getUniformIndex(CProgramIndex::TName(CProgramIndex::Light0ConstAttn + i));
			if (lca != ~0)
			{
				nglProgramUniform1f(progId, lca, _UserLight[src].getConstantAttenuation());
			}

			uint lla = p->getUniformIndex(CProgramIndex::TName(CProgramIndex::Light0LinAttn + i));
			if (lla != ~0)
			{
				nglProgramUniform1f(progId, lla, _UserLight[src].getLinearAttenuation());
			}

			uint lqa = p->getUniformIndex(CProgramIndex::TName(CProgramIndex::Light0QuadAttn + i));
			if (lqa != ~0)
			{
				nglProgramUniform1f(progId, lqa, _UserLight[src].getQuadraticAttenuation());
			}

			if (_LightMode[src] == CLight::SpotLight)
			{
				uint lsd = p->getUniformIndex(CProgramIndex::TName(CProgramIndex::Light0SpotDir + i));
				if (lsd != ~0)
				{
					CVector d = _UserLight[src].getDirection();
					nglProgramUniform3f(progId, lsd, d.x, d.y, d.z);
				}

				uint lsc2 = p->getUniformIndex(CProgramIndex::TName(CProgramIndex::Light0SpotCutoff + i));
				if (lsc2 != ~0)
				{
					nglProgramUniform1f(progId, lsc2, cosf(_UserLight[src].getCutoff()));
				}

				uint lse = p->getUniformIndex(CProgramIndex::TName(CProgramIndex::Light0SpotExp + i));
				if (lse != ~0)
				{
					nglProgramUniform1f(progId, lse, _UserLight[src].getExponent());
				}
			}
		}
	}

	// Upload clip plane uniforms (eye-space plane equations) — skip when camera UBO active
	if (!m_ProgramUsesCameraUBO[program])
	{
		for (uint i = 0; i < MaxClipPlanes; ++i)
		{
			if (!_ClipPlaneEnabled[i])
				continue;
			uint cpIdx = p->getUniformIndex(CProgramIndex::TName(CProgramIndex::ClipPlane0 + i));
			if (cpIdx != ~0u)
				nglProgramUniform4f(progId, cpIdx, _ClipPlaneEye[i][0], _ClipPlaneEye[i][1], _ClipPlaneEye[i][2], _ClipPlaneEye[i][3]);
		}
	}

	// Water bump factors — read from the water UB staging area (single source of truth)
	if (program == PixelProgram && _WaterUB)
	{
		uint b0idx = p->getUniformIndex(CProgramIndex::Bump0ScaleBias);
		if (b0idx != ~0u)
			nglProgramUniform4fv(progId, b0idx, 1, _WaterUB->getFloat4(_WaterUBOOffsets.Bump0ScaleBias));
		uint b1idx = p->getUniformIndex(CProgramIndex::Bump1ScaleBias);
		if (b1idx != ~0u)
			nglProgramUniform4fv(progId, b1idx, 1, _WaterUB->getFloat4(_WaterUBOOffsets.Bump1ScaleBias));
	}
}

void CDriverGL3::setupInitialUniforms(IProgram *program)
{
	IProgramDrvInfos *di = program->m_DrvInfo;
	if (di != NULL)
	{
		CProgramDrvInfosGL3 *drvInfo = static_cast<CProgramDrvInfosGL3 *>(di);
		GLuint id = drvInfo->getProgramId();

		for (uint i = 0; i < std::min(_Extensions.MaxFragmentTextureImageUnits, (GLint)IDRV_PROGRAM_MAXSAMPLERS); ++i)
		{
			uint samplerIdx = program->getUniformIndex((CProgramIndex::TName)(CProgramIndex::Sampler0 + i));
			if (samplerIdx >= 0)
			{
#ifdef USE_OPENGLES3
				// GLES 3.0: nglProgramUniform1i is a no-op (SSO not available).
				// Use glUseProgram + glUniform1i instead.
				nglUseProgram(id);
				glUniform1i(samplerIdx, i);
				nglUseProgram(0);
#else
				nglProgramUniform1i(id, samplerIdx, i);
#endif
			}
		}

		// Resolve and cache NlLightTable UBO block index, bind to its binding point
		GLuint lightTableBlock = nglGetUniformBlockIndex(id, "NlLightTable");
		drvInfo->setLightTableBlockIndex(lightTableBlock);
		if (lightTableBlock != GL_INVALID_INDEX)
			nglUniformBlockBinding(id, lightTableBlock, NL_BUILTIN_LIGHT_TABLE_BINDING);

		// Resolve and cache NlCamera UBO block index, bind to its binding point
		GLuint cameraBlock = nglGetUniformBlockIndex(id, "NlCamera");
		drvInfo->setCameraBlockIndex(cameraBlock);
		if (cameraBlock != GL_INVALID_INDEX)
			nglUniformBlockBinding(id, cameraBlock, NL_BUILTIN_CAMERA_BINDING);

		// Resolve and cache NlModel UBO block index, bind to its binding point
		GLuint objectBlock = nglGetUniformBlockIndex(id, "NlModel");
		drvInfo->setObjectBlockIndex(objectBlock);
		if (objectBlock != GL_INVALID_INDEX)
			nglUniformBlockBinding(id, objectBlock, NL_BUILTIN_MODEL_BINDING);

		// Resolve and cache NlMaterial UBO block index, bind to its binding point
		GLuint materialBlock = nglGetUniformBlockIndex(id, "NlMaterial");
		drvInfo->setMaterialBlockIndex(materialBlock);
		if (materialBlock != GL_INVALID_INDEX)
			nglUniformBlockBinding(id, materialBlock, NL_BUILTIN_MATERIAL_BINDING);

		// Resolve and bind user UBO blocks (VP/PP) — binding points set at link time
		IProgram::CSource *src = program->source();
		if (src)
		{
			static const sint s_UBBindingToGL[] = {
				NL_USER_VERTEX_PROGRAM_BINDING,  // UBBindingVertexProgram
				NL_USER_PIXEL_PROGRAM_BINDING,   // UBBindingPixelProgram
				NL_USER_SKELETON_BINDING,        // UBBindingSkeleton
			};
			for (std::map<sint, NLMISC::CSmartPtr<CUniformBufferFormat> >::const_iterator
				it = src->UniformBufferFormats.begin(); it != src->UniformBufferFormats.end(); ++it)
			{
				nlassert(it->first >= 0 && it->first < UBBindingCount);
				const CUniformBufferFormat &ubf = *it->second;
				std::string blockName = ubf.Name.empty()
					? ("NlUBO" + NLMISC::toString(s_UBBindingToGL[it->first]))
					: ubf.Name;
				GLuint blockIdx = nglGetUniformBlockIndex(id, blockName.c_str());
				if (blockIdx != GL_INVALID_INDEX)
					nglUniformBlockBinding(id, blockIdx, s_UBBindingToGL[it->first]);
			}
		}
	}
}

CShaderProgram *CDriverGL3::linkPrograms(
	IProgram *vpProg, const CProgramFeatures &vpFeatures,
	IProgram *ppProg, const CProgramFeatures &ppFeatures)
{
	if (!vpProg || !ppProg || !vpProg->m_DrvInfo || !ppProg->m_DrvInfo)
		return NULL;

	CProgramDrvInfosGL3 *vpDrvInfo = static_cast<CProgramDrvInfosGL3 *>((IProgramDrvInfos *)vpProg->m_DrvInfo);
	CProgramDrvInfosGL3 *ppDrvInfo = static_cast<CProgramDrvInfosGL3 *>((IProgramDrvInfos *)ppProg->m_DrvInfo);

	// Extract attached shaders from single-stage programs
	GLuint vpShader = 0, ppShader = 0;
	GLsizei count = 0;
	nglGetAttachedShaders(vpDrvInfo->getProgramId(), 1, &count, &vpShader);
	if (count == 0 || vpShader == 0)
	{
		nlwarning("GL3: Failed to extract VP shader for linked program");
		return NULL;
	}
	count = 0;
	nglGetAttachedShaders(ppDrvInfo->getProgramId(), 1, &count, &ppShader);
	if (count == 0 || ppShader == 0)
	{
		nlwarning("GL3: Failed to extract PP shader for linked program");
		return NULL;
	}

	// Create combined linked program
	GLuint linkedProg = nglCreateProgram();
	nglAttachShader(linkedProg, vpShader);
	nglAttachShader(linkedProg, ppShader);
	nglLinkProgram(linkedProg);

	GLint linkOk;
	nglGetProgramiv(linkedProg, GL_LINK_STATUS, &linkOk);
	if (linkOk == 0)
	{
		char errorLog[1024];
		nglGetProgramInfoLog(linkedProg, 1024, NULL, errorLog);
		nlwarning("GL3: Linked program link failed: %s", errorLog);
		nglDeleteProgram(linkedProg);
		return NULL;
	}

	nldebug("GL3: Linked user program id=%u (VP=%s, PP=%s)",
		linkedProg,
		vpProg->source() ? vpProg->source()->DisplayName.c_str() : "?",
		ppProg->source() ? ppProg->source()->DisplayName.c_str() : "?");

	// Override OnlyUBOs based on actual linked program introspection
	bool linkedHasNonUBO = programHasNonUBOUniforms(linkedProg);

	// Create CShaderProgram wrapping the combined linked program
	CShaderProgram *sp = new CShaderProgram();
	{
		IProgram::CSource *src = new IProgram::CSource();
		src->Profile = IProgram::glsl300es;
		src->DisplayName = (vpProg->source() ? vpProg->source()->DisplayName : "VP")
			+ "+" + (ppProg->source() ? ppProg->source()->DisplayName : "PP") + " [linked]";
		sp->VPFeatures = vpFeatures;
		sp->PPFeatures = ppFeatures;
		if (linkedHasNonUBO)
		{
			if (sp->VPFeatures.OnlyUBOs)
				nlwarning("GL3: Linked VP '%s' claims OnlyUBOs but linked program has non-UBO uniforms",
					vpProg->source() ? vpProg->source()->DisplayName.c_str() : "?");
			if (sp->PPFeatures.OnlyUBOs)
				nlwarning("GL3: Linked PP '%s' claims OnlyUBOs but linked program has non-UBO uniforms",
					ppProg->source() ? ppProg->source()->DisplayName.c_str() : "?");
			sp->VPFeatures.OnlyUBOs = false;
			sp->PPFeatures.OnlyUBOs = false;
		}
		else
		{
			// Don't auto-promote OnlyUBOs for nelvp-converted VPs — callers
			// still use setUniform* which routes through the nelvp UBO internally.
			bool vpIsNelvp = vpProg && vpProg->m_DrvInfo
				&& static_cast<CProgramDrvInfosGL3 *>((IProgramDrvInfos *)vpProg->m_DrvInfo)->isNelvpConverted;
			if (!vpIsNelvp)
				sp->VPFeatures.OnlyUBOs = true;
			sp->PPFeatures.OnlyUBOs = true;
		}
		src->Features = sp->VPFeatures;
		// Merge user UBO formats from both VP and PP sources so setupInitialUniforms
		// can resolve all user UBO block bindings in the linked program
		if (vpProg->source())
			src->UniformBufferFormats.insert(vpProg->source()->UniformBufferFormats.begin(),
				vpProg->source()->UniformBufferFormats.end());
		if (ppProg->source())
			src->UniformBufferFormats.insert(ppProg->source()->UniformBufferFormats.begin(),
				ppProg->source()->UniformBufferFormats.end());
		sp->addSource(src);

		ItGPUPrgDrvInfoPtrList it = _GPUPrgDrvInfos.insert(_GPUPrgDrvInfos.end(), (NL3D::IProgramDrvInfos*)NULL);
		CProgramDrvInfosGL3 *drv = new CProgramDrvInfosGL3(this, it);
		*it = drv;
		drv->setProgramId(linkedProg);
		sp->m_DrvInfo = drv;
		sp->buildInfo(src);
		setupInitialUniforms(sp);
	}

	return sp;
}

bool CDriverGL3::setupUserLinkedPrograms(CVertexProgram *vpProg, CPixelProgram *ppProg)
{
	nlassert(vpProg || ppProg); // At least one must be set
	nlassert(_CurrentMaterial);

	CMaterial &mat = *_CurrentMaterial;
	CMaterialDrvInfosGL3 *matDrv = static_cast<CMaterialDrvInfosGL3 *>((IMaterialDrvInfos *)(mat._MatDrvInfo));
	nlassert(matDrv);

	// --- VP-side state ---
	touchVertexFormatVP();

	// Resolve PPL support from both VP and PP
	m_ProgramSupportsPPL = ppProg ? ppProg->features().SupportPPL : true;
	if (vpProg)
		m_ProgramSupportsPPL = m_ProgramSupportsPPL && vpProg->features().SupportPPL;

	{
		uint8 numPpl = (uint8)(m_ProgramSupportsPPL ? _NumPerPixelLights : 0);
		if (m_VPBuiltinCurrent.NumPerPixelLights != numPpl)
		{
			m_VPBuiltinCurrent.NumPerPixelLights = numPpl;
			if (m_VPBuiltinCurrent.Lighting)
				m_VPBuiltinTouched = true;
		}
	}

	if (vpProg)
	{
		m_VPSpecularOutput = vpProg->features().OutputsSpecularColor;
		m_VPWorldSpacePositionOutput = vpProg->features().OutputsWorldSpacePosition;
		m_VPNormalOutput = false;
		if (m_VPBuiltinCurrent.NumPerPixelLights > 0 && vpProg->features().UsesObjectUBO)
		{
			m_VPWorldSpacePositionOutput = true;
			m_VPNormalOutput = true;
		}
	}
	else
	{
		// Mega VP
		m_VPSpecularOutput = true;
		m_VPNormalOutput = false;
		m_VPWorldSpacePositionOutput = false;

		bool pplActive = false;
		if (m_VPBuiltinCurrent.NumPerPixelLights > 0)
		{
			if (ppProg)
			{
				if (ppProg->features().UsesObjectUBO)
					pplActive = true;
				else if (ppProg->features().InputsWorldSpacePosition
				      && ppProg->features().InputsWorldSpaceNormal)
					pplActive = true;
			}
			else
			{
				pplActive = true;
			}
		}
		if (pplActive)
		{
			m_VPWorldSpacePositionOutput = true;
			m_VPNormalOutput = true;
		}
	}

	// --- PP-side state ---
	if (!ppProg)
	{
		// Mega PP state
		matDrv->PPBuiltin.checkDriverStateTouched(this);
	}

	// --- Check linkability ---
	// Insert VPs are linkable via their pre-compiled linked inner variants
	bool vpIsInsert = false;
	if (vpProg && vpProg->m_DrvInfo)
	{
		CProgramDrvInfosGL3 *vpDi = static_cast<CProgramDrvInfosGL3 *>((IProgramDrvInfos *)vpProg->m_DrvInfo);
		vpIsInsert = vpDi->isInsertProgram;
	}
	bool vpLinkable = vpProg
		? (vpIsInsert || (vpProg->source() && vpProg->source()->Profile == IProgram::glsl300esv))
		: true; // Mega VP linked=1 variants always linkable
	bool ppLinkable = ppProg
		? (ppProg->source() && ppProg->source()->Profile == IProgram::glsl300esf)
		: true; // Mega PP linked=1 variants always linkable

	if (!vpLinkable && !ppLinkable)
		return false; // Neither program is linkable, fall back to SSO

	// If only one side is linkable and the other is a non-mega program that's not, fall back
	if (vpProg && !vpLinkable)
		return false;
	if (ppProg && !ppLinkable)
		return false;

	// --- Compile programs if needed ---
	if (vpProg && !vpProg->m_DrvInfo)
	{
		if (!compileVertexProgram(vpProg))
			return false;
	}
	if (ppProg && !ppProg->m_DrvInfo)
	{
		if (!compilePixelProgram(ppProg))
			return false;
	}

	// --- Compute dimension indices ---
	bool pplActive = m_VPBuiltinCurrent.NumPerPixelLights > 0
		&& (m_VPWorldSpacePositionOutput && m_VPNormalOutput);
	int ppClip = m_VPBuiltinCurrent.PPClipPlane ? 1 : 0;
	int fogOrPpl = (m_VPBuiltinCurrent.Fog || pplActive || ppClip) ? 1 : 0;
	int hwClip = (m_VPBuiltinCurrent.ClipPlaneMask != 0) ? 1 : 0;
	int cube = (matDrv->PPBuiltin.TexSamplerMode != 0) ? 1 : 0;
	int specular = m_VPSpecularOutput ? 1 : 0;

	CShaderProgram *sp = NULL;

	if (vpIsInsert && !ppProg)
	{
		// Case: Insert VP + Mega PP — use pre-compiled linked inner VP, link with mega PP
		CProgramDrvInfosGL3 *vpDrv = static_cast<CProgramDrvInfosGL3 *>((IProgramDrvInfos *)vpProg->m_DrvInfo);
		sp = vpDrv->InsertLinkedVPMegaPP[fogOrPpl][cube][specular][ppClip];
		if (!sp)
		{
			CVertexProgram *innerVP = vpDrv->InsertMegaVP[1][fogOrPpl][hwClip];
			if (!innerVP || !innerVP->m_DrvInfo) return false;
			int ppTableUBO = fogOrPpl ? 1 : 0;
			CPixelProgram *megaPP = m_MegaPP[1][fogOrPpl][cube][specular][ppClip][ppTableUBO][1][1][1];
			if (!megaPP || !megaPP->m_DrvInfo) return false;
			sp = linkPrograms(innerVP, innerVP->source()->Features,
				megaPP, megaPP->source()->Features);
			if (!sp) return false;
			vpDrv->InsertLinkedVPMegaPP[fogOrPpl][cube][specular][ppClip] = sp;
		}
	}
	else if (vpProg && !ppProg)
	{
		// Case A: User/Material VP + Mega PP
		CProgramDrvInfosGL3 *vpDrv = static_cast<CProgramDrvInfosGL3 *>((IProgramDrvInfos *)vpProg->m_DrvInfo);
		sp = vpDrv->LinkedVPMegaPP[fogOrPpl][cube][specular][ppClip];
		if (!sp)
		{
			int ppTableUBO = fogOrPpl ? 1 : 0;
			CPixelProgram *megaPP = m_MegaPP[1][fogOrPpl][cube][specular][ppClip][ppTableUBO][1][1][1];
			if (!megaPP || !megaPP->m_DrvInfo) return false;
			sp = linkPrograms(vpProg, vpProg->source()->Features,
				megaPP, megaPP->source()->Features);
			if (!sp) return false;
			vpDrv->LinkedVPMegaPP[fogOrPpl][cube][specular][ppClip] = sp;
		}
	}
	else if (!vpProg && ppProg)
	{
		// Case B: Mega VP + User/Material PP
		CProgramDrvInfosGL3 *ppDrv = static_cast<CProgramDrvInfosGL3 *>((IProgramDrvInfos *)ppProg->m_DrvInfo);
		sp = ppDrv->LinkedMegaVPPP[fogOrPpl][hwClip];
		if (!sp)
		{
			CVertexProgram *megaVP = m_MegaVP[1][fogOrPpl][hwClip][1][1][1][1];
			if (!megaVP || !megaVP->m_DrvInfo) return false;
			sp = linkPrograms(megaVP, megaVP->source()->Features,
				ppProg, ppProg->source()->Features);
			if (!sp) return false;
			ppDrv->LinkedMegaVPPP[fogOrPpl][hwClip] = sp;
		}
	}
	else
	{
		// Case C: User/Material VP + User/Material PP
		CProgramDrvInfosGL3 *vpDrv = static_cast<CProgramDrvInfosGL3 *>((IProgramDrvInfos *)vpProg->m_DrvInfo);
		CProgramDrvInfosGL3 *ppDrv = static_cast<CProgramDrvInfosGL3 *>((IProgramDrvInfos *)ppProg->m_DrvInfo);
		std::map<CProgramDrvInfosGL3*, NLMISC::CSmartPtr<CShaderProgram> >::iterator it = vpDrv->LinkedUserVPPP.find(ppDrv);
		if (it != vpDrv->LinkedUserVPPP.end())
		{
			sp = it->second;
		}
		else
		{
			sp = linkPrograms(vpProg, vpProg->source()->Features,
				ppProg, ppProg->source()->Features);
			if (!sp) return false;
			vpDrv->LinkedUserVPPP[ppDrv] = sp;
		}
	}

	nlassert(sp);
	nlassert(sp->m_DrvInfo);

	CProgramDrvInfosGL3 *spDrvInfo = static_cast<CProgramDrvInfosGL3 *>((IProgramDrvInfos *)sp->m_DrvInfo);
	GLuint linkedProgId = spDrvInfo->getProgramId();
	nlassert(linkedProgId != 0);

	// Unbind PPO and activate linked program
	if (m_PPOBound)
	{
		_DriverGLStates.forceBindProgramPipeline(0);
		m_PPOBound = false;
	}
	_DriverGLStates.forceUseProgram(linkedProgId);

	// Set shader program — getProgram/getProgramId will use this for both VP and PP
	m_DriverShaderProgram = sp;
	m_DriverVertexProgram = NULL;
	m_DriverPixelProgram = NULL;

	// Set UBO flags from VP features
	m_ProgramNoUniforms[VertexProgram] = sp->VPFeatures.NoUniforms;
	m_ProgramNoBuiltinUniforms[VertexProgram] = sp->VPFeatures.NoBuiltinUniforms;
	m_ProgramOnlyUBOs[VertexProgram] = sp->VPFeatures.OnlyUBOs;
	m_ProgramUsesLightTableUBO[VertexProgram] = sp->VPFeatures.UsesLightTableUBO;
	m_ProgramUsesCameraUBO[VertexProgram] = sp->VPFeatures.UsesCameraUBO;
	m_ProgramUsesObjectUBO[VertexProgram] = sp->VPFeatures.UsesObjectUBO;
	m_ProgramUsesMaterialUBO[VertexProgram] = sp->VPFeatures.UsesMaterialUBO;

	// Set UBO flags from PP features
	m_ProgramNoUniforms[PixelProgram] = sp->PPFeatures.NoUniforms;
	m_ProgramNoBuiltinUniforms[PixelProgram] = sp->PPFeatures.NoBuiltinUniforms;
	m_ProgramOnlyUBOs[PixelProgram] = sp->PPFeatures.OnlyUBOs;
	m_ProgramUsesLightTableUBO[PixelProgram] = sp->PPFeatures.UsesLightTableUBO;
	m_ProgramUsesCameraUBO[PixelProgram] = sp->PPFeatures.UsesCameraUBO;
	m_ProgramUsesObjectUBO[PixelProgram] = sp->PPFeatures.UsesObjectUBO;
	m_ProgramUsesMaterialUBO[PixelProgram] = sp->PPFeatures.UsesMaterialUBO;

	// Track nelvp UBO for the linked path
	m_NelvpActiveUB = NULL;
	if (vpProg && vpProg->m_DrvInfo)
	{
		CProgramDrvInfosGL3 *vpDi = static_cast<CProgramDrvInfosGL3 *>(
			(IProgramDrvInfos *)vpProg->m_DrvInfo);
		if (vpDi->isNelvpConverted && vpDi->NelvpConstantUB)
		{
			bindUniformBuffer(UBBindingVertexProgram, vpDi->NelvpConstantUB);
			m_NelvpActiveUB = vpDi->NelvpConstantUB;
			m_ProgramOnlyUBOs[VertexProgram] = true;
		}
	}

	return true;
}

bool CDriverGL3::initMegaLinkedPrograms()
{
	// Linked shader programs are always fully UBO-backed.
	int tableUBO = 1;
	int cameraUBO = 1;
	int objectUBO = 1;
	int materialUBO = 1;

#ifdef __EMSCRIPTEN__
	{
		int linkCount = 0;
		for (int fogOrPpl = 0; fogOrPpl < 2; ++fogOrPpl)
		for (int hwClip = 0; hwClip < 2; ++hwClip)
		{
			if (hwClip && m_PPClipPlanes) continue;
			for (int cube = 0; cube < 2; ++cube)
			for (int specular = 0; specular < 2; ++specular)
			for (int ppClip = 0; ppClip < 2; ++ppClip)
			{
				if (ppClip && !fogOrPpl) continue;
				if (ppClip && !m_PPClipPlanes) continue;
				linkCount++;
			}
		}
		EM_ASM({ window.nlBeginTask('Linking shader programs', $0); }, linkCount);
	}
#endif

	for (int fogOrPpl = 0; fogOrPpl < 2; ++fogOrPpl)
	{
		for (int hwClip = 0; hwClip < 2; ++hwClip)
		{
			// m_PPClipPlanes zeroes ClipPlaneMask on VP, so hwClip=1 is never selected
			if (hwClip && m_PPClipPlanes) continue;

			for (int cube = 0; cube < 2; ++cube)
			{
				for (int specular = 0; specular < 2; ++specular)
				{
					for (int ppClip = 0; ppClip < 2; ++ppClip)
					{
						// ppClip only valid when fogOrPpl (needs ecPos varying)
						if (ppClip && !fogOrPpl)
							continue;
						if (ppClip && !m_PPClipPlanes) continue;

						// Get linked=1 VP and PP single-stage programs.
						// PP tableUBO folds to 0 when !fogOrPpl (no lighting code).
						int ppTableUBO = fogOrPpl ? tableUBO : 0;
						CVertexProgram *vpProg = m_MegaVP[1][fogOrPpl][hwClip][tableUBO][cameraUBO][objectUBO][materialUBO];
						CPixelProgram *ppProg = m_MegaPP[1][fogOrPpl][cube][specular][ppClip][ppTableUBO][cameraUBO][objectUBO][materialUBO];

						if (!vpProg || !ppProg)
						{
							nlwarning("GL3: Missing mega program (vpProg=%p, ppProg=%p) for linked pipeline (fogOrPpl=%d, hwClip=%d, cube=%d, specular=%d, ppClip=%d)", (void *)vpProg, (void *)ppProg, fogOrPpl, hwClip, cube, specular, ppClip);
							return false;
						}

						CShaderProgram *sp = linkPrograms(vpProg, vpProg->source()->Features,
							ppProg, ppProg->source()->Features);
						if (!sp)
						{
							nlwarning("GL3: Failed to link mega program (fogOrPpl=%d, hwClip=%d, cube=%d, specular=%d, ppClip=%d)", fogOrPpl, hwClip, cube, specular, ppClip);
							return false;
						}
#ifdef __EMSCRIPTEN__
						EM_ASM({ window.nlStepTask('Linking shader programs'); });
						emscripten_sleep(0); // Yield to browser to prevent WebGL context timeout
#endif

						m_MegaLinked[fogOrPpl][hwClip][cube][specular][ppClip] = sp;
					}
				}
			}
		}
	}

	return true;
}

bool CDriverGL3::setupMegaLinkedPrograms()
{
	nlassert(!m_UserVertexProgram);
	nlassert(!m_UserPixelProgram);
	nlassert(_CurrentMaterial);

	CMaterial &mat = *_CurrentMaterial;
	CMaterialDrvInfosGL3 *matDrv = static_cast<CMaterialDrvInfosGL3 *>((IMaterialDrvInfos *)(mat._MatDrvInfo));
	nlassert(matDrv);

	// --- VP-side state (replaces setupBuiltinVertexProgram + setupMegaVertexProgram) ---
	touchVertexFormatVP();

	// PPL support: mega VP+PP always supports PPL, no user programs to check
	m_ProgramSupportsPPL = true;
	{
		uint8 numPpl = (uint8)_NumPerPixelLights;
		if (m_VPBuiltinCurrent.NumPerPixelLights != numPpl)
		{
			m_VPBuiltinCurrent.NumPerPixelLights = numPpl;
			if (m_VPBuiltinCurrent.Lighting)
				m_VPBuiltinTouched = true;
		}
	}

	m_VPSpecularOutput = true; // Mega VP always outputs specularColor
	m_VPNormalOutput = false;
	m_VPWorldSpacePositionOutput = false;

	bool pplActive = m_VPBuiltinCurrent.NumPerPixelLights > 0;
	if (pplActive)
	{
		m_VPWorldSpacePositionOutput = true;
		m_VPNormalOutput = true;
	}

	// --- PP-side state (replaces setupBuiltinPixelProgram + setupMegaPixelProgram) ---
	matDrv->PPBuiltin.checkDriverStateTouched(this);
	// --- Compute dimension indices ---
	int fogOrPpl = (m_VPBuiltinCurrent.Fog || pplActive || m_VPBuiltinCurrent.PPClipPlane) ? 1 : 0;
	int hwClip = (m_VPBuiltinCurrent.ClipPlaneMask != 0) ? 1 : 0;
	int ppClip = m_VPBuiltinCurrent.PPClipPlane ? 1 : 0;
	int cube = (matDrv->PPBuiltin.TexSamplerMode != 0) ? 1 : 0;
	int specular = m_VPSpecularOutput ? 1 : 0;

	CShaderProgram *sp = m_MegaLinked[fogOrPpl][hwClip][cube][specular][ppClip];
	nlassert(sp);
	nlassert(sp->m_DrvInfo);

	CProgramDrvInfosGL3 *spDrvInfo = static_cast<CProgramDrvInfosGL3 *>((IProgramDrvInfos *)sp->m_DrvInfo);
	GLuint linkedProgId = spDrvInfo->getProgramId();
	nlassert(linkedProgId != 0);

	// Unbind PPO and activate linked program
	if (m_PPOBound)
	{
		_DriverGLStates.forceBindProgramPipeline(0);
		m_PPOBound = false;
	}
	_DriverGLStates.forceUseProgram(linkedProgId);

	// Set shader program — getProgram/getProgramId will use this for both VP and PP
	m_DriverShaderProgram = sp;
	m_DriverVertexProgram = NULL;
	m_DriverPixelProgram = NULL;

	// Linked shader programs are UBO-only
	m_ProgramNoUniforms[VertexProgram] = false;
	m_ProgramNoBuiltinUniforms[VertexProgram] = false;
	m_ProgramOnlyUBOs[VertexProgram] = true;
	m_ProgramUsesLightTableUBO[VertexProgram] = sp->VPFeatures.UsesLightTableUBO;
	m_ProgramUsesCameraUBO[VertexProgram] = sp->VPFeatures.UsesCameraUBO;
	m_ProgramUsesObjectUBO[VertexProgram] = sp->VPFeatures.UsesObjectUBO;
	m_ProgramUsesMaterialUBO[VertexProgram] = sp->VPFeatures.UsesMaterialUBO;

	m_ProgramNoUniforms[PixelProgram] = false;
	m_ProgramNoBuiltinUniforms[PixelProgram] = false;
	m_ProgramOnlyUBOs[PixelProgram] = true;
	m_ProgramUsesLightTableUBO[PixelProgram] = sp->PPFeatures.UsesLightTableUBO;
	m_ProgramUsesCameraUBO[PixelProgram] = sp->PPFeatures.UsesCameraUBO;
	m_ProgramUsesObjectUBO[PixelProgram] = sp->PPFeatures.UsesObjectUBO;
	m_ProgramUsesMaterialUBO[PixelProgram] = sp->PPFeatures.UsesMaterialUBO;

	return true;
}

bool CDriverGL3::initProgramPipeline()
{
	ppoId = 0;

	if (!m_SupportSSO)
		return true;

	nglGenProgramPipelines(1, &ppoId);
	if (ppoId == 0)
		return false;

	_DriverGLStates.forceBindProgramPipeline(ppoId);

	return true;
}

// ***************************************************************************
// CProgramDrvInfosGL3
// ***************************************************************************

CProgramDrvInfosGL3::CProgramDrvInfosGL3(CDriverGL3 *drv, ItGPUPrgDrvInfoPtrList it) :
IProgramDrvInfos(drv, it)
{
	programId = 0;
	lightTableBlockIndex = GL_INVALID_INDEX;
	cameraBlockIndex = GL_INVALID_INDEX;
	objectBlockIndex = GL_INVALID_INDEX;
	materialBlockIndex = GL_INVALID_INDEX;
	isNelvpConverted = false;
	isInsertProgram = false;
}

CProgramDrvInfosGL3::~CProgramDrvInfosGL3()
{
	// FIXME GL3: Is this not released?!
	programId = 0;
}

uint CProgramDrvInfosGL3::getUniformIndex(const char *name) const
{
	if (isNelvpConverted)
	{
		// Check ParamIndices first (maps friendly names like "viewCenter" to register indices)
		std::map<std::string, uint>::const_iterator it = NelvpParamIndices.find(name);
		if (it != NelvpParamIndices.end())
			return it->second;

		// Fall back to "constantN" pattern for generic constant register access
		if (strncmp(name, "constant", 8) == 0)
		{
			const char *num = name + 8;
			if (*num >= '0' && *num <= '9')
			{
				int idx = atoi(num);
				if (idx >= 0 && idx < 96)
					return (uint)idx;
			}
		}
	}

	int idx = nglGetUniformLocation(programId, name);
	if (idx == -1)
		return ~0;
	else
		return idx;
}

} // NLDRIVERGL3
} // NL3D

/* end of file */
