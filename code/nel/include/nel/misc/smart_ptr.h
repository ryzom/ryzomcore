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

#ifndef NL_SMART_PTR_H
#define NL_SMART_PTR_H


#include "types_nl.h"
#include "debug.h"
#include "stream.h"

#include <cstdio>


namespace NLMISC
{


// ***************************************************************************
/**
 * To use CSmartPtr or CRefPtr, derive from this class.
 * Your class doens't have to be virtual, or doesn't have to provide a virtual dtor.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CRefCount
{
public:
	/// Destructor which release pinfo if necessary.
	~CRefCount();
	/// Default constructor init crefs to 0.
	CRefCount() { crefs = 0; pinfo = &NullPtrInfo; }

	/*  The instance handle.
		Can't put those to private since must be used by CRefPtr (and friend doesn't work with template).
		Use struct CPtrInfoBase / CPtrInfo idiom for NullPtrInfo, because of problems of static constructor:
		NullPtrInfo must be init BEFORE ANY constructor calls.
	*/
	struct	CPtrInfoBase
	{
		const CRefCount* Ptr;	// to know if the instance is valid.
		sint	RefCount;		// RefCount of ptrinfo (!= instance)
		bool	IsNullPtrInfo;	// For dll problems, must use a flag to mark NullPtrInfo.
	};

	struct	CPtrInfo : public CPtrInfoBase
	{
		CPtrInfo(CRefCount const* p) {Ptr=p; RefCount=0; IsNullPtrInfo=false;}
	};

	// OWN null for ref ptr. (Optimisations!!!)
	static	CPtrInfoBase	NullPtrInfo;
	friend struct			CPtrInfo;

	// for special case use only.
	inline	const	sint	&getRefCount()	const
	{
		return	crefs;
	}

public:
	// Can't put this to private since must be used by CSmartPtr (and friend doesn't work with template).
	// Provide incref()/decref() function doen't work since decref() can't do a delete this on a non virtual dtor.
	// So Ptr gestion can only be used via CSmartPtr.
	mutable	sint			crefs;	// The ref counter for SmartPtr use.
	mutable	CPtrInfoBase	*pinfo;	// The ref ptr for RefPtr use.

	/// operator= must NOT copy crefs/pinfo!!
	CRefCount &operator=(const CRefCount &) {return *this;}
	/// copy cons must NOT copy crefs/pinfo!!
	CRefCount(const CRefCount &) { crefs = 0; pinfo = &NullPtrInfo; }
};

// To use CVirtualRefPtr (or if you just want to have a RefCount with virtual destructor), derive from this class.
class CVirtualRefCount : public CRefCount
{
public:
	/// Virtual destructor
	virtual ~CVirtualRefCount() {}
};



// ***************************************************************************
// For debug only.
#define	SMART_TRACE(_s)	((void)0)
#define	REF_TRACE(_s)	((void)0)
//#define	SMART_TRACE(_s)	printf("%s: %d \n", _s, Ptr?Ptr->crefs:0)
//#define	REF_TRACE(_s)	printf("%s: %d \n", _s, pinfo != &CRefCount::NullPtrInfo?pinfo->RefCount:0)


