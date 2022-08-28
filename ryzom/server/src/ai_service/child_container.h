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

#ifndef RYAI_CHILD_CONTAINER
#define RYAI_CHILD_CONTAINER

#include "alias_tree_owner.h"

//////////////////////////////////////////////////////////////////////////////
// Class made to manage hierarchical objets .. supports IALiasChild interface.
//////////////////////////////////////////////////////////////////////////////

template <class TChld>
class CCont;


//////////////////////////////////////////////////////////////////////////////
// CChild                                                                   //
//////////////////////////////////////////////////////////////////////////////

/// A named based hierarchy child.
/**	It references an owner, which can change during object lifetime. It also
	have an index that is the position of the object in owner's container.
*/
template <class TPrnt>
class CChild
{
public:
	explicit CChild(TPrnt* owner, uint32 index = -1);
	virtual ~CChild() { }
	
	/// @name Virtual interface
	//@{
//	virtual std::string getIndexString() const = 0;
//	virtual std::string getEntityIdString() const { return NLMISC::CEntityId().toString(); }
//	virtual std::string getOneLineInfoString() const = 0;
//	virtual std::vector<std::string> getMultiLineInfoString() const = 0;
//	virtual std::string getFullName() const = 0;
//	virtual std::string getName() const = 0;
	//@}
	
	TPrnt* getOwner() const { return _Owner; }
	void setOwner(TPrnt* owner) { _Owner = owner; }
	
	uint32 getChildIndex() const { return _Index; }
	void setChildIndex(uint32 index) { _Index = index; }
	
private:
	TPrnt* _Owner;
	uint32 _Index;
};

//////////////////////////////////////////////////////////////////////////////
// CAliasChild                                                              //
//////////////////////////////////////////////////////////////////////////////

/// A named based aliased hierarchy child.
template <class TPrnt>
class CAliasChild
: public CChild<TPrnt>
, public CAliasTreeOwner
{
public:
	explicit CAliasChild(TPrnt* owner, CAIAliasDescriptionNode* node);
	explicit CAliasChild(TPrnt* owner, uint32 alias, std::string const& name);
};

//////////////////////////////////////////////////////////////////////////////
// CCont                                                                    //
//////////////////////////////////////////////////////////////////////////////

/// A vector container
/// ( indirect use of vector to ensure constraint like Smart Pointers ).
template <class TChld>
class CCont
{
public:
	typedef TChld value_type;
	
	/// @name Special iterator skipping NULL elements
	//@{
	class base_iterator : public std::iterator<std::forward_iterator_tag, TChld>
	{
	public:
		explicit base_iterator()
		: _cont(NULL)
		{
			_index = -1;
		}
		base_iterator& operator=(base_iterator const& other)
		{
			_index = other._index;
			_cont = other._cont;
			return *this;
		}
		void operator++()
		{
			size_t size = _cont->_Childs.size();
			++_index;
			while (size_t(_index)<=(size-1) && _cont->_Childs[size_t(_index)]==NULL)
				++_index;
		}
		void operator--()
		{
			size_t size = _cont->_Childs.size();
			++_index;
			if (_index>=size)
				_index = size-1;
			while (_index>=0 && _cont->_Childs[_index]==NULL)
				--_index;
		}
		
		bool operator==(base_iterator const& other) const
		{
			return _index==other._index && _cont==other._cont;
		}
		bool operator!=(base_iterator const& other) const
		{
			return _index!=other._index || _cont!=other._cont;
		}
		
	protected:
		explicit base_iterator(CCont<TChld> const* cont, bool end = false)
		: _cont(cont)
		{
			int	size = (int)_cont->_Childs.size();
			if (end)
			{
				_index = size;
			}
			else
			{
				_index = 0;
				while (_index<=(size-1) && !_cont->_Childs[_index])
					++_index;
			}
		}
		explicit base_iterator(CCont<TChld> const* cont, TChld const* child)
		: _cont(cont)
		{
			if (!child || &child->getOwner()!=_cont)
				_index = -1;
			else
				_index = child->getIndex();
		}
	protected:
		int					_index;
		CCont<TChld> const*	_cont;
	};
	class iterator
	: public base_iterator
	{
		friend class CCont<TChld>;
	public:
		explicit iterator()
		: base_iterator()
		{
		}
		TChld* operator*()
		{
			return this->_cont->_Childs[this->_index];
		}
		TChld* operator->() const
		{
			return this->_cont->_Childs[this->_index];
		}
		
		iterator& operator=(iterator const& other)
		{
			base_iterator::operator=(other);
			return *this;
		}
		bool operator==(iterator const& other) const
		{
			return base_iterator::operator==(other);
		}
		bool operator!=(iterator const& other) const
		{
			return base_iterator::operator!=(other);
		}
		
	private:
		explicit iterator(CCont<TChld>* cont, bool end = false)
		: base_iterator(cont, end)
		{
		}
		explicit iterator(CCont<TChld> const* cont,	TChld const* child)
		: base_iterator(cont, child)
		{
		}
	};
	class const_iterator
	: public base_iterator
	{
		friend class CCont<TChld>;
	public:
		explicit const_iterator()
		: base_iterator()
		{
		}
		TChld const* operator*() const
		{
			return this->_cont->_Childs[this->_index];
		}
		TChld const* operator->() const
		{
			return this->_cont->_Childs[this->_index];
		}
		const_iterator& operator=(base_iterator const& other)
		{
			base_iterator::operator=(other);
			return *this;
		}
		bool operator==(const_iterator const& other) const
		{
			return base_iterator::operator==(other);
		}
		bool operator!=(const_iterator const& other) const
		{
			return base_iterator::operator!=(other);
		}
		
	private:
		explicit const_iterator(CCont<TChld> const* cont, bool end = false)
		: base_iterator(cont, end)
		{
		}
		explicit const_iterator(CCont<TChld> const* cont, TChld const* child)
		: base_iterator(cont, child)
		{
		}
	};
	//@}
	
