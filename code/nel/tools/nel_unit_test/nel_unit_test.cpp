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

#include <nel/misc/types_nl.h>

#include <fstream>
#include <cpptest.h>

#include <nel/misc/debug.h>

using namespace std;

#ifdef NL_OS_WINDOWS
#	define NEL_UNIT_DATA ""
#endif

#include "ut_misc.h"
#include "ut_net.h"
#include "ut_ligo.h"
// Add a line here when adding a new test MODULE

#ifdef _MSC_VER

#include <Windows.h>

/** A special stream buffer that output in the 'output debug string' feature of windows.
 */
class CDebugOutput : public streambuf
{
	int_type overflow(int_type c)
	{
		string str(pbase(), pptr());

		if (c != traits_type::eof())
			str += c;
		OutputDebugString(str.c_str() );

		return c;
	}
};
// The instance of the streambug
ostream msvDebug(new CDebugOutput);

#endif

static void usage()
{
	cout << "usage: mytest [MODE]\n"
		 << "where MODE may be one of:\n"
		 << "  --compiler\n"
		 << "  --html\n"
		 << "  --text-terse (default)\n"
		 << "  --text-verbose\n";
	exit(0);
}

static CUniquePtr<Test::Output> cmdline(int argc, char* argv[])
{
	if (argc > 2)
		usage(); // will not return
	
	Test::Output* output = 0;
	
	if (argc == 1)
		output = new Test::TextOutput(Test::TextOutput::Verbose);
	else
	{
		const char* arg = argv[1];
		if (strcmp(arg, "--compiler") == 0)
		{
#ifdef _MSC_VER
			output = new Test::CompilerOutput(Test::CompilerOutput::MSVC, msvDebug);
#elif defined(__GNUC__)
			output = new Test::CompilerOutput(Test::CompilerOutput::GCC);
#else
			output = new Test::CompilerOutput;
#endif
		}
		else if (strcmp(arg, "--html") == 0)
			output =  new Test::HtmlOutput;
		else if (strcmp(arg, "--text-terse") == 0)
			output = new Test::TextOutput(Test::TextOutput::Terse);
		else if (strcmp(arg, "--text-verbose") == 0)
			output = new Test::TextOutput(Test::TextOutput::Verbose);
		else
		{
			cout << "invalid commandline argument: " << arg << endl;
			usage(); // will not return
		}
	}

	return CUniquePtr<Test::Output>(output);
}

// Main test program
//
int main(int argc, char *argv[])
{
	static const char *outputFileName = "result.html";

	// init Nel context
	new NLMISC::CApplicationContext;

	// disable nldebug messages in logs in Release
#ifdef NL_RELEASE
	NLMISC::DisableNLDebug = true;
#endif

	NLMISC::createDebug(NULL);

#ifndef NL_DEBUG
	NLMISC::INelContext::getInstance().getDebugLog()->removeDisplayer("DEFAULT_SD");
	NLMISC::INelContext::getInstance().getInfoLog()->removeDisplayer("DEFAULT_SD");
	NLMISC::INelContext::getInstance().getWarningLog()->removeDisplayer("DEFAULT_SD");
	NLMISC::INelContext::getInstance().getErrorLog()->removeDisplayer("DEFAULT_SD");
#endif // NL_DEBUG

	bool noerrors = false;

	try
	{
		Test::Suite ts;

		ts.add(std::auto_ptr<Test::Suite>(new CUTMisc));
		ts.add(std::auto_ptr<Test::Suite>(new CUTNet));
		ts.add(std::auto_ptr<Test::Suite>(new CUTLigo));
		// Add a line here when adding a new test MODULE

		CUniquePtr<Test::Output> output(cmdline(argc, argv));
		noerrors = ts.run(*output);

		Test::HtmlOutput* const html = dynamic_cast<Test::HtmlOutput*>(output.get());
		if (html)
		{
			std::ofstream fout(outputFileName);
			html->generate(fout, true, "NeL");
		}
	}
	catch (...)
	{
		cout << "unexpected exception encountered";
		return EXIT_FAILURE;
	}
	if(noerrors)
		nlinfo("No errors during unit testing");
	else
		nlwarning("Errors during unit testing");
	return noerrors?EXIT_SUCCESS:EXIT_FAILURE;
}
