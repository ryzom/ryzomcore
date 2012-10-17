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

#ifndef STATS_CHAR_FILTER_FACTORY_H
#define STATS_CHAR_FILTER_FACTORY_H

//-------------------------------------------------------------------------------------------------
// includes
//-------------------------------------------------------------------------------------------------

#include "nel/misc/smart_ptr.h"
#include "nel/misc/sstring.h"


//-------------------------------------------------------------------------------------------------
// forward class declarations
//-------------------------------------------------------------------------------------------------

class CStatsScanCharacter;


//-------------------------------------------------------------------------------------------------
// class ICharFilter
//-------------------------------------------------------------------------------------------------

class ICharFilter: public NLMISC::CRefCount
{
public:
	virtual ~ICharFilter() {}
	virtual std::string toString() const=0;
	virtual bool evaluate(const CStatsScanCharacter* c)=0;
};


//-------------------------------------------------------------------------------------------------
// class ICharFilterBuilder
//-------------------------------------------------------------------------------------------------

class ICharFilterBuilder: public NLMISC::CRefCount
{
public:
	virtual ~ICharFilterBuilder() {}
	virtual const char* getName()=0;
	virtual const char* getDescription()=0;
	virtual ICharFilter* build(const std::string& rawArgs)=0;
};


//-------------------------------------------------------------------------------------------------
// class CCharFilterFactory
//-------------------------------------------------------------------------------------------------

class CCharFilterFactory
{
public:
	static CCharFilterFactory* getInstance();

public:
	// register an info extractor instance
	void registerFilter(NLMISC::CSmartPtr<ICharFilterBuilder> filter);

	// display the set of names and descriptions of info extractor instances
	void displayFilterList(NLMISC::CLog* log=NLMISC::InfoLog);

	// basic accessors for getting hold of the registered info extractors
	uint32 getFilterBuilderCount();
	ICharFilterBuilder* getFilterBuilder(uint32 idx);

	// the all important build method
	ICharFilter* build(const NLMISC::CSString& cmdLine);

private:
	// this is a singleton so ctor is private
	CCharFilterFactory() {}

	typedef std::vector<NLMISC::CSmartPtr<ICharFilterBuilder> > TFilters;
	TFilters _Filters;
};


//-------------------------------------------------------------------------------------------------
// class CFilterRegisterer
//-------------------------------------------------------------------------------------------------

template <class C>
class CFilterRegisterer
{
public:
	CFilterRegisterer()
	{
		CCharFilterFactory::getInstance()->registerFilter(new C);
	}
};


//-------------------------------------------------------------------------------------------------
// MACRO FILTER()
//-------------------------------------------------------------------------------------------------

#define FILTER(name,description)\
class CFilter_##name: public ICharFilter\
{\
public:\
	CFilter_##name(const std::string& rawArgs) {_RawArgs=rawArgs;}\
	virtual std::string toString() const {return std::string(#name)+" "+_RawArgs;}\
	virtual bool evaluate(const CStatsScanCharacter* c);\
private:\
	NLMISC::CSString _RawArgs;\
};\
class CFilterBuilder_##name: public ICharFilterBuilder\
{\
public:\
	virtual const char* getName()			{return #name;}\
	virtual const char* getDescription()	{return description;}\
	virtual ICharFilter* build(const std::string& rawArgs)	{return new CFilter_##name(rawArgs);}\
};\
CFilterRegisterer<CFilterBuilder_##name> __Registerer_CFilter_##name;\
bool CFilter_##name::evaluate(const CStatsScanCharacter* c)


//-------------------------------------------------------------------------------------------------
#endif