/**
 * Standard SmartPtr class. T Must derive from CRefCount.
 * Once a normal ptr is assigned to a SmartPtr, the smartptr will own this pointer, and delete it when no other smartptr
 * reference the object (with a reference couting scheme). The following code works, since the object himself must herit
 * from CRefCount, and so hold the refcount.
 * \code
	CSmartPtr<A>	a0, a1;
	A				*p0;
	a0= new A;	// Ok. RefCount==1.
	p0= a0;		// Ok, cast operator. object still owned by a0.
	a1= p0;		// Ok!! RefCount==2. Object owned by a0 and a1;
	// At destruction, a1 unref(), then a0 unref() and delete the object.
 \endcode
 *
 * The ref counter cannot be put directly in the smartptr since the preceding behavior must be supported and inheritance must be supported too.
 * Here, if A is a base class of B, Pa and Pb are smartptr of a and b respectively, then \c Pa=Pb; is a valid operation.
 * But, doing this, you may ensure that you have a virtual dtor(), since dtor() Pa may call ~A() (or you may ensure that Pa
 * won't destruct A, which it sound much more as a normal pointer :) ).
 *
 * Sample:
 *\code
	class A : public CRefCount
	{
	public:
		A() {puts("A()");}
		virtual ~A() {puts("~A()");}
	};


	class B : public A
	{
	public:
		B() {puts("B()");}
		~B() {puts("~B()");}
	};


	void	testPtr()
	{
		CSmartPtr<A>	a0,a1,a2;
		CSmartPtr<B>	b0;

		a0= new A;
		a1= a0;
		a1= new A;
		a2= a1;
		a1=NULL;
		b0= new B;
		a0=b0;

		printf("%d\n", (A*)NULL==a0);
		printf("%d\n", b0!=a0);
		printf("%d\n", (A*)NULL==a1);
		printf("%d\n", a2!=a0);
	}
 *\endcode
 *
 * SmartPtr are compatible with RefPtr. A ptr may be link to a CRefPtr and a CSmartPtr. As example, when the CSmartPtr
 * will destroy him, CRefPtr will be informed...
 * Sample:
 *\code
	void	foo()
	{
		A				*p;
		CSmartPtr<A>	sp;
		CRefPtr<A>		rp;

		p= new A;
		sp= p;		// OK. p is now owned by sp and will be deleted by sp.
		rp= p;		// OK. rp handle p.
		sp= NULL;	// Destruction. p deleted. rp automatically informed.
		p= rp;		// result: p==NULL.
	}
 \endcode
 *
 * \b PERFORMANCE \b WARNING! operator=() are about 10 times slower than normal pointers.
 * For local use, prefer cast the smartptr to a normal Ptr.
 * \sa CRefPtr
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
template <class T>
class CSmartPtr
{
    T* Ptr;
public:
	typedef T element_type;

	/// Init a NULL Ptr.
	CSmartPtr() { Ptr=NULL; SMART_TRACE("ctor()"); }
	/// Attach a ptr to a SmartPtr.
	CSmartPtr(T* p) { Ptr=p; if(Ptr) Ptr->crefs++; SMART_TRACE("ctor(T*)"); }
	/// Copy constructor.
	CSmartPtr(const CSmartPtr &copy) { Ptr=copy.Ptr; if(Ptr) Ptr->crefs++; SMART_TRACE("ctor(Copy)"); }
	/// Release the SmartPtr.
	~CSmartPtr();


	/// Cast operator.
	operator T*(void) const { SMART_TRACE("castT*()"); return Ptr; }
	/// Indirection operator. Doesn't check NULL.
	T& operator*(void) const { SMART_TRACE("ope*()"); return *Ptr; }
	/// Selection operator. Doesn't check NULL.
	T* operator->(void) const { SMART_TRACE("ope->()"); return Ptr; }
	/// returns if there's no object pointed by this SmartPtr.
	bool	isNull	() const { return Ptr==NULL; }
	/// Return the pointer
	T *getPtr() const { return Ptr;}

	/// operator=. Giving a NULL pointer is a valid operation.
	CSmartPtr& operator=(T* p);
	/// operator=. Giving a NULL pointer is a valid operation.
	CSmartPtr& operator=(const CSmartPtr &p);
	/// operator<. Compare the pointers.
	bool operator<(const CSmartPtr &p) const;

	sint getNbRef() { if(Ptr) return Ptr->crefs; else return 0; }
	// No need to do any operator==. Leave the work to cast  operator T*(void).

	std::string toString() { if(Ptr) return toString(*Ptr); else return "<null>"; }

	// serial using serialPtr
	void serialPtr(NLMISC::IStream &f)
	{
		T*	obj= NULL;
		if(f.isReading())
		{
			f.serialPtr(obj);
			// assign correctly (NB: obj may be NULL)
			*this= obj;
		}
		else
		{
			obj= Ptr;
			f.serialPtr(obj);
		}
	}
	// serial using serialPloyPtr
	void serialPolyPtr(NLMISC::IStream &f)
	{
		T*	obj= NULL;
		if(f.isReading())
		{
			f.serialPolyPtr(obj);
			// assign correctly (NB: obj may be NULL)
			*this= obj;
		}
		else
		{
			obj= Ptr;
			f.serialPolyPtr(obj);
		}
	}
};



// ***************************************************************************
/**
 * CRefPtr: an handle on a ptr. T Must derive from CRefCount.
 * If you use CRefPtr, you can kill the object simply by calling delete (T*)RefPtr, or the kill() method. All other CRefPtr which
 * point to it can know if it has been deleted. (but you must be sure that this ptr is not handle by a SmartPtr, of course...)
 *
 * SmartPtr are compatible with RefPtr. A ptr may be link to a CRefPtr and a CSmartPtr. As example, when the CSmartPtr
 * will destroy him, CRefPtr will be informed...
 * Sample:
 *\code
	void	foo()
	{
		A				*p;
		CSmartPtr<A>	sp;
		CRefPtr<A>		rp;

		p= new A;
		sp= p;		// OK. p is now owned by sp and will be deleted by sp.
		rp= p;		// OK. rp handle p.
		sp= NULL;	// Destruction. p deleted. rp automatically informed.
		if(rp==NULL)
			thisIsGood();	// rp==NULL.
	}
 \endcode
 *
 * \b PERFORMANCE \b WARNING! operator=() are about 10 times slower than normal pointers. So use them wisely.
 * For local use, prefer cast the refptr to a normal Ptr.
 * Also, an object used with a CRefPtr will allocate a small PtrInfo (one only per object, not per ptr).
 * \sa CSmartPtr
 */
