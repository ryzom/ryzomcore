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
#include "nel/3d/vertex_program_parse.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

enum TArguments
{
#ifdef NL_LITTLE_ENDIAN
	ADD  = 1094992928,
	ARL  = 1095912480,
	BFC0 = 1111900976,
	BFC1 = 1111900977,
	COL0 = 1129270320,
	COL1 = 1129270321,
	DP3  = 1146106656,
	DP4  = 1146106912,
	DST  = 1146311712,
	END  = 1162757152,
	EXP  = 1163415584,
	EXPP = 1163415632,
	FOGC = 1179600707,
	HPOS = 1213222739,
	LIT  = 1279874080,
	LOG  = 1280263968,
	MAD  = 1296122912,
	MAX  = 1296128032,
	MIN  = 1296649760,
	MOV  = 1297045024,
	MUL  = 1297435680,
	NRML = 1314016588,
	OPOS = 1330663251,
	PSIZ = 1347635546,
	RCP  = 1380143136,
	RSQ  = 1381191968,
	SGE  = 1397179680,
	SLT  = 1397511200,
	TEX0 = 1413830704,
	TEX1 = 1413830705,
	TEX2 = 1413830706,
	TEX3 = 1413830707,
	TEX4 = 1413830708,
	TEX5 = 1413830709,
	TEX6 = 1413830710,
	TEX7 = 1413830711,
	WGHT = 1464289364,
#else
	ADD  = 541344833,
	ARL  = 541872705,
	BFC0 = 809715266,
	BFC1 = 826492482,
	COL0 = 810307395,
	COL1 = 827084611,
	DP3  = 540233796,
	DP4  = 540299332,
	DST  = 542397252,
	END  = 541347397,
	EXP  = 542136389,
	EXPP = 1347442757,
	FOGC = 1128746822,
	HPOS = 1397706824,
	LIT  = 542394700,
	LOG  = 541544268,
	MAD  = 541344077,
	MAX  = 542654797,
	MIN  = 542001485,
	MOV  = 542527309,
	MUL  = 541873485,
	NRML = 1280135758,
	OPOS = 1397706831,
	PSIZ = 1514754896,
	RCP  = 542131026,
	RSQ  = 542200658,
	SGE  = 541411155,
	SLT  = 542395475,
	TEX0 = 811091284,
	TEX1 = 827868500,
	TEX2 = 844645716,
	TEX3 = 861422932,
	TEX4 = 878200148,
	TEX5 = 894977364,
	TEX6 = 911754580,
	TEX7 = 928531796,
	WGHT = 1414022999,
#endif
};

//=====================================
bool CVPParser::parseWriteMask(uint &mask, std::string &errorOutput)
{
	// parse output mask
	if (*_CurrChar != '.')
	{
		// no output masks
		mask = 0xf; //output 4 coordinates
		return true;
	}
	else
	{
		++ _CurrChar;
		mask = 0;
		for(uint k = 0; k < 4; ++k)
		{
			uint maskIndex;
			switch(*_CurrChar)
			{
				case 'x': maskIndex = 0; break;
				case 'y': maskIndex = 1; break;
				case 'z': maskIndex = 2; break;
				case 'w': maskIndex = 3; break;
				default:
					if (k >= 1) return true;
					else
					{
						errorOutput = "Can't parse output mask.";
						return false;
					}
				break;
			}
			++_CurrChar;
			if (mask & (1 << maskIndex))
			{
				errorOutput = "Duplicated output mask component.";
				return false;
			}
			mask |= 1 << maskIndex;
		}
		return true;
	}
}

//=====================================
/** Skip tabulation and space in a source code
  */
void CVPParser::skipSpacesAndComments()
{
	bool stop = false;
	do
	{
		switch(*_CurrChar)
		{
			case '\t':
			case '\r':
			case ' ' :
				++_CurrChar;
			break;
			//
			case '\n':
				++_CurrChar;
				++_LineIndex;
				_LineStart = _CurrChar;
			break;
			case '#': // comment go till end of line
				while (*_CurrChar != '\n' && *_CurrChar != '\0') ++_CurrChar;
				skipSpacesAndComments();
			break;
			default:
				stop = true;
			break;
		}
	}
	while (!stop);
}

