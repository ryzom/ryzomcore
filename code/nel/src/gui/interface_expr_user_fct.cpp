// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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


#include "stdpch.h"
#include "nel/gui/interface_expr.h"
#include "nel/gui/interface_link.h"
#include "nel/gui/interface_element.h"
#include "nel/gui/db_manager.h"
#include "nel/misc/i18n.h"

using namespace std;
using namespace NLMISC;

namespace NLGUI
{

	void ifexprufct_forcelink()
	{
	}


	// alias to CInterfaceExprValue
	typedef CInterfaceExprValue CIEV;

	/** takes arguments of a binary operator, and promote them to the best type.
	  * string are not supported
	  * \return true
	  */
	bool promoteToNumericalBestType(CInterfaceExpr::TArgList &_list)
	{
		uint i;
		bool bIsNumerical = true;

		for (i = 0 ; i < _list.size(); ++i)
			if (!_list[i].isNumerical())
				bIsNumerical = false;

		if (bIsNumerical)
		{
			bool bDouble = false;

			for (i = 0 ; i < _list.size(); ++i)
				if (_list[i].getType() == CIEV::Double)
					bDouble = true;

			if (bDouble)
			{
				for (i = 0 ; i < _list.size(); ++i)
					_list[i].toDouble();
			}
			else
			{
				for (i = 0 ; i < _list.size(); ++i)
					_list[i].toInteger();
			}
			return true;
		}

		return false;
	}


	/////////////////////
	//  ADD 2 numbers  //
	/////////////////////

	static DECLARE_INTERFACE_USER_FCT(userFctAdd)
	{
		if (!promoteToNumericalBestType(args))
		{
			nlwarning("add : invalid entry");
			return false;
		}
		switch(args[0].getType())
		{
			case CIEV::Integer: result.setInteger(args[0].getInteger() + args[1].getInteger()); return true;
			case CIEV::Double: result.setDouble(args[0].getDouble() + args[1].getDouble()); return true;
			default: break;
		}
		return false;
	}
	REGISTER_INTERFACE_USER_FCT("add", userFctAdd)

	/////////////////////
	//   SUB 2 numbers //
	/////////////////////

	static DECLARE_INTERFACE_USER_FCT(userFctSub)
	{
		if (!promoteToNumericalBestType(args))
		{
			nlwarning("sub : invalid entry");
			return false;
		}
		switch(args[0].getType())
		{
			case CIEV::Integer: result.setInteger(args[0].getInteger() - args[1].getInteger()); return true;
			case CIEV::Double: result.setDouble(args[0].getDouble() - args[1].getDouble()); return true;
			default: break;
		}
		return false;
	}
	REGISTER_INTERFACE_USER_FCT("sub", userFctSub)

	///////////////////
	// MUL 2 numbers //
	///////////////////

	static DECLARE_INTERFACE_USER_FCT(userFctMul)
	{
		if (!promoteToNumericalBestType(args))
		{
			nlwarning("mul : invalid entry");
			return false;
		}
		switch(args[0].getType())
		{
			case CIEV::Integer: result.setInteger(args[0].getInteger() * args[1].getInteger()); return true;
			case CIEV::Double: result.setDouble(args[0].getDouble() * args[1].getDouble()); return true;
			default: break;
		}
		return false;
	}
	REGISTER_INTERFACE_USER_FCT("mul", userFctMul)

	///////////////////
	// DIV 2 numbers //
	///////////////////

	static DECLARE_INTERFACE_USER_FCT(userFctDiv)
	{
		if (args.size() != 2 || !args[0].isNumerical() || !args[1].isNumerical())
		{
			nlwarning("div: bad arguments");
			return false;
		}
		args[0].toDouble();
		args[1].toDouble();
		if (args[1].getDouble() == 0)
		{
			nlwarning("div: zero divide");
			return false;
		}
		result.setDouble(args[0].getDouble() / args[1].getDouble());
		return true;
	}
	REGISTER_INTERFACE_USER_FCT("div", userFctDiv)