template <class T>
class CRefPtr
{
private:
	CRefCount::CPtrInfoBase	*pinfo;		// A ptr to the handle of the object.

	mutable T				*Ptr;		// A cache for pinfo->Ptr. Useful to speed up  ope->()  and  ope*()

	void	unRef()  const;				// Just release the handle pinfo, but do not update pinfo/Ptr, if deleted.

public:

	/// Init a NULL Ptr.
	CRefPtr();
	/// Attach a ptr to a RefPtr.
	CRefPtr(T *v);
	/// Copy constructor.
	CRefPtr(const CRefPtr &copy);
	/// Release the RefPtr.
	~CRefPtr(void);


	/// Cast operator. Check if the object has been deleted somewhere, and return NULL if this is the case.
	operator T*()	const;
	/// Indirection operator. Doesn't test if ptr has been deleted somewhere, and doesn't check NULL.
	T& operator*(void)	const;
	/// Selection operator. Doesn't test if ptr has been deleted somewhere, and doesn't check NULL.
	T* operator->(void)	const;


	/// operator=. Giving a NULL pointer is a valid operation.
	CRefPtr& operator=(T *v);
	/// operator=. Giving a NULL pointer is a valid operation.
	CRefPtr& operator=(const CRefPtr &copy);


	/**
	 * kill/delete the object pointed by the pointer, and inform the other RefPtr of this.
	 * "rp.kill()" and "delete (T*)rp" do the same thing, except that rp NULLity is updated with kill().
	 * RefPtr which point to the same object could know if the object is valid, by just testing it (
	 * by an implicit call to the cast operator to T*). But any calls to operator->() or operator*() will have
	 * unpredictible effects (may crash... :) ).
	 */
	void	kill();

	// serial using serialPloyPtr
	void serialPolyPtr(NLMISC::IStream &f)
	{
		T*	obj= NULL;
		if(f.isReading())
		{
			f.serialPolyPtr(obj);
			// assign correctly (NB: obj may be NULL)
			*this= obj;
		}
		else
		{
			obj= Ptr;
			f.serialPolyPtr(obj);
		}
	}
};

