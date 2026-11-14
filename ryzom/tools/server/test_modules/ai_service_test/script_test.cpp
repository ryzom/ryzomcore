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

// The script compiler (script_parser.yacc, script_parser.lex) and the script machine
// (script_vm.cpp) of the AI service, as they are built in the service. The rest of the service is
// not linked: the few things the two use of it are defined below.
//
// With the environment variable RYZOM_AI_SCRIPT_FILE set to a file of scripts, the last test
// compiles each of them as the script of a scenario of a Ring session. The scripts are separated
// by a line that holds the character 0x1E only. With RYZOM_AI_SCRIPT_COMPILE_ALL set as well, a
// script the check of a scenario refuses is compiled all the same, to see what the compiler alone
// does with it.

#include "stdpch.h"
#include "script_compiler.h"
#include "script_vm.h"
#include "ring_scenario_guard.h"

#include "game_share/rt_scenario_check.h"

#include <gtest/gtest.h>

#include <cstdio>
#include <cstdlib>
#include <map>
#include <string>
#include <vector>

using namespace AIVM;
using namespace AICOMP;

extern std::string aiErrorMessage;

//////////////////////////////////////////////////////////////////////////////
// What the compiler and the machine use of the service

uint32 CRingScenarioGuard::s_LoadDepth = 0;

// what the compiler said of the scripts of a scenario it refused, the last one last
static std::vector<std::string> s_RefusedScripts;

void CRingScenarioGuard::refuseScript(const std::string &, const std::string &why)
{
	s_RefusedScripts.push_back(why);
}

CRingScenarioGuard::CLoadScope::CLoadScope(TSessionId sessionId, uint32 instanceNumber)
    : m_PreviousSession(sessionId)
    , m_PreviousInstance(instanceNumber)
    , m_PreviousRefused(0)
{
	++s_LoadDepth;
}

CRingScenarioGuard::CLoadScope::~CLoadScope()
{
	--s_LoadDepth;
}

namespace AICOMP {

CCompiler *CCompiler::_Instance = NULL;

CCompiler::CCompiler()
{
}

// the machine compiles the text of a library with these: no test adds a library
NLMISC::CSmartPtr<const CByteCode> CCompiler::compileCode(const std::string &, const std::string &) const
{
	return NULL;
}

NLMISC::CSmartPtr<const CByteCode> CCompiler::compileCode(const std::vector<std::string> &, const std::string &) const
{
	return NULL;
}

CScriptNativeFuncParams::CScriptNativeFuncParams(const std::string &str, FScrptNativeFunc func)
    : _nbInParams(0)
    , _nbOutParams(0)
    , _va(false)
    , _func(func)
{
	// name_in_out: a 'v' alone as in or out stands for any parameters
	const size_t outIndex = str.find_last_of('_');
	const size_t inIndex = str.find_last_of('_', outIndex - 1);
	_va = str.substr(inIndex + 1, outIndex - inIndex - 1) == "v" || str.substr(outIndex + 1) == "v";
}

// The native functions of the test are the functions of a scenario (the list the AI service and
// the scenario service share), looked up the way the service looks its functions up
CScriptNativeFuncParams *CCompiler::getNativeFunc(const std::string &funcName, const std::string &inparams, const std::string &outparams)
{
	static std::map<std::string, NLMISC::CSmartPtr<CScriptNativeFuncParams> > functions;
	std::string signatures[4];
	signatures[0] = funcName + "_" + inparams + "_" + outparams;
	signatures[1] = funcName + "_v_" + outparams;
	signatures[2] = funcName + "_" + inparams + "_v";
	signatures[3] = funcName + "_v_v";
	for (uint i = 0; i < 4; ++i)
	{
		if (!R2::isRtScenarioScriptFunction(signatures[i]))
			continue;
		NLMISC::CSmartPtr<CScriptNativeFuncParams> &function = functions[signatures[i]];
		if (function.isNull())
			function = new CScriptNativeFuncParams(signatures[i], NULL);
		return function;
	}
	return NULL;
}

} // namespace AICOMP

//////////////////////////////////////////////////////////////////////////////

namespace {

/// A group for the scripts of the tests: it keeps the variables and counts the native calls
class CTestContext : public IScriptContext
{
public:
	CTestContext()
	    : NativeCalls(0)
	{
	}

	virtual std::string getContextName() NL_OVERRIDE { return "test"; }
	virtual void interpretCodeOnChildren(CByteCodeEntry const &) NL_OVERRIDE { }

