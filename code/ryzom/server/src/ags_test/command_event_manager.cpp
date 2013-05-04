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



#include "command_event_manager.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/file.h"
#include "nel/misc/path.h"
#include "nel/misc/command.h"
#include "nel/misc/variable.h"

#include "game_share/tick_event_handler.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;

//using CCommandEventManager::CScript;
//using CCommandEventManager::CScriptNode;
//using CCommandEventManager::CEnvironment;

#define	EXEC_CHILD_AND_EXIT(child)					if (child) return executeChild(child, env)
#define EXEC_CHILD_AND_EXIT_ON_HALT_ERROR(child)	if (child) { EState state = executeChild(child, env); if (state==Halt || state==Error) return state; }

CCommandEventManager::TScriptMap		CCommandEventManager::_RunningScripts;
string									CCommandEventManager::_Empty("");

/*
 * Constructor
 */
CCommandEventManager::CCommandEventManager()
{
}


//
// Env
//
string	CCommandEventManager::CEnvironment::replace(const string &text)
{
	uint	start = 0, stop;
	string	replaced = text;

	do
	{
		start = replaced.find_first_of('$', start);
		if (start == string::npos || start == replaced.size()-1)
			break;

		if (replaced[start+1] == '$')
		{
			replaced.erase(start, 1);
			++start;
			continue;
		}
		else if (replaced[start+1] == '(')
		{
			stop = replaced.find_first_of(')', start+1);
			if (stop == string::npos)
				break;
			string	val = get(replaced.substr(start+2, stop-start-2));
			replaced.erase(start, stop-start+1);
			replaced.insert(start, val);
			start += val.size();
		}
		else if (replaced[start+1] == '[')
		{
			stop = replaced.find_first_of(']', start+1);
			if (stop == string::npos)
				break;

			string	var = replaced.substr(start+2, stop-start-2);
			string	val = "";
			ICommand::TCommand::iterator	it = ICommand::Commands->find(var);
			if (it != ICommand::Commands->end())
			{
				IVariable	*variable = dynamic_cast<IVariable*>((*it).second);
				if (variable != NULL)
				{
					val = variable->toString();
				}
			}
			replaced.erase(start, stop-start+1);
			replaced.insert(start, val);
			start += val.size();
		}
		else
		{
			++start;
		}
	}
	while (start < replaced.size());

	return replaced;
}

//
// Script nodes
//

// -----------------------------------------------------------
// list node
class CInstListNode : public CCommandEventManager::CScriptNode
{
public:
	vector<CCommandEventManager::CScriptNode*>	Nodes;
	uint					NextToRun;

	CInstListNode() : NextToRun(0) {}

	~CInstListNode()
	{
		uint	i;
		for (i=0; i<Nodes.size(); ++i)
			delete Nodes[i];
	}

	EState execute(CCommandEventManager::CEnvironment &env)
	{
		for (; NextToRun<Nodes.size(); ++NextToRun)
		{
			EXEC_CHILD_AND_EXIT_ON_HALT_ERROR(Nodes[NextToRun]);
		}

		NextToRun = 0;
		return  Ok;
	}
};

// -----------------------------------------------------------
// if node
class CIfNode : public CCommandEventManager::CScriptNode
{
public:
	string		Condition;

	CCommandEventManager::CScriptNode	*TrueStatement,
				*FalseStatement;

	CIfNode() : TrueStatement(NULL), FalseStatement(NULL) {}
	~CIfNode()
	{
		if (TrueStatement)
			delete TrueStatement;
		if (FalseStatement)
			delete FalseStatement;
	}

	EState execute(CCommandEventManager::CEnvironment &env)
	{
		if (CurrentRun)
			return goOnCurrentRun(env);

		if (Root->Verbose)
			nldebug("[%s:%d:DEBUG] if %s", Root->Name.c_str(), Line, Condition.c_str());

		double						res;
		CEvalNumExpr::TReturnState	ev = env.evalExpression(Condition.c_str(), res, NULL);
		if (ev == CEvalNumExpr::NoError)
		{
			if (res != 0.0)
			{
				// exec true statement
				EXEC_CHILD_AND_EXIT(TrueStatement);
			}
			else
			{
				// exec false statement
				EXEC_CHILD_AND_EXIT(FalseStatement);
			}
		}
		else
		{
			return Error;
		}

		return Ok;
	}
};

