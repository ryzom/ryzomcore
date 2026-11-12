// NeL - MMORPG Framework <https://wiki.ryzom.dev/>
// Copyright (C) 2026  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <deque>
#include <string>
#include <vector>

#include <nel/misc/algo.h>
#include <nel/misc/common.h>
#include <nel/misc/debug.h>
#include <nel/misc/file.h>
#include <nel/misc/mem_displayer.h>
#include <nel/misc/path.h>
#include <nel/misc/smart_ptr.h>

#include <nel/georges/form.h>
#include <nel/georges/u_form.h>
#include <nel/georges/u_form_elm.h>
#include <nel/georges/u_form_loader.h>

using std::string;

using ::testing::HasSubstr;
using ::testing::StrEq;

// Parents of Georges forms. The forms are written in the current directory, where the other tests write.
class CUTGeorgesFormParents : public testing::Test
{
protected:
	NLGEORGES::UFormLoader *m_Loader = nullptr;
	std::vector<string> m_Files;

	void SetUp() override
	{
		NLMISC::createDebug();
		m_Loader = NLGEORGES::UFormLoader::createLoader();

		writeFile("__ut_string.typ", "<?xml version=\"1.0\"?>\n<TYPE Type=\"String\" UI=\"Edit\"/>\n");
		writeFile("__ut_parents.dfn",
		    "<?xml version=\"1.0\"?>\n"
		    "<DFN>\n"
		    "	<ELEMENT Name=\"A\" Type=\"Type\" Filename=\"__ut_string.typ\"/>\n"
		    "	<ELEMENT Name=\"B\" Type=\"Type\" Filename=\"__ut_string.typ\"/>\n"
		    "	<ELEMENT Name=\"C\" Type=\"Type\" Filename=\"__ut_string.typ\"/>\n"
		    "	<ELEMENT Name=\"Unset\" Type=\"Type\" Filename=\"__ut_string.typ\"/>\n"
		    "</DFN>\n");
	}

	void TearDown() override
	{
		NLGEORGES::UFormLoader::releaseLoader(m_Loader);
		m_Loader = nullptr;
		for (size_t i = 0; i < m_Files.size(); ++i)
			NLMISC::CFile::deleteFile(m_Files[i]);
	}

	// the file name of the form with this short name
	static string formName(const string &name)
	{
		return "__ut_" + name + ".__ut_parents";
	}

	void writeFile(const string &fileName, const string &content)
	{
		m_Files.push_back(fileName);
		FILE *fp = NLMISC::nlfopen(fileName, "wb");
		ASSERT_NE(fp, nullptr);
		ASSERT_EQ(fwrite(content.data(), 1, content.size(), fp), content.size());
		fclose(fp);
	}

	// writes a form that sets one atom to "from <name>", parents are short names separated by spaces
	void writeForm(const string &name, const string &parents, const string &atom)
	{
		std::vector<string> parentNames;
		NLMISC::splitString(parents, " ", parentNames);
		string content = "<?xml version=\"1.0\"?>\n<FORM Version=\"0.0\" State=\"modified\">\n";
		for (size_t i = 0; i < parentNames.size(); ++i)
			content += "	<PARENT Filename=\"" + formName(parentNames[i]) + "\"/>\n";
		content += "	<STRUCT>\n		<ATOM Name=\"" + atom + "\" Value=\"from " + name + "\"/>\n	</STRUCT>\n</FORM>\n";
		writeFile(formName(name), content);
	}

	NLGEORGES::UForm *loadForm(const string &name)
	{
		return m_Loader->loadForm(formName(name));
	}

	// the value of an atom, looked up in the form and its parents
	static string value(NLGEORGES::UForm *form, const string &atom)
	{
		string result;
		form->getRootNode().getValueByName(result, atom);
		return result;
	}

	// the logged warnings that are asset errors
	static std::vector<string> assetErrors(NLMISC::CLightMemDisplayer &warnings)
	{
		std::vector<string> result;
		const std::deque<string> &strings = warnings.lockStrings();
		for (size_t i = 0; i < strings.size(); ++i)
		{
			if (strings[i].find("ASSET ERROR") != string::npos)
				result.push_back(strings[i]);
		}
		warnings.unlockStrings();
		return result;
	}
};

