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



#ifndef XML_AUTO_PTR_H
#define XML_AUTO_PTR_H

#include <string>

/** Simple auto pointer for xml pointers
  */
class CXMLAutoPtr
{
public:
	CXMLAutoPtr(const char *value = NULL) : _Value(value) {}
	CXMLAutoPtr(const unsigned char *value) : _Value((const char *) value) {}
	~CXMLAutoPtr() { destroy(); }
	operator const char *() const { return _Value; }
	operator bool() const { return _Value != NULL; }
	operator std::string() const { return std::string(_Value); }
	bool operator ! () const { return _Value == NULL; }
	operator const unsigned char *() const { return (const unsigned char *)  _Value; }
	char operator * ()  const { nlassert(_Value); return *_Value; }
	/// NB : This remove previous owned pointer with xmlFree
	CXMLAutoPtr &operator = (const char *other)
	{
		if (other == _Value) return *this;
		destroy();
		_Value = other;
		return *this;
	}

	CXMLAutoPtr &operator = (const unsigned char *other)
	{
		*this = (const char *) other;
		return *this;
	}
	char *getDatas() const { return const_cast<char *>(_Value); }
//////////////////////////////////////////////////
private:
	const char *_Value;
private:
	void destroy()
	{
		if (_Value)
		{
			xmlFree(const_cast<char *>(_Value));
			_Value = NULL;
		}
	}

	// We'd rather avoid problems
	CXMLAutoPtr(const CXMLAutoPtr &/* other */)
	{
		nlassert(0);
	}
	CXMLAutoPtr&operator = (const CXMLAutoPtr &/* other */)
	{
		nlassert(0);
		return *this;
	}
};


#endif


