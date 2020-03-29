/*
	Persistent data system test

	project: RYZOM / TEST

*/

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "nel/misc/types_nl.h"
#include "nel/misc/common.h"
#include "nel/misc/debug.h"
#include "nel/misc/command.h"
#include "nel/net/message.h"
#include "game_share/singleton_registry.h"
#include "game_share/persistent_data.h"
#include "game_share/persistent_data_tree.h"
#include "game_share/backup_service_interface.h"

//#include "pd_lib/pd_string_manager.h"


//-----------------------------------------------------------------------------
// setup definitions used by PDR system
//-----------------------------------------------------------------------------

#define PERSISTENT_MACROS_AUTO_UNDEF


//-----------------------------------------------------------------------------
// class CMyPersistentClass_Struct
//-----------------------------------------------------------------------------

class CMyPersistentClass_Struct
{
public:
	DECLARE_PERSISTENCE_METHODS

public:
	CMyPersistentClass_Struct()
	{
	}

	CMyPersistentClass_Struct(std::string s0,std::string s1)
	{
		_S0=s0;
		_S1=s1;
	}

	void clear()
	{
		_S0.clear();
		_S1.clear();
	}

private:
	std::string _S0,_S1;
};


//-----------------------------------------------------------------------------
// Persistent data for CMyPersistentClass_Struct
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CMyPersistentClass_Struct
#define PERSISTENT_DATA\
	PROP(std::string,_S0)\
	PROP(std::string,_S1)

#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"


//-----------------------------------------------------------------------------
// setup token family definition used by PDR system
//-----------------------------------------------------------------------------

// note - this is defined here and not above so as to test the case where a sub-class
// such as CMyPersistentClass_Struct is built outside the token family

#define PERSISTENT_TOKEN_FAMILY TestTokenFamily


//-----------------------------------------------------------------------------
// class CMyPersistentClass
//-----------------------------------------------------------------------------

class CMyPersistentClass
{
public:
	DECLARE_PERSISTENCE_METHODS

public:
	CMyPersistentClass()
	{
		static uint32 counter=0;
		counter+=100;

		b		= (counter%200)!=0;
		ui8		= ((uint8)counter)+8;
		ui16	= ((uint16)counter)+16;
		ui32	= counter+32;
		ui64	= counter+64;
		si8		= ((sint8)counter)+9;
		si16	= ((sint16)counter)+17;
		si32	= counter+33;
		si64	= counter+65;
		f		= (float)counter+123.456f;
		d		= counter+987.654;
		s		= NLMISC::toString("Testing...%d",counter);
		ucs		= (ucstring)NLMISC::toString("Tésting UCString ...%d",counter);
		sid		= NLMISC::CSheetId();
		eid		= NLMISC::CEntityId();
		mpcs	= CMyPersistentClass_Struct("hello",NLMISC::toString("World...%d",counter));
		mpcpv.push_back(counter+0);
		mpcpv.push_back(counter+1);
		mpcpv.push_back(counter+2);
		mpcsv.push_back(CMyPersistentClass_Struct("one",NLMISC::toString("two...%d",counter)));
		mpcsv.push_back(CMyPersistentClass_Struct("three",NLMISC::toString("four...%d",counter)));
		mpcpm["mapOne"]=counter-1;
		mpcpm["mapTwo"]=counter-2;
		mpcsm["mappyOne"]=CMyPersistentClass_Struct(NLMISC::toString("one...%d",counter),"one");
		mpcsm["mappyTwo"]=CMyPersistentClass_Struct(NLMISC::toString("two...%d",counter),"two");
	}

private:
	bool b;
	uint8  ui8;
	uint16 ui16;
	uint32 ui32;
	uint64 ui64;
	sint8  si8;
	sint16 si16;
	sint32 si32;
	sint64 si64;
	float f;
	double d;
	std::string s;
	ucstring ucs;
	NLMISC::CSheetId sid;
	NLMISC::CEntityId eid;

	CMyPersistentClass_Struct						mpcs;
	std::vector<uint32>								mpcpv;
	std::vector<CMyPersistentClass_Struct>			mpcsv;
	std::map<std::string,uint32>					mpcpm;
	std::map<std::string,CMyPersistentClass_Struct>	mpcsm;
};


