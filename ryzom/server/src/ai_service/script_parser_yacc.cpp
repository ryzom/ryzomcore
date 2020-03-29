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

/* A Bison parser, made by GNU Bison 1.875.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Written by Richard Stallman by simplifying the original so called
   ``semantic'' parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0

/* If NAME_PREFIX is specified substitute the variables and functions
   names.  */
#define yyparse aiparse
#define yylex   ailex
#define yyerror aierror
#define yylval  ailval
#define yychar  aichar
#define yydebug aidebug
#define yynerrs ainerrs


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     TOKEN_IF = 258,
     TOKEN_ELSE = 259,
     TOKEN_WHILE = 260,
     TOKEN_PRINT = 261,
     TOKEN_LOG = 262,
     TOKEN_CASE = 263,
     TOKEN_POINT = 264,
     TOKEN_SEPARATOR = 265,
     TOKEN_PV = 266,
     TOKEN_PP = 267,
     TOKEN_LP = 268,
     TOKEN_LA = 269,
     TOKEN_RP = 270,
     TOKEN_RA = 271,
     TOKEN_ASSIGNATOR = 272,
     TOKEN_SWITCH = 273,
     TOKEN_RAND = 274,
     TOKEN_NUMBER = 275,
     TOKEN_ONCHILDREN = 276,
     TOKEN_CHAIN = 277,
     TOKEN_NAME = 278,
     TOKEN_STRNAME = 279,
     TOKEN_CTXNAME = 280,
     TOKEN_LOGIC = 281,
     TOKEN_INCRDECR = 282,
     TOKEN_ADD = 283,
     TOKEN_SUB = 284,
     TOKEN_COMP = 285,
     TOKEN_ASSIGN = 286,
     TOKEN_FACTOR = 287
   };
#endif
#define TOKEN_IF 258
#define TOKEN_ELSE 259
#define TOKEN_WHILE 260
#define TOKEN_PRINT 261
#define TOKEN_LOG 262
#define TOKEN_CASE 263
#define TOKEN_POINT 264
#define TOKEN_SEPARATOR 265
#define TOKEN_PV 266
#define TOKEN_PP 267
#define TOKEN_LP 268
#define TOKEN_LA 269
#define TOKEN_RP 270
#define TOKEN_RA 271
#define TOKEN_ASSIGNATOR 272
#define TOKEN_SWITCH 273
#define TOKEN_RAND 274
#define TOKEN_NUMBER 275
#define TOKEN_ONCHILDREN 276
#define TOKEN_CHAIN 277
#define TOKEN_NAME 278
#define TOKEN_STRNAME 279
#define TOKEN_CTXNAME 280
#define TOKEN_LOGIC 281
#define TOKEN_INCRDECR 282
#define TOKEN_ADD 283
#define TOKEN_SUB 284
#define TOKEN_COMP 285
#define TOKEN_ASSIGN 286
#define TOKEN_FACTOR 287




/* Copy the first part of user declarations.  */
#line 1 "ai_service/script_parser.yacc"


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
	aiInputScriptLength = strlen (script);
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
		MessageBoxA (NULL, aiErrorMessage.c_str (), "AI Script Compiler", MB_OK|(error?MB_ICONEXCLAMATION:MB_ICONINFORMATION));
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
		size += (*ite)->size ();
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
		size += ite->second->ByteCode->size();
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



/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 295 "ai_service/script_parser.yacc"
typedef union YYSTYPE {
	AICOMP::COpcodeYacc			Opcode;
	AICOMP::COperatorYacc		Operator;
	AICOMP::CByteCodeYacc		ByteCode;
	AICOMP::CByteCodeListYacc	ByteCodeList;
	AICOMP::CCaseYacc			Case;
	AICOMP::CSwitchYacc			Switch;
	struct {} 					Nothing;
} YYSTYPE;
/* Line 191 of yacc.c.  */
#line 450 "ai_service/script_parser_yacc.cpp"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 214 of yacc.c.  */
#line 462 "ai_service/script_parser_yacc.cpp"

#if ! defined (yyoverflow) || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# if YYSTACK_USE_ALLOCA
#  define YYSTACK_ALLOC alloca
# else
#  ifndef YYSTACK_USE_ALLOCA
#   if defined (alloca) || defined (_ALLOCA_H)
#    define YYSTACK_ALLOC alloca
#   else
#    ifdef __GNUC__
#     define YYSTACK_ALLOC __builtin_alloca
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
# else
#  if defined (__STDC__) || defined (__cplusplus)
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   define YYSIZE_T size_t
#  endif
#  define YYSTACK_ALLOC malloc
#  define YYSTACK_FREE free
# endif
#endif /* ! defined (yyoverflow) || YYERROR_VERBOSE */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE))				\
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  register YYSIZE_T yyi;		\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (0)
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif

#if defined (__STDC__) || defined (__cplusplus)
   typedef signed char yysigned_char;
#else
   typedef short yysigned_char;
#endif

