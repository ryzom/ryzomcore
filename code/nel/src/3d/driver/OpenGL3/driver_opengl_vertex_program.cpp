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

#include "stdopengl.h"

#include "driver_opengl.h"
#include "nel/3d/index_buffer.h"
#include "nel/3d/vertex_program.h"
#include "nel/3d/vertex_program_parse.h"
#include <algorithm>

// tmp
#include "nel/misc/file.h"

using namespace std;
using namespace NLMISC;

//#define DEBUG_SETUP_EXT_VERTEX_SHADER

namespace NL3D {

#ifdef NL_STATIC
namespace NLDRIVERGL3 {
#endif

// ***************************************************************************
CVertexProgamDrvInfosGL3::CVertexProgamDrvInfosGL3 (CDriverGL3 *drv, ItVtxPrgDrvInfoPtrList it) : IVertexProgramDrvInfos (drv, it)
{
	H_AUTO_OGL(CVertexProgamDrvInfosGL_CVertexProgamDrvInfosGL);

	nglGenProgramsARB(1, &ID);
}

//=================================================================================================
static const char *ARBVertexProgramInstrToName[] =
{
	"MOV  ",
	"ARL  ",
	"MUL  ",
	"ADD  ",
	"MAD  ",
	"RSQ  ",
	"DP3  ",
	"DP4  ",
	"DST  ",
	"LIT  ",
	"MIN  ",
	"MAX  ",
	"SLT  ",
	"SGE  ",
	"EXP  ",
	"LOG  ",
	"RCP  "
};

//=================================================================================================
static const char *ARBVertexProgramOutputRegisterToName[] =
{
	"position",
	"color.primary",
	"color.secondary",
	"color.back.primary",
	"color.back.secondary",
	"fogcoord",
	"pointsize",
	"texcoord[0]",
	"texcoord[1]",
	"texcoord[2]",
	"texcoord[3]",
	"texcoord[4]",
	"texcoord[5]",
	"texcoord[6]",
	"texcoord[7]"
};


//=================================================================================================
static void ARBVertexProgramDumpWriteMask(uint mask, std::string &out)
{
	H_AUTO_OGL(ARBVertexProgramDumpWriteMask)
	if (mask == 0xf)
	{
		out = "";
		return;
	}
	out = ".";
	if (mask & 1) out +="x";
	if (mask & 2) out +="y";
	if (mask & 4) out +="z";
	if (mask & 8) out +="w";
}

//=================================================================================================
static void ARBVertexProgramDumpSwizzle(const CVPSwizzle &swz, std::string &out)
{
	H_AUTO_OGL(ARBVertexProgramDumpSwizzle)
	if (swz.isIdentity())
	{
		out = "";
		return;
	}
	out = ".";
	for(uint k = 0; k < 4; ++k)
	{
		switch(swz.Comp[k])
		{
			case CVPSwizzle::X: out += "x"; break;
			case CVPSwizzle::Y: out += "y"; break;
			case CVPSwizzle::Z: out += "z"; break;
			case CVPSwizzle::W: out += "w"; break;
			default:
				nlassert(0);
			break;
		}
		if (swz.isScalar() && k == 0) break;
	}

}

//=================================================================================================
static void ARBVertexProgramDumpOperand(const CVPOperand &op, bool destOperand, std::string &out)
{
	H_AUTO_OGL(ARBVertexProgramDumpOperand)
	out = op.Negate ? " -" : " ";
	switch(op.Type)
	{
		case CVPOperand::Variable: out += "R" + NLMISC::toString(op.Value.VariableValue); break;
		case CVPOperand::Constant:
			out += "c[";
			if (op.Indexed)
			{
				out += "A0.x + ";
			}
			out += NLMISC::toString(op.Value.ConstantValue) + "]";
		break;
		case CVPOperand::InputRegister: out += "vertex.attrib[" + NLMISC::toString((uint) op.Value.InputRegisterValue) + "]"; break;
		case CVPOperand::OutputRegister:
			nlassert(op.Value.OutputRegisterValue < CVPOperand::OutputRegisterCount);
			out += "result." + std::string(ARBVertexProgramOutputRegisterToName[op.Value.OutputRegisterValue]);
		break;
		case CVPOperand::AddressRegister:
			out += "A0.x";
		break;
        default:
            break;
	}
	std::string suffix;
	if (destOperand)
	{
		ARBVertexProgramDumpWriteMask(op.WriteMask, suffix);
	}
	else
	{
		ARBVertexProgramDumpSwizzle(op.Swizzle, suffix);
	}
	out += suffix;
}

//=================================================================================================
/** Dump an instruction in a string
  */
static void ARBVertexProgramDumpInstr(const CVPInstruction &instr, std::string &out)
{
	H_AUTO_OGL(ARBVertexProgramDumpInstr)
	nlassert(instr.Opcode < CVPInstruction::OpcodeCount);
	// Special case for EXP with a scalar output argument (y component) -> translate to FRC
	out = ARBVertexProgramInstrToName[instr.Opcode];
	uint nbOp = instr.getNumUsedSrc();
	std::string destOperand;
	ARBVertexProgramDumpOperand(instr.Dest, true, destOperand);
	out += destOperand;
	for(uint k = 0; k < nbOp; ++k)
	{
		out += ", ";
		std::string srcOperand;
		ARBVertexProgramDumpOperand(instr.getSrc(k), false, srcOperand);
		out += srcOperand;
	}
	out +="; \n";

}

// ***************************************************************************
bool CDriverGL3::setupARBVertexProgram (const CVPParser::TProgram &inParsedProgram, GLuint id, bool &specularWritten)
{
	H_AUTO_OGL(CDriverGL3_setupARBVertexProgram);

	// tmp
	CVPParser::TProgram parsedProgram = inParsedProgram;
	//
	std::string code;
	code = "!!ARBvp1.0\n";
	// declare temporary registers
	code += "TEMP ";
	const uint NUM_TEMPORARIES = 12;
	for(uint k = 0; k < NUM_TEMPORARIES; ++k)
	{
		code += toString("R%d", (int) k);
		if (k != (NUM_TEMPORARIES - 1))
		{
			code +=", ";
		}
	}
	code += "; \n";
	// declare address register
	code += "ADDRESS A0;\n";
	// declare constant register
	code += "PARAM  c[96]  = {program.env[0..95]}; \n";
	uint writtenSpecularComponents = 0;
	for(uint k = 0; k < parsedProgram.size(); ++k)
	{
		if (parsedProgram[k].Dest.Type ==  CVPOperand::OutputRegister && parsedProgram[k].Dest.Value.OutputRegisterValue == CVPOperand::OSecondaryColor)
		{
			writtenSpecularComponents |= parsedProgram[k].Dest.WriteMask;
		}
	}
	// tmp fix : write unwritten components of specular (seems that glDisable(GL_COLOR_SUM_ARB) does not work in a rare case for me ...)
	if (writtenSpecularComponents != 0xf)
	{
		// add a new instruction to write 0 in unwritten components
		CVPSwizzle sw;
		sw.Comp[0] = CVPSwizzle::X;
		sw.Comp[1] = CVPSwizzle::Y;
		sw.Comp[2] = CVPSwizzle::Z;
		sw.Comp[3] = CVPSwizzle::W;

		CVPInstruction vpi;
		vpi.Opcode = CVPInstruction::ADD;
		vpi.Dest.WriteMask = 0xf ^ writtenSpecularComponents;
		vpi.Dest.Type = CVPOperand::OutputRegister;
		vpi.Dest.Value.OutputRegisterValue = CVPOperand::OSecondaryColor;
		vpi.Dest.Indexed = false;
		vpi.Dest.Negate = false;
		vpi.Dest.Swizzle = sw;
		vpi.Src1.Type = CVPOperand::InputRegister;
		vpi.Src1.Value.InputRegisterValue = CVPOperand::IPosition; // tmp -> check that position is present
		vpi.Src1.Indexed = false;
		vpi.Src1.Negate = false;
		vpi.Src1.Swizzle = sw;
		vpi.Src2.Type = CVPOperand::InputRegister;
		vpi.Src2.Value.InputRegisterValue = CVPOperand::IPosition; // tmp -> chec
		vpi.Src2.Indexed = false;
		vpi.Src2.Negate = true;
		vpi.Src2.Swizzle = sw;
		//
		parsedProgram.push_back(vpi);
	}
	specularWritten = (writtenSpecularComponents != 0);

	for(uint k = 0; k < parsedProgram.size(); ++k)
	{
		std::string instr;
		ARBVertexProgramDumpInstr(parsedProgram[k], instr);
		code += instr + "\r\n";
	}
	code += "END\n";
	//
	/*
	static COFile output;
	static bool opened = false;
	if (!opened)
	{
		output.open("vp.txt", false, true);
		opened = true;
	}
	std::string header = "=====================================================================================";
	output.serial(header);
	output.serial(code);
	*/
	//
	nglBindProgramARB( GL_VERTEX_PROGRAM_ARB, id);
	glGetError();
	nglProgramStringARB( GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, (GLsizei)code.size(), code.c_str() );
	GLenum err = glGetError();
	if (err != GL_NO_ERROR)
	{
		if (err == GL_INVALID_OPERATION)
		{
			GLint position;
			glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &position);
			nlassert(position != -1); // there was an error..
			nlassert(position < (GLint) code.size());
			uint line = 0;
			const char *lineStart = code.c_str();
			for(uint k = 0; k < (uint) position; ++k)
			{
				if (code[k] == '\n')
				{
					lineStart = code.c_str() + k;
					++line;
				}
			}
			nlwarning("ARB vertex program parse error at line %d.", (int) line);
			// search end of line
			const char *lineEnd = code.c_str() + code.size();
			for(uint k = position; k < code.size(); ++k)
			{
				if (code[k] == '\n')
				{
					lineEnd = code.c_str() + k;
					break;
				}
			}
			nlwarning(std::string(lineStart, lineEnd).c_str());
			// display the gl error msg
			const GLubyte *errorMsg = glGetString(GL_PROGRAM_ERROR_STRING_ARB);
			nlassert((const char *) errorMsg);
			nlwarning((const char *) errorMsg);
		}
		nlassert(0);
		return false;
	}

#ifdef NL_OS_MAC
	// Wait for GPU to finish program upload, else draw comands might crash.
	// Happened to CVegetableBlendLayerModel (glDrawElements()).
	// For more information, see http://dev.ryzom.com/issues/1006
	glFinish();
#endif