//=================================================================================================
uint CVPInstruction::getNumUsedSrc() const
{
	switch(Opcode)
	{
		case CVPInstruction::ARL:
		case CVPInstruction::RSQ:
		case CVPInstruction::EXPP:
		case CVPInstruction::LOG:
		case CVPInstruction::RCP:
		case CVPInstruction::MOV:
		case CVPInstruction::LIT:
			return 1;
		//
		case CVPInstruction::MAD:
			return 3;
		//
		case CVPInstruction::MUL:
		case CVPInstruction::ADD:
		case CVPInstruction::DP3:
		case CVPInstruction::DP4:
		case CVPInstruction::DST:
		case CVPInstruction::MIN:
		case CVPInstruction::MAX:
		case CVPInstruction::SLT:
		case CVPInstruction::SGE:
			return 2;
		//
		default:
			nlstop;
	}
	return 0;
}

//=================================================================================================
bool CVPParser::parseOperand(CVPOperand &operand, bool outputOperand, std::string &errorOutput)
{
	skipSpacesAndComments();
	bool result;
	if (outputOperand)
	{
		operand.Negate = false;
		switch(*_CurrChar)
		{
			case 'o': result = parseOutputRegister(operand, errorOutput); break;
			case 'R':
				result = parseVariableRegister(operand, errorOutput);
			break;
			case 'A': result = parseAddressRegister(operand, errorOutput); break;
			case '-':
				errorOutput = "Negation not allowed on output register.";
				return false;
			default:
				errorOutput = "Output, Address, or Temporary register expected as an output operand.";
				return false;
		}
		if (!result) return false;

		// parse the write mask
		return parseWriteMask(operand.WriteMask, errorOutput);
	}
	else
	{
		operand.Negate = false;
		switch(*_CurrChar)
		{
			case 'v': result = parseInputRegister(operand, errorOutput); break;
			case 'R': result = parseVariableRegister(operand, errorOutput); break;
			case 'c': result = parseConstantRegister(operand, errorOutput); break;
			case 'a': result = parseAddressRegister(operand, errorOutput); break;
			case '-':
			{
				operand.Negate = true;
				// negation
				++ _CurrChar;
				skipSpacesAndComments();
				switch(*_CurrChar)
				{
					case 'v': result = parseInputRegister(operand, errorOutput); break;
					case 'R': result = parseVariableRegister(operand, errorOutput); break;
					case 'c': result = parseConstantRegister(operand, errorOutput); break;
					default:
						errorOutput = "Negation must be followed by an input register, a variable register, or a constant.";
						return false;
					break;
				}
			}
			break;
			default:
				errorOutput = "Syntax error.";
				return false;
			break;
		}
		if (!result) return false;
		if (operand.Type != CVPOperand::AddressRegister)
		{
			if (!parseSwizzle(operand.Swizzle, errorOutput)) return false;
			if (operand.Type == CVPOperand::Variable)
			{
				for(uint k = 0; k < 4; ++k)
				{
					if (!(_RegisterMask[operand.Value.VariableValue] & (1 << operand.Swizzle.Comp[k])))
					{
						errorOutput = "Can't read a register component before writing to it.";
						return false;
					}
				}
			}
		}
		return true;
	}


}