#if defined(NL_COMP_VC) && NL_COMP_VC_VERSION >= 80 && NL_COMP_VC_VERSION < 140

// This operator only purpose is to compare with NULL value
template <class T>
bool operator == (const CRefPtr<T> &refPtr, int null)
{
	nlassert(null == 0);
	return (T*)refPtr == (T*)null;
}

#endif

template <class T>
bool operator == (const CRefPtr<T> &refPtr, T *ptr)
{
	return (T*)refPtr == ptr;
}

template <class T>
bool operator == (const CRefPtr<T> &leftRef, const CRefPtr<T> &rightRef)
{
	return (T*)leftRef == (T*) rightRef;
}

template <class T>
class CVirtualRefPtr
{
private:
	CRefCount::CPtrInfoBase	*pinfo;		// A ptr to the handle of the object.
    mutable T				*Ptr;		// A cache for pinfo->Ptr. Useful to speed up  ope->()  and  ope*()

	void	unRef()  const;				// Just release the handle pinfo, but do not update pinfo/Ptr, if deleted.

public:

	/// Init a NULL Ptr.
	CVirtualRefPtr();
	/// Attach a ptr to a VirtualRefPtr.
	CVirtualRefPtr(T *v);
	/// Copy constructor.
	CVirtualRefPtr(const CVirtualRefPtr &copy);
	/// Release the VirtualRefPtr.
	~CVirtualRefPtr(void);


	/// Cast operator. Check if the object has been deleted somewhere, and return NULL if this is the case.
	operator T*()	const;
	/// Indirection operator. Doesn't test if ptr has been deleted somewhere, and doesn't check NULL.
	T& operator*(void)	const;
	/// Selection operator. Doesn't test if ptr has been deleted somewhere, and doesn't check NULL.
	T* operator->(void)	const;


	/// operator=. Giving a NULL pointer is a valid operation.
	CVirtualRefPtr& operator=(T *v);
	/// operator=. Giving a NULL pointer is a valid operation.
	CVirtualRefPtr& operator=(const CVirtualRefPtr &copy);


	/**
	 * kill/delete the object pointed by the pointer, and inform the other VirtualRefPtr of this.
	 * "rp.kill()" and "delete (T*)rp" do the same thing, except that rp NULLity is updated with kill().
	 * VirtualRefPtr which point to the same object could know if the object is valid, by just testing it (
	 * by an implicit call to the cast operator to T*). But any calls to operator->() or operator*() will have
	 * unpredictible effects (may crash... :) ).
	 */
	void	kill();


	// No need to do any operator==. Leave the work to cast  operator T*(void).
};


	/**
	 * A little quick coded class made to find invalid object pointer existence while a destructor call on an objet.
	 * Futur job is to do not have CstCDbgPtr .. (error due to a lack of time).
	 *
	 * Feature:
	 *   If you try to delete an object and if there still some pointer referencing it, it causes an assertion.
	 *   This Debug feature depends on NL_DEBUG_PTR definition (change it as your convenience).
	 *   These are only debug classes and are not design to use information for your code comportment
	 *  as they are made to desappear in final version (NL_DEBUG_PTR undefined),
	 *  so don't use method calls outside NL_DEBUG_PTR scope.
	 *
	 * How to:
	 *   Derivate your objet from CDbgRefCount<T> where T is the type of pointer you allow to use on this object (you can use many).
	 *  then use CDbgPtr<T> to point an object of type T (which must derivates from CDbgRefCount<T>), its the pointer.
	 *
	 * Warning:
	 *  Be carefull to derivates first from CDbgRefCount<T> as derivation order implicates constructor/destructor order calls.
	 *  Sometimes u may have to write some explicit casts, don't worry, it would be only common courtesy ;)
	 *
	 * Futur work (only if you need it):
	 *  enhanced features (like __FILE__ __LINE__ information).
	 *
	 * Extension by Jerome Vuarand
	 *  I've added additional information to trace back invalid pointers. A linked list in the ref counter can be used to access to
	 *  invalid pointers on referenced object deletion, and an additionnal data field in each pointer let pointer owner to specify
	 *  additionnal information. The linked list insertion occurs on head. The linked is doubly-linked to avoid parsing it on deletion.
	 *  When a reference is still present on referenced object deletion, an assert is issued and access to pointers is given. Each
	 *  CDbgPtr owner can attach a data pointer to the CDbgPtr. A good way to use the feature is to derive the owner from IDbgPtrData,
	 *  pass this as the data to the CDbgPtr, and to let RTTI find the owner during assert manual handling.
	 *
	 * \author Stephane Le Dorze
	 * \author Jerome Vuarand
	 * \author Nevrax France
	 * \date 2003-2005
	 */