	friend class base_iterator;
	friend class iterator;
	friend class const_iterator;
	
public:
	explicit CCont();
	virtual	~CCont();
	
	void clear() { _Childs.clear(); }
	void setChildSize(size_t newSize);
	TChld* addChild(TChld* child, uint32 index);
	// Reserved space, not really used.
	size_t size() const { return _Childs.size(); }
	
	TChld* operator[](uint32 index) const { return getChild(index); }
	
	void removeChildByIndex(size_t index);
	
	TChld* addChild(TChld* child);
	
	iterator begin() { return iterator(this); }
	const_iterator begin() const { return const_iterator(this); }
	
	iterator end() { return iterator(this, true); }
	const_iterator end() const { return const_iterator(this, true); }
	
	iterator find(TChld const* child) { return iterator(this, child); }
	const_iterator find(TChld const* child) const { return const_iterator(this,child); }
	
	// :OBSOLETE:
	TChld* getNextValidChild(TChld* child = NULL);
	
	bool isEmpty() const;
	
	void swap(CCont<TChld>& other) { other._Childs.swap(_Childs); }
	
	std::vector<NLMISC::CSmartPtr<TChld> >& getInternalCont() { return _Childs; }
	
protected:
	uint32 getFirstFreeChild();
	
private:
	TChld* getChild(uint32 index) const;
	
protected:
	typedef std::vector<NLMISC::CSmartPtr<TChld> > TChildCont;
	TChildCont _Childs;
};

//////////////////////////////////////////////////////////////////////////////
// CAliasCont                                                               //
//////////////////////////////////////////////////////////////////////////////

template <class TChld>
class CAliasCont
: public CCont<TChld>
, public IAliasCont
{
public:
	explicit CAliasCont();
	
	uint32 size() const { return (uint32)this->_Childs.size(); }
	
	uint32 getChildIndexByAlias(uint32 alias) const;
	TChld* getChildByAlias(uint32 alias) const;
	TChld* getChildByName(std::string const& name) const;
	TChld* getFirstChild() const;
	
	CAliasTreeOwner* getAliasChildByAlias(uint32 alias) const;
	CAliasTreeOwner* addAliasChild(CAliasTreeOwner* child);
	CAliasTreeOwner* addAliasChild(CAliasTreeOwner* child, uint32 index);
	
	void removeChildByAlias(uint32 alias);
	void removeChildByIndex(uint32 index);
};

/****************************************************************************/
/* Inlined methods                                                          */
/****************************************************************************/

//////////////////////////////////////////////////////////////////////////////
// CChild                                                                   //
//////////////////////////////////////////////////////////////////////////////

template <class TPrnt>
CChild<TPrnt>::CChild(TPrnt* owner, uint32 index)
: _Owner(owner)
, _Index(index)
{
}
/*
template <class TPrnt>
std::string CChild<TPrnt>::getOneLineInfoString() const
{
	return std::string("No info available");
}

template <class TPrnt>
std::vector<std::string> CChild<TPrnt>::getMultiLineInfoString() const
{
	std::vector<std::string> container;
	container.push_back(std::string("No info available"));
	return container;
}
*/
//////////////////////////////////////////////////////////////////////////////
// CAliasChild                                                              //
//////////////////////////////////////////////////////////////////////////////

template <class TPrnt>
CAliasChild<TPrnt>::CAliasChild(TPrnt* owner, CAIAliasDescriptionNode* node)
: CChild<TPrnt>(owner)
, CAliasTreeOwner(node)
{
}

template <class TPrnt>
CAliasChild<TPrnt>::CAliasChild(TPrnt* owner, uint32 alias, std::string const& name)
: CChild<TPrnt>(owner)
, CAliasTreeOwner(alias, name)
{
}

//////////////////////////////////////////////////////////////////////////////
// CCont                                                                    //
//////////////////////////////////////////////////////////////////////////////

template <class TChld>
CCont<TChld>::CCont()
{
	_Childs.resize(0);
}

