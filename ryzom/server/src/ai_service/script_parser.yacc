%{

#include "stdpch.h"
#include "script_compiler.h"
#include <list>

using namespace std;
using namespace NLMISC;
using namespace AIVM;
using namespace AICOMP;

// Define this to output verbose debug parsing info
//#define AI_COMP_DEBUG

extern int aierrorline (const char *s, int);
extern int aierror (const char *s);
extern int ailex ();
extern int aiparse ();
extern void addSignature (char *dest, char *src);
extern int	aiErrorCount;
extern int	aiLine;
extern char aiFile[512];
extern const char *aiInputScript;
extern uint aiInputScriptLength;
extern void aiClean ();
extern int aidebug;			/*  nonzero means print parse trace	*/
#define fprintf myfprintf
int myfprintf (FILE *file, const char *format, ...)
{
	int result;
	va_list args;
	va_start( args, format );
	if (file == stderr)
	{
		char buffer[1024];
		result = vsnprintf(buffer, 1024, format, args);
		nldebug (buffer);
	}
	else
	{
		result = vfprintf (file, format, args);
	}
	va_end( args );
	return result;
}

std::vector<size_t>	*aiRoot;
string aiErrorMessage;

// Main method
bool aiCompile (std::vector<size_t> &dest, const char *script, const char *scriptName, bool win32Report = false)
{
#ifdef AI_COMP_DEBUG
	aidebug = 1;
#else // AI_COMP_DEBUG
	aidebug = 0;
#endif // AI_COMP_DEBUG
	aiLine = 1;
	aiErrorCount = 0;
	aiInputScript = script;
	aiInputScriptLength = (uint)strlen (script);
	strcpy (aiFile, scriptName);
	aiRoot = NULL;
	
	nldebug("script compilation of %s", scriptName);
	aiErrorMessage = toString ("script compilation of %s\n", scriptName);
	int err = aiparse();
	bool error = err || aiErrorCount;
	if (error)
	{
		nlwarning ("compilation failed for %s - %d error(s)", scriptName, aiErrorCount);
		aiErrorMessage += toString ("compilation failed for %s - %d error(s)\n", scriptName, aiErrorCount);
	}
	else
	{
		nlassert (aiRoot);
		nldebug ("compilation success. (code size %d)", aiRoot->size()*4);
		aiErrorMessage += toString ("compilation success. (code size %d)\n", aiRoot->size()*4);
		dest = *aiRoot;
	}

#ifdef NL_OS_WINDOWS
	if (win32Report)
		MessageBox (NULL, aiErrorMessage.c_str (), "AI Script Compiler", MB_OK|(error?MB_ICONEXCLAMATION:MB_ICONINFORMATION));
#endif // NL_OS_WINDOWS

	// Clean all
	aiClean ();
	return !error;
}

/* The parsing tree is composed of different types of node:
 - Opcode for terminal opcode node
 - ByteCode for a bunch of code node
 - ByteCodeList for bunch of code when maintening substructure is important (for rand)
 - Case for storing case index and code
 - Cases for storing list of case

 Opcode, ByteCode and ByteCodeList can be appended to a ByteCode node.
 */

// Memory to delete
vector<vector<size_t> *>			NodeToClear;
vector<list<vector<size_t> *> *>	ListToClear;
vector<CCase *>						CaseToClear;
vector<map<size_t, CCase *> *>		SwitchToClear;

void aiClean ()
{
	uint i;
	for (i=0; i<NodeToClear.size (); i++)
		delete NodeToClear[i];
	for (i=0; i<SwitchToClear.size (); i++)
		delete SwitchToClear[i];
	for (i=0; i<CaseToClear.size (); i++)
		delete CaseToClear[i];
	for (i=0; i<ListToClear.size (); i++)
		delete ListToClear[i];
	NodeToClear.clear ();
	ListToClear.clear ();
	CaseToClear.clear ();
	SwitchToClear.clear ();
}

void aiOutputError(int line, const char *format, ...)
{
	aiErrorCount++;
	va_list args;
	va_start (args, format);
	char buffer[1024];
	vsnprintf (buffer, sizeof (buffer), format, args);
	va_end (args);
	nlwarning ("%s(%d):%s", aiFile, line, buffer);
	aiErrorMessage += toString ("%s(%d):%s\n", aiFile, line, buffer);
}

// *** Node

void createNode (CByteCodeYacc &dest)
{
	dest.ByteCode = new vector<size_t>;
	dest.Signature[0] = 0;
	NodeToClear.push_back (dest.ByteCode);
}

void addNode (CByteCodeYacc &dest, size_t src)
{
	dest.ByteCode->push_back (src);
}

void addNode (CByteCodeYacc &dest, const AICOMP::COpcodeYacc &src)
{
	dest.ByteCode->push_back (src.Opcode);
}

void addNode (CByteCodeYacc &dest, const CByteCodeYacc &src)
{
	dest.ByteCode->insert (dest.ByteCode->end(), src.ByteCode->begin (), src.ByteCode->end ());
}

void addNode (CByteCodeYacc &dest, const CByteCodeListYacc &src)
{
	list<vector<size_t> * >::iterator ite = src.ByteCodeList->begin();
	while (ite != src.ByteCodeList->end())
	{
		dest.ByteCode->insert (dest.ByteCode->end(), (*ite)->begin (), (*ite)->end ());
		ite++;
	}	
}

// *** List

void createList (CByteCodeListYacc &dest)
{
	dest.ByteCodeList = new list<vector<size_t> *>;
	dest.Signature[0] = 0;
	ListToClear.push_back (dest.ByteCodeList);
}

void addNode (CByteCodeListYacc &dest, const CByteCodeYacc &src)
{
	dest.ByteCodeList->push_back (src.ByteCode);
}

// Returns the size of the children bytecode
uint getChildrenByteCodeSize (const list<vector<size_t> * > *l)
{
	uint size = 0;
	list<vector<size_t> * >::const_iterator ite = l->begin();
	while (ite != l->end())
	{
		size += (uint)(*ite)->size ();
		ite++;
	}	
	return size;
}

// *** Case

void createCase (CCaseYacc &dest, COpcodeYacc &_case, CByteCodeYacc &byteCode)
{
	dest.Case  = new CCase;
	dest.Case->Case = _case.Opcode;
	dest.Case->ByteCode = byteCode.ByteCode;
	dest.Case->Line = _case.Line;	
	strcpy (dest.Signature, _case.Signature);
	strcpy (dest.Case->Signature, _case.Signature);
	CaseToClear.push_back (dest.Case);
}

// *** Switch

void createSwitch (CSwitchYacc &dest)
{
	dest.Cases = new map<size_t, CCase *>;
	dest.Signature[0] = 0;
	SwitchToClear.push_back (dest.Cases);
}

void addNode (CSwitchYacc &dest, CCase *src)
{
	dest.Cases->insert (map<size_t, CCase*>::value_type (src->Case, src));
}

// Returns the size of the children bytecode
uint getChildrenByteCodeSize (const map<size_t, CCase *> *l)
{
	uint size = 0;
	map<size_t, CCase *>::const_iterator ite = l->begin ();
	while (ite != l->end ())
	{
		size += (uint)ite->second->ByteCode->size();
		ite ++;
	}	
	return size;
}

// Write native function code
void nativeFunc (CByteCodeYacc &dest, size_t name, const char *in, const char *out)
{
	string funcName = CStringMapper::unmap ((TStringId)name);
	string inParamsSig = in;
	string outParamsSig = out;
	
	// Get the native function
	CScriptNativeFuncParams *funcParam=CCompiler::getNativeFunc(funcName, inParamsSig, outParamsSig);
	if (!funcParam)
	{
		string signature = funcName + "_" + inParamsSig + "_" + outParamsSig;
		aiOutputError (aiLine, "Unknown function name or bad parameters %s", signature.c_str ());
	}
	else
	{
		size_t mode = 0;
		if (funcParam->_va)
			mode |= 1; // :KLUDGE: Hardcoded 1 :TODO: replace with a named constant
		
		TStringId inStrId;
		TStringId outStrId;
		inStrId = CStringMapper::map(inParamsSig);
		outStrId = CStringMapper::map(outParamsSig);
		
		// Add the node
		addNode (dest, CScriptVM::NATIVE_CALL);
		addNode (dest, name);
		addNode (dest, mode);
		addNode (dest, *((size_t*)&inStrId));
		addNode (dest, *((size_t*)&outStrId));
	}
}

#define NODE0(dest) createNode (dest);
#define NODE1(dest,a) createNode (dest); addNode (dest, a);
#define NODE2(dest,a,b) createNode (dest); addNode (dest, a); addNode (dest, b);
#define NODE3(dest,a,b,c) createNode (dest); addNode (dest, a); addNode (dest, b); addNode (dest, c);
#define NODE4(dest,a,b,c,d) createNode (dest); addNode (dest, a); addNode (dest, b); addNode (dest, c); addNode (dest, d);
#define NODE5(dest,a,b,c,d,e) createNode (dest); addNode (dest, a); addNode (dest, b); addNode (dest, c); addNode (dest, d); addNode (dest, e);
#define NODE6(dest,a,b,c,d,e,f) createNode (dest); addNode (dest, a); addNode (dest, b); addNode (dest, c); addNode (dest, d); addNode (dest, e); addNode (dest, f);
#define NODE7(dest,a,b,c,d,e,f,g) createNode (dest); addNode (dest, a); addNode (dest, b); addNode (dest, c); addNode (dest, d); addNode (dest, e); addNode (dest, f); addNode (dest, g);
#define NODE8(dest,a,b,c,d,e,f,g,h) createNode (dest); addNode (dest, a); addNode (dest, b); addNode (dest, c); addNode (dest, d); addNode (dest, e); addNode (dest, f); addNode (dest, g); addNode (dest, h);

#define TYPEF(dest) dest.Signature[0] = 'f'; dest.Signature[1] = 0;
#define TYPEL(dest) dest.Signature[0] = 'l'; dest.Signature[1] = 0;
#define TYPES(dest) dest.Signature[0] = 's'; dest.Signature[1] = 0;
#define TYPEC(dest) dest.Signature[0] = 'c'; dest.Signature[1] = 0;
#define TYPE1(dest,a) strcpy (dest.Signature, a.Signature);
#define TYPE2(dest,a,b) strcpy (dest.Signature, a.Signature); addSignature (dest.Signature, b.Signature);

#define ERROR_DETECTED(a,b) NODE0 (a); aiOutputError (aiLine, b)

%}

