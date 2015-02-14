
/*  A Bison parser, made from cf_gramatical.yxx
    by GNU Bison version 1.28  */

#define YYBISON 1  /* Identify Bison output.  */

#define yyparse cfparse
#define yylex cflex
#define yyerror cferror
#define yylval cflval
#define yychar cfchar
#define yydebug cfdebug
#define yynerrs cfnerrs
#define	ADD_ASSIGN	257
#define	ASSIGN	258
#define	VARIABLE	259
#define	STRING	260
#define	SEMICOLON	261
#define	PLUS	262
#define	MINUS	263
#define	MULT	264
#define	DIVIDE	265
#define	RPAREN	266
#define	LPAREN	267
#define	RBRACE	268
#define	LBRACE	269
#define	COMMA	270
#define	INTEGER	271
#define	REAL	272
#define	FILELINE	273

#line 1 "cf_gramatical.yxx"


/* Includes */

#ifdef NL_OS_WINDOWS
#pragma warning (disable : 4786)
#endif // NL_OS_WINDOWS


#include "nel/misc/config_file.h"
#include "nel/misc/common.h"
#include "nel/misc/debug.h"

#include <cstdio>
#include <vector>
#include <string>

using namespace std;
using namespace NLMISC;

/* Constantes */

#define YYPARSE_PARAM pvararray

// WARNING!!!! DEBUG_PRINTF are commented using // so IT MUST HAVE NO INSTRUCTION AFTER A DEBUG_PRINTF OR THEY LL BE COMMENTED
/*
#define DEBUG_PRINTF	InfoLog->displayRaw
#define DEBUG_PRINT(a)	InfoLog->displayRaw(a)
*/

#define DEBUG_PRINT(a)
#ifdef __GNUC__
#define DEBUG_PRINTF(format, args...)
#else // __GNUC__
#define DEBUG_PRINTF	// InfoLog->displayRaw
#endif // __GNUC__


/* Types */

enum cf_operation { OP_PLUS, OP_MINUS, OP_MULT, OP_DIVIDE, OP_NEG };

struct cf_value
{
	NLMISC::CConfigFile::CVar::TVarType	Type;
	int						Int;
	double					Real;
	char					String[1024];
};

/* Externals */

extern bool cf_Ignore;

extern bool LoadRoot;

extern FILE *yyin;

/* Variables */

NLMISC::CConfigFile::CVar		cf_CurrentVar;

int		cf_CurrentLine;
char	*cf_CurrentFile;

bool	cf_OverwriteExistingVariable;	// setup in the config_file.cpp reparse()



/* Prototypes */

int yylex (void);

cf_value cf_op (cf_value a, cf_value b, cf_operation op);

void cf_print (cf_value Val);

void cf_setVar (NLMISC::CConfigFile::CVar &Var, cf_value Val);

int yyerror (const char *);


#line 85 "cf_gramatical.yxx"
typedef union	{
			cf_value Val;
		} YYSTYPE;
#include <cstdio>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		47
#define	YYFLAG		-32768
#define	YYNTBASE	20

#define YYTRANSLATE(x) ((unsigned)(x) <= 273 ? yytranslate[x] : 29)

static const char yytranslate[] = {     0,
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
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     1,     3,     4,     5,     6,
     7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
    17,    18,    19
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     2,     3,     6,     8,    12,    17,    22,    24,    28,
    33,    36,    38,    42,    44,    48,    52,    54,    58,    62,
    65,    68,    72,    74,    76,    78,    80
};

static const short yyrhs[] = {    21,
     0,     0,    21,    22,     0,    22,     0,    19,     6,    17,
     0,     5,     4,    23,     7,     0,     5,     3,    23,     7,
     0,    25,     0,    15,    24,    14,     0,    15,    24,    16,
    14,     0,    15,    14,     0,    25,     0,    24,    16,    25,
     0,    26,     0,    25,     8,    26,     0,    25,     9,    26,
     0,    27,     0,    26,    10,    27,     0,    26,    11,    27,
     0,     8,    27,     0,     9,    27,     0,    13,    23,    12,
     0,    17,     0,    18,     0,     6,     0,    28,     0,     5,
     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
   104,   104,   107,   108,   111,   123,   185,   245,   246,   247,
   248,   251,   252,   255,   256,   257,   260,   261,   262,   265,
   266,   267,   268,   269,   270,   271,   274
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","ADD_ASSIGN",
"ASSIGN","VARIABLE","STRING","SEMICOLON","PLUS","MINUS","MULT","DIVIDE","RPAREN",
"LPAREN","RBRACE","LBRACE","COMMA","INTEGER","REAL","FILELINE","ROOT","instlist",
"inst","expression","exprbrace","expr2","expr3","expr4","variable", NULL
};
#endif

static const short yyr1[] = {     0,
    20,    20,    21,    21,    22,    22,    22,    23,    23,    23,
    23,    24,    24,    25,    25,    25,    26,    26,    26,    27,
    27,    27,    27,    27,    27,    27,    28
};

static const short yyr2[] = {     0,
     1,     0,     2,     1,     3,     4,     4,     1,     3,     4,
     2,     1,     3,     1,     3,     3,     1,     3,     3,     2,
     2,     3,     1,     1,     1,     1,     1
};