//=================================================================================================
bool CVPParser::parseInputRegister(CVPOperand &operand, std::string &errorOutput)
{
	++_CurrChar;
	operand.Type = CVPOperand::InputRegister;
	if (*_CurrChar != '[')
	{
		errorOutput = "'[' expected when parsing an input register.";
		return false;
	}
	++_CurrChar;
	skipSpacesAndComments();
	if (isdigit(*_CurrChar))
	{
		// The input register is expressed as an index
		uint index = *_CurrChar - '0';
		++_CurrChar;
		if (isdigit(*_CurrChar))
		{
			index =  10 * index + (*_CurrChar - '0');
			++_CurrChar;
		}
		if (index > 15)
		{
			errorOutput = "Invalid index for input register, must be in [0, 15].";
			return false;
		}
		operand.Value.InputRegisterValue = (CVPOperand::EInputRegister) index;
	}
	else
	{
		// The input register is expressed as a string
		uint32 strValue = 0;
		// read the 4 letters
		for(uint k = 0; k < 4; ++k)
		{
			if (!isalnum(*_CurrChar))
			{
				errorOutput = "Can't parse index for input register.";
				return false;
			}
			strValue |= ((uint32) *_CurrChar) << (8 * (3 - k));
			++_CurrChar;
		}
		switch ((TArguments)strValue)
		{
			case OPOS: operand.Value.InputRegisterValue = CVPOperand::IPosition; break;
			case WGHT: operand.Value.InputRegisterValue = CVPOperand::IWeight; break;
			case NRML: operand.Value.InputRegisterValue = CVPOperand::INormal; break;
			case COL0: operand.Value.InputRegisterValue = CVPOperand::IPrimaryColor; break;
			case COL1: operand.Value.InputRegisterValue = CVPOperand::ISecondaryColor; break;
			case FOGC: operand.Value.InputRegisterValue = CVPOperand::IFogCoord; break;
			// texture argument
			case TEX0:
			case TEX1:
			case TEX2:
			case TEX3:
			case TEX4:
			case TEX5:
			case TEX6:
			case TEX7:
				operand.Value.InputRegisterValue = (CVPOperand::EInputRegister) (((CVPOperand::ITex0 + strValue) & 0xff) - '0');
			break;
			default:
				errorOutput = "Can't parse index for input register.";
				return false;
		}
	}
	skipSpacesAndComments();
	if (*_CurrChar != ']')
	{
		errorOutput = "']' expected when parsing an input register.";
		return false;
	}
	++ _CurrChar;
	return true;
}

//=================================================================================================
static inline bool letterToSwizzleComp(char letter, CVPSwizzle::EComp &comp)
{
	switch (letter)
	{
		case 'x': comp = CVPSwizzle::X; return true;
		case 'y': comp = CVPSwizzle::Y; return true;
		case 'z': comp = CVPSwizzle::Z; return true;
		case 'w': comp = CVPSwizzle::W; return true;
	}
	return false;
}

//=================================================================================================
bool CVPParser::parseSwizzle(CVPSwizzle &swizzle,std::string &errorOutput)
{
	if (*_CurrChar != '.')
	{
		// no swizzle
		swizzle.Comp[0] = CVPSwizzle::X;
		swizzle.Comp[1] = CVPSwizzle::Y;
		swizzle.Comp[2] = CVPSwizzle::Z;
		swizzle.Comp[3] = CVPSwizzle::W;
		return true;
	}
	++_CurrChar;
	// 4 letters case
	for(uint k = 0; k < 4; ++k)
	{
		if (!isalpha(*_CurrChar))
		{
			if (k == 1) // 1 letter case
			{
				switch(*_CurrChar)
				{
					case ',':
					case ';':
					case ' ':
					case '\t':
					case '\r':
					case '\n':
					case '#':
						swizzle.Comp[1] = swizzle.Comp[2] = swizzle.Comp[3] = swizzle.Comp[0];
						return true;
					break;
					default:
						errorOutput = "Can't parse swizzle.";

				}
			}
			else
			{
				errorOutput = "Invalid swizzle value.";
				return false;
			}
		}

		if (!letterToSwizzleComp(*_CurrChar, swizzle.Comp[k]))
		{
			errorOutput = "Invalid swizzle value.";
			return false;
		}
		++ _CurrChar;
	}

	return true;
}

