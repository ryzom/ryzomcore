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

#ifndef NL_DEBUG_H
#define NL_DEBUG_H

#include "common.h"
#include "log.h"
#include "mutex.h"
#include "mem_displayer.h"
#include "displayer.h"
#include "app_context.h"

#include <cstdio>
#include <set>

namespace NLMISC
{

#ifdef ASSERT_THROW_EXCEPTION
#define ASSERT_THROW_EXCEPTION_CODE(exp) ASSERT_THROW_EXCEPTION_CODE_EX(exp, #exp)
#define ASSERT_THROW_EXCEPTION_CODE_EX(exp, str) if(!(exp)) throw NLMISC::Exception(str" returns false");
#else
#define ASSERT_THROW_EXCEPTION_CODE(exp)
#define ASSERT_THROW_EXCEPTION_CODE_EX(exp, str)
#endif

/** Imposter class to wrap all global access to the nel context for backward compatibility
 *	Yoyo note: This was a template before, hence with inline.
 *	We removed the inline because there was a hard compilation bug
 *	in plugin max under some compiler which caused operator-> to crash.... (don't understand why grrrr)
 *	Btw the method is optimized like this (1 call instead of 3 (and one with virtual)) because we added a local cache (_Log)
 *	Thus it is much better like this.
 */
class CImposterLog
{
private:
	typedef CLog *(INelContext::*TAccessor)();