#ifdef NL_DEBUG
	#define NL_DEBUG_PTR
#endif

template <class T>	class CDbgPtr;
template <class T>	class CstCDbgPtr;

class IDbgPtrData
{
public:
	virtual ~IDbgPtrData() { }
};

template <class T>
class CDbgRefCount
{
#ifdef NL_DEBUG_PTR
public:
	CDbgRefCount(const CDbgRefCount& other)
	: _DbgCRefs(0)
	, _DbgCCstRefs(0)
	, _MaxRef(other._MaxRef)
	, _CheckOn(other._CheckOn)
	, _FirstReference(NULL)
	, _FirstCstReference(NULL)
	{
	}
	CDbgRefCount(sint32 maxRef = (1<<30))
	: _DbgCRefs(0)
	, _DbgCCstRefs(0)
	, _MaxRef(maxRef)
	, _FirstReference(NULL)
	, _FirstCstReference(NULL)
	{
	}
	virtual	~CDbgRefCount()
	{
		if (_DbgCRefs!=0 || _DbgCCstRefs!=0)
		{
			const CDbgPtr<T>* ref0, *ref1, *ref2, *ref3, *ref4; ref0=ref1=ref2=ref3=ref4=(CDbgPtr<T>*)NULL;
			IDbgPtrData* dat0, *dat1, *dat2, *dat3, *dat4; dat0=dat1=dat2=dat3=dat4=(IDbgPtrData*)NULL;
			if (_DbgCRefs>0) { ref0 = _FirstReference; dat0 = ref0->getData(); }
			if (_DbgCRefs>1) { ref1 = ref0->getNextReference(); dat1 = ref1->getData(); }
			if (_DbgCRefs>2) { ref2 = ref1->getNextReference(); dat2 = ref2->getData(); }
			if (_DbgCRefs>3) { ref3 = ref2->getNextReference(); dat3 = ref3->getData(); }
			if (_DbgCRefs>4) { ref4 = ref3->getNextReference(); dat4 = ref4->getData(); }
			const CstCDbgPtr<T>* cref0, *cref1, *cref2, *cref3, *cref4; cref0=cref1=cref2=cref3=cref4=(CstCDbgPtr<T>*)NULL;
			IDbgPtrData* cdat0, *cdat1, *cdat2, *cdat3, *cdat4; cdat0=dat1=cdat2=cdat3=cdat4=(IDbgPtrData*)NULL;
			if (_DbgCCstRefs>0) { cref0 = _FirstCstReference; cdat0 = cref0->getData(); }
			if (_DbgCCstRefs>1) { cref1 = cref0->getNextReference(); cdat1 = cref1->getData(); }
			if (_DbgCCstRefs>2) { cref2 = cref1->getNextReference(); cdat2 = cref2->getData(); }
			if (_DbgCCstRefs>3) { cref3 = cref2->getNextReference(); cdat3 = cref3->getData(); }
			if (_DbgCCstRefs>4) { cref4 = cref3->getNextReference(); cdat4 = cref4->getData(); }
			nlassert(_DbgCRefs==0);
			nlassert(_DbgCCstRefs==0);
		}
	}
	sint getDbgRef(const CDbgPtr<T>& ptr) const
	{
		return _DbgCRefs + _DbgCCstRefs;
	}
	sint getDbgRef(const CstCDbgPtr<T>& ptr) const
	{
		return _DbgCRefs + _DbgCCstRefs;
	}
	void incRef(const CDbgPtr<T>& ptr) const
	{
		if (_CheckOn)
			nlassert(_DbgCRefs<_MaxRef);
		++_DbgCRefs;
		// Linked list management
		nlassert(_FirstReference!=&ptr);
		ptr.setNextReference(_FirstReference);
		ptr.setPrevReference((CDbgPtr<T>*)NULL);
		if (_FirstReference)
			_FirstReference->setPrevReference(&ptr);
		_FirstReference = &ptr;
	}
	void decRef(const CDbgPtr<T>& ptr) const
	{
		nlassert(_DbgCRefs>0);
		--_DbgCRefs;
		// Linked list management
		if (ptr.getNextReference())
			ptr.getNextReference()->setPrevReference(ptr.getPrevReference());
		if (ptr.getPrevReference())
			ptr.getPrevReference()->setNextReference(ptr.getNextReference());
		if (_FirstReference==&ptr)
			_FirstReference = ptr.getNextReference();
	}
	void incRef(const CstCDbgPtr<T>& ptr) const
	{
		if (_CheckOn)
			nlassert(_DbgCCstRefs<_MaxRef);
		++_DbgCCstRefs;
	}
	void decRef(const CstCDbgPtr<T>& ptr) const
	{
		nlassert(_DbgCCstRefs>0);
		--_DbgCCstRefs;
	}
	void setCheckMax(const bool checkOnOff) const
	{
		_CheckOn = checkOnOff;
	}
private:
	mutable	sint	_DbgCRefs;
	mutable	sint	_DbgCCstRefs;
	sint32			_MaxRef;
	mutable	bool	_CheckOn;
	mutable	const CDbgPtr<T>*		_FirstReference;
	mutable	const CstCDbgPtr<T>*	_FirstCstReference;
#endif
};