%start script

%union	{
	AICOMP::COpcodeYacc			Opcode;
	AICOMP::COperatorYacc		Operator;
	AICOMP::CByteCodeYacc		ByteCode;
	AICOMP::CByteCodeListYacc	ByteCodeList;
	AICOMP::CCaseYacc			Case;
	AICOMP::CSwitchYacc			Switch;
	struct {} 					Nothing;
}

%token	<Nothing>	TOKEN_IF TOKEN_ELSE TOKEN_WHILE TOKEN_PRINT TOKEN_LOG TOKEN_CASE TOKEN_POINT
%token	<Nothing>	TOKEN_SEPARATOR TOKEN_PV TOKEN_PP TOKEN_LP TOKEN_LA TOKEN_RP TOKEN_RA TOKEN_ASSIGNATOR
%token	<Nothing>	TOKEN_SWITCH TOKEN_RAND 

%token	<Opcode> 	TOKEN_NUMBER TOKEN_ONCHILDREN TOKEN_CHAIN TOKEN_NAME TOKEN_STRNAME TOKEN_CTXNAME TOKEN_LOGIC
%token	<Opcode> 	TOKEN_INCRDECR TOKEN_ADD TOKEN_SUB

%token	<Operator>	TOKEN_COMP TOKEN_ASSIGN TOKEN_FACTOR

