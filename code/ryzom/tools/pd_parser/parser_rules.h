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


#ifndef RY_PD_PARSE_RULES_H
#define RY_PD_PARSE_RULES_H

#include "parse_node.h"

#define DECLARE_PARSE(nodename) CParseNode	*parse##nodename(CTokenizer &tokenizer);


DECLARE_PARSE(Main)
DECLARE_PARSE(File)
DECLARE_PARSE(Include)
DECLARE_PARSE(UsePch)
DECLARE_PARSE(Db)
DECLARE_PARSE(CppCode)
DECLARE_PARSE(Type)
DECLARE_PARSE(Class)
DECLARE_PARSE(Declaration)
DECLARE_PARSE(Enum)
DECLARE_PARSE(EnumSimpleValue)
DECLARE_PARSE(EnumRange)
DECLARE_PARSE(Dimension)
DECLARE_PARSE(LogMsg)
DECLARE_PARSE(LogContext)



/*
 * Parser Help Macros
 */

/*
 * Start node parsing without optional description
 * PARSE_START_NO_DESCRIPTION(Main, CParseNode)
 *
 *	...
 *
 * PARSE_END
 */
#define PARSE_START_NO_DESCRIPTION(nodename, nodetype)	\
CParseNode	*parse##nodename(CTokenizer &tokenizer)	\
{	\
	tokenizer.push();	\
	nodetype	*main = new nodetype();	\
	main->StartToken = tokenizer.currentToken();	\
	CParseNode	*parsed;	\
	parsed = NULL;

#define PARSE_END	\
	return main;	\
}


/*
 * Start node parsing with optional description
 * PARSE_START(Main, CParseNode)
 *
 *	...
 *
 * PARSE_END
 */
