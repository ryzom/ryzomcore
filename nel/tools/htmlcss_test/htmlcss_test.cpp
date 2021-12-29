/*
 * File:   main.cpp
 * Author: Karu <nimetu@gmail.com>
 *
 * Created on 2015-04-11
 */

typedef struct _xmlNode xmlNode;

#include <string>
#include <fstream>

#include "nel/misc/types_nl.h"
#include "nel/misc/file.h"
#include "nel/misc/path.h"

#include "nel/gui/html_parser.h"
#include "nel/gui/css_parser.h"
#include "nel/gui/html_element.h"
#include "nel/gui/css_style.h"

#include "nel/gui/libwww.h"

using namespace std;
using namespace NLMISC;
using namespace NLGUI;

sint indent { 0 };

// ***************************************************************************
void checkRuleset(CHtmlElement &elm, CCssStyle &style, TStyleVec testset, bool exists)
{
	bool verbose = false;
	std::string existsMessage = exists ? "exist" : "unset";
	for (auto it : testset)
	{
		bool failed = (exists != style.hasStyle(it.first));
		if (failed)
		{
			bool failed2 = true;
			if (it.first == "font-size")
			{
				printf("[%s]: font-size: %d; expected '%s'\n", existsMessage.c_str(), style.Current.FontSize, it.second.c_str());
				printf("     (%s)\n", elm.toString().c_str());
				failed2 = false;
			}
			else if (it.first == "background-color")
			{
				printf("[%s]: background-color: '%s'; expected '%s'\n", existsMessage.c_str(), style.Current.Background.color.toString().c_str(), it.second.c_str());
				printf("     (%s)\n", elm.toString().c_str());
				failed2 = false;
			}

			if (failed2)
			{
				printf("[%s] FAIL: '%s': '%s'\n", existsMessage.c_str(), it.first.c_str(), it.second.c_str());
				printf("     (%s)\n", elm.toString().c_str());
				for (auto it2 : style.Current.StyleRules)
					printf("'%s': '%s'\n", it2.first.c_str(), it2.second.c_str());
			}
		}
		else if (exists && !style.checkStyle(it.first, it.second))
		{
			printf("[%s] FAIL: expecting '%s': '%s', got '%s'\n", existsMessage.c_str(), it.first.c_str(), it.second.c_str(), style.getStyle(it.first).c_str());
			printf("     (%s)\n", elm.toString().c_str());
		}
		else if (!failed)
		{
			if (verbose)
				printf("[%s] PASS: '%s': '%s'\n", existsMessage.c_str(), it.first.c_str(), it.second.c_str());
		}
	}
}

// ***************************************************************************
void recursiveHtmlRender(CHtmlElement &elm, CCssStyle &style)
{
	bool verbose = false;
	if (elm.Type == CHtmlElement::TEXT_NODE)
	{
		std::string val = trim(elm.Value);
		if (verbose)
			if (!val.empty())
				printf("[%d] '%s'\n", indent, val.c_str());
	}
	else if (elm.Type == CHtmlElement::ELEMENT_NODE)
	{
		style.pushStyle();

		if (verbose)
			printf("========= '%s'\n", elm.toString().c_str());

		style.getStyleFor(elm);
		style.applyStyle(elm.Style);
		if (elm.hasAttribute("data-ruleset"))
		{
			TStyleVec testset = CCssParser::parseDecls(elm.getAttribute("data-ruleset"));
			checkRuleset(elm, style, testset, true);
		}

		if (elm.hasAttribute("data-ruleunset"))
		{
			TStyleVec testset = CCssParser::parseDecls(elm.getAttribute("data-ruleunset"));
			checkRuleset(elm, style, testset, false);
		}

		if (elm.hasAttribute("data-ruleset-before"))
		{
			TStyleVec testset = CCssParser::parseDecls(elm.getAttribute("data-ruleset-before"));

		}

		for (auto it = elm.Children.begin(); it != elm.Children.end(); ++it)
		{
			recursiveHtmlRender(*it, style);
		}

		style.popStyle();
	}
}

// ***************************************************************************
void runTestOnFile(const std::string &filename)
{
	CHtmlElement dom;

	CHtmlParser htmlParser;

	std::vector<CHtmlParser::StyleLink> links;
	std::vector<std::string> styles;
	//, elm, styles, links

	ifstream f(filename);
	if (!f.is_open())
	{
		printf("!! failed to open file '%s'\n", filename.c_str());
		return;
	}

	printf(": %s\n", filename.c_str());
	std::string htmlString;
	std::string line;
	while (getline(f, line))
		htmlString += line;

	htmlParser.getDOM(htmlString, dom, styles, links);

	CCssStyle style;
	for (std::string s : styles)
	{
		if (!s.empty())
			style.parseStylesheet(s);
	}

	for (auto it = dom.Children.begin(); it != dom.Children.end(); ++it)
		recursiveHtmlRender(*it, style);
}

// ***************************************************************************
int main(int argc, const char *argv[])
{
	CApplicationContext *appContext = new CApplicationContext;

	// htmlcss_test file.html
	if (argc == 2)
	{
		runTestOnFile(argv[1]);
	}
	else
	{
		std::vector<std::string> result;
		CPath::getPathContent("tests/", true /*recursive*/, false /*wantDir*/, true /*wantFile*/, result, NULL /*callback*/, true /*showEverything*/);
		printf(":: got %ld files\n", result.size());
		for (const auto &fname : result)
		{
			if (endsWith(fname, ".html") && fname != "tests/XX-template.html")
				runTestOnFile(fname);
		}
	}

	printf(">>> all done\n");
	return EXIT_SUCCESS;
}
