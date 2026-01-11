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

#ifndef UT_MISC_COMMAND
#define UT_MISC_COMMAND

#include <nel/misc/command.h>

vector<string>	callList;

class TTest : public NLMISC::ICommandsHandler
{
protected:
	std::string _Name;
public:
	const std::string &getCommandHandlerName() const
	{
		return _Name;
	}

	void setName(const std::string &name)
	{
		nlassert(_Name.empty());
		_Name = name;

		registerCommandsHandler();
	}

	NLMISC_COMMAND_HANDLER_TABLE_BEGIN(TTest)
		NLMISC_COMMAND_HANDLER_ADD(TTest, theCommand1, "help", "args")
		NLMISC_COMMAND_HANDLER_ADD(TTest, theCommand2, "other help", "other args")
	NLMISC_COMMAND_HANDLER_TABLE_END


	NLMISC_CLASS_COMMAND_DECL(theCommand1)
	{
		callList.push_back(_Name+".theCommand1");
		return true;
	}

	NLMISC_CLASS_COMMAND_DECL(theCommand2)
	{
		callList.push_back(_Name+".theCommand2");
		return true;
	}

};

class TTestDerived : public TTest
{
public:
	NLMISC_COMMAND_HANDLER_TABLE_EXTEND_BEGIN(TTestDerived, TTest)
		NLMISC_COMMAND_HANDLER_ADD(TTestDerived, derivedCommand, "help", "args")
		NLMISC_COMMAND_HANDLER_ADD(TTestDerived, commandToOverride, "help", "args")
	NLMISC_COMMAND_HANDLER_TABLE_END

	NLMISC_CLASS_COMMAND_DECL(derivedCommand)
	{
		callList.push_back(_Name+".derivedCommand");
		return true;
	}

	NLMISC_CLASS_COMMAND_DECL(commandToOverride)
	{
		callList.push_back(_Name+".commandToOverride");
		return true;
	}

};

class TTestDerived2 : public TTestDerived
{
public:
	NLMISC_COMMAND_HANDLER_TABLE_EXTEND_BEGIN(TTestDerived2, TTestDerived)
		NLMISC_COMMAND_HANDLER_ADD(TTestDerived2, derivedCommand2, "help", "args")
		NLMISC_COMMAND_HANDLER_ADD(TTestDerived2, commandToOverride, "help", "args")
	NLMISC_COMMAND_HANDLER_TABLE_END

	NLMISC_CLASS_COMMAND_DECL(derivedCommand2)
	{
		callList.push_back(_Name+".derivedCommand2");
		return true;
	}

	NLMISC_CLASS_COMMAND_DECL(commandToOverride)
	{
		callList.push_back(_Name+".command Overidden");
		return true;
	}

};

class TTestDerived3 : public TTestDerived2
{
	// empty class
};

class TTestDerived4 : public TTestDerived3
{
public:
	NLMISC_COMMAND_HANDLER_TABLE_EXTEND_BEGIN(TTestDerived4, TTestDerived3)
		NLMISC_COMMAND_HANDLER_ADD(TTestDerived4, derivedCommand4, "help", "args")
		NLMISC_COMMAND_HANDLER_ADD(TTestDerived4, theCommand1, "help", "args")
	NLMISC_COMMAND_HANDLER_TABLE_END

	NLMISC_CLASS_COMMAND_DECL(derivedCommand4)
	{
		callList.push_back(_Name+".derivedCommand4");
		return true;
	}

	NLMISC_CLASS_COMMAND_DECL(theCommand1)
	{
		callList.push_back(_Name+".recallBase");
		NLMISC_CLASS_COMMAND_CALL_BASE(TTestDerived3, theCommand1);
		return true;
	}
};

class CUTMiscCommand : public Test::Suite
{
	TTest	*t1;
	TTest	*t2;
public:
	CUTMiscCommand()
	{
		TEST_ADD(CUTMiscCommand::createOneInstance);
		TEST_ADD(CUTMiscCommand::createAnotherInstance);
		TEST_ADD(CUTMiscCommand::deleteOneInstance);
		TEST_ADD(CUTMiscCommand::derivedClass);
		TEST_ADD(CUTMiscCommand::derivedClassAndBaseCall);
	}