	virtual float getLogicVar(NLMISC::TStringId varId) NL_OVERRIDE { return Floats[varId]; }
	virtual void setLogicVar(NLMISC::TStringId varId, float value) NL_OVERRIDE { Floats[varId] = value; }
	virtual std::string getStrLogicVar(NLMISC::TStringId varId) NL_OVERRIDE { return Strings[varId]; }
	virtual void setStrLogicVar(NLMISC::TStringId varId, std::string const &value) NL_OVERRIDE { Strings[varId] = value; }
	virtual IScriptContext *getCtxLogicVar(NLMISC::TStringId) NL_OVERRIDE { return NULL; }
	virtual void setCtxLogicVar(NLMISC::TStringId, IScriptContext *) NL_OVERRIDE { }

	virtual IScriptContext *findContext(NLMISC::TStringId const) NL_OVERRIDE { return NULL; }

	virtual void setScriptCallBack(NLMISC::TStringId const &, CByteCodeEntry const &) NL_OVERRIDE { }
	virtual CByteCodeEntry const *getScriptCallBackPtr(NLMISC::TStringId const &) const NL_OVERRIDE { return NULL; }
	virtual void callScriptCallBack(IScriptContext *, NLMISC::TStringId const &, int, std::string const &, std::string const &, CScriptStack *) NL_OVERRIDE { }
	virtual void callNativeCallBack(IScriptContext *, std::string const &, int, std::string const &inParamsSig, std::string const &outParamsSig, CScriptStack *stack) NL_OVERRIDE
	{
		++NativeCalls;
		for (size_t i = 0; i < inParamsSig.size(); ++i)
			stack->pop();
		for (size_t i = 0; i < outParamsSig.size(); ++i)
		{
			if (outParamsSig[i] == 's')
				stack->push(std::string());
			else
				stack->push(0.f);
		}
	}

	float getFloat(const char *name) { return Floats[NLMISC::CStringMapper::map(name)]; }
	std::string getString(const char *name) { return Strings[NLMISC::CStringMapper::map(name)]; }

	std::map<NLMISC::TStringId, float> Floats;
	std::map<NLMISC::TStringId, std::string> Strings;
	uint NativeCalls;
};

/// Compiles the lines of a script the way the service does: between braces
bool compile(const std::string &script, std::vector<size_t> &code)
{
	std::string text = "{ \n" + script + "\n }";
	code.clear();
	return aiCompile(code, text.c_str(), "test", false);
}

bool compiles(const std::string &script)
{
	std::vector<size_t> code;
	return compile(script, code);
}

bool compilesForAScenario(const std::string &script)
{
	CRingScenarioGuard::CLoadScope scope(TSessionId(1), 0);
	std::vector<size_t> code;
	return compile(script, code);
}

void run(CTestContext &context, const std::vector<size_t> &code)
{
	NLMISC::CSmartPtr<CByteCode> byteCode = new CByteCode("test");
	byteCode->_opcodes = code;
	NLMISC::CSmartPtr<CByteCode const> constCode = &(*byteCode);
	CScriptVM::getInstance()->interpretCode(&context, NULL, NULL, CByteCodeEntry(constCode, 0));
}

void compileAndRun(CTestContext &context, const std::string &script)
{
	std::vector<size_t> code;
	ASSERT_TRUE(compile(script, code)) << script;
	run(context, code);
}

std::string nested(char open, char close, uint depth, const std::string &inside)
{
	return std::string(depth, open) + inside + std::string(depth, close);
}

} // namespace

//////////////////////////////////////////////////////////////////////////////
// The compiler

TEST(AiScriptCompiler, AScriptCompilesAndRuns)
{
	CTestContext context;
	compileAndRun(context, "v0 = 1 + 2;\n$text = \"a\";\nif (v0 == 3) { v1 = 5; } else { v1 = 6; }");
	EXPECT_EQ(3.f, context.getFloat("v0"));
	EXPECT_EQ(5.f, context.getFloat("v1"));
	EXPECT_EQ("a", context.getString("$text"));
}

