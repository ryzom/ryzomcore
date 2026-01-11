/*

Copyright (c) 2009-2014, Jan BOON
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef NLMISC_CALLBACK_H
#define NLMISC_CALLBACK_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes
#include <nel/misc/debug.h>

// Project includes

namespace NLMISC {

#define NLMISC_CALLBACK_TEMPLATE \
/** \
 * \brief NLMISC_CALLBACK_ARGS_CLASS \
 * \date 2009-03-03 18:09GMT \
 * \author Jan BOON \
 * Callback template \
 */ \
template<typename TReturn NLMISC_CALLBACK_ARGS_TYPENAME> \
class NLMISC_CALLBACK_ARGS_CLASS \
{ \
	/* Very simple reference counting callback base */ \
	class CCallbackBase \
	{ \
	public: \
		CCallbackBase() : m_RefCount(0) \
		{ \
			 \
		} \
		 \
		virtual ~CCallbackBase() \
		{ \
			nlassert(!m_RefCount); \
		} \
		 \
		void refAdd() \
		{ \
			++m_RefCount; \
		} \
		 \
		void refRemove() \
		{ \
			--m_RefCount; \
			if (!m_RefCount) \
				delete this; \
		} \
		 \
		virtual TReturn callback(NLMISC_CALLBACK_ARGS_DECL) = 0; \
		 \
		virtual bool equals(const CCallbackBase *callbackBase) = 0; \
		 \
		/* disable copy */ \
		CCallbackBase(const CCallbackBase &); \
		CCallbackBase &operator=(const CCallbackBase &); \
		 \
	private: \
		uint m_RefCount; \
	}; \
	 \
	typedef TReturn TCallbackFunction(NLMISC_CALLBACK_ARGS_DECL); \
	class CCallbackFunction : public CCallbackBase \
	{ \
	public: \
		CCallbackFunction(TCallbackFunction *callbackFunction) : m_CallbackFunction(callbackFunction) \
		{ \
			nlassert(m_CallbackFunction); \
		} \
		 \
		virtual ~CCallbackFunction() \
		{ \
			m_CallbackFunction = NULL; \
		} \
		 \
		virtual TReturn callback(NLMISC_CALLBACK_ARGS_DECL) \
		{ \
			return m_CallbackFunction(NLMISC_CALLBACK_ARGS_IMPL); \
		} \
		 \
		virtual bool equals(const CCallbackBase *callbackBase) \
		{ \
			const CCallbackFunction *callbackFunction = \
				dynamic_cast<const CCallbackFunction *>(callbackBase); \
			if (!callbackFunction) return false; \
			return m_CallbackFunction == callbackFunction->m_CallbackFunction; \
		} \
		 \
	private: \
		TCallbackFunction *m_CallbackFunction; \
	}; \
	 \
	template<typename TClass> \
	class CCallbackMethod : public CCallbackBase \
	{ \
		typedef TReturn (TClass::*TCallbackMethod)(NLMISC_CALLBACK_ARGS_DECL); \
	public: \
		CCallbackMethod(TClass *callbackObject, TCallbackMethod callbackMethod) : m_CallbackObject(callbackObject), m_CallbackMethod(callbackMethod) \
		{ \
			nlassert(m_CallbackObject); \
			nlassert(m_CallbackMethod); \
		} \
		 \
		virtual ~CCallbackMethod() \
		{ \
			m_CallbackObject = NULL; \
			m_CallbackMethod = NULL; \
		} \
		 \
		virtual TReturn callback(NLMISC_CALLBACK_ARGS_DECL) \
		{ \
			return (m_CallbackObject->*m_CallbackMethod)(NLMISC_CALLBACK_ARGS_IMPL); \
		} \
		 \
		virtual bool equals(const CCallbackBase *callbackBase) \
		{ \
			const CCallbackMethod *callbackMethod = \
				dynamic_cast<const CCallbackMethod *>(callbackBase); \
			if (!callbackMethod) return false; \
			return m_CallbackObject == callbackMethod->m_CallbackObject \
				&& m_CallbackMethod == callbackMethod->m_CallbackMethod; \
		} \
		 \
	private: \
		TClass *m_CallbackObject; \
		TCallbackMethod m_CallbackMethod; \
	}; \
	 \
public: \
	CCallback() : m_CallbackBase(NULL) \
	{ \
		 \
	} \
	 \
	CCallback(TCallbackFunction *callbackFunction) : m_CallbackBase(new CCallbackFunction(callbackFunction)) \
	{ \
		nlassert(m_CallbackBase); \
		m_CallbackBase->refAdd(); \
	} \
	 \
	template<typename TClass> \
	CCallback(TClass *callbackObject, TReturn (TClass::*callbackMethod)(NLMISC_CALLBACK_ARGS_DECL)) : m_CallbackBase(new CCallbackMethod<TClass>(callbackObject, callbackMethod)) \
	{ \
		nlassert(m_CallbackBase); \
		m_CallbackBase->refAdd(); \
	} \
	 \
	CCallback(const CCallback &callback) \
	{ \
		m_CallbackBase = callback.m_CallbackBase; \
		if (m_CallbackBase) \
			m_CallbackBase->refAdd(); \
	} \
	 \
	CCallback &operator=(const CCallback &callback) \
	{ \
		if (m_CallbackBase != callback.m_CallbackBase) \
		{ \
			if (m_CallbackBase) \
				m_CallbackBase->refRemove(); \
			m_CallbackBase = callback.m_CallbackBase; \
			if (m_CallbackBase) \
				m_CallbackBase->refAdd(); \
		} \
		return *this; \
	} \
	 \
	~CCallback() \
	{ \
		if (m_CallbackBase) \
		{ \
			m_CallbackBase->refRemove(); \
			m_CallbackBase = NULL; \
		} \
	} \
	 \
	TReturn callback(NLMISC_CALLBACK_ARGS_DECL) \
	{ \
		nlassert(m_CallbackBase); \
		return m_CallbackBase->callback(NLMISC_CALLBACK_ARGS_IMPL); \
	} \
	 \
	TReturn operator()(NLMISC_CALLBACK_ARGS_DECL) \
	{ \
		nlassert(m_CallbackBase); \
		return m_CallbackBase->callback(NLMISC_CALLBACK_ARGS_IMPL); \
	} \
	 \
	bool valid() const \
	{ \
		return m_CallbackBase != NULL; \
	} \
	 \
	operator bool() const \
	{ \
		return m_CallbackBase != NULL; \
	} \
	 \
	bool operator==(const CCallback &callback) \
	{ \
		return m_CallbackBase->equals(callback.m_CallbackBase); \
	} \
	 \
private: \
	CCallbackBase *m_CallbackBase; \
	 \
}; /* class CCallback */ \

