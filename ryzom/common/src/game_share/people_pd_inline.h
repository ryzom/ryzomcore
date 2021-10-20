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

namespace EGSPD
{

/* -----------------------------------------
* Inline implementation of CPeople
* ----------------------------------------- */
inline const std::string&		CPeople::toString(TPeople v)
{
	if (v < 0 || v >= ___TPeople_useSize)
	{
		nlwarning("TPeople::toString(): value '%u' is not matched, \"Unknown\" string returned", v);
		return _UnknownString;
	}
	if (!_Initialised)
	{
		init();
	}
	return _StrTable[v];
}
inline CPeople::TPeople			CPeople::fromString(const std::string& v)
{
	if (!_Initialised)
	{
		init();
	}
	if(v==_UnknownString)
	{
		return Unknown;
	}
	const std::map<std::string, TPeople>::const_iterator	it = _ValueMap.find(NLMISC::toLower(v));
	if (it == _ValueMap.end())
	{
		nlwarning("TPeople::toString(): string '%s' is not matched, 'Unknown' enum value returned", v.c_str());
		return Unknown;
	}
	return (*it).second;
}
// End of inline implementation of CPeople

/* -----------------------------------------
* Inline implementation of CClassificationType
* ----------------------------------------- */
inline const std::string&		CClassificationType::toString(TClassificationType v)
{
	if (v < 0 || v >= ___TClassificationType_useSize)
	{
		nlwarning("TClassificationType::toString(): value '%u' is not matched, \"Unknown\" string returned", v);
		return _UnknownString;
	}
	if (!_Initialised)
	{
		init();
	}
	return _StrTable[v];
}
inline CClassificationType::TClassificationType	CClassificationType::fromString(const std::string& v)
{
	if (!_Initialised)
	{
		init();
	}
	if(v==_UnknownString)
	{
		return Unknown;
	}
	const std::map<std::string, TClassificationType>::const_iterator	it = _ValueMap.find(NLMISC::toLower(v));
	if (it == _ValueMap.end())
	{
		nlwarning("TClassificationType::toString(): string '%s' is not matched, 'Unknown' enum value returned", v.c_str());
		return Unknown;
	}
	return (*it).second;
}
// End of inline implementation of CClassificationType


} // End of EGSPD