static const short yydefact[] = {     2,
     0,     0,     1,     4,     0,     0,     0,     3,    27,    25,
     0,     0,     0,     0,    23,    24,     0,     8,    14,    17,
    26,     0,     5,    20,    21,     0,    11,     0,    12,     7,
     0,     0,     0,     0,     6,    22,     9,     0,    15,    16,
    18,    19,    10,    13,     0,     0,     0
};

static const short yydefgoto[] = {    45,
     3,     4,    17,    28,    18,    19,    20,    21
};

static const short yypact[] = {    -4,
    17,     2,    -4,-32768,     1,     1,   -15,-32768,-32768,-32768,
    50,    50,     1,    22,-32768,-32768,    10,    14,    23,-32768,
-32768,    25,-32768,-32768,-32768,    31,-32768,    -3,    14,-32768,
    50,    50,    50,    50,-32768,-32768,-32768,    36,    23,    23,
-32768,-32768,-32768,    14,    29,    46,-32768
};

static const short yypgoto[] = {-32768,
-32768,    44,    -1,-32768,   -14,     6,    -8,-32768
};


#define	YYLAST		68


static const short yytable[] = {    29,
     1,    23,    24,    25,    22,     9,    10,     7,    11,    12,
    37,    26,    38,    13,     2,    14,    30,    15,    16,     5,
     6,    31,    32,    44,    41,    42,     9,    10,    46,    11,
    12,    35,    33,    34,    13,    27,    39,    40,    15,    16,
     9,    10,    36,    11,    12,    47,     8,     0,    13,    43,
     0,     0,    15,    16,     9,    10,     0,    11,    12,     0,
     0,     0,    13,     0,     0,     0,    15,    16
};

static const short yycheck[] = {    14,
     5,    17,    11,    12,     6,     5,     6,     6,     8,     9,
    14,    13,    16,    13,    19,    15,     7,    17,    18,     3,
     4,     8,     9,    38,    33,    34,     5,     6,     0,     8,
     9,     7,    10,    11,    13,    14,    31,    32,    17,    18,
     5,     6,    12,     8,     9,     0,     3,    -1,    13,    14,
    -1,    -1,    17,    18,     5,     6,    -1,     8,     9,    -1,
    -1,    -1,    13,    -1,    -1,    -1,    17,    18
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "cfbison.simple"
/* This file comes from bison-1.28.  */

/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Free Software Foundation, Inc.

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

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

#ifndef YYSTACK_USE_ALLOCA
#ifdef alloca
#define YYSTACK_USE_ALLOCA
#else /* alloca not defined */
#ifdef __GNUC__
#define YYSTACK_USE_ALLOCA
#define alloca __builtin_alloca
#else /* not GNU C.  */
#if (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc) || defined (__sgi) || (defined (__sun) && defined (__i386))
#define YYSTACK_USE_ALLOCA
#include <alloca.h>
#else /* not sparc */
/* We think this test detects Watcom and Microsoft C.  */
/* This used to test MSDOS, but that is a bad idea
   since that symbol is in the user namespace.  */
#if (defined (_MSDOS) || defined (_MSDOS_)) && !defined (__TURBOC__)
#if 0 /* No need for malloc.h, which pollutes the namespace;
	 instead, just don't use alloca.  */
#include <malloc.h>
#endif
#else /* not MSDOS, or __TURBOC__ */
#if defined(_AIX)
/* I don't know what this was needed for, but it pollutes the namespace.
   So I turned it off.   rms, 2 May 1997.  */
/* #include <malloc.h>  */
 #pragma alloca
#define YYSTACK_USE_ALLOCA
#else /* not MSDOS, or __TURBOC__, or _AIX */
#if 0
#ifdef __hpux /* haible@ilog.fr says this works for HPUX 9.05 and up,
		 and on HPUX 10.  Eventually we can turn this on.  */
#define YYSTACK_USE_ALLOCA
#define alloca __builtin_alloca
#endif /* __hpux */
#endif
#endif /* not _AIX */
#endif /* not MSDOS, or __TURBOC__ */
#endif /* not sparc */
#endif /* not GNU C */
#endif /* alloca not defined */
#endif /* YYSTACK_USE_ALLOCA not defined */

#ifdef YYSTACK_USE_ALLOCA
#define YYSTACK_ALLOC alloca
#else
#define YYSTACK_ALLOC malloc
#endif

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	goto yyacceptlab
#define YYABORT 	goto yyabortlab
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(token, value) \
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    { yychar = (token), yylval = (value);			\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { yyerror ("syntax error: cannot back up"); YYERROR; }	\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

#ifndef YYPURE
#define YYLEX		yylex()
#endif

#ifdef YYPURE
#ifdef YYLSP_NEEDED
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, &yylloc, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval, &yylloc)
#endif
#else /* not YYLSP_NEEDED */
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval)
#endif
#endif /* not YYLSP_NEEDED */
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYPURE

int	yychar;			/*  the lookahead symbol		*/
YYSTYPE	yylval;			/*  the semantic value of the		*/
				/*  lookahead symbol			*/

#ifdef YYLSP_NEEDED
YYLTYPE yylloc;			/*  location data for the lookahead	*/
				/*  symbol				*/