//=================================================================================================
bool CVPParser::parseOutputRegister(CVPOperand &operand, std::string &errorOutput)
{
	++_CurrChar;
	operand.Type = CVPOperand::OutputRegister;
	if (*_CurrChar != '[')
	{
		errorOutput = "'[' expected when parsing an output register.";
		return false;
	}
	++_CurrChar;
	skipSpacesAndComments();
	// The input register is expressed as a string
	uint32 strValue = 0;
	// read the 4 letters
	for(uint k = 0; k < 4; ++k)
	{
		if (!isalnum(*_CurrChar))
		{
			errorOutput = "Can't parse index for output register.";
			return false;
		}
		strValue |= ((uint32) *_CurrChar) << (8 * (3 - k));
		++_CurrChar;
	}
	// convert to enum
	switch((TArguments)strValue)
	{
		case HPOS: operand.Value.OutputRegisterValue = CVPOperand::OHPosition; break;
		case COL0: operand.Value.OutputRegisterValue = CVPOperand::OPrimaryColor; break;
		case COL1: operand.Value.OutputRegisterValue = CVPOperand::OSecondaryColor; break;
		case BFC0: operand.Value.OutputRegisterValue = CVPOperand::OBackFacePrimaryColor; break;
		case BFC1: operand.Value.OutputRegisterValue = CVPOperand::OBackFaceSecondaryColor; break;
		case FOGC: operand.Value.OutputRegisterValue = CVPOperand::OFogCoord; break;
		case PSIZ: operand.Value.OutputRegisterValue = CVPOperand::OPointSize; break;
		case TEX0: operand.Value.OutputRegisterValue = CVPOperand::OTex0; break;
		case TEX1: operand.Value.OutputRegisterValue = CVPOperand::OTex1; break;
		case TEX2: operand.Value.OutputRegisterValue = CVPOperand::OTex2; break;
		case TEX3: operand.Value.OutputRegisterValue = CVPOperand::OTex3; break;
		case TEX4: operand.Value.OutputRegisterValue = CVPOperand::OTex4; break;
		case TEX5: operand.Value.OutputRegisterValue = CVPOperand::OTex5; break;
		case TEX6: operand.Value.OutputRegisterValue = CVPOperand::OTex6; break;
		case TEX7: operand.Value.OutputRegisterValue = CVPOperand::OTex7; break;
		default:
			errorOutput = "Can't read index for output register.";
			return false;
		break;
	}
	skipSpacesAndComments();
	if (*_CurrChar != ']')
	{
		errorOutput = "']' expected when parsing an output register.";
		return false;
	}
	++_CurrChar;
	return true;
}

//=================================================================================================
static inline const char *parseUInt(const char *src, uint &dest)
{
	uint index = 0;
	while (isdigit(*src))
	{
		index = 10 * index + *src - '0';
		++ src;
	}
	dest = index;
	return src;
}


//=================================================================================================
bool CVPParser::parseConstantRegister(CVPOperand &operand, std::string &errorOutput)
{
	++_CurrChar;
	operand.Type = CVPOperand::Constant;
	if (*_CurrChar != '[')
	{
		errorOutput = "'[' expected when parsing a constant register.";
		return false;
	}
	++_CurrChar;
	skipSpacesAndComments();
	sint &index = operand.Value.ConstantValue;
	if (isdigit(*_CurrChar))
	{
		// immediat case : c[0] to c[95]
		uint uIndex;
		_CurrChar = parseUInt(_CurrChar, uIndex);
		if (uIndex > 95)
		{
			errorOutput = "Constant register index must range from 0 to 95.";
			return false;
		}
		index = (sint) uIndex;
		operand.Indexed = false;
	}
	else if (*_CurrChar == 'A')
	{
		// indexed case : c[A0.x - 64] to c[A0.x + 63]
		operand.Indexed = true;
		index = 0;
		if (_CurrChar[1] == '0'
			&& _CurrChar[2] == '.'
			&& _CurrChar[3] == 'x')
		{
			_CurrChar += 4;
			skipSpacesAndComments();
			if (*_CurrChar == '+')
			{
				++ _CurrChar;
				skipSpacesAndComments();
				if (isdigit(*_CurrChar))
				{
					uint uIndex;
					_CurrChar = parseUInt(_CurrChar, uIndex);
					if (uIndex > 63)
					{
						errorOutput = "Constant register index must range from -64 to +63.";
						return false;
					}
					index = (sint) uIndex;
				}
				else
				{
					errorOutput = "Can't parse offset for constant register.";
					return false;
				}
			}
			else
			if (*_CurrChar == '-')
			{
				++ _CurrChar;
				skipSpacesAndComments();
				if (isdigit(*_CurrChar))
				{
					uint uIndex;
					_CurrChar = parseUInt(_CurrChar, uIndex);
					if (uIndex > 64)
					{
						errorOutput = "Constant register index must range from -64 to +63.";
						return false;
					}
					index = - (sint) uIndex;
				}
				else
				{
					errorOutput = "Can't parse offset for constant register.";
					return false;
				}
			}
		}
		else
		{
			errorOutput = "Can't parse constant register index.";
			return false;
		}
	}
	skipSpacesAndComments();
	if (*_CurrChar != ']')
	{
		errorOutput = "']' expected when parsing an input register.";
		return false;
	}
	++_CurrChar;
	return true;
}

