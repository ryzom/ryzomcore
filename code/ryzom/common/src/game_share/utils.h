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


#ifndef UTILS_H
#define UTILS_H


//-------------------------------------------------------------------------------------------------
// includes
//-------------------------------------------------------------------------------------------------

#include "nel/misc/types_nl.h"
#include "nel/misc/common.h"
#include "nel/misc/debug.h"
#include "nel/misc/sstring.h"

#include <list>

//-------------------------------------------------------------------------------------------------
// UTILITY FUNCTIONS
//-------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
inline std::string capitalize(const std::string & s)
{
	if ( s.empty() )
		return s;

	return NLMISC::toUpper( s.substr(0,1) ) + NLMISC::toLower( s.substr(1,std::string::npos) );
}

inline ucstring capitalize(const ucstring & s)
{
	if ( s.empty() )
		return s;

	return NLMISC::toUpper( s.substr(0,1) ) + NLMISC::toLower( s.substr(1,std::string::npos) );
}


//-------------------------------------------------------------------------------------------------
// HANDY MACROS - For forcing the pre-preprocessor to evaluate concatenation operations nicely
//-------------------------------------------------------------------------------------------------

// a Few examples:
// ---------------
//
//	#define TOTO TATA
//	MACRO_CONCAT(a,TOTO)		=> aTOTO
//	MACRO_CONCAT2(a,TOTO)		=> aTATA
//	MACRO_TOTXT(TOTO)			=> "TOTO"
//	MACRO_TOTXT2(TOTO)			=> "TATA"
//
///	MACRO_CONCAT(a,__LINE__)	=> a__LINE__
//	MACRO_CONCAT2(a,__LINE__)	=> a123
//	MACRO_TOTXT(__LINE__)		=> "__LINE__"
//	MACRO_TOTXT2(__LINE__)		=> "123"
//
//	__FILE_LINE__				=> "utils.h:123:"

#define MACRO_CONCAT(a,b) a##b
#define MACRO_CONCAT2(a,b) CONCAT(a,b)
#define MACRO_TOTXT(a) #a
#define MACRO_TOTXT2(a) TOTXT(a)
#define __FILE_LINE__ __FILE__ ":"TOTXT2(__LINE__)":"


//-------------------------------------------------------------------------------------------------
// LOGGING / DEBUGGING MACROS
//-------------------------------------------------------------------------------------------------

#ifdef NL_DEBUG
	#define DEBUG_STOP nlstop;
	#define nlassertd(a) nlassert(a)
#else
	#define DEBUG_STOP \
		std::string stack; \
		NLMISC::getCallStack(stack); \
		std::vector<std::string> contexts;\
		NLMISC::explode(stack, std::string("\n"), contexts);\
		nldebug("Dumping callstack :"); \
		for (uint i=0; i<contexts.size(); ++i) \
			nldebug("  %3u : %s", i, contexts[i].c_str());
	#define	nlassertd(a) if (0) { } else { }
#endif

// the following set of definess can be undefined and re-defined to add user code to execute
// systematically when GIVEUP, DROP or BOMB macros are triggered
// eg #define ON_BOMB logError(__FILE__,__LINE__,msg);
#define ON_GIVEUP
#define ON_DROP
#define ON_BOMB

// info and warning macros
#define INFO(msg) nlinfo((NLMISC::CSString()<<msg).c_str());
#define WARN(msg) nlwarning((NLMISC::CSString()<<msg).c_str());

// unconditional abort macros
#define STOP(msg) { WARN(msg) DEBUG_STOP }
#define GIVEUP(msg,action)	{ INFO(msg) { ON_GIVEUP; } { action; } }
#define DROP(msg,action)	{ WARN(msg) { ON_DROP;   } { action; } }
#define BOMB(msg,action)	{ STOP(msg)	{ ON_BOMB;   } { action; } }

// conditional warn and abort macros
#define GIVEUP_IF(condition,msg,action)		if (!(condition));else GIVEUP(msg,action)
#define WARN_IF(condition,msg)				if (!(condition));else WARN(msg)
#define DROP_IF(condition,msg,action)		if (!(condition));else DROP(msg,action)
#define BOMB_IF(condition,msg,action)		if (!(condition));else BOMB(msg,action)
#define STOP_IF(condition,msg)				if (!(condition));else STOP(msg)

