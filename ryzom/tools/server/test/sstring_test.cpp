/* 
	SString test

	project: RYZOM / TEST
*/

#include "nel/misc/types_nl.h"
#include "nel/misc/common.h"
#include "nel/misc/debug.h"
#include "game_share/singleton_registry.h"
#include "nel/misc/sstring.h"

using namespace NLMISC;

static void xmlTest(const NLMISC::CSString& s,bool isCompat,bool isCompatParam,bool isEncoded,const char* encode,const char* encodeParam,const char* decode)
{
	nlinfo("%s: isXMLCompat(f):%s isXMLCompat(t):%s isXMLCode:%s encode(f):%s encode(t):%s decode:%s",s.c_str(),s.isXMLCompatible()?"Y":"N",s.isXMLCompatible(true)?"Y":"N",s.isEncodedXML()?"Y":"N",s.encodeXML().c_str(),s.encodeXML(true).c_str(),s.decodeXML().c_str());
	nlassert(s.isXMLCompatible()==isCompat);
	nlassert(s.isXMLCompatible(true)==isCompatParam);
	nlassert(s.isEncodedXML()==isEncoded);
	nlassert(s.encodeXML()==encode);
	nlassert(s.encodeXML(true)==encodeParam);
	nlassert(s.decodeXML()==decode);
}

static void xmlTestSet()
{
	xmlTest("&hello&",false,false,false,"&amp;hello&amp;","&amp;hello&amp;","&hello&");
	xmlTest("&amp;&",false,false,false,"&amp;amp;&amp;","&amp;amp;&amp;","&&");
	xmlTest("&amp;",true,true,true,"&amp;amp;","&amp;amp;","&");
	xmlTest("&amp;_",true,true,true,"&amp;amp;_","&amp;amp;_","&_");
	xmlTest("_&amp;",true,true,true,"_&amp;amp;","_&amp;amp;","_&");
	xmlTest("_&amp;_",true,true,true,"_&amp;amp;_","_&amp;amp;_","_&_");
	xmlTest("&quot;",true,true,true,"&amp;quot;","&amp;quot;","\"");
	xmlTest("&lt;",true,true,true,"&amp;lt;","&amp;lt;","<");
	xmlTest("&gt;",true,true,true,"&amp;gt;","&amp;gt;",">");
	xmlTest("<",false,false,false,"&lt;","&lt;","<");
	xmlTest(">",false,false,false,"&gt;","&gt;",">");
	xmlTest("&",false,false,false,"&amp;","&amp;","&");
	xmlTest("\"",false,false,false,"&quot;","&quot;","\"");
	xmlTest("\t",true,false,false,"\t","&#x09;","\t");
	xmlTest("\n",true,false,false,"\n","&#x0A;","\n");
	xmlTest("\r",true,false,false,"\r","&#x0D;","\r");
	xmlTest("\x9A",false,false,false,"&#x9A;","&#x9A;","\x9A");
	xmlTest("\xC3",false,false,false,"&#xC3;","&#xC3;","\xC3");
	xmlTest("&#x09;",true,true,true,"&amp;#x09;","&amp;#x09;","\t");
	xmlTest("&#x5A;",true,true,true,"&amp;#x5A;","&amp;#x5A;","\x5A");
	xmlTest("&#xA5;",true,true,true,"&amp;#xA5;","&amp;#xA5;","\xA5");
	xmlTest("&#xA5;&#x5A;",true,true,true,"&amp;#xA5;&amp;#x5A;","&amp;#xA5;&amp;#x5A;","\xA5\x5A");
	xmlTest("&#xA;&#xD;",true,true,true,"&amp;#xA;&amp;#xD;","&amp;#xA;&amp;#xD;","\n\r");
}

static void quoteTest(const NLMISC::CSString& raw,const NLMISC::CSString& quoted)
{
	nlinfo("Testing quotes: %s",quoted.c_str());
	nlassert(raw.quote().unquote()==raw);
	nlassert(quoted.unquote().quote()==quoted);
	nlassert(quoted.quote().unquote()==quoted);
	nlassert(raw.quote()==quoted);
	nlassert(quoted.unquote()==raw);
}

