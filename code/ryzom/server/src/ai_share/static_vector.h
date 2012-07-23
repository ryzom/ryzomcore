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




#ifndef RYAI_STATIC_VECTOR_H
#define RYAI_STATIC_VECTOR_H


/*

  This class implements an array of entities of any derived class of a given base class
  The entities are allocated in a single memory block.

*/

//	not the way to use templates .. ( bad generalisation implementation ).

/*
template <class BaseClass> 
class StaticVector
{
public:
	// ctor & dtor --------------------------------------------------
	StaticVector(): _data(0), _count(0), _size(0) {}
	~StaticVector() { if (_data) delete[] _data; }

	// allocate memory and initialise objects -----------------------
	template <class DerivedClass>
	void init(uint count,DerivedClass *&dc)
	{
		nlassert(_data==NULL);
		_count=count;
		
		_data=(BaseClass*)malloc(count*sizeof(DerivedClass));
		_size=sizeof(DerivedClass);
		nlassert(_data!=NULL);
		
		for (uint i=0;i<count;++i)
		{
//			std::construct(&((DerivedClass*)_data)[i], DerivedClass());
#undef new
			new (&((DerivedClass*)_data)[i]) DerivedClass();
#define new NL_NEW
		}
	}

	// destroy array of objects and free memory ---------------------
	void release()
	{
		_count=0;
		_size=0;
		if (_data)
			delete[] _data;
	}

	// return number of objects in array or 0 if not initialised ----
	uint count() const
	{
		return _count;
	}

	// [] operator for accessing data -------------------------------
	BaseClass &operator[](uint index) const
	{
		nlassert( index < _count );
		return *(BaseClass *)((char *)_data+index*_size);
	}

	void setElm(BaseClass *obj, uint index)
	{
		nlassert( index < _count );
//		BaseClass *ptr = (BaseClass *) ((char *)_data+index*_size);
//		memcpy( ptr, obj, _size );
		( *(BaseClass *)((char *)_data+index*_size) ) = *obj; 
	}

private:
	BaseClass*  _data;
	uint _count;
	uint _size;
	
};
*/

#endif