//=================================================================================================
bool CVPParser::parseVariableRegister(CVPOperand &operand, std::string &errorOutput)
{
	++_CurrChar;
	operand.Type = CVPOperand::Variable;
	if (!isdigit(*_CurrChar))
	{
		errorOutput = "Can't parse variable register.";
		return false;
	}
	uint &index = operand.Value.VariableValue;
	_CurrChar = parseUInt(_CurrChar, index);
	if (index > 11)
	{
		errorOutput = "Variable register index must range from 0 to 11.";
		return false;
	}
	return true;
}

//=================================================================================================
bool CVPParser::parseAddressRegister(CVPOperand &operand, std::string &errorOutput)
{
	++_CurrChar;
	operand.Type = CVPOperand::AddressRegister;
	if (_CurrChar[0] != '0' || _CurrChar[1] != '.' || _CurrChar[2] != 'x')
	{
		errorOutput = "Can't parse address register.";
		return false;
	}
	_CurrChar += 3;
	return true;
}

//=================================================================================================
bool CVPParser::parseOp2(CVPInstruction &instr,std::string &errorOutput)
{
	skipSpacesAndComments();
	// parse output
	if (!parseOperand(instr.Dest, true, errorOutput)) return false;
	// Can't write in input or consant register
	if (instr.Dest.Type == CVPOperand::Constant || instr.Dest.Type == CVPOperand::InputRegister)
	{
		errorOutput = "Can't write to a constant or input register";
		return false;
	}
	//
	skipSpacesAndComments();
	if (*_CurrChar != ',')
	{
		errorOutput = "',' expected.";
		return false;
	}
	++_CurrChar;
	skipSpacesAndComments();
	// parse src1
	if (!parseOperand(instr.Src1, false, errorOutput)) return false;
	if (instr.Src1.Type == CVPOperand::AddressRegister
		|| instr.Src1.Type == CVPOperand::OutputRegister)
	{
		errorOutput = "Src1 must be constant, variable, or input register.";
		return false;
	}
	return true;
}

//=================================================================================================
bool CVPParser::parseOp3(CVPInstruction &instr, std::string &errorOutput)
{
	if (!parseOp2(instr, errorOutput)) return false;
	skipSpacesAndComments();
	if (*_CurrChar != ',')
	{
		errorOutput = "',' expected.";
		return false;
	}
	++_CurrChar;
	skipSpacesAndComments();
	// parse src2
	if (!parseOperand(instr.Src2, false, errorOutput)) return false;
	if (instr.Src2.Type == CVPOperand::AddressRegister
		|| instr.Src2.Type == CVPOperand::OutputRegister)
	{
		errorOutput = "Src2 must be constant, variable, or input register.";
		return false;
	}
	// make sure we do not have 2 =/= contant register as src (or in put register)

	// 2 constant registers ?
	if (instr.Src1.Type == CVPOperand::Constant
		&& instr.Src2.Type == CVPOperand::Constant)
	{
		// the index must be the same
		if (!
			(
				instr.Src1.Indexed == instr.Src2.Indexed
			 && instr.Src1.Value.ConstantValue == instr.Src2.Value.ConstantValue
			)
		   )
		{
			errorOutput = "Can't read 2 different constant registers in a single instruction.";
			return false;
		}
	}

	// 2 input registers ?
	if (instr.Src1.Type == CVPOperand::InputRegister
		&& instr.Src2.Type == CVPOperand::InputRegister)
	{
		// the index must be the same
		if (instr.Src1.Value.InputRegisterValue != instr.Src2.Value.InputRegisterValue)
		{
			errorOutput = "Can't read 2 different input registers in a single instruction.";
			return false;
		}
	}
	return true;
}

