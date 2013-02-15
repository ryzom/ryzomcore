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



#ifndef NL_VERTEX_PROGRAM_PARSE_H
#define NL_VERTEX_PROGRAM_PARSE_H

#include <vector>


/// Swizzle of an operand in a vertex program
struct CVPSwizzle
{
	enum EComp { X = 0, Y = 1, Z = 2, W = 3};
	EComp	Comp[4];
	// Test if all values are the same
	bool isScalar() const
	{
		return    Comp[0] == Comp[1]
			   && Comp[0] == Comp[2]
			   && Comp[0] == Comp[3];
	}
	// Test if no swizzle is applied
	bool isIdentity() const
	{
		return Comp[0] == X
			   && Comp[1] == Y
			   && Comp[2] == Z
			   && Comp[3] == W;
	}
};



/** An operand in a vertex program
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2002
  */
struct CVPOperand
{
	// type of operand
	enum EOperandType
	{
		Variable = 0, // (R[0], R[1] ..)
		Constant, // (
		InputRegister,
		OutputRegister,
		AddressRegister, // for now, only means a0.x, no additionnal info is needed
		OperandTypeCount
	};
	// input registers
	enum EInputRegister
	{
		IPosition = 0,
		IWeight,
		INormal,
		IPrimaryColor,
		ISecondaryColor,
		IFogCoord,
		IPaletteSkin,
		IEmpty,
		ITex0,
		ITex1,
		ITex2,
		ITex3,
		ITex4,
		ITex5,
		ITex6,
		ITex7,
		InputRegisterCount
	};
	// output registers
	enum EOutputRegister
	{
		OHPosition = 0,
		OPrimaryColor,
		OSecondaryColor,
		OBackFacePrimaryColor,   // warning : backface colors are not supported on all implementations
		OBackFaceSecondaryColor,
		OFogCoord,
		OPointSize,
		OTex0,
		OTex1,
		OTex2,
		OTex3,
		OTex4,
		OTex5,
		OTex6,
		OTex7,
		OutputRegisterCount
	};

	EOperandType Type;

	// Datas for the various types.
	union
	{
		EOutputRegister OutputRegisterValue;
		EInputRegister  InputRegisterValue;
		uint		    VariableValue; // Index from 0 to 11
		sint            ConstantValue; // Index from 0 to 95, or -64 to +63 for constants with displacement
	} Value;

	bool Indexed; // true if it is a constant value, and if it is indexed

	// write mask (for output operands)
	uint WriteMask; // b0 -> X, b1 -> Y, b2 -> Z, b3 -> W

	// swizzle & negate
	bool		Negate;
	CVPSwizzle	Swizzle;
};

/// An instruction in a vertex program with its operands
struct CVPInstruction
{
	enum EOpcode
	{
		MOV = 0,
		ARL, // in D3D, is equivalent to MOV
		MUL,
		ADD,
		MAD,
		RSQ,
		DP3,
		DP4,
		DST,
		LIT,
		MIN,
		MAX,
		SLT,
		SGE,
		EXPP,
		LOG,
		RCP,
		OpcodeCount
	};
	EOpcode		Opcode;
	CVPOperand  Dest;
	CVPOperand  Src1;
	CVPOperand  Src2; // if used
	CVPOperand  Src3; // if used

	const CVPOperand &getSrc(uint index) const
	{
		nlassert(index < getNumUsedSrc());
		switch(index)
		{
			case 0: return Src1;
			case 1: return Src2;
			case 2: return Src3;
			default: nlstop;
		}
		return Src1; // avoid warning
	}

	// Get the number of source used depending on the opcode. Might be 1, 2, or 3
	uint		getNumUsedSrc() const;
};

/** A vertex program parser.
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2002
  */
class CVPParser
{
public:
	typedef std::vector<CVPInstruction> TProgram;
public:
	/** Parse a vertex program, and convert to proprietary format.
	  * It is intended to be used by a driver implementation.
	  * \warning: Only syntax is checked. It doesn't check that a register has been initialised before use.
	  * \param src The input text of a vertex program, in OpenGL format.
	  * \param result The result program.
	  * \param errorOutput If parsing failed, contains the reason
	  * \result true if the parsing succeeded
	  */
	bool parse(const char *src, TProgram &result, std::string &errorOutput);

	/** Debugging purpose : This output a parsed vertex program in a string, with the standard format.
	  * This can serve as a base for other format code generation
	  */
	static void dump(const TProgram &prg, std::string &dest);

	// test if a specific input is used by a vertex program
	static bool isInputUsed(const TProgram &prg, CVPOperand::EInputRegister input);
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
private:
	const char *_CurrChar;
	const char *_LineStart;
	uint		_LineIndex;
	uint		_RegisterMask[96]; // which components of registers have been written
private:
	bool parseOperand(CVPOperand &operand, bool outputOperand, std::string &errorOutput);
	//
	bool parseInputRegister(CVPOperand &operand, std::string &errorOutput);
	bool parseOutputRegister(CVPOperand &operand, std::string &errorOutput);
	bool parseConstantRegister(CVPOperand &operand, std::string &errorOutput);
	bool parseVariableRegister(CVPOperand &operand, std::string &errorOutput);
	bool parseAddressRegister(CVPOperand &operand, std::string &errorOutput);
	//
	bool parseSwizzle(CVPSwizzle &swizzle, std::string &errorOutput);
	bool parseWriteMask(uint &mask, std::string &errorOutput);
	//
	bool parseInstruction(CVPInstruction &instr, std::string &errorOutput, bool &endEncountered);
	// parse instruction in the form dest, src1, src2
	bool parseOp2(CVPInstruction &instr, std::string &errorOutput);
	bool parseOp3(CVPInstruction &instr, std::string &errorOutput);
	bool parseOp4(CVPInstruction &instr, std::string &errorOutput);
	// skip spaces and count lines
	void skipSpacesAndComments();
};




#endif