// -----------------------------------------------------------
// while node
class CWhileNode : public CCommandEventManager::CScriptNode
{
public:
	string		Condition;

	CCommandEventManager::CScriptNode	*Statement;

	CWhileNode() : Statement(NULL) {}
	~CWhileNode()
	{
		if (Statement)
			delete Statement;
	}

	EState execute(CCommandEventManager::CEnvironment &env)
	{
		if (CurrentRun)
		{
			EState	state = goOnCurrentRun(env);
			if (state == Error || state == Halt)
				return state;
		}

		do
		{
			if (Root->Verbose)
				nldebug("[%s:%d:DEBUG] while %s", Root->Name.c_str(), Line, Condition.c_str());

			double						res;
			CEvalNumExpr::TReturnState	ev = env.evalExpression(Condition.c_str(), res, NULL);
			if (ev == CEvalNumExpr::NoError)
			{
				if (res != 0.0)
				{
					// exec true statement
					if (Statement)
					{
						EXEC_CHILD_AND_EXIT_ON_HALT_ERROR(Statement);
					}
				}
				else
				{
					break;
				}
			}
			else
			{
				return Error;
			}
		}
		while (true);

		return Ok;
	}
};

// -----------------------------------------------------------
// cmd node
class CCmdNode : public CCommandEventManager::CScriptNode
{
public:
	string		Command;

	EState	execute(CCommandEventManager::CEnvironment &env)
	{
		if (Root->Verbose)
			nldebug("[%s:%d:DEBUG] cmd %s", Root->Name.c_str(), Line, Command.c_str());

		ICommand::execute(env.replace(Command), *InfoLog);
		return Ok;
	}
};

// -----------------------------------------------------------
// set text node
class CSetTextNode : public CCommandEventManager::CScriptNode
{
public:
	string		Variable;
	string		Text;

	EState	execute(CCommandEventManager::CEnvironment &env)
	{
		if (Root->Verbose)
			nldebug("[%s:%d:DEBUG] set$ %s %s", Root->Name.c_str(), Line, Variable.c_str(), Text.c_str());

		env.set(Variable, env.replace(Text));
		return Ok;
	}
};

// -----------------------------------------------------------
// set text node
class CSetNumericNode : public CCommandEventManager::CScriptNode
{
public:
	string		Variable;
	string		Text;

	EState	execute(CCommandEventManager::CEnvironment &env)
	{
		if (Root->Verbose)
			nldebug("[%s:%d:DEBUG] set# %s %s", Root->Name.c_str(), Line, Variable.c_str(), Text.c_str());

		double						res;
		CEvalNumExpr::TReturnState	ev = env.evalExpression(Text.c_str(), res, NULL);
		if (ev == CEvalNumExpr::NoError)
		{
			env.set(Variable, toString(res));
			return Ok;
		}
		else
		{
			return Error;
		}
	}
};

// -----------------------------------------------------------
// erase node
class CEraseNode : public CCommandEventManager::CScriptNode
{
public:
	string		Variable;

	EState	execute(CCommandEventManager::CEnvironment &env)
	{
		if (Root->Verbose)
			nldebug("[%s:%d:DEBUG] erase %s", Root->Name.c_str(), Line, Variable.c_str());

		env.erase(Variable);
		return Ok;
	}
};

// -----------------------------------------------------------
// wait node
class CWaitNode : public CCommandEventManager::CScriptNode
{
public:
	uint		Wait;
	TGameCycle	StartCycle;

	CWaitNode() : Wait(0), StartCycle(0) {}

	EState	execute(CCommandEventManager::CEnvironment &env)
	{
		if (!StartCycle)
		{
			if (Root->Verbose)
				nldebug("[%s:%d:DEBUG] wait %d", Root->Name.c_str(), Line, Wait);

			StartCycle = CTickEventHandler::getGameCycle();
			return Halt;
		}
		else if (CTickEventHandler::getGameCycle() >= StartCycle+Wait)
		{
			StartCycle = 0;
			return Ok;
		}
		else
		{
			return Halt;
		}
	}
};

