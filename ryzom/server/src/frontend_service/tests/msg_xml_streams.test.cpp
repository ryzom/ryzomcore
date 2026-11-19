// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

// The stream classes of msg.xml (CGenericXmlMsgHeaderManager): what the parser makes of them, the signature of the
// file that leaves them out, and the frontend's check of how they are written. RYZOM_MSG_XML is the msg.xml of the
// repository.

#include <gtest/gtest.h>

#include "nel/misc/types_nl.h"
#include "nel/misc/bit_mem_stream.h"
#include "nel/misc/file.h"
#include "nel/misc/md5.h"
#include "nel/misc/path.h"

#include <cstdio>
#include <string>

#include "game_share/generic_xml_msg_mngr.h"
#include "game_share/stream_record.h"

using namespace NLMISC;

#define CHECK(cond) EXPECT_TRUE(cond)

namespace /* anonymous */ {

/// A directory for the files the tests write
std::string testDirectory()
{
	std::string dir = CPath::standardizePath(CPath::getTemporaryDirectory()) + "ryzom_frontend_streams_test";
	CFile::createDirectoryTree(dir);
	return dir;
}

// The class the frontend finds for a message, the way it does (frontend_service.cpp, streamClassOfClientMessage)
static uint8 classOf(CGenericXmlMsgHeaderManager &mngr, const char *name)
{
	CBitMemStream out;
	if (!mngr.pushNameToStream(name, out))
		return 0xFE;
	uint32 payload = 0x12345678;
	out.serial(payload);
	CBitMemStream in(true);
	in.fill(out.buffer(), std::min(out.length(), (uint32)16));
	CGenericXmlMsgHeaderManager::CNodeId node = mngr.getNodeId(in);
	if (!node.isValid())
		return 0xFD;
	return mngr.getNodeStreamClass(node);
}

static void writeFile(const std::string &path, const std::string &text)
{
	FILE *f = fopen(path.c_str(), "wb");
	fwrite(text.data(), 1, text.size(), f);
	fclose(f);
}

static std::string readFile(const std::string &path)
{
	std::string text;
	FILE *f = fopen(path.c_str(), "rb");
	if (!f)
		return text;
	char buffer[4096];
	size_t n;
	while ((n = fread(buffer, 1, sizeof(buffer), f)) != 0)
		text.append(buffer, n);
	fclose(f);
	return text;
}

static NLMISC::CHashKeyMD5 md5Of(const std::string &text)
{
	return NLMISC::getMD5((const uint8 *)text.data(), (uint32)text.size());
}

static std::string stripped(const std::string &text)
{
	std::vector<uint8> buffer(text.begin(), text.end());
	CGenericXmlMsgHeaderManager::removeStreamClasses(buffer);
	return std::string(buffer.begin(), buffer.end());
}

// The signature of msg.xml leaves the stream classes out: the file of the repository has the signature of the file
// before any class was declared (argument 3), byte for byte
TEST(MsgXmlStreams, SignatureLeavesTheClassesOut)
{
	const char *current = RYZOM_MSG_XML;
	const std::string dir = testDirectory();
	const std::string now = readFile(current);
	// The file as it was before it had stream classes (ryzomcore4 797099490), which is known by its MD5
	const std::string old = stripped(now);
	CHECK(md5Of(old).toString() == "b820fd606f07032eee6b01fdb2006d7f");
	const std::string before = dir + "/before.xml";
	writeFile(before, old);
	CHECK(!now.empty() && !old.empty() && now != old);
	CHECK(now.find("<stream_class") != std::string::npos && now.find(" stream=\"") != std::string::npos);
	CHECK(old.find("stream") == std::string::npos || old.find("<stream_class") == std::string::npos);

	CHECK(stripped(now) == old);
	CHECK(stripped(old) == old); // a file without classes is left as it is
	CHECK(CGenericXmlMsgHeaderManager::getSignature(current, false) == md5Of(old));
	CHECK(CGenericXmlMsgHeaderManager::getSignature(before, false) == md5Of(old));
	CHECK(CGenericXmlMsgHeaderManager::getSignature(current, true) == md5Of(old)); // (the file has no carriage returns)

	// The same file with the line ends of a Windows checkout: the frontend's signature of it is that of the old
	// file with those line ends, and the client's second signature that of the old file without them
	std::string nowCrlf, oldCrlf;
	for (size_t i = 0; i < now.size(); ++i)
	{
		if (now[i] == '\n')
			nowCrlf += '\r';
		nowCrlf += now[i];
	}
	for (size_t i = 0; i < old.size(); ++i)
	{
		if (old[i] == '\n')
			oldCrlf += '\r';
		oldCrlf += old[i];
	}
	writeFile(dir + "/crlf.xml", nowCrlf);
	CHECK(stripped(nowCrlf) == oldCrlf);
	CHECK(CGenericXmlMsgHeaderManager::getSignature(dir + "/crlf.xml", false) == md5Of(oldCrlf));
	CHECK(CGenericXmlMsgHeaderManager::getSignature(dir + "/crlf.xml", true) == md5Of(old));

	// More classes, more attributes, tabs, several attributes on a line, a class on the last line without a line end
	const std::string plain = "<client_messages_description>\n\t<branch name=\"A\">\n\t\t<leaf name=\"X\" description=\"a stream=of words\"/>\n\t\t<leaf name=\"Y\" format=\"u8\"/>\n\t</branch>\n  <leaf name=\"C\"/>\n</client_messages_description>\n";
	const std::string withClasses = "<client_messages_description>\n\t<!-- stream_class: some words\n\t  more words -->\n\t<stream_class name=\"one\"/>\n  <stream_class name=\"two\" />\n\t<branch name=\"A\" stream=\"one\">\n\t\t<leaf name=\"X\" description=\"a stream=of words\"/>\n\t\t<leaf name=\"Y\" stream=\"two\" format=\"u8\"/>\n\t</branch>\n  <leaf name=\"C\" stream=\"one\"/>\n</client_messages_description>\n";
	CHECK(stripped(withClasses) == plain);
	CHECK(stripped("<a>\n<stream_class name=\"x\"/>") == "<a>\n");
	CHECK(stripped("<a>\n<!-- stream_class one line -->\n<b/>\n") == "<a>\n<b/>\n");
	CHECK(stripped("<!-- another comment -->\n<b stream=\"x\"/>\n") == "<!-- another comment -->\n<b/>\n");
	CHECK(stripped("") == "");

	// The frontend's check of how the classes are written: nothing to say of the file of the repository, of the one
	// before it, or of the variants above; something to say of each way to write them that the signature would keep,
	// or that would take something else out of it
	CHECK(CGenericXmlMsgHeaderManager::checkStreamClassesText(current).empty());
	CHECK(CGenericXmlMsgHeaderManager::checkStreamClassesText(before).empty());
	writeFile(dir + "/variant.xml", withClasses);
	CHECK(CGenericXmlMsgHeaderManager::checkStreamClassesText(dir + "/variant.xml").empty());
	CHECK(CGenericXmlMsgHeaderManager::checkStreamClassesText(dir + "/crlf.xml").empty());
	const char *faulty[] = {
		"<a>\n<stream_class name=\"x\"/>\n<b name=\"B\"  stream=\"x\"/>\n</a>\n", // two spaces
		"<a>\n<stream_class name=\"x\"/>\n<b name=\"B\"\tstream=\"x\"/>\n</a>\n", // a tab
		"<a>\n<stream_class name=\"x\"/>\n<b name=\"B\" stream = \"x\"/>\n</a>\n", // spaces around =
		"<a>\n<stream_class name=\"x\"/>\n<b name=\"B\"\n   stream=\"x\"/>\n</a>\n", // on the next line
		"<a><stream_class name=\"x\"/>\n<b name=\"B\"/>\n</a>\n", // the element on the line of the root
		"<a>\n<stream_class name=\"x\"/> <b name=\"B\"/>\n</a>\n", // the element with another one
		"<a>\n<stream_class\n name=\"x\"/>\n<b name=\"B\"/>\n</a>\n", // the element on two lines
		"<a>\n<!-- stream_class\n  words --> <b name=\"B\"/>\n</a>\n", // something behind the comment
		"<a>\n<!-- stream_class\n  words\n<b name=\"B\"/>\n</a>\n", // a comment that does not end
		"<a>\n<!--\n stream_class words\n-->\n<b name=\"B\"/>\n</a>\n", // the comment opened on a line of its own
		"<a>\n<!--stream_class words -->\n<b name=\"B\"/>\n</a>\n", // without the space
		"<a>\n<!-- Stream classes (stream_class): words -->\n<b name=\"B\"/>\n</a>\n", // with other words first
		"<a>\n<!-- a \" quote --> <b name=\"B\"  stream=\"x\"/>\n</a>\n", // two spaces behind a comment with a quote
		"<a>\n<b description=\"two\nlines\"  stream=\"x\"/>\n</a>\n", // two spaces behind a value of two lines
		"<a>\n<b name=\"B\" stream=\"x\ny\"/>\n</a>\n", // a value of two lines
		"<a>\n<b name=\"B\" stream='x'/>\n</a>\n", // single quotes
	};
	for (uint i = 0; i < sizeof(faulty) / sizeof(faulty[0]); ++i)
	{
		writeFile(dir + "/faulty.xml", faulty[i]);
		std::vector<std::string> problems = CGenericXmlMsgHeaderManager::checkStreamClassesText(dir + "/faulty.xml");
		if (problems.empty())
			printf("no fault found in variant %u\n", i);
		CHECK(!problems.empty());
	}

	// and nothing to say of files that only look like it
	const char *fine[] = {
		"<a>\n<!-- stream = strings, stream=strings -->\n<b name=\"B\"/>\n</a>\n", // the words in a comment
		"<a>\n<b name='B' description='the stream = of words'/>\n</a>\n", // in a value in single quotes
		"<a>\n<b name=\"B\" description=\"two\nlines stream = x\"/>\n</a>\n", // in a value of two lines
		"<a>\n<!-- a \" quote -->\n<b name=\"B\" description=\"stream = x\"/>\n</a>\n", // behind a comment with a quote
		"<a>\n<streams name=\"B\" stream_id=\"1\" upstream=\"2\" streamed=\"3\"/>\n<stream/>\n</a>\n", // other names
		"<a>\n  <!-- stream_class: words\n   stream=\"name\" more words -->\n  <stream_class name=\"x\" />\r\n\t<b name=\"B\" stream=\"x\"/>\n</a>\n", // as it should be
	};
	for (uint i = 0; i < sizeof(fine) / sizeof(fine[0]); ++i)
	{
		writeFile(dir + "/fine.xml", fine[i]);
		std::vector<std::string> problems = CGenericXmlMsgHeaderManager::checkStreamClassesText(dir + "/fine.xml");
		if (!problems.empty())
			printf("a fault found in fine variant %u: %s\n", i, problems[0].c_str());
		CHECK(problems.empty());
	}

	// A file that is not there
	NLMISC::CHashKeyMD5 none;
	none.clear();
	CHECK(CGenericXmlMsgHeaderManager::getSignature(dir + "/not_there.xml", false) == none);
}

TEST(MsgXmlStreams, ClassesOfTheNodes)
{
	const std::string dir = testDirectory();

	// The msg.xml of the repository
	{
		CGenericXmlMsgHeaderManager mngr;
		mngr.init(RYZOM_MSG_XML);
		CHECK(!mngr.hasStreamClassError());
		CHECK(mngr.getStreamClasses().size() == 1 && mngr.getStreamClasses()[0] == "strings");
		CHECK(classOf(mngr, "STRING_MANAGER:PHRASE_SEND") == 0);
		CHECK(classOf(mngr, "STRING_MANAGER:STRING_RESP") == 0);
		CHECK(classOf(mngr, "STRING_MANAGER:RELOAD_CACHE") == 0);
		CHECK(classOf(mngr, "STRING:CHAT") == CLFECOMMON::NoStreamClass);
		CHECK(classOf(mngr, "STRING:DYN_STRING") == CLFECOMMON::NoStreamClass);
		CHECK(classOf(mngr, "DB_INIT:PLR") == CLFECOMMON::NoStreamClass);
		CHECK(classOf(mngr, "CONNECTION:USER_CHARS") == CLFECOMMON::NoStreamClass);
		CHECK(classOf(mngr, "MODULE_GATEWAY:GATEWAY_MSG") == CLFECOMMON::NoStreamClass);
		CHECK(classOf(mngr, "TARGET:PARTY") == CLFECOMMON::NoStreamClass); // the first node of the file
	}


	// Several classes, a leaf that names another class than its branch, a class nobody uses
	{
		writeFile(dir + "/ok.xml", "<client_messages_description>"
		                           "<stream_class name=\"strings\"/><stream_class name=\"chat\"/><stream_class name=\"spare\"/>"
		                           "<branch name=\"A\" stream=\"chat\"><leaf name=\"X\"/><leaf name=\"Y\" stream=\"strings\"/></branch>"
		                           "<branch name=\"B\"><leaf name=\"X\"/><leaf name=\"Z\" stream=\"chat\"/></branch>"
		                           "<leaf name=\"C\" stream=\"strings\"/>"
		                           "</client_messages_description>");
		CGenericXmlMsgHeaderManager mngr;
		mngr.init(dir + "/ok.xml");
		CHECK(!mngr.hasStreamClassError());
		CHECK(mngr.getStreamClasses().size() == 3 && mngr.getStreamClasses()[1] == "chat" && mngr.getStreamClasses()[2] == "spare");
		CHECK(classOf(mngr, "A:X") == 1);
		CHECK(classOf(mngr, "A:Y") == 0);
		CHECK(classOf(mngr, "B:X") == CLFECOMMON::NoStreamClass);
		CHECK(classOf(mngr, "B:Z") == 1);
		CHECK(classOf(mngr, "C") == 0);
	}

	// A class that is not declared
	{
		writeFile(dir + "/undeclared.xml", "<client_messages_description><stream_class name=\"strings\"/>"
		                                   "<branch name=\"A\" stream=\"chat\"><leaf name=\"X\"/></branch></client_messages_description>");
		CGenericXmlMsgHeaderManager mngr;
		mngr.init(dir + "/undeclared.xml");
		CHECK(mngr.hasStreamClassError());
	}

	// A class declared twice, and one without a name
	{
		writeFile(dir + "/twice.xml", "<client_messages_description><stream_class name=\"a\"/><stream_class name=\"a\"/>"
		                              "<leaf name=\"X\"/></client_messages_description>");
		CGenericXmlMsgHeaderManager mngr;
		mngr.init(dir + "/twice.xml");
		CHECK(mngr.hasStreamClassError());

		writeFile(dir + "/noname.xml", "<client_messages_description><stream_class/><leaf name=\"X\"/></client_messages_description>");
		CGenericXmlMsgHeaderManager mngr2;
		mngr2.init(dir + "/noname.xml");
		CHECK(mngr2.hasStreamClassError());
	}

	// As many classes as a client allows streams, and one more
	{
		std::string classes;
		for (uint i = 0; i < CLFECOMMON::StreamClassMax; ++i)
			classes += toString("<stream_class name=\"c%u\"/>", i);
		writeFile(dir + "/max.xml", "<client_messages_description>" + classes + "<leaf name=\"X\" stream=\"c15\"/></client_messages_description>");
		CGenericXmlMsgHeaderManager mngr;
		mngr.init(dir + "/max.xml");
		CHECK(!mngr.hasStreamClassError());
		CHECK(mngr.getStreamClasses().size() == CLFECOMMON::StreamClassMax);

		writeFile(dir + "/toomany.xml", "<client_messages_description>" + classes + "<stream_class name=\"more\"/><leaf name=\"X\"/></client_messages_description>");
		CGenericXmlMsgHeaderManager mngr2;
		mngr2.init(dir + "/toomany.xml");
		CHECK(mngr2.hasStreamClassError());
	}

}

} /* anonymous namespace */

/* end of file */
