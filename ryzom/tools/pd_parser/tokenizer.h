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


#ifndef RY_PD_TOKENIZER_H
#define RY_PD_TOKENIZER_H

// Nel Misc
#include "nel/misc/types_nl.h"
#include <nel/misc/debug.h>
#include <nel/misc/file.h>

// STL
#include <string>
#include <map>
#include <string.h>
#include <ctype.h>


enum TToken
{
	TokenUnknown,
	TokenEOF,

	TokenIncludeScript,
	
	TokenIdentifier,
	TokenScopedIdentifier,
	TokenRefineScope,
	TokenNumber,
	TokenString,
	TokenCppCode,
	TokenSlash,
	TokenDescription,

	TokenOpenBrace,
	TokenCloseBrace,
	TokenOpenParenthesis,
	TokenCloseParenthesis,
	TokenOpenBracket,
	TokenCloseBracket,
	TokenLessThan,
	TokenGreaterThan,
	TokenEqual,
	TokenComma,
	TokenDot,
	TokenColon,
	TokenSemiColon,
	TokenMinus,
	TokenPlus,
	TokenTimes,
	TokenAntiSlash,
	TokenMod,
	TokenSharp,
	TokenAnd,
	TokenOr,
	TokenCirc,
	TokenInterrog,
	TokenExclam,

	TokenClass,
	TokenEnum,
	TokenDimension,
	TokenParent,
	TokenFlag,
	TokenFile,
	TokenDb,
	TokenType,
	TokenKey,
	TokenHidden,
	TokenExtern,
	TokenMirrored,
	TokenImplements,
	TokenMapped,
	TokenDerived,
	TokenInitFill,
	TokenReserve,
	TokenInclude,
	TokenWriteTrigger,
	TokenSeparated,

	TokenUsePch,

	TokenLogMsg,
	TokenLogContext,
};


class CTokenizer
{
public:

	class CToken
	{
	public:
		CToken(uint start = 0, uint end = 0, TToken token = TokenUnknown, CTokenizer* tokenizer = NULL) : Tokenizer(tokenizer), Start(start), End(end), Token(token) 			{}
		CToken(CTokenizer* tokenizer) : Tokenizer(tokenizer), Start(0), End(0), Token(TokenUnknown) 	{}

		CTokenizer*	Tokenizer;
		uint		Start;
		uint		End;
		TToken		Token;

		std::string	get() const		{ return Tokenizer->get(*this); }
	};

	CTokenizer() : _Buffer(NULL), _Size(0), _CurrentToken(0)	{ }

	CTokenizer(const std::string &text)
	{
		init(text);
	}

	~CTokenizer()
	{
		clear();
	}

	/// Init
	void	readFile(const std::string &filename)
	{
		clear();

		NLMISC::CIFile	f;
		if (!f.open(filename))
			return;

		uint	size = f.getFileSize();
		char	*buffer = new char[size+1];
		f.serialBuffer((uint8*)buffer, size);
		buffer[size] = '\0';

		f.close();

		_File = filename;

		init(std::string(buffer));

		delete buffer;
	}

	/// Init
	void	init(const std::string &text)
	{
		_Str = text;
		init(_Str.c_str());
	}

	/// Init
	void	init(const char* text, uint size = 0)
	{
		_Size = (size > 0 ? size : (uint)strlen(text));
		_Buffer = text;
		nlassert(_Buffer != NULL);
		_TempToken.Start = 0;
		_TempToken.End = 0;
		_CurrentToken = 0;
		_Mark = 0;

		initOneLetterTokens();
		initKeywords();
	}

	///
	bool	tokenize()
	{
		while (true)
		{
			CToken	token = nextToken();

			nlassert(token.Tokenizer != NULL);

			if (token.Token == TokenUnknown)
				return false;

			if (token.Token == TokenEOF)
				break;

			if (token.Token == TokenIncludeScript)
			{
				CToken	scriptFile = nextToken();
				if (scriptFile.Token != TokenString)
					error(scriptFile);

				std::string	file = scriptFile.get();

				CTokenizer*	sub = new CTokenizer();
				_Includes.push_back(sub);
				sub->readFile(file);
				if (!sub->tokenize())
					return false;
				_Tokens.insert(_Tokens.end(), sub->_Tokens.begin(), sub->_Tokens.end());
			}
			else
			{
				_Tokens.push_back(token);
			}
		}

		std::vector<CToken>::iterator	it;
		for (it=_Tokens.begin(); it!=_Tokens.end(); ++it)
		{
			if ((*it).Token == TokenIdentifier)
			{
				std::vector<CToken>::iterator	startit = it;
				std::vector<CToken>::iterator	endit = it;
				while ((++it) != _Tokens.end() &&
					   (*it).Token == TokenRefineScope &&
					   (*it).Tokenizer == (*startit).Tokenizer &&
					   (++it) != _Tokens.end() &&
					   (*it).Token == TokenIdentifier &&
					   (*it).Tokenizer == (*startit).Tokenizer)
				{
					endit = it;
				}
				if (endit != startit)
				{
					CToken	newToken(this);
					newToken.Token = TokenScopedIdentifier;
					newToken.Start = (*startit).Start;
					newToken.End = (*endit).End;
					++endit;
					it = _Tokens.erase(startit, endit);
					it = _Tokens.insert(it, newToken);
				}
			}
		}

		return true;
	}