//-----------------------------------------------------------------------------
// Persistent data for CMyPersistentClass
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CMyPersistentClass
#define PERSISTENT_DATA\
	PROP(bool,b)\
	PROP(uint8,ui8)\
	PROP(uint16,ui16)\
	PROP(uint32,ui32)\
	PROP(uint64,ui64)\
	PROP(sint8,si8)\
	PROP(sint16,si16)\
	PROP(sint32,si32)\
	PROP(sint64,si64)\
	PROP(float,f)\
	PROP(double,d)\
	PROP(std::string,s)\
	PROP2(ucs,std::string,ucs.toUtf8(),ucs.fromUtf8(val))\
	PROP(NLMISC::CSheetId,sid)\
	PROP(NLMISC::CEntityId,eid)\
	STRUCT(mpcs)\
	PROP_VECT(uint32,mpcpv)\
	STRUCT_VECT(mpcsv)\
	PROP_MAP(std::string,uint32,mpcpm)\
	STRUCT_MAP(std::string,CMyPersistentClass_Struct,mpcsm)

#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"


//-----------------------------------------------------------------------------
// class CMyPersistentClass2
//-----------------------------------------------------------------------------

class CMyPersistentClass2
{
public:
	DECLARE_PERSISTENCE_METHODS

public:
	CMyPersistentClass2()
	{
		static uint32 counter=0;
		counter+=100;

		s0	= NLMISC::toString("MPC2 0...%d",counter);
		s1	= NLMISC::toString("MPC2 1...%d",counter);
	}

private:
	std::string s0, s1;
};


//-----------------------------------------------------------------------------
// Persistent data for CMyPersistentClass2
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CMyPersistentClass2
#define PERSISTENT_DATA\
	PROP(std::string,s0)\
	PROP(std::string,s1)\

#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"


//-----------------------------------------------------------------------------
// class CMyPersistentClass3
//-----------------------------------------------------------------------------

class CMyPersistentClass3
{
public:
	DECLARE_PERSISTENCE_METHODS

public:
	CMyPersistentClass3()
	{
		static uint32 counter=0;
		counter+=100;

		s0	= NLMISC::toString("MPC3 0...%d",counter);
		s1	= NLMISC::toString("MPC3 1...%d",counter);
		i0	= counter + 1;
		i1	= counter + 2;
		i2	= counter + 3;
	}

private:
	std::string s0, s1;
	uint32 i0, i1, i2;
};


//-----------------------------------------------------------------------------
// Persistent data for CMyPersistentClass3
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CMyPersistentClass3
#define PERSISTENT_DATA\
	PROP(uint32,i0)\
	PROP(std::string,s0)\
	PROP(uint32,i1)\
	PROP(std::string,s1)\
	PROP(uint32,i2)\

#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"


//-----------------------------------------------------------------------------
// class CMyPersistentClass4
//-----------------------------------------------------------------------------

class CMyPersistentClass4
{
public:
	DECLARE_PERSISTENCE_METHODS

public:
	CMyPersistentClass4()
	{
		static uint32 counter=0;
		counter+=100;

		s0	= NLMISC::toString("MPC4 0...%d",counter);
		s1	= NLMISC::toString("MPC4 1...%d",counter);
		mpcsm["mappyOne"]=CMyPersistentClass_Struct(NLMISC::toString("MPC4 one...%d",counter),"MPC4 one");
		mpcsm["mappyTwo"]=CMyPersistentClass_Struct(NLMISC::toString("MPC4 two...%d",counter),"MPC4 two");
	}

private:
	std::string s0, s1;
	std::map<std::string,CMyPersistentClass_Struct>	mpcsm;
};


//-----------------------------------------------------------------------------
// Persistent data for CMyPersistentClass4
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CMyPersistentClass4
#define PERSISTENT_DATA\
	PROP(std::string,s0)\
	STRUCT_MAP(std::string,CMyPersistentClass_Struct,mpcsm)\
	PROP(std::string,s1)\

#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"


//-----------------------------------------------------------------------------
// class CMyPersistentClass5
//-----------------------------------------------------------------------------

class CMyPersistentClass5
{
public:
	DECLARE_PERSISTENCE_METHODS

public:
	CMyPersistentClass5()
	{
		ui8=	(uint8(~0))^	((uint8(~0))>>1);
		ui16=	(uint16(~0))^	((uint16(~0))>>1);
		ui32=	(uint32(~0))^	((uint32(~0))>>1);
		ui64=	(uint64(~0))^	((uint64(~0))>>1);

		si8=	(sint8)ui8;
		si16=	(sint16)ui16;
		si32=	(sint32)ui32;
		si64=	(sint64)ui64;

		f0=(float)ui64;
		f1=(float)si64;
		d0=(double)ui64;
		d1=(double)si64;
	}

private:
	uint8 ui8;
	uint16 ui16;
	uint32 ui32;
	uint64 ui64;
	sint8 si8;
	sint16 si16;
	sint32 si32;
	sint64 si64;
	float f0,f1;
	double d0,d1;
};


