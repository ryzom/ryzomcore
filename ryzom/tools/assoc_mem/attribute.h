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

#ifndef NL_ATTRIBUTE_H_
#define NL_ATTRIBUTE_H_

#include <string>

class IAttribute
{
	private:
		std::string _Name;
	public:
		IAttribute(std::string);
		std::string &getName();
		void setName(std::string);
};

template<class T> class CAttribute : public IAttribute
{
	private:
		T	_Value;
	public:
		CAttribute();
		CAttribute(std::string);
		CAttribute(std::string,T);
		T &getValue();
		void setValue(T &);
};

template<class T> CAttribute<T>::CAttribute() : IAttribute("<unnanamed>")
{
}

template<class T> CAttribute<T>::CAttribute(std::string name) : IAttribute(name)
{
}

template<class T> CAttribute<T>::CAttribute(std::string name, T value) : IAttribute(name)
{
	_Value = value;
}

template<class T> T &CAttribute<T>::getValue()
{
	return _Value;
}
template<class T> void CAttribute<T>::setValue(T &value)
{
	_Value = value;
}

#endif