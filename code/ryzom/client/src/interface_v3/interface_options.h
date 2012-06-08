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



#ifndef RZ_INTERFACE_LAYER_H
#define RZ_INTERFACE_LAYER_H

#include "nel/misc/smart_ptr.h"


namespace NL3D
{
class UAnimationSet;
}


// ***************************************************************************
class CInterfaceOptionValue
{
public:
	CInterfaceOptionValue()
	{
		_Color= NLMISC::CRGBA::White;
		_Int= 0;
		_Float= 0;
		_Boolean= false;
	}

	const std::string		&getValStr	() const {return _Str;}
	sint32					getValSInt32() const {return _Int;}
	float					getValFloat	() const {return _Float;}
	NLMISC::CRGBA			getValColor	() const {return _Color;}
	bool					getValBool	() const {return _Boolean;}

	void					init(const std::string &str);

	// returned when InterfaceOptions param not found
	static const CInterfaceOptionValue	NullValue;

private:

	std::string		_Str;
	NLMISC::CRGBA	_Color;
	sint32			_Int;
	float			_Float;
	bool			_Boolean;
};


// ***************************************************************************
class CInterfaceOptions : public NLMISC::CRefCount
{

public:

	CInterfaceOptions();
	virtual ~CInterfaceOptions();

	virtual bool parse (xmlNodePtr cur);

	// return NullValue if param not found
	const CInterfaceOptionValue		&getValue(const std::string &sParamName) const;

	// shortcuts to getValue(paramName).getValXXX()
	const std::string		&getValStr		(const std::string &sParamName) const;
	sint32					getValSInt32	(const std::string &sParamName) const;
	float					getValFloat		(const std::string &sParamName) const;
	NLMISC::CRGBA			getValColor		(const std::string &sParamName) const;
	bool					getValBool		(const std::string &sParamName) const;

	// copy basic map only from other CInterfaceOptions (non virtual method)
	void	copyBasicMap(const CInterfaceOptions &other);

protected:

	std::map<std::string, CInterfaceOptionValue> _ParamValue;

};


#endif // NL_INTERFACE_LAYER_H

/* End of interface_layer.h */