template<typename TReturn, typename TArgsA = void, typename TArgsB = void, typename TArgsC = void, typename TArgsD = void, typename TArgsE = void, typename TArgsF = void, typename TArgsG = void, typename TDummy = void>
class CCallback;

#define NLMISC_CALLBACK_ARGS_CLASS CCallback<TReturn, void, void, void, void, void, void, void, void>
#define NLMISC_CALLBACK_ARGS_TYPENAME 
#define NLMISC_CALLBACK_ARGS_DECL 
#define NLMISC_CALLBACK_ARGS_IMPL 
NLMISC_CALLBACK_TEMPLATE
#undef NLMISC_CALLBACK_ARGS_CLASS
#undef NLMISC_CALLBACK_ARGS_TYPENAME
#undef NLMISC_CALLBACK_ARGS_DECL
#undef NLMISC_CALLBACK_ARGS_IMPL

#define NLMISC_CALLBACK_ARGS_CLASS CCallback<TReturn, TArgsA, void, void, void, void, void, void, void>
#define NLMISC_CALLBACK_ARGS_TYPENAME , typename TArgsA
#define NLMISC_CALLBACK_ARGS_DECL TArgsA argsA
#define NLMISC_CALLBACK_ARGS_IMPL argsA
NLMISC_CALLBACK_TEMPLATE
#undef NLMISC_CALLBACK_ARGS_CLASS
#undef NLMISC_CALLBACK_ARGS_TYPENAME
#undef NLMISC_CALLBACK_ARGS_DECL
#undef NLMISC_CALLBACK_ARGS_IMPL

#define NLMISC_CALLBACK_ARGS_CLASS CCallback<TReturn, TArgsA, TArgsB, void, void, void, void, void, void>
#define NLMISC_CALLBACK_ARGS_TYPENAME , typename TArgsA, typename TArgsB
#define NLMISC_CALLBACK_ARGS_DECL TArgsA argsA, TArgsB argsB
#define NLMISC_CALLBACK_ARGS_IMPL argsA, argsB
NLMISC_CALLBACK_TEMPLATE
#undef NLMISC_CALLBACK_ARGS_CLASS
#undef NLMISC_CALLBACK_ARGS_TYPENAME
#undef NLMISC_CALLBACK_ARGS_DECL
#undef NLMISC_CALLBACK_ARGS_IMPL