#endif

int yynerrs;			/*  number of parse errors so far       */
#endif  /* not YYPURE */

#if YYDEBUG != 0
int yydebug;			/*  nonzero means print parse trace	*/
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif

/*  YYINITDEPTH indicates the initial size of the parser's stacks	*/

#ifndef	YYINITDEPTH
#define YYINITDEPTH 200
#endif

/*  YYMAXDEPTH is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#if YYMAXDEPTH == 0
#undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif

/* Define __yy_memcpy.  Note that the size argument
   should be passed with type unsigned int, because that is what the non-GCC
   definitions require.  With GCC, __builtin_memcpy takes an arg
   of type size_t, but it can handle unsigned int.  */

#if __GNUC__ > 1		/* GNU C and GNU C++ define this.  */
#define __yy_memcpy(TO,FROM,COUNT)	__builtin_memcpy(TO,FROM,COUNT)
#else				/* not GNU C or C++ */
#ifndef __cplusplus

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (to, from, count)
     char *to;
     char *from;
     unsigned int count;
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#else /* __cplusplus */

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (char *to, char *from, unsigned int count)
{
  register char *t = to;
  register char *f = from;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#endif
#endif

#line 217 "cfbison.simple"

/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
#ifdef __cplusplus
#define YYPARSE_PARAM_ARG void *YYPARSE_PARAM
#define YYPARSE_PARAM_DECL
#else /* not __cplusplus */
#define YYPARSE_PARAM_ARG YYPARSE_PARAM
#define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
#endif /* not __cplusplus */
#else /* not YYPARSE_PARAM */
#define YYPARSE_PARAM_ARG
#define YYPARSE_PARAM_DECL
#endif /* not YYPARSE_PARAM */

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
#ifdef YYPARSE_PARAM
int yyparse (void *);
#else
int yyparse (void);
#endif
#endif

int
yyparse(YYPARSE_PARAM_ARG)
     YYPARSE_PARAM_DECL
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YYSTYPE *yyvsp;
  int yyerrstatus;	/*  number of tokens to shift before error messages enabled */
  int yychar1 = 0;		/*  lookahead token as an internal (translated) token number */

  short	yyssa[YYINITDEPTH];	/*  the state stack			*/
  YYSTYPE yyvsa[YYINITDEPTH];	/*  the semantic value stack		*/

  short *yyss = yyssa;		/*  refer to the stacks thru separate pointers */
  YYSTYPE *yyvs = yyvsa;	/*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YYLSP_NEEDED
  YYLTYPE yylsa[YYINITDEPTH];	/*  the location stack			*/
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  int yystacksize = YYINITDEPTH;
  int yyfree_stacks = 0;

#ifdef YYPURE
  int yychar;
  YYSTYPE yylval;
  int yynerrs;
#ifdef YYLSP_NEEDED
  YYLTYPE yylloc;
#endif
#endif

  YYSTYPE yyval;		/*  the variable used to return		*/
				/*  semantic values from the action	*/
				/*  routines				*/
 // ace: big fake for VC7 because it checks if yyval is init or not
  yyval.Val.Int = 0;
  int yylen;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Starting parse\n");
#endif

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YYLSP_NEEDED
  yylsp = yyls;
#endif

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
yynewstate:

  *++yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YYSTYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YYLSP_NEEDED
      YYLTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
#ifdef YYLSP_NEEDED
      /* This used to be a conditional around just the two extra args,
	 but that might be undefined if yyoverflow is a macro.  */
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yyls1, size * sizeof (*yylsp),
		 &yystacksize);
#else
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yystacksize);
#endif

      yyss = yyss1; yyvs = yyvs1;
#ifdef YYLSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	{
	  yyerror("parser stack overflow");
	  if (yyfree_stacks)
	    {
	      free (yyss);
	      free (yyvs);
#ifdef YYLSP_NEEDED
	      free (yyls);
#endif
	    }
	  return 2;
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
#ifndef YYSTACK_USE_ALLOCA
      yyfree_stacks = 1;
#endif
      yyss = (short *) YYSTACK_ALLOC (yystacksize * sizeof (*yyssp));
      __yy_memcpy ((char *)yyss, (char *)yyss1,
		   size * (unsigned int) sizeof (*yyssp));
      yyvs = (YYSTYPE *) YYSTACK_ALLOC (yystacksize * sizeof (*yyvsp));
      __yy_memcpy ((char *)yyvs, (char *)yyvs1,
		   size * (unsigned int) sizeof (*yyvsp));
#ifdef YYLSP_NEEDED
      yyls = (YYLTYPE *) YYSTACK_ALLOC (yystacksize * sizeof (*yylsp));
      __yy_memcpy ((char *)yyls, (char *)yyls1,
		   size * (unsigned int) sizeof (*yylsp));
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YYLSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Entering state %d\n", yystate);
#endif

  goto yybackup;
 yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Reading a token: ");
#endif
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(yychar);