	return true;
}



// ***************************************************************************
bool CDriverGL3::activeARBVertexProgram (CVertexProgram *program)
{
	H_AUTO_OGL(CDriverGL3_activeARBVertexProgram);

	// Setup or unsetup ?
	if (program)
	{
		// Driver info
		CVertexProgamDrvInfosGL3 *drvInfo;

		// Program setuped ?
		if (program->_DrvInfo==NULL)
		{
			// try to parse the program
			CVPParser parser;
			CVPParser::TProgram parsedProgram;
			std::string errorOutput;
			bool result = parser.parse(program->getProgram().c_str(), parsedProgram, errorOutput);
			if (!result)
			{
				nlwarning("Unable to parse a vertex program.");
				#ifdef NL_DEBUG
					nlerror(errorOutput.c_str());
				#endif
				return false;
			}
			// Insert into driver list. (so it is deleted when driver is deleted).
			ItVtxPrgDrvInfoPtrList	it= _VtxPrgDrvInfos.insert(_VtxPrgDrvInfos.end(), (NL3D::IVertexProgramDrvInfos*)NULL);

			// Create a driver info
			*it = drvInfo = new CVertexProgamDrvInfosGL3 (this, it);
			// Set the pointer
			program->_DrvInfo=drvInfo;

			if (!setupARBVertexProgram(parsedProgram, drvInfo->ID, drvInfo->SpecularWritten))
			{
				delete drvInfo;
				program->_DrvInfo = NULL;
				_VtxPrgDrvInfos.erase(it);
				return false;
			}
		}
		else
		{
			// Cast the driver info pointer
			drvInfo=safe_cast<CVertexProgamDrvInfosGL3*>((IVertexProgramDrvInfos*)program->_DrvInfo);
		}
		glEnable( GL_VERTEX_PROGRAM_ARB );
		_VertexProgramEnabled = true;
		nglBindProgramARB( GL_VERTEX_PROGRAM_ARB, drvInfo->ID );
		if (drvInfo->SpecularWritten)
		{
			glEnable( GL_COLOR_SUM_ARB );
		}
		else
		{
			glDisable( GL_COLOR_SUM_ARB ); // no specular written
		}
		_LastSetuppedVP = program;
	}
	else
	{
		glDisable( GL_VERTEX_PROGRAM_ARB );
		glDisable( GL_COLOR_SUM_ARB );
		_VertexProgramEnabled = false;
	}
	return true;
}

// ***************************************************************************
bool CDriverGL3::activeVertexProgram (CVertexProgram *program)
{
	H_AUTO_OGL(CDriverGL3_activeVertexProgram)
	// Extension here ?
	return activeARBVertexProgram(program);
	
	// Can't do anything
	return false;
}


// ***************************************************************************

void CDriverGL3::setConstant (uint index, float f0, float f1, float f2, float f3)
{
	H_AUTO_OGL(CDriverGL3_setConstant);

	// Vertex program exist ?
	nglProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, index, f0, f1, f2, f3);

}