static void quoteTestSet()
{
	quoteTest("\"","\"\\\"\"");
	quoteTest("\\","\"\\\\\"");
	quoteTest("\a","\"\\a\"");
	quoteTest("\b","\"\\b\"");
	quoteTest("\f","\"\\f\"");
	quoteTest("\n","\"\\n\"");
	quoteTest("\r","\"\\r\"");
	quoteTest("\t","\"\\t\"");
	quoteTest("\v","\"\\v\"");
	quoteTest("\x01","\"\\x01\"");
	quoteTest("\xFE","\"\\xFE\"");
	quoteTest("\xef","\"\\xef\"");
}

static void splitWordsTest(const NLMISC::CSString& pre,char separator,const NLMISC::CSString& post)
{
	nlinfo("Testing split: %s",pre.c_str());

	NLMISC::CVectorSString words;
	pre.splitWords(words);

	NLMISC::CSString joined;
	joined.join(words,separator);
	nlassert(joined==post);

	joined.clear();
	joined.join(words,NLMISC::CSString(separator));
	nlassert(joined==post);
}

static void splitJoinTestSet()
{
	splitWordsTest("bob bub",',',"bob,bub");
	splitWordsTest("\t bob\t bub\t ",',',"bob,bub");
	splitWordsTest("\t +bob\t -bub\t ",',',"+,bob,-,bub");
}

static void splitToLineCommentTest(const NLMISC::CSString& pre, const NLMISC::CSString& post)
{
	CSString s= pre;

	// make sure post matches result of split
	nlassert(post==s.splitToLineComment());
	// make sure split is non destructive
	nlassert(s==pre);

	// try splitting destructively
	CSString s2= s.splitToLineComment(true);
	nlassert((s2+s)==pre);
}

static void splitToLineCommentTest2(const NLMISC::CSString& pre, const NLMISC::CSString& postWithEscape, const NLMISC::CSString& postWithoutEscape)
{
	CSString s= pre;

	// make sure post matches result of split
	nlassert(postWithEscape==s.splitToLineComment(false,true));
	// make sure split is non destructive
	nlassert(s==pre);

	// make sure post matches result of split
	nlassert(postWithoutEscape==s.splitToLineComment(false,false));
	// make sure split is non destructive
	nlassert(s==pre);
}

static void splitToLineCommentTestSet()
{
	// some basic strings with no comments
	splitToLineCommentTest("a","a");
	splitToLineCommentTest(" a "," a ");
	splitToLineCommentTest("\"\"","\"\"");
	splitToLineCommentTest(" \" \" "," \" \" ");
	splitToLineCommentTest("\"a","\"a");
	splitToLineCommentTest(" \" a "," \" a ");
	splitToLineCommentTest("a\"","a\"");
	splitToLineCommentTest(" a \" "," a \" ");
	splitToLineCommentTest("/","/");
	splitToLineCommentTest("/ /","/ /");
	splitToLineCommentTest(" / "," / ");
	splitToLineCommentTest("/ ","/ ");
	splitToLineCommentTest(" /"," /");

	// some basic cases with a comment
	splitToLineCommentTest("//","");
	splitToLineCommentTest("a//","a");
	splitToLineCommentTest("//a","");
	splitToLineCommentTest("a//b","a");
	splitToLineCommentTest("\"a\"//b","\"a\"");
	splitToLineCommentTest("a//\"b\"","a");

	// some basic cases with a '//' in a string (no comment)
	splitToLineCommentTest("\"//\"","\"//\"");
	splitToLineCommentTest(" \" // \" "," \" // \" ");
	splitToLineCommentTest("\" // a \"","\" // a \"");

	// some cases with a bit of everything
	splitToLineCommentTest("a\"//b\"//c","a\"//b\"");
	splitToLineCommentTest(" a \" // b \" // c "," a \" // b \" ");
	splitToLineCommentTest("\" // a \" b // c ","\" // a \" b ");

	// test some simeple escape cases
	splitToLineCommentTest2("\\",		"\\",		"\\"		);
	splitToLineCommentTest2("a\\bc",	"a\\bc",	"a\\bc"		);
	splitToLineCommentTest2("\\/",		"\\/",		"\\/"		);
	splitToLineCommentTest2("\\//",		"\\//",		"\\"		);
	splitToLineCommentTest2("\\///",	"\\/",		"\\"		);
	splitToLineCommentTest2("\"//\"",	"\"//\"",	"\"//\""	);
	splitToLineCommentTest2("\\\"//\"//",	"\\\"",			"\\\"//\""	);
	splitToLineCommentTest2("\"\\//\"//",	"\"\\//\"",		"\"\\//\""	);
	splitToLineCommentTest2("\"//\\\"//",	"\"//\\\"//",	"\"//\\\""	);
	splitToLineCommentTest2("\"//\"\\//",	"\"//\"\\//",	"\"//\"\\"	);
	splitToLineCommentTest2("\"//\"//\\",	"\"//\"",		"\"//\""	);
}

