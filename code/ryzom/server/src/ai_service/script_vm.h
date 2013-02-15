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

#ifndef __SCRIPT_VM__
#define __SCRIPT_VM__

#include "nel/misc/smart_ptr.h"
//#include "ai_grp.h"

#include <limits>

namespace AIVM
{

/****************************************************************************/
/* Compiler classes                                                         */
/****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
// Helper classes                                                           //
//////////////////////////////////////////////////////////////////////////////

/// A helper class seperating a string in substrings seperated by a pattern.
/** Be carefull, this objet keeps a reference to the string.
*/
class CStringSeparator
{
public:
	CStringSeparator(std::string const& str, std::string const& pattern);
	virtual ~CStringSeparator();
	void reset() const;
	std::string get() const;
	bool hasNext() const;
protected:
	void calcNext() const;
private:
	mutable size_t _Index;
	mutable size_t _NewIndex;
	mutable size_t _Delta;
	mutable size_t _StartIndex;
	std::string const& _Str;
	std::string const _Motif;
};

/****************************************************************************/
/* Script Virtual Machine classes                                           */
/****************************************************************************/

//////////////////////////////////////////////////////////////////////////////
// ByteCode interpretation                                                 //
//////////////////////////////////////////////////////////////////////////////

class CByteCode : public NLMISC::CRefCount
{
public:
	CByteCode(std::string const& sourceName);
	std::vector<size_t> _opcodes;
	std::string const _sourceName;
};

class CByteCodeEntry {
public:
	CByteCodeEntry()
	: _Index(std::numeric_limits<size_t>::max())
	{
	}
	CByteCodeEntry(NLMISC::CSmartPtr<CByteCode const> code, size_t index)
	: _Code(code)
	, _Index(index)
	{
	}
	bool isValid()const
	{
		return _Index != std::numeric_limits<size_t>::max();
	}
	NLMISC::CSmartPtr<CByteCode const>& code() { return _Code; }
	size_t& index() { return _Index; }
	NLMISC::CSmartPtr<CByteCode const> const& code() const { return _Code; }
	size_t const& index() const { return _Index; }
private:
	NLMISC::CSmartPtr<CByteCode const> _Code;
	size_t _Index;
};

//////////////////////////////////////////////////////////////////////////////

class IScriptContext;

class CScriptStack
{
public:
	enum TStackTypes { ENone, EOther, EString, EFloat, EContext };
	
	class CStackEntry
	{
	public:
		CStackEntry();
		~CStackEntry();
		
		void clean();
		
		CStackEntry& operator=(float const& f);
		CStackEntry& operator=(int const& i);
		CStackEntry& operator=(std::string const& str);
		CStackEntry& operator=(IScriptContext* si);
		
		bool operator==(CStackEntry const& other) const;
		bool operator!=(CStackEntry const& other) const;
		bool operator<=(CStackEntry const& other) const;
		bool operator>=(CStackEntry const& other) const;
		bool operator<(CStackEntry const& other) const;
		bool operator>(CStackEntry const& other) const;
		
		operator float&();
		operator int&();
		operator std::string&();
		operator IScriptContext*();
		TStackTypes type() const;
		
	protected:
		std::string& getString();
		std::string const& getString() const;
		IScriptContext* getIScriptContext();
		int& getInt();
		int const& getInt() const;
		float& getFloat();
		float const& getFloat() const;
		
		int	_val;
		TStackTypes	_type;
	};
	
	void push(float const& val);
	void push(std::string const& val);
	void push(IScriptContext* val);
	void push(int val);
	
	CStackEntry& top();	//	is this an optimisation of the method below ?
	CStackEntry& top(int index);
	
	void pop();
	
#if	!FINAL_VERSION
	size_t size()
	{
		return _Stack.size();
	}
#endif
	
	private:
		std::deque<CStackEntry>	_Stack;
};

//////////////////////////////////////////////////////////////////////////////

class IScriptContext
{
public:
	virtual std::string getContextName() = 0;
	virtual void interpretCodeOnChildren(CByteCodeEntry const& codeScriptEntry) = 0;
	