// ***************************************************************************

void CDriverGL3::setConstant (uint index, double d0, double d1, double d2, double d3)
{
	H_AUTO_OGL(CDriverGL3_setConstant);

	nglProgramEnvParameter4dARB(GL_VERTEX_PROGRAM_ARB, index, d0, d1, d2, d3);
}


// ***************************************************************************

void CDriverGL3::setConstant (uint index, const NLMISC::CVector& value)
{
	H_AUTO_OGL(CDriverGL3_setConstant);

	nglProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, index, value.x, value.y, value.z, 0);
}


// ***************************************************************************

void CDriverGL3::setConstant (uint index, const NLMISC::CVectorD& value)
{
	H_AUTO_OGL(CDriverGL3_setConstant);

	nglProgramEnvParameter4dARB(GL_VERTEX_PROGRAM_ARB, index, value.x, value.y, value.z, 0);
}


// ***************************************************************************
void	CDriverGL3::setConstant (uint index, uint num, const float *src)
{
	H_AUTO_OGL(CDriverGL3_setConstant);

	for(uint k = 0; k < num; ++k)
	{
		nglProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, index + k, src + 4 * k);
	}

}

// ***************************************************************************
void	CDriverGL3::setConstant (uint index, uint num, const double *src)
{
	H_AUTO_OGL(CDriverGL3_setConstant);

	for(uint k = 0; k < num; ++k)
	{
		nglProgramEnvParameter4dvARB(GL_VERTEX_PROGRAM_ARB, index + k, src + 4 * k);
	}
}