template <class T>
class CDbgPtr
{
	T* Ptr;

#ifdef NL_DEBUG_PTR
	/// \name Linked list management
	//@{
private: // Data
	IDbgPtrData*				Data;
	mutable const CDbgPtr<T>*	NextReference;
	mutable const CDbgPtr<T>*	PrevReference;
public: // Methods
	void setData(IDbgPtrData* data) { Data = data; }
	IDbgPtrData* getData() const { return Data; }
	void setNextReference(const CDbgPtr<T>* reference) const { NextReference = reference; }
	const CDbgPtr<T>* getNextReference() const { return NextReference; }
	void setPrevReference(const CDbgPtr<T>* reference) const { PrevReference = reference; }
	const CDbgPtr<T>* getPrevReference() const { return PrevReference; }
	//@}
#endif

public:
	CDbgPtr()
	: Ptr(NULL)
#ifdef NL_DEBUG_PTR
	, Data(NULL)
	, NextReference(NULL)
	, PrevReference(NULL)
#endif
	{
	}

	template <class W>
	CDbgPtr(const W* p)
#ifdef NL_DEBUG_PTR
	: Data(NULL)
	, NextReference(NULL)
	, PrevReference(NULL)
#endif
	{
		Ptr = const_cast<T*>(NLMISC::type_cast<const T*>(p));
	#ifdef NL_DEBUG_PTR
		if (Ptr)
		{
			CDbgRefCount<T>* ref = static_cast<CDbgRefCount<T>*>(Ptr);
			ref->incRef(*this);
		}
	#endif
	}