//-----------------------------------------------------------------------------
// Persistent data for CMyPersistentClass5
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CMyPersistentClass5
#define PERSISTENT_DATA\
	PROP(uint8,ui8)\
	PROP(uint16,ui16)\
	PROP(uint32,ui32)\
	PROP(uint64,ui64)\
	PROP(sint8,si8)\
	PROP(sint16,si16)\
	PROP(sint32,si32)\
	PROP(sint64,si64)\
	PROP(float,f0)\
	PROP(float,f1)\
	PROP(double,d0)\
	PROP(double,d1)\

#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"


//-----------------------------------------------------------------------------
// class CMyPersistentClass6
//-----------------------------------------------------------------------------

class CMyPersistentClass6
{
public:
	DECLARE_PERSISTENCE_METHODS

public:
	CMyPersistentClass6()
	{
		ui8=	0.0;
		ui16=	0.0;
		ui32=	0.0;
		ui64=	0.0;

		si8=	0.0;
		si16=	0.0;
		si32=	0.0;
		si64=	0.0;

		f0=	0;
		f1=	0;
		d0=	0;
		d1=	0;
	}

private:
	double ui8;
	double ui16;
	double ui32;
	double ui64;
	double si8;
	double si16;
	double si32;
	double si64;
	uint64 f0,d0;
	sint64 f1,d1;
};


//-----------------------------------------------------------------------------
// Persistent data for CMyPersistentClass6
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CMyPersistentClass6
#define PERSISTENT_DATA\
	PROP(double,ui8)\
	PROP(double,ui16)\
	PROP(double,ui32)\
	PROP(double,ui64)\
	PROP(double,si8)\
	PROP(double,si16)\
	PROP(double,si32)\
	PROP(double,si64)\
	PROP(uint64,f0)\
	PROP(sint64,f1)\
	PROP(uint64,d0)\
	PROP(sint64,d1)\

#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"


//-----------------------------------------------------------------------------
// class CMyPersistentClass7
//-----------------------------------------------------------------------------

class CMyPersistentClass7
{
public:
	DECLARE_PERSISTENCE_METHODS

public:
	CMyPersistentClass7():
		struct2("struct2_0","struct2_1"),
		lstructa("lstructa_0","lstructa_1"),
		lstructb("lstructb_0","lstructb_1"),
		lstruct2a("lstruct2a_0","lstruct2a_1"),
		lstruct2b("lstruct2b_0","lstruct2b_1")
	{
		static uint32 version=0;

		prop2	= "prop2";
		lpropa	= "lpropa";
		lpropb	= "lpropb";
		lprop2a	= "lprop2a";
		lprop2b	= "lprop2b";

		propSet.insert("propSet_0");
		propSet.insert("propSet_1");

		lpropVect.push_back("lpropVect_0");
		lpropVect.push_back("lpropVect_1");

		lpropVect2.push_back("lpropVect2_0");
		lpropVect2.push_back("lpropVect2_1");

		structPtrVect.push_back(new CMyPersistentClass_Struct("structPtrVect_0","structPtrVect_0_0"));
		structPtrVect.push_back(new CMyPersistentClass_Struct("structPtrVect_1","structPtrVect_1_1"));

		propArray.push_back("a");
		propArray.push_back("b");
		if ((version&1)==0)
			propArray.push_back("c - optional");

		structArray.push_back(CMyPersistentClass_Struct("structPtrVect_0","structPtrVect_0_0"));
		structArray.push_back(CMyPersistentClass_Struct("structPtrVect_1","structPtrVect_1_1"));
		if ((version&1)==1)
			structArray.push_back(CMyPersistentClass_Struct("structPtrVect_2 - optional","structPtrVect_2_2 - optional"));

		version++;
	}

	void clear()
	{
		prop2.clear();
		lpropa.clear();
		lpropb.clear();
		lprop2a.clear();
		lprop2b.clear();

		struct2.clear();
		lstructa.clear();
		lstructb.clear();
		lstruct2a.clear();
		lstruct2b.clear();

		propSet.clear();
		lpropVect.clear();
		lpropVect2.clear();
		structPtrVect.clear();
	}

private:
	std::string prop2;
	std::string lpropa;
	std::string lpropb;
	std::string lprop2a;
	std::string lprop2b;

	CMyPersistentClass_Struct struct2;
	CMyPersistentClass_Struct lstructa;
	CMyPersistentClass_Struct lstructb;
	CMyPersistentClass_Struct lstruct2a;
	CMyPersistentClass_Struct lstruct2b;

	std::set<std::string> propSet;

	std::vector<std::string> lpropVect;
	std::vector<std::string> lpropVect2;

	std::vector<std::string> propArray;
	std::vector<CMyPersistentClass_Struct> structArray;

	std::vector<CMyPersistentClass_Struct*> structPtrVect;
};