	virtual float getLogicVar(NLMISC::TStringId varId) = 0;
	virtual void setLogicVar(NLMISC::TStringId varId, float value) = 0;
	virtual std::string getStrLogicVar(NLMISC::TStringId varId) = 0;
	virtual void setStrLogicVar(NLMISC::TStringId varId, std::string const& value) = 0;
	virtual IScriptContext* getCtxLogicVar(NLMISC::TStringId varId) = 0;
	virtual void setCtxLogicVar(NLMISC::TStringId varId, IScriptContext* value) = 0;
	
	virtual IScriptContext* findContext(NLMISC::TStringId const strId) = 0;
	
	virtual void setScriptCallBack(NLMISC::TStringId const& eventName, CByteCodeEntry const& codeScriptEntry) = 0;
	virtual CByteCodeEntry const* getScriptCallBackPtr(NLMISC::TStringId const& eventName) const = 0;
	virtual void callScriptCallBack(IScriptContext* caller, NLMISC::TStringId const& funcName, int mode = 0, std::string const& inParamsSig = "", std::string const& outParamsSig = "", CScriptStack* stack = NULL) = 0;
	virtual void callNativeCallBack(IScriptContext* caller, std::string const&       funcName, int mode = 0, std::string const& inParamsSig = "", std::string const& outParamsSig = "", CScriptStack* stack = NULL) = 0;
};

class CScriptVM
{
public:
	//	Operations behave on stack elems.
	enum EOpcode
	{
/*00*/		INVALID_OPCODE=0,
/*01*/		EOP,	//	End Of Program
		//	Need: - After: -
		//	Need: FuncName, NbParams, Params .. After: ReturnValue (1)
/*02*/		EQ,							// ==																				StackBef: Value1,Value2		StackAft: Value1==Value2 (Boolean as float)
/*03*/		NEQ,						// !=																				StackBef: Value1,Value2		StackAft: Value1!=Value2 (Boolean as float)
/*04*/		INF,						// <																				StackBef: Value1,Value2		StackAft: Value1<Value2 (Boolean as float)
/*05*/		INFEQ,						// <=																				StackBef: Value1,Value2		StackAft: Value1<=Value2 (Boolean as float)
/*06*/		SUP,						// >																				StackBef: Value1,Value2		StackAft: Value1>Value2 (Boolean as float)
/*07*/		SUPEQ,						// >=																				StackBef: Value1,Value2		StackAft: Value1>=Value2 (Boolean as float)
/*08*/		ADD,						// +																				StackBef: Value1,Value2		StackAft: Value1+Value2
/*09*/		SUB,						// -																				StackBef: Value1,Value2		StackAft: Value1-Value2
/*0a*/		MUL,						// *																				StackBef: Value1,Value2		StackAft: Value1/Value2
/*0b*/		DIV,						// /																				StackBef: Value1,Value2		StackAft: Value1/Value2	!Exception Gestion.
/*0c*/		AND,						// &&																				StackBef: Value1,Value2		StackAft: Value1&&Value2
/*0d*/		OR,							// ||																				StackBef: Value1,Value2		StackAft: Value1||Value2
/*0e*/		NOT,						// !																				StackBef: Value				StackAft: !Value
/*0f*/		PUSH_ON_STACK,				// Set a Value (Float,TStringId .. etc)												StackBef: -					StackAft: Value
/*10*/		POP,						// Pop																				StackBef: Value				StackAft: -
/*11*/		SET_VAR_VAL,				// Set a value to a float variable.							Code: VarName			StackBef: VarValue			StackAft: -
/*12*/		SET_STR_VAR_VAL,			// Set a value to a string variable.						Code: VarName			StackBef: VarValue			StackAft: -
/*13*/		SET_CTX_VAR_VAL,			// Set a value to a context variable.						Code: VarName			StackBef: VarValue			StackAft: -
/*14*/		PUSH_VAR_VAL,				// Push the value of a float variable.						Code: VarName			StackBef: -					StackAft: VarValue
/*15*/		PUSH_STR_VAR_VAL,			// Push the value of a string variable.						Code: VarName			StackBef: -					StackAft: VarValue
/*16*/		PUSH_CTX_VAR_VAL,			// Push the value of a context variable.					Code: VarName			StackBef: -					StackAft: VarValue
/*17*/	//	SET_OTHER_VAR_VAL,			// Set a value to a float variable in another group.		Code: GroupName,VarName	StackBef: VarValue			StackAft: -
/*18*/	//	SET_OTHER_STR_VAR_VAL,		// Set a value to a string variable in another group.		Code: GroupName,VarName	StackBef: VarValue			StackAft: -
/*19*/	//	SET_OTHER_CTX_VAR_VAL,		// Set a value to a context variable in another group.		Code: GroupName,VarName	StackBef: VarValue			StackAft: -
/*1a*/	//	PUSH_OTHER_VAR_VAL,			// Push the value of a float variable of another group.		Code: GroupName,VarName	StackBef: -					StackAft: VarValue
/*1b*/	//	PUSH_OTHER_STR_VAR_VAL,		// Push the value of a string variable of another group.	Code: GroupName,VarName	StackBef: -					StackAft: VarValue
/*1c*/	//	PUSH_OTHER_CTX_VAR_VAL,		// Push the value of a context variable of another group.	Code: GroupName,VarName	StackBef: -					StackAft: VarValue
/*1d*/		SET_CONTEXT_VAR_VAL,		// Set a value to a float variable in another group.		Code: VarName			StackBef: VarValue,Group	StackAft: -
/*1e*/		SET_CONTEXT_STR_VAR_VAL,	// Set a value to a float variable in another group.		Code: VarName			StackBef: VarValue,Group	StackAft: -
/*1f*/		SET_CONTEXT_CTX_VAR_VAL,	// Set a value to a float variable in another group.		Code: VarName			StackBef: VarValue,Group	StackAft: -
/*20*/		PUSH_CONTEXT_VAR_VAL,		// Push the value of a float variable of another group.		Code: VarName			StackBef: Group				StackAft: VarValue
/*21*/		PUSH_CONTEXT_STR_VAR_VAL,	// Push the value of a float variable of another group.		Code: VarName			StackBef: Group				StackAft: VarValue
/*22*/		PUSH_CONTEXT_CTX_VAR_VAL,	// Push the value of a float variable of another group.		Code: VarName			StackBef: Group				StackAft: VarValue
/*23*/		JUMP,						// Jump + nb size_t to jump (relative).						Code: JumpOffset		StackBef: -					StackAft: -			//< May be innaccurate
/*24*/		JE,							// Jump if last stack value is FALSE(==0).					Code: JumpOffset		StackBef: Bool(float)		StackAft: -			//< May be innaccurate
/*25*/		JNE,						// Jump if last stack value is TRUE(==1).					Code: JumpOffset		StackBef: Bool(float)		StackAft: -			//< May be innaccurate
/*26*/		PUSH_PRINT_STRING,
/*27*/		PUSH_PRINT_VAR,
/*28*/		PUSH_PRINT_STR_VAR,
/*29*/		PRINT_STRING,
/*2a*/		LOG_STRING,
/*2b*/		FUNCTION,
/*2c*/		CALL,
/*2d*/		PUSH_THIS,
/*2e*/		PUSH_GROUP,
/*2f*/		PUSH_STRING,
/*30*/		ASSIGN_FUNC_FROM,
/*31*/		NATIVE_CALL,
/*32*/		RAND,
/*33*/		RANDEND,
/*34*/		RET,						// Get the value on the pile to retrieve the good IP (absolute).
/*35*/		ONCHILDREN,
/*36*/		SWITCH,
/*37*/		INCR,
/*38*/		DECR,
/*39*/		CONCAT,
/*3a*/		FTOS,
	};
	
public:
	void interpretCode(
		IScriptContext* thisContext,
		IScriptContext* parentContext,
		IScriptContext* callerContext,
		CByteCodeEntry const& codeScriptEntry);
	static CScriptVM* getInstance();
	static void destroyInstance();
private:
	uint32 rand32(uint32 mod)
	{ 
		if (mod==0) return 0;
		return ((((uint32)_Random.rand())<<16) + (uint32)_Random.rand()) % mod;
	}
	static CScriptVM* _Instance;
	NLMISC::CRandom _Random;
};

//////////////////////////////////////////////////////////////////////////////

class CLibrary
{
public:
	static CLibrary& getInstance();
	static void destroyInstance();
	