//=================================================================================================
bool CVPParser::parseOp4(CVPInstruction &instr, std::string &errorOutput)
{
	if (!parseOp3(instr, errorOutput)) return false;
	// parse src 3
	skipSpacesAndComments();
	if (*_CurrChar != ',')
	{
		errorOutput = "',' expected.";
		return false;
	}
	++_CurrChar;
	skipSpacesAndComments();
	// parse src4
	if (!parseOperand(instr.Src3, false, errorOutput)) return false;
	if (instr.Src3.Type == CVPOperand::AddressRegister
		|| instr.Src3.Type == CVPOperand::OutputRegister)
	{
		errorOutput = "Src3 must be constant, variable, or input register.";
		return false;
	}

	///////////////////////////////////////////////////
	// check for different contant / input registers //
	///////////////////////////////////////////////////

	// Duplicated constant register
	if (instr.Src3.Type == CVPOperand::Constant)
	{
		if (instr.Src1.Type == CVPOperand::Constant)
		{
			if (!
			    (
				    instr.Src1.Indexed == instr.Src3.Indexed
			     && instr.Src1.Value.ConstantValue == instr.Src3.Value.ConstantValue
			    )
		       )
			{
				errorOutput = "Can't read 2 different constant registers in a single instruction.";
				return false;
			}
		}
		if (instr.Src2.Type == CVPOperand::Constant)
		{
			if (!
			    (
				    instr.Src2.Indexed == instr.Src3.Indexed
			     && instr.Src2.Value.ConstantValue == instr.Src3.Value.ConstantValue
			    )
		       )
			{
				errorOutput = "Can't read 2 different constant registers in a single instruction.";
				return false;
			}
		}
	}

	// Duplicated input register
	if (instr.Src3.Type == CVPOperand::InputRegister)
	{
		if (instr.Src1.Type == CVPOperand::InputRegister)
		{
			if (instr.Src1.Value.InputRegisterValue != instr.Src3.Value.InputRegisterValue)
			{
				errorOutput = "Can't read 2 different input registers in a single instruction.";
				return false;
			}
		}
		if (instr.Src2.Type == CVPOperand::InputRegister)
		{
			if (instr.Src2.Value.InputRegisterValue != instr.Src3.Value.InputRegisterValue)
			{
				errorOutput = "Can't read 2 different input registers in a single instruction.";
				return false;
			}
		}
	}


	return true;
}