//-----------------------------------------------------------------------------
// Persistent data for CMyPersistentClass7
//-----------------------------------------------------------------------------

#define PERSISTENT_CLASS CMyPersistentClass7

#define PERSISTENT_PRE_STORE nlinfo("PERSISTENT_PRE_STORE");
#define PERSISTENT_PRE_APPLY nlinfo("PERSISTENT_PRE_APPLY");
#define PERSISTENT_POST_STORE nlinfo("PERSISTENT_POST_STORE");
#define PERSISTENT_POST_APPLY nlinfo("PERSISTENT_POST_APPLY");

#define PERSISTENT_DATA\
	FLAG(flag,nlinfo("flag");clear())\
	PROP2(prop2,std::string,prop2,prop2=val)\
	LPROP(std::string,lpropa,if(true))\
	LPROP(std::string,lpropb,if(false))\
	LPROP2(lprop2a,std::string,if(true),lprop2a,lprop2a=val)\
	LPROP2(lprop2b,std::string,if(false),lprop2b,lprop2b=val)\
	PROP_SET(std::string,propSet)\
	LPROP_VECT(std::string,lpropVect,VECT_LOGIC(lpropVect))\
	LPROP_VECT2(lpropVect2,std::string,VECT_LOGIC(lpropVect2),lpropVect2[i],lpropVect2.push_back(val))\
	STRUCT2(struct2,struct2.store(pdr),struct2.apply(pdr))\
	LSTRUCT(lstructa,if(true))\
	LSTRUCT(lstructb,if(false))\
	LSTRUCT2(lstruct2a,if(true),lstruct2a.store(pdr),lstruct2a.apply(pdr))\
	LSTRUCT2(lstruct2b,if(false),lstruct2b.store(pdr),lstruct2b.apply(pdr))\
	STRUCT_PTR_VECT(CMyPersistentClass_Struct,structPtrVect)\
	PROP_ARRAY(std::string,propArray,propArray.size())\
	STRUCT_ARRAY(structArray,structArray.size())\
	LFLAG(lflaga,if(true),nlinfo("lflag"))\
	LFLAG(lflagb,if(false),nlinfo("lflagb"))\

#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"


//-----------------------------------------------------------------------------
// basic test routines
//-----------------------------------------------------------------------------

static void test1()
{
	CPersistentDataTree pdt;
	pdt.readFromFile("pdrLines.txt");
	pdt.writeToFile("pdrLines_out1.txt");

	CPersistentDataRecord pdr;
	pdt.writeToPdr(pdr);
	pdr.writeToFile("pdrLines_out.txt");
	pdr.writeToFile("pdrLines_out.xml");

	CPersistentDataTree pdt2;
	pdt2.readFromPdr(pdr);
	pdt2.writeToFile("pdrLines_out2.txt");

}

static void test2()
{
	NLMISC::CSheetId::init(false);

	CMyPersistentClass mpc0,mpc1;
	CPersistentDataRecord pdr("TestTokenFamily");

	// use a pdr record once
	mpc0.store(pdr);
	pdr.writeToTxtFile("test0a.xml");
	pdr.writeToBinFile("test0a.bin");

	// reuse the same pdr record a second time
	pdr.clear();
	mpc1.store(pdr);
	pdr.writeToTxtFile("test1a.xml");
	pdr.writeToBinFile("test1a.bin");
}

