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

#ifndef UT_LIGO_PRIMITIVE
#define UT_LIGO_PRIMITIVE

#include <nel/ligo/ligo_config.h>
#include <nel/ligo/primitive_utils.h>

class CUTLigoPrimitive : public Test::Suite
{
public:
	CUTLigoPrimitive()
	{
		TEST_ADD(CUTLigoPrimitive::testAliasGenerator)
	}

private:

	string	_RestorePath;
	string	_WorkingPath;
	string	_RefPrimFileName;
	void setup()
	{
		_RestorePath = NLMISC::CPath::getCurrentPath();
		NLMISC::CPath::setCurrentPath(_WorkingPath.c_str());

		_RefPrimFileName = "__test_prim.primitive";

		// register ligo class factory
		NLLIGO::Register();

		// create a primitive config file
		nlinfo("Building a default ligo class file");

		const char	*CLASS_FILE_NAME = "__ligo_class.xml";

		string classfile;
		classfile = string()
		+	"<?xml version=\"1.0\"?>\n"
		+	"<NEL_LIGO_PRIMITIVE_CLASS>\n"
		+	"	<ALIAS_DYNAMIC_BITS BIT_COUNT=\"20\"/>\n"
		+	"	<ALIAS_STATIC_FILE_ID FILE_NAME=\"file_index.cfg\"/>\n"
		+	"\n"
		+	"	<PRIMITIVE CLASS_NAME=\"root\" TYPE=\"node\" AUTO_INIT=\"true\" DELETABLE=\"true\">\n"
		+	"		<PARAMETER NAME=\"name\" TYPE=\"string\" VISIBLE=\"true\"/>\n"
		+	"		<PARAMETER NAME=\"path\" TYPE=\"string\" VISIBLE=\"true\"/>\n"
		+	"		<DYNAMIC_CHILD CLASS_NAME=\"test\"/>\n"
		+	"	</PRIMITIVE>\n"
		+	"\n"
		+	"	<!-- the alias class, used by all other class that need persistent aliases-->\n"
		+	"	<PRIMITIVE CLASS_NAME=\"alias\" TYPE=\"alias\" AUTO_INIT=\"true\" DELETABLE=\"false\">\n"
		+	"	</PRIMITIVE>\n"
		+	"\n"
		+	"	<PRIMITIVE CLASS_NAME=\"test\" TYPE=\"node\" AUTO_INIT=\"false\" DELETABLE=\"true\" NUMBERIZE=\"false\">\n"
		+	"		<PARAMETER NAME=\"name\" TYPE=\"string\" VISIBLE=\"true\"/>\n"
		+	"		<STATIC_CHILD CLASS_NAME=\"alias\" NAME=\"alias\"/>\n"
		+	"		<DYNAMIC_CHILD CLASS_NAME=\"test\"/>\n"
		+	"	</PRIMITIVE>\n"
		+	"</NEL_LIGO_PRIMITIVE_CLASS>";

		FILE *fp = NLMISC::nlfopen(CLASS_FILE_NAME, "wt");
		nlassert(fp != NULL);
		size_t s = fwrite(classfile.data(), 1, classfile.size(), fp);
		nlassert(s == classfile.size());
		fclose(fp);

		// init ligo
		NLLIGO::CPrimitiveContext::instance().CurrentLigoConfig = &_LigoConfig;
		_LigoConfig.readPrimitiveClass(CLASS_FILE_NAME, false);

		// create a reference primitive
		if (NLMISC::CFile::isExists(_RefPrimFileName))
		{
			NLMISC::CFile::deleteFile(_RefPrimFileName);
		}
		NLLIGO::CPrimitives primDoc;
		nlassert(primDoc.RootNode != NULL);

		NLLIGO::CPrimitiveContext::instance().CurrentPrimitive = &primDoc;

		NLLIGO::IPrimitive *p = dynamic_cast<NLLIGO::IPrimitive *> (NLMISC::CClassRegistry::create ("CPrimNode"));
		p->addPropertyByName("class", new NLLIGO::CPropertyString("test"));
		p->addPropertyByName("name", new NLLIGO::CPropertyString("test_root"));
		primDoc.RootNode->insertChild(p);

		NLLIGO::CPrimAlias *pa = dynamic_cast<NLLIGO::CPrimAlias *> (NLMISC::CClassRegistry::create ("CPrimAlias"));
		pa->addPropertyByName("class", new NLLIGO::CPropertyString("alias"));
		pa->addPropertyByName("name", new NLLIGO::CPropertyString("alias"));
		p->insertChild(pa);
		
		NLLIGO::CPrimitiveContext::instance().CurrentPrimitive = NULL;

		// save the file
		saveXmlPrimitiveFile(primDoc, _RefPrimFileName);
	}
	
	void tear_down()
	{
		NLMISC::CPath::setCurrentPath(_RestorePath.c_str());
	}

	void testAliasGenerator()
	{
		//Known bug : is we load/save a primitive and replacing a primitive node with itself (conserving the alias), the 
		// 'last generated alias' counter is incremented.
		uint32 lastGeneratedAlias;

		// First, load then save the doc
		{
			NLLIGO::CPrimitives primDoc;

			NLLIGO::CPrimitiveContext::instance().CurrentPrimitive = &primDoc;
			loadXmlPrimitiveFile(primDoc, _RefPrimFileName, _LigoConfig);
			NLLIGO::CPrimitiveContext::instance().CurrentPrimitive = NULL;

			lastGeneratedAlias = primDoc.getLastGeneratedAlias();

			// get a copy of the primitive
			NLLIGO::IPrimitive *prim = NULL;
			NLLIGO::IPrimitive *primCopy = NULL;
			TEST_ASSERT(primDoc.RootNode->getChild(prim, 0));
			if (prim)
			{
				primCopy = prim->copy();
				TEST_ASSERT(primCopy != NULL);
				if (primCopy)
				{
					// remove the primitive
					primDoc.RootNode->removeChild(prim);

					// insert the copy
					NLLIGO::CPrimitiveContext::instance().CurrentPrimitive = &primDoc;
					primDoc.RootNode->insertChild(primCopy);
					NLLIGO::CPrimitiveContext::instance().CurrentPrimitive = NULL;
				}
			}

			// save the file
			saveXmlPrimitiveFile(primDoc, _RefPrimFileName);
		}

		// second, reload the file and check the last generated alias
		{
			NLLIGO::CPrimitives primDoc;

			NLLIGO::CPrimitiveContext::instance().CurrentPrimitive = &primDoc;
			loadXmlPrimitiveFile(primDoc, _RefPrimFileName, _LigoConfig);
			NLLIGO::CPrimitiveContext::instance().CurrentPrimitive = NULL;

			TEST_ASSERT(lastGeneratedAlias == primDoc.getLastGeneratedAlias());
		}
	}

	NLLIGO::CLigoConfig		_LigoConfig;
};

#endif