/* YYFINAL -- State number of the termination state. */
#define YYFINAL  72
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   425

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  33
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  35
/* YYNRULES -- Number of rules. */
#define YYNRULES  120
/* YYNRULES -- Number of states. */
#define YYNSTATES  227

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   287

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short yyprhs[] =
{
       0,     0,     3,     5,     9,    13,    17,    21,    25,    29,
      33,    35,    40,    46,    49,    52,    54,    57,    61,    64,
      67,    72,    76,    80,    84,    89,    93,    98,   102,   107,
     111,   115,   117,   121,   124,   126,   128,   130,   133,   135,
     138,   140,   143,   145,   147,   149,   152,   154,   157,   159,
     162,   166,   169,   171,   175,   178,   181,   185,   190,   195,
     199,   201,   205,   207,   211,   213,   218,   223,   225,   227,
     233,   238,   243,   251,   258,   265,   271,   276,   281,   289,
     296,   303,   309,   314,   319,   322,   325,   328,   330,   333,
     336,   339,   342,   344,   346,   348,   352,   356,   361,   366,
     371,   377,   383,   389,   396,   403,   405,   408,   410,   413,
     416,   418,   420,   425,   427,   430,   438,   445,   452,   459,
     463
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const yysigned_char yyrhs[] =
{
      34,     0,    -1,    60,    -1,    35,    26,    35,    -1,    13,
      35,    15,    -1,    36,    30,    36,    -1,    36,    28,    36,
      -1,    36,    29,    36,    -1,    36,    32,    36,    -1,    13,
      36,    15,    -1,    49,    -1,    23,    13,    15,    66,    -1,
      23,    13,    15,    14,    16,    -1,    23,     9,    -1,    25,
       9,    -1,    37,    -1,    38,    37,    -1,    23,    13,    15,
      -1,    23,    13,    -1,    23,    15,    -1,    38,    23,    13,
      15,    -1,    38,    23,    13,    -1,    38,    23,    15,    -1,
      47,    23,    52,    -1,    47,    38,    23,    52,    -1,    23,
      13,    15,    -1,    38,    23,    13,    15,    -1,    23,    13,
      15,    -1,    38,    23,    13,    15,    -1,    44,    17,    43,
      -1,    46,    10,    50,    -1,    50,    -1,    13,    46,    15,
      -1,    13,    15,    -1,    50,    -1,    47,    -1,    23,    -1,
      38,    23,    -1,    24,    -1,    38,    24,    -1,    25,    -1,
      38,    25,    -1,    22,    -1,    20,    -1,    23,    -1,    38,
      23,    -1,    24,    -1,    38,    24,    -1,    25,    -1,    38,
      25,    -1,    51,    10,    36,    -1,    51,    36,    -1,    36,
      -1,    13,    51,    15,    -1,    13,    51,    -1,    13,    15,
      -1,    48,    17,    36,    -1,    48,    17,    13,    36,    -1,
      48,    17,    36,    15,    -1,    54,    10,    22,    -1,    22,
      -1,    54,    10,    23,    -1,    23,    -1,    54,    10,    24,
      -1,    24,    -1,     6,    13,    54,    15,    -1,     7,    13,
      54,    15,    -1,    58,    -1,    59,    -1,     3,    13,    35,
      15,    57,    -1,     3,    13,    35,    57,    -1,     3,    35,
      15,    57,    -1,     3,    13,    35,    15,    59,     4,    58,
      -1,     3,    13,    35,    59,     4,    58,    -1,     3,    35,
      15,    59,     4,    58,    -1,     5,    13,    35,    15,    58,
      -1,     5,    13,    35,    58,    -1,     5,    35,    15,    58,
      -1,     3,    13,    35,    15,    59,     4,    59,    -1,     3,
      13,    35,    59,     4,    59,    -1,     3,    35,    15,    59,
       4,    59,    -1,     5,    13,    35,    15,    59,    -1,     5,
      13,    35,    59,    -1,     5,    35,    15,    59,    -1,    53,
      11,    -1,    55,    11,    -1,    56,    11,    -1,    39,    -1,
      40,    11,    -1,    45,    11,    -1,    41,    11,    -1,    42,
      11,    -1,    61,    -1,    67,    -1,    65,    -1,    23,    27,
      11,    -1,    27,    23,    11,    -1,    38,    23,    27,    11,
      -1,    27,    38,    23,    11,    -1,    23,    31,    36,    11,
      -1,    23,    31,    13,    36,    11,    -1,    23,    31,    36,
      15,    11,    -1,    38,    23,    31,    36,    11,    -1,    38,
      23,    31,    13,    36,    11,    -1,    38,    23,    31,    36,
      15,    11,    -1,    66,    -1,     1,    11,    -1,    57,    -1,
      60,    57,    -1,    19,    66,    -1,    22,    -1,    20,    -1,
       8,    62,    12,    57,    -1,    63,    -1,    64,    63,    -1,
      18,    13,    36,    15,    14,    64,    16,    -1,    18,    13,
      36,    14,    64,    16,    -1,    18,    36,    15,    14,    64,
      16,    -1,    18,    13,    36,    15,    64,    16,    -1,    14,
      60,    16,    -1,    21,    13,    15,    66,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   360,   360,   362,   363,   364,   371,   375,   381,   387,
     388,   390,   398,   404,   405,   407,   409,   411,   412,   413,
     415,   417,   419,   422,   437,   445,   447,   449,   451,   453,
     455,   456,   458,   459,   461,   462,   464,   474,   479,   489,
     494,   504,   509,   514,   520,   522,   523,   525,   526,   528,
     530,   531,   532,   534,   535,   536,   538,   543,   544,   546,
     547,   548,   549,   550,   551,   553,   555,   557,   558,   560,
     565,   566,   567,   573,   574,   575,   581,   582,   584,   590,
     591,   592,   598,   599,   600,   601,   602,   603,   604,   605,
     606,   607,   608,   609,   610,   611,   612,   614,   616,   617,
     618,   619,   621,   623,   625,   626,   627,   629,   630,   632,
     680,   681,   683,   688,   694,   700,   753,   754,   755,   758,
     760
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "TOKEN_IF", "TOKEN_ELSE", "TOKEN_WHILE", 
  "TOKEN_PRINT", "TOKEN_LOG", "TOKEN_CASE", "TOKEN_POINT", 
  "TOKEN_SEPARATOR", "TOKEN_PV", "TOKEN_PP", "TOKEN_LP", "TOKEN_LA", 
  "TOKEN_RP", "TOKEN_RA", "TOKEN_ASSIGNATOR", "TOKEN_SWITCH", 
  "TOKEN_RAND", "TOKEN_NUMBER", "TOKEN_ONCHILDREN", "TOKEN_CHAIN", 
  "TOKEN_NAME", "TOKEN_STRNAME", "TOKEN_CTXNAME", "TOKEN_LOGIC", 
  "TOKEN_INCRDECR", "TOKEN_ADD", "TOKEN_SUB", "TOKEN_COMP", 
  "TOKEN_ASSIGN", "TOKEN_FACTOR", "$accept", "script", "condition", 
  "expression", "setFunction", "context", "function", "call", 
  "nativeFunc", "nativeOtherFunc", "setFromFuncGet", "setFromFuncSet", 
  "setFromFunction", "tupleElem", "tuple", "lValue", "readVar", 
  "writeVar", "expressions", "params", "exp", "printContent", 
  "printString", "logString", "statement", "openStatement", 
  "closedStatement", "statements", "randEx", "caseIndex", "case", "cases", 
  "switch", "statementBlock", "onChildren", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    33,    34,    35,    35,    35,    36,    36,    36,    36,
      36,    37,    37,    38,    38,    39,    39,    40,    40,    40,
      40,    40,    40,    41,    42,    43,    43,    44,    44,    45,
      46,    46,    47,    47,    48,    48,    49,    49,    49,    49,
      49,    49,    49,    49,    50,    50,    50,    50,    50,    50,
      51,    51,    51,    52,    52,    52,    53,    53,    53,    54,
      54,    54,    54,    54,    54,    55,    56,    57,    57,    58,
      58,    58,    58,    58,    58,    58,    58,    58,    59,    59,
      59,    59,    59,    59,    59,    59,    59,    59,    59,    59,
      59,    59,    59,    59,    59,    59,    59,    59,    59,    59,
      59,    59,    59,    59,    59,    59,    59,    60,    60,    61,
      62,    62,    63,    64,    64,    65,    65,    65,    65,    66,
      67
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     3,     3,     3,     3,     3,     3,     3,
       1,     4,     5,     2,     2,     1,     2,     3,     2,     2,
       4,     3,     3,     3,     4,     3,     4,     3,     4,     3,
       3,     1,     3,     2,     1,     1,     1,     2,     1,     2,
       1,     2,     1,     1,     1,     2,     1,     2,     1,     2,
       3,     2,     1,     3,     2,     2,     3,     4,     4,     3,
       1,     3,     1,     3,     1,     4,     4,     1,     1,     5,
       4,     4,     7,     6,     6,     5,     4,     4,     7,     6,
       6,     5,     4,     4,     2,     2,     2,     1,     2,     2,
       2,     2,     1,     1,     1,     3,     3,     4,     4,     4,
       5,     5,     5,     6,     6,     1,     2,     1,     2,     2,
       1,     1,     4,     1,     2,     7,     6,     6,     6,     3,
       4
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    44,    46,    48,     0,     0,    15,     0,    87,     0,
       0,     0,     0,     0,    35,     0,    34,     0,     0,     0,
     107,    67,    68,     0,    92,    94,   105,    93,   106,     0,
      43,    42,    36,    38,    40,     0,     0,     0,    10,     0,
       0,     0,     0,    33,    44,     0,     0,    31,     0,     0,
       0,   109,     0,    13,    18,    19,     0,     0,    14,     0,
       0,     0,     1,    45,    47,    49,    16,    88,    90,    91,
       0,    89,     0,     0,     0,    84,    85,    86,   108,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    37,    39,
      41,     0,     0,    60,    62,    64,     0,     0,    45,     0,
      32,   119,     0,     0,     0,     0,    17,    95,     0,     0,
      96,     0,    21,    22,     0,     0,     0,     0,    29,     0,
      23,     0,     0,    56,     0,     0,    70,    68,     9,    71,
      68,     3,     6,     7,     5,     8,     0,    76,    82,    77,
      83,     0,    65,    66,    30,     0,     0,     9,     0,   120,
       0,    11,     0,    99,     0,    98,    20,    97,     0,     0,
       0,     0,    55,    52,    54,    24,    57,    58,     4,    69,
      68,     0,     0,    75,    81,    59,    61,    63,     0,   113,
       0,     0,     0,     0,    12,   100,   101,     0,   102,     0,
      25,     0,     0,    53,    51,     0,    73,    79,    74,    80,
     111,   110,     0,   116,   114,     0,   118,   117,   103,   104,
      26,    50,    72,    78,     0,   115,   112
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    15,    45,    46,    16,    47,    18,    19,    20,    21,
     128,    22,    23,    56,    24,    25,    48,    26,   174,   130,
      27,   106,    28,    29,    30,    31,    32,    58,    34,   212,
     189,   190,    35,    36,    37
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -176
static const short yypact[] =
{
     311,     5,   346,   352,    32,    54,    60,   311,   360,    18,
      61,    37,  -176,    10,   141,    77,  -176,   246,  -176,    82,
      83,    92,   102,   111,   184,   130,  -176,   137,   163,   173,
    -176,  -176,  -176,   138,  -176,  -176,  -176,  -176,  -176,   366,
    -176,  -176,   179,  -176,    10,   -13,   187,   271,  -176,   366,
      -4,   297,   297,  -176,   179,   325,    19,  -176,   261,   374,
      50,  -176,   177,  -176,   185,  -176,   190,   380,  -176,   227,
      10,   188,  -176,     8,  -176,  -176,  -176,  -176,  -176,  -176,
     240,  -176,    91,   195,   388,  -176,  -176,  -176,  -176,   366,
     172,    96,   311,   366,   374,   374,   374,   374,  -176,  -176,
    -176,   207,   311,  -176,  -176,  -176,   108,   127,  -176,   397,
    -176,  -176,   374,   140,   209,    18,   166,  -176,   374,    15,
    -176,   216,   229,  -176,   239,   394,   158,   222,  -176,   338,
    -176,   243,   374,   174,    16,   234,  -176,   268,  -176,  -176,
     269,  -176,   249,   249,   101,  -176,   234,  -176,  -176,  -176,
    -176,   401,  -176,  -176,  -176,   214,   275,    40,   275,  -176,
     286,  -176,    44,  -176,   279,  -176,   237,  -176,   374,   121,
     282,   285,  -176,   101,   332,  -176,   214,  -176,  -176,  -176,
     299,   311,   311,  -176,  -176,  -176,  -176,  -176,   256,  -176,
      41,   275,    55,    79,  -176,  -176,  -176,   312,  -176,   290,
    -176,   291,   374,  -176,   101,   311,  -176,  -176,  -176,  -176,
    -176,  -176,   296,  -176,  -176,   105,  -176,  -176,  -176,  -176,
    -176,   101,  -176,  -176,   311,  -176,  -176
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -176,  -176,    17,     2,   298,     0,  -176,  -176,  -176,  -176,
    -176,  -176,  -176,  -176,  -176,  -176,  -176,     6,  -176,   191,
    -176,   274,  -176,  -176,   -30,   -93,   -65,   328,  -176,  -176,
    -175,  -153,  -176,    -8,  -176
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -29
static const short yytable[] =
{
      17,    61,    92,    88,   192,   193,    55,    17,   147,   149,
      60,   102,    57,    93,    71,   214,    38,   214,   214,    68,
      50,   122,    93,   123,    83,   137,   163,   140,    88,   109,
     164,   178,     7,    17,   110,   124,   148,   150,   215,   125,
     214,    91,    93,    94,    95,    51,    63,    97,   188,   188,
      64,    91,    65,   183,   191,   195,    90,   213,    17,   138,
     136,   113,   139,   188,    66,   114,   101,    52,    67,   119,
     180,   216,    94,    95,    62,    53,    97,    72,    94,    95,
     127,   184,    97,    54,    12,    13,   133,   188,   206,   208,
      17,    91,    17,    77,    78,   217,   142,   143,   144,   145,
      63,    17,    17,    79,   129,   179,   134,   159,   161,    55,
     141,   138,   222,   188,   155,   154,   207,   209,   151,    80,
     162,   225,    81,   152,    94,    95,    96,   169,    97,    94,
      95,   173,   198,    97,   176,    17,   199,   151,    -2,     1,
     223,     2,   153,     3,     4,     5,    17,    84,    85,    94,
      95,     6,     7,    97,   156,   157,     8,     9,   161,    10,
      17,    11,    12,    13,    69,    14,    70,    63,    94,    95,
     197,   170,    97,     1,    86,     2,   204,     3,     4,     5,
     160,    17,    17,   -27,    87,     6,     7,   135,    63,   177,
       8,     9,   115,    10,   226,    11,    12,    13,    93,    14,
     116,   117,    94,    95,   221,    17,    97,    82,     1,    70,
       2,   121,     3,     4,     5,    94,    95,    96,   131,    97,
       6,     7,   146,   158,    17,     8,     9,   165,    10,   138,
      11,    12,    13,    93,    14,     1,    63,     2,   120,     3,
       4,     5,    94,    95,   166,   171,    97,     6,     7,    -4,
     167,   160,     8,     9,   -28,    10,   129,    11,    12,    13,
      -4,    14,     1,   126,     2,    70,     3,     4,     5,    73,
      74,    75,   181,   182,     6,     7,   210,   111,   211,     8,
       9,    97,    10,   188,    11,    12,    13,     1,    14,     2,
     196,     3,     4,     5,    98,    99,   100,   200,   201,     6,
       7,   219,   194,   205,     8,     9,   220,    10,   224,    11,
      12,    13,     1,    14,     2,    76,     3,     4,     5,   103,
     104,   105,   175,   218,     6,     7,   107,   138,    33,     8,
       9,     0,    10,     0,    11,    12,    13,     0,    14,     0,
      94,    95,   202,     0,    97,   112,     0,   203,   108,    74,
      75,   112,    40,   172,    41,    42,    43,    44,    40,    39,
      41,    42,    43,    44,     0,    49,    40,     0,    41,    42,
      43,    44,    40,    59,    41,    42,    43,    44,     0,    89,
      40,     0,    41,    42,    43,    44,    40,   112,    41,    42,
      43,    44,     0,   118,    40,     0,    41,    42,    43,    44,
      40,   132,    41,    42,    43,    44,     0,   168,    40,     0,
      41,    42,    43,    44,    40,     0,    41,    42,    43,    44,
      54,    12,    13,   185,   186,   187
};

static const short yycheck[] =
{
       0,     9,    15,    33,   157,   158,     6,     7,   101,   102,
       8,    15,     6,    26,    14,   190,    11,   192,   193,     9,
       3,    13,    26,    15,    24,    90,    11,    92,    58,    10,
      15,    15,    14,    33,    15,    27,   101,   102,   191,    31,
     215,    39,    26,    28,    29,    13,     9,    32,     8,     8,
      13,    49,    15,   146,    14,    11,    39,    16,    58,    15,
      90,    59,    92,     8,    27,    15,    49,    13,    31,    67,
     135,    16,    28,    29,    13,    15,    32,     0,    28,    29,
      80,   146,    32,    23,    24,    25,    84,     8,   181,   182,
      90,    89,    92,    11,    11,    16,    94,    95,    96,    97,
       9,   101,   102,    11,    13,   135,    89,   115,   116,   109,
      93,    15,   205,     8,   112,   109,   181,   182,    10,    17,
     118,    16,    11,    15,    28,    29,    30,   125,    32,    28,
      29,   129,    11,    32,   132,   135,    15,    10,     0,     1,
     205,     3,    15,     5,     6,     7,   146,    17,    11,    28,
      29,    13,    14,    32,    14,    15,    18,    19,   166,    21,
     160,    23,    24,    25,    23,    27,    25,     9,    28,    29,
     168,    13,    32,     1,    11,     3,   174,     5,     6,     7,
      14,   181,   182,    17,    11,    13,    14,    15,     9,    15,
      18,    19,    15,    21,   224,    23,    24,    25,    26,    27,
      15,    11,    28,    29,   202,   205,    32,    23,     1,    25,
       3,    23,     5,     6,     7,    28,    29,    30,    23,    32,
      13,    14,    15,    14,   224,    18,    19,    11,    21,    15,
      23,    24,    25,    26,    27,     1,     9,     3,    11,     5,
       6,     7,    28,    29,    15,    23,    32,    13,    14,    15,
      11,    14,    18,    19,    17,    21,    13,    23,    24,    25,
      26,    27,     1,    23,     3,    25,     5,     6,     7,    23,
      24,    25,     4,     4,    13,    14,    20,    16,    22,    18,
      19,    32,    21,     8,    23,    24,    25,     1,    27,     3,
      11,     5,     6,     7,    23,    24,    25,    15,    13,    13,
      14,    11,    16,     4,    18,    19,    15,    21,    12,    23,
      24,    25,     1,    27,     3,    17,     5,     6,     7,    22,
      23,    24,   131,    11,    13,    14,    52,    15,     0,    18,
      19,    -1,    21,    -1,    23,    24,    25,    -1,    27,    -1,
      28,    29,    10,    -1,    32,    13,    -1,    15,    23,    24,
      25,    13,    20,    15,    22,    23,    24,    25,    20,    13,
      22,    23,    24,    25,    -1,    13,    20,    -1,    22,    23,
      24,    25,    20,    13,    22,    23,    24,    25,    -1,    13,
      20,    -1,    22,    23,    24,    25,    20,    13,    22,    23,
      24,    25,    -1,    13,    20,    -1,    22,    23,    24,    25,
      20,    13,    22,    23,    24,    25,    -1,    13,    20,    -1,
      22,    23,    24,    25,    20,    -1,    22,    23,    24,    25,
      23,    24,    25,    22,    23,    24
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,     1,     3,     5,     6,     7,    13,    14,    18,    19,
      21,    23,    24,    25,    27,    34,    37,    38,    39,    40,
      41,    42,    44,    45,    47,    48,    50,    53,    55,    56,
      57,    58,    59,    60,    61,    65,    66,    67,    11,    13,
      20,    22,    23,    24,    25,    35,    36,    38,    49,    13,
      35,    13,    13,    15,    23,    38,    46,    50,    60,    13,
      36,    66,    13,     9,    13,    15,    27,    31,     9,    23,
      25,    38,     0,    23,    24,    25,    37,    11,    11,    11,
      17,    11,    23,    38,    17,    11,    11,    11,    57,    13,
      35,    36,    15,    26,    28,    29,    30,    32,    23,    24,
      25,    35,    15,    22,    23,    24,    54,    54,    23,    10,
      15,    16,    13,    36,    15,    15,    15,    11,    13,    36,
      11,    23,    13,    15,    27,    31,    23,    38,    43,    13,
      52,    23,    13,    36,    35,    15,    57,    59,    15,    57,
      59,    35,    36,    36,    36,    36,    15,    58,    59,    58,
      59,    10,    15,    15,    50,    36,    14,    15,    14,    66,
      14,    66,    36,    11,    15,    11,    15,    11,    13,    36,
      13,    23,    15,    36,    51,    52,    36,    15,    15,    57,
      59,     4,     4,    58,    59,    22,    23,    24,     8,    63,
      64,    14,    64,    64,    16,    11,    11,    36,    11,    15,
      15,    13,    10,    15,    36,     4,    58,    59,    58,    59,
      20,    22,    62,    16,    63,    64,    16,    16,    11,    11,
      15,    36,    58,    59,    12,    16,    57
};

#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# if defined (__STDC__) || defined (__cplusplus)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# endif
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrlab1

/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { 								\
      yyerror ("syntax error: cannot back up");\
      YYERROR;							\
    }								\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

/* YYLLOC_DEFAULT -- Compute the default location (before the actions
   are run).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)         \
  Current.first_line   = Rhs[1].first_line;      \
  Current.first_column = Rhs[1].first_column;    \
  Current.last_line    = Rhs[N].last_line;       \
  Current.last_column  = Rhs[N].last_column;
#endif

/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (0)

# define YYDSYMPRINT(Args)			\
do {						\
  if (yydebug)					\
    yysymprint Args;				\
} while (0)

# define YYDSYMPRINTF(Title, Token, Value, Location)		\
do {								\
  if (yydebug)							\
    {								\
      YYFPRINTF (stderr, "%s ", Title);				\
      yysymprint (stderr, 					\
                  Token, Value);	\
      YYFPRINTF (stderr, "\n");					\
    }								\
} while (0)

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (cinluded).                                                   |
`------------------------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_stack_print (short *bottom, short *top)
#else
static void
yy_stack_print (bottom, top)
    short *bottom;
    short *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (/* Nothing. */; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_reduce_print (int yyrule)
#else
static void
yy_reduce_print (yyrule)
    int yyrule;
#endif
{
  int yyi;
  unsigned int yylineno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %u), ",
             yyrule - 1, yylineno);
  /* Print the symbols being reduced, and their result.  */
  for (yyi = yyprhs[yyrule]; 0 <= yyrhs[yyi]; yyi++)
    YYFPRINTF (stderr, "%s ", yytname [yyrhs[yyi]]);
  YYFPRINTF (stderr, "-> %s\n", yytname [yyr1[yyrule]]);
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (Rule);		\
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YYDSYMPRINT(Args)
# define YYDSYMPRINTF(Title, Token, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   SIZE_MAX < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#if YYMAXDEPTH == 0
# undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  register const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  register char *yyd = yydest;
  register const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

#endif /* !YYERROR_VERBOSE */



#if YYDEBUG
/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yysymprint (FILE *yyoutput, int yytype, YYSTYPE *yyvaluep)
#else
static void
yysymprint (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  if (yytype < YYNTOKENS)
    {
      YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
# ifdef YYPRINT
      YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
    }
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  switch (yytype)
    {
      default:
        break;
    }
  YYFPRINTF (yyoutput, ")");
}

#endif /* ! YYDEBUG */
/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yydestruct (int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yytype, yyvaluep)
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  switch (yytype)
    {

      default:
        break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM);
# else
int yyparse ();
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM)
# else
int yyparse (YYPARSE_PARAM)
  void *YYPARSE_PARAM;
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  short	yyssa[YYINITDEPTH];
  short *yyss = yyssa;
  register short *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;



#define YYPOPSTACK   (yyvsp--, yyssp--)

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* When reducing, the number of symbols on the RHS of the reduced
     rule.  */
  int yylen;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyoverflowlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	short *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyoverflowlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YYDSYMPRINTF ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */
  YYDPRINTF ((stderr, "Shifting token %s, ", yytname[yytoken]));

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;


  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 360 "ai_service/script_parser.yacc"
    { NODE1 (yyval.ByteCode, yyvsp[0].ByteCodeList); aiRoot = yyval.ByteCode.ByteCode; ;}
    break;

  case 3:
#line 362 "ai_service/script_parser.yacc"
    { NODE3 (yyval.ByteCode, yyvsp[-2].ByteCode, yyvsp[0].ByteCode, yyvsp[-1].Opcode); TYPEL (yyval.ByteCode); ;}
    break;

  case 4:
#line 363 "ai_service/script_parser.yacc"
    { yyval.ByteCode = yyvsp[-1].ByteCode; ;}
    break;

  case 5:
#line 365 "ai_service/script_parser.yacc"
    { 
					if (yyvsp[-2].ByteCode.getType () != yyvsp[0].ByteCode.getType ()) aiOutputError (aiLine, "the left and right '%s' expressions have not the same type : left is a %s and right is a %s",
						yyvsp[-1].Operator.Operator, yyvsp[-2].ByteCode.getType (), yyvsp[0].ByteCode.getType ());
					NODE3 (yyval.ByteCode, yyvsp[-2].ByteCode, yyvsp[0].ByteCode, yyvsp[-1].Operator); TYPEL (yyval.ByteCode); 
				;}
    break;

  case 6:
#line 372 "ai_service/script_parser.yacc"
    { 
					NODE3 (yyval.ByteCode, yyvsp[-2].ByteCode, yyvsp[0].ByteCode, yyvsp[-1].Opcode); TYPEF (yyval.ByteCode); 
				;}
    break;

  case 7:
#line 376 "ai_service/script_parser.yacc"
    { 
					if (!yyvsp[-2].ByteCode.isFloat ()) aiOutputError (aiLine, "the left '-' expression must be a float but it is a %s", yyvsp[-2].ByteCode.getType ());
					if (!yyvsp[0].ByteCode.isFloat ()) aiOutputError (aiLine, "the right '-' expression must be a float but it is a %s", yyvsp[0].ByteCode.getType ());
					NODE3 (yyval.ByteCode, yyvsp[-2].ByteCode, yyvsp[0].ByteCode, yyvsp[-1].Opcode); TYPEF (yyval.ByteCode); 
				;}
    break;

  case 8:
#line 382 "ai_service/script_parser.yacc"
    { 
					if (!yyvsp[-2].ByteCode.isFloat ()) aiOutputError (aiLine, "the left '%s' expression must be a float but it is a %s", yyvsp[-1].Operator.Operator, yyvsp[-2].ByteCode.getType ());
					if (!yyvsp[0].ByteCode.isFloat ()) aiOutputError (aiLine, "the right '%s' expression must be a float but it is a %s", yyvsp[-1].Operator.Operator, yyvsp[0].ByteCode.getType ());
					NODE3 (yyval.ByteCode, yyvsp[-2].ByteCode, yyvsp[0].ByteCode, yyvsp[-1].Operator); TYPEF (yyval.ByteCode); 
				;}
    break;

  case 9:
#line 387 "ai_service/script_parser.yacc"
    { yyval.ByteCode = yyvsp[-1].ByteCode; ;}
    break;

  case 10:
#line 388 "ai_service/script_parser.yacc"
    { yyval.ByteCode = yyvsp[0].ByteCode; ;}
    break;

  case 11:
#line 391 "ai_service/script_parser.yacc"
    { 
					// Get the size of the total byte code of statementBlock
					int sizeToJump = (int)getChildrenByteCodeSize (yyvsp[0].ByteCodeList.ByteCodeList);
					sizeToJump++;	// 1 jump offset
					sizeToJump++;	// 1 final EOP to escape
					NODE6 (yyval.ByteCode, CScriptVM::FUNCTION, yyvsp[-3].Opcode, CScriptVM::JUMP, sizeToJump, yyvsp[0].ByteCodeList, CScriptVM::EOP);
				;}
    break;

  case 12:
#line 399 "ai_service/script_parser.yacc"
    { 
					int sizeToJump = + 2;		// 1 jump instruction and EOP to escape
					NODE5 (yyval.ByteCode, CScriptVM::FUNCTION, yyvsp[-4].Opcode, CScriptVM::JUMP, sizeToJump, CScriptVM::EOP); 
				;}
    break;

  case 13:
#line 404 "ai_service/script_parser.yacc"
    { NODE2 (yyval.ByteCode, CScriptVM::PUSH_GROUP, yyvsp[-1].Opcode); ;}
    break;

  case 14:
#line 405 "ai_service/script_parser.yacc"
    { NODE2 (yyval.ByteCode, CScriptVM::PUSH_CTX_VAR_VAL, yyvsp[-1].Opcode); ;}
    break;

  case 15:
#line 407 "ai_service/script_parser.yacc"
    { NODE2 (yyval.ByteCode, CScriptVM::PUSH_THIS, yyvsp[0].ByteCode); ;}
    break;

  case 16:
#line 409 "ai_service/script_parser.yacc"
    { NODE2 (yyval.ByteCode, yyvsp[-1].ByteCode, yyvsp[0].ByteCode); ;}
    break;

  case 17:
#line 411 "ai_service/script_parser.yacc"
    { NODE3 (yyval.ByteCode, CScriptVM::PUSH_THIS, CScriptVM::CALL, yyvsp[-2].Opcode); ;}
    break;

  case 18:
#line 412 "ai_service/script_parser.yacc"
    { ERROR_DETECTED (yyval.ByteCode, "missing ')' after the function name"); ;}
    break;

  case 19:
#line 413 "ai_service/script_parser.yacc"
    { ERROR_DETECTED (yyval.ByteCode, "missing '(' after the function name");	;}
    break;

  case 20:
#line 415 "ai_service/script_parser.yacc"
    { NODE3 (yyval.ByteCode, yyvsp[-3].ByteCode, CScriptVM::CALL, yyvsp[-2].Opcode); ;}
    break;

  case 21:
#line 417 "ai_service/script_parser.yacc"
    { ERROR_DETECTED (yyval.ByteCode, "missing ')' after the function name");	;}
    break;

  case 22:
#line 419 "ai_service/script_parser.yacc"
    { ERROR_DETECTED (yyval.ByteCode, "missinga '(' after the function name"); ;}
    break;

  case 23:
#line 423 "ai_service/script_parser.yacc"
    { 
					NODE2 (yyval.ByteCode, yyvsp[0].ByteCode, CScriptVM::PUSH_THIS);
					nativeFunc (yyval.ByteCode, yyvsp[-1].Opcode.Opcode, yyvsp[0].ByteCode.Signature, yyvsp[-2].ByteCode.Signature);
					addNode (yyval.ByteCode, yyvsp[-2].ByteCode);
					TYPE1 (yyval.ByteCode, yyvsp[-2].ByteCode);
				;}
    break;

  case 24:
#line 438 "ai_service/script_parser.yacc"
    { 
					NODE2 (yyval.ByteCode, yyvsp[0].ByteCode, yyvsp[-2].ByteCode);
					nativeFunc (yyval.ByteCode, yyvsp[-1].Opcode.Opcode, yyvsp[0].ByteCode.Signature, yyvsp[-3].ByteCode.Signature);
					addNode (yyval.ByteCode, yyvsp[-3].ByteCode);
					TYPE1 (yyval.ByteCode, yyvsp[-3].ByteCode);
				;}
    break;

  case 25:
#line 445 "ai_service/script_parser.yacc"
    { NODE3 (yyval.ByteCode, CScriptVM::PUSH_THIS, CScriptVM::PUSH_STRING, yyvsp[-2].Opcode); ;}
    break;

  case 26:
#line 447 "ai_service/script_parser.yacc"
    { NODE3 (yyval.ByteCode, yyvsp[-3].ByteCode, CScriptVM::PUSH_STRING, yyvsp[-2].Opcode); ;}
    break;

  case 27:
#line 449 "ai_service/script_parser.yacc"
    { NODE3 (yyval.ByteCode, CScriptVM::PUSH_THIS, CScriptVM::PUSH_STRING, yyvsp[-2].Opcode); ;}
    break;

  case 28:
#line 451 "ai_service/script_parser.yacc"
    { NODE3 (yyval.ByteCode, yyvsp[-3].ByteCode, CScriptVM::PUSH_STRING, yyvsp[-2].Opcode); ;}
    break;

  case 29:
#line 453 "ai_service/script_parser.yacc"
    { NODE3 (yyval.ByteCode, yyvsp[-2].ByteCode, yyvsp[0].ByteCode, CScriptVM::ASSIGN_FUNC_FROM); ;}
    break;

  case 30:
#line 455 "ai_service/script_parser.yacc"
    { NODE2 (yyval.ByteCode, yyvsp[-2].ByteCode, yyvsp[0].ByteCode); TYPE2 (yyval.ByteCode, yyvsp[-2].ByteCode, yyvsp[0].ByteCode); ;}
    break;

  case 31:
#line 456 "ai_service/script_parser.yacc"
    { yyval.ByteCode = yyvsp[0].ByteCode; ;}
    break;

  case 32:
#line 458 "ai_service/script_parser.yacc"
    { yyval.ByteCode = yyvsp[-1].ByteCode; ;}
    break;

  case 33:
#line 459 "ai_service/script_parser.yacc"
    { NODE0 (yyval.ByteCode); ;}
    break;

  case 34:
#line 461 "ai_service/script_parser.yacc"
    { yyval.ByteCode = yyvsp[0].ByteCode; ;}
    break;

  case 35:
#line 462 "ai_service/script_parser.yacc"
    { yyval.ByteCode = yyvsp[0].ByteCode; ;}
    break;

  case 36:
#line 465 "ai_service/script_parser.yacc"
    { 
					NODE2 (yyval.ByteCode, CScriptVM::PUSH_VAR_VAL, yyvsp[0].Opcode);
					TYPEF (yyval.ByteCode);
				;}
    break;

  case 37:
#line 475 "ai_service/script_parser.yacc"
    {
					NODE3 (yyval.ByteCode, yyvsp[-1].ByteCode, CScriptVM::PUSH_CONTEXT_VAR_VAL, yyvsp[0].Opcode);
					TYPEF (yyval.ByteCode); 
				;}
    break;

  case 38:
#line 480 "ai_service/script_parser.yacc"
    { 
					NODE2 (yyval.ByteCode, CScriptVM::PUSH_STR_VAR_VAL, yyvsp[0].Opcode);
					TYPES (yyval.ByteCode);
				;}
    break;

  case 39:
#line 490 "ai_service/script_parser.yacc"
    { 
					NODE3 (yyval.ByteCode, yyvsp[-1].ByteCode, CScriptVM::PUSH_CONTEXT_STR_VAR_VAL, yyvsp[0].Opcode);
					TYPES (yyval.ByteCode);
				;}
    break;

  case 40:
#line 495 "ai_service/script_parser.yacc"
    { 
					NODE2 (yyval.ByteCode, CScriptVM::PUSH_CTX_VAR_VAL, yyvsp[0].Opcode);
					TYPEC (yyval.ByteCode);
				;}
    break;

  case 41:
#line 505 "ai_service/script_parser.yacc"
    { 
					NODE3 (yyval.ByteCode, yyvsp[-1].ByteCode, CScriptVM::PUSH_CONTEXT_CTX_VAR_VAL, yyvsp[0].Opcode);
					TYPEC (yyval.ByteCode);
				;}
    break;

  case 42:
#line 510 "ai_service/script_parser.yacc"
    { 
					NODE2 (yyval.ByteCode, CScriptVM::PUSH_STRING, yyvsp[0].Opcode);
					TYPES (yyval.ByteCode);
				;}
    break;

  case 43:
#line 515 "ai_service/script_parser.yacc"
    { 
					NODE2 (yyval.ByteCode, CScriptVM::PUSH_ON_STACK, yyvsp[0].Opcode);
					TYPEF (yyval.ByteCode);
				;}
    break;

  case 44:
#line 520 "ai_service/script_parser.yacc"
    { NODE2 (yyval.ByteCode, CScriptVM::SET_VAR_VAL, yyvsp[0].Opcode); TYPEF (yyval.ByteCode); ;}
    break;

  case 45:
#line 522 "ai_service/script_parser.yacc"
    { NODE3 (yyval.ByteCode, yyvsp[-1].ByteCode, CScriptVM::SET_CONTEXT_VAR_VAL, yyvsp[0].Opcode); TYPEF (yyval.ByteCode); ;}
    break;

  case 46:
#line 523 "ai_service/script_parser.yacc"
    { NODE2 (yyval.ByteCode, CScriptVM::SET_STR_VAR_VAL, yyvsp[0].Opcode); TYPES (yyval.ByteCode); ;}
    break;

  case 47:
#line 525 "ai_service/script_parser.yacc"
    { NODE3 (yyval.ByteCode, yyvsp[-1].ByteCode, CScriptVM::SET_CONTEXT_STR_VAR_VAL, yyvsp[0].Opcode); TYPES (yyval.ByteCode); ;}
    break;

  case 48:
#line 526 "ai_service/script_parser.yacc"
    { NODE2 (yyval.ByteCode, CScriptVM::SET_CTX_VAR_VAL, yyvsp[0].Opcode); TYPEC (yyval.ByteCode); ;}
    break;

  case 49:
#line 528 "ai_service/script_parser.yacc"
    { NODE3 (yyval.ByteCode, yyvsp[-1].ByteCode, CScriptVM::SET_CONTEXT_CTX_VAR_VAL, yyvsp[0].Opcode); TYPEC (yyval.ByteCode); ;}
    break;

  case 50:
#line 530 "ai_service/script_parser.yacc"
    { NODE2 (yyval.ByteCode, yyvsp[-2].ByteCode, yyvsp[0].ByteCode); TYPE2 (yyval.ByteCode, yyvsp[-2].ByteCode, yyvsp[0].ByteCode); ;}
    break;

  case 51:
#line 531 "ai_service/script_parser.yacc"
    { ERROR_DETECTED (yyval.ByteCode, "missing ',' between two expressions"); ;}
    break;

  case 52:
#line 532 "ai_service/script_parser.yacc"
    { yyval.ByteCode = yyvsp[0].ByteCode; ;}
    break;

  case 53:
#line 534 "ai_service/script_parser.yacc"
    { yyval.ByteCode = yyvsp[-1].ByteCode; ;}
    break;

  case 54:
#line 535 "ai_service/script_parser.yacc"
    { ERROR_DETECTED (yyval.ByteCode, "missing ')' at the end of the parameters"); ;}
    break;

  case 55:
#line 536 "ai_service/script_parser.yacc"
    { NODE0 (yyval.ByteCode); ;}
    break;

  case 56:
#line 539 "ai_service/script_parser.yacc"
    { 
					// No need to check the types. All assignations are possibles.
					NODE2 (yyval.ByteCode, yyvsp[0].ByteCode, yyvsp[-2].ByteCode); 
				;}
    break;

  case 57:
#line 543 "ai_service/script_parser.yacc"
    { ERROR_DETECTED (yyval.ByteCode, "missing ')' at the end of the expression"); ;}
    break;

  case 58:
#line 544 "ai_service/script_parser.yacc"
    { ERROR_DETECTED (yyval.ByteCode, "missing '(' at the beginning of the expression");;}
    break;

  case 59:
#line 546 "ai_service/script_parser.yacc"
    { NODE3 (yyval.ByteCode, yyvsp[-2].ByteCode, CScriptVM::PUSH_PRINT_STRING, yyvsp[0].Opcode); ;}
    break;

  case 60:
#line 547 "ai_service/script_parser.yacc"
    { NODE2 (yyval.ByteCode, CScriptVM::PUSH_PRINT_STRING, yyvsp[0].Opcode); ;}
    break;

  case 61:
#line 548 "ai_service/script_parser.yacc"
    { NODE3 (yyval.ByteCode, yyvsp[-2].ByteCode, CScriptVM::PUSH_PRINT_VAR, yyvsp[0].Opcode); ;}
    break;

  case 62:
#line 549 "ai_service/script_parser.yacc"
    { NODE2 (yyval.ByteCode, CScriptVM::PUSH_PRINT_VAR, yyvsp[0].Opcode); ;}
    break;

  case 63:
#line 550 "ai_service/script_parser.yacc"
    { NODE3 (yyval.ByteCode, yyvsp[-2].ByteCode, CScriptVM::PUSH_PRINT_STR_VAR, yyvsp[0].Opcode); ;}
    break;

  case 64:
#line 551 "ai_service/script_parser.yacc"
    { NODE2 (yyval.ByteCode, CScriptVM::PUSH_PRINT_STR_VAR, yyvsp[0].Opcode); ;}
    break;

  case 65:
#line 553 "ai_service/script_parser.yacc"
    { NODE2 (yyval.ByteCode, yyvsp[-1].ByteCode, CScriptVM::PRINT_STRING); ;}
    break;

  case 66:
#line 555 "ai_service/script_parser.yacc"
    { NODE2 (yyval.ByteCode, yyvsp[-1].ByteCode, CScriptVM::LOG_STRING); ;}
    break;

  case 67:
#line 557 "ai_service/script_parser.yacc"
    { yyval.ByteCode = yyvsp[0].ByteCode; ;}
    break;

  case 68:
#line 558 "ai_service/script_parser.yacc"
    { yyval.ByteCode = yyvsp[0].ByteCode; ;}
    break;

  case 69:
#line 561 "ai_service/script_parser.yacc"
    {
					int sizeToJump = yyvsp[0].ByteCode.ByteCode->size() + 1;	// 1 jump instruction to escape
					NODE4 (yyval.ByteCode, yyvsp[-2].ByteCode, CScriptVM::JE, sizeToJump, yyvsp[0].ByteCode);
				;}
    break;

  case 70:
#line 565 "ai_service/script_parser.yacc"
    {ERROR_DETECTED (yyval.ByteCode, "missing ')' at the end of the if condition");;}
    break;

  case 71:
#line 566 "ai_service/script_parser.yacc"
    {ERROR_DETECTED (yyval.ByteCode, "missing '(' at the beginning of the if condition");;}
    break;

  case 72:
#line 568 "ai_service/script_parser.yacc"
    { 
					int sizeToJump0 = yyvsp[-2].ByteCode.ByteCode->size() + 3;	// 2 jump instructions to escape
					int sizeToJump1 = yyvsp[0].ByteCode.ByteCode->size() + 1;	// 1 jump instruction to escape
					NODE7 (yyval.ByteCode, yyvsp[-4].ByteCode, CScriptVM::JE, sizeToJump0, yyvsp[-2].ByteCode, CScriptVM::JUMP, sizeToJump1, yyvsp[0].ByteCode);
				;}
    break;

  case 73:
#line 573 "ai_service/script_parser.yacc"
    { ERROR_DETECTED (yyval.ByteCode, "missing ')' at the end of the if condition");;}
    break;

  case 74:
#line 574 "ai_service/script_parser.yacc"
    { ERROR_DETECTED (yyval.ByteCode, "missing '(' at the beginning of the if condition");;}
    break;

  case 75:
#line 576 "ai_service/script_parser.yacc"
    { 
					int sizeToJump0 = yyvsp[0].ByteCode.ByteCode->size() + 3;		// 2 jump instructions to escape
					int sizeToJump1 = -(int)yyvsp[0].ByteCode.ByteCode->size() - 3 - (int)yyvsp[-2].ByteCode.ByteCode->size();	// 1 jump instruction to escape
					NODE6 (yyval.ByteCode, yyvsp[-2].ByteCode, CScriptVM::JE, sizeToJump0, yyvsp[0].ByteCode, CScriptVM::JUMP, sizeToJump1);
				;}
    break;

  case 76:
#line 581 "ai_service/script_parser.yacc"
    { ERROR_DETECTED (yyval.ByteCode, "missing ')' at the end of the while condition");;}
    break;

  case 77:
#line 582 "ai_service/script_parser.yacc"
    { ERROR_DETECTED (yyval.ByteCode, "missing '(' at the beginning of the while condition");;}
    break;

  case 78:
#line 585 "ai_service/script_parser.yacc"
    { 
					int sizeToJump0 = yyvsp[-2].ByteCode.ByteCode->size() + 3;	// 2 jump instructions to escape
					int sizeToJump1 = yyvsp[0].ByteCode.ByteCode->size() + 1;	// 1 jump instruction to escape
					NODE7 (yyval.ByteCode, yyvsp[-4].ByteCode, CScriptVM::JE, sizeToJump0, yyvsp[-2].ByteCode, CScriptVM::JUMP, sizeToJump1, yyvsp[0].ByteCode);
				;}
    break;

  case 79:
#line 590 "ai_service/script_parser.yacc"
    { ERROR_DETECTED (yyval.ByteCode, "missing ')' at the end of the if condition");;}
    break;

  case 80:
#line 591 "ai_service/script_parser.yacc"
    { ERROR_DETECTED (yyval.ByteCode, "missing '(' at the end of the if condition");;}
    break;

  case 81:
#line 593 "ai_service/script_parser.yacc"
    { 
					int sizeToJump0 = yyvsp[0].ByteCode.ByteCode->size() + 3;		// 2 jump instructions to escape
					int sizeToJump1 = -(int)yyvsp[0].ByteCode.ByteCode->size() - 3 - (int)yyvsp[-2].ByteCode.ByteCode->size();	// 1 jump instruction to escape
					NODE6 (yyval.ByteCode, yyvsp[-2].ByteCode, CScriptVM::JE, sizeToJump0, yyvsp[0].ByteCode, CScriptVM::JUMP, sizeToJump1);
				;}
    break;

  case 82:
#line 598 "ai_service/script_parser.yacc"
    { ERROR_DETECTED (yyval.ByteCode, "missing ')' at the end of the while condition");;}
    break;

  case 83:
#line 599 "ai_service/script_parser.yacc"
    { ERROR_DETECTED (yyval.ByteCode, "missing '(' at the beginning of the while condition");;}
    break;

  case 84:
#line 600 "ai_service/script_parser.yacc"
    { yyval.ByteCode = yyvsp[-1].ByteCode; ;}
    break;

  case 85:
#line 601 "ai_service/script_parser.yacc"
    { yyval.ByteCode = yyvsp[-1].ByteCode; ;}
    break;

  case 86:
#line 602 "ai_service/script_parser.yacc"
    { yyval.ByteCode = yyvsp[-1].ByteCode; ;}
    break;

  case 87:
#line 603 "ai_service/script_parser.yacc"
    { yyval.ByteCode = yyvsp[0].ByteCode; ;}
    break;

  case 88:
#line 604 "ai_service/script_parser.yacc"
    { yyval.ByteCode = yyvsp[-1].ByteCode; ;}
    break;

  case 89:
#line 605 "ai_service/script_parser.yacc"
    { yyval.ByteCode = yyvsp[-1].ByteCode; ;}
    break;

  case 90:
#line 606 "ai_service/script_parser.yacc"
    { yyval.ByteCode = yyvsp[-1].ByteCode; ;}
    break;

  case 91:
#line 607 "ai_service/script_parser.yacc"
    { yyval.ByteCode = yyvsp[-1].ByteCode; ;}
    break;

  case 92:
#line 608 "ai_service/script_parser.yacc"
    { yyval.ByteCode = yyvsp[0].ByteCode; ;}
    break;

  case 93:
#line 609 "ai_service/script_parser.yacc"
    { yyval.ByteCode = yyvsp[0].ByteCode; ;}
    break;

  case 94:
#line 610 "ai_service/script_parser.yacc"
    { yyval.ByteCode = yyvsp[0].ByteCode; ;}
    break;

  case 95:
#line 611 "ai_service/script_parser.yacc"
    { NODE5 (yyval.ByteCode, CScriptVM::PUSH_VAR_VAL, yyvsp[-2].Opcode, yyvsp[-1].Opcode, CScriptVM::SET_VAR_VAL, yyvsp[-2].Opcode); ;}
    break;

  case 96:
#line 612 "ai_service/script_parser.yacc"
    { NODE5 (yyval.ByteCode, CScriptVM::PUSH_VAR_VAL, yyvsp[-1].Opcode, yyvsp[-2].Opcode, CScriptVM::SET_VAR_VAL, yyvsp[-1].Opcode); ;}
    break;

  case 97:
#line 614 "ai_service/script_parser.yacc"
    { NODE7 (yyval.ByteCode, yyvsp[-3].ByteCode, CScriptVM::PUSH_CONTEXT_VAR_VAL, yyvsp[-2].Opcode, yyvsp[-1].Opcode, yyvsp[-3].ByteCode, CScriptVM::SET_CONTEXT_VAR_VAL, yyvsp[-2].Opcode); ;}
    break;

  case 98:
#line 616 "ai_service/script_parser.yacc"
    { NODE7 (yyval.ByteCode, yyvsp[-2].ByteCode, CScriptVM::PUSH_CONTEXT_VAR_VAL, yyvsp[-1].Opcode, yyvsp[-3].Opcode, yyvsp[-2].ByteCode, CScriptVM::SET_CONTEXT_VAR_VAL, yyvsp[-1].Opcode); ;}
    break;

  case 99:
#line 617 "ai_service/script_parser.yacc"
    { NODE6 (yyval.ByteCode, CScriptVM::PUSH_VAR_VAL, yyvsp[-3].Opcode, yyvsp[-1].ByteCode, yyvsp[-2].Operator, CScriptVM::SET_VAR_VAL, yyvsp[-3].Opcode); ;}
    break;

  case 100:
#line 618 "ai_service/script_parser.yacc"
    { ERROR_DETECTED (yyval.ByteCode, "missing ')' at the end of the expression");;}
    break;

  case 101:
#line 619 "ai_service/script_parser.yacc"
    { ERROR_DETECTED (yyval.ByteCode, "missing '(' at the beginning of the expression");;}
    break;

  case 102:
#line 621 "ai_service/script_parser.yacc"
    { NODE8 (yyval.ByteCode, yyvsp[-4].ByteCode, CScriptVM::PUSH_CONTEXT_VAR_VAL, yyvsp[-4].ByteCode, yyvsp[-1].ByteCode, yyvsp[-2].Operator, yyvsp[-4].ByteCode, CScriptVM::SET_CONTEXT_VAR_VAL, yyvsp[-3].Opcode); ;}
    break;

  case 103:
#line 623 "ai_service/script_parser.yacc"
    { ERROR_DETECTED (yyval.ByteCode, "missing ')' at the end of the expression");;}
    break;

  case 104:
#line 625 "ai_service/script_parser.yacc"
    { ERROR_DETECTED (yyval.ByteCode, "missing '(' at the beginning of the expression");;}
    break;

  case 105:
#line 626 "ai_service/script_parser.yacc"
    { NODE1 (yyval.ByteCode, yyvsp[0].ByteCodeList); ;}
    break;

  case 106:
#line 627 "ai_service/script_parser.yacc"
    { NODE0 (yyval.ByteCode); ;}
    break;

  case 107:
#line 629 "ai_service/script_parser.yacc"
    { createList (yyval.ByteCodeList);  addNode (yyval.ByteCodeList, yyvsp[0].ByteCode);  ;}
    break;

  case 108:
#line 630 "ai_service/script_parser.yacc"
    { yyval.ByteCodeList = yyvsp[-1].ByteCodeList; addNode (yyval.ByteCodeList, yyvsp[0].ByteCode); ;}
    break;

  case 109:
#line 633 "ai_service/script_parser.yacc"
    { 
					createNode (yyval.ByteCode);
					int childCount = yyvsp[0].ByteCodeList.ByteCodeList->size ();

					// Sum all the children size
					uint sizeToJump = getChildrenByteCodeSize (yyvsp[0].ByteCodeList.ByteCodeList);
					sizeToJump += 1*childCount;	// One for the JUMP
					sizeToJump += 1*childCount;	// One for the offset
					sizeToJump += 1*childCount;	// One for the RET
					
					sizeToJump += 1;	// One for the additionnal jump offset
					sizeToJump += 1;	// One for the RANDEND

					addNode (yyval.ByteCode, CScriptVM::RAND);
					addNode (yyval.ByteCode, childCount);
					addNode (yyval.ByteCode, CScriptVM::JUMP);
					addNode (yyval.ByteCode, sizeToJump);
					sizeToJump -= 2;

					// Write the jump table
					list<vector<size_t> * >::reverse_iterator rite = yyvsp[0].ByteCodeList.ByteCodeList->rbegin();
					while (rite != yyvsp[0].ByteCodeList.ByteCodeList->rend())
					{
						sizeToJump -= (*rite)->size ();
						
						addNode (yyval.ByteCode, CScriptVM::JUMP);
						addNode (yyval.ByteCode, sizeToJump);
						sizeToJump -= 1;	// One for the JUMP
						sizeToJump -= 1;	// One for the offset
						sizeToJump -= 1;	// One for the RET

						rite++;
					}	

					// Write the code
					list<vector<size_t> * >::iterator ite = yyvsp[0].ByteCodeList.ByteCodeList->begin();
					while (ite != yyvsp[0].ByteCodeList.ByteCodeList->end())
					{
						yyval.ByteCode.ByteCode->insert (yyval.ByteCode.ByteCode->end(), (*ite)->begin(), (*ite)->end());
						addNode (yyval.ByteCode, CScriptVM::RET);
						ite++;
					}

					// End
					addNode (yyval.ByteCode, CScriptVM::RANDEND);
				;}
    break;

  case 110:
#line 680 "ai_service/script_parser.yacc"
    { yyval.Opcode = yyvsp[0].Opcode; TYPES (yyval.Opcode); ;}
    break;

  case 111:
#line 681 "ai_service/script_parser.yacc"
    { yyval.Opcode = yyvsp[0].Opcode; TYPEF (yyval.Opcode); ;}
    break;

  case 112:
#line 684 "ai_service/script_parser.yacc"
    { 
					createCase (yyval.Case, yyvsp[-2].Opcode, yyvsp[0].ByteCode);	// Create a two entry list to keep track of the case index and the case code
				;}
    break;

  case 113:
#line 689 "ai_service/script_parser.yacc"
    { 
					createSwitch (yyval.Switch);	// Create a new case list
					addNode (yyval.Switch, yyvsp[0].Case.Case);	// Add the first case
					TYPE1 (yyval.Switch, yyvsp[0].Case);		// Propagate the key type
				;}
    break;

  case 114:
#line 695 "ai_service/script_parser.yacc"
    { 
					yyval.Switch = yyvsp[-1].Switch; 
					addNode (yyval.Switch, yyvsp[0].Case.Case);	// Add a case to the list
				;}
    break;

  case 115:
#line 701 "ai_service/script_parser.yacc"
    {
					// Expression
					createNode (yyval.ByteCode);
					addNode (yyval.ByteCode, yyvsp[-4].ByteCode);
					
					int childCount = yyvsp[-1].Switch.Cases->size ();
					
					// Sum all the children size
					uint sizeChild = getChildrenByteCodeSize (yyvsp[-1].Switch.Cases);
					sizeChild += 1*childCount;	// One for the RET

					uint sizeToJump = 0;
					sizeToJump += 1*childCount;	// One for the case key
					sizeToJump += 1*childCount;	// One for the offset
					
					sizeToJump += 1;	// One for the additionnal jump offset

					addNode (yyval.ByteCode, CScriptVM::SWITCH);	// Switch opcode
					addNode (yyval.ByteCode, childCount);			// Number of switch cases
					addNode (yyval.ByteCode, sizeToJump+sizeChild);	// Return address
					sizeToJump -= 2;

					// Write the jump table
					map<size_t, CCase *>::const_iterator ite = yyvsp[-1].Switch.Cases->begin ();
					while (ite != yyvsp[-1].Switch.Cases->end ())
					{
						const CCase *_case = ite->second;

						// Type checking
						if (_case->getType () != yyvsp[-4].ByteCode.getType ())
							aiOutputError (_case->Line, "case has not the same type than the switch expression: the case is %s and the switch is %s", 
								_case->getType (), yyvsp[-4].ByteCode.getType ());

						addNode (yyval.ByteCode, _case->Case);
						addNode (yyval.ByteCode, sizeToJump);
						sizeToJump += _case->ByteCode->size ();
						sizeToJump += 1;	// One for the RET
						sizeToJump -= 1;	// One for the case key
						sizeToJump -= 1;	// One for the offset
						ite++;
					}	

					// Write the code
					ite = yyvsp[-1].Switch.Cases->begin ();
					while (ite != yyvsp[-1].Switch.Cases->end ())
					{
						const CCase *_case = ite->second;
						yyval.ByteCode.ByteCode->insert (yyval.ByteCode.ByteCode->end(), _case->ByteCode->begin(), _case->ByteCode->end());
						addNode (yyval.ByteCode, CScriptVM::RET);
						ite++;
					}
				;}
    break;

  case 116:
#line 753 "ai_service/script_parser.yacc"
    {ERROR_DETECTED (yyval.ByteCode, "missing ')' at the end of the switch expression");;}
    break;

  case 117:
#line 754 "ai_service/script_parser.yacc"
    {ERROR_DETECTED (yyval.ByteCode, "missing '(' at the beginning of the switch expression");;}
    break;

  case 118:
#line 755 "ai_service/script_parser.yacc"
    {ERROR_DETECTED (yyval.ByteCode, "missing '{' at the beginning of the switch cases");;}
    break;

  case 119:
#line 758 "ai_service/script_parser.yacc"
    { yyval.ByteCodeList = yyvsp[-1].ByteCodeList; ;}
    break;

  case 120:
#line 761 "ai_service/script_parser.yacc"
    { 
					int sizeToJump = getChildrenByteCodeSize (yyvsp[0].ByteCodeList.ByteCodeList);
					sizeToJump ++;	// One for the jump offset
					sizeToJump ++;	// One for the EOP instruction
					NODE5 (yyval.ByteCode, CScriptVM::ONCHILDREN, CScriptVM::JUMP, sizeToJump, yyvsp[0].ByteCodeList, CScriptVM::EOP);
				;}
    break;


    }