	///////////////////
	// MOD 2 numbers //
	///////////////////

	static DECLARE_INTERFACE_USER_FCT(userFctMod)
	{
		if (args.size() != 2 || !args[0].isNumerical() || !args[1].isNumerical())
		{
			nlwarning("mod: bad arguments");
			return false;
		}
		args[0].toDouble();
		args[1].toDouble();
		if (args[1].getDouble() == 0)
		{
			nlwarning("mod: zero divide");
			return false;
		}
		result.setDouble(fmod(args[0].getDouble(), args[1].getDouble()));
		return true;
	}
	REGISTER_INTERFACE_USER_FCT("mod", userFctMod)

	///////////////////
	// abs 1 number  //
	///////////////////

	static DECLARE_INTERFACE_USER_FCT(userFctAbs)
	{
		if (args.size() != 1 || !args[0].isNumerical())
		{
			nlwarning("abs: bad arguments");
			return false;
		}
		args[0].toDouble();
		result.setDouble(fabs(args[0].getDouble()));
		return true;
	}
	REGISTER_INTERFACE_USER_FCT("abs", userFctAbs)

	////////////////////////////////////
	// Identity  (copy its first arg) //
	////////////////////////////////////

	static DECLARE_INTERFACE_USER_FCT(userFctIdentity)
	{
		if (args.size() > 0)
		{
			result = args[0];
			return true;
		}
		else
		{
			return false;
		}
	}
	REGISTER_INTERFACE_USER_FCT("identity", userFctIdentity)


	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// Evaluate all of the args, and return 0, so may be used to create dependances on database entries //
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	static DECLARE_INTERFACE_USER_FCT(userFctDepends)
	{
		result.setInteger(0);
		return true;
	}
	REGISTER_INTERFACE_USER_FCT("depends", userFctDepends)


	//////////////////////////
	// comparison operators //
	//////////////////////////

