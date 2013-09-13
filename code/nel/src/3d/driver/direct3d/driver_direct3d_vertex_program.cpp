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

#include "stddirect3d.h"

#include "driver_direct3d.h"

using namespace std;
using namespace NLMISC;

namespace NL3D
{

// ***************************************************************************

CVertexProgamDrvInfosD3D::CVertexProgamDrvInfosD3D(IDriver *drv, ItGPUPrgDrvInfoPtrList it) : IProgramDrvInfos (drv, it)
{
	H_AUTO_D3D(CVertexProgamDrvInfosD3D_CVertexProgamDrvInfosD3D)
	Shader = NULL;
}

// ***************************************************************************

CVertexProgamDrvInfosD3D::~CVertexProgamDrvInfosD3D()
{
	H_AUTO_D3D(CVertexProgamDrvInfosD3D_CVertexProgamDrvInfosD3DDtor)
	if (Shader)
		Shader->Release();
}

// ***************************************************************************

bool CDriverD3D::supportVertexProgram (CVertexProgram::TProfile profile) const
{
	H_AUTO_D3D(CDriverD3D_supportVertexProgram )
	return (profile == CVertexProgram::nelvp) && _VertexProgram;
}

// ***************************************************************************

bool CDriverD3D::isVertexProgramEmulated () const
{
	H_AUTO_D3D(CDriverD3D_isVertexProgramEmulated )
	// Pure HAL driver, no emulation available
	return false;
}

// ***************************************************************************

static const char *instrToName[] =
{
	"mov  ",
	"mov  ",
	"mul  ",
	"add  ",
	"mad  ",
	"rsq  ",
	"dp3  ",
	"dp4  ",
	"dst  ",
	"lit  ",
	"min  ",
	"max  ",
	"slt  ",
	"sge  ",
	"expp ",
	"log  ",
	"rcp  "
};

// ***************************************************************************

static const char *outputRegisterToName[] =
{
	"Pos",
	"D0",
	"D1",
	"BFC0",
	"BFC1",
	"Fog",
	"Pts",
	"T0",
	"T1",
	"T2",
	"T3",
	"T4",
	"T5",
	"T6",
	"T7"
};

// ***************************************************************************

void dumpWriteMask(uint mask, std::string &out)
{
	H_AUTO_D3D(dumpWriteMask)
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

// ***************************************************************************

void dumpSwizzle(const CVPSwizzle &swz, std::string &out)
{
	H_AUTO_D3D(dumpSwizzle)
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

// ***************************************************************************

void dumpOperand(const CVPOperand &op, bool destOperand, std::string &out, set<uint> &inputs)
{
	H_AUTO_D3D(dumpOperand)
	out = op.Negate ? " -" : " ";
	switch(op.Type)
	{
		case CVPOperand::Variable: out += "r" + NLMISC::toString(op.Value.VariableValue); break;
		case CVPOperand::Constant:
			out += "c[";
			if (op.Indexed)
			{
				out += "a0.x + ";
			}
			out += NLMISC::toString(op.Value.ConstantValue) + "]";
		break;
		case CVPOperand::InputRegister:
			out += "v" + NLMISC::toString((uint) op.Value.InputRegisterValue);
			inputs.insert (op.Value.InputRegisterValue);
		break;
		case CVPOperand::OutputRegister:
			nlassert(op.Value.OutputRegisterValue < CVPOperand::OutputRegisterCount);
			out += "o" + std::string(outputRegisterToName[op.Value.OutputRegisterValue]);
		break;
		case CVPOperand::AddressRegister:
			out += "a0.x";
		break;
	}
	std::string suffix;
	if (destOperand)
	{
		// No mask for the fog value
		if (op.Value.OutputRegisterValue != CVPOperand::OFogCoord)
			dumpWriteMask(op.WriteMask, suffix);
	}
	else
	{
		dumpSwizzle(op.Swizzle, suffix);
	}
	out += suffix;
}

// ***************************************************************************

void dumpInstr(const CVPInstruction &instr, std::string &out, set<uint> &inputs)
{
	H_AUTO_D3D(dumpInstr)
	nlassert(instr.Opcode < CVPInstruction::OpcodeCount);
	out = instrToName[instr.Opcode];
	uint nbOp = instr.getNumUsedSrc();
	std::string destOperand;
	dumpOperand(instr.Dest, true, destOperand, inputs);
	out += destOperand;
	for(uint k = 0; k < nbOp; ++k)
	{
		out += ", ";
		std::string srcOperand;
		dumpOperand(instr.getSrc(k), false, srcOperand, inputs);
		out += srcOperand;
	}
	out +="; \n";
}

// ***************************************************************************

static const char *inputToDecl[CVPOperand::InputRegisterCount] =
{
	"dcl_position v0",
	"dcl_blendweight v1",
	"dcl_normal v2",
	"dcl_color0 v3",
	"dcl_color1 v4",
	"dcl_fog v5",
	"dcl_blendindices v6",
	"",
	"dcl_texcoord0 v8",
	"dcl_texcoord1 v9",
	"dcl_texcoord2 v10",
	"dcl_texcoord3 v11",
	"dcl_texcoord4 v12",
	"dcl_texcoord5 v13",
	"dcl_texcoord6 v14",
	"dcl_texcoord7 v15",
};

// ***************************************************************************

void dump(const CVPParser::TProgram &prg, std::string &dest)
{
	H_AUTO_D3D(dump)
	// Set of input registers used
	set<uint> inputs;

	string program;
	for(uint k = 0; k < prg.size(); ++k)
	{
		std::string instr;
		dumpInstr(prg[k], instr, inputs);
		program += instr;
	}

	// Write the header
	dest = "vs.1.1\n";
	set<uint>::iterator ite = inputs.begin();
	while (ite != inputs.end())
	{
		dest += inputToDecl[*ite] + string("\n");
		ite++;
	}
	dest += program;
}

// ***************************************************************************

bool CDriverD3D::compileVertexProgram(NL3D::CVertexProgram *program)
{
	// Program setuped ?
	if (program->m_DrvInfo == NULL)
	{
		// Find nelvp
		IProgram::CSource *source = NULL;
		for (uint i = 0; i < program->getSourceNb(); ++i)
		{
			if (program->getSource(i)->Profile == CVertexProgram::nelvp)
			{
				source = program->getSource(i);
			}
		}
		if (!source)
		{
			nlwarning("Direct3D driver only supports 'nelvp' profile, vertex program cannot be used");
			return false;
		}

		_GPUPrgDrvInfos.push_front (NULL);
		ItGPUPrgDrvInfoPtrList itTex = _GPUPrgDrvInfos.begin();
		CVertexProgamDrvInfosD3D *drvInfo;
		*itTex = drvInfo = new CVertexProgamDrvInfosD3D(this, itTex);

		// Create a driver info structure
		program->m_DrvInfo = *itTex;

		/** Check with our parser if the program will works with other implemented extensions, too. (EXT_vertex_shader ..).
		  * There are some incompatibilities.
		  */
		CVPParser parser;
		CVPParser::TProgram parsedProgram;
		std::string errorOutput;
		bool result = parser.parse(source->SourcePtr, parsedProgram, errorOutput);
		if (!result)
		{
			nlwarning("Unable to parse a vertex program :");
			nlwarning(errorOutput.c_str());
			#ifdef NL_DEBUG_D3D
				nlassert(0);
			#endif // NL_DEBUG_D3D
			return false;
		}

		// tmp fix for Radeon 8500/9000/9200
		// Currently they hang when PaletteSkin / SkinWeight are present in the vertex declaration, but not used
		// so disable them in the vertex declaration
		// We don't use these component in vertex programs currently..
		#ifdef NL_DEBUG
			for(uint k = 0; k < parsedProgram.size(); ++k)
			{
				for(uint l = 0; l < parsedProgram[k].getNumUsedSrc(); ++l)
				{
					const CVPOperand &op = parsedProgram[k].getSrc(l);
					if (op.Type == CVPOperand::InputRegister)
					{
						nlassert(op.Value.InputRegisterValue != CVPOperand::IWeight);
						nlassert(op.Value.InputRegisterValue != CVPOperand::IPaletteSkin);
					}
				}
			}
		#endif

		// Dump the vertex program
		std::string dest;
		dump(parsedProgram, dest);
#ifdef NL_DEBUG_D3D
		nlinfo("Assemble Vertex Shader : ");
		string::size_type lineBegin = 0;
		string::size_type lineEnd;
		while ((lineEnd = dest.find('\n', lineBegin)) != string::npos)
		{
			nlinfo(dest.substr (lineBegin, lineEnd-lineBegin).c_str());
			lineBegin = lineEnd+1;
		}
		nlinfo(dest.substr (lineBegin, lineEnd-lineBegin).c_str());
#endif // NL_DEBUG_D3D

		LPD3DXBUFFER pShader;
		LPD3DXBUFFER pErrorMsgs;
		if (D3DXAssembleShader (dest.c_str(), (UINT)dest.size(), NULL, NULL, 0, &pShader, &pErrorMsgs) == D3D_OK)
		{
			if (_DeviceInterface->CreateVertexShader((DWORD*)pShader->GetBufferPointer(), &(getVertexProgramD3D(*program)->Shader)) != D3D_OK)
				return false;
		}
		else
		{
			nlwarning ("Can't assemble vertex program:");
			nlwarning ((const char*)pErrorMsgs->GetBufferPointer());
			return false;
		}

		// Set parameters for assembly programs
		drvInfo->ParamIndices = source->ParamIndices;

		// Build the feature info
		program->buildInfo(source);
	}

	return true;
}

// ***************************************************************************

bool CDriverD3D::activeVertexProgram (CVertexProgram *program)
{
	H_AUTO_D3D(CDriverD3D_activeVertexProgram )
	if (_DisableHardwareVertexProgram)
		return false;

	// Set the vertex program
	if (program)
	{
		if (!CDriverD3D::compileVertexProgram(program)) return false;

		CVertexProgamDrvInfosD3D *info = NLMISC::safe_cast<CVertexProgamDrvInfosD3D *>((IProgramDrvInfos*)program->m_DrvInfo);
		_VertexProgramUser = program;
		setVertexProgram (info->Shader, program);

		/* D3DRS_FOGSTART and D3DRS_FOGEND must be set with [1, 0] else the fog doesn't work properly on VertexShader and non-VertexShader objects
		(random fog flicking) with Geforce4 TI 4200 (drivers 53.03 and 45.23). The other cards seam to interpret the "oFog"'s values using D3DRS_FOGSTART,
		D3DRS_FOGEND.
		Related to setUniformFog().
		 */
		float z = 0;
		float o = 1;
		setRenderState (D3DRS_FOGSTART, *((DWORD*) (&o)));
		setRenderState (D3DRS_FOGEND, *((DWORD*) (&z)));
	}
	else
	{
		setVertexProgram (NULL, NULL);
		_VertexProgramUser = NULL;

		// Set the old fog range
		setRenderState (D3DRS_FOGSTART, *((DWORD*) (&_FogStart)));
		setRenderState (D3DRS_FOGEND, *((DWORD*) (&_FogEnd)));
	}

	return true;
}

// ***************************************************************************

void CDriverD3D::enableVertexProgramDoubleSidedColor(bool /* doubleSided */)
{
	H_AUTO_D3D(CDriverD3D_enableVertexProgramDoubleSidedColor)
}

// ***************************************************************************

bool CDriverD3D::supportVertexProgramDoubleSidedColor() const
{
	H_AUTO_D3D(CDriverD3D_supportVertexProgramDoubleSidedColor)
	// Not supported under D3D
	return false;
}

// ***************************************************************************

void CDriverD3D::disableHardwareVertexProgram()
{
	H_AUTO_D3D(CDriverD3D_disableHardwareVertexProgram)
	_DisableHardwareVertexProgram = true;
	_VertexProgram = false;
}

// ***************************************************************************

} // NL3D