static void test3()
{
	CMyPersistentClass mpc0,mpc1;

	// perform a swap
	CPersistentDataRecord pdr0;
	CPersistentDataRecord pdr1;
	mpc0.store(pdr0);
	mpc1.store(pdr1);
	mpc1.apply(pdr0);
	mpc0.apply(pdr1);

	// use a clean pair of records to store results
	CPersistentDataRecord pdr2;
	CPersistentDataRecord pdr3;
	mpc0.store(pdr2);
	mpc1.store(pdr3);

	pdr2.writeToTxtFile("test0c.xml");
	pdr3.writeToTxtFile("test1c.xml");

	CPersistentDataRecord pdr4,pdr5;
	pdr4.readFromBinFile("test0a.bin");
	CMyPersistentClass mpc4;
	mpc4.apply(pdr4);
	mpc4.store(pdr5);
	pdr5.writeToTxtFile("test0d.xml");
	pdr5.writeToBinFile("test0d.bin");

	CPersistentDataRecord pdr6;
	pdr6.readFromTxtFile("test0a.xml");
	CMyPersistentClass mpc6;
	mpc6.apply(pdr6);
	pdr6.writeToTxtFile("test0e.xml");

	CPersistentDataRecord pdr7a,pdr7b;
	CMyPersistentClass3().store(pdr7a);
	CMyPersistentClass2 mpc7;
	mpc7.apply(pdr7a);
	pdr7a.writeToTxtFile("test7a.xml");
	mpc7.store(pdr7b);
	pdr7b.writeToTxtFile("test7b.xml");

	CPersistentDataRecord pdr8a,pdr8b;
	CMyPersistentClass4().store(pdr8a);
	CMyPersistentClass2 mpc8;
	mpc8.apply(pdr8a);
	pdr8a.writeToTxtFile("test8a.xml");
	mpc8.store(pdr8b);
	pdr8b.writeToTxtFile("test8b.xml");

	CPersistentDataRecord pdr9a,pdr9b;
	CMyPersistentClass5().store(pdr9a);
	pdr9a.writeToTxtFile("test9a.xml");
	CMyPersistentClass6 mpc9;
	mpc9.apply(pdr9a);
	mpc9.store(pdr9b);
	pdr9b.writeToTxtFile("test9b.xml");

	CPersistentDataRecord pdr10a,pdr10b;
	CMyPersistentClass7().store(pdr10a);
	pdr10a.writeToTxtFile("test10a.xml");
	CMyPersistentClass7 mpc10a;
	mpc10a.apply(pdr10a);
	mpc10a.store(pdr10b);
	pdr10b.writeToTxtFile("test10b.xml");
	CMyPersistentClass_Struct mpc10b;
	pdr10a.rewind();
	mpc10b.apply(pdr10a);

	CPersistentDataRecord pdr11a, pdr11b, pdr11c;
	CMyPersistentClass_Struct("abc","def").store(pdr11a);
	pdr11a.writeToFile("test11a.bin");
	pdr11a.writeToFile("test11b.xml");
	pdr11a.writeToFile("test11c.bin");
	pdr11a.writeToFile("test11d.xml");
	pdr11b.readFromFile("test11a.bin");
	pdr11c.readFromFile("test11b.xml");
	NLMISC::InfoLog->displayNL("\nTest11:");
	std:: string s;
	pdr11b.toString(s);
	NLMISC::InfoLog->displayNL(s.c_str());
	pdr11c.toString(s);
	NLMISC::InfoLog->displayNL(s.c_str());
}