	void addLib(std::string const& name, std::string const& code);
	void addLib(std::string const& name, std::vector<std::string> const& code);
	void addLib(std::string const& name, NLMISC::CSmartPtr<CByteCode const> const& byteCode);
	NLMISC::CSmartPtr<CByteCode const> getLib(std::string const& name);
	
protected:
	CLibrary() { }
	
private:
	typedef std::map<std::string, NLMISC::CSmartPtr<CByteCode const> > TLibContainer;
	static CLibrary* _instance;
	static TLibContainer _libs;
};

/****************************************************************************/
/* Inlined functions                                                        */
/****************************************************************************/

inline
CByteCode::CByteCode(std::string const& sourceName)
: _sourceName(sourceName)
{
}

inline
CLibrary& CLibrary::getInstance()
{
	if (_instance==NULL)
		_instance = new CLibrary();
	return *_instance;
}

inline
void CLibrary::destroyInstance()
{
	if (_instance!=NULL)
	{
		delete _instance;
		_instance = NULL;
	}
}

inline
CScriptStack::CStackEntry::CStackEntry()
: _type(ENone)
{
}

inline
CScriptStack::CStackEntry::~CStackEntry()
{
	clean();
}

inline
void CScriptStack::CStackEntry::clean()
{
	if (_type==EString)
		delete &getString();
}

inline
CScriptStack::CStackEntry& CScriptStack::CStackEntry::operator=(float const& f)
{
	clean();
	_val = *((int*)&f);
	_type = EFloat;
	return *this;
}
inline
CScriptStack::CStackEntry& CScriptStack::CStackEntry::operator=(int const& i)
{
	clean();
	_val = i;
	_type = EOther;
	return *this;
}
inline
CScriptStack::CStackEntry& CScriptStack::CStackEntry::operator=(std::string const& str)
{
	clean();
	std::string* const strPt = new std::string(str);
	_val = *((int*)&strPt);
	_type = EString;
	return *this;
}
inline
CScriptStack::CStackEntry& CScriptStack::CStackEntry::operator=(IScriptContext* sc)
{
	clean();
	_val = *((int*)&sc);
	_type = EContext;
	return *this;
}

inline
bool CScriptStack::CStackEntry::operator==(CStackEntry const& other) const
{
	nlassert(_type==other._type);
	switch (_type)
	{
	case EString:
		return getString()==other.getString();
	case EFloat:
		return getFloat()==other.getFloat();
	case EOther:
	default:
		return _val==other._val;
	}
}

inline
bool CScriptStack::CStackEntry::operator!=(CStackEntry const& other) const
{
	return !(*this == other);
}

inline
bool CScriptStack::CStackEntry::operator<=(CStackEntry const& other) const
{
	return !(*this > other);
}

inline
bool CScriptStack::CStackEntry::operator>=(CStackEntry const& other) const
{
	return !(*this < other);
}

inline
bool CScriptStack::CStackEntry::operator<(CStackEntry const& other) const
{
	nlassert(_type==other._type);
	switch (_type)
	{
	case EString:
		return getString()<other.getString();
	case EFloat:
		return getFloat()<other.getFloat();
	case EOther:
	default:
		return _val<other._val;
	}
}

inline
bool CScriptStack::CStackEntry::operator>(CStackEntry const& other) const
{
	nlassert(_type==other._type);
	switch (_type)
	{
	case EString:
		return getString()>other.getString();
	case EFloat:
		return getFloat()>other.getFloat();
	case EOther:
	default:
		return _val>other._val;
	}
}

inline
CScriptStack::CStackEntry::operator float&()
{
	return getFloat();
}
inline
CScriptStack::CStackEntry::operator int&()
{
	return getInt();
}
inline
CScriptStack::CStackEntry::operator std::string&()
{
	return getString();
}
inline
CScriptStack::CStackEntry::operator IScriptContext*()
{
	return getIScriptContext();
}

inline
CScriptStack::TStackTypes CScriptStack::CStackEntry::type() const
{
	return _type;
}

inline
std::string& CScriptStack::CStackEntry::getString()
{
	nlassert(_type==EString);
	return *(*((std::string**)&_val));
}
inline
std::string const& CScriptStack::CStackEntry::getString() const
{
	nlassert(_type==EString);
	return *(*((std::string**)&_val));
}
inline
IScriptContext* CScriptStack::CStackEntry::getIScriptContext()
{
	nlassert(_type==EContext);
	return *((IScriptContext**)&_val);
}
inline
int& CScriptStack::CStackEntry::getInt()
{
	nlassert(_type==EOther);
	return _val;
}
inline
int const& CScriptStack::CStackEntry::getInt() const
{
	nlassert(_type==EOther);
	return _val;
}
inline
float& CScriptStack::CStackEntry::getFloat()
{
	nlassert(_type==EFloat);
	return *((float*)&_val);
}
inline
float const& CScriptStack::CStackEntry::getFloat() const
{
	nlassert(_type==EFloat);
	return *((float const*)&_val);
}

inline
void CScriptStack::push(float const& val)
{
	_Stack.push_back(CStackEntry());
	_Stack.back() = val;
}
inline
void CScriptStack::push(std::string const& val)
{
	_Stack.push_back(CStackEntry());
	_Stack.back() = val;
}
inline
void CScriptStack::push(IScriptContext* val)
{
	_Stack.push_back(CStackEntry());
	_Stack.back() = val;
}
inline
void CScriptStack::push(int val)
{
	_Stack.push_back(CStackEntry());
	_Stack.back() = val;
}

inline
CScriptStack::CStackEntry& CScriptStack::top()	//	is this an optimisation of the method below ?
{
	return _Stack.back();
}
inline
CScriptStack::CStackEntry& CScriptStack::top(int index)
{
	return *(_Stack.rbegin()+index);
}

inline
void CScriptStack::pop()
{
	_Stack.pop_back();
}

inline
CStringSeparator::CStringSeparator(const std::string &str, const std::string &motif)
	:_Index(0)
	,_NewIndex(0)
	,_Delta(0)
	,_StartIndex(0)			
	,_Str(str)
	,_Motif(motif)
{
}
inline
CStringSeparator::~CStringSeparator()
{
}
inline
void CStringSeparator::reset() const
{
	_Index=0;
	_NewIndex=0;
	_Delta=0;
	_StartIndex=0;					
}
inline
std::string	CStringSeparator::get() const
{
#if !FINAL_VERSION
	nlassert(_Delta>0);
#endif
	return _Str.substr(_StartIndex, _Delta);
}
inline
bool CStringSeparator::hasNext() const
{
	calcNext();
	return	_Delta>0;
}
inline
void CStringSeparator::calcNext() const
{
	_Delta = 0;
	while (	_Index!=std::string::npos
		&&	_Delta==0)
	{
		_NewIndex = _Str.find_first_of(_Motif, _Index);
		_StartIndex = _Index;
		_Delta = ((_NewIndex==std::string::npos)?_Str.size():_NewIndex)-_Index;
		if (_NewIndex==std::string::npos)
		{
			_Index = std::string::npos;
			break;
		}
		_Index = _Str.find_first_not_of(_Motif, _NewIndex);
	}
}

} // namespace

#endif