/* Line 991 of yacc.c.  */
#line 2375 "ai_service/script_parser_yacc.cpp"

  yyvsp -= yylen;
  yyssp -= yylen;


  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (YYPACT_NINF < yyn && yyn < YYLAST)
	{
	  YYSIZE_T yysize = 0;
	  int yytype = YYTRANSLATE (yychar);
	  char *yymsg;
	  int yyx, yycount;

	  yycount = 0;
	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  for (yyx = yyn < 0 ? -yyn : 0;
	       yyx < (int) (sizeof (yytname) / sizeof (char *)); yyx++)
	    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	      yysize += yystrlen (yytname[yyx]) + 15, yycount++;
	  yysize += yystrlen ("syntax error, unexpected ") + 1;
	  yysize += yystrlen (yytname[yytype]);
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "syntax error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[yytype]);

	      if (yycount < 5)
		{
		  yycount = 0;
		  for (yyx = yyn < 0 ? -yyn : 0;
		       yyx < (int) (sizeof (yytname) / sizeof (char *));
		       yyx++)
		    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
		      {
			const char *yyq = ! yycount ? ", expecting " : " or ";
			yyp = yystpcpy (yyp, yyq);
			yyp = yystpcpy (yyp, yytname[yyx]);
			yycount++;
		      }
		}
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    yyerror ("syntax error; also virtual memory exhausted");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror ("syntax error");
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      /* Return failure if at end of input.  */
      if (yychar == YYEOF)
        {
	  /* Pop the error token.  */
          YYPOPSTACK;
	  /* Pop the rest of the stack.  */
	  while (yyss < yyssp)
	    {
	      YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
	      yydestruct (yystos[*yyssp], yyvsp);
	      YYPOPSTACK;
	    }
	  YYABORT;
        }

      YYDSYMPRINTF ("Error: discarding", yytoken, &yylval, &yylloc);
      yydestruct (yytoken, &yylval);
      yychar = YYEMPTY;

    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab2;


/*----------------------------------------------------.
| yyerrlab1 -- error raised explicitly by an action.  |
`----------------------------------------------------*/
yyerrlab1:

  /* Suppress GCC warning that yyerrlab1 is unused when no action
     invokes YYERROR.  */
#if defined (__GNUC_MINOR__) && 2093 <= (__GNUC__ * 1000 + __GNUC_MINOR__)
//  __attribute__ ((__unused__))
#endif


  goto yyerrlab2;


/*---------------------------------------------------------------.
| yyerrlab2 -- pop states until the error token can be shifted.  |
`---------------------------------------------------------------*/
yyerrlab2:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;

      YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
      yydestruct (yystos[yystate], yyvsp);
      yyvsp--;
      yystate = *--yyssp;

      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  YYDPRINTF ((stderr, "Shifting error token, "));

  *++yyvsp = yylval;


  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*----------------------------------------------.
| yyoverflowlab -- parser overflow comes here.  |
`----------------------------------------------*/
yyoverflowlab:
  yyerror ("parser stack overflow");
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}


#line 360 "ai_service/script_parser.yacc"


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