// A compilation reads its own text and nothing else, whatever the text before it was
TEST(AiScriptCompiler, ATextIsCompiledFromItsOwnStart)
{
	const std::string plain = "v0 = 7;\nv1 = v0 + 1;";
	std::vector<size_t> first;
	ASSERT_TRUE(compile(plain, first));

	// texts the compiler leaves before their end, or that end inside a token
	std::vector<std::string> broken;
	broken.push_back("v0 = " + nested('(', ')', 20000, "1") + ";\nv5 = 1;\nv6 = 2;");
	broken.push_back(nested('{', '}', 20000, "v5 = 1;") + "\nv6 = 2;");
	broken.push_back("v0 = 1; ) v5 = 1; v6 = 2;");
	broken.push_back("v0 = \"a text that has no end\nv5 = 1;");
	broken.push_back("v0 = 1; } v5 = 1; { v6 = 2;");
	broken.push_back(std::string(100000, '('));
	broken.push_back("");
	for (size_t i = 0; i < broken.size(); ++i)
	{
		std::vector<size_t> code;
		compile(broken[i], code);

		std::vector<size_t> again;
		ASSERT_TRUE(compile(plain, again)) << "after the text " << i;
		EXPECT_EQ(first, again) << "after the text " << i;

		CTestContext context;
		run(context, again);
		EXPECT_EQ(7.f, context.getFloat("v0")) << "after the text " << i;
		EXPECT_EQ(8.f, context.getFloat("v1")) << "after the text " << i;
		EXPECT_EQ(0.f, context.getFloat("v5")) << "after the text " << i;
		EXPECT_EQ(0.f, context.getFloat("v6")) << "after the text " << i;
	}
}

TEST(AiScriptCompiler, ATextThatIsTooDeepDoesNotCompile)
{
	EXPECT_TRUE(compiles("v0 = " + nested('(', ')', 30, "1") + ";"));
	EXPECT_FALSE(compiles("v0 = " + nested('(', ')', 20000, "1") + ";"));
	EXPECT_FALSE(compiles(nested('{', '}', 20000, "v0 = 1;")));
}

// In the script of a scenario numbers are added, and a number variable is changed by a number
TEST(AiScriptCompiler, AScenarioAddsNumbers)
{
	EXPECT_TRUE(compilesForAScenario("v0 = v1 + 1;"));
	EXPECT_TRUE(compilesForAScenario("v0 = (v1 + 1) * 2 - v2;"));
	EXPECT_TRUE(compilesForAScenario("v0 += 2;"));
	EXPECT_TRUE(compilesForAScenario("$text = \"a\";"));

	EXPECT_FALSE(compilesForAScenario("$text = $text + \"a\";"));
	EXPECT_FALSE(compilesForAScenario("$text = \"a\" + \"b\";"));
	EXPECT_FALSE(compilesForAScenario("v0 = v1 + \"a\";"));
	EXPECT_FALSE(compilesForAScenario("v0 = \"a\" + 1;"));
	EXPECT_FALSE(compilesForAScenario("v0 = @group + 1;"));
	EXPECT_FALSE(compilesForAScenario("v0 += \"a\";"));

	// the scripts of the shard keep what they had
	EXPECT_TRUE(compiles("$text = $text + \"a\";"));
	EXPECT_TRUE(compiles("v0 = v1 + \"a\";"));
}

// A function of a scenario is called, another function is not a function
TEST(AiScriptCompiler, AScenarioCallsItsFunctions)
{
	uint32 count = 0;
	const char *const *functions = R2::getRtScenarioScriptFunctions(count);
	ASSERT_NE(0u, count);
	ASSERT_TRUE(functions != NULL);
	EXPECT_FALSE(compilesForAScenario("()aFunctionThatIsNoFunction();"));
	EXPECT_FALSE(compiles("()aFunctionThatIsNoFunction();"));
}

//////////////////////////////////////////////////////////////////////////////
// The machine

// A script ends when the operations it was given are used up
TEST(AiScriptMachine, AScriptRunsForTheOperationsItWasGiven)
{
	std::vector<size_t> loop;
	ASSERT_TRUE(compile("while (v0 == 0) { v1 = v1 + 1; }", loop));

	CTestContext context;
	CScriptVM::setOpcodeBudget(10000);
	EXPECT_FALSE(CScriptVM::isOpcodeBudgetSpent());
	run(context, loop);
	EXPECT_TRUE(CScriptVM::isOpcodeBudgetSpent());
	EXPECT_GT(context.getFloat("v1"), 100.f);
	EXPECT_LT(context.getFloat("v1"), 10000.f);

	// nothing more runs for that work
	CTestContext other;
	std::vector<size_t> plain;
	ASSERT_TRUE(compile("v0 = 7;", plain));
	run(other, plain);
	EXPECT_EQ(0.f, other.getFloat("v0"));

	// and a script runs again when the count is taken away
	CScriptVM::clearOpcodeBudget();
	EXPECT_FALSE(CScriptVM::isOpcodeBudgetSpent());
	run(other, plain);
	EXPECT_EQ(7.f, other.getFloat("v0"));

	// a script that ends within its operations ends as without a count
	CScriptVM::setOpcodeBudget(10000);
	CTestContext third;
	run(third, plain);
	EXPECT_EQ(7.f, third.getFloat("v0"));
	EXPECT_FALSE(CScriptVM::isOpcodeBudgetSpent());
	CScriptVM::clearOpcodeBudget();
}