//=================================================================================================
bool CVPParser::parseInstruction(CVPInstruction &instr, std::string &errorOutput, bool &endEncountered)
{
	skipSpacesAndComments();
	endEncountered = false;
	uint32 instrStr = 0;
	uint k;
	for(k = 0; k < 4; ++k)
	{
		if (!isalnum(*_CurrChar))
		{
			if (k < 3) // at least 3 letter in an instruction
			{
				errorOutput = "Syntax error : can't read opcode.";
				return false;
			}
			else break;
		}
		instrStr |= ((uint) *_CurrChar) << (8 * (3 - k));
		++ _CurrChar;
	}
	if (k != 4)
	{
		instrStr |= (uint32) ' ';
	}
	switch ((TArguments)instrStr)
	{
		case ARL:
			instr.Opcode = CVPInstruction::ARL;
			if (!parseOp2(instr, errorOutput)) return false;
			if (!instr.Src1.Swizzle.isScalar())
			{
				errorOutput = "ARL need a scalar src value.";
				return false;
			}
		break;
		case RSQ:
			instr.Opcode = CVPInstruction::RSQ;
			if (!parseOp2(instr, errorOutput)) return false;
			if (!instr.Src1.Swizzle.isScalar())
			{
				errorOutput = "RSQ need a scalar src value.";
				return false;
			}
		break;
		case EXP:
		case EXPP:
			instr.Opcode = CVPInstruction::EXPP;
			if (!parseOp2(instr, errorOutput)) return false;
			if (!instr.Src1.Swizzle.isScalar())
			{
				errorOutput = "EXP need a scalar src value.";
				return false;
			}
			/*
			if (instr.Src1.Swizzle.Comp[0] != CVPSwizzle.W)
			{
				errorOutput = "EXPP input scalar must be w";
				return false;
			}*/
		break;
		case LOG:
			instr.Opcode = CVPInstruction::LOG;
			if (!parseOp2(instr, errorOutput)) return false;
			if (!instr.Src1.Swizzle.isScalar())
			{
				errorOutput = "LOG need a scalar src value.";
				return false;
			}
			/*
			if (instr.Src1.Swizzle.Comp[0] != CVPSwizzle.W)
			{
				errorOutput = "LOG input scalar must be w";
				return false;
			}
			*/
		break;
		case RCP:
			instr.Opcode = CVPInstruction::RCP;
			if (!parseOp2(instr, errorOutput)) return false;
			if (!instr.Src1.Swizzle.isScalar())
			{
				errorOutput = "RCP need a scalar src value.";
				return false;
			}
		break;
		/////////////////
		case MOV:
			instr.Opcode = CVPInstruction::MOV;
			if (!parseOp2(instr, errorOutput)) return false;

		break;
		case LIT:
			instr.Opcode = CVPInstruction::LIT;
			if (!parseOp2(instr, errorOutput)) return false;
		break;
		/////////////////
		case MAD:
			instr.Opcode = CVPInstruction::MAD;
			if (!parseOp4(instr, errorOutput)) return false;
		break;
		/////////////////
		case ADD:
			instr.Opcode = CVPInstruction::ADD;
			if (!parseOp3(instr, errorOutput)) return false;
		break;
		/////////////////
		case MUL:
			instr.Opcode = CVPInstruction::MUL;
			if (!parseOp3(instr, errorOutput)) return false;
		break;
		case DP3:
			instr.Opcode = CVPInstruction::DP3;
			if (!parseOp3(instr, errorOutput)) return false;
		break;
		case DP4:
			instr.Opcode = CVPInstruction::DP4;
			if (!parseOp3(instr, errorOutput)) return false;
		break;
		case DST:
			instr.Opcode = CVPInstruction::DST;
			if (!parseOp3(instr, errorOutput)) return false;
		break;
		case MIN:
			instr.Opcode = CVPInstruction::MIN;
			if (!parseOp3(instr, errorOutput)) return false;
		break;
		case MAX:
			instr.Opcode = CVPInstruction::MAX;
			if (!parseOp3(instr, errorOutput)) return false;
		break;
		case SLT:
			instr.Opcode = CVPInstruction::SLT;
			if (!parseOp3(instr, errorOutput)) return false;
		break;
		case SGE:
			instr.Opcode = CVPInstruction::SGE;
			if (!parseOp3(instr, errorOutput)) return false;
		break;
		/////////////////
		case END:
			endEncountered = true;
			return true;
		break;
		default:
			errorOutput = "Syntax error : unknow opcode.";
			return false;
		break;
	}


	if (instr.Dest.Type == CVPOperand::Variable)
	{
		_RegisterMask[instr.Dest.Value.VariableValue] |= instr.Dest.WriteMask;
	}

	// it is not allowed to write to an adress register except for ARL
	if (instrStr != NELID("ARL "))
	{
		if (instr.Dest.Type == CVPOperand::AddressRegister)
		{
			errorOutput = "Can't write to address register.";
			return false;
		}
	}

	// parse semi-colon
	skipSpacesAndComments();
	//
	if (*_CurrChar != ';')
	{
		errorOutput = "';' expected.";
		return false;
	}
	++_CurrChar;
	return true;
}


//=================================================================================================
bool CVPParser::isInputUsed(const TProgram &prg, CVPOperand::EInputRegister input)
{
	for(uint k = 0; k < prg.size(); ++k)
	{
		uint numSrc = prg[k].getNumUsedSrc();
		for(uint l = 0; l < numSrc; ++l)
		{
			const CVPOperand &src =  prg[k].getSrc(l);
			if (src.Type == CVPOperand::InputRegister && src.Value.InputRegisterValue == input) return true;
		}
	}
	return false;
}

//=================================================================================================
static std::string getStringUntilCR(const char *src)
{
	nlassert(src);
	std::string result;
	while (*src != '\n' && *src != '\r' && *src != '\0')
	{
		result += *src;
		++src;
	}
	return result;
}