	///
	bool	next()
	{
		++_CurrentToken;
		return (!end());
	}

	///
	const CToken	&currentToken() const
	{
		nlassert(!end());
		return _Tokens[_CurrentToken];
	}

	///
	TToken	current() const
	{
		return currentToken().Token;
	}

	///
	bool	end() const
	{
		return _CurrentToken >= _Tokens.size();
	}

	///
	void	push()
	{
		_Stack.push_back(_CurrentToken);
	}

	///
	void	pop()
	{
		nlassert(!_Stack.empty());
		_CurrentToken = _Stack.back();
		if (_CurrentToken < _Mark)
			error(_Tokens[_Mark], "parse");
		_Stack.pop_back();
	}

	///
	void	leaveMark()
	{
		_Mark = _CurrentToken;
	}

	/// get token text
	std::string	get(const CToken &token) const
	{
		nlassert(token.Tokenizer != NULL);

		if (token.Tokenizer != this)
			return token.Tokenizer->get(token);

		std::string		str(_Buffer+token.Start, token.End-token.Start);
		if (token.Token == TokenString)
		{
			std::string::size_type	pos = 0;
			while ((pos = str.find('\\', pos)) != std::string::npos)
			{
				if (pos+1 == str.size())
					break;
				switch (str[pos+1])
				{
				case 'n':
					str.erase(pos, 2);
					str.insert(pos, "\n");
					break;
				case 'r':
					str.erase(pos, 2);
					str.insert(pos, "\r");
					break;
				case 't':
					str.erase(pos, 2);
					str.insert(pos, "\t");
					break;
				default:
					str.erase(pos, 1);
					++pos;
					break;
				}
			}
		}
		return str;
	}

	/// get file, line
	void	getFileLine(const CToken& token, uint &line, uint &col, std::string &file)
	{
		nlassert(token.Tokenizer != NULL);

		if (token.Tokenizer != this)
		{
			token.Tokenizer->getFileLine(token, line, col, file);
			return;
		}

		file = _File;

		uint	n = 0;
		uint	pos = token.Start;

		line = 1;
		col = 1;

		while (n < pos)
		{
			if (_Buffer[n] == '\0')
				break;
			if (_Buffer[n] == '\t')
				col += 4;
			else if (_Buffer[n] != '\r')
				++col;
			if (_Buffer[n] == '\n')
				++line, col = 1;

			++n;
		}
	}

	/// error at
	void	error(const CToken& token, const char *errType = "syntax", const char *errMsg = NULL)
	{
		if (token.Tokenizer != this && token.Tokenizer != NULL)
		{
			token.Tokenizer->error(token, errType, errMsg);
			return;
		}

		uint	pos = token.Start;
		uint	n = 0;
		uint	line = 1, col = 1;
		uint	lineStartAt = 0, lineEndAt;

		while (n < pos)
		{
			if (_Buffer[n] == '\0')
				break;
			if (_Buffer[n] == '\t')
				col += 4;
			else if (_Buffer[n] != '\r')
				++col;
			else
				lineStartAt = n+1;
			if (_Buffer[n] == '\n')
				++line, col = 1, lineStartAt = n+1;

			++n;
		}

		lineEndAt = n;
		while (_Buffer[lineEndAt] != '\0' && _Buffer[lineEndAt] != '\n' && _Buffer[lineEndAt] != '\r')
			++lineEndAt;

		NLMISC::createDebug ();

		std::string	errorMsg = NLMISC::toString("PD_PARSE: file %s, %s error at line %d, column %d%s%s", _File.c_str(), errType, line, col, (errMsg != NULL ? ": " : ""), (errMsg != NULL ? errMsg : ""));

		NLMISC::ErrorLog->displayRawNL("%s", errorMsg.c_str());
		std::string	extr(_Buffer+lineStartAt, lineEndAt-lineStartAt);
		NLMISC::ErrorLog->displayRawNL("%s", extr.c_str());
		uint	i;
		for (i=0; i<extr.size() && i<n-lineStartAt; ++i)
			if (extr[i] != '\t')
				extr[i] = ' ';
		extr.erase(n-lineStartAt);
		extr += '^';
		NLMISC::ErrorLog->displayRawNL("%s", extr.c_str());
		nlerror("%s", errorMsg.c_str());
	}

private:

