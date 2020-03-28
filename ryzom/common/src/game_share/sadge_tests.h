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

#ifndef SADGE_TESTS_H
#define SADGE_TESTS_H

#include "nel/misc/sstring.h"

#ifdef ENABLE_TESTS


class CTestContext
{
public:
	CTestContext(const NLMISC::CSString& title, const NLMISC::CSString& anticipatedResult)
	{
		_Title= title;
		anticipatedResult.splitLines(_Lines);
		_Index= 0;
		_Errors= false;
		_ErrorLine= ~0u;

		// add self to top of the context stack
		CTestContext*& theTopContext= _getTopContext();
		_NextContext= theTopContext;
		theTopContext= this;
	}

	virtual ~CTestContext()
	{
		// setup a ref to the pointer to this context in the context stack
		CTestContext*& ptr= _getTopContext();
		while (ptr!=this)
		{
			if (ptr==NULL)
				return;
			ptr= ptr->_NextContext;
		}

		// remove from the context stack...
		ptr= _NextContext;

		// make sure we reached the end of th test vector correctly
		if (!_Errors && _Index!=_Lines.size())
		{
			_Errors= true;
			_ErrorLine= _Index;
			if (NLMISC::WarningLog!=NULL)
			{
				nlwarning("TEST FAILED: No more output but expecting: %s",_Lines[_Index].c_str());
			}
		}

		// display success or failure message
		if (NLMISC::InfoLog!=NULL)
		{
			nlinfo("TEST: %s: %s",_Title.c_str(),(_Errors?(NLMISC::toString("FAILED AT LINE %d",_ErrorLine).c_str()):"SUCCESS"));
			if (_Errors)
			{
				// calculate the length of the lines...
				uint32 lineLength=20;
				for (uint32 i=0;i<_Lines.size();++i)
				{
					lineLength= std::max(lineLength,(uint32)_Lines[i].size()+3);
				}

				// display a header line
				nlinfo("     %-*s %s",lineLength,"EXPECTED","FOUND");

				// display the lines that exist in both lines and result vectors
				uint32 line;
				for (line=0;line<_Lines.size() && line<_Result.size();++line)
				{
					nlinfo("%4d %-*s %s",line,lineLength,_Lines[line].c_str(),_Result[line].c_str());
				}

				// display any remaining lines from the lines vector
				for (;line<_Lines.size();++line)
				{
					nlinfo("%4d %s",line,_Lines[line].c_str());
				}

				// display any remaining lines from the result vector
				for (;line<_Result.size();++line)
				{
					nlinfo("%4d %*s %s",line,lineLength,"",_Result[line].c_str());
				}
			}
		}
	}

	static void testOutput(const char* file, uint32 line, const NLMISC::CSString& txt)
	{
		// if we're not in a test context then just quit
		if(_getTopContext()==NULL)
		{
			#ifdef NL_DEBUG
			NLMISC::DebugLog->displayNL("%s: %d: Test output with no context: %s",file,line,txt.c_str());
			#endif
			return;
		}

		_getTopContext()->_testOutput(file,line,txt);
	}

	static bool foundErrors()
	{
		if(_getTopContext()==NULL)
			return false;
		return _getTopContext()->_Errors;
	}

protected:
	virtual void _testOutput(const char* file, uint32 line, const NLMISC::CSString& txt)
	{
		// add the output to our result vector
		_Result.push_back(txt);

		// if there have already been errors then just display a warning
		if (_Errors)
		{
			NLMISC::DebugLog->displayNL("%s: %d: After error: %s",file,line,txt.c_str());
			return;
		}

		// check that this test doesn't provoke an error
		if (_Index>=_Lines.size())
		{
			_Errors=true;
			NLMISC::WarningLog->displayNL("%s: %d: TEST FAILED: Found extra output after end of test: %s",file,line,txt.c_str());
			_ErrorLine=_Index;
			return;
		}

		// check that this test doesn't provoke an error
		if (txt!=_Lines[_Index])
		{
			_Errors=true;
			NLMISC::WarningLog->displayNL("%s: %d: TEST FAILED: \n- EXPECTED - %s\n- FOUND    - %s",file,line,_Lines[_Index].c_str(),txt.c_str());
			_ErrorLine=_Index;
			return;
		}

		// display the no-error message
		nldebug("%s",txt.c_str());
		// increment the index
		++_Index;
	}

	static CTestContext*& _getTopContext()
	{
		static CTestContext* topContext= NULL;
		return topContext;
	}

	NLMISC::CSString _Title;
	NLMISC::CVectorSString _Lines;
	NLMISC::CVectorSString _Result;
	uint32 _Index;
	bool _Errors;
	uint32 _ErrorLine;
	CTestContext* _NextContext;
};

class CTestContextMute: public CTestContext
{
public:
	CTestContextMute(): CTestContext("","")
	{
	}

protected:
	virtual void _testOutput(const char* file, uint32 line, const NLMISC::CSString& txt)
	{
	}
};

#define DOTEST(cmd,result) { CTestContext tc(NLMISC::toString("TEST("__FILE__":%d)",__LINE__),result); cmd; TEST_ASSERT(!tc.foundErrors()) }
#define MUTETEST(cmd) { CTestContextMute tcm; cmd; }
#define TEST(txt_and_args) CTestContext::testOutput(__FILE__,__LINE__,NLMISC::toString txt_and_args )

#else // ENABLE_TESTS

#define DOTEST(cmd,result) while(0) {}
#define MUTETEST(cmd) while(0) {}
#define TEST(txt)  while(0) {}

#endif // ENABLE_TESTS

#endif // HEADER FILE