//=================================================================================================
bool CVPParser::parse(const char *src, CVPParser::TProgram &result, std::string &errorOutput)
{
	if (!src) return false;
	//
	std::fill(_RegisterMask, _RegisterMask + 96, 0);

	//
	_CurrChar = src;
	_LineStart = src;
	_LineIndex = 1;
	//
	//skipSpacesAndComments(); // in fact space are not allowed at the start of the vertex program

	// parse version
	if (   _CurrChar[0] != '!'
		|| _CurrChar[1] != '!'
		|| _CurrChar[2] != 'V'
		|| _CurrChar[3] != 'P'
		|| _CurrChar[4] != '1'
		|| _CurrChar[5] != '.'
		|| (_CurrChar[6] != '0' && _CurrChar[6] != '1'))
	{
		errorOutput = "Can't parse version.";
		return false;
	}
	_CurrChar += 7;

	errorOutput.clear();
	// parse instructions
	bool endEncoutered = false;

	std::string errorMess;
	for(;;)
	{
		CVPInstruction instr;
		if (!parseInstruction(instr, errorMess, endEncoutered))
		{
			errorOutput = std::string("CVPParser::parse : Error encountered at line ") + NLMISC::toString(_LineIndex) + std::string(" : ") + errorMess + std::string(" Text : ") + getStringUntilCR(_LineStart);
			return false;
		}
		if (endEncoutered) break;
		result.push_back(instr);
	}
	return true;
}


//=================================================================================================
static const char *instrToName[] =
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
	"EXPP ",
	"LOG  ",
	"RCP  "
};

//=================================================================================================
static const char *outputRegisterToName[] =
{
	"HPOS",
	"COL0",
	"COL1",
	"BFC0",
	"BFC1",
	"FOGC",
	"PSIZ",
	"TEX0",
	"TEX1",
	"TEX2",
	"TEX3",
	"TEX4",
	"TEX5",
	"TEX6",
	"TEX7"
};


//=================================================================================================
static void dumpWriteMask(uint mask, std::string &out)
{
	if (mask == 0xf)
	{
		out.clear();
		return;
	}
	out = ".";
	if (mask & 1) out +="x";
	if (mask & 2) out +="y";
	if (mask & 4) out +="z";
	if (mask & 8) out +="w";
}

//=================================================================================================
static void dumpSwizzle(const CVPSwizzle &swz, std::string &out)
{
	if (swz.isIdentity())
	{
		out.clear();
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
static void dumpOperand(const CVPOperand &op, bool destOperand, std::string &out)
{
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
		case CVPOperand::InputRegister: out += "v[" + NLMISC::toString((uint) op.Value.InputRegisterValue) + "]"; break;
		case CVPOperand::OutputRegister:
			nlassert(op.Value.OutputRegisterValue < CVPOperand::OutputRegisterCount);
			out += "o[" + std::string(outputRegisterToName[op.Value.OutputRegisterValue]) + "]";
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
		dumpWriteMask(op.WriteMask, suffix);
	}
	else
	{
		dumpSwizzle(op.Swizzle, suffix);
	}
	out += suffix;
}

//=================================================================================================
/** Dump an instruction in a string
  */
static void dumpInstr(const CVPInstruction &instr, std::string &out)
{
	nlassert(instr.Opcode < CVPInstruction::OpcodeCount);
	out = instrToName[instr.Opcode];
	uint nbOp = instr.getNumUsedSrc();
	std::string destOperand;
	dumpOperand(instr.Dest, true, destOperand);
	out += destOperand;
	for(uint k = 0; k < nbOp; ++k)
	{
		out += ", ";
		std::string srcOperand;
		dumpOperand(instr.getSrc(k), false, srcOperand);
		out += srcOperand;
	}
	out +="; \n";
}

//=================================================================================================
void CVPParser::dump(const TProgram &prg, std::string &dest)
{
	dest = "!!VP1.0 \n";
	for(uint k = 0; k < prg.size(); ++k)
	{
		std::string instr;
		dumpInstr(prg[k], instr);
		dest += instr;
	}
	dest +="END";
}