%type	<Opcode>	caseIndex

%type	<ByteCode>	script
%type	<ByteCode>	condition
%type	<ByteCode>	expression
%type	<ByteCode>	setFunction
%type	<ByteCode>	context
%type	<ByteCode>	function
%type	<ByteCode>	call
%type	<ByteCode>	nativeFunc
%type	<ByteCode>	nativeOtherFunc
%type	<ByteCode>	setFromFuncGet
%type	<ByteCode>	setFromFuncSet
%type	<ByteCode>	setFromFunction
%type	<ByteCode>	tupleElem
%type	<ByteCode>	tuple
%type	<ByteCode>	lValue
%type	<ByteCode>	readVar
%type	<ByteCode>	writeVar
%type	<ByteCode>	expressions
%type	<ByteCode>	params
%type	<ByteCode>	exp
%type	<ByteCode>	printContent
%type	<ByteCode>	printString
%type	<ByteCode>	logString
%type	<ByteCode>	statement
%type	<ByteCode>	openStatement
%type	<ByteCode>	closedStatement
%type	<ByteCode>	randEx
%type	<ByteCode>	onChildren
%type	<ByteCode>	switch

%type	<ByteCodeList>	statements
%type	<ByteCodeList>	statementBlock

%type	<Case>		case

%type	<Switch>	cases

%left TOKEN_LOGIC 
%left TOKEN_COMP
%left TOKEN_ADD TOKEN_SUB
%left TOKEN_FACTOR

%%

script:			statements { NODE1 ($$, $1); aiRoot = $$.ByteCode; }