#if YYDEBUG != 0
      if (yydebug)
	{
	  fprintf (stderr, "Next token is %d (%s", yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
#endif
	  fprintf (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting token %d (%s), ", yychar, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (yyerrstatus) yyerrstatus--;

  yystate = yyn;
  goto yynewstate;

/* Do the default action for the current state.  */
yydefault:

  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;

/* Do a reduction.  yyn is the number of a rule to reduce with.  */
yyreduce:
  yylen = yyr2[yyn];
  if (yylen > 0)
    yyval = yyvsp[1-yylen]; /* implement default value of the action */

#if YYDEBUG != 0
  if (yydebug)
    {
      int i;

      fprintf (stderr, "Reducing via rule %d (line %d), ",
	       yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (i = yyprhs[yyn]; yyrhs[i] > 0; i++)
	fprintf (stderr, "%s ", yytname[yyrhs[i]]);
      fprintf (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif


  switch (yyn) {

case 2:
#line 104 "cf_gramatical.yxx"
{ ;
    break;}
case 3:
#line 107 "cf_gramatical.yxx"
{ ;
    break;}
case 4:
#line 108 "cf_gramatical.yxx"
{ ;
    break;}
case 5:
#line 112 "cf_gramatical.yxx"
{
				DEBUG_PRINTF("Forcing current file %s and line %u\n", yyvsp[-1].Val.String, yyvsp[0].Val.Int-1);

				if (cf_CurrentFile != NULL)
					free(cf_CurrentFile);
				// store the filename
				cf_CurrentFile = strdup(yyvsp[-1].Val.String);
				// store the current line minus 1 because the #fileline count for a line
				cf_CurrentLine = yyvsp[0].Val.Int-1;
			;
    break;}
case 6:
#line 124 "cf_gramatical.yxx"
{
				DEBUG_PRINTF("                                   (TYPE %d VARIABLE=", yyvsp[-3].Val.Type);
				cf_print (yyvsp[-3].Val);
				DEBUG_PRINTF("), (TYPE %d VALUE=", yyvsp[-1].Val.Type);
				cf_print (yyvsp[-1].Val);
				DEBUG_PRINT(")\n");
				int i;
				// on recherche l'existence de la variable
				for(i = 0; i < (int)((*((vector<NLMISC::CConfigFile::CVar>*)(YYPARSE_PARAM))).size()); i++)
				{
					if ((*((vector<NLMISC::CConfigFile::CVar>*)(YYPARSE_PARAM)))[i].Name == yyvsp[-3].Val.String)
					{
						if (cf_OverwriteExistingVariable || (*((vector<NLMISC::CConfigFile::CVar>*)(YYPARSE_PARAM)))[i].Root || !strcmp(yyvsp[-3].Val.String,"RootConfigFilename"))
						{
							DEBUG_PRINTF("Variable '%s' existe deja, ecrasement\n", yyvsp[-3].Val.String);
						}
						break;
					}
				}
				NLMISC::CConfigFile::CVar Var;
				Var.Comp = false;
				Var.Callback = NULL;
				if (cf_CurrentVar.Comp)
				{
					DEBUG_PRINTF ("yacc: new assign complex variable '%s'\n", yyvsp[-3].Val.String);
					Var = cf_CurrentVar;
				}
				else
				{
					DEBUG_PRINTF ("yacc: new assign normal variable '%s'\n", yyvsp[-3].Val.String);
					cf_setVar (Var, yyvsp[-1].Val);
				}
				Var.Name = yyvsp[-3].Val.String;
				if (i == (int)((*((vector<NLMISC::CConfigFile::CVar>*)(YYPARSE_PARAM))).size ()))
				{
					// nouvelle variable
					DEBUG_PRINTF ("yacc: new assign var '%s'\n", yyvsp[-3].Val.String);
					(*((vector<NLMISC::CConfigFile::CVar>*)(YYPARSE_PARAM))).push_back (Var);
				}
				else if (cf_OverwriteExistingVariable || (*((vector<NLMISC::CConfigFile::CVar>*)(YYPARSE_PARAM)))[i].Root || !strcmp(yyvsp[-3].Val.String,"RootConfigFilename"))
				{
					// reaffectation d'une variable
					Var.Callback = (*((vector<NLMISC::CConfigFile::CVar>*)(YYPARSE_PARAM)))[i].Callback;
					DEBUG_PRINTF ("yacc: reassign var name '%s' type %d\n", Var.Name.c_str(), Var.Type);
					if (Var != (*((vector<NLMISC::CConfigFile::CVar>*)(YYPARSE_PARAM)))[i] && Var.Callback != NULL)
						(Var.Callback)(Var);
					(*((vector<NLMISC::CConfigFile::CVar>*)(YYPARSE_PARAM)))[i] = Var;
				}
				else
				{
					DEBUG_PRINTF ("yacc: don't reassign var '%s' because the variable already exists\n", yyvsp[-3].Val.String);
				}

				cf_CurrentVar.IntValues.clear ();
				cf_CurrentVar.RealValues.clear ();
				cf_CurrentVar.StrValues.clear ();
				cf_CurrentVar.Comp = false;
				cf_CurrentVar.Type = NLMISC::CConfigFile::CVar::T_UNKNOWN;
			;
    break;}
case 7:
#line 186 "cf_gramatical.yxx"
{
				DEBUG_PRINT("                                   (VARIABLE+=");
				cf_print (yyvsp[-3].Val);
				DEBUG_PRINT("), (VALUE=");
				cf_print (yyvsp[-1].Val);
				DEBUG_PRINT(")\n");
				int i;
				// on recherche l'existence de la variable
				for(i = 0; i < (int)((*((vector<NLMISC::CConfigFile::CVar>*)(YYPARSE_PARAM))).size()); i++)
				{
					if ((*((vector<NLMISC::CConfigFile::CVar>*)(YYPARSE_PARAM)))[i].Name == yyvsp[-3].Val.String)
					{
						DEBUG_PRINTF("Variable '%s' existe deja, ajout\n", yyvsp[-3].Val.String);
						break;
					}
				}
				NLMISC::CConfigFile::CVar Var;
				Var.Comp = false;
				Var.Callback = NULL;
				if (cf_CurrentVar.Comp) Var = cf_CurrentVar;
				else cf_setVar (Var, yyvsp[-1].Val);
				Var.Name = yyvsp[-3].Val.String;
				if (i == (int)((*((vector<NLMISC::CConfigFile::CVar>*)(YYPARSE_PARAM))).size ()))
				{
					// nouvelle variable
					DEBUG_PRINTF ("yacc: new add assign var '%s'\n", yyvsp[-3].Val.String);
					(*((vector<NLMISC::CConfigFile::CVar>*)(YYPARSE_PARAM))).push_back (Var);
				}
				else
				{
					// reaffectation d'une variable
					Var.Callback = (*((vector<NLMISC::CConfigFile::CVar>*)(YYPARSE_PARAM)))[i].Callback;
					DEBUG_PRINTF ("yacc: add assign var '%s'\n", yyvsp[-3].Val.String);
					if ((*((vector<NLMISC::CConfigFile::CVar>*)(YYPARSE_PARAM)))[i].FromLocalFile)
					{
						// this var was created in the current cfg, append the new value at the end
						(*((vector<NLMISC::CConfigFile::CVar>*)(YYPARSE_PARAM)))[i].add(Var);

						if (Var.size() > 0 && Var.Callback != NULL)
							(Var.Callback)((*((vector<NLMISC::CConfigFile::CVar>*)(YYPARSE_PARAM)))[i]);
					}
					else
					{
						// this var has been created in a parent Cfg, append at the beginning of the array
						Var.add ((*((vector<NLMISC::CConfigFile::CVar>*)(YYPARSE_PARAM)))[i]);
						if (Var != (*((vector<NLMISC::CConfigFile::CVar>*)(YYPARSE_PARAM)))[i] && Var.Callback != NULL)
							(Var.Callback)(Var);
						(*((vector<NLMISC::CConfigFile::CVar>*)(YYPARSE_PARAM)))[i] = Var;
					}
				}

				cf_CurrentVar.IntValues.clear ();
				cf_CurrentVar.RealValues.clear ();
				cf_CurrentVar.StrValues.clear ();
				cf_CurrentVar.Comp = false;
				cf_CurrentVar.Type = NLMISC::CConfigFile::CVar::T_UNKNOWN;
			;
    break;}
case 8:
#line 245 "cf_gramatical.yxx"
{ yyval.Val = yyvsp[0].Val; cf_CurrentVar.Comp = false; DEBUG_PRINT("false\n"); ;
    break;}
case 9:
#line 246 "cf_gramatical.yxx"
{ yyval.Val = yyvsp[-1].Val; cf_CurrentVar.Comp = true; DEBUG_PRINT("true\n"); ;
    break;}
case 10:
#line 247 "cf_gramatical.yxx"
{ yyval.Val = yyvsp[-2].Val; cf_CurrentVar.Comp = true; DEBUG_PRINT("true\n"); ;
    break;}
case 11:
#line 248 "cf_gramatical.yxx"
{ yyval.Val = yyvsp[0].Val; cf_CurrentVar.Comp = true; DEBUG_PRINT("true\n"); ;
    break;}
case 12:
#line 251 "cf_gramatical.yxx"
{ yyval.Val = yyvsp[0].Val; /*cf_CurrentVar.Type = $1.Type;*/ cf_setVar (cf_CurrentVar, yyvsp[0].Val); ;
    break;}
case 13:
#line 252 "cf_gramatical.yxx"
{ yyval.Val = yyvsp[0].Val; /*cf_CurrentVar.Type = $3.Type;*/ cf_setVar (cf_CurrentVar, yyvsp[0].Val); ;
    break;}
case 14:
#line 255 "cf_gramatical.yxx"
{ yyval.Val = yyvsp[0].Val; ;
    break;}
case 15:
#line 256 "cf_gramatical.yxx"
{ yyval.Val = cf_op(yyvsp[-2].Val, yyvsp[0].Val, OP_PLUS); ;
    break;}
case 16:
#line 257 "cf_gramatical.yxx"
{ yyval.Val = cf_op(yyvsp[-2].Val, yyvsp[0].Val, OP_MINUS); ;
    break;}
case 17:
#line 260 "cf_gramatical.yxx"
{ yyval.Val = yyvsp[0].Val; ;
    break;}
case 18:
#line 261 "cf_gramatical.yxx"
{ yyval.Val = cf_op(yyvsp[-2].Val, yyvsp[0].Val, OP_MULT); ;
    break;}
case 19:
#line 262 "cf_gramatical.yxx"
{ yyval.Val = cf_op (yyvsp[-2].Val, yyvsp[0].Val, OP_DIVIDE); ;
    break;}
case 20:
#line 265 "cf_gramatical.yxx"
{ yyval.Val = yyvsp[0].Val; ;
    break;}
case 21:
#line 266 "cf_gramatical.yxx"
{ cf_value v; v.Type=NLMISC::CConfigFile::CVar::T_INT; /* just to avoid a warning, I affect 'v' with a dummy value */ yyval.Val = cf_op(yyvsp[0].Val,v,OP_NEG); ;
    break;}
case 22:
#line 267 "cf_gramatical.yxx"
{ yyval.Val = yyvsp[-1].Val; ;
    break;}
case 23:
#line 268 "cf_gramatical.yxx"
{ yyval.Val = yylval.Val; ;
    break;}
case 24:
#line 269 "cf_gramatical.yxx"
{ yyval.Val = yylval.Val; ;
    break;}
case 25:
#line 270 "cf_gramatical.yxx"
{ yyval.Val = yylval.Val; ;
    break;}
case 26:
#line 271 "cf_gramatical.yxx"
{ yyval.Val = yyvsp[0].Val; ;
    break;}
case 27:
#line 275 "cf_gramatical.yxx"
{
				DEBUG_PRINT("yacc: cont\n");
				bool ok=false;
				int i;
				for(i = 0; i < (int)((*((vector<NLMISC::CConfigFile::CVar>*)(YYPARSE_PARAM))).size()); i++)
				{
					if ((*((vector<NLMISC::CConfigFile::CVar>*)(YYPARSE_PARAM)))[i].Name == yyvsp[0].Val.String)
					{
						ok = true;
						break;
					}
				}
				if (ok)
				{
					cf_value Var;
					Var.Type = (*((vector<NLMISC::CConfigFile::CVar>*)(YYPARSE_PARAM)))[i].Type;
					DEBUG_PRINTF("vart %d\n", Var.Type);
					switch (Var.Type)
					{
					case NLMISC::CConfigFile::CVar::T_INT: Var.Int = (*((vector<NLMISC::CConfigFile::CVar>*)(YYPARSE_PARAM)))[i].IntValues[0]; break;
					case NLMISC::CConfigFile::CVar::T_REAL: Var.Real = (*((vector<NLMISC::CConfigFile::CVar>*)(YYPARSE_PARAM)))[i].RealValues[0]; break;
					case NLMISC::CConfigFile::CVar::T_STRING: strcpy (Var.String, (*((vector<NLMISC::CConfigFile::CVar>*)(YYPARSE_PARAM)))[i].StrValues[0].c_str()); break;
					default: DEBUG_PRINT("*** CAN T DO THAT!!!\n"); break;
					}
					yyval.Val = Var;
				}
				else
				{
					DEBUG_PRINT("var existe pas\n");
				}
			;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 543 "cfbison.simple"

  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YYLSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = yylloc.first_line;
      yylsp->first_column = yylloc.first_column;
      yylsp->last_line = (yylsp-1)->last_line;
      yylsp->last_column = (yylsp-1)->last_column;
      yylsp->text = 0;
    }
  else
    {
      yylsp->last_line = (yylsp+yylen-1)->last_line;
      yylsp->last_column = (yylsp+yylen-1)->last_column;
    }
#endif

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;

yyerrlab:   /* here on detecting error */

  if (! yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  int size = 0;
	  char *msg;
	  int x, count;

	  count = 0;
	  /* Start X at -yyn if nec to avoid negative indexes in yycheck.  */
	  for (x = (yyn < 0 ? -yyn : 0);
	       x < (sizeof(yytname) / sizeof(char *)); x++)
	    if (yycheck[x + yyn] == x)
	      size += strlen(yytname[x]) + 15, count++;
	  msg = (char *) malloc(size + 15);
	  if (msg != 0)
	    {
	      strcpy(msg, "parse error");

	      if (count < 5)
		{
		  count = 0;
		  for (x = (yyn < 0 ? -yyn : 0);
		       x < (sizeof(yytname) / sizeof(char *)); x++)
		    if (yycheck[x + yyn] == x)
		      {
			strcat(msg, count == 0 ? ", expecting `" : " or `");
			strcat(msg, yytname[x]);
			strcat(msg, "'");
			count++;
		      }
		}
	      yyerror(msg);
	      free(msg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror("parse error");
    }

  goto yyerrlab1;
yyerrlab1:   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Discarding token %d (%s).\n", yychar, yytname[yychar1]);
#endif

      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;

yyerrdefault:  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) goto yydefault;
#endif

yyerrpop:   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#ifdef YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

yyerrhandle:

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;

 yyacceptlab:
  /* YYACCEPT comes here.  */
  if (yyfree_stacks)
    {
      free (yyss);
      free (yyvs);
#ifdef YYLSP_NEEDED
      free (yyls);
#endif
    }
  return 0;

 yyabortlab:
  /* YYABORT comes here.  */
  if (yyfree_stacks)
    {
      free (yyss);
      free (yyvs);
#ifdef YYLSP_NEEDED
      free (yyls);
#endif
    }
  return 1;
}
#line 307 "cf_gramatical.yxx"


/* compute the good operation with a, b and op */
cf_value cf_op (cf_value a, cf_value b, cf_operation op)
{
	DEBUG_PRINTF("[OP:%d; ", op);
	cf_print(a);
	DEBUG_PRINT("; ");
	cf_print(b);
	DEBUG_PRINT("; ");

	switch (op)
	{
	case OP_MULT:																//  *********************
		switch (a.Type)
		{
		case NLMISC::CConfigFile::CVar::T_INT:
			switch (b.Type)
			{
			case NLMISC::CConfigFile::CVar::T_INT:		a.Int *= b.Int; break;
			case NLMISC::CConfigFile::CVar::T_REAL:	a.Int *= (int)b.Real; break;
			case NLMISC::CConfigFile::CVar::T_STRING:	DEBUG_PRINT("ERROR: int*str\n"); break;
			default: break;
			}
			break;
		case NLMISC::CConfigFile::CVar::T_REAL:
			switch (b.Type)
			{
			case NLMISC::CConfigFile::CVar::T_INT:		a.Real *= (double)b.Int; break;
			case NLMISC::CConfigFile::CVar::T_REAL:	a.Real *= b.Real; break;
			case NLMISC::CConfigFile::CVar::T_STRING:	DEBUG_PRINT("ERROR: real*str\n"); break;
			default: break;
			}
			break;
		case NLMISC::CConfigFile::CVar::T_STRING:
			switch (b.Type)
			{
			case NLMISC::CConfigFile::CVar::T_INT:		DEBUG_PRINT("ERROR: str*int\n");  break;
			case NLMISC::CConfigFile::CVar::T_REAL:	DEBUG_PRINT("ERROR: str*real\n");  break;
			case NLMISC::CConfigFile::CVar::T_STRING:	DEBUG_PRINT("ERROR: str*str\n");  break;
			default: break;
			}
			break;
		default: break;
		}
		break;
	case OP_DIVIDE:																//  //////////////////////
		switch (a.Type)
		{
		case NLMISC::CConfigFile::CVar::T_INT:
			switch (b.Type)
			{
			case NLMISC::CConfigFile::CVar::T_INT:		a.Int /= b.Int; break;
			case NLMISC::CConfigFile::CVar::T_REAL:	a.Int /= (int)b.Real; break;
			case NLMISC::CConfigFile::CVar::T_STRING:	DEBUG_PRINT("ERROR: int/str\n"); break;
			default: break;
			}
			break;
		case NLMISC::CConfigFile::CVar::T_REAL:
			switch (b.Type)
			{
			case NLMISC::CConfigFile::CVar::T_INT:		a.Real /= (double)b.Int; break;
			case NLMISC::CConfigFile::CVar::T_REAL:	a.Real /= b.Real; break;
			case NLMISC::CConfigFile::CVar::T_STRING:	DEBUG_PRINT("ERROR: real/str\n"); break;
			default: break;
			}
			break;
		case NLMISC::CConfigFile::CVar::T_STRING:
			switch (b.Type)
			{
			case NLMISC::CConfigFile::CVar::T_INT:		DEBUG_PRINT("ERROR: str/int\n"); break;
			case NLMISC::CConfigFile::CVar::T_REAL:	DEBUG_PRINT("ERROR: str/real\n"); break;
			case NLMISC::CConfigFile::CVar::T_STRING:	DEBUG_PRINT("ERROR: str/str\n"); break;
			 default: break;
			}
			break;
		default: break;
		}
		break;
	case OP_PLUS:																//  ++++++++++++++++++++++++
		switch (a.Type)
		{
		case NLMISC::CConfigFile::CVar::T_INT:
			switch (b.Type)
			{
			case NLMISC::CConfigFile::CVar::T_INT:	a.Int += b.Int; break;
			case NLMISC::CConfigFile::CVar::T_REAL:	a.Int += (int)b.Real; break;
			case NLMISC::CConfigFile::CVar::T_STRING:	a.Int += atoi(b.String); break;
			default: break;
			}
			break;
		case NLMISC::CConfigFile::CVar::T_REAL:
			switch (b.Type)
			{
			case NLMISC::CConfigFile::CVar::T_INT:	a.Real += (double)b.Int; break;
			case NLMISC::CConfigFile::CVar::T_REAL:	a.Real += b.Real; break;
			case NLMISC::CConfigFile::CVar::T_STRING:	a.Real += atof (b.String); break;
			default: break;
			}
			break;
		case NLMISC::CConfigFile::CVar::T_STRING:
			switch (b.Type)
			{
			case NLMISC::CConfigFile::CVar::T_INT:	{ char str2[60]; NLMISC::smprintf(str2, 60, "%d", b.Int); strcat(a.String, str2); break; }
			case NLMISC::CConfigFile::CVar::T_REAL:	{ char str2[60]; NLMISC::smprintf(str2, 60, "%f", b.Real); strcat(a.String, str2); break; }
			case NLMISC::CConfigFile::CVar::T_STRING:	strcat (a.String, b.String); break;
			default: break;
			}
			break;
		default: break;
		}
		break;
	case OP_MINUS:																//  -------------------------
		switch (a.Type)
		{
		case NLMISC::CConfigFile::CVar::T_INT:
			switch (b.Type)
			{
			case NLMISC::CConfigFile::CVar::T_INT:	a.Int -= b.Int; break;
			case NLMISC::CConfigFile::CVar::T_REAL:	a.Int -= (int)b.Real; break;
			case NLMISC::CConfigFile::CVar::T_STRING:	DEBUG_PRINT("ERROR: int-str\n"); break;
			default: break;
			}
			break;
		case NLMISC::CConfigFile::CVar::T_REAL:
			switch (b.Type)
			{
			case NLMISC::CConfigFile::CVar::T_INT:	a.Real -= (double)b.Int; break;
			case NLMISC::CConfigFile::CVar::T_REAL:	a.Real -= b.Real; break;
			case NLMISC::CConfigFile::CVar::T_STRING:	DEBUG_PRINT("ERROR: real-str\n"); break;
			default: break;
			}
			break;
		case NLMISC::CConfigFile::CVar::T_STRING:
			switch (b.Type)
			{
			case NLMISC::CConfigFile::CVar::T_INT:	DEBUG_PRINT("ERROR: str-int\n"); break;
			case NLMISC::CConfigFile::CVar::T_REAL:	DEBUG_PRINT("ERROR: str-real\n"); break;
			case NLMISC::CConfigFile::CVar::T_STRING:	DEBUG_PRINT("ERROR: str-str\n"); break;
			default: break;
			}
			break;
		default: break;
		}
		break;
	case OP_NEG:																// neg
		switch (a.Type)
		{
		case NLMISC::CConfigFile::CVar::T_INT:		a.Int = -a.Int; break;
		case NLMISC::CConfigFile::CVar::T_REAL:		a.Real = -a.Real; break;
		case NLMISC::CConfigFile::CVar::T_STRING:		DEBUG_PRINT("ERROR: -str\n"); break;
		default: break;
		}
		break;
	}
	cf_print(a);
	DEBUG_PRINT("]\n");
	return a;
}

/* print a value, it's only for debug purpose */
void cf_print (cf_value Val)
{
	switch (Val.Type)
	{
	case NLMISC::CConfigFile::CVar::T_INT:
		DEBUG_PRINTF("'%d'", Val.Int);
		break;
	case NLMISC::CConfigFile::CVar::T_REAL:
		DEBUG_PRINTF("`%f`", Val.Real);
		break;
	case NLMISC::CConfigFile::CVar::T_STRING:
		DEBUG_PRINTF("\"%s\"", Val.String);
		break;
	default: break;
	}
}

/* put a value into a var */
void cf_setVar (NLMISC::CConfigFile::CVar &Var, cf_value Val)
{
	DEBUG_PRINTF("Set var (type %d var name '%s') with new var type %d with value : ", Var.Type, Var.Name.c_str(), Val.Type);
	cf_print(Val);
	DEBUG_PRINTF("\n");
	Var.Root = LoadRoot;
	if (Var.Type == NLMISC::CConfigFile::CVar::T_UNKNOWN || Var.Type == Val.Type)
	{
		if (Var.Type == NLMISC::CConfigFile::CVar::T_UNKNOWN)
		{
			DEBUG_PRINTF("var type is unknown, set to the val type\n");
		}
		else
		{
			DEBUG_PRINTF("val type is same var type, just add\n");
		}

		Var.Type = Val.Type;
		switch (Val.Type)
		{
		case NLMISC::CConfigFile::CVar::T_INT: Var.IntValues.push_back (Val.Int); break;
		case NLMISC::CConfigFile::CVar::T_REAL: Var.RealValues.push_back (Val.Real); break;
		case NLMISC::CConfigFile::CVar::T_STRING: Var.StrValues.push_back(Val.String); break;
		default: break;
		}
	}
	else
	{
		// need to convert the type
		switch (Var.Type)
		{
		case NLMISC::CConfigFile::CVar::T_INT:
			switch (Val.Type)
			{
			case NLMISC::CConfigFile::CVar::T_REAL:		Var.IntValues.push_back ((int)Val.Real); break;
			case NLMISC::CConfigFile::CVar::T_STRING:	{ int val = 0; NLMISC::fromString(Val.String, val); Var.IntValues.push_back(val); break; }
			default: break;
			}
			break;
		case NLMISC::CConfigFile::CVar::T_REAL:
			switch (Val.Type)
			{
			case NLMISC::CConfigFile::CVar::T_INT:		Var.RealValues.push_back ((double)Val.Int); break;
			case NLMISC::CConfigFile::CVar::T_STRING:	{ double val = 0.0; NLMISC::fromString(Val.String, val); Var.RealValues.push_back(val); break; }
			default: break;
			}
			break;
		case NLMISC::CConfigFile::CVar::T_STRING:
			switch (Val.Type)
			{
			case NLMISC::CConfigFile::CVar::T_INT:	Var.StrValues.push_back(toString(Val.Int)); break;
			case NLMISC::CConfigFile::CVar::T_REAL:	Var.StrValues.push_back(toString(Val.Real)); break;
			default: break;
			}
			break;
		default: break;
		}
	}
}

int yyerror (const char *s)
{
	DEBUG_PRINTF("%s\n",s);
	return 1;
}