static void test4()
{
	CPersistentDataRecord pdr("TestTokenFamily");

	// test 0 chars
	CMyPersistentClass_Struct("","").store(pdr);
	CMyPersistentClass_Struct("","").store(pdr);

	// test 1 char
	CMyPersistentClass_Struct("A","B").store(pdr);
	CMyPersistentClass_Struct("B","A").store(pdr);

	// test 2 chars
	CMyPersistentClass_Struct("AA","BA").store(pdr);
	CMyPersistentClass_Struct("BA","AA").store(pdr);

	// test 3 chars
	CMyPersistentClass_Struct("AAA","BAA").store(pdr);
	CMyPersistentClass_Struct("BAA","AAA").store(pdr);
	CMyPersistentClass_Struct("AAB","BAB").store(pdr);
	CMyPersistentClass_Struct("BAB","AAB").store(pdr);

	// test 4 chars
	CMyPersistentClass_Struct("AAAA","BAAA").store(pdr);
	CMyPersistentClass_Struct("BAAA","AAAA").store(pdr);

	// test 5 chars
	CMyPersistentClass_Struct("AAAAA","BAAAA").store(pdr);
	CMyPersistentClass_Struct("BAAAA","AAAAA").store(pdr);
	CMyPersistentClass_Struct("AAAAB","BAAAB").store(pdr);
	CMyPersistentClass_Struct("BAAAB","AAAAB").store(pdr);

	// test 8 chars
	CMyPersistentClass_Struct("AAAAAAAA","BAAAAAAA").store(pdr);
	CMyPersistentClass_Struct("BAAAAAAA","AAAAAAAA").store(pdr);
	CMyPersistentClass_Struct("AAAAAAAB","BAAAAAAB").store(pdr);
	CMyPersistentClass_Struct("BAAAAAAB","AAAAAAAB").store(pdr);

	// test 9 chars
	CMyPersistentClass_Struct("AAAAAAAAA","BAAAAAAAA").store(pdr);
	CMyPersistentClass_Struct("BAAAAAAAA","AAAAAAAAA").store(pdr);
	CMyPersistentClass_Struct("AAAAAAAAB","BAAAAAAAB").store(pdr);
	CMyPersistentClass_Struct("BAAAAAAAB","AAAAAAAAB").store(pdr);
	CMyPersistentClass_Struct("AAAABAAAB","BAAABAAAB").store(pdr);
	CMyPersistentClass_Struct("BAAABAAAB","AAAABAAAB").store(pdr);

	// test 12 chars
	CMyPersistentClass_Struct("AAAAxxxxAAAA","BAAAxxxxAAAA").store(pdr);
	CMyPersistentClass_Struct("BAAAxxxxAAAA","AAAAxxxxAAAA").store(pdr);
	CMyPersistentClass_Struct("AAAAxxxxAAAB","BAAAxxxxAAAB").store(pdr);
	CMyPersistentClass_Struct("BAAAxxxxAAAB","AAAAxxxxAAAB").store(pdr);
	CMyPersistentClass_Struct("AAAAYxxxAAAB","BAAAYxxxAAAB").store(pdr);
	CMyPersistentClass_Struct("BAAAYxxxAAAB","AAAAYxxxAAAB").store(pdr);
	CMyPersistentClass_Struct("AAAAxxxYAAAB","BAAAxxxYAAAB").store(pdr);
	CMyPersistentClass_Struct("BAAAxxxYAAAB","AAAAxxxYAAAB").store(pdr);

	// test 13 chars
	CMyPersistentClass_Struct("AAAAxxxxxAAAA","BAAAxxxxxAAAA").store(pdr);
	CMyPersistentClass_Struct("BAAAxxxxxAAAA","AAAAxxxxxAAAA").store(pdr);
	CMyPersistentClass_Struct("AAAAxxxxxAAAB","BAAAxxxxxAAAB").store(pdr);
	CMyPersistentClass_Struct("BAAAxxxxxAAAB","AAAAxxxxxAAAB").store(pdr);
	CMyPersistentClass_Struct("AAAAYxxxxAAAB","BAAAYxxxxAAAB").store(pdr);
	CMyPersistentClass_Struct("BAAAYxxxxAAAB","AAAAYxxxxAAAB").store(pdr);
	CMyPersistentClass_Struct("AAAAxxxxYAAAB","BAAAxxxxYAAAB").store(pdr);
	CMyPersistentClass_Struct("BAAAxxxxYAAAB","AAAAxxxxYAAAB").store(pdr);

	// test 16 chars
	CMyPersistentClass_Struct("AAAAxxwwwwxxAAAA","BAAAxxwwwwxxAAAA").store(pdr);
	CMyPersistentClass_Struct("BAAAxxwwwwxxAAAA","AAAAxxwwwwxxAAAA").store(pdr);
	CMyPersistentClass_Struct("AAAAxxwwwwxxAAAB","BAAAxxwwwwxxAAAB").store(pdr);
	CMyPersistentClass_Struct("BAAAxxwwwwxxAAAB","AAAAxxwwwwxxAAAB").store(pdr);
	CMyPersistentClass_Struct("AAAAYxwwwwxxAAAB","BAAAYxwwwwxxAAAB").store(pdr);
	CMyPersistentClass_Struct("BAAAYxwwwwxxAAAB","AAAAYxwwwwxxAAAB").store(pdr);
	CMyPersistentClass_Struct("AAAAxxwwwwxYAAAB","BAAAxxwwwwxYAAAB").store(pdr);
	CMyPersistentClass_Struct("BAAAxxwwwwxYAAAB","AAAAxxwwwwxYAAAB").store(pdr);

	// test 17 chars
	CMyPersistentClass_Struct("AAAAxxxxwxxxxAAAA","BAAAxxxxwxxxxAAAA").store(pdr);
	CMyPersistentClass_Struct("BAAAxxxxwxxxxAAAA","AAAAxxxxwxxxxAAAA").store(pdr);
	CMyPersistentClass_Struct("AAAAxxxxwxxxxAAAB","BAAAxxxxwxxxxAAAB").store(pdr);
	CMyPersistentClass_Struct("BAAAxxxxwxxxxAAAB","AAAAxxxxwxxxxAAAB").store(pdr);
	CMyPersistentClass_Struct("AAAAYxxxwxxxxAAAB","BAAAYxxxwxxxxAAAB").store(pdr);
	CMyPersistentClass_Struct("BAAAYxxxwxxxxAAAB","AAAAYxxxwxxxxAAAB").store(pdr);
	CMyPersistentClass_Struct("AAAAxxxxwxxxYAAAB","BAAAxxxxwxxxYAAAB").store(pdr);
	CMyPersistentClass_Struct("BAAAxxxxwxxxYAAAB","AAAAxxxxwxxxYAAAB").store(pdr);
	CMyPersistentClass_Struct("AAAAxxxxYxxxxAAAA","BAAAxxxxYxxxxAAAA").store(pdr);
	CMyPersistentClass_Struct("BAAAxxxxYxxxxAAAA","AAAAxxxxYxxxxAAAA").store(pdr);

	// test 20 chars
	CMyPersistentClass_Struct("AAAAxxxxwwwwxxxxAAAA","BAAAxxxxwwwwxxxxAAAA").store(pdr);
	CMyPersistentClass_Struct("BAAAxxxxwwwwxxxxAAAA","AAAAxxxxwwwwxxxxAAAA").store(pdr);
	CMyPersistentClass_Struct("AAAAxxxxwwwwxxxxAAAB","BAAAxxxxwwwwxxxxAAAB").store(pdr);
	CMyPersistentClass_Struct("BAAAxxxxwwwwxxxxAAAB","AAAAxxxxwwwwxxxxAAAB").store(pdr);
	CMyPersistentClass_Struct("AAAAYxxxwwwwxxxxAAAB","BAAAYxxxwwwwxxxxAAAB").store(pdr);
	CMyPersistentClass_Struct("BAAAYxxxwwwwxxxxAAAB","AAAAYxxxwwwwxxxxAAAB").store(pdr);
	CMyPersistentClass_Struct("AAAAxxxxwwwwxxxYAAAB","BAAAxxxxwwwwxxxYAAAB").store(pdr);
	CMyPersistentClass_Struct("BAAAxxxxwwwwxxxYAAAB","AAAAxxxxwwwwxxxYAAAB").store(pdr);
	CMyPersistentClass_Struct("AAAAxxxxYwwwxxxxAAAA","BAAAxxxxYwwwxxxxAAAA").store(pdr);
	CMyPersistentClass_Struct("BAAAxxxxYwwwxxxxAAAA","AAAAxxxxYwwwxxxxAAAA").store(pdr);

	pdr.writeToTxtFile("test0a.xml");
}