// -----------------------------------------------------------
// display node
class CDisplayNode : public CCommandEventManager::CScriptNode
{
public:
	string		Text;

	EState	execute(CCommandEventManager::CEnvironment &env)
	{
		if (Root->Verbose)
			nldebug("[%s:%d:DEBUG] ? %s", Root->Name.c_str(), Line, Text.c_str());

		nlinfo("[%s] %s", Root->Name.c_str(), env.replace(Text).c_str());
		return Ok;
	}
};

// -----------------------------------------------------------
// wait node
class CReceiveNode : public CCommandEventManager::CScriptNode
{
public:
	bool		Waiting;
	string		Expression;

	CReceiveNode() : Waiting(false) {}

	EState	execute(CCommandEventManager::CEnvironment &env)
	{
		if (!Waiting)
		{
			if (Root->Verbose)
				nldebug("[%s:%d:DEBUG] receive %s", Root->Name.c_str(), Line, Expression.c_str());
			Waiting = true;
		}

		double						res;
		CEvalNumExpr::TReturnState	ev = env.evalExpression(Expression.c_str(), res, NULL);
		if (ev != CEvalNumExpr::NoError)
			return Error;
		else
		{
			if (res != 0.0)
			{
				Waiting = false;
				return Ok;
			}
			else
			{
				return Halt;
			}
		}
		return (ev != CEvalNumExpr::NoError) ? Error : ((res != 0.0) ? Ok : Halt);
	}
};

// -----------------------------------------------------------
// wait node
class CSendNode : public CCommandEventManager::CScriptNode
{
public:
	string		Variable;
	string		Text;

	EState	execute(CCommandEventManager::CEnvironment &env)
	{
		if (Root->Verbose)
			nldebug("[%s:%d:DEBUG] send %s %s", Root->Name.c_str(), Line, Variable.c_str(), Text.c_str());

		CCommandEventManager::sendEvent(Variable, env.replace(Text));
		return Ok;
	}
};

// -----------------------------------------------------------
// Parse script

CCommandEventManager::CScript		*parsedScript = NULL;

enum
{
	TOKEN_UNKNOWN = 0,

	TOKEN_END,

	TOKEN_COMMENT,

	TOKEN_IF,
	TOKEN_ELSE,
	TOKEN_ENDIF,
	TOKEN_WHILE,
	TOKEN_ENDLOOP,

	TOKEN_CMD,				// execute a command (while command is evaluated using $(...) syntax)
	TOKEN_SETTEXT,			// set a variable to a text value (evaluated using $(...) syntax)
	TOKEN_SETNUMERIC,		// set a variable to a numeric value (evaluated as a normal numeric expression, variables don't need any $(...) form)
	TOKEN_ERASE,			// erase a variable from env
	TOKEN_WAIT,				// wait for n game cycles
	TOKEN_RECEIVE,			// wait for expression to be evaluated to true
	TOKEN_SEND,
	TOKEN_DISPLAY,
};

class CAssociatedToken
{
public:
	CAssociatedToken(uint token, string chars) : Token(token), Chars(chars) {}
	uint Token; 
	string Chars; 
};

CAssociatedToken	CTable[] = 
{
	CAssociatedToken(TOKEN_IF,			"if"),
	CAssociatedToken(TOKEN_ELSE,		"else"),
	CAssociatedToken(TOKEN_ENDIF,		"endif"),
	CAssociatedToken(TOKEN_WHILE,		"while"),
	CAssociatedToken(TOKEN_ENDLOOP,		"endloop"),

	CAssociatedToken(TOKEN_CMD,			"cmd"),
	CAssociatedToken(TOKEN_SETTEXT,		"setText"),
	CAssociatedToken(TOKEN_SETTEXT,		"set$"),
	CAssociatedToken(TOKEN_SETNUMERIC,	"setNumeric"),
	CAssociatedToken(TOKEN_SETNUMERIC,	"set#"),
	CAssociatedToken(TOKEN_ERASE,		"erase"),
	CAssociatedToken(TOKEN_WAIT,		"wait"),
	CAssociatedToken(TOKEN_RECEIVE,		"receive"),
	CAssociatedToken(TOKEN_SEND,		"send"),
	CAssociatedToken(TOKEN_DISPLAY,		"display"),
	CAssociatedToken(TOKEN_DISPLAY,		"?"),

	CAssociatedToken(TOKEN_COMMENT,		"!"),
};