template <class TChld>
CCont<TChld>::~CCont()
{
	setChildSize(0);
	_Childs.clear();
}

template <class TChld>
void CCont<TChld>::setChildSize(size_t newSize)
{
	// have to erase some children
	if (newSize<_Childs.size())
	{
		size_t size = _Childs.size();
		for	(size_t	i=newSize; i<size; ++i)
			_Childs[i] = NULL; // automatic smart-pointed object deletion.
	}
	_Childs.resize(newSize, NLMISC::CSmartPtr<TChld>());
}

template <class TChld>
TChld* CCont<TChld>::addChild(TChld* child, uint32 index)
{		
	if (_Childs.size()<=index)
	{
		_Childs.resize(index+1, NLMISC::CSmartPtr<TChld>());
	}
	
	_Childs[index] = child;
	if (child)
		child->setChildIndex(index);
	return child;
}

template <class TChld>
void CCont<TChld>::removeChildByIndex(size_t index)
{
	nlassert(index<_Childs.size());
	_Childs[index] = NULL;
}	

template <class TChld>
TChld* CCont<TChld>::addChild(TChld* child)
{
	return addChild(child, getFirstFreeChild());
}

template <class TChld>
TChld* CCont<TChld>::getNextValidChild(TChld* child)
{
	size_t childCount = _Childs.size();
	size_t index = 0;
	if (child!=NULL)
		index = child->getChildIndex()+1;
	
	for	(; index<childCount; ++index)
	{
		if (_Childs[index])
			return _Childs[index];
	}
	return NULL;
}

template <class TChld>
bool CCont<TChld>::isEmpty() const
{
	size_t childCount = _Childs.size();
	size_t index = 0;
	for	(; index<childCount; ++index)
	{
		if (_Childs[index])
			return false;
	}
	return true;
}

template <class TChld>
uint32 CCont<TChld>::getFirstFreeChild()
{
	size_t childCount = _Childs.size();
	size_t index = 0;		
	for	(; index<childCount; ++index)
	{
		if (!_Childs[index])
			break;
	}
	return (uint32)index;
}

template <class TChld>
TChld* CCont<TChld>::getChild(uint32 index) const
{
	if (index >= _Childs.size())
		return NULL;
	return _Childs[index];
}

//////////////////////////////////////////////////////////////////////////////
// CAliasCont                                                               //
//////////////////////////////////////////////////////////////////////////////

template <class TChld>
CAliasCont<TChld>::CAliasCont()
: CCont<TChld>()
{
}

template <class TChld>
uint32 CAliasCont<TChld>::getChildIndexByAlias(uint32 alias) const
{
	size_t size = this->_Childs.size();
	for	(size_t	i=0; i<size; ++i)
	{
		TChld* child = this->_Childs[i];
		if (child!=NULL && child->getAlias()==alias)
			return (uint32)i;
	}
	return std::numeric_limits<uint32>::max();
}

template <class TChld>
TChld* CAliasCont<TChld>::getChildByAlias(uint32 alias) const
{
	size_t size = this->_Childs.size();
	for	(size_t	i=0; i<size; ++i)
	{
		TChld* child = this->_Childs[i];
		if (child!=NULL && child->getAlias()==alias)
			return child;
	}
	return NULL;
}

template <class TChld>
TChld* CAliasCont<TChld>::getChildByName(std::string const& name) const
{
	size_t size = this->_Childs.size();
	for	(size_t	i=0; i<size; ++i)
	{
		TChld* child = this->_Childs[i];
		if (child!=NULL && child->getName()==name)
			return child;
	}
	return NULL;
}

template <class TChld>
TChld* CAliasCont<TChld>::getFirstChild() const
{
	size_t size = this->_Childs.size();
	for	(size_t	i=0; i<size; ++i)
	{
		TChld* child = this->_Childs[i];
		if (child!=NULL)
			return child;
	}
	return NULL;
}

template <class TChld>
CAliasTreeOwner* CAliasCont<TChld>::getAliasChildByAlias(uint32 alias) const
{
	return NLMISC::type_cast<CAliasTreeOwner*>(getChildByAlias(alias));
}

template <class TChld>
CAliasTreeOwner* CAliasCont<TChld>::addAliasChild(CAliasTreeOwner* child)
{
	return NLMISC::type_cast<CAliasTreeOwner*>(this->addChild(static_cast<TChld*>(child)));
}

template <class TChld>
CAliasTreeOwner* CAliasCont<TChld>::addAliasChild(CAliasTreeOwner* child, uint32 index)
{
	return NLMISC::type_cast<CAliasTreeOwner*>(this->addChild(static_cast<TChld*>(child), index));
}

template <class TChld>
void CAliasCont<TChld>::removeChildByAlias(uint32 alias)
{
	CCont<TChld>::removeChildByIndex(getChildIndexByAlias(alias));
}

template <class TChld>
void CAliasCont<TChld>::removeChildByIndex(uint32 index)
{
	CCont<TChld>::removeChildByIndex(index);
}

#endif
