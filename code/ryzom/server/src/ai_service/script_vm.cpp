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

#include "script_vm.h"
#include "script_compiler.h"

using namespace std;
using namespace NLMISC;

namespace AIVM
{

//////////////////////////////////////////////////////////////////////////////
// Library management                                                       //
//////////////////////////////////////////////////////////////////////////////

CLibrary* CLibrary::_instance = NULL;
CLibrary::TLibContainer CLibrary::_libs;

#if !FINAL_VERSION
CVariable<bool>	AIScriptDisplayPrint("aiscript", "AIScriptDisplayPrint", "Display the script 'print's in the AIS log.", true, 0, true);
#else
CVariable<bool>	AIScriptDisplayPrint("aiscript", "AIScriptDisplayPrint", "Display the script 'print's in the AIS log.", false, 0, true);
#endif
CVariable<bool>	AIScriptDisplayLog("aiscript", "AIScriptDisplayLog", "Display the script 'log's in the AIS log.", true, 0, true);

void CLibrary::addLib(std::string const& name, std::string const& code)
{
	vector<string> codeVect;
	size_t start, end;
	start = 0;
	// Cut code in lines
	while (start<code.length())
	{
		end = code.find("\n", start);
		if (end!=std::string::npos)
		{
			codeVect.push_back(code.substr(start, end-start));
			start = end+1;
		}
		else
		{
			codeVect.push_back(code.substr(start));
			break;
		}
	}
	NLMISC::CSmartPtr<CByteCode const> byteCode = AICOMP::CCompiler::getInstance().compileCode(codeVect, name);
	if (byteCode!=NULL)
		addLib(name, byteCode);
}

void CLibrary::addLib(std::string const& name, std::vector<std::string> const& code)
{
	NLMISC::CSmartPtr<CByteCode const> byteCode=AICOMP::CCompiler::getInstance().compileCode(code, name);
	if (byteCode!=NULL)
		addLib(name, byteCode);
}

void CLibrary::addLib(std::string const& name, NLMISC::CSmartPtr<CByteCode const> const& byteCode)
{
	if (byteCode==NULL)
	{
		nlwarning("Trying to register an empty library script.");
		return;
	}
	nlinfo("Adding script library %s", name.c_str());
	_libs.insert(std::make_pair(name, byteCode));
}

//////////////////////////////////////////////////////////////////////////////
// Virtual machine                                                          //
//////////////////////////////////////////////////////////////////////////////

CSmartPtr<CByteCode const> CLibrary::getLib(std::string const& name)
{
	TLibContainer::const_iterator it = _libs.find(name);
	if (it!=_libs.end())
		return it->second;
	else
		return CSmartPtr<CByteCode const>(NULL);
}

CScriptVM* CScriptVM::_Instance = NULL;

CScriptVM* CScriptVM::getInstance()
{
	if (_Instance==NULL)
		_Instance = new CScriptVM;
	return _Instance;
}

void CScriptVM::destroyInstance()
{
	if (_Instance!=NULL)
	{
		delete _Instance;
		_Instance = NULL;
	}
}

void CScriptVM::interpretCode(
	IScriptContext* thisContext,
	IScriptContext* parentContext,
	IScriptContext* callerContext,
	CByteCodeEntry const& codeScriptEntry)
{
	NLMISC::CSmartPtr<CByteCode const> const& byteCode = codeScriptEntry.code();
	size_t startIndex = codeScriptEntry.index();
	
	if (byteCode.isNull())
		return;
	vector<size_t> const& opcodes = byteCode->_opcodes;
	static TStringId parentStrId = CStringMapper::map("parent");
	static TStringId callerStrId = CStringMapper::map("caller");
	
	CScriptStack stack;
	size_t index = startIndex;
	string currentString;
	
	while (index < opcodes.size())
	{
	#if !FINAL_VERSION
		EOpcode	op = (EOpcode)opcodes[index];
	#endif
		
		switch (opcodes[index])
		{
		default:
		case	INVALID_OPCODE:
			nlwarning("Invalid Opcode for Group '%s' with code in '%s'", thisContext->getContextName().c_str(), byteCode->_sourceName.c_str());
			nlassert(false);
			break;
		case	EOP:
			return;		//	End Of Program

		case	EQ:		//	==		Need: Value1: Value2 After: Value1==Value2 (Boolean as float)
			{
				const	float	res=stack.top(1)==stack.top()?1.f:0.f;
				stack.pop();
				stack.top()=res;
				++index;
			}
			continue;
		case	NEQ:	//	!=		Need: Value1: Value2 After: Value1!=Value2 (Boolean as float)
			{
				const	float	res=stack.top(1)!=stack.top()?1.f:0.f;
				stack.pop();
				stack.top()=res;
				index++;
			}
			continue;
		case	INF:	//	<		Need: Value1: Value2 After: Value1<Value2 (Boolean as float)
			{
				const	float	res=stack.top(1)<stack.top()?1.f:0.f;
				stack.pop();
				stack.top()=res;
				index++;
			}
			continue;
		case	INFEQ:	//	<=		Need: Value1: Value2 After: Value1<=Value2 (Boolean as float)
			{
				const	float	res=stack.top(1)<=stack.top()?1.f:0.f;
				stack.pop();
				stack.top()=res;
				index++;
			}
			continue;
		case	SUP:	//	>		Need: Value1: Value2 After: Value1>Value2 (Boolean as float)
			{
				const	float	res=stack.top(1)>stack.top()?1.f:0.f;
				stack.pop();
				stack.top()=res;
				index++;
			}
			continue;
		case	SUPEQ:	//	>=		Need: Value1: Value2 After: Value1>=Value2 (Boolean as float)
			{
				const	float	res=stack.top(1)>=stack.top()?1.f:0.f;
				stack.pop();
				stack.top()=res;
				index++;
			}
			continue;
		case	ADD:	//	+		Need: Value1: Value2 After: Value1+Value2
			{
				CScriptStack::CStackEntry	&entry0=stack.top();
				CScriptStack::CStackEntry	&entry1=stack.top(1);

				nlassert((entry0.type()==CScriptStack::EFloat||entry0.type()==CScriptStack::EString)
						&&(entry1.type()==CScriptStack::EFloat||entry1.type()==CScriptStack::EString));
					
				breakable
				{
					if (entry0.type()==entry1.type())
					{
						if	(entry0.type()==CScriptStack::EFloat)
							(float&)entry1+=(float&)entry0;
						else
							(string&)entry1+=(string&)entry0;
					}
					else
					{
						if	(entry0.type()==CScriptStack::EFloat)
							(string&)entry1+=toString("%g", (float&)entry0);
						else
						{
							const	float	value=entry1;
							entry1=toString("%g", value);
							(string&)entry1+=(string&)entry0;
						}

					}

				}
				stack.pop();
				index++;
			}
			continue;
		case	SUB:	//	-		Need: Value1: Value2 After: Value1-Value2
			{
				const	float	val=stack.top();
				stack.pop();
				(float&)stack.top()-=val;
				index++;
			}
			continue;
		case	MUL:	//	*		Need: Value1: Value2 After: Value1/Value2
			{
				float	&res=stack.top(1);
				res*=(float&)stack.top();
				stack.pop();
				index++;
			}
			continue;
		case	DIV:	//	/		Need: Value1: Value2 After: Value1/Value2	!Exception Gestion.
			{
				float	&res=stack.top(1);
				const	float	&divisor=stack.top();
				if (divisor==0)
					res=1;
				else
					res/=divisor;
				stack.pop();
				index++;
			}
			continue;
		case	AND:	//	&&		Need: Value1: Value2 After: Value1&&Value2
			{
				const	bool	val1=(float&)stack.top(1)!=0.f;
				const	bool	val2=(float&)stack.top()!=0.f;
				stack.pop();
				stack.top()=(val1&&val2)?1.f:0.f;
				index++;
			}
			continue;
		case	OR:		//	||		Need: Value1: Value2 After: Value1||Value2
			{
				const	bool	val1=(float&)stack.top(1)!=0.f;
				const	bool	val2=(float&)stack.top()!=0.f;
				stack.pop();
				stack.top()=(val1||val2)?1.f:0.f;
				index++;
			}
			continue;
		case	NOT:	//	!		Need: Value After: !Value
			{
				float	&val=stack.top();
				val=(val==0.f)?1.f:0.f;
				index++;
			}
			continue;
		case	PUSH_ON_STACK:	//	Set a Value (float)						Need: - After: Value(float)
			{
				stack.push(*((float*)&opcodes[index+1]));
				index+=2;
			}
			continue;
		case	POP:		//	Pop										Need: ValToPop After: -
			{
				stack.pop();
				index++;
			}
			continue;
		case	SET_VAR_VAL:		//	Set a Value to a Var.				Need: VarName:	VarValue After:	-
			{
				float f = 0.0f;
				switch	(stack.top().type())
				{
				case CScriptStack::EString:
					{
						string	&str=stack.top();
						f=(float)atof(str.c_str());
					}
					break;
				case CScriptStack::EFloat:
					{
						f = (float&)stack.top();
					}
					break;
				default:
					nlwarning("Stack top type invalid, poping top value!");
				}
				stack.pop();
				thisContext->setLogicVar(*((TStringId*)&opcodes[index+1]), f);
				index+=2;
			}
			continue;
		case	SET_STR_VAR_VAL:		//	Set a Value to a Var.				Need: VarName:	VarValue After:	-
			{
				switch (stack.top().type())
				{
				case CScriptStack::EString:
					{
						thisContext->setStrLogicVar(*((TStringId*)&opcodes[index+1]), stack.top());
					}
					break;
				case CScriptStack::EFloat:
					{
						float const& val = stack.top();
						thisContext->setStrLogicVar(*((TStringId*)&opcodes[index+1]),NLMISC::toString("%g", val));
					}
					break;
				default:
					nlwarning("Stack top type invalid, poping top value!");
					thisContext->setStrLogicVar(*((TStringId*)&opcodes[index+1]),std::string());
				}
				stack.pop();
				index+=2;
			}
			continue;
		case	SET_CTX_VAR_VAL:		//	Set a Value to a Var.				Need: VarName:	VarValue After:	-
			{
				switch (stack.top().type())
				{
				case CScriptStack::EContext:
					{
						thisContext->setCtxLogicVar(*((TStringId*)&opcodes[index+1]), stack.top());
					}
					break;
				default:
					nlwarning("Stack top type invalid, poping top value!");
					thisContext->setCtxLogicVar(*((TStringId*)&opcodes[index+1]), (IScriptContext*)0);
				}
				stack.pop();
				index+=2;
			}
			continue;
		case	PUSH_VAR_VAL:	//	Push the Value of a Var.			Need: - (VarName on next IP) After:	VarValue(float)
			{
				const	float	f=thisContext->getLogicVar(*((TStringId*)&opcodes[index+1]));
				stack.push(f);
				index+=2;
			}
			continue;
		case	PUSH_STR_VAR_VAL:	//	Push the Value of a Var.			Need: - (VarName on next IP) After:	VarValue(float)
			{
				std::string str = thisContext->getStrLogicVar(*((TStringId*)&opcodes[index+1]));
				stack.push(str);
				index+=2;
			}
			continue;
		case	PUSH_CTX_VAR_VAL:	//	Push the Value of a Var.			Need: - (VarName on next IP) After:	VarValue(float)
			{
				IScriptContext* ctx = thisContext->getCtxLogicVar(*((TStringId*)&opcodes[index+1]));
				stack.push(ctx);
				index+=2;
			}
			continue;
		/*
		case	SET_OTHER_VAR_VAL:
			{
				const	TStringId	strId=*((TStringId*)&opcodes[index+1]);
				
				IScriptContext* otherContext = NULL;
				if (strId==parentStrId)
				{
					otherContext = parentContext;
				}
				else if (strId==callerStrId)
				{
					otherContext = callerContext;
				}
				else
				{
					otherContext = thisContext->findContext(strId);
				}
				
				float f = 0.0f;
				switch	(stack.top().type())	//stack.type(0))
				{
				case CScriptStack::EString:
					{
						string& str = stack.top();
						f = (float)atof(str.c_str());
					}
					break;
				case CScriptStack::EFloat:
					{
						f = (float&)stack.top();
					}
					break;
				default:
					nlwarning("Stack top type invalid, poping top value!");
				}
				if (otherContext)
					otherContext->setLogicVar(*((TStringId*)&opcodes[index+2]), f);
				stack.pop();
				index+=3;
			}
			continue;
		case	SET_OTHER_STR_VAR_VAL:
			{
				const	TStringId	strId=*((TStringId*)&opcodes[index+1]);
				
				IScriptContext* otherContext = NULL;
				if (strId==parentStrId)
				{
					otherContext = parentContext;
				}
				else if (strId==callerStrId)
				{
					otherContext = callerContext;
				}
				else
				{
					otherContext = thisContext->findContext(strId);
				}
				
				std::string str;
				switch (stack.top().type())
				{
				case CScriptStack::EString:
					{
						str = (string&)stack.top();
					}
					break;
				case CScriptStack::EFloat:
					{
						float& val = (float&)stack.top();
						str = NLMISC::toString("%g", val);
					}
					break;
				default:
					nlwarning("Stack top type invalid, poping top value!");
				}
				if (otherContext)
					otherContext->setStrLogicVar(*((TStringId*)&opcodes[index+2]), str);
				stack.pop();
				index += 3;
			}
			continue;
		case	SET_OTHER_CTX_VAR_VAL:
			{
				TStringId const strId = *((TStringId*)&opcodes[index+1]);
				
				IScriptContext* otherContext = NULL;
				if (strId==parentStrId)
				{
					otherContext = parentContext;
				}
				else if (strId==callerStrId)
				{
					otherContext = callerContext;
				}
				else
				{
					otherContext = thisContext->findContext(strId);
				}
				
				IScriptContext* ctx = (IScriptContext*)0;
				switch (stack.top().type())
				{
				case CScriptStack::EContext:
					{
						ctx = (IScriptContext*)stack.top();
					}
					break;
				default:
					nlwarning("Stack top type invalid, poping top value!");
				}
				if (otherContext)
					otherContext->setCtxLogicVar(*((TStringId*)&opcodes[index+2]), ctx);
				stack.pop();
				index += 3;
			}
			continue;
		case	PUSH_OTHER_VAR_VAL:
			{
				const	TStringId	strId=*((TStringId*)&opcodes[index+1]);
				
				IScriptContext* otherContext = NULL;
				if (strId==parentStrId)
				{
					otherContext = parentContext;
				}
				else if (strId==callerStrId)
				{
					otherContext = callerContext;
				}
				else
				{
					otherContext = thisContext->findContext(strId);
				}
				
				float f;
				if (otherContext)
					f = otherContext->getLogicVar(*((TStringId*)&opcodes[index+2]));
				else
					f = 1.0f;

				stack.push(f);
				index += 3;
			}
			continue;
		case	PUSH_OTHER_STR_VAR_VAL:
			{
				const	TStringId	strId=*((TStringId*)&opcodes[index+1]);
				
				IScriptContext* otherContext = NULL;
				if (strId==parentStrId)
				{
					otherContext = parentContext;
				}
				else if (strId==callerStrId)
				{
					otherContext = callerContext;
				}
				else
				{
					otherContext = thisContext->findContext(strId);
				}
				
				if (otherContext)
					stack.push(otherContext->getStrLogicVar(*((TStringId*)&opcodes[index+2])));
				else
					stack.push(std::string());	//	accepted coz const &
				
				index += 3;
			}
			continue;
		case	PUSH_OTHER_CTX_VAR_VAL:
			{
				TStringId const strId = *((TStringId*)&opcodes[index+1]);
				
				IScriptContext* otherContext = NULL;
				if (strId==parentStrId)
				{
					otherContext = parentContext;
				}
				else if (strId==callerStrId)
				{
					otherContext = callerContext;
				}
				else
				{
					otherContext = thisContext->findContext(strId);
				}
				
				if (otherContext)
					stack.push(otherContext->getCtxLogicVar(*((TStringId*)&opcodes[index+2])));
				else
					stack.push((IScriptContext*)0);
				
				index += 3;
			}
			continue;
		*/
		case	SET_CONTEXT_VAR_VAL:
			{
				IScriptContext* otherContext = (IScriptContext*)0;
				switch (stack.top().type())
				{
				case CScriptStack::EContext:
					{
						otherContext = (IScriptContext*)stack.top();
					}
					break;
				default:
					nlwarning("Stack top type invalid, poping top value!");
				}
				stack.pop();
				
				float f = 0.0f;
				switch	(stack.top().type())	//stack.type(0))
				{
				case CScriptStack::EString:
					{
						string& str = stack.top();
						f = (float)atof(str.c_str());
					}
					break;
				case CScriptStack::EFloat:
					{
						f = (float&)stack.top();
					}
					break;
				default:
					nlwarning("Stack top type invalid, poping top value!");
				}
				if (otherContext)
					otherContext->setLogicVar(*((TStringId*)&opcodes[index+1]), f);
				stack.pop();
				index+=2;
			}
			continue;
		case	SET_CONTEXT_STR_VAR_VAL:
			{
				IScriptContext* otherContext = (IScriptContext*)0;
				switch (stack.top().type())
				{
				case CScriptStack::EContext:
					{
						otherContext = (IScriptContext*)stack.top();
					}
					break;
				default:
					nlwarning("Stack top type invalid, poping top value!");
				}
				stack.pop();
				
				std::string str;
				switch (stack.top().type())
				{
				case CScriptStack::EString:
					{
						str = (string&)stack.top();
					}
					break;
				case CScriptStack::EFloat:
					{
						float& val = (float&)stack.top();
						str = NLMISC::toString("%g", val);
					}
					break;
				default:
					nlwarning("Stack top type invalid, poping top value!");
				}
				if (otherContext)
					otherContext->setStrLogicVar(*((TStringId*)&opcodes[index+1]), str);
				stack.pop();
				index += 2;
			}
			continue;
		case	SET_CONTEXT_CTX_VAR_VAL:
			{
				IScriptContext* otherContext = (IScriptContext*)0;
				switch (stack.top().type())
				{
				case CScriptStack::EContext:
					{
						otherContext = (IScriptContext*)stack.top();
					}
					break;
				default:
					nlwarning("Stack top type invalid, poping top value!");
				}
				stack.pop();
				
				IScriptContext* ctx = (IScriptContext*)0;
				switch (stack.top().type())
				{
				case CScriptStack::EContext:
					{
						ctx = (IScriptContext*)stack.top();
					}
					break;
				default:
					nlwarning("Stack top type invalid, poping top value!");
				}
				if (otherContext)
					otherContext->setCtxLogicVar(*((TStringId*)&opcodes[index+1]), ctx);
				stack.pop();
				index += 2;
			}
			continue;
		case	PUSH_CONTEXT_VAR_VAL:
			{
				IScriptContext* otherContext = (IScriptContext*)0;
				switch (stack.top().type())
				{
				case CScriptStack::EContext:
					{
						otherContext = (IScriptContext*)stack.top();
					}
					break;
				default:
					nlwarning("Stack top type invalid, poping top value!");
				}
				stack.pop();
				
				float f;
				if (otherContext)
					f = otherContext->getLogicVar(*((TStringId*)&opcodes[index+1]));
				else
					f = 1.0f;
				
				stack.push(f);
				index += 2;
			}
			continue;
		case	PUSH_CONTEXT_STR_VAR_VAL:
			{
				IScriptContext* otherContext = (IScriptContext*)0;
				switch (stack.top().type())
				{
				case CScriptStack::EContext:
					{
						otherContext = (IScriptContext*)stack.top();
					}
					break;
				default:
					nlwarning("Stack top type invalid, poping top value!");
				}
				stack.pop();
				
				if (otherContext)
					stack.push(otherContext->getStrLogicVar(*((TStringId*)&opcodes[index+1])));
				else
					stack.push(std::string());	//	accepted coz const &
				
				index += 2;
			}
			continue;
		case	PUSH_CONTEXT_CTX_VAR_VAL:
			{
				IScriptContext* otherContext = (IScriptContext*)0;
				switch (stack.top().type())
				{
				case CScriptStack::EContext:
					{
						otherContext = (IScriptContext*)stack.top();
					}
					break;
				default:
					nlwarning("Stack top type invalid, poping top value!");
				}
				stack.pop();
				
				if (otherContext)
					stack.push(otherContext->getCtxLogicVar(*((TStringId*)&opcodes[index+1])));
				else
					stack.push((IScriptContext*)0);
				
				index += 2;
			}
			continue;
		case	JUMP:		//	Jump + nb size_t to jump (relative).	Need: NewJumpOffset After: -
			{
				index+=opcodes[index+1]+1;	//	AGI .. Not Opt
			}
			continue;
		case	JE:			//	Jump if last stack value is FALSE(==0).	Need: BoolValue(float) (NewJumpOffset on  Next Ip) After: -
			{
				if ((float&)stack.top()==0.f)
					index+=opcodes[index+1]+1;	//	AGI .. Not Opt
				else
					index+=2;
				stack.pop();
			}
			continue;
		case	JNE:		//	Jump if last stack value is TRUE(!=0).	Need: BoolValue(float) (NewJumpOffset on  Next Ip) After: -
			{
				if ((float&)stack.top()!=0.f)
					index+=opcodes[index+1]+1;	//	AGI .. Not Opt
				else
					index+=2;
				stack.pop();
			}
			continue;
		case	PUSH_PRINT_STRING:
			{
				currentString+=CStringMapper::unmap(*((TStringId*)&opcodes[index+1]));	//	strPt.substr(1,strPt.size()-2);
				index+=2;
			}
			continue;
		case	PUSH_PRINT_VAR:
			{
				float const val = thisContext->getLogicVar(*((TStringId*)&opcodes[index+1]));
				currentString += NLMISC::toString("%g", val);
				index += 2;
			}
			continue;
		case	PUSH_PRINT_STR_VAR:
			{
				string const str = thisContext->getStrLogicVar(*((TStringId*)&opcodes[index+1]));
				currentString += str;
				index += 2;
			}
			continue;
		case	PRINT_STRING:
			{
				if (AIScriptDisplayPrint)
				{
					nlinfo(currentString.c_str());
				}
				currentString.resize(0);
				++index;
			}
			continue;
		case	LOG_STRING:
			{
				if (AIScriptDisplayLog)
				{
					nlinfo(currentString.c_str());
				}
				currentString.resize(0);
				++index;
			}
			continue;
		case	FUNCTION:
			{
				// on_event
				TStringId const eventName = *((TStringId*)&opcodes[index+1]);
				
				IScriptContext* const sc = stack.top();
				stack.pop();
				if (sc)
					sc->setScriptCallBack(eventName, CByteCodeEntry(byteCode, index+4));
				index+=2;
			}
			continue;
		case	CALL:
			{
				// set_event
				const	TStringId	eventName=*((TStringId*)&opcodes[index+1]);

				IScriptContext* const sc = stack.top();
				stack.pop();
				if (sc)
					sc->callScriptCallBack(thisContext, eventName, 0, "", "", &stack);
				
				index+=2;
			}
			continue;
		case	PUSH_THIS:
			{
				IScriptContext* const	sc=thisContext;
				stack.push(sc);
				index++;
			}
			continue;
		case	PUSH_GROUP:
			{
				const	TStringId	strId=*((TStringId*)&opcodes[index+1]);
				
				IScriptContext* otherContext = NULL;
				if (strId==parentStrId)
				{
					otherContext = parentContext;
				}
				else if (strId==callerStrId)
				{
					otherContext = callerContext;
				}
				else
				{
					otherContext = thisContext->findContext(strId);
				}
				if (!otherContext)
					nlinfo("Group %s unknown, pushing a NULL context on the stack, this may lead to bad behaviours", CStringMapper::unmap(strId).c_str());
				
				stack.push(otherContext);
				index+=2;
			}
			continue;
		case	PUSH_STRING:
			{
				const	string &str = CStringMapper::unmap(*((TStringId*)&opcodes[index+1]));
				stack.push(str);
				index+=2;
			}
			continue;
		case	ASSIGN_FUNC_FROM:
			{
				const	TStringId	srcFunc=CStringMapper::map(stack.top());
				stack.pop();
				
				IScriptContext* const src=stack.top();
				stack.pop();

				const	TStringId	destFunc=CStringMapper::map(stack.top());
				stack.pop();
				
				IScriptContext* const dest=stack.top();
				stack.pop();
				
				if	(	dest
					&&	src	)
				{
					CByteCodeEntry const* pcode = src->getScriptCallBackPtr(srcFunc);
					if	(pcode!=NULL)
						dest->setScriptCallBack(destFunc, *pcode);
				}
				index++;
			}
			continue;
		case	NATIVE_CALL:
			{
				IScriptContext* const sc = stack.top();
				stack.pop();
				string const& funcName = CStringMapper::unmap(*((TStringId*)&opcodes[++index]));
				int mode = (int)opcodes[++index];
				string const& inParamsSig = CStringMapper::unmap(*((TStringId*)&opcodes[++index]));
				string const& outParamsSig = CStringMapper::unmap(*((TStringId*)&opcodes[++index]));
				if (sc)
				{
					sc->callNativeCallBack(thisContext, funcName, mode, inParamsSig, outParamsSig, &stack);
				}
				else
				{
					nlwarning("Calling a native function (%s) on a NULL group/context, rebuilding stack (this situation previously led to unknown behaviour, most of the time crashes)", funcName.c_str());
					// If we have parameters we got to rebuild the stack
					if (!inParamsSig.empty())
					{
						for (size_t i=0; i<inParamsSig.length(); ++i)
							stack.pop();
					}
					if (!outParamsSig.empty())
					{
						for (size_t i=0; i<outParamsSig.length(); ++i)
						{
							switch (outParamsSig[i])
							{
							case 'f': stack.push(0.f); break;
							case 's': stack.push(string()); break;
							case 'c': stack.push((IScriptContext*)0); break;
							default: nlassert("Unknown parameter type in native function call while rebuilding stack");
							}
						}
					}
				}
				
				++index;
			}
			continue;
		case	RAND:
			{
				const	size_t	randIndex=rand32((uint32)opcodes[index+1]); // rand(RANDCOUNT)
				index+=3;	//	pass RAND + RANDCOUNT + 1
				
				stack.push((int)(index+opcodes[index]));	//	push the absolute address for RET.
				
				index+=(randIndex+1)*2;
				index+=opcodes[index];	//	we jump at the random sequence.
			}
			continue;
		case	RET:
			{
				index=(int&)stack.top();
				stack.pop();
			}
			continue;
		case	ONCHILDREN:
			{
				if (thisContext)
				{
					thisContext->interpretCodeOnChildren(CByteCodeEntry(byteCode, index+3));
				}
				index++;	//	let's go to jump ..
			}
			continue;
		case	SWITCH:
			{
			//	!!!!!
				size_t	compValue=0;
				switch (stack.top().type())
				{
				case CScriptStack::EString:
					{
						TStringId strId = CStringMapper::map(stack.top());
						stack.pop();
						compValue = *((size_t*)&strId);
					}
					break;
				case CScriptStack::EFloat:
					{
						float val = (float&)stack.top();
						stack.pop();
						compValue = *((size_t*)&val);
					}
					break;
				default:
					nlwarning("Stack top type invalid, poping top value");
					stack.pop();
				}
				
				size_t	nbCase=opcodes[index+1];
				index+=2;	//	SWITCH + #Case
				stack.push((int)(index+opcodes[index]));	//	push the absolute address for RET.
				index++;
				size_t	offset=index;
				bool	found=false;
				while (nbCase>0)
				{
					if (compValue==opcodes[offset])	//	we could replace this with binary retrieval.
					{
						index=offset+1;
						index+=opcodes[index];	//	we jump at the random sequence.
						found=true;
						break;
					}
					nbCase--;
					offset+=2;
				}
				//	if not found, don't do anything.
				if (!found)
				{
					index=(int&)stack.top();
					stack.pop();
				}

			}
			continue;
		case	INCR:		//	Increment top of stack.
			{
				float &f = stack.top();
				++f;
				++index;
			}
			continue;
		case	DECR:		//	Decrement top of stack.
			{
				float &f = stack.top();
				--f;
				++index;
			}
			continue;
		case	CONCAT:		//	Concatenates 2 strings
			{
				(string&)stack.top(1) += (string&)stack.top();
				stack.pop();
			}
			continue;
		case	FTOS:		//	Convert a float to a string
			{
				stack.top()=NLMISC::toString("%g", (float&)stack.top());
			}
			continue;
		}
		nlassert(false);	//	must use continue !!	Not implemented.
	}
}

}