	void derivedClassAndBaseCall()
	{
		TTestDerived4	t4;
		t4.setName("T4");

		callList.clear();

		NLMISC::ICommand::execute("T4.derivedCommand4", *NLMISC::InfoLog);
		TEST_ASSERT(callList.size() == 1);
		TEST_ASSERT(callList[0] == "T4.derivedCommand4");

		NLMISC::ICommand::execute("T4.theCommand1", *NLMISC::InfoLog);
		TEST_ASSERT(callList.size() == 3);
		TEST_ASSERT(callList[1] == "T4.recallBase");
		TEST_ASSERT(callList[2] == "T4.theCommand1");
	}

	void derivedClass()
	{
		TTestDerived t1;
		t1.setName("T1");
		TTestDerived2 t2;
		t2.setName("T2");

		callList.clear();

		NLMISC::ICommand::execute("T1.theCommand1", *NLMISC::InfoLog);
		TEST_ASSERT(callList.size() == 1);
		TEST_ASSERT(callList[0] == "T1.theCommand1");

		NLMISC::ICommand::execute("T1.derivedCommand", *NLMISC::InfoLog);
		TEST_ASSERT(callList.size() == 2);
		TEST_ASSERT(callList[1] == "T1.derivedCommand");

		NLMISC::ICommand::execute("T1.commandToOverride", *NLMISC::InfoLog);
		TEST_ASSERT(callList.size() == 3);
		TEST_ASSERT(callList[2] == "T1.commandToOverride");
		

		NLMISC::ICommand::execute("T2.theCommand1", *NLMISC::InfoLog);
		TEST_ASSERT(callList.size() == 4);
		TEST_ASSERT(callList[3] == "T2.theCommand1");

		NLMISC::ICommand::execute("T2.derivedCommand", *NLMISC::InfoLog);
		TEST_ASSERT(callList.size() == 5);
		TEST_ASSERT(callList[4] == "T2.derivedCommand");

		NLMISC::ICommand::execute("T2.commandToOverride", *NLMISC::InfoLog);
		TEST_ASSERT(callList.size() == 6);
		TEST_ASSERT(callList[5] == "T2.command Overidden");
	}

	
	void createOneInstance()
	{
		t1 = new TTest;
		t1->setName("inst1");

		TEST_ASSERT(callList.empty());

		NLMISC::ICommand::execute("inst1.theCommand1", *NLMISC::InfoLog);
		TEST_ASSERT(callList.size() == 1);
		TEST_ASSERT(callList[0] == "inst1.theCommand1");

		NLMISC::ICommand::execute("inst1.theCommand2", *NLMISC::InfoLog);
		TEST_ASSERT(callList.size() == 2);
		TEST_ASSERT(callList[0] == "inst1.theCommand1");
		TEST_ASSERT(callList[1] == "inst1.theCommand2");
	}

	void createAnotherInstance()
	{
		t2 = new TTest;
		t2->setName("inst2");

		TEST_ASSERT(callList.size() == 2);

		NLMISC::ICommand::execute("inst2.theCommand1", *NLMISC::InfoLog);
		TEST_ASSERT(callList.size() == 3);
		TEST_ASSERT(callList[0] == "inst1.theCommand1");
		TEST_ASSERT(callList[1] == "inst1.theCommand2");
		TEST_ASSERT(callList[2] == "inst2.theCommand1");

		NLMISC::ICommand::execute("inst2.theCommand2", *NLMISC::InfoLog);
		TEST_ASSERT(callList.size() == 4);
		TEST_ASSERT(callList[0] == "inst1.theCommand1");
		TEST_ASSERT(callList[1] == "inst1.theCommand2");
		TEST_ASSERT(callList[2] == "inst2.theCommand1");
		TEST_ASSERT(callList[3] == "inst2.theCommand2");
	}

	void deleteOneInstance()
	{
		delete t1;

		NLMISC::ICommand::execute("inst1.theCommand2", *NLMISC::InfoLog);
		TEST_ASSERT(callList.size() == 4);
		TEST_ASSERT(callList[0] == "inst1.theCommand1");
		TEST_ASSERT(callList[1] == "inst1.theCommand2");
		TEST_ASSERT(callList[2] == "inst2.theCommand1");
		TEST_ASSERT(callList[3] == "inst2.theCommand2");

	}

};

#endif