class CParser
{
public:
	const char	*Text;
	const char	*TokenStart;
	uint		Line;

	CParser() : Text(NULL), Line(0) {}


	void		skipSpace()
	{
		while (*Text != '\0' && (*Text == ' ' || *Text == '\t'))
			++Text;
	}

	void		skipAllSpace()
	{
		while (*Text != '\0' && (*Text == ' ' || *Text == '\t' || *Text == '\r' || *Text == '\n'))
		{
			if (*Text == '\n')
				++Line;
			++Text;
		}
	}

	void		skipToSpace()
	{
		while (*Text != '\0' && (*Text != ' ' && *Text != '\t' && *Text != '\r' && *Text != '\n'))
			++Text;
	}

	void		skipLine()
	{
		while (*Text != '\0' && (*Text != '\n' && *Text != '\r'))
			++Text;
	}

	string		getLine()
	{
		const char *first = Text;
		const char *last = Text;
		while (*Text != '\0' && (*Text != '\n' && *Text != '\r'))
		{
			if (*Text != ' ' && *Text != '\t')
				last = Text;
			++Text;
		}
		return string(first, last-first+1);
	}

	bool		compare(const char *a, const char *t)
	{
		while (*t>32 && *t==*a)
			++t, ++a;

		return ((*a=='\0' || *a==' ' || *a=='\n' || *a=='\r' || *a=='\t') 
			 && (*t=='\0' || *t==' ' || *t=='\n' || *t=='\r' || *t=='\t'));
	}

	string		getWord()
	{
		skipSpace();
		const char	*start = Text;
		skipToSpace();
		return string(start, Text-start);
	}

	uint		getToken()
	{
		skipAllSpace();
		if (*Text == '\0')
			return TOKEN_END;

		TokenStart = Text;
		skipToSpace();
		skipSpace();

		uint	i;
		for (i=0; i<sizeof(CTable)/sizeof(CAssociatedToken); ++i)
			if (compare(CTable[i].Chars.c_str(), TokenStart))
				return CTable[i].Token;

		return TOKEN_UNKNOWN;
	}

	void		rewindToken()
	{
		Text = TokenStart;
	}
};


CParser		Parser;

CCommandEventManager::CScriptNode	*parseInstructionList();

//
CCommandEventManager::CScriptNode	*parseIf()
{
	Parser.skipSpace();

	CIfNode	*node = new CIfNode();
	node->Root = parsedScript;
	node->Line = Parser.Line;
	node->Condition = Parser.getLine();
	Parser.skipLine();

	node->TrueStatement = parseInstructionList();

	uint	token = Parser.getToken();
	if (token == TOKEN_ELSE)
	{
		node->FalseStatement = parseInstructionList();
		token = Parser.getToken();
	}

	if (token != TOKEN_ENDIF)
	{
		nlwarning("[%s] Unknown token found in structure if, expected endif at line %d. Skip line", parsedScript->Name.c_str(), Parser.Line);
		Parser.skipLine();
	}

	return (CCommandEventManager::CScriptNode*)node;
}

//
CCommandEventManager::CScriptNode	*parseWhile()
{
	Parser.skipSpace();

	CWhileNode	*node = new CWhileNode();
	node->Root = parsedScript;
	node->Line = Parser.Line;
	node->Condition = Parser.getLine();
	Parser.skipLine();

	node->Statement = parseInstructionList();

	if (Parser.getToken() != TOKEN_ENDLOOP)
	{
		nlwarning("[%s] Unknown token found in structure while, expected endloop at line %d. Skip line", parsedScript->Name.c_str(), Parser.Line);
		Parser.skipLine();
	}

	return (CCommandEventManager::CScriptNode*)node;
}

//
CCommandEventManager::CScriptNode	*parseCmd()
{
	Parser.skipSpace();

	CCmdNode	*node = new CCmdNode();
	node->Root = parsedScript;
	node->Line = Parser.Line;
	node->Command = Parser.getLine();
	Parser.skipLine();

	return (CCommandEventManager::CScriptNode*)node;
}