condition:		condition TOKEN_LOGIC condition { NODE3 ($$, $1, $3, $2); TYPEL ($$); }
				| TOKEN_LP condition TOKEN_RP { $$ = $2; }
				| expression TOKEN_COMP expression
				{ 
					if ($1.getType () != $3.getType ()) aiOutputError (aiLine, "the left and right '%s' expressions have not the same type : left is a %s and right is a %s",
						$2.Operator, $1.getType (), $3.getType ());
					NODE3 ($$, $1, $3, $2); TYPEL ($$); 
				}

expression:		expression TOKEN_ADD expression 
				{ 
					NODE3 ($$, $1, $3, $2); TYPEF ($$); 
				}
				| expression TOKEN_SUB expression 
				{ 
					if (!$1.isFloat ()) aiOutputError (aiLine, "the left '-' expression must be a float but it is a %s", $1.getType ());
					if (!$3.isFloat ()) aiOutputError (aiLine, "the right '-' expression must be a float but it is a %s", $3.getType ());
					NODE3 ($$, $1, $3, $2); TYPEF ($$); 
				}
				| expression TOKEN_FACTOR expression
				{ 
					if (!$1.isFloat ()) aiOutputError (aiLine, "the left '%s' expression must be a float but it is a %s", $2.Operator, $1.getType ());
					if (!$3.isFloat ()) aiOutputError (aiLine, "the right '%s' expression must be a float but it is a %s", $2.Operator, $3.getType ());
					NODE3 ($$, $1, $3, $2); TYPEF ($$); 
				}
				| TOKEN_LP expression TOKEN_RP { $$ = $2; }
				| readVar { $$ = $1; }

setFunction:	TOKEN_NAME TOKEN_LP TOKEN_RP statementBlock
				{ 
					// Get the size of the total byte code of statementBlock
					int sizeToJump = (int)getChildrenByteCodeSize ($4.ByteCodeList);
					sizeToJump++;	// 1 jump offset
					sizeToJump++;	// 1 final EOP to escape
					NODE6 ($$, CScriptVM::FUNCTION, $1, CScriptVM::JUMP, sizeToJump, $4, CScriptVM::EOP);
				}
				| TOKEN_NAME TOKEN_LP TOKEN_RP TOKEN_LA TOKEN_RA 
				{ 
					int sizeToJump = + 2;		// 1 jump instruction and EOP to escape
					NODE5 ($$, CScriptVM::FUNCTION, $1, CScriptVM::JUMP, sizeToJump, CScriptVM::EOP); 
				}

context:		TOKEN_NAME TOKEN_POINT { NODE2 ($$, CScriptVM::PUSH_GROUP, $1); }
				| TOKEN_CTXNAME TOKEN_POINT { NODE2 ($$, CScriptVM::PUSH_CTX_VAR_VAL, $1); }

function:		setFunction { NODE2 ($$, CScriptVM::PUSH_THIS, $1); }
				| context setFunction { NODE2 ($$, $1, $2); }

call:			TOKEN_NAME TOKEN_LP TOKEN_RP { NODE3 ($$, CScriptVM::PUSH_THIS, CScriptVM::CALL, $1); }
				| TOKEN_NAME TOKEN_LP { ERROR_DETECTED ($$, "missing ')' after the function name"); }
				| TOKEN_NAME TOKEN_RP { ERROR_DETECTED ($$, "missing '(' after the function name");	}
				| context TOKEN_NAME TOKEN_LP TOKEN_RP { NODE3 ($$, $1, CScriptVM::CALL, $2); }
				| context TOKEN_NAME TOKEN_LP { ERROR_DETECTED ($$, "missing ')' after the function name");	}
				| context TOKEN_NAME TOKEN_RP { ERROR_DETECTED ($$, "missinga '(' after the function name"); }


nativeFunc:		tuple TOKEN_NAME params
				{ 
					NODE2 ($$, $3, CScriptVM::PUSH_THIS);
					nativeFunc ($$, $2.Opcode, $3.Signature, $1.Signature);
					addNode ($$, $1);
					TYPE1 ($$, $1);
				}

nativeOtherFunc:	tuple context TOKEN_NAME params 
				{ 
					NODE2 ($$, $4, $2);
					nativeFunc ($$, $3.Opcode, $4.Signature, $1.Signature);
					addNode ($$, $1);
					TYPE1 ($$, $1);
				}

setFromFuncGet:	TOKEN_NAME TOKEN_LP TOKEN_RP { NODE3 ($$, CScriptVM::PUSH_THIS, CScriptVM::PUSH_STRING, $1); }
				| context TOKEN_NAME TOKEN_LP TOKEN_RP { NODE3 ($$, $1, CScriptVM::PUSH_STRING, $2); }