#define PARSE_START(nodename, nodetype)	\
CParseNode	*parse##nodename(CTokenizer &tokenizer)	\
{	\
	tokenizer.push();	\
	nodetype	*main = new nodetype();	\
	main->StartToken = tokenizer.currentToken();	\
	CParseNode	*parsed;	\
	parsed = NULL;	\
	PARSE_OPT_KEYWORD_IN(Description, Description)


/*
 * Lock parsing to the current token. If parse fail later, this will cause
 * parser to generate an error after this token.
 * PARSE_KEYWORD(Colon)
 * PARSE_MARK
 */
#define PARSE_MARK	tokenizer.leaveMark();



/*
 * Force the parser to fail and leave current node
 */
#define PARSE_FAIL \
	{	\
		delete main;	\
		tokenizer.pop();	\
		return NULL;	\
	}

/*
 * Parse a given token
 * PARSE_KEYWORD(Extern)
 */
#define PARSE_KEYWORD(keyword) \
	{	\
		if (tokenizer.end() || tokenizer.current() != Token##keyword)	\
			PARSE_FAIL	\
		tokenizer.next();	\
	}

/*
 * Parse a token and store value into a string
 * PARSE_KEYWORD_IN_TEMP(Identifier, Name)
 */
#define PARSE_KEYWORD_IN_TEMP(keyword, temp) \
	{	\
		if (tokenizer.end() || tokenizer.current() != Token##keyword)	\
			PARSE_FAIL	\
		temp = tokenizer.get(tokenizer.currentToken());	\
		tokenizer.next();	\
	}

/*
 * Parse a token and store value into a node string attribute
 * PARSE_KEYWORD_IN(Identifier, Name) where Name is an attribute of the current node
 */
#define PARSE_KEYWORD_IN(keyword, savein) PARSE_KEYWORD_IN_TEMP(keyword, main->savein)

/*
 * Parse a token and push value into a vector of string of the current node
 * PARSE_ADD_KEYWORD_IN(Identifier, Names) where Names is a vector<string> of the node
 */
#define PARSE_ADD_KEYWORD_IN(keyword, savein) \
	{	\
		if (tokenizer.end() || tokenizer.current() != Token##keyword)	\
			PARSE_FAIL	\
		main->savein.push_back(tokenizer.get(tokenizer.currentToken()));	\
		tokenizer.next();	\
	}

/*
 * Parse a couple of 2 tokens and push them as string into a vector of pair of string of the current node
 * PARSE_ADD_2_KEYWORD_IN(Identifier, Identifier, Prototype) where Prototype is a vector<pair<string, string> > of the node
 */
#define PARSE_ADD_2_KEYWORD_IN(keyword1, keyword2, savein) \
	{	\
		if (tokenizer.end() || tokenizer.current() != Token##keyword1)	\
			PARSE_FAIL	\
		CTokenizer::CToken	t1 = tokenizer.currentToken();	\
		tokenizer.next();	\
		if (tokenizer.end() || tokenizer.current() != Token##keyword2)	\
			PARSE_FAIL	\
		CTokenizer::CToken	t2 = tokenizer.currentToken();	\
		tokenizer.next();	\
		main->savein.push_back(std::make_pair<std::string, std::string>(tokenizer.get(t1), tokenizer.get(t2)));	\
	}

/*
 * Parse a string into a attribute of the node
 * PARSE_STRING(FileName)
 */
#define PARSE_STRING(savein)		PARSE_KEYWORD_IN(String, savein)

/*
 * Parse a scoped identifier and store value into an attribute of the node
 * PARSE_SCOPE_IDF(ScopedType) where a scoped identifier may be of the form 'Identifier' or 'Identifier::Identifier'
 */
#define PARSE_SCOPE_IDF(savein)	\
	{	\
		if (tokenizer.end() || (tokenizer.current() != TokenIdentifier && tokenizer.current() != TokenScopedIdentifier))	\
			PARSE_FAIL	\
		main->savein = tokenizer.get(tokenizer.currentToken());	\
		tokenizer.next();	\
	}


/*
 * Parse a single Identifier into a node'a attribute
 * PARSE_IDENTIFIER(Name)
 */
#define PARSE_IDENTIFIER(savein)	PARSE_KEYWORD_IN(Identifier, savein)

/*
 * Parse a single Identifier and push it to a vector<string>
 * PARSE_ADD_IDENTIFIER(Names)
 */
#define PARSE_ADD_IDENTIFIER(savein)	PARSE_ADD_KEYWORD_IN(Identifier, savein)

/*
 * Parse a integer and store value (as int)
 * PARSE_INT(DefaultIntValue)
 */
#define PARSE_INT(savein)	\
	{	\
		if (tokenizer.end() || tokenizer.current() != TokenNumber)	\
			PARSE_FAIL	\
		NLMISC::fromString(tokenizer.get(tokenizer.currentToken()), main->savein);	\
		tokenizer.next();	\
	}

/*
 * Parse a value, which may be a string or a number (float/int), and store it as a string
 * PARSE_VALUE(DefaultValue)
 */
#define PARSE_VALUE(savein) \
	{	\
		if (tokenizer.end() || (tokenizer.current() != TokenIdentifier && tokenizer.current() != TokenNumber && tokenizer.current() != TokenString))	\
			PARSE_FAIL	\
		main->savein = tokenizer.get(tokenizer.currentToken());	\
		tokenizer.next();	\
	}




/*
 * Start a bloc of alternative bloc to examine. Parser will examine first alternative, and if parsing fails
 * it will examine the second alternative. Usually alternative bloc is ended by a PARSE_FAIL so parsing may examine
 * previous following rules...
 * PARSE_ALTERNATIVE(Include)
 * PARSE_ALTERNATIVE(Type)
 * PARSE_ALTERNATIVE(Class)
 * PARSE_FAIL					// parsing fails if neither Include can be parsed nor Type nor Class
 */
#define PARSE_ALTERNATIVE(nodename) \
	if ( (parsed = parse##nodename(tokenizer)) )	\
	{	\
		parsed->Parent = main;	\
		main->Nodes.push_back(parsed);	\
	}	\
	else

/*
 * End a bloc of alternative without failing, usually to allow parsing of optionnal node blocs
 */
#define PARSE_END_ALTERNATIVE() \
	{}

/*
 * Parse a node. This is not an alternative, if node parsing fails, current node parsing fails
 * PARSE_NODE(Include)
 */
#define PARSE_NODE(nodename, savein) \
	if ( (main->savein = parse##nodename(tokenizer)) )	\
	{	\
		main->savein->Parent = main;	\
	}	\
	else	\
		PARSE_FAIL




/*
 * Start a bloc with '{'
 */
#define PARSE_START_BLOC	\
	PARSE_KEYWORD(OpenBrace)	\
	while (!tokenizer.end() && tokenizer.current() != TokenCloseBrace)	\
	{

/*
 * End a bloc with '}'
 */
#define PARSE_END_BLOC	\
	}	\
	PARSE_KEYWORD(CloseBrace)



/*
 * Parse a list of tokens (unknown yet) separated by special tokens
 * PARSE_START_LIST(OpenParenthesis, CloseParenthesis)
 *   PARSE_ADD_KEYWORD_IN(Identifier, Names)
 * PARSE_END_LIST(CloseParenthesis, Comma)
 */
#define PARSE_START_LIST(starttoken, endtoken)	\
	PARSE_KEYWORD(starttoken)	\
	while (!tokenizer.end() && tokenizer.current() != Token##endtoken)	\
	{

#define PARSE_END_LIST(endtoken, separator)	\
		if (tokenizer.end() || tokenizer.current() != Token##separator)	\
			break;	\
		tokenizer.next();	\
	}	\
	PARSE_KEYWORD(endtoken)


/*
 * Parse an optionnal list, see also PARSE_START_LIST/PARSE_END_LIST
 */
#define PARSE_OPT_LIST(starttoken, endtoken)	\
	if (!tokenizer.end() && tokenizer.current() == Token##starttoken)	\
	{	\
		PARSE_START_LIST(starttoken, endtoken)

#define PARSE_END_OPT_LIST(endtoken, separator)	\
		PARSE_END_LIST(endtoken, separator)	\
	}


/*
 * Repeat parsing between PARSE_OPT_BEFORE and PARSE_END_OPT_BEFORE while next token is not reached
 * To be used with PARSE_OPT_IN
 * PARSE_OPT_BEFORE(Class)
 *     PARSE_OPT_IN(Mapped, MappedFlag);
 *     PARSE_OPT_IN(Derived, DerivedFlag);
 * PARSE_END_OPT_BEFORE()
 */
#define PARSE_OPT_BEFORE(token)	\
	while (!tokenizer.end() && tokenizer.current() != Token##token)	\
	{	\
		switch (tokenizer.current())	\
		{

#define PARSE_OPT_BEFORE_2(token1, token2)	\
	while (!tokenizer.end() && tokenizer.current() != Token##token1 && tokenizer.current() != Token##token2)	\
	{	\
		switch (tokenizer.current())	\
		{

#define PARSE_OPT_IN(token, flag)	\
		case Token##token:	\
			if (!main->flag)	\
			{	\
				main->flag = true;	\
			}	\
			else	\
			{	\
				PARSE_FAIL	\
			}	\
			break;

#define PARSE_END_OPT_BEFORE()	\
		default:	\
			PARSE_FAIL	\
			break;	\
		}	\
		tokenizer.next();	\
	}


/*
 * Parse a list of keywork separated by a given token, and store them into a vector<string>
 * see also PARSE_ADD_KEYWORD_IN
 */
#define PARSE_LIST_KEYWORDS(keyword, savein, separator)	\
	PARSE_ADD_KEYWORD_IN(keyword, savein)	\
	while (!tokenizer.end() && tokenizer.current() == Token##separator)	\
	{	\
		tokenizer.next();	\
		PARSE_ADD_KEYWORD_IN(keyword, savein)	\
	}

/*
 * PArse a list of couple of 2 keywords and store them into a vector<pair<string, string> >
 * see also PARSE_ADD_2_KEYWORD_IN
 */
#define PARSE_LIST_2_KEYWORDS(keyword1, keyword2, savein, separator)	\
	PARSE_ADD_2_KEYWORD_IN(keyword1, keyword2, savein)	\
	while (!tokenizer.end() && tokenizer.current() == Token##separator)	\
	{	\
		tokenizer.next();	\
		PARSE_ADD_2_KEYWORD_IN(keyword1, keyword2, savein)	\
	}

#define PARSE_LIST_IDENTIFIERS(starttoken, separator, endtoken, savein)	\
		PARSE_START_LIST(starttoken, endtoken)	\
			PARSE_ADD_IDENTIFIER(savein)	\
		PARSE_END_LIST(endtoken, separator)





#define PARSE_OPT(opttoken)	\
	if (!tokenizer.end() && tokenizer.current() == Token##opttoken)	\
	{	\
		tokenizer.next();	\

#define PARSE_NEXT_OPT(opttoken)	\
	}	\
	else if (!tokenizer.end() && tokenizer.current() == Token##opttoken)	\
	{	\
		tokenizer.next();	\

#define PARSE_LAST_OPT	\
	}	\
	else	\
	{

#define PARSE_END_OPT	\
	}



#define PARSE_OPT_KEYWORD_IN(keyword, savein) \
	{	\
		if (!tokenizer.end() && tokenizer.current() == Token##keyword)	\
		{	\
			main->savein = tokenizer.get(tokenizer.currentToken());	\
			tokenizer.next();	\
		}	\
	}

#endif
