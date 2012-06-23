/**
 * \file callback.h
 * \brief PIPELINE_CALLBACK_ARGS_CLASS
 * \date 2009-03-03 18:09GMT
 * \author Jan Boon (Kaetemi)
 * PIPELINE_CALLBACK_ARGS_CLASS
 */

/* 
 * Copyright (C) 2009-2012  by authors
 * 
 * This file is part of RYZOM CORE PIPELINE.
 * RYZOM CORE PIPELINE is free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 2 of
 * the License, or (at your option) any later version.
 * 
 * RYZOM CORE PIPELINE is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with RYZOM CORE PIPELINE; see the file COPYING.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef PIPELINE_CALLBACK_H
#define PIPELINE_CALLBACK_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes
#include <nel/misc/debug.h>

// Project includes

namespace PIPELINE {

#define PIPELINE_CALLBACK_TEMPLATE \
/** \
 * \brief PIPELINE_CALLBACK_ARGS_CLASS \
 * \date 2009-03-03 18:09GMT \
 * \author Jan Boon (Kaetemi) \
 * Awesome callback template \
 */ \
template<typename TReturn PIPELINE_CALLBACK_ARGS_TYPENAME> \
class PIPELINE_CALLBACK_ARGS_CLASS \
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
		virtual TReturn callback(PIPELINE_CALLBACK_ARGS_DECL) = 0; \
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
	typedef TReturn TCallbackFunction(PIPELINE_CALLBACK_ARGS_DECL); \
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
		virtual TReturn callback(PIPELINE_CALLBACK_ARGS_DECL) \
		{ \
			return m_CallbackFunction(PIPELINE_CALLBACK_ARGS_IMPL); \
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
		typedef TReturn (TClass::*TCallbackMethod)(PIPELINE_CALLBACK_ARGS_DECL); \
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
		virtual TReturn callback(PIPELINE_CALLBACK_ARGS_DECL) \
		{ \
			return (m_CallbackObject->*m_CallbackMethod)(PIPELINE_CALLBACK_ARGS_IMPL); \
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
	CCallback(TClass *callbackObject, TReturn (TClass::*callbackMethod)(PIPELINE_CALLBACK_ARGS_DECL)) : m_CallbackBase(new CCallbackMethod<TClass>(callbackObject, callbackMethod)) \
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
	TReturn callback(PIPELINE_CALLBACK_ARGS_DECL) \
	{ \
		nlassert(m_CallbackBase); \
		return m_CallbackBase->callback(PIPELINE_CALLBACK_ARGS_IMPL); \
	} \
	 \
	TReturn operator()(PIPELINE_CALLBACK_ARGS_DECL) \
	{ \
		nlassert(m_CallbackBase); \
		return m_CallbackBase->callback(PIPELINE_CALLBACK_ARGS_IMPL); \
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

#define PIPELINE_CALLBACK_ARGS_CLASS CCallback<TReturn, void, void, void, void, void, void, void, void>
#define PIPELINE_CALLBACK_ARGS_TYPENAME 
#define PIPELINE_CALLBACK_ARGS_DECL 
#define PIPELINE_CALLBACK_ARGS_IMPL 
PIPELINE_CALLBACK_TEMPLATE
#undef PIPELINE_CALLBACK_ARGS_CLASS
#undef PIPELINE_CALLBACK_ARGS_TYPENAME
#undef PIPELINE_CALLBACK_ARGS_DECL
#undef PIPELINE_CALLBACK_ARGS_IMPL

#define PIPELINE_CALLBACK_ARGS_CLASS CCallback<TReturn, TArgsA, void, void, void, void, void, void, void>
#define PIPELINE_CALLBACK_ARGS_TYPENAME , typename TArgsA
#define PIPELINE_CALLBACK_ARGS_DECL TArgsA argsA
#define PIPELINE_CALLBACK_ARGS_IMPL argsA
PIPELINE_CALLBACK_TEMPLATE
#undef PIPELINE_CALLBACK_ARGS_CLASS
#undef PIPELINE_CALLBACK_ARGS_TYPENAME
#undef PIPELINE_CALLBACK_ARGS_DECL
#undef PIPELINE_CALLBACK_ARGS_IMPL

#define PIPELINE_CALLBACK_ARGS_CLASS CCallback<TReturn, TArgsA, TArgsB, void, void, void, void, void, void>
#define PIPELINE_CALLBACK_ARGS_TYPENAME , typename TArgsA, typename TArgsB
#define PIPELINE_CALLBACK_ARGS_DECL TArgsA argsA, TArgsB argsB
#define PIPELINE_CALLBACK_ARGS_IMPL argsA, argsB
PIPELINE_CALLBACK_TEMPLATE
#undef PIPELINE_CALLBACK_ARGS_CLASS
#undef PIPELINE_CALLBACK_ARGS_TYPENAME
#undef PIPELINE_CALLBACK_ARGS_DECL
#undef PIPELINE_CALLBACK_ARGS_IMPL

#define PIPELINE_CALLBACK_ARGS_CLASS CCallback<TReturn, TArgsA, TArgsB, TArgsC, void, void, void, void, void>
#define PIPELINE_CALLBACK_ARGS_TYPENAME , typename TArgsA, typename TArgsB, typename TArgsC
#define PIPELINE_CALLBACK_ARGS_DECL TArgsA argsA, TArgsB argsB, TArgsC argsC
#define PIPELINE_CALLBACK_ARGS_IMPL argsA, argsB, argsC
PIPELINE_CALLBACK_TEMPLATE
#undef PIPELINE_CALLBACK_ARGS_CLASS
#undef PIPELINE_CALLBACK_ARGS_TYPENAME
#undef PIPELINE_CALLBACK_ARGS_DECL
#undef PIPELINE_CALLBACK_ARGS_IMPL

#define PIPELINE_CALLBACK_ARGS_CLASS CCallback<TReturn, TArgsA, TArgsB, TArgsC, TArgsD, void, void, void, void>
#define PIPELINE_CALLBACK_ARGS_TYPENAME , typename TArgsA, typename TArgsB, typename TArgsC, typename TArgsD
#define PIPELINE_CALLBACK_ARGS_DECL TArgsA argsA, TArgsB argsB, TArgsC argsC, TArgsD argsD
#define PIPELINE_CALLBACK_ARGS_IMPL argsA, argsB, argsC, argsD
PIPELINE_CALLBACK_TEMPLATE
#undef PIPELINE_CALLBACK_ARGS_CLASS
#undef PIPELINE_CALLBACK_ARGS_TYPENAME
#undef PIPELINE_CALLBACK_ARGS_DECL
#undef PIPELINE_CALLBACK_ARGS_IMPL

#define PIPELINE_CALLBACK_ARGS_CLASS CCallback<TReturn, TArgsA, TArgsB, TArgsC, TArgsD, TArgsE, void, void, void>
#define PIPELINE_CALLBACK_ARGS_TYPENAME , typename TArgsA, typename TArgsB, typename TArgsC, typename TArgsD, typename TArgsE
#define PIPELINE_CALLBACK_ARGS_DECL TArgsA argsA, TArgsB argsB, TArgsC argsC, TArgsD argsD, TArgsE argsE
#define PIPELINE_CALLBACK_ARGS_IMPL argsA, argsB, argsC, argsD, argsE
PIPELINE_CALLBACK_TEMPLATE
#undef PIPELINE_CALLBACK_ARGS_CLASS
#undef PIPELINE_CALLBACK_ARGS_TYPENAME
#undef PIPELINE_CALLBACK_ARGS_DECL
#undef PIPELINE_CALLBACK_ARGS_IMPL

#define PIPELINE_CALLBACK_ARGS_CLASS CCallback<TReturn, TArgsA, TArgsB, TArgsC, TArgsD, TArgsE, TArgsF, void, void>
#define PIPELINE_CALLBACK_ARGS_TYPENAME , typename TArgsA, typename TArgsB, typename TArgsC, typename TArgsD, typename TArgsE, typename TArgsF
#define PIPELINE_CALLBACK_ARGS_DECL TArgsA argsA, TArgsB argsB, TArgsC argsC, TArgsD argsD, TArgsE argsE, TArgsF argsF
#define PIPELINE_CALLBACK_ARGS_IMPL argsA, argsB, argsC, argsD, argsE, argsF
PIPELINE_CALLBACK_TEMPLATE
#undef PIPELINE_CALLBACK_ARGS_CLASS
#undef PIPELINE_CALLBACK_ARGS_TYPENAME
#undef PIPELINE_CALLBACK_ARGS_DECL
#undef PIPELINE_CALLBACK_ARGS_IMPL

#define PIPELINE_CALLBACK_ARGS_CLASS CCallback<TReturn, TArgsA, TArgsB, TArgsC, TArgsD, TArgsE, TArgsF, TArgsG, void>
#define PIPELINE_CALLBACK_ARGS_TYPENAME , typename TArgsA, typename TArgsB, typename TArgsC, typename TArgsD, typename TArgsE, typename TArgsF, typename TArgsG
#define PIPELINE_CALLBACK_ARGS_DECL TArgsA argsA, TArgsB argsB, TArgsC argsC, TArgsD argsD, TArgsE argsE, TArgsF argsF, TArgsG argsG
#define PIPELINE_CALLBACK_ARGS_IMPL argsA, argsB, argsC, argsD, argsE, argsF, argsG
PIPELINE_CALLBACK_TEMPLATE
#undef PIPELINE_CALLBACK_ARGS_CLASS
#undef PIPELINE_CALLBACK_ARGS_TYPENAME
#undef PIPELINE_CALLBACK_ARGS_DECL
#undef PIPELINE_CALLBACK_ARGS_IMPL
#undef PIPELINE_CALLBACK_ARGS_CLASSNAME

#undef PIPELINE_CALLBACK_TEMPLATE

} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_CALLBACK_H */

/* end of file */
