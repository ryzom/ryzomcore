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
/* Line 1248 of yacc.c.  */
#line 110 "ai_service/script_parser_yacc.hpp"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE ailval;