	/// Original text
	std::string						_Str;

	/// Parsed buffer
	const char						*_Buffer;

	/// Buffer size
	uint							_Size;

	/// Keywords
	std::map<std::string, TToken>	_Keywords;

	/// One letter tokens
	TToken							_OneLetterTokens[256];

	/// List of tokens
	std::vector<CToken>				_Tokens;

	/// Current token
	uint							_CurrentToken;

	/// State stack
	std::vector<uint>				_Stack;

	/// Currently used token
	CToken							_TempToken;

	/// Mark
	uint							_Mark;

	/// Loaded file
	std::string						_File;

	/// Subtokenizers
	std::vector<CTokenizer*>		_Includes;


	void	clear()
	{
		_Str.clear();
		_Size = 0;
		_TempToken.Start = 0;
		_TempToken.End = 0;
		_Buffer = NULL;
		_Mark = 0;
		_File.clear();
	}


	CToken	nextToken()
	{
		skipSpaces();
		if (posAtEnd())
			return CToken(0, 0, TokenEOF, this);

		_TempToken.Start = _TempToken.End;
		_TempToken.Token = TokenUnknown;
		_TempToken.Tokenizer = this;

		char	parse = popChar();

		if (isalpha(parse) || parse == '_')
		{
			// identifier
			while (!posAtEnd() && (isalnum(parse = getChar()) || parse == '_'))
				popChar();

			std::map<std::string, TToken>::iterator	it;
			_TempToken.Token = ((it = _Keywords.find(get(_TempToken))) != _Keywords.end() ? (*it).second : TokenIdentifier);
		}
		else if (isdigit(parse))
		{
			// number
			while (!posAtEnd() && isdigit(getChar()))
				popChar();

			_TempToken.Token = TokenNumber;
		}
		else if (parse == '"')
		{
			// string
			do
			{
				if (posAtEnd())
					error(_TempToken);

				parse = popChar();
				if (parse == '"')
					break;

				if (parse == '\\')
				{
					if (posAtEnd())
						error(_TempToken);
					parse = popChar();
				}
			}
			while (true);
			_TempToken.Token = TokenString;
			return CToken(_TempToken.Start+1, _TempToken.End-1, _TempToken.Token, this);
		}
		else if (parse == '@')
		{
			if (posAtEnd())
				error(_TempToken);

			parse = popChar();

			if (parse == '[')
			{
				// user code
				do
				{
					if (posAtEnd())
						error(_TempToken);

					parse = popChar();
					if (parse == ']')
					{
						if (posAtEnd())
							error(_TempToken);
						if (popChar() == '@')
							break;
						else
							pushChar();
					}
				}
				while (true);

				uint	startTrim = _TempToken.Start+2;
				uint	endTrim = _TempToken.End-2;

				while (startTrim < endTrim && (isspace(_Buffer[startTrim]) || _Buffer[startTrim] == '\r'))
					++startTrim;
				while (startTrim < endTrim && (isspace(_Buffer[endTrim-1]) || _Buffer[endTrim-1] == '\r'))
					--endTrim;

				_TempToken.Token = TokenCppCode;
				return CToken(startTrim, endTrim, _TempToken.Token, this);
			}
			else if (parse == '/')
			{
				// description
				do
				{
					if (posAtEnd())
						error(_TempToken);

					parse = popChar();
					if (parse == '/')
					{
						if (posAtEnd())
							error(_TempToken);
						if (popChar() == '@')
							break;
						else
							pushChar();
					}
				}
				while (true);

				uint	startTrim = _TempToken.Start+2;
				uint	endTrim = _TempToken.End-2;

				while (startTrim < endTrim && (isspace(_Buffer[startTrim]) || _Buffer[startTrim] == '\r'))
					++startTrim;
				while (startTrim < endTrim && (isspace(_Buffer[endTrim-1]) || _Buffer[endTrim-1] == '\r'))
					--endTrim;

				_TempToken.Token = TokenDescription;
				return CToken(startTrim, endTrim, _TempToken.Token, this);
			}
			else
			{
				error(_TempToken);
			}
		}
		else if (parse == '/')
		{
			_TempToken.Token = TokenSlash;
			if (!posAtEnd())
			{
				parse = popChar();
				if (parse == '/')
				{
					// skip to end of line
					while (!posAtEnd() && (parse = popChar()) != '\n' && parse != '\r')
						;
					return nextToken();
				}
				else if (parse == '*')
				{
					// skip to comment close
					while (true)
					{
						if (posAtEnd())
							error(_TempToken);

						parse = popChar();
						if (parse == '*')
						{
							if (posAtEnd())
								error(_TempToken);
							if (popChar() == '/')
								break;
							else
								pushChar();
						}
					}
					return nextToken();
				}
				else
				{
					pushChar();
				}
			}
		}
		else if (parse == ':')
		{
			if (posAtEnd())
				_TempToken.Token = TokenColon;
			else
			{
				parse = popChar();
				if (parse == ':')
				{
					_TempToken.Token = TokenRefineScope;
				}
				else
				{
					pushChar();
					_TempToken.Token = TokenColon;
				}
			}
		}
		else if (getOLToken(parse) != TokenUnknown)
			_TempToken.Token = getOLToken(parse);

		if (_TempToken.Token == TokenUnknown)
			error(_TempToken);

		return _TempToken;
	}