	CDbgPtr(const CDbgPtr& copy)
#ifdef NL_DEBUG_PTR
	: Data(NULL)
	, NextReference(NULL)
	, PrevReference(NULL)
#endif
	{
		Ptr = copy.Ptr;
	#ifdef NL_DEBUG_PTR
		if (Ptr)
		{
			CDbgRefCount<T>* ref = static_cast<CDbgRefCount<T>*>(Ptr);
			ref->incRef(*this);
		}
	#endif
	}
	~CDbgPtr();

	bool isNULL() const
	{
		return Ptr==NULL;
	}

	T* ptr() const
	{
		return Ptr;
	}

	operator T*(void) const
	{
		return Ptr;
	}

	template <class W>
	operator W*(void) const
	{
		return NLMISC::type_cast<W*>(Ptr);
	}

	T& operator*(void) const { return *Ptr; }
	T* operator->(void) const { return Ptr; }

	template <class W>
	CDbgPtr<T>& operator=(const W* p)
	{
	#ifdef NL_DEBUG_PTR
		CDbgRefCount<T>* oldRef = (CDbgRefCount<T>*)NULL, *newRef = (CDbgRefCount<T>*)NULL;
		if (Ptr)
			oldRef = static_cast<CDbgRefCount<T>*>(Ptr);
		if (p)
			newRef = static_cast<CDbgRefCount<T>*>(const_cast<T*>(NLMISC::type_cast<const T*>(p)));
		if (oldRef!=newRef)
		{
			if (oldRef) oldRef->decRef(*this);
			Ptr = const_cast<T*>(NLMISC::type_cast<const T*>(p));
			if (newRef) newRef->incRef(*this);
		}
	#else
		Ptr = const_cast<T*>(NLMISC::type_cast<const T*>(p));
	#endif
		return *this;
	}
	/*
	CDbgPtr<T>& operator=(const int value)
	{
	#ifdef NL_DEBUG
		nlassert(value==NULL);
	#endif

	#ifdef NL_DEBUG_PTR
		if (Ptr)
		{
			CDbgRefCount<T>* ref=static_cast<CDbgRefCount<T>*>(Ptr);
			ref->decRef(*this);
		}
	#endif
		Ptr = NULL;
		return *this;
	}
	*/
	CDbgPtr<T>& operator =(const CDbgPtr& p);
	bool operator <(const CDbgPtr& p) const;

	template <class W>
	bool operator ==(const W* p) const
	{
		return Ptr==NLMISC::type_cast<const T*>(p);
	}
	template <class W>
	bool operator !=(const W* p) const
	{
		return Ptr!=NLMISC::type_cast<const T*>(p);
	}

	bool operator ==(const CDbgPtr &p) const
	{
		return Ptr==p.Ptr;
	}
	bool operator !=(const CDbgPtr &p) const
	{
		return Ptr!=p.Ptr;
	}

	bool operator ==(int p) const
	{
		nlassert(p == 0);
		return Ptr==0;
	}
	bool operator !=(int p) const
	{
		nlassert(p == 0);
		return Ptr!=0;
	}
};

template<class T>
inline CDbgPtr<T>::~CDbgPtr(void)
{
#ifdef NL_DEBUG_PTR
	if (Ptr)
	{
		CDbgRefCount<T>* ref = static_cast<CDbgRefCount<T>*>(Ptr);
		ref->decRef(*this);
		Ptr = NULL;
	}
#else
	Ptr=NULL;
#endif
}


template<class T>
CDbgPtr<T>& CDbgPtr<T>::operator =(const CDbgPtr& p)
{
	return CDbgPtr<T>::operator =(p.Ptr);
}

template<class T>
bool CDbgPtr<T>::operator <(const CDbgPtr& p) const
{
	return Ptr<p.Ptr;
}