// testing for variable value changes
#define ON_CHANGE(type,var,code)\
	class __COnChange##var\
	{\
	public:\
		const type& _Var;\
		const type _OldVal;\
		__COnChange##var(const type& var): _Var(var), _OldVal(var) {}\
		~__COnChange##var() { if(_OldVal!=_Var) { code; } }\
	}\
	__onChange##__LINE__(var);

#define ON_CHANGE_ASSERT(type,var) ON_CHANGE(type,var,nlerror("Variable "#var" changed from %s to %s",NLMISC::toString(_OldVal).c_str(),NLMISC::toString(_Var).c_str()))


//-------------------------------------------------------------------------------------------------
// A handy 'nldebug', 'nlinfo' & 'nlwarning' override system
//-------------------------------------------------------------------------------------------------
//
// The system includes a set of object classes
// To override one or more of the standard NeL log channels one simply instantiates the appropriate class
// with the new log channel as a parameter.
// The log channel in question will revert to its previous value on destruction of the override object
//
// Usage Example:
//
//	void doSomething()
//	{
//		nlinfo("bla");
//		nlwarning("bla");
//	}
//
//	NLMISC_COMMAND(bla,"bla","bla")
//	{
//		CNLLogOverride(&log);
//		doSomething();
//		return true;
//	}
//

//-------------------------------------------------------------------------------------------------

//class CNLDebugOverride
//{
//public:
//	CNLDebugOverride(NLMISC::CLog *debugLog)
//	{
//		nlassert(debugLog!=NULL);
//		_OldValue=NLMISC::DebugLog;
//		nlassert(_OldValue!=NULL);
//		NLMISC::INelContext::getInstance().setDebugLog(debugLog);
//	}
//	~CNLDebugOverride()
//	{
//		NLMISC::INelContext::getInstance().setDebugLog(_OldValue);
//	}
//private:
//	// prohibit copy
//	CNLDebugOverride(const CNLDebugOverride&);
//	NLMISC::CLog *_OldValue;
//};
//
////-------------------------------------------------------------------------------------------------
//
//class CNLInfoOverride
//{
//public:
//	CNLInfoOverride(NLMISC::CLog *infoLog)
//	{
//		nlassert(infoLog!=NULL);
//		_OldValue=NLMISC::InfoLog;
//		nlassert(_OldValue!=NULL);
//		NLMISC::INelContext::getInstance().setInfoLog(infoLog);
//	}
//	~CNLInfoOverride()
//	{
//		NLMISC::INelContext::getInstance().setInfoLog(_OldValue);
//	}
//private:
//	// prohibit copy
//	CNLInfoOverride(const CNLInfoOverride&);
//	NLMISC::CLog *_OldValue;
//};
//
////-------------------------------------------------------------------------------------------------
//
//class CNLWarningOverride
//{
//public:
//	CNLWarningOverride(NLMISC::CLog *warningLog)
//	{
//		nlassert(warningLog!=NULL);
//		_OldValue=NLMISC::WarningLog;
//		nlassert(_OldValue!=NULL);
//		NLMISC::INelContext::getInstance().setWarningLog(warningLog);
//	}
//	~CNLWarningOverride()
//	{
//		NLMISC::INelContext::getInstance().setWarningLog(_OldValue);
//	}
//private:
//	// prohibit copy
//	CNLWarningOverride(const CNLWarningOverride&);
//	NLMISC::CLog *_OldValue;
//};
//
////-------------------------------------------------------------------------------------------------
//
//class CNLLogOverride
//{
//public:
//	CNLLogOverride(NLMISC::CLog *commonLog): _DebugLog(commonLog), _InfoLog(commonLog), _WarningLog(commonLog)	{}
//
//private:
//	CNLDebugOverride	_DebugLog;
//	CNLInfoOverride		_InfoLog;
//	CNLWarningOverride	_WarningLog;
//};
//
//
////-------------------------------------------------------------------------------------------------
//
//class CNLSmartLogOverride
//{
//public:
//	CNLSmartLogOverride(NLMISC::CLog *commonLog):
//		_DebugLog(commonLog==NLMISC::InfoLog?NLMISC::DebugLog:commonLog),
//		_InfoLog(commonLog),
//		_WarningLog(commonLog==NLMISC::InfoLog?NLMISC::WarningLog:commonLog)
//		{}
//
//private:
//	CNLDebugOverride	_DebugLog;
//	CNLInfoOverride		_InfoLog;
//	CNLWarningOverride	_WarningLog;
//};


