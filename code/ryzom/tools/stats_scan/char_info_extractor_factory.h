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

#ifndef CHAR_INFO_EXTRACTOR_FACTORY_H
#define CHAR_INFO_EXTRACTOR_FACTORY_H


//-------------------------------------------------------------------------------------------------
// includes
//-------------------------------------------------------------------------------------------------

#include "nel/misc/smart_ptr.h"
#include "nel/misc/sstring.h"


//-------------------------------------------------------------------------------------------------
// forward class declarations
//-------------------------------------------------------------------------------------------------

class CCharacterScanJob;
class CStatsScanCharacter;


//-------------------------------------------------------------------------------------------------
// class ICharInfoExtractor
//-------------------------------------------------------------------------------------------------

class ICharInfoExtractor: public NLMISC::CRefCount
{
public:
	virtual ~ICharInfoExtractor() {}
	virtual std::string toString() const=0;
	virtual void execute(CCharacterScanJob* job,const CStatsScanCharacter* c)=0;
};


//-------------------------------------------------------------------------------------------------
// class ICharInfoExtractorBuilder
//-------------------------------------------------------------------------------------------------

class ICharInfoExtractorBuilder: public NLMISC::CRefCount
{
public:
	virtual ~ICharInfoExtractorBuilder() {}
	virtual const char* getName()=0;
	virtual const char* getDescription()=0;
	virtual const char* getFields()=0;
	virtual ICharInfoExtractor* build(const std::string& rawArgs)=0;
};


//-------------------------------------------------------------------------------------------------
// class CCharInfoExtractorFactory
//-------------------------------------------------------------------------------------------------

class CCharInfoExtractorFactory
{
public:
	static CCharInfoExtractorFactory* getInstance();

public:
	// register an info extractor instance
	void registerInfoExtractor(NLMISC::CSmartPtr<ICharInfoExtractorBuilder> infoExtractor);

	// display the set of names and descriptions of info extractor instances
	void displayInfoExtractorList(NLMISC::CLog* log=NLMISC::InfoLog);

	// basic accessors for getting hold of the registered info extractors
	uint32 getInfoExtractorBuilderCount();
	ICharInfoExtractorBuilder* getInfoExtractorBuilder(uint32 idx);

	// the all important build method
	ICharInfoExtractor* build(const NLMISC::CSString& cmdLine);

private:
	// this is a singleton so ctor is private
	CCharInfoExtractorFactory() {}

	typedef std::vector<NLMISC::CSmartPtr<ICharInfoExtractorBuilder> > TInfoExtractors;
	TInfoExtractors _InfoExtractors;
};


//-------------------------------------------------------------------------------------------------
// class CInfoExtractorRegisterer
//-------------------------------------------------------------------------------------------------

template <class C>
class CInfoExtractorRegisterer
{
public:
	CInfoExtractorRegisterer()
	{
		CCharInfoExtractorFactory::getInstance()->registerInfoExtractor(new C);
	}
};


//-------------------------------------------------------------------------------------------------
// MACRO INFO_EXTRACTOR()
//-------------------------------------------------------------------------------------------------

#define INFO_EXTRACTOR(name,description,fields)\
class CInfoExtractor_##name: public ICharInfoExtractor\
{\
public:\
	CInfoExtractor_##name(const std::string& rawArgs) {_RawArgs=rawArgs;}\
	virtual std::string toString() const {return std::string(#name)+" "+_RawArgs;}\
	virtual void execute(CCharacterScanJob* job,const CStatsScanCharacter* c);\
private:\
	NLMISC::CSString _RawArgs;\
};\
class CInfoExtractorBuilder_##name: public ICharInfoExtractorBuilder\
{\
public:\
	virtual const char* getName()			{return #name;}\
	virtual const char* getDescription()	{return description;}\
	virtual const char* getFields()			{return fields;}\
	virtual ICharInfoExtractor* build(const std::string& rawArgs)	{return new CInfoExtractor_##name(rawArgs);}\
};\
CInfoExtractorRegisterer<CInfoExtractorBuilder_##name> __Registerer_CInfoExtractor_##name;\
void CInfoExtractor_##name::execute(CCharacterScanJob* job,const CStatsScanCharacter* c)


//-------------------------------------------------------------------------------------------------
#endif