//
CCommandEventManager::CScriptNode	*parseSetText()
{
	Parser.skipSpace();

	CSetTextNode	*node = new CSetTextNode();
	node->Root = parsedScript;
	node->Line = Parser.Line;
	node->Variable = Parser.getWord();
	Parser.skipSpace();
	node->Text = Parser.getLine();
	Parser.skipLine();

	return (CCommandEventManager::CScriptNode*)node;
}

//
CCommandEventManager::CScriptNode	*parseSetNumeric()
{
	Parser.skipSpace();

	CSetNumericNode	*node = new CSetNumericNode();
	node->Root = parsedScript;
	node->Line = Parser.Line;
	node->Variable = Parser.getWord();
	Parser.skipSpace();
	node->Text = Parser.getLine();
	Parser.skipLine();

	return (CCommandEventManager::CScriptNode*)node;
}

//
CCommandEventManager::CScriptNode	*parseErase()
{
	Parser.skipSpace();

	CEraseNode	*node = new CEraseNode();
	node->Root = parsedScript;
	node->Line = Parser.Line;
	node->Variable = Parser.getWord();
	Parser.skipLine();

	return (CCommandEventManager::CScriptNode*)node;
}

//
CCommandEventManager::CScriptNode	*parseWait()
{
	Parser.skipSpace();

	CWaitNode	*node = new CWaitNode();
	node->Root = parsedScript;
	node->Line = Parser.Line;
	node->Wait = atoi(Parser.getWord().c_str());
	Parser.skipLine();

	return (CCommandEventManager::CScriptNode*)node;
}

//
CCommandEventManager::CScriptNode	*parseReceive()
{
	Parser.skipSpace();

	CReceiveNode	*node = new CReceiveNode();
	node->Root = parsedScript;
	node->Line = Parser.Line;
	node->Expression = Parser.getLine();
	Parser.skipLine();

	return (CCommandEventManager::CScriptNode*)node;
}

//
CCommandEventManager::CScriptNode	*parseSend()
{
	Parser.skipSpace();

	CSendNode	*node = new CSendNode();
	node->Root = parsedScript;
	node->Line = Parser.Line;
	node->Variable = Parser.getWord();
	Parser.skipSpace();
	node->Text = Parser.getLine();
	Parser.skipLine();

	return (CCommandEventManager::CScriptNode*)node;
}

//
CCommandEventManager::CScriptNode	*parseDisplay()
{
	Parser.skipSpace();

	CDisplayNode	*node = new CDisplayNode();
	node->Root = parsedScript;
	node->Line = Parser.Line;
	node->Text = Parser.getLine();
	Parser.skipLine();

	return (CCommandEventManager::CScriptNode*)node;
}

//
CCommandEventManager::CScriptNode	*parseInstructionList()
{
	CInstListNode	*list = new CInstListNode();
	list->Root = parsedScript;
	list->Line = Parser.Line;

	do
	{
		uint	token = Parser.getToken();

		switch (token)
		{
		case TOKEN_END:
			return list;
			break;

		case TOKEN_IF:
			list->Nodes.push_back(parseIf());
			break;
		case TOKEN_ELSE:
			Parser.rewindToken();
			return list;
			break;
		case TOKEN_ENDIF:
			Parser.rewindToken();
			return list;
			break;

		case TOKEN_WHILE:
			list->Nodes.push_back(parseWhile());
			break;
		case TOKEN_ENDLOOP:
			Parser.rewindToken();
			return list;
			break;

		case TOKEN_CMD:
			list->Nodes.push_back(parseCmd());
			break;
		case TOKEN_SETTEXT:
			list->Nodes.push_back(parseSetText());
			break;
		case TOKEN_SETNUMERIC:
			list->Nodes.push_back(parseSetNumeric());
			break;
		case TOKEN_ERASE:
			list->Nodes.push_back(parseErase());
			break;
		case TOKEN_WAIT:
			list->Nodes.push_back(parseWait());
			break;
		case TOKEN_RECEIVE:
			list->Nodes.push_back(parseReceive());
			break;
		case TOKEN_SEND:
			list->Nodes.push_back(parseSend());
			break;
		case TOKEN_DISPLAY:
			list->Nodes.push_back(parseDisplay());
			break;

		case TOKEN_COMMENT:
			Parser.skipLine();
			break;

		case TOKEN_UNKNOWN:
			nlwarning("[%s] Unknow token at line %d: '%20s...', line ignored", parsedScript->Name.c_str(), Parser.Line, Parser.TokenStart);
			Parser.skipLine();
			break;
		}
	}
	while (true);

	return (CCommandEventManager::CScriptNode*)list;
}