//-----------------------------------------------------------------------------
// class CIntTestObj
//-----------------------------------------------------------------------------

class CIntTestObj
{
public:
	DECLARE_PERSISTENCE_METHODS
	sint32 IntData;
};

#define PERSISTENT_CLASS CIntTestObj
#define PERSISTENT_DATA	PROP(sint32,IntData)

#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"


//-----------------------------------------------------------------------------
// class CStringTestObj
//-----------------------------------------------------------------------------

class CStringTestObj
{
public:
	DECLARE_PERSISTENCE_METHODS
	std::string StringData;
};

#define PERSISTENT_CLASS CStringTestObj
#define PERSISTENT_DATA	PROP(std::string,StringData)

#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"


//-----------------------------------------------------------------------------
// class CVectorTestObj
//-----------------------------------------------------------------------------

class CVectorTestObj
{
public:
	DECLARE_PERSISTENCE_METHODS
	std::vector<sint32> VectorData;
};

#define PERSISTENT_CLASS CVectorTestObj
#define PERSISTENT_DATA	PROP_VECT(sint32,VectorData)

#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"


//-----------------------------------------------------------------------------
// class CMapTestObj
//-----------------------------------------------------------------------------

class CMapTestObj
{
public:
	DECLARE_PERSISTENCE_METHODS
	std::map<sint32,sint32> MapData;
};

#define PERSISTENT_CLASS CMapTestObj
#define PERSISTENT_DATA	PROP_MAP(sint32,sint32,MapData)

#pragma message( PERSISTENT_GENERATION_MESSAGE )
#include "game_share/persistent_data_template.h"


//-----------------------------------------------------------------------------
// speed test routines
//-----------------------------------------------------------------------------

void miniSpeedTestA(CPersistentDataRecord& pdr,CIntTestObj& testObj);

void miniSpeedTest()
{
	CPersistentDataRecord pdr("TestTokenFamily");
	CIntTestObj testObj;
	miniSpeedTestA(pdr,testObj);
}

void miniSpeedTestA(CPersistentDataRecord& pdr,CIntTestObj& testObj)
{
	testObj.store(pdr);
}

