%{

/* In order to compile this file with visual c++ 6, you have to create a unistd.h 
	file in on of your include directories with this inside:
	#include <io.h>
*/

#include "stdpch.h"
#include "script_compiler.h"

#define YY_NEVER_INTERACTIVE 1
#define PR_PRINTF(a) printf a

#include "script_parser_yacc.h"
#include "script_vm.h"

using namespace std;
using namespace NLMISC;
using namespace AICOMP;
using namespace AIVM;

extern void aiClean ();

int	aiErrorCount;
int	aiLine;
char aiFile[512] = {0};
const char *aiInputScript;
uint aiInputScriptLength;

#define YY_INPUT(buf,result,max_size) \
	result = (uint)std::min ((uint)max_size, (uint)aiInputScriptLength); \
	memcpy (buf, aiInputScript, result); \
	aiInputScript += result; \
	aiInputScriptLength -= result;

size_t makeStringId (const char *str)
{
	string strRef=str;
	TStringId strId;
	if ( strRef.at(0)=='"'
		&& strRef.at(0)==strRef.at(strRef.size()-1))
		strId=CStringMapper::map(strRef.substr(1,strRef.size()-2));
	else
		strId=CStringMapper::map(strRef);

	return (size_t)strId;
}

void addSignature (char *dest, char s)
{
	uint size = (uint)strlen (dest);
	if (size+1 < (uint)(AICOMP_MAX_SIGNATURE))
	{
		dest[size] = s;
		dest[size+1] = 0;
	}
}
			
void addSignature (char *dest, char *src)
{
	uint size0 = (uint)strlen (dest);
	uint size1 = (uint)strlen (src);
	if (size0+size1+1 < (uint)(AICOMP_MAX_SIGNATURE))
		strcat (dest, src);
}
			
#define INIT(a) ailval.Opcode.Opcode = a; ailval.Opcode.Line = aiLine; ailval.Opcode.Signature[0] = 0;
#define INIT_OP(a,b) ailval.Operator.Opcode = a; ailval.Opcode.Line = aiLine; ailval.Operator.Operator= b; ailval.Opcode.Signature[0] = 0;
		   
%}

%option nounput prefix="ai"
%option 8bit full
%pointer

alpha		[A-Za-z]
digit		[0-9]
ctxname		@({alpha}|[_])({alpha}|{digit}|[_])*
strname		$({alpha}|[_])({alpha}|{digit}|[_])*
name		({alpha}|[_])({alpha}|{digit}|[_])*
double1		[-+]?{digit}+\.([eE][-+]?{digit}+)?
double2		[-+]?{digit}+[eE][-+]?{digit}+
double3		[-+]?{digit}*\.{digit}+([eE][-+]?{digit}+)?
int			[-+]?{digit}+
double		{double1}|{double2}|{double3}|{int}
comment1	^#[^\n]*\n
comment2	\/\/[^\n]*\n
string		\"([^\n"]|(\\\"))*\"
	
%%

(\ |\t)		{ }

{comment1}	{ aiLine++; }

{comment2}	{ aiLine++; }

"\n"		{ aiLine++; }

if			{ return TOKEN_IF; }
else		{ return TOKEN_ELSE; }
while		{ return TOKEN_WHILE; }
print		{ return TOKEN_PRINT; }
log			{ return TOKEN_LOG; }
switch		{ return TOKEN_SWITCH; }
case		{ return TOKEN_CASE; }
rand		{ return TOKEN_RAND; }
onchildren	{ return TOKEN_ONCHILDREN; }
"."			{ return TOKEN_POINT; }
","			{ return TOKEN_SEPARATOR; }
"="			{ return TOKEN_ASSIGNATOR; }
";"			{ return TOKEN_PV; }
":"			{ return TOKEN_PP; }
"("			{ return TOKEN_LP; }
")"			{ return TOKEN_RP; }
"{"			{ return TOKEN_LA; }
"}"			{ return TOKEN_RA; }


{double}	{				
				const float f=(float)atof(yytext);
				INIT(*((size_t*)&f));
				return TOKEN_NUMBER;
			}
{string}	{ INIT(makeStringId (yytext)); return TOKEN_CHAIN; }
{name}		{ INIT(makeStringId (yytext)); return TOKEN_NAME; }
{ctxname}	{ INIT(makeStringId (yytext)); return TOKEN_CTXNAME; }
{strname}	{ INIT(makeStringId (yytext)); return TOKEN_STRNAME; }

"&&"		{ INIT(CScriptVM::AND); return TOKEN_LOGIC; }
"||"		{ INIT(CScriptVM::OR); return TOKEN_LOGIC; }

"=="		{ INIT_OP(CScriptVM::EQ, "=="); return TOKEN_COMP; }
">="		{ INIT_OP(CScriptVM::SUPEQ, ">="); return TOKEN_COMP; }
"=>"		{ INIT_OP(CScriptVM::SUPEQ, "=>"); return TOKEN_COMP; }
"<="		{ INIT_OP(CScriptVM::INFEQ, "<="); return TOKEN_COMP; }
"=<"		{ INIT_OP(CScriptVM::INFEQ, "=<"); return TOKEN_COMP; }
"!="		{ INIT_OP(CScriptVM::NEQ, "!="); return TOKEN_COMP; }
"<>"		{ INIT_OP(CScriptVM::NEQ, "<>"); return TOKEN_COMP; }
"><"		{ INIT_OP(CScriptVM::NEQ, "><"); return TOKEN_COMP; }
">"			{ INIT_OP(CScriptVM::SUP, ">"	); return TOKEN_COMP; }
"<"			{ INIT_OP(CScriptVM::INF, "<"	); return TOKEN_COMP; }

"+="		{ INIT_OP(CScriptVM::ADD, "+="); return TOKEN_ASSIGN; }
"-="		{ INIT_OP(CScriptVM::SUB, "-="); return TOKEN_ASSIGN; }
"*="		{ INIT_OP(CScriptVM::MUL, "*="); return TOKEN_ASSIGN; }
"/="		{ INIT_OP(CScriptVM::DIV, "/="); return TOKEN_ASSIGN; }

"++"		{ INIT(CScriptVM::INCR); return TOKEN_INCRDECR; }
"--"		{ INIT(CScriptVM::DECR); return TOKEN_INCRDECR; }

"+"		{ INIT(CScriptVM::ADD); return TOKEN_ADD; }
"-"		{ INIT(CScriptVM::SUB); return TOKEN_SUB; }
"*"		{ INIT_OP(CScriptVM::MUL, "*"); return TOKEN_FACTOR; }
"/"		{ INIT_OP(CScriptVM::DIV, "/"); return TOKEN_FACTOR; }

%%

int aiwrap()
{
	// ai_delete_buffer (yy_current_buffer);
	return 1;
}
