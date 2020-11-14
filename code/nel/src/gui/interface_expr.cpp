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
#include "nel/misc/algo.h"
#include <algorithm>
#include "nel/gui/db_manager.h"
#include "nel/gui/interface_expr.h"
#include "nel/gui/interface_expr_node.h"

using namespace std;
using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NLGUI
{

	void ifexprufct_forcelink();

	// Needed because otherwise GCC and co. omit the code in interface_expr_user_fct.cpp code
	// causing the GUI not to work.
	// It all happens because no function is called *directly* from that module.
	struct LinkTrickster
	{
		LinkTrickster()
		{
			ifexprufct_forcelink();
		}
	};

	LinkTrickster linkTrickster;

	// Yoyo: Act like a singleton, else registerUserFct may crash.
	CInterfaceExpr::TUserFctMap *CInterfaceExpr::_UserFct= NULL;

	static const std::string ExprLuaId="lua:";

	//==================================================================
	// release memory
	void CInterfaceExpr::release()
	{
		delete _UserFct;
		_UserFct = NULL;
	}

	//==================================================================
	void formatLuaCall(const std::string &expr, std::string &tempStr)
	{
		/* Call the LUA interface exp fct, with the script as line, and resolve string definition conflicts:
			eg:  replace
				lua:getSkillFromName('SM')
			into
				lua('getSkillFromName(\"SM\")')
		*/
		tempStr= expr.substr(ExprLuaId.size());			// eg: tempStr= getSkillFromName('SM')
		while(strFindReplace(tempStr, "'", "\\\""));	// eg: tempStr= getSkillFromName(\"SM\")
		tempStr= string("lua('") + tempStr + "')";		// eg: tempStr= lua('getSkillFromName(\"SM\")')
	}

	//==================================================================
	bool CInterfaceExpr::eval(const std::string &expr, CInterfaceExprValue &result, std::vector<ICDBNode *> *nodes, bool noFctCalls /* = false */)
	{
		// Yoyo: Special InterfaceExpr Form to execute lua code?
		if(expr.compare(0, ExprLuaId.size(), ExprLuaId) ==0 )
		{
			std::string	tempStr;
			formatLuaCall(expr, tempStr);
			return evalExpr(tempStr.c_str(), result, nodes, noFctCalls) != NULL;
		}
		else
		{
			return evalExpr(expr.c_str(), result, nodes, noFctCalls) != NULL;
		}
	}

	//==================================================================
	CInterfaceExprNode *CInterfaceExpr::buildExprTree(const std::string &expr)
	{
		CInterfaceExprNode *node;

		// Yoyo: Special InterfaceExpr Form to execute lua code?
		if(expr.compare(0, ExprLuaId.size(), ExprLuaId) ==0 )
		{
			std::string	tempStr;
			formatLuaCall(expr, tempStr);
			if (!buildExprTree(tempStr.c_str(), node)) return NULL;
		}
		else
		{
			if (!buildExprTree(expr.c_str(), node)) return NULL;
		}

		return node;
	}


	//==================================================================
	void CInterfaceExpr::registerUserFct(const char *name,TUserFct fct)
	{
		if(!_UserFct)	_UserFct= new TUserFctMap;

		nlassert(fct != NULL);
		(*_UserFct)[std::string(name)] = fct;
	}

	//==================================================================
	/** tool fct : skip space, tab and carret-returns
	  */
	static const char *skipBlank(const char *start)
	{
		nlassert(start);
		while (*start == ' ' || *start == '\t' || *start == '\r' || *start == '\n') ++start;
		return start;
	}

	//==================================================================
	const char *CInterfaceExpr::evalExpr(const char *expr, CInterfaceExprValue &result, std::vector<ICDBNode *> *nodes, bool noFctCalls)
	{
		nlassert(expr != NULL);
		expr = skipBlank(expr);
		if (isalpha(*expr)) // alpha character means this is a function name
		{
			return evalFct(expr, result, nodes, noFctCalls);
		}
		else if (*expr == '@') // is it a database entry ?
		{
			++ expr;
			expr = skipBlank(expr);
			return evalDBEntry(expr, result, nodes);
		}

		// try to parse a literal value
		const char *newExpr = result.initFromString(expr);
		if (!newExpr)
		{
			nlwarning("<CInterfaceExpr::evalExpr> : syntax error : %s", expr);
			return NULL;
		}
		return newExpr;
	}

	//==================================================================
	const char *CInterfaceExpr::buildExprTree(const char *expr, CInterfaceExprNode *&result)
	{
		nlassert(expr != NULL);
		expr = skipBlank(expr);
		if (isalpha(*expr)) // alpha character means this is a function name
		{
			return buildFctNode(expr, result);
		}
		else if (*expr == '@') // is it a database entry ?
		{
			++ expr;
			expr = skipBlank(expr);
			return buildDBEntryNode(expr, result);
		}
		else
		{
			CInterfaceExprValue value;
			// try to parse a literal value
			const char *newExpr = value.initFromString(expr);
			if (!newExpr)
			{
				nlwarning("<CInterfaceExpr::buildExprTree> : syntax error : %s", expr);
				return NULL;
			}
			CInterfaceExprNodeValue *node = new CInterfaceExprNodeValue;
			node->Value = value;
			result = node;
			return newExpr;
		}
		return NULL;
	}


	//==================================================================
	const char *CInterfaceExpr::evalFct(const char *expr, CInterfaceExprValue &result, std::vector<ICDBNode *> *nodes, bool noFctCalls)
	{
		if(!_UserFct)	_UserFct= new TUserFctMap;

		const char *start = expr;
		while (isalnum(*expr)) ++ expr;
		std::string fctName(start, expr - start);
		// find entry in the map
		TUserFctMap::iterator fctIt = _UserFct->find(fctName);
		if (fctIt == _UserFct->end())
		{
			nlwarning("<CInterfaceExpr::evalFct> : Unknown function %s", fctName.c_str());
			return NULL;
		}
		nlassert(fctIt->second != NULL);
		// eval list of arguments
		TArgList argList;
		expr = skipBlank(expr);
		if (*expr != '(')
		{
			nlwarning("<CInterfaceExpr::evalFct> : '(' expected for function %s", fctName.c_str());
			return NULL;
		}
		++ expr;
		expr = skipBlank(expr);
		if (*expr != ')')
		{
			for(;;)
			{
				expr = skipBlank(expr);
				// parse an argument
				argList.push_back(CInterfaceExprValue());
				expr = evalExpr(expr, argList.back(), nodes, noFctCalls);
				if (expr == NULL) return NULL;
				expr = skipBlank(expr);
				if (*expr == ')') break;
				// if it isn't the end of the expression, then we should find a ',' before next argument
				if (*expr != ',')
				{
					nlwarning("<CInterfaceExpr::evalFct> : ',' expected in function %s", fctName.c_str());
					return NULL;
				}
				++ expr;
			}
		}
		++ expr;
		// call the fct
		if (!noFctCalls) // should we make terminal function calls ?
		{
			if (fctIt->second(argList, result)) return expr;
		}
		else
		{
			return expr;
		}
		return NULL;
	}

	//==================================================================
	const char *CInterfaceExpr::buildFctNode(const char *expr, CInterfaceExprNode *&result)
	{
		if(!_UserFct)	_UserFct= new TUserFctMap;

		const char *start = expr;
		while (isalnum(*expr)) ++ expr;
		std::string fctName(start, expr - start);
		// find entry in the map
		TUserFctMap::iterator fctIt = _UserFct->find(fctName);
		if (fctIt == _UserFct->end())
		{
			nlwarning("<CInterfaceExpr::buildFctNode> : Unknown function %s", fctName.c_str());
			return NULL;
		}
		nlassert(fctIt->second != NULL);
		// List of parameters
		expr = skipBlank(expr);
		if (*expr != '(')
		{
			nlwarning("<CInterfaceExpr::buildFctNode> : '(' expected for function %s", fctName.c_str());
			return NULL;
		}
		++ expr;
		expr = skipBlank(expr);
		std::vector<CInterfaceExprNode *> Params;
		if (*expr != ')')
		{
			for(;;)
			{
				expr = skipBlank(expr);
				// parse an argument
				CInterfaceExprNode *node = NULL;
				expr = buildExprTree(expr, node);
				if (expr == NULL)
				{
					for(uint k = 0; k < Params.size(); ++k)
					{
						delete Params[k];
					}
					return NULL;
				}
				Params.push_back(node);
				expr = skipBlank(expr);
				if (*expr == ')') break;
				// if it isn't the end of the expression, then we should find a ',' before next argument
				if (*expr != ',')
				{
					for(uint k = 0; k < Params.size(); ++k)
					{
						delete Params[k];
					}
					nlwarning("CInterfaceExpr::evalFct : ',' expected in function %s", fctName.c_str());
					return NULL;
				}
				++ expr;
			}
		}
		++ expr;
		CInterfaceExprNodeValueFnCall *node = new CInterfaceExprNodeValueFnCall;
		node->Params.swap(Params);
		node->Func = fctIt->second;
		result = node;
		return expr;
	}

	//==================================================================
	const char *CInterfaceExpr::evalDBEntry(const char *expr, CInterfaceExprValue &result, std::vector<ICDBNode *> *nodes)
	{
		std::string dbEntry;
		expr = unpackDBentry(expr, nodes, dbEntry);
		if (!expr) return NULL;
		// TestYoyo
		//nlassert(NLGUI::CDBManager::getInstance()->getDbProp(dbEntry, false) || CInterfaceManager::getInstance()->getDbBranch(dbEntry));
		// get the db value
		CCDBNodeLeaf *nl = NLGUI::CDBManager::getInstance()->getDbProp(dbEntry);
		if (nl)
		{
			if (nodes)
			{
				// insert node if not already present
				if (std::find(nodes->begin(), nodes->end(), nl) == nodes->end())
				{
					nodes->push_back(nl);
				}
			}
			result.setInteger(nl->getValue64());
			return expr;
		}
		else
		{
			CCDBNodeBranch *nb = NLGUI::CDBManager::getInstance()->getDbBranch(dbEntry);
			if (nodes && nb)
			{
				if (std::find(nodes->begin(), nodes->end(), nb) == nodes->end())
				{
					nodes->push_back(nb);
				}
			}
			if (!nb) return NULL;
			result.setInteger(0);
			return expr;
		}
		return NULL;
	}

	//==================================================================
	const char *CInterfaceExpr::buildDBEntryNode(const char *expr, CInterfaceExprNode *&result)
	{
		std::string dbEntry;
		bool indirection;
		const char *startChar = expr;
		expr = unpackDBentry(expr, NULL, dbEntry, &indirection);
		if (!expr) return NULL;
		if (indirection)
		{
			// special node with no optimisation
			CInterfaceExprNodeDependantDBRead *node = new CInterfaceExprNodeDependantDBRead;
			node->Expr.resize(expr - startChar + 1);
			std::copy(startChar, expr, node->Expr.begin() + 1);
			node->Expr[0] = '@';
			result = node;
			return expr;
		}
		else
		{
			// TestYoyo
			//nlassert(NLGUI::CDBManager::getInstance()->getDbProp(dbEntry, false) || CInterfaceManager::getInstance()->getDbBranch(dbEntry));
			CCDBNodeLeaf *nl = NLGUI::CDBManager::getInstance()->getDbProp(dbEntry);
			if (nl)
			{
				CInterfaceExprNodeDBLeaf *node = new CInterfaceExprNodeDBLeaf;
				node->Leaf = nl;
				result = node;
				return expr;
			}
			else
			{
				CCDBNodeBranch *nb = NLGUI::CDBManager::getInstance()->getDbBranch(dbEntry);
				if (nb)
				{
					CInterfaceExprNodeDBBranch *node = new CInterfaceExprNodeDBBranch;
					node->Branch = nb;
					result = node;
					return expr;
				}
			}
			return NULL;
		}
	}

	//==================================================================
	const char *CInterfaceExpr::unpackDBentry(const char *expr, std::vector<ICDBNode *> *nodes, std::string &dest, bool *hasIndirections /* = NULL*/)
	{
		std::string entryName;
		bool indirection = false;
		for (;;)
		{
			if (*expr == '[')
			{
				indirection = true;
				++ expr;
				std::string subEntry;
				expr = unpackDBentry(expr, nodes, subEntry);
				if (!expr) return NULL;
				// Read DB Index Offset.
				sint32	indirectionOffset= 0;
				if (*expr == '-' || *expr =='+' )
				{
					bool	negative= *expr == '-';
					std::string offsetString;
					++ expr;
					while(*expr!=0 && isdigit(*expr))
					{
						offsetString.push_back(*expr);
						++ expr;
					}
					// get offset
					fromString(offsetString, indirectionOffset);
					if(negative)
						indirectionOffset= -indirectionOffset;
				}
				// Test end of indirection
				if (*expr != ']')
				{
					nlwarning("CInterfaceExpr::unpackDBentry: ']' expected");
					return NULL;
				}
				++ expr;
				// get the db value at sub entry
				// TestYoyo
				//nlassert(NLGUI::CDBManager::getInstance()->getDbProp(subEntry, false) || CInterfaceManager::getInstance()->getDbBranch(subEntry));
				CCDBNodeLeaf *nl = NLGUI::CDBManager::getInstance()->getDbProp(subEntry);
				if (nodes)
				{
					if (std::find(nodes->begin(), nodes->end(), nl) == nodes->end())
					{
						nodes->push_back(nl);
					}
				}
				// compute indirection, (clamp).
				sint32	indirectionValue= nl->getValue32() + indirectionOffset;
				indirectionValue= std::max((sint32)0, indirectionValue);

				// Append to entry name.
				entryName += NLMISC::toString(indirectionValue);
			}
			else if (isalnum(*expr) || *expr == '_' || *expr == ':')
			{
				entryName += *expr;
				++ expr;
			}
			else
			{
				break;
			}
		}
		if (hasIndirections)
		{
			*hasIndirections = indirection;
		}
		dest = entryName;
		return expr;
	}


	//==================================================================
	bool CInterfaceExpr::evalAsInt(const std::string &expr, sint64 &dest)
	{
		CInterfaceExprValue result;
		if (!eval(expr, result)) return false;
		if (!result.toInteger())
		{
			nlwarning("<CInterfaceExpr::evalAsInt> Can't convert value to an integer, expr = %s", expr.c_str());
			return false;
		}
		dest = result.getInteger();
		return true;
	}

	//==================================================================
	bool CInterfaceExpr::evalAsDouble(const std::string &expr, double &dest)
	{
		CInterfaceExprValue result;
		if (!eval(expr, result)) return false;
		if (!result.toDouble())
		{
			nlwarning("<CInterfaceExpr::evalAsDouble> Can't convert value to a double, expr = %s", expr.c_str());
			return false;
		}
		dest = result.getDouble();
		return true;
	}

	//==================================================================
	bool CInterfaceExpr::evalAsBool(const std::string &expr, bool &dest)
	{
		CInterfaceExprValue result;
		if (!eval(expr, result)) return false;
		if (!result.toBool())
		{
			nlwarning("<CInterfaceExpr::evalAsBool> Can't convert value to a boolean, expr = %s", expr.c_str());
			return false;
		}
		dest = result.getBool();
		return true;
	}

	//==================================================================
	bool CInterfaceExpr::evalAsString(const std::string &expr, std::string &dest)
	{
		CInterfaceExprValue result;
		if (!eval(expr, result)) return false;
		if (!result.toString())
		{
			nlwarning("<CInterfaceExpr::evalAsString> Can't convert value to a string, expr = %s", expr.c_str());
			return false;
		}
		dest = result.getString();
		return true;
	}

	//==================================================================
	//==================================================================
	//==================================================================
	//==================================================================


	//==================================================================
	bool CInterfaceExprValue::toBool()
	{
		switch(_Type)
		{
			case Boolean: return true;
			case Integer: setBool(_IntegerValue != 0); return true;
			case Double:  setBool(_DoubleValue != 0); return true;
			case String:  return evalBoolean(_StringValue.toString().c_str()) != NULL;
			default: break;
		}
		return false;

	}

	//==================================================================
	bool CInterfaceExprValue::toInteger()
	{
		switch(_Type)
		{
			case Boolean: setInteger(_BoolValue ? 1 : 0); return true;
			case Integer: return true;
			case Double:  setInteger((sint64) _DoubleValue); return true;
			case String:
				if (evalNumber(_StringValue.toString().c_str())) return toInteger();
				return false;
			case RGBA:	setInteger((sint64) _RGBAValue); return true;
			default: break;
		}
		return false;
	}

	//==================================================================
	bool CInterfaceExprValue::toDouble()
	{
		switch(_Type)
		{
			case Boolean:	setDouble(_BoolValue ? 1 : 0); return true;
			case Integer:	setDouble((double) _IntegerValue); return true;
			case Double:	return true;
			case String:
				if (evalNumber(_StringValue.toString().c_str())) return toBool();
				return false;
			case RGBA:	setDouble((double) _RGBAValue); return true;
			default: break;
		}
		return false;
	}

	//==================================================================
	bool CInterfaceExprValue::toString()
	{
		switch(_Type)
		{
			case Boolean:	setString(_BoolValue ? "true" : "false"); return true;
			case Integer:	setString(NLMISC::toString(_IntegerValue)); return true;
			case Double:	setString(NLMISC::toString("%.2f", _DoubleValue)); return true;
			case String:	return true;
			case RGBA:
			{
				uint	r,g,b,a;
				r= (_RGBAValue&0xff);
				g= ((_RGBAValue>>8)&0xff);
				b= ((_RGBAValue>>16)&0xff);
				a= ((_RGBAValue>>24)&0xff);
				setString(NLMISC::toString("%d %d %d %d", r, g, b, a));
				return true;
			}
			default: break;
		}
		return false;
	}

	//==================================================================
	bool CInterfaceExprValue::toRGBA()
	{
		switch(_Type)
		{
			case RGBA:
				return true;

			case Integer:
				setRGBA(NLMISC::CRGBA((uint8)(_IntegerValue&0xff), (uint8)((_IntegerValue>>8)&0xff),
					(uint8)((_IntegerValue>>16)&0xff), (uint8)((_IntegerValue>>24)&0xff)));
				return true;

			case String:
				setRGBA( NLMISC::CRGBA::stringToRGBA(_StringValue.toString().c_str()));
				return true;

			default:
				break;
		}
		return false;
	}

	//==================================================================
	bool CInterfaceExprValue::isNumerical() const
	{
		return _Type == Boolean || _Type == Integer || _Type == Double;
	}

	//==================================================================
	const char *CInterfaceExprValue::initFromString(const char *expr)
	{
		nlassert(expr);
		expr = skipBlank(expr);
		if (isdigit(*expr) || *expr == '.' || *expr == '-') return evalNumber(expr);
		switch(*expr)
		{
			case 't':
			case 'T':
			case 'f':
			case 'F':
				return evalBoolean(expr);
			case '\'':
				return evalString(expr);
			default:
				return NULL;
		}
	}

	//==================================================================
	const char *CInterfaceExprValue::evalBoolean(const char *expr)
	{
		nlassert(expr);
		expr = skipBlank(expr);
		if (toupper(expr[0]) == 'T' &&
			toupper(expr[1]) == 'R' &&
			toupper(expr[2]) == 'U' &&
			toupper(expr[3]) == 'E')
		{
			setBool(true);
			return expr + 4;
		}
		//
		if (toupper(expr[0]) == 'F' &&
			toupper(expr[1]) == 'A' &&
			toupper(expr[2]) == 'L' &&
			toupper(expr[3]) == 'S' &&
			toupper(expr[4]) == 'E')
		{
			setBool(false);
			return expr + 5;
		}
		return NULL;
	}

	//==================================================================
	const char *CInterfaceExprValue::evalNumber(const char *expr)
	{
		bool negative;
		bool hasPoint = false;

		expr = skipBlank(expr);

		if (*expr == '-')
		{
			negative = true;
			++ expr;
			expr = skipBlank(expr);
		}
		else
		{
			negative = false;
		}

		const char *start = expr;
		while (*expr == '.' || isdigit(*expr))
		{
			if (*expr == '.') hasPoint = true;
			++ expr;
		}
		if (start == expr) return NULL;
		if (!hasPoint)
		{
			sint64 value = 0;
			// this is an integer
			for (const char *nbPtr = start; nbPtr < expr; ++ nbPtr)
			{
				value *= 10;
				value += (sint64) (*nbPtr - '0');
			}
			setInteger(negative ? - value : value);
			return expr;
		}
		else // floating point value : use scanf
		{
			// well, for now, we only parse a float
			float value;
			std::string floatValue(start, expr - start);
			if (fromString(floatValue, value))
			{
				setDouble(negative ? - value : value);
				return expr;
			}
			else
			{
				return NULL;
			}
		}
	}

	//==================================================================
	const char *CInterfaceExprValue::evalString(const char *expr)
	{
		expr = skipBlank(expr);
		if (*expr != '\'') return NULL;
		++expr;
		std::string str;
		for (;;)
		{
			if (*expr == '\0')
			{
				nlwarning("CInterfaceExprValue::evalString : end of buffer encountered in a string");
				return NULL;
			}
			else
			if (*expr == '\'')
			{
				++ expr;
				break;
			}
			if (*expr == '\\') // special char
			{
				++ expr;
				switch (*expr)
				{
					case 't': str  += '\t'; break;
					case 'r': str  += '\r'; break;
					case 'n': str  += '\n'; break;
					case '\'': str += '\''; break;
					case '"': str  += '"'; break;
					case '\\': str += '\\'; break;
					case '\n':
					case '\r':
						// string continue on next line, so do nothing
					break;
					case '\0': continue;
					default:
						nlwarning("CInterfaceExprValue::evalString : unknown escape sequence : \\%c", *expr);
						if (*expr) str += *expr;
					break;
				}
			}
			else if (*expr == '\n' || *expr == '\r')
			{
				nlwarning("CInterfaceExprValue::evalString : line break encountered in a string");
				return NULL;
			}
			else
			{
				str += *expr;
			}
			++ expr;
		}
		setString(str);
		return expr;
	}

	//==================================================================
	bool CInterfaceExprValue::toType(TType type)
	{
		switch(type)
		{
			case Boolean: return toBool();
			case Integer: return toInteger();
			case Double:  return toDouble();
			case String:  return toString();
			case RGBA:	  return toRGBA();
			default: return false;
		}
	}


	//==================================================================
	void CInterfaceExprValue::clean()
	{
		switch (_Type)
		{
			case String:   _StringValue.clear(); break;
			case UserType: delete _UserTypeValue; break;
			default: break;
		}
	}

	//==================================================================
	void CInterfaceExprValue::setUserType(CInterfaceExprUserType *value)
	{
		if (_Type == UserType && value == _UserTypeValue) return;
		clean();
		_Type = UserType;
		_UserTypeValue = value;
	}

	//==================================================================
	bool CInterfaceExprValue::getBool() const
	{
		if (_Type != Boolean)
		{
			nlwarning("<CInterfaceExprValue::getBool> bad type!");
			return false;
		}
		return _BoolValue;
	}

	//==================================================================
	sint64 CInterfaceExprValue::getInteger() const
	{
		if (_Type != Integer)
		{
			nlwarning("<CInterfaceExprValue::getInteger> bad type!");
			return 0;
		}
		return _IntegerValue;
	}

	//==================================================================
	double CInterfaceExprValue::getDouble() const
	{
		if (_Type != Double)
		{
			nlwarning("<CInterfaceExprValue::getDouble> bad type!");
			return 0;
		}
		return _DoubleValue;
	}

	//==================================================================
	std::string CInterfaceExprValue::getString() const
	{
		if (_Type != String)
		{
			nlwarning("<CInterfaceExprValue::getString> bad type!");
			return "";
		}
		return _StringValue.toString();
	}

	//==================================================================
	NLMISC::CRGBA CInterfaceExprValue::getRGBA() const
	{
		if (_Type != RGBA)
		{
			nlwarning("<CInterfaceExprValue::getRGBA> bad type!");
			return CRGBA::White;
		}
		NLMISC::CRGBA col;
		col.R = (uint8)(_RGBAValue&0xff);
		col.G = (uint8)((_RGBAValue>>8)&0xff);
		col.B = (uint8)((_RGBAValue>>16)&0xff);
		col.A = (uint8)((_RGBAValue>>24)&0xff);
		return col;
	}


	//==================================================================
	const ucstring &CInterfaceExprValue::getUCString() const
	{
		if (_Type != String)
		{
			nlwarning("<CInterfaceExprValue::getString> bad type!");
			static ucstring emptyString;
			return emptyString;
		}
		return _StringValue;
	}

	//==================================================================
	CInterfaceExprUserType *CInterfaceExprValue::getUserType() const
	{
		if (_Type != UserType)
		{
			nlwarning("<CInterfaceExprValue::getUserType> bad type!");
			return NULL;
		}
		return _UserTypeValue;
	}

	//==================================================================
	CInterfaceExprValue::CInterfaceExprValue(const CInterfaceExprValue &other) : _Type(NoType)
	{
		*this = other;
	}

	//==================================================================
	CInterfaceExprValue &CInterfaceExprValue::operator = (const CInterfaceExprValue &other)
	{
		if (this != &other)
		{
			clean();
			switch(other._Type)
			{
				case Boolean:  _BoolValue    = other._BoolValue;    break;
				case Integer:  _IntegerValue = other._IntegerValue; break;
				case Double:   _DoubleValue  = other._DoubleValue;  break;
				case String:   _StringValue  = other._StringValue;  break;
				case RGBA:	   _RGBAValue    = other._RGBAValue;    break;
				case UserType:
					if (other._UserTypeValue != NULL)
					{
						_UserTypeValue = other._UserTypeValue->clone();
					}
					else
					{
						_UserTypeValue = NULL;
					}
				break;
				case NoType: break;
				default:
					nlwarning("<CInterfaceExprValue::operator=> bad source type") ;
					return *this;
				break;
			}
			_Type = other._Type;
		}
		return *this;
	}

}