setFromFuncSet:	TOKEN_NAME TOKEN_LP TOKEN_RP { NODE3 ($$, CScriptVM::PUSH_THIS, CScriptVM::PUSH_STRING, $1); }
				| context TOKEN_NAME TOKEN_LP TOKEN_RP { NODE3 ($$, $1, CScriptVM::PUSH_STRING, $2); }

setFromFunction:	setFromFuncSet TOKEN_ASSIGNATOR setFromFuncGet { NODE3 ($$, $1, $3, CScriptVM::ASSIGN_FUNC_FROM); }

tupleElem:		tupleElem TOKEN_SEPARATOR writeVar { NODE2 ($$, $1, $3); TYPE2 ($$, $1, $3); }
				| writeVar { $$ = $1; }

tuple:			TOKEN_LP tupleElem TOKEN_RP { $$ = $2; }
				| TOKEN_LP TOKEN_RP { NODE0 ($$); }

lValue:			writeVar { $$ = $1; }
				| tuple { $$ = $1; }

readVar:		TOKEN_NAME 
				{ 
					NODE2 ($$, CScriptVM::PUSH_VAR_VAL, $1);
					TYPEF ($$);
				}
				| context TOKEN_NAME 
				{
					NODE3 ($$, $1, CScriptVM::PUSH_CONTEXT_VAR_VAL, $2);
					TYPEF ($$); 
				}
				| TOKEN_STRNAME 
				{ 
					NODE2 ($$, CScriptVM::PUSH_STR_VAR_VAL, $1);
					TYPES ($$);
				}
				| context TOKEN_STRNAME
				{ 
					NODE3 ($$, $1, CScriptVM::PUSH_CONTEXT_STR_VAR_VAL, $2);
					TYPES ($$);
				}
				| TOKEN_CTXNAME 
				{ 
					NODE2 ($$, CScriptVM::PUSH_CTX_VAR_VAL, $1);
					TYPEC ($$);
				}
				| context TOKEN_CTXNAME
				{ 
					NODE3 ($$, $1, CScriptVM::PUSH_CONTEXT_CTX_VAR_VAL, $2);
					TYPEC ($$);
				}
				| TOKEN_CHAIN 
				{ 
					NODE2 ($$, CScriptVM::PUSH_STRING, $1);
					TYPES ($$);
				}
				| TOKEN_NUMBER 
				{ 
					NODE2 ($$, CScriptVM::PUSH_ON_STACK, $1);
					TYPEF ($$);
				}

writeVar:		TOKEN_NAME { NODE2 ($$, CScriptVM::SET_VAR_VAL, $1); TYPEF ($$); }
				| context TOKEN_NAME { NODE3 ($$, $1, CScriptVM::SET_CONTEXT_VAR_VAL, $2); TYPEF ($$); }
				| TOKEN_STRNAME { NODE2 ($$, CScriptVM::SET_STR_VAR_VAL, $1); TYPES ($$); }
				| context TOKEN_STRNAME { NODE3 ($$, $1, CScriptVM::SET_CONTEXT_STR_VAR_VAL, $2); TYPES ($$); }
				| TOKEN_CTXNAME { NODE2 ($$, CScriptVM::SET_CTX_VAR_VAL, $1); TYPEC ($$); }
				| context TOKEN_CTXNAME { NODE3 ($$, $1, CScriptVM::SET_CONTEXT_CTX_VAR_VAL, $2); TYPEC ($$); }
				
expressions:	expressions TOKEN_SEPARATOR expression { NODE2 ($$, $1, $3); TYPE2 ($$, $1, $3); }
				| expressions expression { ERROR_DETECTED ($$, "missing ',' between two expressions"); }
				| expression { $$ = $1; }

params:			TOKEN_LP expressions TOKEN_RP { $$ = $2; }
				| TOKEN_LP expressions { ERROR_DETECTED ($$, "missing ')' at the end of the parameters"); }
				| TOKEN_LP TOKEN_RP { NODE0 ($$); }

exp:			lValue TOKEN_ASSIGNATOR expression 
				{ 
					// No need to check the types. All assignations are possibles.
					NODE2 ($$, $3, $1); 
				}
				| lValue TOKEN_ASSIGNATOR TOKEN_LP expression { ERROR_DETECTED ($$, "missing ')' at the end of the expression"); }
				| lValue TOKEN_ASSIGNATOR expression TOKEN_RP { ERROR_DETECTED ($$, "missing '(' at the beginning of the expression");}