//-------------------------------------------------------------------------------------------------
// A Little CallStack system
//-------------------------------------------------------------------------------------------------
// example:
// void traceTest2(int i)
//	{
//		CSTRACE;
//		if (i<2)
//			traceTest2(++i);
//		else
//			SHOW_CALLSTACK;
//	}
//
//	void traceTest()
//	{
//		CSTRACE_MSG("foo");
//		traceTest2(0);
//		{
//			int i=0;
//			CSTRACE_VAR(int,i);
//			CSTRACE_VAL(int,i);
//			i=1;
//			SHOW_CALLSTACK;
//		}
//		WARN_CALLSTACK;
//	}
//
// output:
//	INF 3980: >>test.cpp:21: Call Stack:
//	INF 3980: >>test.cpp:17
//	INF 3980: >>test.cpp:17
//	INF 3980: >>test.cpp:17
//	INF 3980: >>test.cpp:26: foo
//	INF 3980:
//	INF 3980: >>test.cpp:33: Call Stack:
//	INF 3980: >>test.cpp:31: i=[0]
//	INF 3980: >>test.cpp:30: i=>[1]
//	INF 3980: >>test.cpp:26: foo
//	INF 3980:
//	WRN 3980: >>test.cpp:35: Call Stack:
//	WRN 3980: >>test.cpp:26: foo
//	WRN 3980:


//-------------------------------------------------------------------------------------------------
// A Little CallStack system - MACROS
//-------------------------------------------------------------------------------------------------
// CSTRACE					- displays source file and line number
// CSTRACE_MSG(msg)			- as with trace but with additional simple text message	eg TRACE_MSG("hello world")
// CSTRACE_VAL(type,name)	- displays a value (calculated at the moment that the trace is created)
// CSTRACE_VAR(type,name)	- displays the value of the given variable at the moment that callstack is displayed
// SHOW_CALLSTACK			- displays the callstack using NLMISC::InfoLog
// WARN_CALLSTACK			- displays the callstack using NLMISC::WarningLog

#define CSTRACE\
	class __CCallStackEntry##__LINE__: public ICallStackEntry\
	{\
	public:\
		virtual void displayEntry(NLMISC::CLog& log) const\
		{\
			log.displayNL(">>"__FILE__":%d",__LINE__);\
		}\
	}\
	__callStackEntry##__LINE__;

#define CSTRACE_MSG(msg)\
	class __CCallStackEntry##__LINE__: public ICallStackEntry\
	{\
	public:\
		virtual void displayEntry(NLMISC::CLog& log) const\
		{\
			log.displayNL(">>"__FILE__":%d: %s",__LINE__,msg);\
		}\
	}\
	__callStackEntry##__LINE__;