template <class T>
class CstCDbgPtr
{
	const T* Ptr;

#ifdef NL_DEBUG_PTR
	/// \name Linked list management
	//@{
private: // Data
	IDbgPtrData*					Data;
	mutable const CstCDbgPtr<T>*	NextReference;
	mutable const CstCDbgPtr<T>*	PrevReference;
public: // Methods
	void setData(IDbgPtrData* data) { Data = data; }
	IDbgPtrData* getData() const { return Data; }
	void setNextReference(const CstCDbgPtr<T>* reference) const { NextReference = reference; }
	const CstCDbgPtr<T>* getNextReference() const { return NextReference; }
	void setPrevReference(const CstCDbgPtr<T>* reference) const { PrevReference = reference; }
	const CstCDbgPtr<T>* getPrevReference() const { return PrevReference; }
	//@}
#endif

public:
	CstCDbgPtr()
	: Ptr(NULL)
#ifdef NL_DEBUG_PTR
	, Data(NULL)
	, NextReference(NULL)
	, PrevReference(NULL)
#endif
	{
	}
	CstCDbgPtr(const T* p)
#ifdef NL_DEBUG_PTR
	: Data(NULL)
	, NextReference(NULL)
	, PrevReference(NULL)
#endif
	{
		Ptr = p;
	#ifdef NL_DEBUG_PTR
		if (Ptr)
		{
			CDbgRefCount<T>* ref = static_cast<CDbgRefCount<T>*>(const_cast<T*>(Ptr));
			ref->incRef(*this);
		}
	#endif
	}
	CstCDbgPtr(const CstCDbgPtr& copy)
#ifdef NL_DEBUG_PTR
	: Data(NULL)
	, NextReference(NULL)
	, PrevReference(NULL)
#endif
	{
		Ptr = copy.Ptr;
	#ifdef NL_DEBUG_PTR
		if (Ptr)
		{
			CDbgRefCount<T>* ref = static_cast<CDbgRefCount<T>*>(const_cast<T*>(Ptr));
			ref->incRef(*this);
		}
	#endif
	}
	~CstCDbgPtr();

	const T* ptr() const
	{
		return Ptr;
	}

	bool isNULL() const
	{
		return Ptr==NULL;
	}

	operator const T*(void) const { return Ptr; }
	const T& operator*(void) const { return *Ptr; }
	const T* operator->(void) const { return Ptr; }

	CstCDbgPtr& operator =(const T* p);
	CstCDbgPtr& operator =(const CstCDbgPtr& p);
	bool operator <(const CstCDbgPtr& p) const;
};


template<class T>
CstCDbgPtr<T>::~CstCDbgPtr(void)
{
#ifdef NL_DEBUG_PTR
	if (Ptr)
	{
		CDbgRefCount<T>* ref = static_cast<CDbgRefCount<T>*>(const_cast<T*>(Ptr));
		ref->decRef(*this);
	}
#endif
	Ptr = NULL;
}

template<class T>
CstCDbgPtr<T>& CstCDbgPtr<T>::operator =(const T* p)
{
#ifdef NL_DEBUG_PTR
	if (p)
	{
		CDbgRefCount<T>* ref = static_cast<CDbgRefCount<T>*>(const_cast<T*>(p));
		ref->incRef(*this);
	}
	if (Ptr)
	{
		CDbgRefCount<T>* ref = static_cast<CDbgRefCount<T>*>(const_cast<T*>(Ptr));
		ref->decRef(*this);
	}
#endif
	Ptr = p;
	return *this;
}

template<class T>
CstCDbgPtr<T>& CstCDbgPtr<T>::operator =(const CstCDbgPtr& p)
{
	return CstCDbgPtr<T>::operator =(p.Ptr);
}

template<class T>
bool CstCDbgPtr<T>::operator <(const CstCDbgPtr& p) const
{
	return Ptr<p.Ptr;
}

}


// ***************************************************************************
// ***************************************************************************
// Implementation.
// ***************************************************************************
// ***************************************************************************


#include "smart_ptr_inline.h"
#undef	SMART_TRACE
#undef	REF_TRACE



#endif // NL_SMART_PTR_H

/* End of smart_ptr.h */