printContent:	printContent TOKEN_SEPARATOR TOKEN_CHAIN { NODE3 ($$, $1, CScriptVM::PUSH_PRINT_STRING, $3); }
				| TOKEN_CHAIN { NODE2 ($$, CScriptVM::PUSH_PRINT_STRING, $1); }
				| printContent TOKEN_SEPARATOR TOKEN_NAME { NODE3 ($$, $1, CScriptVM::PUSH_PRINT_VAR, $3); }
				| TOKEN_NAME { NODE2 ($$, CScriptVM::PUSH_PRINT_VAR, $1); }
				| printContent TOKEN_SEPARATOR TOKEN_STRNAME { NODE3 ($$, $1, CScriptVM::PUSH_PRINT_STR_VAR, $3); }
				| TOKEN_STRNAME { NODE2 ($$, CScriptVM::PUSH_PRINT_STR_VAR, $1); }

printString:	TOKEN_PRINT TOKEN_LP printContent TOKEN_RP { NODE2 ($$, $3, CScriptVM::PRINT_STRING); }

logString:		TOKEN_LOG TOKEN_LP printContent TOKEN_RP { NODE2 ($$, $3, CScriptVM::LOG_STRING); }

statement:		openStatement { $$ = $1; }
				| closedStatement { $$ = $1; }

openStatement:	TOKEN_IF TOKEN_LP condition TOKEN_RP statement
				{
					int sizeToJump = (int)$5.ByteCode->size() + 1;	// 1 jump instruction to escape
					NODE4 ($$, $3, CScriptVM::JE, sizeToJump, $5);
				}
				| TOKEN_IF TOKEN_LP condition statement {ERROR_DETECTED ($$, "missing ')' at the end of the if condition");}
				| TOKEN_IF condition TOKEN_RP statement {ERROR_DETECTED ($$, "missing '(' at the beginning of the if condition");}
				| TOKEN_IF TOKEN_LP condition TOKEN_RP closedStatement TOKEN_ELSE openStatement 
				{ 
					int sizeToJump0 = (int)$5.ByteCode->size() + 3;	// 2 jump instructions to escape
					int sizeToJump1 = (int)$7.ByteCode->size() + 1;	// 1 jump instruction to escape
					NODE7 ($$, $3, CScriptVM::JE, sizeToJump0, $5, CScriptVM::JUMP, sizeToJump1, $7);
				}
				| TOKEN_IF TOKEN_LP condition closedStatement TOKEN_ELSE openStatement  { ERROR_DETECTED ($$, "missing ')' at the end of the if condition");}
				| TOKEN_IF condition TOKEN_RP closedStatement TOKEN_ELSE openStatement  { ERROR_DETECTED ($$, "missing '(' at the beginning of the if condition");}
				| TOKEN_WHILE TOKEN_LP condition TOKEN_RP openStatement
				{ 
					int sizeToJump0 = (int)$5.ByteCode->size() + 3;		// 2 jump instructions to escape
					int sizeToJump1 = -(int)$5.ByteCode->size() - 3 - (int)$3.ByteCode->size();	// 1 jump instruction to escape
					NODE6 ($$, $3, CScriptVM::JE, sizeToJump0, $5, CScriptVM::JUMP, sizeToJump1);
				}
				| TOKEN_WHILE TOKEN_LP condition openStatement { ERROR_DETECTED ($$, "missing ')' at the end of the while condition");}
				| TOKEN_WHILE condition TOKEN_RP openStatement { ERROR_DETECTED ($$, "missing '(' at the beginning of the while condition");}