#define CSTRACE_VAL(type,var)\
	class __TraceVal_##var: public ICallStackEntry\
	{\
	public:\
		__TraceVal_##var(const type& var): _Val(var) \
		{\
		}\
		virtual void displayEntry(NLMISC::CLog& log) const\
		{\
			log.displayNL(">>"__FILE__":%d: %s=[%s]",__LINE__,#var,NLMISC::toString(_Val).c_str());\
		}\
		const type _Val;\
	}\
	__traceVal_##var(var);

#define CSTRACE_VAR(type,var)\
	class __TraceVar_##var: public ICallStackEntry\
	{\
	public:\
		__TraceVar_##var(const type& var): _Var(var) \
		{\
		}\
		virtual void displayEntry(NLMISC::CLog& log) const\
		{\
			log.displayNL(">>"__FILE__":%d: %s=>[%s]",__LINE__,#var,NLMISC::toString(_Var).c_str());\
		}\
		const type& _Var;\
	}\
	__traceVar_##var(var);

#define SHOW_CALLSTACK { CSTRACE_MSG("Call Stack:"); CCallStackSingleton::display(NLMISC::InfoLog); }
#define WARN_CALLSTACK { CSTRACE_MSG("Call Stack:"); CCallStackSingleton::display(NLMISC::WarningLog); }


//-------------------------------------------------------------------------------------------------
// A Little CallStack system - Private stack entry base class
//-------------------------------------------------------------------------------------------------

class ICallStackEntry
{
public:
	ICallStackEntry();
	virtual ~ICallStackEntry();
	void displayStack(NLMISC::CLog& log) const;

	virtual void displayEntry(NLMISC::CLog& log) const=0;

private:
	ICallStackEntry* _Next;
};


//-------------------------------------------------------------------------------------------------
// A Little CallStack system - Public Singleton Class
//-------------------------------------------------------------------------------------------------

class CCallStackSingleton
{
public:
	static ICallStackEntry* getTopStackEntry();
	static void setTopStackEntry(ICallStackEntry* newEntry);
	static void display(NLMISC::CLog *log=NLMISC::InfoLog);

private:
	// this is a singleton so prohibit public construction
	CCallStackSingleton() {}

	// encapsulation of a variable to make it a singleton
	static ICallStackEntry*& topStackEntry();
};


//-------------------------------------------------------------------------------------------------
// A Little CallStack system - Public Singleton inlines
//-------------------------------------------------------------------------------------------------

inline ICallStackEntry* CCallStackSingleton::getTopStackEntry()
{
	return topStackEntry();
}

inline void CCallStackSingleton::setTopStackEntry(ICallStackEntry* newEntry)
{
	topStackEntry()= newEntry;
}

inline void CCallStackSingleton::display(NLMISC::CLog *log)
{
	nlassert(log!=NULL);
	getTopStackEntry()->displayStack(*log);
	log->displayNL("");
}

inline ICallStackEntry*& CCallStackSingleton::topStackEntry()
{
	static ICallStackEntry* stackEntry=NULL;
	return stackEntry;
}


//-------------------------------------------------------------------------------------------------
// A Little CallStack system - Private stack entry base inlines
//-------------------------------------------------------------------------------------------------

inline ICallStackEntry::ICallStackEntry()
{
	// add self to the call stack
	_Next=CCallStackSingleton::getTopStackEntry();
	CCallStackSingleton::setTopStackEntry(this);
}

inline ICallStackEntry::~ICallStackEntry()
{
	// if this object is in the call stack then pop items off the top of the stack
	// until this object is no longer in the stack
	while (_Next!=this)
	{
		// get a pointer to the top object on the call stack
		ICallStackEntry* entry= CCallStackSingleton::getTopStackEntry();
		nlassertd(entry!=NULL);

		// pop the object off the callstack
		CCallStackSingleton::setTopStackEntry(entry->_Next);

		// mark object as no longer in callstack
		entry->_Next=entry;
	}
}

inline void ICallStackEntry::displayStack(NLMISC::CLog& log) const
{
	// stop recursing when we reach a NULL object
	// (this is implemented in this way in order to ximplify call code)
	if (this==NULL)
		return;

	// display this entry
	displayEntry(log);

	// recurse through call stack
	_Next->displayStack(log);
}


//-------------------------------------------------------------------------------------------------
// HANDY Utility methods...
//-------------------------------------------------------------------------------------------------

inline NLMISC::CVectorSString& operator<<(NLMISC::CVectorSString& vect,const NLMISC::CSString s)
{
	vect.push_back(s);
	return vect;
}


template<class T> inline T& vectAppend(std::vector<T>& vect)
{
	vect.resize(vect.size()+1);
	return vect.back();
}
template<class T0,class T1> inline void vectInsert(std::vector<T0>& vect,const T1& value)
{
	for (uint32 i=0;i<vect.size();++i)
		if (vect[i]== value)
			return;

	vect.push_back(value);
}

template<class T> inline T& listAppend(std::list<T>& list)
{
	list.resize(list.size()+1);
	return list.back();
}

inline NLMISC::CSString popString(NLMISC::IStream& stream)
{
	std::string s;
	stream.serial(s);
	return s;
}

inline sint32 popSint(NLMISC::IStream& stream)
{
	sint32 val;
	stream.serial(val);
	return val;
}

inline uint32 popUint(NLMISC::IStream& stream)
{
	uint32 val;
	stream.serial(val);
	return val;
}

inline bool popBool(NLMISC::IStream& stream)
{
	bool val;
	stream.serial(val);
	return val;
}

template<class T> inline void pushToStream(NLMISC::IStream& stream,const T& value)
{
	stream.serial(const_cast<T&>(value));
}

inline void pushToStream(NLMISC::IStream& stream,const char* txt)
{
	std::string s(txt);
	stream.serial(s);
}

//-------------------------------------------------------------------------------------------------
// HANDY IPtr and IConstPtr CLASSES
//-------------------------------------------------------------------------------------------------
// This class gives a base that can be specialised in order to make pointer encapsulation classes
// it offers the basic standard methods that you have to define every time in the normal way...

template <class C>
class IPtr
{
public:
	IPtr()										{ _Ptr= NULL; }
	IPtr(C* p)									{ operator=(p); }
	IPtr& operator=(C* p)						{ _Ptr=p; return *this; }
	IPtr& operator=(IPtr& other)				{ _Ptr=other._Ptr; return *this; }
	IPtr& operator++()							{ ++_Ptr; return *this; }
	IPtr& operator--()							{ --_Ptr; return *this; }
	const C* operator->() const					{ return _Ptr; }
	const C& operator*() const					{ return *_Ptr; }
	operator C const *() const					{ return _Ptr; }
	C* operator->()								{ return _Ptr; }
	C& operator*()								{ return *_Ptr; }
	operator C*()								{ return _Ptr; }
	bool operator==(const IPtr& other) const	{ return _Ptr==other._Ptr; }
	bool operator!=(const IPtr& other) const	{ return _Ptr!=other._Ptr; }
	bool operator==(const C* p) const			{ return _Ptr==p; }
	bool operator!=(const C* p) const			{ return _Ptr!=p; }
private:
	C * _Ptr;
};


template <class C>
class IConstPtr
{
public:
	IConstPtr()										{ _Ptr= NULL; }
	IConstPtr(const C* p)							{ operator=(p); }
	IConstPtr& operator=(const C* p)				{ _Ptr=p; return *this; }
	IConstPtr& operator=(const IConstPtr& other)	{ _Ptr=other._Ptr; return *this; }
	IConstPtr& operator++()							{ ++_Ptr; return *this; }
	IConstPtr& operator--()							{ --_Ptr; return *this; }
	const C* operator->() const						{ return _Ptr; }
	const C& operator*() const						{ return *_Ptr; }
	operator C const *() const						{ return _Ptr; }
	bool operator==(const IConstPtr& other) const	{ return _Ptr==other._Ptr; }
	bool operator!=(const IConstPtr& other) const	{ return _Ptr!=other._Ptr; }
	bool operator==(const C* p) const				{ return _Ptr==p; }
	bool operator!=(const C* p) const				{ return _Ptr!=p; }
private:
	C const * _Ptr;
};


//-------------------------------------------------------------------------------------------------
// HANDY cleanPath() method for cleaning file system paths
//-------------------------------------------------------------------------------------------------

// Clean a path performing the following operations:
//	- convert '\\' characters to '/'
//	- replace '//' strings in the middle of the path with '/'
//	- remove '.' directory entries
//	- colapse '..' directory entries (removing parent entries)
//	- append a final '/' (optionally)
//
// examples:
//	- a:/bcd/efg/		=>	a:/bcd/efg/ (no change)
//	- a:\bcd\efg		=>	a:/bcd/efg/
//	- \bcd\\efg			=>	/bcd/efg/
//	- \\bcd\efg			=>	//bcd/efg/
//	- \bcd\.\efg		=>	/bcd/efg/
//	- \bcd\..\efg		=>	/efg/
//	- bcd\..\efg		=>	efg/
//	- bcd\..\..\efg		=>	../efg/
//	- \bcd\..\..\efg	=>	/efg/		(NOTE: the redundant '..' entry is lost due to leading '\')
//
NLMISC::CSString cleanPath(const NLMISC::CSString& path,bool addTrailingSlash);


template <class T>
struct TTypeLimits
{
	static T max();
	static T min();
	static T	floor(T value);
};

template <>
struct TTypeLimits<uint8>
{
	static uint8 max()			{		return (uint8)0xff;	}
	static uint8 min()			{		return 0;		}
	enum
	{
		IsSigned = 0,
		IsInteger = 1,
	};
	static uint8	floor(uint8 value)	{	return value;		}
};
template <>
struct TTypeLimits<uint16>
{
	static uint16 max()			{		return (uint16)0xffff;	}
	static uint16 min()			{		return 0;		}
	enum
	{
		IsSigned = 0,
		IsInteger = 1,
	};
	static uint16	floor(uint16 value)	{	return value;		}
};
template <>
struct TTypeLimits<uint32>
{
	static uint32 max()			{		return 0xffffffffu;	}
	static uint32 min()			{		return 0;			}
	enum
	{
		IsSigned = 0,
		IsInteger = 1,
	};
	static uint32	floor(uint32 value)	{	return value;		}
};
/*
#ifdef NL_OS_WINDOWS
template <>
struct TTypeLimits<unsigned int> : public TTypeLimits<uint32>
{
};
#endif
*/
template <>
struct TTypeLimits<uint64>
{
	static uint64 max()			{		return UINT64_CONSTANT(0xffffffffffffffff);	}
	static uint64 min()			{		return 0;					}
	enum
	{
		IsSigned = 0,
		IsInteger = 1,
	};
	static uint64	floor(uint64 value)	{	return value;		}
};

template <>
struct TTypeLimits<sint8>
{
	static sint8 max()		{		return (sint8)0x7f;	}
	static sint8 min()		{		return (sint8)0x80;	}
	enum
	{
		IsSigned = 1,
		IsInteger = 1,
	};
	static sint8	floor(sint8 value)	{	return value;		}
};
template <>
struct TTypeLimits<sint16>
{
	static sint16 max()			{		return (sint16)0x7fff;	}
	static sint16 min()			{		return (sint16)0x8000;	}
	enum
	{
		IsSigned = 1,
		IsInteger = 1,
	};
	static sint16	floor(sint16 value)	{	return value;		}
};
template <>
struct TTypeLimits<sint32>
{
	static sint32 max()			{		return (sint32)0x7fffffff;	}
	static sint32 min()			{		return (sint32)0x80000000;	}
	enum
	{
		IsSigned = 1,
		IsInteger = 1,
	};
	static sint32	floor(sint32 value)	{	return value;		}
};
/*#ifdef NL_OS_WINDOWS
template <>
struct TTypeLimits<int> : public TTypeLimits<sint32>
{
};
#endif*/
template <>
struct TTypeLimits<sint64>
{
	static sint64 max()			{		return SINT64_CONSTANT(0x7fffffffffffffff);	}
	static sint64 min()			{		return SINT64_CONSTANT(0x8000000000000000);	}
	enum
	{
		IsSigned = 1,
		IsInteger = 1,
	};
	static sint64	floor(sint64 value)	{	return value;		}
};

template <>
struct TTypeLimits<float>
{
	static float max()			{		return FLT_MAX;	}
	static float min()			{		return FLT_MIN;	}
	enum
	{
		IsSigned = 1,
		IsInteger = 0,
	};
	static float	floor(float f)	{ return f < 0 ? (float)::ceil(f) : (float)::floor(f);	}
};
template <>
struct TTypeLimits<double>
{
	static double max()			{		return DBL_MAX;	}
	static double min()			{		return DBL_MIN;	}
	enum
	{
		IsSigned = 1,
		IsInteger = 0,
	};
	static double	floor(double d)	{ return d < 0 ? ::ceil(d) : ::floor(d);	}
};



template <class T, class U>
inline T checkedCast(U val)
{
	typedef TTypeLimits<U>	TLimitIn;
	typedef TTypeLimits<T>	TLimitOut;

	// Only allow checked cast to integer type !
	nlctassert(TLimitOut::IsInteger);

	T dest = (T) val;
	U check = (U) dest;

	if (val < 0)
	{
		BOMB_IF(check != TLimitIn::floor(val), "checkedCast : Value "<<val<<" exceed the negative capacity of "<<typeid(T).name()<<" clamping at min value", return TLimitOut::min());
	}
	else
	{
		BOMB_IF(check != TLimitIn::floor(val), "checkedCast : Value "<<val<<" exceed the positive capacity of "<<typeid(T).name()<<" clamping at max value", return TLimitOut::max());
	}

	return T(dest);
}


//-------------------------------------------------------------------------------------------------
#endif