// ***************************************************************************

const uint CDriverGL3::GLMatrix[IDriver::NumMatrix]=
{
	GL_MODELVIEW,
	GL_PROJECTION,
	GL_MODELVIEW_PROJECTION_NV
};


// ***************************************************************************

const uint CDriverGL3::GLTransform[IDriver::NumTransform]=
{
	GL_IDENTITY_NV,
	GL_INVERSE_NV,
	GL_TRANSPOSE_NV,
	GL_INVERSE_TRANSPOSE_NV
};


// ***************************************************************************

void CDriverGL3::setConstantMatrix (uint index, IDriver::TMatrix matrix, IDriver::TTransform transform)
{
	H_AUTO_OGL(CDriverGL3_setConstantMatrix);

	{
		// First, ensure that the render setup is correctly setuped.
		refreshRenderSetup();
		CMatrix mat;
		switch (matrix)
		{
			case IDriver::ModelView:
				mat = _ModelViewMatrix;
			break;
			case IDriver::Projection:
				{
					mat = _GLProjMat;
				}
			break;
			case IDriver::ModelViewProjection:
				mat = _GLProjMat * _ModelViewMatrix;
			break;
            default:
                break;
		}

		switch(transform)
		{
			case IDriver::Identity: break;
			case IDriver::Inverse:
				mat.invert();
			break;
			case IDriver::Transpose:
				mat.transpose();
			break;
			case IDriver::InverseTranspose:
				mat.invert();
				mat.transpose();
			break;
            default:
                break;
		}
		mat.transpose();
		float matDatas[16];
		mat.get(matDatas);

		nglProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, index, matDatas);
		nglProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, index + 1, matDatas + 4);
		nglProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, index + 2, matDatas + 8);
		nglProgramEnvParameter4fvARB(GL_VERTEX_PROGRAM_ARB, index + 3, matDatas + 12);

	}
}

// ***************************************************************************

void CDriverGL3::setConstantFog (uint index)
{
	H_AUTO_OGL(CDriverGL3_setConstantFog)
	const float *values = _ModelViewMatrix.get();
	setConstant (index, -values[2], -values[6], -values[10], -values[14]);
}

// ***************************************************************************

void CDriverGL3::enableVertexProgramDoubleSidedColor(bool doubleSided)
{
	H_AUTO_OGL(CDriverGL3_enableVertexProgramDoubleSidedColor);

	// change mode (not cached because supposed to be rare)
	if(doubleSided)
		glEnable (GL_VERTEX_PROGRAM_TWO_SIDE_ARB);
	else
		glDisable (GL_VERTEX_PROGRAM_TWO_SIDE_ARB);
}


#ifdef NL_STATIC
} // NLDRIVERGL/ES
#endif

} // NL3D