bool	CCommandEventManager::CScript::parse(const char *text)
{
	parsedScript = this;
	Parser.Text = text;
	Parser.Line = 1;
	Root = parseInstructionList();
	return true;
}

// -----------------------------------------------------------
// Runs script till it is finished or it waits for an event
bool	CCommandEventManager::CScript::run()
{
	CScriptNode::EState	state = Root->execute(Environment);
	if (state == CScriptNode::Error)
	{
		nlwarning("[%s] An error occured in script, script ended", Name.c_str());
		return false;
	}
	else if (state == CScriptNode::Ok)
	{
		nlinfo("[%s] Script succesfully terminated, script ended", Name.c_str());
		return false;
	}
	else
	{
		return true;
	}
}

//
void	cbProcessScriptEvent( CMessage& msgin, const string &serviceName, uint16 serviceId )
{
	string	var;
	string	value;

	msgin.serial(var, value);

	CCommandEventManager::processEvent(var, value);
}

TUnifiedCallbackItem CbScriptArray[] = 
{
	{	"S_EVENT",			cbProcessScriptEvent,	}
};

//
void	CCommandEventManager::init()
{
	CUnifiedNetwork::getInstance()->addCallbackArray( CbScriptArray, sizeof(CbScriptArray)/sizeof(CbScriptArray[0]) );
}

//
void	CCommandEventManager::release()
{
	TScriptMap::iterator	it;
	for (it = _RunningScripts.begin(); it != _RunningScripts.end(); ++it)
		delete (*it).second;
	_RunningScripts.clear();
}

//
bool	CCommandEventManager::update()
{
	TScriptMap::iterator	it;
	for (it = _RunningScripts.begin(); it != _RunningScripts.end(); )
		if (!((*it).second->run()))
		{
			TScriptMap::iterator	itn = it++;
			delete (*itn).second;
			_RunningScripts.erase(itn);
		}
		else
		{
			++it;
		}

	return true;
}


//
bool				CCommandEventManager::runScript(const std::string &scriptName, const std::string &scriptFile, bool verbosity)
{
	if (_RunningScripts.find(scriptName) != _RunningScripts.end())
	{
		nlwarning("Script '%s' already running, stop script or wait for its end before restarting it", scriptName.c_str());
		return false;
	}

	string	fname = CPath::lookup(scriptFile, false);

	if (fname == "")
	{
		nlwarning("Can't find file '%s' to launch script '%s'", scriptFile.c_str(), scriptName.c_str());
		return false;
	}

	CIFile	file(fname);

	uint	sz = file.getFileSize();
	char	*buffer = new char[sz+1];

	try
	{
		file.serialBuffer((uint8*)buffer, sz);
		buffer[sz] = '\0';
	}
	catch (EReadError&)
	{
		nlwarning("Unable to read script file '%s' for script '%s'", scriptFile.c_str(), scriptName.c_str());
		delete buffer;
		return false;
	}

	CScript	*script = new CScript(scriptName);
	script->Verbose = verbosity;

	if (!script->parse(buffer))
	{
		nlwarning("Unable to parse script file '%s' for script '%s'", scriptFile.c_str(), scriptName.c_str());
		delete buffer;
		delete script;
		return false;
	}

	script->init();

	nlinfo("Loaded script file '%s' for script '%s' and initialized, ready to run (verbosity %s)", scriptFile.c_str(), scriptName.c_str(), verbosity ? "on":"off");
	_RunningScripts.insert(make_pair<string,CScript*>(scriptName, script));

	delete buffer;

	return true;
}