// Without a loop: every parent is used, a form that is reached twice is fine
TEST_F(CUTGeorgesFormParents, diamond)
{
	NLMISC::CLightMemDisplayer warnings;
	NLMISC::CLog logger;
	logger.addDisplayer(&warnings);
	NLMISC::CNLWarningOverride override(&logger);

	writeForm("top", "", "A");
	writeForm("left", "top", "B");
	writeForm("right", "top", "C");
	writeForm("bottom", "left right", "Unused");

	NLMISC::CSmartPtr<NLGEORGES::UForm> bottom = loadForm("bottom");
	ASSERT_NE(bottom, nullptr);
	EXPECT_EQ(bottom->getNumParent(), 2u);
	EXPECT_EQ(value(bottom, "A"), "from top");
	EXPECT_EQ(value(bottom, "B"), "from left");
	EXPECT_EQ(value(bottom, "C"), "from right");
	EXPECT_EQ(value(bottom, "Unset"), "");
	EXPECT_TRUE(assetErrors(warnings).empty());
}

// a -> b -> c -> a. The parent that closes the loop is refused, the lookups end.
TEST_F(CUTGeorgesFormParents, loopOfThree)
{
	NLMISC::CLightMemDisplayer warnings;
	NLMISC::CLog logger;
	logger.addDisplayer(&warnings);
	NLMISC::CNLWarningOverride override(&logger);

	writeForm("a", "b", "A");
	writeForm("b", "c", "B");
	writeForm("c", "a", "C");

	NLMISC::CSmartPtr<NLGEORGES::UForm> a = loadForm("a");
	ASSERT_NE(a, nullptr);
	NLMISC::CSmartPtr<NLGEORGES::UForm> b = loadForm("b");
	ASSERT_NE(b, nullptr);
	NLMISC::CSmartPtr<NLGEORGES::UForm> c = loadForm("c");
	ASSERT_NE(c, nullptr);

	// c got a as parent while a was being read, b got c, and a was refused b
	EXPECT_EQ(a->getNumParent(), 0u);
	EXPECT_EQ(b->getNumParent(), 1u);
	EXPECT_EQ(c->getNumParent(), 1u);

	std::vector<string> errors = assetErrors(warnings);
	ASSERT_EQ(errors.size(), (size_t)1);
	EXPECT_THAT(errors[0], StrEq("ASSET ERROR: the form '" + formName("a") + "' names the parent '" + formName("b") + "', whose own parents lead back to '" + formName("a") + "', this parent is not used and is left out when the form is saved\n"));

	EXPECT_EQ(value(a, "A"), "from a");
	EXPECT_EQ(value(a, "B"), "");
	EXPECT_EQ(value(a, "C"), "");
	EXPECT_EQ(value(b, "A"), "from a");
	EXPECT_EQ(value(b, "B"), "from b");
	EXPECT_EQ(value(b, "C"), "from c");
	EXPECT_EQ(value(c, "A"), "from a");
	EXPECT_EQ(value(c, "B"), "");
	EXPECT_EQ(value(c, "C"), "from c");

	// a value that no form sets is looked up in all the parents
	EXPECT_EQ(value(a, "Unset"), "");
	EXPECT_EQ(value(b, "Unset"), "");
	EXPECT_EQ(value(c, "Unset"), "");
}