	bool	posAtEnd() const
	{
		return _TempToken.End >= _Size;
	}

	/// reset the buffer
	void	reset()
	{
		_TempToken.End = 0;
	}

	/// skip spaces
	void	skipSpaces()
	{
		while (!posAtEnd() && isspace(_Buffer[_TempToken.End]))
			++(_TempToken.End);
	}

	/// pop char
	char	popChar()
	{
		nlassert(!posAtEnd());
		return _Buffer[(_TempToken.End)++];
	}

	/// get char
	char	getChar()
	{
		nlassert(!posAtEnd());
		return _Buffer[_TempToken.End];
	}

	/// push char
	void	pushChar()
	{
		nlassert(_TempToken.End > 0);
		--(_TempToken.End);
	}

	/// init one letter tokens
	void	initOneLetterTokens()
	{
		uint	i;
		for (i=0; i<256; ++i)
			_OneLetterTokens[i] = TokenUnknown;

		setOLToken('{', TokenOpenBrace);
		setOLToken('}', TokenCloseBrace);
		setOLToken('(', TokenOpenParenthesis);
		setOLToken(')', TokenCloseParenthesis);
		setOLToken('[', TokenOpenBracket);
		setOLToken(']', TokenCloseBracket);
		setOLToken('<', TokenLessThan);
		setOLToken('>', TokenGreaterThan);
		setOLToken('=', TokenEqual);
		setOLToken(',', TokenComma);
		setOLToken('.', TokenDot);
		setOLToken(':', TokenColon);
		setOLToken(';', TokenSemiColon);
		setOLToken('-', TokenMinus);
		setOLToken('+', TokenPlus);
		setOLToken('*', TokenTimes);
		setOLToken('\\', TokenAntiSlash);
		setOLToken('%', TokenMod);
		setOLToken('#', TokenSharp);
		setOLToken('&', TokenAnd);
		setOLToken('|', TokenOr);
		setOLToken('^', TokenCirc);
		setOLToken('?', TokenInterrog);
		setOLToken('!', TokenExclam);
	}

	/// set one letter token
	void	setOLToken(char c, TToken token)
	{
		_OneLetterTokens[(uint)c] = token;
	}

	/// set one letter token
	TToken	getOLToken(char c) const
	{
		return _OneLetterTokens[(uint)c];
	}

	/// init keywords
	void	initKeywords()
	{
		_Keywords.clear();

		_Keywords["verbatim"] = TokenIncludeScript;

		_Keywords["class"] = TokenClass;
		_Keywords["enum"] = TokenEnum;
		_Keywords["dimension"] = TokenDimension;
		_Keywords["parent"] = TokenParent;
		_Keywords["flag"] = TokenFlag;
		_Keywords["file"] = TokenFile;
		_Keywords["db"] = TokenDb;
		_Keywords["type"] = TokenType;
		_Keywords["key"] = TokenKey;
		_Keywords["hidden"] = TokenHidden;
		_Keywords["extern"] = TokenExtern;
		_Keywords["mirrored"] = TokenMirrored;
		_Keywords["implements"] = TokenImplements;
		_Keywords["mapped"] = TokenMapped;
		_Keywords["derived"] = TokenDerived;
		_Keywords["initfill"] = TokenInitFill;
		_Keywords["logmsg"] = TokenLogMsg;
		_Keywords["logcontext"] = TokenLogContext;
		_Keywords["reserve"] = TokenReserve;
		_Keywords["include"] = TokenInclude;
		_Keywords["usepch"] = TokenUsePch;
		_Keywords["writetriggered"] = TokenWriteTrigger;
		_Keywords["separated"] = TokenSeparated;
	}
};


#endif