	/** ugly macro to declare comparison operator in a 'compact' way
	  * this can compare number vs number & string vs string
	  */
	#define CREATE_CMP_OPERATOR(op, name)                                            \
	static DECLARE_INTERFACE_USER_FCT(userFct##name)                                 \
	{                                                                                \
																					 \
		if (args.size() != 2)                                                        \
		{                                                                            \
			nlwarning("comparison : bad number of arguments");                       \
			return false;                                                            \
		}                                                                            \
		if (args[0].getType() == CIEV::String && args[1].getType() == CIEV::String)  \
		{																			 \
			result.setBool(strcmp(args[0].getString().c_str(),                       \
						   args[1].getString().c_str()) op 0);                       \
			return true;	                                                         \
		}																			 \
		if (args.size() != 2 || !args[0].isNumerical() || !args[1].isNumerical())    \
		{                                                                            \
			nlwarning("comparison : arguments are not numerical");                   \
			return false;                                                            \
		}                                                                            \
		args[0].toDouble();                                                          \
		args[1].toDouble();	                                                         \
		result.setBool(args[0].getDouble() op args[1].getDouble());                  \
		return true;                                                                 \
	}																				 \
	REGISTER_INTERFACE_USER_FCT(#name, userFct##name)



	// declare all comparison operators
	//CREATE_CMP_OPERATOR(==, eq)
	CREATE_CMP_OPERATOR(!=, ne)
	CREATE_CMP_OPERATOR(==, eq)
	CREATE_CMP_OPERATOR(< , lt)
	CREATE_CMP_OPERATOR(<=, le)
	CREATE_CMP_OPERATOR(> , gt)
	CREATE_CMP_OPERATOR(>=, ge)



	//////////////////
	// Logical OR   //
	//////////////////
	static DECLARE_INTERFACE_USER_FCT(userFctOr)
	{
		for(uint k = 0; k < args.size(); ++k)
		{
			if (!args[k].toBool())
			{
				nlwarning("Argument is not boolean");
				return false;
			}
			if (args[k].getBool())
			{
				result.setBool(true);
				return true;
			}
		}
		result.setBool(false);
		return true;
	}
	REGISTER_INTERFACE_USER_FCT("or", userFctOr)

	//////////////////
	// Logical AND  //
	//////////////////
	static DECLARE_INTERFACE_USER_FCT(userFctAnd)
	{
		for(uint k = 0; k < args.size(); ++k)
		{
			if (!args[k].toBool())
			{
				nlwarning("Argument is not boolean");
				return false;
			}
			if (!args[k].getBool())
			{
				result.setBool(false);
				return true;
			}
		}
		result.setBool(true);
		return true;
	}
	REGISTER_INTERFACE_USER_FCT("and", userFctAnd)

	//////////////////
	// Logical NOT  //
	//////////////////
	static DECLARE_INTERFACE_USER_FCT(userFctNot)
	{
		if (args.size() != 1)
		{
			nlwarning("not : bad number of arguments");
			return false;
		}
		if (!args[0].toBool())
		{
			nlwarning("Argument is not boolean");
			return false;
		}
		result.setBool(!args[0].getBool());
		return true;
	}
	REGISTER_INTERFACE_USER_FCT("not", userFctNot)


	////////////////////////////////////
	// String Add Operation           //
	////////////////////////////////////

	static DECLARE_INTERFACE_USER_FCT(userFctStr)
	{
		if (args.size() > 0)
		{
			ucstring res("");
			for (uint32 i = 0; i < args.size(); ++i)
			{
				args[i].toString();
				res += args[i].getUCString();
			}
			result.setUCString (res);

			return true;
		}
		else
		{
			return false;
		}
	}
	REGISTER_INTERFACE_USER_FCT("str", userFctStr)

	////////////////////////////////////
	// Integer Operation           //
	////////////////////////////////////

	static DECLARE_INTERFACE_USER_FCT(userFctInt)
	{
		if (args.size() != 1)
		{
			return false;
		}

		args[0].toInteger();
		result.setInteger(args[0].getInteger());
		return true;
	}
	REGISTER_INTERFACE_USER_FCT("int", userFctInt)

	////////////////////////////////////
	// Branching Operation ifthenelse //
	////////////////////////////////////

	static DECLARE_INTERFACE_USER_FCT(userFctIfThenElse)
	{
		if ((args.size() < 1) || (args.size() > 3))
			return false;

		if (!args[0].toBool())
			return false;

		if (args[0].getBool())
		{
			result = args[1];
		}
		else
		{
			if (args.size() == 3)
				result = args[2];
			else
				result.setBool (false);
		}

		return true;
	}
	REGISTER_INTERFACE_USER_FCT("ifthenelse", userFctIfThenElse)

	////////////////////////////////
	// Branching Operation switch //
	////////////////////////////////

	static DECLARE_INTERFACE_USER_FCT(userFctSwitch)
	{
		if (args.size() < 2)
			return false;

		if (!args[0].toInteger())
			return false;

		sint64 n = args[0].getInteger();
		if ((n > ((sint64)args.size()-2)) || (n < 0))
			return false;

		result = args[(uint)n+1];

		return true;
	}
	REGISTER_INTERFACE_USER_FCT("switch", userFctSwitch)

	/////////////////////////////////
	// Takes maximum of any numbers //
	/////////////////////////////////
	static DECLARE_INTERFACE_USER_FCT(userFctMax)
	{
		// compute type of output
		if (!promoteToNumericalBestType(args))
		{
			nlwarning("max : invalid entry");
			return false;
		}
		uint i;
		if (args[0].getType() == CIEV::Integer)
		{
			sint64 m;
			m = args[0].getInteger();
			for (i = 1; i < args.size(); ++i)
				m = std::max(m,args[i].getInteger());
			result.setInteger(m);
		}
		else
		{
			double m;
			m = args[0].getDouble();
			for (i = 1; i < args.size(); ++i)
				m = std::max(m,args[i].getDouble());
			result.setDouble(m);
		}
		return true;
	}
	REGISTER_INTERFACE_USER_FCT("max", userFctMax)

	/////////////////////////////////
	// Takes minimum of 2 numbers  //
	/////////////////////////////////
	static DECLARE_INTERFACE_USER_FCT(userFctMin)
	{
		// compute type of output
		if (!promoteToNumericalBestType(args))
		{
			nlwarning("max : invalid entry");
			return false;
		}
		uint i;
		if (args[0].getType() == CIEV::Integer)
		{
			sint64 m;
			m = args[0].getInteger();
			for (i = 1; i < args.size(); ++i)
				m = std::min(m,args[i].getInteger());
			result.setInteger(m);
		}
		else
		{
			double m;
			m = args[0].getDouble();
			for (i = 1; i < args.size(); ++i)
				m = std::min(m,args[i].getDouble());
			result.setDouble(m);
		}
		return true;
	}
	REGISTER_INTERFACE_USER_FCT("min", userFctMin)

	//////////////////////////////
	// Get Reflected Property   //
	//////////////////////////////
	static DECLARE_INTERFACE_USER_FCT(userFctGetProp)
	{
		if (args.size() != 1)
			return false;

		if (args[0].getType() != CIEV::String)
			return false;

		string sTmp = args[0].getString();
		std::vector<CInterfaceLink::CTargetInfo> targetsVector;
		CInterfaceLink::splitLinkTargets(sTmp, NULL, targetsVector);

		if (targetsVector.empty())
		{
			nlwarning("no target found");
			return false;
		}
		CInterfaceLink::CTargetInfo &rTI = targetsVector[0];

		CInterfaceElement *elem = rTI.Elem;
		if (!elem)
		{
			nlwarning("<CInterfaceExpr::getprop> : Element is NULL");
			return false;
		}
		const CReflectedProperty *pRP = elem->getReflectedProperty(rTI.PropertyName);

		if (!pRP) return false;
		switch(pRP->Type)
		{
			case CReflectedProperty::Boolean:
				result.setBool ((elem->*(pRP->GetMethod.GetBool))());
			break;
			case CReflectedProperty::SInt32:
				result.setInteger ((elem->*(pRP->GetMethod.GetSInt32))());
			break;
			case CReflectedProperty::Float:
				result.setDouble ((elem->*(pRP->GetMethod.GetFloat))());
			break;
			case CReflectedProperty::String:
				result.setString ((elem->*(pRP->GetMethod.GetString))());
			break;
			case CReflectedProperty::UCString:
				result.setUCString ((elem->*(pRP->GetMethod.GetUCString))());
			break;
			case CReflectedProperty::RGBA:
				result.setRGBA ((elem->*(pRP->GetMethod.GetRGBA))());
			break;
			default:
				nlstop;
				return false;
		}
		return true;
	}
	REGISTER_INTERFACE_USER_FCT("getprop", userFctGetProp)


	///////////////////////////////
	// Convert an int to a color //
	///////////////////////////////
	static DECLARE_INTERFACE_USER_FCT(userIntToColor)
	{
		if (args.size() != 1)
		{
			nlwarning("Bad number of args");
			return false;
		}
		if (!args[0].toInteger())
		{
			nlwarning("Bad type for arg 0 : should be an integer");
			return false;
		}
		CRGBA col;
		uint32 intCol = (uint32) args[0].getInteger();
		col.R = uint8(intCol & 0xff);
		col.G = uint8((intCol >> 8) & 0xff);
		col.B = uint8((intCol >> 16) & 0xff);
		col.A = uint8((intCol >> 24) & 0xff);
		result.setRGBA(col);
		return true;
	}
	REGISTER_INTERFACE_USER_FCT("intToColor", userIntToColor)

	///////////////////////////////
	// Get components of a color //
	///////////////////////////////
	static DECLARE_INTERFACE_USER_FCT(userGetRed)
	{
		if (args.size() != 1)
		{
			nlwarning("Bad number of args");
			return false;
		}
		if (!args[0].toRGBA())
		{
			nlwarning("Bad type for arg 0 : should be a color");
			return false;
		}
		result.setInteger((sint64) args[0].getRGBA().R);
		return true;
	}
	REGISTER_INTERFACE_USER_FCT("getRed", userGetRed)
	//
	static DECLARE_INTERFACE_USER_FCT(userGetGreen)
	{
		if (args.size() != 1)
		{
			nlwarning("Bad number of args");
			return false;
		}
		if (!args[0].toRGBA())
		{
			nlwarning("Bad type for arg 0 : should be a color");
			return false;
		}
		result.setInteger((sint64) args[0].getRGBA().G);
		return true;
	}
	REGISTER_INTERFACE_USER_FCT("getGreen", userGetGreen)
	//
	static DECLARE_INTERFACE_USER_FCT(userGetBlue)
	{
		if (args.size() != 1)
		{
			nlwarning("Bad number of args");
			return false;
		}
		if (!args[0].toRGBA())
		{
			nlwarning("Bad type for arg 0 : should be a color");
			return false;
		}
		result.setInteger((sint64) args[0].getRGBA().B);
		return true;
	}
	REGISTER_INTERFACE_USER_FCT("getBlue", userGetBlue)
	//
	static DECLARE_INTERFACE_USER_FCT(userGetAlpha)
	{
		if (args.size() != 1)
		{
			nlwarning("Bad number of args");
			return false;
		}
		if (!args[0].toRGBA())
		{
			nlwarning("Bad type for arg 0 : should be a color");
			return false;
		}
		result.setInteger((sint64) args[0].getRGBA().A);
		return true;
	}
	REGISTER_INTERFACE_USER_FCT("getAlpha", userGetAlpha)

	////////////////////////////////////////////////
	// make a rgb color from 3 components R, G, B //
	////////////////////////////////////////////////
	static DECLARE_INTERFACE_USER_FCT(userMakeRGB)
	{
		if ((args.size() != 3) && (args.size() != 4))
		{
			nlwarning("Bad number of args : 3 or 4 args required : R, G, B, [A]");
			return false;
		}
		if (!args[0].toInteger() || !args[1].toInteger() || !args[2].toInteger())
		{
			nlwarning("Not all args converting to integer");
			return false;
		}
		uint8 nAlpha = 255;
		if (args.size() == 4 )
			nAlpha = (uint8)args[3].getInteger();

		NLMISC::CRGBA col((uint8) args[0].getInteger(), (uint8) args[1].getInteger(), (uint8) args[2].getInteger(),nAlpha);
		result.setRGBA(col);

		return true;
	}
	REGISTER_INTERFACE_USER_FCT("makeRGB", userMakeRGB)


	//////////////////////////////
	// Get number of DB entries //
	//////////////////////////////
	static DECLARE_INTERFACE_USER_FCT(userDBCount)
	{
		if (args.size() != 1)
			return false;

		if (args[0].getType() != CIEV::String)
			return false;

		string sTmp = args[0].getString();

		if (sTmp.find('$') == string::npos)
			return false;

		string sFirstPart = sTmp.substr(0,sTmp.find('$'));
		string sSecondPart = sTmp.substr(sTmp.find('$')+1,sTmp.size());

		sint i = 0;
		bool bExit = false;

		while (!bExit)
		{
			sTmp = sFirstPart + NLMISC::toString(i) + sSecondPart;
			CCDBNodeLeaf *pNL = NLGUI::CDBManager::getInstance()->getDbProp(sTmp,false);
			CCDBNodeBranch *pNB = NLGUI::CDBManager::getInstance()->getDbBranch(sTmp);
			if (pNL != NULL)
			{
				if (pNL->getValue64() == 0)
					bExit = true;
				else
					++i;
			}
			else if (pNB != NULL)
			{
				++i;
			}
			else
			{
				bExit = true;
			}
		}

		result.setInteger(i);

		return true;
	}
	REGISTER_INTERFACE_USER_FCT("dbcount", userDBCount)

	////////////////////////
	// Get a random value //
	////////////////////////
	static DECLARE_INTERFACE_USER_FCT(userFctRand)
	{
		if ((args.size() != 2) && (args.size() != 0))
			return false;

		if (args.size() == 2)
		{
			if (!args[0].toDouble()) return false;
			if (!args[1].toDouble()) return false;

			double s = args[0].getDouble();
			double e = 0.999+args[1].getDouble();

			result.setDouble ( s + (e-s)*NLMISC::frand(1.0));
		}
		else
		{
			result.setDouble (NLMISC::frand(1.0));
		}

		return true;
	}
	REGISTER_INTERFACE_USER_FCT("rand", userFctRand)


	static DECLARE_INTERFACE_USER_FCT(userFctGetBit)
	{
		if (args.size() != 2)
			return false;

		if (!args[0].toInteger()) return false;
		if (!args[1].toInteger()) return false;

		sint64 i = args[0].getInteger();
		sint64 s = args[1].getInteger();

		result.setInteger (0);

		if ((i & (SINT64_CONSTANT(1)<<s)) != 0)
			result.setInteger (1);

		return true;
	}
	REGISTER_INTERFACE_USER_FCT("getbit", userFctGetBit)




	// ***********************
	// INTERPOLATION FUNCTIONS
	// ***********************

	//////////////
	// i_linear //
	//////////////

	static DECLARE_INTERFACE_USER_FCT(userFctILinear)
	{
		if (args.size() != 3)
			return false;

		if (!args[0].toDouble()) return false;
		if (!args[1].toDouble()) return false;
		if (!args[2].toDouble()) return false;

		double i = args[0].getDouble(); // Interpolant
		double s = args[1].getDouble(); // Start
		double e = args[2].getDouble(); // End

		result.setDouble ( s + i * (e - s) );

		return true;
	}
	REGISTER_INTERFACE_USER_FCT("ilinear", userFctILinear)



	//////////////////
	// Bitwise OR   //
	//////////////////
	static DECLARE_INTERFACE_USER_FCT(userFctBitOr)
	{
		uint64	res= 0;
		for(uint k = 0; k < args.size(); ++k)
		{
			if (!args[k].toInteger())
			{
				nlwarning("Argument is not integer");
				return false;
			}
			res|= args[k].getInteger();
		}
		result.setInteger(res);
		return true;
	}
	REGISTER_INTERFACE_USER_FCT("bor", userFctBitOr)

	//////////////////
	// Bitwise AND  //
	//////////////////
	static DECLARE_INTERFACE_USER_FCT(userFctBitAnd)
	{
		if(args.empty())
		{
			result.setInteger(0);
			return true;
		}

		uint64	res= UINT64_CONSTANT(0xFFFFFFFFFFFFFFFF);
		for(uint k = 0; k < args.size(); ++k)
		{
			if (!args[k].toInteger())
			{
				nlwarning("Argument is not integer");
				return false;
			}
			res&= args[k].getInteger();
		}
		result.setInteger(res);
		return true;
	}
	REGISTER_INTERFACE_USER_FCT("band", userFctBitAnd)

	//////////////////
	// Bitwise NOT  //
	//////////////////
	static DECLARE_INTERFACE_USER_FCT(userFctBitNot)
	{
		if (args.size() != 1)
		{
			nlwarning("bnot : bad number of arguments");
			return false;
		}
		if (!args[0].toInteger())
		{
			nlwarning("Argument is not integer");
			return false;
		}
		result.setInteger(~args[0].getInteger());
		return true;
	}
	REGISTER_INTERFACE_USER_FCT("bnot", userFctBitNot)

	//////////////////
	// Bitwise XOR  //
	//////////////////
	static DECLARE_INTERFACE_USER_FCT(userFctBitXor)
	{
		if (args.size() != 2)
		{
			nlwarning("bxor : bad number of arguments");
			return false;
		}
		if (!args[0].toInteger() || !args[1].toInteger())
		{
			nlwarning("Argument is not integer");
			return false;
		}
		result.setInteger(args[0].getInteger() ^ args[1].getInteger());
		return true;
	}
	REGISTER_INTERFACE_USER_FCT("bxor", userFctBitXor)

	///////////////////////////
	// Unsigned right shift  //
	///////////////////////////
	static DECLARE_INTERFACE_USER_FCT(userFctSHR)
	{
		if (args.size() != 2)
		{
			nlwarning("shr : bad number of arguments");
			return false;
		}
		if (!args[0].toInteger() || !args[1].toInteger())
		{
			nlwarning("Argument is not integer");
			return false;
		}
		result.setInteger(((uint64)args[0].getInteger()) >> args[1].getInteger());
		return true;
	}
	REGISTER_INTERFACE_USER_FCT("shr", userFctSHR)

	//////////////////////////
	// Unsigned left shift  //
	//////////////////////////
	static DECLARE_INTERFACE_USER_FCT(userFctSHL)
	{
		if (args.size() != 2)
		{
			nlwarning("shl : bad number of arguments");
			return false;
		}
		if (!args[0].toInteger() || !args[1].toInteger())
		{
			nlwarning("Argument is not integer");
			return false;
		}
		result.setInteger(((uint64)args[0].getInteger()) << args[1].getInteger());
		return true;
	}
	REGISTER_INTERFACE_USER_FCT("shl", userFctSHL)

	/////////////////////////
	// Signed right shift  //
	/////////////////////////
	static DECLARE_INTERFACE_USER_FCT(userFctSAR)
	{
		if (args.size() != 2)
		{
			nlwarning("sar : bad number of arguments");
			return false;
		}
		if (!args[0].toInteger() || !args[1].toInteger())
		{
			nlwarning("Argument is not integer");
			return false;
		}
		result.setInteger(args[0].getInteger() >> args[1].getInteger());
		return true;
	}
	REGISTER_INTERFACE_USER_FCT("sar", userFctSAR)

	////////////////////////
	// Signed left shift  //
	////////////////////////
	static DECLARE_INTERFACE_USER_FCT(userFctSAL)
	{
		if (args.size() != 2)
		{
			nlwarning("sal : bad number of arguments");
			return false;
		}
		if (!args[0].toInteger() || !args[1].toInteger())
		{
			nlwarning("Argument is not integer");
			return false;
		}
		result.setInteger(args[0].getInteger() << args[1].getInteger());
		return true;
	}
	REGISTER_INTERFACE_USER_FCT("sal", userFctSAL)

	/////////////////////////////////////////////////////////////////////////////
	// Extend the sign of the argument considered over 8 bits to a 64 bits int //
	/////////////////////////////////////////////////////////////////////////////
	static DECLARE_INTERFACE_USER_FCT(extSign8To64)
	{
		if (args.size() != 1)
		{
			nlwarning("extSign8To64 : bad number of arguments");
			return false;
		}
		if (!args[0].toInteger())
		{
			nlwarning("Argument is not integer");
			return false;
		}
		sint8 i = (sint8)args[0].getInteger();
		result.setInteger((sint64)i);
		return true;
	}
	REGISTER_INTERFACE_USER_FCT("extSign8To64", extSign8To64)

	//////////////////////////////////////////////////////////////////////////////
	// Extend the sign of the argument considered over 11 bits to a 64 bits int //
	//////////////////////////////////////////////////////////////////////////////
	static DECLARE_INTERFACE_USER_FCT(extSign11To64)
	{
		if (args.size() != 1)
		{
			nlwarning("extSign12To64 : bad number of arguments");
			return false;
		}
		if (!args[0].toInteger())
		{
			nlwarning("Argument is not integer");
			return false;
		}
		sint32 i = (sint16)args[0].getInteger() & 0x7ff;
		if( i > 1023 )
			i |= 0xfffff800;
		result.setInteger((sint64)i);
		return true;
	}
	REGISTER_INTERFACE_USER_FCT("extSign11To64", extSign11To64)

	////////////////////////
	// Ryzom version info //
	////////////////////////
	static DECLARE_INTERFACE_USER_FCT(isFinalVersion)
	{
		if (!args.empty())
		{
			nlwarning("isFinalVersion : no args required");
			return false;
		}
		#if FINAL_VERSION
			result.setBool(true);
		#else
			result.setBool(false);
		#endif
		return true;
	}
	REGISTER_INTERFACE_USER_FCT("isFinalVersion", isFinalVersion)

	////////////////////////
	////////////////////////
	static DECLARE_INTERFACE_USER_FCT(secondsToTimeString)
	{
		if (args.size() != 1)
		{
			nlwarning("intToTimeString : 1 args required");
			return false;
		}
		if (!args[0].toInteger())
		{
			nlwarning("intToTimeString : args 0 required to be an int");
			return false;
		}

		sint64 nVal = args[0].getInteger();
		ucstring sTmp;

		if (nVal < 0) nVal = 0;

		sTmp = toString(nVal % 60) + " " + CI18N::get("uiMissionTimerSecond");

		nVal = nVal / 60;

		if( nVal > 0 )
		{
			sTmp = toString(nVal % 60) + " " + CI18N::get("uiMissionTimerMinute") + " " + sTmp;

			nVal = nVal / 60;

			if( nVal > 0 )
			{
				sTmp = toString(nVal % 24) + " " + CI18N::get("uiMissionTimerHour") + " " + sTmp;

				nVal = nVal / 24;

				if( nVal > 0 )
				{
					sTmp = toString(nVal) + " " + CI18N::get("uiMissionTimerDay") + " " + sTmp;
				}
			}
		}

		result.setUCString(sTmp);

		return true;
	}
	REGISTER_INTERFACE_USER_FCT("secondsToTimeString", secondsToTimeString)

	////////////////////////
	////////////////////////
	static DECLARE_INTERFACE_USER_FCT(secondsToTimeStringShort)
	{
		if (args.size() != 1)
		{
			nlwarning("intToTimeString : 1 args required");
			return false;
		}
		if (!args[0].toInteger())
		{
			nlwarning("intToTimeString : args 0 required to be an int");
			return false;
		}

		sint64 nVal = args[0].getInteger();
		ucstring sTmp;

		if (nVal < 0) nVal = 0;

		sTmp = toString("%02d", nVal % 60);

		nVal = nVal / 60;

		if( nVal > 0 )
		{
			sTmp = toString(nVal % 60) + "'" + sTmp;

			nVal = nVal / 60;

			// if at least one hour, just display number of hour
			if( nVal > 0 )
			{
				sTmp = toString(nVal % 24) + "h";

				nVal = nVal / 24;

				// if at least one hour, just display number of days
				if( nVal > 0 )
				{
					sTmp = toString(nVal) + "d";
				}
			}
		}

		result.setUCString(sTmp);

		return true;
	}
	REGISTER_INTERFACE_USER_FCT("secondsToTimeStringShort", secondsToTimeStringShort)

	////////////////////////
	////////////////////////
	static DECLARE_INTERFACE_USER_FCT(oldvalue)
	{
		if (args.size() != 1)
		{
			nlwarning("oldvalue : 1 arg required");
			return false;
		}
		CCDBNodeLeaf *nl = NLGUI::CDBManager::getInstance()->getDbProp(args[0].getString());
		if (!nl)
		{
			nlwarning("oldvalue : arg 0 required to be an interface leaf");
			return false;
		}
		result.setInteger (nl->getOldValue64());

		return true;
	}
	REGISTER_INTERFACE_USER_FCT("oldvalue", oldvalue)

	////////////////////////
	////////////////////////
	static DECLARE_INTERFACE_USER_FCT(localize)
	{
		 if (args.size() != 1)
		 {
			  nlwarning("localize : 1 arg required");
			  return false;
		 }
		 result.setUCString(CI18N::get(args[0].getString()));
		 return true;
	}
	REGISTER_INTERFACE_USER_FCT("localize", localize);

}