// d has the parents e and f, both have the parent d
TEST_F(CUTGeorgesFormParents, loopWithTwoParents)
{
	NLMISC::CLightMemDisplayer warnings;
	NLMISC::CLog logger;
	logger.addDisplayer(&warnings);
	NLMISC::CNLWarningOverride override(&logger);

	writeForm("d", "e f", "A");
	writeForm("e", "d", "B");
	writeForm("f", "d", "C");

	NLMISC::CSmartPtr<NLGEORGES::UForm> d = loadForm("d");
	ASSERT_NE(d, nullptr);
	NLMISC::CSmartPtr<NLGEORGES::UForm> e = loadForm("e");
	ASSERT_NE(e, nullptr);
	NLMISC::CSmartPtr<NLGEORGES::UForm> f = loadForm("f");
	ASSERT_NE(f, nullptr);

	// fatal: the lookups below would not end with the loop in place
	ASSERT_EQ(d->getNumParent(), 0u);
	ASSERT_EQ(e->getNumParent(), 1u);
	ASSERT_EQ(f->getNumParent(), 1u);
	ASSERT_EQ(assetErrors(warnings).size(), (size_t)2);

	EXPECT_EQ(value(d, "Unset"), "");
	EXPECT_EQ(value(e, "Unset"), "");
	EXPECT_EQ(value(f, "Unset"), "");
	EXPECT_EQ(value(e, "A"), "from d");
	EXPECT_EQ(value(f, "A"), "from d");
}

// A form that names itself as parent
TEST_F(CUTGeorgesFormParents, selfParent)
{
	NLMISC::CLightMemDisplayer warnings;
	NLMISC::CLog logger;
	logger.addDisplayer(&warnings);
	NLMISC::CNLWarningOverride override(&logger);

	writeForm("self", "self", "A");

	NLMISC::CSmartPtr<NLGEORGES::UForm> self = loadForm("self");
	ASSERT_NE(self, nullptr);
	EXPECT_EQ(self->getNumParent(), 0u);

	std::vector<string> errors = assetErrors(warnings);
	ASSERT_EQ(errors.size(), (size_t)1);
	EXPECT_THAT(errors[0], StrEq("ASSET ERROR: the form '" + formName("self") + "' names itself as a parent, this parent is not used and is left out when the form is saved\n"));

	EXPECT_EQ(value(self, "A"), "from self");
	EXPECT_EQ(value(self, "Unset"), "");
}

// The same check for a parent that a tool inserts into a loaded form
TEST_F(CUTGeorgesFormParents, insertParent)
{
	NLMISC::CLightMemDisplayer warnings;
	NLMISC::CLog logger;
	logger.addDisplayer(&warnings);
	NLMISC::CNLWarningOverride override(&logger);

	writeForm("base", "", "A");
	writeForm("child", "base", "B");
	writeForm("other", "", "C");

	NLMISC::CSmartPtr<NLGEORGES::UForm> base = loadForm("base");
	ASSERT_NE(base, nullptr);
	NLMISC::CSmartPtr<NLGEORGES::UForm> child = loadForm("child");
	ASSERT_NE(child, nullptr);
	NLMISC::CSmartPtr<NLGEORGES::UForm> other = loadForm("other");
	ASSERT_NE(other, nullptr);
	EXPECT_TRUE(assetErrors(warnings).empty());

	NLGEORGES::CForm *baseForm = static_cast<NLGEORGES::CForm *>(base.getPtr());
	NLGEORGES::CForm *childForm = static_cast<NLGEORGES::CForm *>(child.getPtr());
	NLGEORGES::CForm *otherForm = static_cast<NLGEORGES::CForm *>(other.getPtr());

	// the child of a form, and the form itself, are refused as its parent
	EXPECT_FALSE(baseForm->insertParent(0, formName("child"), childForm));
	EXPECT_FALSE(baseForm->insertParent(0, formName("base"), baseForm));
	EXPECT_EQ(baseForm->getParentCount(), 0u);
	std::vector<string> errors = assetErrors(warnings);
	ASSERT_EQ(errors.size(), (size_t)2);
	EXPECT_THAT(errors[0], HasSubstr("names the parent '" + formName("child") + "'"));
	EXPECT_THAT(errors[1], HasSubstr("names itself as a parent"));

	// a form that does not lead back is accepted
	EXPECT_TRUE(baseForm->insertParent(0, formName("other"), otherForm));
	EXPECT_EQ(baseForm->getParentCount(), 1u);
	EXPECT_EQ(value(child, "C"), "from other");
	EXPECT_EQ(assetErrors(warnings).size(), (size_t)2);
}