closedStatement:TOKEN_IF TOKEN_LP condition TOKEN_RP closedStatement TOKEN_ELSE closedStatement
				{ 
					int sizeToJump0 = (int)$5.ByteCode->size() + 3;	// 2 jump instructions to escape
					int sizeToJump1 = (int)$7.ByteCode->size() + 1;	// 1 jump instruction to escape
					NODE7 ($$, $3, CScriptVM::JE, sizeToJump0, $5, CScriptVM::JUMP, sizeToJump1, $7);
				}
				| TOKEN_IF TOKEN_LP condition closedStatement TOKEN_ELSE closedStatement { ERROR_DETECTED ($$, "missing ')' at the end of the if condition");}
				| TOKEN_IF condition TOKEN_RP closedStatement TOKEN_ELSE closedStatement { ERROR_DETECTED ($$, "missing '(' at the end of the if condition");}
				| TOKEN_WHILE TOKEN_LP condition TOKEN_RP closedStatement
				{ 
					int sizeToJump0 = (int)$5.ByteCode->size() + 3;		// 2 jump instructions to escape
					int sizeToJump1 = -(int)$5.ByteCode->size() - 3 - (int)$3.ByteCode->size();	// 1 jump instruction to escape
					NODE6 ($$, $3, CScriptVM::JE, sizeToJump0, $5, CScriptVM::JUMP, sizeToJump1);
				}
				| TOKEN_WHILE TOKEN_LP condition closedStatement { ERROR_DETECTED ($$, "missing ')' at the end of the while condition");}
				| TOKEN_WHILE condition TOKEN_RP closedStatement { ERROR_DETECTED ($$, "missing '(' at the beginning of the while condition");}
				| exp TOKEN_PV { $$ = $1; }
				| printString TOKEN_PV { $$ = $1; }
				| logString TOKEN_PV { $$ = $1; }
				| function { $$ = $1; }
				| call TOKEN_PV { $$ = $1; }
				| setFromFunction TOKEN_PV { $$ = $1; }
				| nativeFunc TOKEN_PV { $$ = $1; }
				| nativeOtherFunc TOKEN_PV { $$ = $1; }
				| randEx { $$ = $1; }
				| onChildren { $$ = $1; }
				| switch { $$ = $1; }
				| TOKEN_NAME TOKEN_INCRDECR TOKEN_PV { NODE5 ($$, CScriptVM::PUSH_VAR_VAL, $1, $2, CScriptVM::SET_VAR_VAL, $1); }
				| TOKEN_INCRDECR TOKEN_NAME TOKEN_PV { NODE5 ($$, CScriptVM::PUSH_VAR_VAL, $2, $1, CScriptVM::SET_VAR_VAL, $2); }
				| context TOKEN_NAME TOKEN_INCRDECR TOKEN_PV { NODE7 ($$, $1, CScriptVM::PUSH_CONTEXT_VAR_VAL, $2, $3, $1, CScriptVM::SET_CONTEXT_VAR_VAL, $2); }
				| TOKEN_INCRDECR context TOKEN_NAME TOKEN_PV { NODE7 ($$, $2, CScriptVM::PUSH_CONTEXT_VAR_VAL, $3, $1, $2, CScriptVM::SET_CONTEXT_VAR_VAL, $3); }
				| TOKEN_NAME TOKEN_ASSIGN expression TOKEN_PV { NODE6 ($$, CScriptVM::PUSH_VAR_VAL, $1, $3, $2, CScriptVM::SET_VAR_VAL, $1); }
				| TOKEN_NAME TOKEN_ASSIGN TOKEN_LP expression TOKEN_PV  { ERROR_DETECTED ($$, "missing ')' at the end of the expression");}
				| TOKEN_NAME TOKEN_ASSIGN expression TOKEN_RP TOKEN_PV  { ERROR_DETECTED ($$, "missing '(' at the beginning of the expression");}
				| context TOKEN_NAME TOKEN_ASSIGN expression TOKEN_PV { NODE8 ($$, $1, CScriptVM::PUSH_CONTEXT_VAR_VAL, $1, $4, $3, $1, CScriptVM::SET_CONTEXT_VAR_VAL, $2); }
				| context TOKEN_NAME TOKEN_ASSIGN TOKEN_LP expression TOKEN_PV  { ERROR_DETECTED ($$, "missing ')' at the end of the expression");}
				| context TOKEN_NAME TOKEN_ASSIGN expression TOKEN_RP TOKEN_PV  { ERROR_DETECTED ($$, "missing '(' at the beginning of the expression");}
				| statementBlock { NODE1 ($$, $1); }
				| error TOKEN_PV { NODE0 ($$); }

statements:		statement { createList ($$);  addNode ($$, $1);  }
				| statements statement { $$ = $1; addNode ($$, $2); }

randEx:			TOKEN_RAND statementBlock 
				{ 
					createNode ($$);
					int childCount = $2.ByteCodeList->size ();

					// Sum all the children size
					uint sizeToJump = getChildrenByteCodeSize ($2.ByteCodeList);
					sizeToJump += 1*childCount;	// One for the JUMP
					sizeToJump += 1*childCount;	// One for the offset
					sizeToJump += 1*childCount;	// One for the RET
					
					sizeToJump += 1;	// One for the additionnal jump offset
					sizeToJump += 1;	// One for the RANDEND

					addNode ($$, CScriptVM::RAND);
					addNode ($$, childCount);
					addNode ($$, CScriptVM::JUMP);
					addNode ($$, sizeToJump);
					sizeToJump -= 2;

					// Write the jump table
					list<vector<size_t> * >::reverse_iterator rite = $2.ByteCodeList->rbegin();
					while (rite != $2.ByteCodeList->rend())
					{
						sizeToJump -= (*rite)->size ();
						
						addNode ($$, CScriptVM::JUMP);
						addNode ($$, sizeToJump);
						sizeToJump -= 1;	// One for the JUMP
						sizeToJump -= 1;	// One for the offset
						sizeToJump -= 1;	// One for the RET

						rite++;
					}	

					// Write the code
					list<vector<size_t> * >::iterator ite = $2.ByteCodeList->begin();
					while (ite != $2.ByteCodeList->end())
					{
						$$.ByteCode->insert ($$.ByteCode->end(), (*ite)->begin(), (*ite)->end());
						addNode ($$, CScriptVM::RET);
						ite++;
					}

					// End
					addNode ($$, CScriptVM::RANDEND);
				}

