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

#ifndef NL_VALUE_H_
#define NL_VALUE_H_

#include <string>

class IValue {
	public:
		virtual ~IValue() { }

		virtual void getValue(IValue &) = 0;
		virtual void setValue(IValue &) = 0;

		virtual bool operator==(IValue *) const = 0 ;
		virtual bool operator<(IValue &) = 0;
		virtual bool operator>(IValue &) = 0;
};

template<class T> class CValue : public IValue {
	private:
		T _Value;
	public:
		CValue();
		CValue(T);
		CValue(const CValue<T> &);
		virtual void getValue(IValue &);
		virtual void setValue(IValue &);

//		virtual CValue<T> &operator=(T);

		virtual bool operator==(IValue *) const;
		bool operator<(IValue &);
		bool operator>(IValue &);

		virtual T getValue();
		virtual void setValue(T);
};

template<class T> CValue<T>::CValue()
{
}

template<class T> CValue<T>::CValue(T value)
{
	_Value = value;
}

template<class T> CValue<T>::CValue(const CValue<T> &value)
{
	_Value = value.getValue();
}

template<class T> void CValue<T>::getValue(IValue &value)
{
	( (CValue<T> &)value)._Value = _Value;
}

template<class T> void CValue<T>::setValue(IValue &value)
{
	_Value = ( ( (CValue<T> &)value ).getValue() );
}

template<class T> bool CValue<T>::operator==(IValue *value) const
{
	return ( _Value == ( ( (CValue<T> *)value )->getValue() ) );
}

template<class T> bool CValue<T>::operator<(IValue &value)
{
	if ( _Value < ( ( (CValue<T> &)value ).getValue() ) )
		return true;
	else
		return false;
}

template<class T> bool CValue<T>::operator>(IValue &value)
{
	if ( _Value > ( ( (CValue<T> &)value ).getValue() ) )
		return true;
	else
		return false;
}

template<class T> void CValue<T>::setValue(T value)
{
	_Value = value;
}

template<class T> T CValue<T>::getValue()
{
	return _Value;
}

#endif