static void speedTest1()
{
	// store the previous value of the Bench global and set Bench to true for the duration of our test
	bool oldBench= NLMISC::CHTimer::benching();
	if (!oldBench)
		NLMISC::CHTimer::startBench(false, true, false);

	// time the whole test
	{
		H_AUTO(PDRSpeedTest)

		// run the test 1000 times
		for (uint32 i=0;i<1000;++i)
		{
			if ((i%100)==0) nlinfo("PDR Speed test: Benching iteration: %3i",i);

			// time each iteartion
			H_AUTO(PDRSpeedIteration)

			// setup the pdr that we're going to store to
			static CPersistentDataRecord pdr("TestTokenFamily");
			pdr.clear();

			// int test
			{
				CIntTestObj testObj;

				H_AUTO(PDRSpeed_IntTest)
				for (uint32 j=0;j<1000;++j)
				{
					testObj.store(pdr);
				}
			}

			// string test1 (empty string)
			{
				CStringTestObj testObj;

				H_AUTO(PDRSpeed_StringTest1)
				for (uint32 j=0;j<1000;++j)
				{
					testObj.store(pdr);
				}
			}

			// string test2 (40 character long string - but always the same)
			{
				CStringTestObj testObj;
				testObj.StringData= "This is a string of a reasonable length.";

				H_AUTO(PDRSpeed_StringTest2)
				for (uint32 j=0;j<1000;++j)
				{
					testObj.store(pdr);
				}
			}

			// string test3 (strings are always different)
			{
				H_AUTO(PDRSpeed_StringTest3)

				static char *words[]= { "This", "is", "a", "string", "bla", "of", "a", "reasonable", "length", "bla" };

				CStringTestObj testObj[1000];
				{
					H_AUTO(PDRSpeed_StringTest3Init)
					for (uint32 j=0;j<1000;++j)
					{
						testObj[j].StringData= std::string()+ words[j%10]+" "+words[(j/10)%10]+" "+words[(j/100)%10]+" "+words[(j/1000)%10];
					}
				}

				{
					H_AUTO(PDRSpeed_StringTest3Body)
					for (uint32 j=0;j<1000;++j)
					{
						testObj[j].store(pdr);
					}
				}
			}

			// vector test1 (empty vector)
			{
				CVectorTestObj testObj;

				H_AUTO(PDRSpeed_VectorTest)
				for (uint32 j=0;j<1000;++j)
				{
					testObj.store(pdr);
				}
			}

			// vector test2 (vector with one element)
			{
				CVectorTestObj testObj;
				testObj.VectorData.push_back(0);

				H_AUTO(PDRSpeed_VectorTest2)
				for (uint32 j=0;j<1000;++j)
				{
					testObj.store(pdr);
				}
			}

			// vector test3 (vector of 100 entries)
			{
				CVectorTestObj testObj;

				H_AUTO(PDRSpeed_VectorTest3)
				{
					H_AUTO(PDRSpeed_VectorTest3Init)
					testObj.VectorData.resize(100);
					for (sint32 j=0;j<100;++j)
					{
						testObj.VectorData.push_back(j);
					}
				}
				{
					H_AUTO(PDRSpeed_VectorTest3Body)
					for (uint32 j=0;j<10;++j)
					{
						testObj.store(pdr);
					}
				}
			}

			// map test1 (empty map)
			{
				CMapTestObj testObj;

				H_AUTO(PDRSpeed_MapTest)
				for (uint32 j=0;j<1000;++j)
				{
					testObj.store(pdr);
				}
			}

			// map test2 (map with one entry)
			{
				CMapTestObj testObj;
				testObj.MapData[0]= 0;

				H_AUTO(PDRSpeed_MapTest)
				for (uint32 j=0;j<1000;++j)
				{
					testObj.store(pdr);
				}
			}

			// map test3 (map with 100 entries)
			{
				CMapTestObj testObj;

				H_AUTO(PDRSpeed_MapTest3)
				{
					H_AUTO(PDRSpeed_MapTest3Init)
					for (sint32 j=0;j<100;++j)
					{
						testObj.MapData[j]=j;
					}
				}
				{
					H_AUTO(PDRSpeed_MapTest3Body)
					for (uint32 j=0;j<10;++j)
					{
						testObj.store(pdr);
					}
				}
			}

			// store the pdr to a CMessage
			{
				static NLNET::CMessage* theMessage=NULL;
				if (theMessage!=NULL) delete theMessage;
				theMessage=new NLNET::CMessage;

				H_AUTO(PDRSpeed_WriteMessage)
				pdr.toStream(*theMessage);
			}

			#if 0
			// on the first run also store the result to different file types
			if (i==0)
			{
				// store the pdr to a BIN file
				{
					H_AUTO(PDRSpeed_WriteBin)
					pdr.writeToFile("pdr_speed_test_output.bin");
				}

				// store the pdr to a TXT file
				{
					H_AUTO(PDRSpeed_WriteTxt)
					pdr.writeToFile("pdr_speed_test_output.txt");
				}

				// store the pdr to an XML file
				{
					H_AUTO(PDRSpeed_WriteXml)
					pdr.writeToFile("pdr_speed_test_output.xml");
				}
			}
			#endif
		}
	}

	// restore previous value of the 'Bench' global
	if (!oldBench)
		NLMISC::CHTimer::endBench();
}

//-----------------------------------------------------------------------------
// class CPersistentDataTest
//-----------------------------------------------------------------------------

class CPersistentDataTest: public IServiceSingleton
{
public:
	void init() 
	{
		// test1(); // test pd tree & lines format
		// test2(); // test a pdr containing a bit of everything
		// test3();
		// test4(); // test different cases of addString

		//miniSpeedTest();
		speedTest1();
	}
};

static CPersistentDataTest PersistentDataTest;