caseIndex:		TOKEN_CHAIN { $$ = $1; TYPES ($$); }
				| TOKEN_NUMBER { $$ = $1; TYPEF ($$); }

case:			TOKEN_CASE caseIndex TOKEN_PP statement 
				{ 
					createCase ($$, $2, $4);	// Create a two entry list to keep track of the case index and the case code
				}

cases:			case 
				{ 
					createSwitch ($$);	// Create a new case list
					addNode ($$, $1.Case);	// Add the first case
					TYPE1 ($$, $1);		// Propagate the key type
				}
				| cases case
				{ 
					$$ = $1; 
					addNode ($$, $2.Case);	// Add a case to the list
				}

switch:			TOKEN_SWITCH TOKEN_LP expression TOKEN_RP TOKEN_LA cases TOKEN_RA 
				{
					// Expression
					createNode ($$);
					addNode ($$, $3);
					
					int childCount = $6.Cases->size ();
					
					// Sum all the children size
					uint sizeChild = getChildrenByteCodeSize ($6.Cases);
					sizeChild += 1*childCount;	// One for the RET

					uint sizeToJump = 0;
					sizeToJump += 1*childCount;	// One for the case key
					sizeToJump += 1*childCount;	// One for the offset
					
					sizeToJump += 1;	// One for the additionnal jump offset

					addNode ($$, CScriptVM::SWITCH);	// Switch opcode
					addNode ($$, childCount);			// Number of switch cases
					addNode ($$, sizeToJump+sizeChild);	// Return address
					sizeToJump -= 2;

					// Write the jump table
					map<size_t, CCase *>::const_iterator ite = $6.Cases->begin ();
					while (ite != $6.Cases->end ())
					{
						const CCase *_case = ite->second;

						// Type checking
						if (_case->getType () != $3.getType ())
							aiOutputError (_case->Line, "case has not the same type than the switch expression: the case is %s and the switch is %s", 
								_case->getType (), $3.getType ());

						addNode ($$, _case->Case);
						addNode ($$, sizeToJump);
						sizeToJump += (uint)_case->ByteCode->size ();
						sizeToJump += 1;	// One for the RET
						sizeToJump -= 1;	// One for the case key
						sizeToJump -= 1;	// One for the offset
						ite++;
					}	

					// Write the code
					ite = $6.Cases->begin ();
					while (ite != $6.Cases->end ())
					{
						const CCase *_case = ite->second;
						$$.ByteCode->insert ($$.ByteCode->end(), _case->ByteCode->begin(), _case->ByteCode->end());
						addNode ($$, CScriptVM::RET);
						ite++;
					}
				}
				| TOKEN_SWITCH TOKEN_LP expression TOKEN_LA cases TOKEN_RA  {ERROR_DETECTED ($$, "missing ')' at the end of the switch expression");}
				| TOKEN_SWITCH expression TOKEN_RP TOKEN_LA cases TOKEN_RA  {ERROR_DETECTED ($$, "missing '(' at the beginning of the switch expression");}
				| TOKEN_SWITCH TOKEN_LP expression TOKEN_RP cases TOKEN_RA	{ERROR_DETECTED ($$, "missing '{' at the beginning of the switch cases");}


statementBlock:	TOKEN_LA statements TOKEN_RA { $$ = $2; }

onChildren:		TOKEN_ONCHILDREN TOKEN_LP TOKEN_RP statementBlock 
				{ 
					int sizeToJump = getChildrenByteCodeSize ($4.ByteCodeList);
					sizeToJump ++;	// One for the jump offset
					sizeToJump ++;	// One for the EOP instruction
					NODE5 ($$, CScriptVM::ONCHILDREN, CScriptVM::JUMP, sizeToJump, $4, CScriptVM::EOP);
				}


%%

int aierror (const char *s)
{
	aiOutputError (aiLine, s);
	return 1;
}

// This is just a compile fix for bison-1.875+gcc-3.2
static void dummy()
{
//	if (false)
//		YYERROR;
}