bool				CCommandEventManager::stopScript(const std::string &scriptName)
{
	TScriptMap::iterator	it = _RunningScripts.find(scriptName);
	if (it == _RunningScripts.end())
	{
		nlwarning("Can't stop script '%s', script not found (already stopped?)", scriptName.c_str());
		return false;
	}

	// delete script
	delete (*it).second;
	_RunningScripts.erase(it);

	return true;
}

const std::string	&CCommandEventManager::getVarInScript(const std::string &scriptName, const std::string &varName)
{
	TScriptMap::iterator	it = _RunningScripts.find(scriptName);
	if (it == _RunningScripts.end())
	{
		nlwarning("Can't find script '%s' to get variable '%s'", scriptName.c_str(), varName.c_str());
		return _Empty;
	}

	return (*it).second->Environment.get(varName);
}

void				CCommandEventManager::setVarInScript(const std::string &scriptName, const std::string &varName, const std::string &value)
{
	TScriptMap::iterator	it = _RunningScripts.find(scriptName);
	if (it == _RunningScripts.end())
	{
		nlwarning("Can't find script '%s' to set variable '%s' to '%s'", scriptName.c_str(), varName.c_str(), value.c_str());
		return;
	}

	(*it).second->Environment.set(varName, value);
}

void				CCommandEventManager::eraseVarInScript(const std::string &scriptName, const std::string &varName)
{
	TScriptMap::iterator	it = _RunningScripts.find(scriptName);
	if (it == _RunningScripts.end())
	{
		nlwarning("Can't find script '%s' to erase variable '%s'", scriptName.c_str(), varName.c_str());
		return;
	}

	(*it).second->Environment.erase(varName);
}

void				CCommandEventManager::sendEvent(const std::string &var, const std::string &value)
{
	CMessage	msg("S_EVNT");
	msg.serial(const_cast<string&>(var));
	msg.serial(const_cast<string&>(value));
	//CUnifiedNetwork::getInstance()->sendAll(msg);

	processEvent(var, value);
}

void				CCommandEventManager::processEvent(const std::string &var, const std::string &value)
{
	TScriptMap::iterator	it;
	for (it = _RunningScripts.begin(); it != _RunningScripts.end(); ++it)
		(*it).second->Environment.set(var, value);
}

//
void				CCommandEventManager::setScriptVerbose(const std::string &scriptName, bool verbosity)
{
	TScriptMap::iterator	it = _RunningScripts.find(scriptName);
	if (it == _RunningScripts.end())
	{
		nlwarning("Can't find script '%s' to set verbosity", scriptName.c_str());
		return;
	}

	(*it).second->Verbose = verbosity;
}

// commands
NLMISC_COMMAND(runScript, "Run a script","<scriptName> <fileName> [verbosity]")
{
	if (args.size() < 2 || args.size() > 3)
		return false;

	bool	verb = false;
	if (args.size() == 3)
		verb = !(args[2] == "0");

	CCommandEventManager::runScript(args[0], args[1], verb);
	return true;
}

NLMISC_COMMAND(stopScript, "Stop a running script","<scriptName>")
{
	if (args.size() != 1)
		return false;

	CCommandEventManager::stopScript(args[0]);
	return true;
}

NLMISC_COMMAND(getVarInScript, "Get a variable value in a script","<scriptName> <varName>")
{
	if (args.size() != 2)
		return false;

	CCommandEventManager::getVarInScript(args[0], args[1]);
	return true;
}

NLMISC_COMMAND(setVarInScript, "Set a variable value in a script","<scriptName> <varName> <value>")
{
	if (args.size() != 3)
		return false;

	CCommandEventManager::setVarInScript(args[0], args[1], args[2]);
	return true;
}

NLMISC_COMMAND(eraseVarInScript, "Erase a variable value in a script","<scriptName> <varName>")
{
	if (args.size() != 2)
		return false;

	CCommandEventManager::eraseVarInScript(args[0], args[1]);
	return true;
}

NLMISC_COMMAND(sendEvent, "Send a script event to all scripts (on all services of the shard)","<varName> <value>")
{
	if (args.size() != 2)
		return false;

	CCommandEventManager::sendEvent(args[0], args[1]);
	return true;
}