#define NLMISC_CALLBACK_ARGS_CLASS CCallback<TReturn, TArgsA, TArgsB, TArgsC, void, void, void, void, void>
#define NLMISC_CALLBACK_ARGS_TYPENAME , typename TArgsA, typename TArgsB, typename TArgsC
#define NLMISC_CALLBACK_ARGS_DECL TArgsA argsA, TArgsB argsB, TArgsC argsC
#define NLMISC_CALLBACK_ARGS_IMPL argsA, argsB, argsC
NLMISC_CALLBACK_TEMPLATE
#undef NLMISC_CALLBACK_ARGS_CLASS
#undef NLMISC_CALLBACK_ARGS_TYPENAME
#undef NLMISC_CALLBACK_ARGS_DECL
#undef NLMISC_CALLBACK_ARGS_IMPL

#define NLMISC_CALLBACK_ARGS_CLASS CCallback<TReturn, TArgsA, TArgsB, TArgsC, TArgsD, void, void, void, void>
#define NLMISC_CALLBACK_ARGS_TYPENAME , typename TArgsA, typename TArgsB, typename TArgsC, typename TArgsD
#define NLMISC_CALLBACK_ARGS_DECL TArgsA argsA, TArgsB argsB, TArgsC argsC, TArgsD argsD
#define NLMISC_CALLBACK_ARGS_IMPL argsA, argsB, argsC, argsD
NLMISC_CALLBACK_TEMPLATE
#undef NLMISC_CALLBACK_ARGS_CLASS
#undef NLMISC_CALLBACK_ARGS_TYPENAME
#undef NLMISC_CALLBACK_ARGS_DECL
#undef NLMISC_CALLBACK_ARGS_IMPL

#define NLMISC_CALLBACK_ARGS_CLASS CCallback<TReturn, TArgsA, TArgsB, TArgsC, TArgsD, TArgsE, void, void, void>
#define NLMISC_CALLBACK_ARGS_TYPENAME , typename TArgsA, typename TArgsB, typename TArgsC, typename TArgsD, typename TArgsE
#define NLMISC_CALLBACK_ARGS_DECL TArgsA argsA, TArgsB argsB, TArgsC argsC, TArgsD argsD, TArgsE argsE
#define NLMISC_CALLBACK_ARGS_IMPL argsA, argsB, argsC, argsD, argsE
NLMISC_CALLBACK_TEMPLATE
#undef NLMISC_CALLBACK_ARGS_CLASS
#undef NLMISC_CALLBACK_ARGS_TYPENAME
#undef NLMISC_CALLBACK_ARGS_DECL
#undef NLMISC_CALLBACK_ARGS_IMPL

#define NLMISC_CALLBACK_ARGS_CLASS CCallback<TReturn, TArgsA, TArgsB, TArgsC, TArgsD, TArgsE, TArgsF, void, void>
#define NLMISC_CALLBACK_ARGS_TYPENAME , typename TArgsA, typename TArgsB, typename TArgsC, typename TArgsD, typename TArgsE, typename TArgsF
#define NLMISC_CALLBACK_ARGS_DECL TArgsA argsA, TArgsB argsB, TArgsC argsC, TArgsD argsD, TArgsE argsE, TArgsF argsF
#define NLMISC_CALLBACK_ARGS_IMPL argsA, argsB, argsC, argsD, argsE, argsF
NLMISC_CALLBACK_TEMPLATE
#undef NLMISC_CALLBACK_ARGS_CLASS
#undef NLMISC_CALLBACK_ARGS_TYPENAME
#undef NLMISC_CALLBACK_ARGS_DECL
#undef NLMISC_CALLBACK_ARGS_IMPL

#define NLMISC_CALLBACK_ARGS_CLASS CCallback<TReturn, TArgsA, TArgsB, TArgsC, TArgsD, TArgsE, TArgsF, TArgsG, void>
#define NLMISC_CALLBACK_ARGS_TYPENAME , typename TArgsA, typename TArgsB, typename TArgsC, typename TArgsD, typename TArgsE, typename TArgsF, typename TArgsG
#define NLMISC_CALLBACK_ARGS_DECL TArgsA argsA, TArgsB argsB, TArgsC argsC, TArgsD argsD, TArgsE argsE, TArgsF argsF, TArgsG argsG
#define NLMISC_CALLBACK_ARGS_IMPL argsA, argsB, argsC, argsD, argsE, argsF, argsG
NLMISC_CALLBACK_TEMPLATE
#undef NLMISC_CALLBACK_ARGS_CLASS
#undef NLMISC_CALLBACK_ARGS_TYPENAME
#undef NLMISC_CALLBACK_ARGS_DECL
#undef NLMISC_CALLBACK_ARGS_IMPL
#undef NLMISC_CALLBACK_ARGS_CLASSNAME

#undef NLMISC_CALLBACK_TEMPLATE

} /* namespace NLMISC */

#endif /* #ifndef NLMISC_CALLBACK_H */

/* end of file */