	// Method to access the Log
	TAccessor	_Accessor;
public:
	CImposterLog(TAccessor accessor);
	CLog* operator -> ();
	operator CLog*();
	CLog &operator ()();
};


//
// Externals
//

// NOTE: The following are all NULL until createDebug() has been called at least once
// NOTE2: You must not use this class before the main() (not inside a static class ctor)
extern CImposterLog		ErrorLog;
extern CImposterLog		WarningLog;
extern CImposterLog		InfoLog;
extern CImposterLog		DebugLog;
extern CImposterLog		AssertLog;

extern CMemDisplayer *DefaultMemDisplayer;
extern CMsgBoxDisplayer *DefaultMsgBoxDisplayer;


//
// Functions
//

// internal use only
void createDebug (const char *logPath = NULL, bool logInFile = true, bool eraseLastLog = false);

/// Do not call this, unless you know what you're trying to do (it kills debug)!
void destroyDebug();

// call this if you want to change the dir of the log.log file
void changeLogDirectory(const std::string &dir);

// call this if you want to get the dir of the log.log file
std::string getLogDirectory();

// internal breakpoint window
void enterBreakpoint (const char *message);

// if true, the assert generates an assert
// if false, the assert just displays a warning and continue
void setAssert (bool assert);

// Beep (Windows only, no effect elsewhere)
void beep( uint freq, uint duration );


typedef std::string (*TCrashCallback)();

// this function enables user application to add information in the log when a crash occurs
void setCrashCallback(TCrashCallback crashCallback);

// For Crash report window. allow to know if a crash has already raised in the application
bool	isCrashAlreadyReported();
void	setCrashAlreadyReported(bool state);


// This very amazing macro __FUNCTION__ doesn't exist on VC6, map it to NULL
#ifdef NL_COMP_VC6
#	define __FUNCTION__ NULL
#endif


// Macros

/// Utility macro used by NL_MACRO_TO_STR to concatenate macro in text message.
#define NL_MACRO_TO_STR_SUBPART(x) #x

/** Use this macro to concatenate macro such
 *	You can use this macro to build '#pragma message' friendly macro
 *	or to make macro definition into string
 *	eg : #define M1 foo
 *		 #define MESSAGE "the message is "NL_MACRO_TO_STR(M1)
 *		 #pragma message(MESSAGE)
 *		 printf(NL_MACRO_TO_STR(M1));
 */
#define NL_MACRO_TO_STR(x) NL_MACRO_TO_STR_SUBPART(x)

/** the two following macros help to build compiler message using #pragma message
 *	on visual C++.
 *	The macro generate a message formated like the visual C++ compiler message.
 *	NL_LOC_MSG generate informative message and
 *	NL_LOC_WRN generate warning message not differentiable to genuine Visual C++ warning.
 *	The two message allow automatic source access with F4 or double click in
 *	output window.
 *
 *  usage : #pragma message( NL_LOC_MGS "your message" )
 *
 *  Note : If you want to concatenate another macro to your message, you
 *			can append using the NL_MACRO_TO_STR macro like in
 *			#define CLASS_NAME TheClassName
 *			#pragma message( NL_LOC_MGS "The class name is " NL_MACRO_TO_STR(CLASS_NAME))
 *  Note 2 : To show a warning under GCC, use #warning "Your warning here",
 *           see nel/net/net_manager.h for an example on how to use these correctly.
 */
#define NL_LOC_MSG __FILE__"("NL_MACRO_TO_STR(__LINE__)") : Message: "
#define NL_LOC_WRN __FILE__"("NL_MACRO_TO_STR(__LINE__)") : Warning Msg: "


/**
 * \def nldebug(exp)
 * Log a debug string. You don't have to put the final new line. It will be automatically append at the end of the string.
 *
 * Example:
 *\code
	void function(sint type)
	{
		// display the type value.
		nldebug("type is %d", type);
	}
 *\endcode
 */
#ifdef NL_NO_DEBUG
#	if defined(NL_COMP_VC) && NL_COMP_VC_VERSION >= 71
#		define nldebug __noop
#	else
#		define nldebug 0&&
#	endif
#else // NL_NO_DEBUG
	extern bool DisableNLDebug;
#	define nldebug if (NLMISC::DisableNLDebug) {} else (NLMISC::createDebug(), NLMISC::INelContext::getInstance().getDebugLog()->setPosition( __LINE__, __FILE__, __FUNCTION__ ), NLMISC::INelContext::getInstance().getDebugLog())->displayNL
#endif // NL_NO_DEBUG

/**
 * \def nlinfo(exp)
 * Same as nldebug but it will be display in debug and in release mode.
 */
#ifdef NL_NO_DEBUG
#	if defined(NL_COMP_VC) && NL_COMP_VC_VERSION >= 71
#		define nlinfo __noop
#	else
#		define nlinfo 0&&
#	endif
#else // NL_NO_DEBUG
#	define nlinfo (NLMISC::createDebug(), NLMISC::INelContext::getInstance().getInfoLog()->setPosition( __LINE__, __FILE__, __FUNCTION__ ), NLMISC::INelContext::getInstance().getInfoLog())->displayNL
#endif // NL_NO_DEBUG

/**
 * \def nlwarning(exp)
 * Same as nlinfo but you have to call this macro when something goes wrong but it's not a fatal error, the program could continue.
 *
 * Example:
 *\code
	void function(char *str)
	{
		// display the type value.
		if (str==NULL)
		{
			nlwarning("in function(), str should not be NULL, assume it's an empty string");
			str="";
		}
	}
 *\endcode
 */

#ifdef NL_NO_DEBUG
#	if defined(NL_COMP_VC) && NL_COMP_VC_VERSION >= 71
#		define nlwarning __noop
#	else
#		define nlwarning 0&&
#	endif
#else // NL_NO_DEBUG
#	define nlwarning (NLMISC::createDebug(), NLMISC::INelContext::getInstance().getWarningLog()->setPosition( __LINE__, __FILE__, __FUNCTION__ ), NLMISC::INelContext::getInstance().getWarningLog())->displayNL
#endif // NL_NO_DEBUG

/**
 * \def nlerror(exp)
 * Same as nlinfo but you have to call it when you have a fatal error, this macro display the text and \b exit the application
 * automatically. nlerror must be in a try/catch because it generates an EFatalError exception to exit the application.
 *
 *\code
	void function(char *filename)
	{
		FILE *fp = fopen (filename, "r");
		if (fp==NULL)
		{
			nlerror("file not found");
		}
	}
 *\endcode
 */
#define nlerror (NLMISC::createDebug (), NLMISC::INelContext::getInstance().getErrorLog()->setPosition( __LINE__, __FILE__, __FUNCTION__ ), NLMISC::nlFatalError)


/**
 * \def nlerrornoex(exp)
 * Same as nlerror but it doesn't generate any exceptions. It's used only in very specific case, for example, when you
 * call a nlerror in a catch block (look the service.cpp)
 */
#define nlerrornoex (NLMISC::createDebug (), NLMISC::INelContext::getInstance().getErrorLog()->setPosition( __LINE__, __FILE__, __FUNCTION__ ), NLMISC::nlError)


/**
 * \def nlassert(exp)
 * Try the assertion of \c exp. In release mode, nlassert do \b *nothing*. In debug mode, If \c exp is true, nothing happen,
 * otherwise, the program stop on the assertion line.
 *
 * Example:
 *\code
	void function(char *string)
	{
		// the string must not be NULL.
		nlassert(string!=NULL);
	}
 *\endcode
 */

/**
 * \def nlassertonce(exp)
 * Same behaviour as nlassert but the assertion will be test on time only. It's useful when you are under an inner loop and
 * you don't want to flood log file for example.
 *
 * Example:
 *\code
	for (int i = 0; i < Vect.size(); i++)
	{
		// all string must not be NULL, but if it's happen, log only the first time.
		nlassertonce(Vect[i].string!=NULL);
	}
 *\endcode
 */

/**
 * \def nlassertex(exp,str)
 * Same behaviour as nlassert but add a user defined \c str variables args string that will be display with the assert message.
 * Very useful when you can't debug directly on your computer.
 *
 * Example:
 *\code
	void function(sint type)
	{
		// the \c type must be between 0 and 15.
		nlassertex(type>=0&&type<=16, ("type was %d", type));
		// it'll display something like "assertion failed line 10 of test.cpp: type>=&&type<=16, type was 423555463"
	}
 *\endcode
 */

/**
 * \def nlverify(exp)
 * Same behaviour as nlassert but the \c exp will be executed in release mode too (but not tested).
 *
 * Example:
 *\code
	// Load a file and assert if the load failed. This example will work \b only in debug mode because in release mode,
	// nlassert do nothing, the load function will not be called...
	nlassert(load("test.tga"));

	// If you want to do that anyway, you could call nlverify. In release mode, the assertion will not be tested but
	// the \c load function will be called.
	nlverify(load("test.tga"));

	// You can also do this:
	bool res = load ("test.tga"));
	assert(res);

 *\endcode
 */

/**
 * \def nlverifyonce(exp)
 * Same behaviour as nlassertonce but it will execute \c exp in debug and release mode.
 */

/**
 * \def nlverifyex(exp,str)
 * Same behaviour as nlassertex but it will execute \c exp in debug and release mode.
 */

/**
 * \def nlstop
 * It stop the application at this point. It's exactly the same thing as "nlassert(false)".
 * Example:
 *\code
	switch(type)
	{
	case 1: ... break;
	case 2: ... break;
	default: nlstop;	// it should never happen...
	}
 *\endcode
 */

/**
 * \def nlstoponce
 * Same as nlassertonce(false);
 */

/**
 * \def nlstopex(exp)
 * Same as nlassertex(false,exp);
 */

// removed because we always check assert (even in release mode) #if defined (NL_OS_WINDOWS) && defined (NL_DEBUG)
#if defined (NL_OS_WINDOWS)
#define NLMISC_BREAKPOINT __debugbreak();
#else
#define NLMISC_BREAKPOINT abort()
#endif

// Internal, don't use it (make smaller assert code)
extern bool _assert_stop(bool &ignoreNextTime, sint line, const char *file, const char *funcName, const char *exp);
extern void _assertex_stop_0(bool &ignoreNextTime, sint line, const char *file, const char *funcName, const char *exp);
extern bool _assertex_stop_1(bool &ignoreNextTime);

// removed because we always check assert (even in release mode) #if defined(NL_DEBUG)

#ifdef NL_NO_DEBUG
#	define nlassert(exp) if(false)
#	define nlassertonce(exp) if(false)
#	define nlassertex(exp, str) if(false)
#	define nlverify(exp) { exp; }
#	define nlverifyonce(exp) { exp; }
#	define nlverifyex(exp, str) { exp; }
#else // NL_NO_DEBUG

#	ifdef NL_OS_UNIX

// Linux set of asserts is reduced due to that there is no message box displayer

#define nlassert(exp) \
do { \
	if (!(exp)) { \
		NLMISC::createDebug (); \
		NLMISC::INelContext::getInstance().getAssertLog()->setPosition (__LINE__, __FILE__, __FUNCTION__); \
		NLMISC::INelContext::getInstance().getAssertLog()->displayNL ("\"%s\" ", #exp); \
		NLMISC_BREAKPOINT; \
	} \
} while(0)

#define nlassertonce(exp) nlassert(exp)

#define nlassertex(exp, str) \
do { \
	if (!(exp)) { \
		NLMISC::createDebug (); \
		NLMISC::INelContext::getInstance().getAssertLog()->setPosition (__LINE__, __FILE__, __FUNCTION__); \
		NLMISC::INelContext::getInstance().getAssertLog()->displayNL ("\"%s\" ", #exp); \
		NLMISC::INelContext::getInstance().getAssertLog()->displayRawNL str; \
		NLMISC_BREAKPOINT; \
	} \
} while(0)

#define nlverify(exp) nlassert(exp)
#define nlverifyonce(exp) nlassert(exp)
#define nlverifyex(exp, str) nlassertex(exp, str)

#	else // NL_OS_UNIX

#define nlassert(exp) \
do { \
	static bool ignoreNextTime = false; \
	bool _expResult_ = (exp) ? true : false; \
	if (!ignoreNextTime && !_expResult_) { \
		if(NLMISC::_assert_stop(ignoreNextTime, __LINE__, __FILE__, __FUNCTION__, #exp)) \
			NLMISC_BREAKPOINT; \
	} \
	ASSERT_THROW_EXCEPTION_CODE_EX(_expResult_, #exp) \
} while(0)

#define nlassertonce(exp) \
do { \
	static bool ignoreNextTime = false; \
	if (!ignoreNextTime && !(exp)) { \
		ignoreNextTime = true; \
		if(NLMISC::_assert_stop(ignoreNextTime, __LINE__, __FILE__, __FUNCTION__, #exp)) \
			NLMISC_BREAKPOINT; \
	} \
} while(0)

#define nlassertex(exp, str) \
do { \
	static bool ignoreNextTime = false; \
	bool _expResult_ = (exp) ? true : false; \
	if (!ignoreNextTime && !_expResult_) { \
		NLMISC::_assertex_stop_0(ignoreNextTime, __LINE__, __FILE__, __FUNCTION__, #exp); \
		NLMISC::INelContext::getInstance().getAssertLog()->displayRawNL str; \
		if(NLMISC::_assertex_stop_1(ignoreNextTime)) \
			NLMISC_BREAKPOINT; \
	} \
	ASSERT_THROW_EXCEPTION_CODE_EX(_expResult_, #exp) \
} while(0)

#define nlverify(exp) \
do { \
	static bool ignoreNextTime = false; \
	bool _expResult_ = (exp) ? true : false; \
	if (!_expResult_ && !ignoreNextTime) { \
		if(NLMISC::_assert_stop(ignoreNextTime, __LINE__, __FILE__, __FUNCTION__, #exp)) \
			NLMISC_BREAKPOINT; \
	} \
	ASSERT_THROW_EXCEPTION_CODE_EX(_expResult_, #exp) \
} while(0)

#define nlverifyonce(exp) \
do { \
	static bool ignoreNextTime = false; \
	bool _expResult_ = (exp) ? true : false; \
	if (!_expResult_ && !ignoreNextTime) { \
		ignoreNextTime = true; \
		if(NLMISC::_assert_stop(ignoreNextTime, __LINE__, __FILE__, __FUNCTION__, #exp)) \
			NLMISC_BREAKPOINT; \
	} \
} while(0)

#define nlverifyex(exp, str) \
do { \
	static bool ignoreNextTime = false; \
	bool _expResult_ = (exp) ? true : false; \
	if (!_expResult_ && !ignoreNextTime) { \
		NLMISC::_assertex_stop_0(ignoreNextTime, __LINE__, __FILE__, __FUNCTION__, #exp); \
		NLMISC::INelContext::getInstance().getAssertLog()->displayRawNL str; \
		if(NLMISC::_assertex_stop_1(ignoreNextTime)) \
			NLMISC_BREAKPOINT; \
	} \
	ASSERT_THROW_EXCEPTION_CODE_EX(_expResult_, #exp) \
} while(0)

#	endif // NL_OS_UNIX

#endif // NL_NO_DEBUG

#define nlunreferenced(identifier) (void)identifier

#define nlstop \
do { \
	static bool ignoreNextTime = false; \
	if (!ignoreNextTime) { \
		if(NLMISC::_assert_stop(ignoreNextTime, __LINE__, __FILE__, __FUNCTION__, NULL)) \
			NLMISC_BREAKPOINT; \
	} \
	ASSERT_THROW_EXCEPTION_CODE(false) \
} while(0)

#define nlstoponce \
do { \
	static bool ignoreNextTime = false; \
	if (!ignoreNextTime) { \
		ignoreNextTime = true; \
		if(NLMISC::_assert_stop(ignoreNextTime, __LINE__, __FILE__, __FUNCTION__, NULL)) \
			NLMISC_BREAKPOINT; \
	} \
} while(0)


#define nlstopex(str) \
do { \
	static bool ignoreNextTime = false; \
	if (!ignoreNextTime) { \
		NLMISC::_assertex_stop_0(ignoreNextTime, __LINE__, __FILE__, __FUNCTION__, NULL); \
		NLMISC::INelContext::getInstance().getAssertLog()->displayRawNL str; \
		if(NLMISC::_assertex_stop_1(ignoreNextTime)) \
			NLMISC_BREAKPOINT; \
	} \
} while(0)


struct EFatalError : public Exception
{
	EFatalError() : Exception( "nlerror() called" ) {}
};

class ETrapDebug : public Exception
{
};

// undef default assert to force people to use nlassert() instead of assert()
// if NL_MAP_ASSERT is set we map assert to nlassert instead of removing it
// this makes it compatible with zeroc's ICE network library

#ifdef NL_MAP_ASSERT
#	ifdef assert
#		undef assert
#		define assert nlassert
#	endif
#else
#	ifdef assert
#		undef assert
#		define assert(a) you_must_not_use_assert___use_nl_assert___read_debug_h_file
#	endif
#endif


/// Get the call stack and set it with result
void getCallStack(std::string &result, sint skipNFirst = 0);

/// Get the call stack and the logs and set it with result
void getCallStackAndLog (std::string &result, sint skipNFirst = 0);

/**
 * safe_cast<>: this is a function which nlassert() a dynamic_cast in Debug, and just do a static_cast in release.
 * So slow check is made in debug, but only fast cast is made in release.
 */
template<class T, class U>	inline T	safe_cast(U o)
{
	// NB: must check debug because assert may still be here in release
#ifdef NL_DEBUG
	nlassert(dynamic_cast<T>(o));
#endif
	return static_cast<T>(o);
}

/**
 * type_cast<>: this is a function which nlassert() a dynamic_cast in Debug, and just do a static_cast in release.
 * So slow check is made in debug, but only fast cast is made in release.
 * Differs from safe_cast by allowinf NULL objets. (ask Stephane LE DORZE for more explanations).
 */
template<class T, class U>	inline T	type_cast(U o)
{
	// NB: must check debug because assert may still be here in release
#ifdef NL_DEBUG
	if (o)
		nlassert(dynamic_cast<T>(o));
#endif
	//	optimization made to check pointer validity before address translation. (hope it works on linux).
	if ((size_t)(static_cast<T>((U)0x0400)) == (size_t)((U)0x0400))
	{
		return static_cast<T>(o);
	}
	else
	{
		return (o==0)?0:static_cast<T>(o);
	}
}

/** Compile time assertion
  */
#ifdef NL_ISO_CPP0X_AVAILABLE
#	define nlctassert(cond) static_assert(cond, "Compile time assert in "#cond)
#else
#	define nlctassert(cond) sizeof(uint[(cond) ? 1 : 0])
#endif

/**
*	Allow to verify an object was accessed before its destructor call.
*	For instance, it could be used to check if the user take care of method call return.
*	ex:
*		CMustConsume<TErrorCode>	foo()
*		{
*			...
*			return	ErrorInvalidateType;		//	part of TErrorCode enum.
*		}
*	Exclusive implementation samples:
*		TerrorCode code=foo().consumeValue();	//	Good!
*		foo().consume();						//	Good!
*		TerrorCode code=foo();					//	Mistake!
*		foo();									//	Will cause an assert at next ending brace during execution time.
*		TerrorCode code=foo().Value();			//	Will cause an assert at next ending brace during execution time.
*	(ask Stephane LE DORZE for more explanations).
*/

// Need a breakpoint in the assert / verify macro
extern bool DebugNeedAssert;

// Internal process, don't use it
extern bool NoAssert;


template<class T>
class CMustConsume
{
public:
	CMustConsume(const T &val) : Value(val)
#if !FINAL_VERSION
	, Consumed(false)
#endif
	{
	}

	~CMustConsume()
	{
#if !FINAL_VERSION
		nlassert(Consumed == true);
#endif
	}

	//	Get the value without validating the access.
	const T &value() const
	{
		return Value;
	}

	operator const T &() const
	{
#if !FINAL_VERSION
		Consumed = true;
#endif
		return Value;
	}

	//	Get the value and validate the access.
	const T &consumeValue() const
	{
#if !FINAL_VERSION
		Consumed = true;
#endif
		return Value;
	}
	//	Only consume the access.
	void consume() const
	{
#if !FINAL_VERSION
		Consumed = true;
#endif
	}

private:
	T				Value;
#if !FINAL_VERSION
	mutable	bool	Consumed;
#endif
};

/// Data for instance counting
struct TInstanceCounterData
{
	sint32		_InstanceCounter;
	sint32		_DeltaCounter;
	const char	*_ClassName;
	bool		_Touched;

	TInstanceCounterData(const char *className);

	~TInstanceCounterData();
};

// forward declaration for members of CInstanceCounterManager
class CInstanceCounterLocalManager;

// The singleton used to display the instance counter
class CInstanceCounterManager
{
//	NLMISC_SAFE_SINGLETON_DECL(CInstanceCounterManager);
	private:
		/* declare private constructors*/
		CInstanceCounterManager () {}
		CInstanceCounterManager (const CInstanceCounterManager &) {}
		/* the local static pointer to the singleton instance */
		static CInstanceCounterManager	*_Instance;
	public:
		static CInstanceCounterManager &getInstance()
		{
			if (_Instance == NULL)
			{
				/* the nel context MUST be initialised */
//				nlassert(NLMISC::NelContext != NULL);
				void *ptr = NLMISC::INelContext::getInstance().getSingletonPointer("CInstanceCounterManager");
				if (ptr == NULL)
				{
					/* allocate the singleton and register it */
					_Instance = new CInstanceCounterManager;
					NLMISC::INelContext::getInstance().setSingletonPointer("CInstanceCounterManager", _Instance);
				}
				else
				{
					_Instance = reinterpret_cast<CInstanceCounterManager*>(ptr);
				}
			}
			return *_Instance;
		}
	private:
public:

	std::string displayCounters() const;

	void resetDeltaCounter();

	uint32 getInstanceCounter(const std::string &className) const;
	sint32 getInstanceCounterDelta(const std::string &className) const;

private:

	friend class CInstanceCounterLocalManager;

	void registerInstaceCounterLocalManager(CInstanceCounterLocalManager *localMgr);
	void unregisterInstaceCounterLocalManager(CInstanceCounterLocalManager *localMgr);

//	static CInstanceCounterManager		*_Instance;
	std::set<CInstanceCounterLocalManager*>	_InstanceCounterMgrs;
};

//
// Local instance counter
//
class CInstanceCounterLocalManager
{
public:
	static CInstanceCounterLocalManager &getInstance()
	{
		if (_Instance == NULL)
		{
			_Instance = new CInstanceCounterLocalManager;
		}
		return *_Instance;
	}

	static void releaseInstance()
	{
		if (_Instance != NULL)
		{
			delete _Instance;
			_Instance = NULL;
		}
	}

	void registerInstanceCounter(TInstanceCounterData *counter)
	{
		_InstanceCounters.insert(counter);
	}

	void unregisterInstanceCounter(TInstanceCounterData *counter);

	~CInstanceCounterLocalManager()
	{
		CInstanceCounterManager::getInstance().unregisterInstaceCounterLocalManager(this);
	}

private:
	friend class CInstanceCounterManager;
	friend class INelContext;

	CInstanceCounterLocalManager()
	{
	}

	void registerLocalManager()
	{
		CInstanceCounterManager::getInstance().registerInstaceCounterLocalManager(this);
	}

	static CInstanceCounterLocalManager		*_Instance;
	std::set<TInstanceCounterData*>			_InstanceCounters;
};


/** Utility to count instance of class.
 *	This class is designed to be lightweight and to trace
 *	the number of instance of 'tagged' class.
 *	Commands are provided to display the actual number
 *	of object of each class and to compute delta
 *	between two call of the displayer.
 *	Usage is simple, you just have to put a macro
 *	inside the class definition to trace it's allocation
 *	and a macro in the implementation file.
 *	The macro only add a compiler minimum size for member
 *	struct of 0 octets (witch can be 0 or 1 octet, compiler
 *	dependent).
 *	usage :
 *
 *	In the header :
 *	class foo	// This is the class we want to count instance
 *	{
 *		NL_INSTANCE_COUNTER_DECL(foo);
 *	}
 *	In the cpp :
 *	NL_INSTANCE_COUNTER_IMPL(foo);
 */
#define NL_INSTANCE_COUNTER_DECL(className) \
public: \
	struct className##InstanceCounter \
	{ \
		className##InstanceCounter() \
		{ \
			_InstanceCounterData._InstanceCounter++; \
			_InstanceCounterData._Touched = true; \
		} \
		className##InstanceCounter(const className##InstanceCounter &/* other */) \
		{ \
			_InstanceCounterData._InstanceCounter++; \
			_InstanceCounterData._Touched = true; \
		} \
		\
		~className##InstanceCounter()\
		{ \
			_InstanceCounterData._InstanceCounter--; \
		} \
		static sint32 getInstanceCounter() \
		{ \
			return _InstanceCounterData._InstanceCounter; \
		} \
		static sint32 getInstanceCounterDelta() \
		{ \
			return _InstanceCounterData._InstanceCounter - _InstanceCounterData._DeltaCounter; \
		} \
		static NLMISC::TInstanceCounterData	_InstanceCounterData; \
	}; \
	 \
	className##InstanceCounter	_##className##InstanceCounter; \
private:

/// The macro to make the implementation of the counter
#define NL_INSTANCE_COUNTER_IMPL(className) NLMISC::TInstanceCounterData className::className##InstanceCounter::_InstanceCounterData(#className);

/// An utility macro to get the instance counter for a class
#define NL_GET_LOCAL_INSTANCE_COUNTER(className) className::className##InstanceCounter::getInstanceCounter()
#define NL_GET_INSTANCE_COUNTER(className) NLMISC::CInstanceCounterManager::getInstance().getInstanceCounter(#className)

/// An utility macro to get the delta since the last counter reset.
#define NL_GET_LOCAL_INSTANCE_COUNTER_DELTA(className) className::className##InstanceCounter::getInstanceCounterDelta()
#define NL_GET_INSTANCE_COUNTER_DELTA(className) NLMISC::CInstanceCounterManager::getInstance().getInstanceCounterDelta(#className)

//
// Following are internal functions, you should never use them
//

/// Never use this function (internal use only)
void nlFatalError (const char *format, ...);

/// Never use this function but call the nlerror macro (internal use only)
void nlError (const char *format, ...);

#define NL_CRASH_DUMP_FILE "nel_debug.dmp"

// Standard API to retrieve error code and test for windows or unix platform

/// Return the last error code generated by a system call
int getLastError();

/// Return a readable text according to the error code submited
std::string formatErrorMessage(int errorCode);


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

class CNLDebugOverride
{
public:
	CNLDebugOverride(NLMISC::CLog *debugLog)
	{
		nlassert(debugLog!=NULL);
		_OldValue=NLMISC::DebugLog;
		nlassert(_OldValue!=NULL);
		NLMISC::INelContext::getInstance().setDebugLog(debugLog);
	}
	~CNLDebugOverride()
	{
		NLMISC::INelContext::getInstance().setDebugLog(_OldValue);
	}
private:
	// prohibit copy
	CNLDebugOverride(const CNLDebugOverride&);
	NLMISC::CLog *_OldValue;
};

//-------------------------------------------------------------------------------------------------

class CNLInfoOverride
{
public:
	CNLInfoOverride(NLMISC::CLog *infoLog)
	{
		nlassert(infoLog!=NULL);
		_OldValue=NLMISC::InfoLog;
		nlassert(_OldValue!=NULL);
		NLMISC::INelContext::getInstance().setInfoLog(infoLog);
	}
	~CNLInfoOverride()
	{
		NLMISC::INelContext::getInstance().setInfoLog(_OldValue);
	}
private:
	// prohibit copy
	CNLInfoOverride(const CNLInfoOverride&);
	NLMISC::CLog *_OldValue;
};

//-------------------------------------------------------------------------------------------------

class CNLWarningOverride
{
public:
	CNLWarningOverride(NLMISC::CLog *warningLog)
	{
		nlassert(warningLog!=NULL);
		_OldValue=NLMISC::WarningLog;
		nlassert(_OldValue!=NULL);
		NLMISC::INelContext::getInstance().setWarningLog(warningLog);
	}
	~CNLWarningOverride()
	{
		NLMISC::INelContext::getInstance().setWarningLog(_OldValue);
	}
private:
	// prohibit copy
	CNLWarningOverride(const CNLWarningOverride&);
	NLMISC::CLog *_OldValue;
};

//-------------------------------------------------------------------------------------------------

class CNLLogOverride
{
public:
	CNLLogOverride(NLMISC::CLog *commonLog): _DebugLog(commonLog), _InfoLog(commonLog), _WarningLog(commonLog)	{}

private:
	CNLDebugOverride	_DebugLog;
	CNLInfoOverride		_InfoLog;
	CNLWarningOverride	_WarningLog;
};


//-------------------------------------------------------------------------------------------------

class CNLSmartLogOverride
{
public:
	CNLSmartLogOverride(NLMISC::CLog *commonLog):
		_DebugLog(commonLog==NLMISC::InfoLog?NLMISC::DebugLog:commonLog),
		_InfoLog(commonLog),
		_WarningLog(commonLog==NLMISC::InfoLog?NLMISC::WarningLog:commonLog)
		{}

private:
	CNLDebugOverride	_DebugLog;
	CNLInfoOverride		_InfoLog;
	CNLWarningOverride	_WarningLog;
};




} // NLMISC

#endif // NL_DEBUG_H

/* End of debug.h */