// A text a script makes by joining texts has a largest size
TEST(AiScriptMachine, ATextOfAScriptHasASize)
{
	std::vector<size_t> grow;
	ASSERT_TRUE(compile("$text = \"0123456789abcdef\";\nwhile (v0 == 0) { $text = $text + $text; }", grow));

	CTestContext context;
	CScriptVM::setOpcodeBudget(1000000);
	EXPECT_THROW(run(context, grow), EScriptStop);
	EXPECT_FALSE(CScriptVM::isOpcodeBudgetSpent());
	CScriptVM::clearOpcodeBudget();
	EXPECT_GE(context.getString("$text").size(), 16u);
	EXPECT_LE(context.getString("$text").size(), MaxScriptTextSize);
}

// What a script cannot go on from ends its run, and the service goes on
TEST(AiScriptMachine, AValueOfTheWrongTypeStopsTheScript)
{
	CTestContext context;

	// a group is no number to add
	std::vector<size_t> code;
	ASSERT_TRUE(compile("v0 = @group + 1;", code));
	EXPECT_THROW(run(context, code), EScriptStop);

	// a value is taken from a stack that has none
	code.clear();
	code.push_back(CScriptVM::SET_VAR_VAL);
	code.push_back((size_t)NLMISC::CStringMapper::map("v0"));
	code.push_back(CScriptVM::EOP);
	EXPECT_THROW(run(context, code), EScriptStop);

	code.clear();
	code.push_back(CScriptVM::ADD);
	code.push_back(CScriptVM::EOP);
	EXPECT_THROW(run(context, code), EScriptStop);

	// an operation the machine does not have
	code.clear();
	code.push_back((size_t)100000);
	EXPECT_THROW(run(context, code), EScriptStop);

	// the machine runs the next script
	compileAndRun(context, "v0 = 4;");
	EXPECT_EQ(4.f, context.getFloat("v0"));
}

//////////////////////////////////////////////////////////////////////////////
// The compiler and the scripts of a scenario that are too much

namespace {

std::string chain(const std::string &first, const std::string &element, uint count)
{
	std::string text = first;
	text.reserve(first.size() + element.size() * count);
	for (uint i = 0; i < count; ++i)
		text += element;
	return text;
}

} // namespace

// A list of a statement is built in place: its code has the size of the list, and so has the
// memory of its compilation
TEST(AiScriptCompiler, ALongListCompilesInTheSizeOfTheList)
{
	const uint count = 20000;
	std::vector<std::string> scripts;
	scripts.push_back(chain("v0 = 1", "*1", count) + ";");
	scripts.push_back(chain("v0 = 1", " + 1", count) + ";");
	scripts.push_back(chain("if (v0 == 1", " && v0 == 1", count) + ") { v1 = 1; }");
	scripts.push_back(chain("print(\"a\"", ",v0", count) + ");");
	scripts.push_back(chain("(v1", ",v1", count) + ")setEvent(1);");
	for (size_t i = 0; i < scripts.size(); ++i)
	{
		NLMISC::TTime before = NLMISC::CTime::getLocalTime();
		std::vector<size_t> code;
		// the tuple of results is a call that does not exist: it is read all the same
		bool compiled = compile(scripts[i], code);
		NLMISC::TTime after = NLMISC::CTime::getLocalTime();
		printf("[   TIME   ] a list of %u elements, text %u: %u ms, %u words of code\n", count, (uint)i, (uint)(after - before), (uint)code.size());
		if (i + 1 < scripts.size())
		{
			EXPECT_TRUE(compiled) << "text " << i;
			EXPECT_GE(code.size(), (size_t)count) << "text " << i;
			EXPECT_LE(code.size(), (size_t)count * 8) << "text " << i;
		}
		EXPECT_LT(after - before, 20000) << "text " << i;
	}

	CTestContext context;
	compileAndRun(context, chain("v0 = 1", " + 1", 500) + ";\nv1 = 2" + chain("", "*2", 9) + ";");
	EXPECT_EQ(501.f, context.getFloat("v0"));
	EXPECT_EQ(1024.f, context.getFloat("v1"));
}