static void atoiTest()
{
	// atosi - valid values
	nlassert(NLMISC::CSString("-2147483648").atosi()==-(int)0x80000000u);
	nlassert(NLMISC::CSString("-1").atosi()==-1);
	nlassert(NLMISC::CSString("0").atosi()==0);
	nlassert(NLMISC::CSString("1").atosi()==1);
	nlassert(NLMISC::CSString("2147483647").atosi()==0x7FFFFFFF);
	nlassert(NLMISC::CSString("+2147483647").atosi()==0x7FFFFFFF);

	// atosi - invalid values
	nlassert(NLMISC::CSString("-2147483649").atosi()==0);
	nlassert(NLMISC::CSString("2147483648").atosi()==0);
	nlassert(NLMISC::CSString("123abc").atosi()==0);
	nlassert(NLMISC::CSString("++1").atosi()==0);
	nlassert(NLMISC::CSString("--1").atosi()==0);
	nlassert(NLMISC::CSString("").atosi()==0);

	// atoui - valid values
	nlassert(NLMISC::CSString("0").atoui()==0);
	nlassert(NLMISC::CSString("1").atoui()==1);
	nlassert(NLMISC::CSString("2147483648").atoui()==0x80000000u);
	nlassert(NLMISC::CSString("4294967295").atoui()==0xFFFFFFFFu);

	// atoui - invalid values
	nlassert(NLMISC::CSString("").atoui()==0);
	nlassert(NLMISC::CSString("-1").atoui()==0);
	nlassert(NLMISC::CSString("+1").atoui()==0);
	nlassert(NLMISC::CSString("4294967296").atoui()==0);
}

static void loadAndSaveTest()
{
	const char* normalFile= "string_test.tmp";
	const char* lockedFile= "a/b/c/d/non_existent_directory/string_test_locked.tmp";

	// test a normal case - write text to a file and read it back...
	NLMISC::CSString s0="hello world";
	NLMISC::CSString s1="moby";
	bool r0=s0.writeToFile(normalFile);
	nlassert(r0);
	bool r1=s1.readFromFile(normalFile);
	nlassert(r1);
	nlassert(s0==s1);

	// test a case with an inaccessible file...
	NLMISC::CSString s2="bye bye blackbird";
	NLMISC::CSString s3="mop mop";
	bool r2=s2.writeToFile(lockedFile);
	nlassert(!r2);
	bool r3=s3.readFromFile(lockedFile);
	nlassert(!r3);
	nlassert(s3.empty());
}

class CSStringTest: public IServiceSingleton
{
public:
	void init()
	{
		xmlTestSet();
		quoteTestSet();
		splitJoinTestSet();
		splitToLineCommentTestSet();
		atoiTest();
		loadAndSaveTest();
	}
};

static CSStringTest Test;