// The script of a scenario that does not compile gives one refusal, with its first error, and the
// compiler does not read it on past a few errors
TEST(AiScriptCompiler, AScenarioScriptThatDoesNotCompileIsRefusedOnce)
{
	s_RefusedScripts.clear();
	EXPECT_FALSE(compilesForAScenario("v0 = v1 + \"a\";\n" + chain("", "v0 = \"b\" + 1;\n", 2000) + "v0 = 1;"));
	ASSERT_EQ(1u, s_RefusedScripts.size());
	EXPECT_NE(std::string::npos, s_RefusedScripts[0].find("does not compile"));
	EXPECT_NE(std::string::npos, s_RefusedScripts[0].find("line 2"));
	// the errors that are kept, and the line that says the compilation failed
	size_t lines = 0;
	for (size_t i = 0; i < aiErrorMessage.size(); ++i)
		lines += aiErrorMessage[i] == '\n' ? 1 : 0;
	EXPECT_LE(lines, 6u);

	// a character that is no character of the language
	s_RefusedScripts.clear();
	EXPECT_FALSE(compilesForAScenario("v0 = 1 ? 2;"));
	EXPECT_EQ(1u, s_RefusedScripts.size());

	// a script of the shard is not a refusal of a scenario, and the next text compiles
	s_RefusedScripts.clear();
	EXPECT_FALSE(compiles("v0 = ;"));
	EXPECT_TRUE(s_RefusedScripts.empty());
	EXPECT_TRUE(compilesForAScenario("v0 = 1;"));
	EXPECT_TRUE(s_RefusedScripts.empty());
}

//////////////////////////////////////////////////////////////////////////////
// The scripts of a file

TEST(AiScriptCompiler, TheScriptsOfAFileCompileForAScenario)
{
	const char *path = getenv("RYZOM_AI_SCRIPT_FILE");
	if (!path || !path[0])
		GTEST_SKIP() << "RYZOM_AI_SCRIPT_FILE is not set";

	FILE *file = fopen(path, "rb");
	ASSERT_TRUE(file != NULL) << path;
	std::string content;
	char buffer[65536];
	size_t read;
	while ((read = fread(buffer, 1, sizeof(buffer), file)) != 0)
		content.append(buffer, read);
	fclose(file);

	const std::string separator = "\n\x1e\n";
	uint scripts = 0;
	uint refusedByTheCheck = 0;
	uint refusedByTheCompiler = 0;
	uint notCompiled = 0;
	size_t begin = 0;
	while (begin < content.size())
	{
		size_t end = content.find(separator, begin);
		if (end == std::string::npos)
			end = content.size();
		std::string script = content.substr(begin, end - begin);
		begin = end + separator.size();
		++scripts;

		std::string refusal;
		if (!R2::checkRtScenarioScript(script, false, refusal))
		{
			++refusedByTheCheck;
			if (refusedByTheCheck <= 20)
				printf("[  CHECK   ] script %u: %s\n", scripts, refusal.c_str());
			// to see what the compiler alone does with a text the check refuses
			const char *compileAll = getenv("RYZOM_AI_SCRIPT_COMPILE_ALL");
			if (compileAll && compileAll[0])
				printf("[ COMPILER ] script %u, refused by the check: %s\n", scripts, compilesForAScenario(script) ? "compiles" : "does not compile");
			continue;
		}
		if (!compilesForAScenario(script))
		{
			std::string message = aiErrorMessage;
			// a script that does not compile for the shard either is no script that was lost
			if (!compiles(script))
			{
				++notCompiled;
				continue;
			}
			++refusedByTheCompiler;
			if (refusedByTheCompiler <= 20)
				printf("[ COMPILER ] script %u: %s\n", scripts, message.c_str());
		}
	}
	printf("[  COUNT   ] %u scripts, %u that do not compile, %u refused by the check of a script, %u refused by the compiler for a scenario only\n",
	    scripts, notCompiled, refusedByTheCheck, refusedByTheCompiler);
	EXPECT_EQ(0u, refusedByTheCheck);
	EXPECT_EQ(0u, refusedByTheCompiler);
}
