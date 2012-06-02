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

#include "templatizer.h"


 
bool			isBlocStart(CTemplatizerParser t)	{ return (t.isValid() && t[0] == '{' && t[1] == '{'); }
bool			isBlocEnd(CTemplatizerParser t)	{ return (t.isValid() && t[0] == '}' && t[1] == '}'); }

enum TTemplatizerToken
{
	BlocStart,
	BlocEnd,
	OpenParenth,
	CloseParenth,
	Equal,
	Comma,
	Arobace,
	Dollar,
	Quote,
	CommentStart,
	CommentEnd,
	Identifier,
	ListIdentifier,

	Unknown
};

struct SToken
{
	TTemplatizerToken Token;
	const char* Text;
};

SToken SimpleTokens[] =
{
	{	BlocStart,		"{"		},
	{	BlocEnd,		"}"		},
	{	OpenParenth,	"("		},
	{	CloseParenth,	")"		},
	{	Equal,			"="		},
	{	Arobace,		"@"		},
	{	Dollar,			"$"		},
	{	Comma,			","		},
	{	Quote,			"\""	},
	{	CommentStart,	"/*"	},
	{	CommentEnd,		"*/"	},
};

CTemplatizerParser		skipSpace(CTemplatizerParser t)
{
	while (t.isValid() && *t != '\0' && isspace(*t))
		++t;
	return t;
}

const char*			skipSpace(const char* t)
{
	while (t != NULL && *t != '\0' && isspace(*t))
		++t;
	return t;
}

CTemplatizerParser	match(const char* keyword, CTemplatizerParser match)
{
	while (*keyword != '\0' && *match != '\0' && *keyword == *match)
	{
		++keyword;
		++match;
	}

	return *keyword == '\0' ? match : CTemplatizerParser();
}

TTemplatizerToken	getToken(CTemplatizerParser& t, bool skipspc, std::string* value = NULL)
{
	if (skipspc)
		t = skipSpace(t);

	if (!t.isValid())
		return Unknown;

	uint		i;
	CTemplatizerParser	result;
	for (i=0; i<sizeof(SimpleTokens)/sizeof(SimpleTokens[0]); ++i)
	{
		result = match(SimpleTokens[i].Text, t);
		if (result.isValid())
		{
			t = result;
			return SimpleTokens[i].Token;
		}
	}

	if (isalpha(*t))
	{
		TTemplatizerToken	retToken = Identifier;
		
		if (value != NULL)
			*value = "";
		do
		{
			while (isalpha(*t))
			{
				if (value != NULL)
					*value += *t;
				++t;
			}

			t = skipSpace(t);

			if (*t != EnvSeparator)
				break;

			retToken = ListIdentifier;

			if (value != NULL)
				*value += *t;
			++t;
		}
		while (true);

		return retToken;
	}

	return Unknown;
}

bool	popToken(CTemplatizerParser& t, TTemplatizerToken token, bool skipspc, std::string* value = NULL)
{
	CTemplatizerParser	save = t;
	if (getToken(save, skipspc, value) == token)
	{
		t = save;
		return true;
	}

	return false;
}

bool	isNextToken(CTemplatizerParser t, TTemplatizerToken token, bool skipspc, std::string* value = NULL)
{
	return getToken(t, skipspc, value) == token;
}

 
 
 
/*
 * Destructor
 */
CTemplatizerEnv::~CTemplatizerEnv()
{
	clear();
}

// Clear Env
void	CTemplatizerEnv::clear()
{
	TEnvMap::iterator	ite;
	for (ite=Envs.begin(); ite!=Envs.end(); ++ite)
		delete (*ite).second;

	TValueMap::iterator	itv;
	for (itv=Values.begin(); itv!=Values.end(); ++itv)
		delete (*itv).second;

	Envs.clear();
	Values.clear();
}


/*
 * Evaluate string (string replacement)
 */
std::string	CTemplatizerEnv::eval(const std::string& text)
{
	std::string	res;
	const char*	ptr = text.c_str();

	while (*ptr != '\0')
	{
		if (ptr[0] == '$' && ptr[1] == '(')
		{
			ptr += 2;
			std::string	var;

			ptr = skipSpace(ptr);

			while (isalpha(*ptr) || *ptr == '/' || *ptr == '.')
				var += *(ptr++);

			while (*ptr != '\0' && *ptr != ')')
				++ptr;
			if (*ptr == ')')
				++ptr;

			res += get(var);
		}
		else if (*ptr != '\r')
		{
			res += *(ptr++);
		}
	}

	return res;
}


/*
 * Constructor
 */
ITemplatizerBloc::ITemplatizerBloc()
{
}

/*
 * Destructor
 */
ITemplatizerBloc::~ITemplatizerBloc()
{
	uint	i;
	for (i=0; i<Blocs.size(); ++i)
		delete Blocs[i];

	TParamMap::iterator	it;
	for (it=Params.begin(); it!=Params.end(); ++it)
		delete (*it).second;
}


/*
 * Constructor
 */
CTemplatizer::CTemplatizer()
{
	RootBloc = NULL;
	RootEnv = NULL;
}


/*
 * Destructor
 */
CTemplatizer::~CTemplatizer()
{
	if (RootBloc != NULL)
		delete RootBloc;
	if (RootEnv != NULL)
		delete RootEnv;
}




/*
 * Build templatizer from text
 */
bool	CTemplatizer::build(const char* text)
{
	CTemplatizerParser	parser(text);
	RootBloc = ITemplatizerBloc::parseBloc(parser);
	RootEnv = new CTemplatizerEnv(NULL);

	return (RootBloc != NULL);
}


/**
 * Evaluate template and render to string
 */
std::string	CTemplatizer::eval()
{
	if (RootBloc != NULL && RootEnv != NULL)
		return RootBloc->eval(RootEnv);
	else
		return "";
}








/*
 * Parse bloc
 */
ITemplatizerBloc*	ITemplatizerBloc::parseBloc(CTemplatizerParser& ptr)
{
	std::string			blocType;
	ITemplatizerBloc*	bloc = NULL;

	if (popToken(ptr, Identifier, true, &blocType) || popToken(ptr, ListIdentifier, true, &blocType))
	{
		if		(blocType == "root")				bloc = new CTemplatizerRootBloc();
		else if (blocType == "sub")					bloc = new CTemplatizerSubBloc();
		else if (blocType == "loop")				bloc = new CTemplatizerLoopBloc();
		else if (blocType == "ifdefenv")			bloc = new CTemplatizerIfDefEnvBloc();
		else if (blocType == "ifdef")				bloc = new CTemplatizerIfDefBloc();
		else if (blocType == "ifnotdefenv")			bloc = new CTemplatizerIfNotDefEnvBloc();
		else if (blocType == "ifnotdef")			bloc = new CTemplatizerIfNotDefBloc();
		else if (blocType == "switch")				bloc = new CTemplatizerSwitchBloc();
		else if (blocType == "file")				bloc = new CTemplatizerFileBloc();
		else if (blocType == "set")					bloc = new CTemplatizerSetBloc();
		else if (blocType == "append")				bloc = new CTemplatizerAppendBloc();
		else if (blocType == "define")				bloc = new CTemplatizerDefineBloc();
		else if (blocType == "if")					bloc = new CTemplatizerIfBloc();
		else if (blocType == "ifnot")				bloc = new CTemplatizerIfNotBloc();
		else if (blocType == "join")				bloc = new CTemplatizerJoinBloc();
		else if (blocType == "class")				bloc = new CTemplatizerClassBloc();
		else if (blocType == "object")				bloc = new CTemplatizerObjectBloc();
		else if (blocType == "ref")					bloc = new CTemplatizerReferenceBloc();
		else if (blocType == "refenv")				bloc = new CTemplatizerRefEnvBloc();
		else if (blocType == "breakpoint")			bloc = new CTemplatizerBreakpointBloc();
		else										bloc = new CTemplatizerUserFunctionBloc(blocType);

		if (bloc == NULL)
		{
			nlwarning("Templatizer: failed to decode bloc '%s' at line %d", blocType.c_str(), ptr.getLine());
			return NULL;
		}

		ptr = bloc->parseHeader(ptr);

		if (!ptr.isValid())
		{
			nlwarning("Templatizer: failed to decode header of bloc '%s' at line %d", blocType.c_str(), ptr.getLine());
			delete bloc;
			return NULL;
		}

		if (bloc->hasInternal())
		{
			if (!popToken(ptr, BlocStart, true))
			{
				nlwarning("Templatizer: failed to decode start of bloc '%s' at line %d", blocType.c_str(), ptr.getLine());
				delete bloc;
				return NULL;
			}

			ptr = bloc->parseInternal(ptr);

			if (!ptr.isValid())
			{
				nlwarning("Templatizer: failed to parse bloc '%s' at line %d", blocType.c_str(), ptr.getLine());
				delete bloc;
				return NULL;
			}

			if (!popToken(ptr, BlocEnd, true))
			{
				nlwarning("Templatizer: failed to decode end of bloc '%s' at line %d", blocType.c_str(), ptr.getLine());
				delete bloc;
				return NULL;
			}
		}
	}
	else if (isNextToken(ptr, CommentStart, true))
	{
		bloc = new CTemplatizerCommentBloc();
		ptr = bloc->parseInternal(ptr);
		if (!ptr.isValid())
			nlwarning("Templatizer: failed to parse bloc 'Comment' at line %d", ptr.getLine());
	}
	else if (isNextToken(ptr, Quote, true))
	{
		bloc = new CTemplatizerTextBloc();
		ptr = bloc->parseInternal(ptr);
		if (!ptr.isValid())
			nlwarning("Templatizer: failed to parse bloc 'Text' at line %d", ptr.getLine());
	}

	if (!ptr.isValid())
	{
		delete bloc;
		return NULL;
	}

	return bloc;
}

/*
 * Parse bloc header
 */
CTemplatizerParser	ITemplatizerBloc::parseHeader(CTemplatizerParser ptr)
{
	if (popToken(ptr, OpenParenth, true))
	{
		uint			currentDefArg = 0;
		const char**	args = getDefParamList();

		if (popToken(ptr, CloseParenth, true))
			return ptr;

		do
		{
			std::string	paramName;
			if (!popToken(ptr, Identifier, true, &paramName))
			{
				if (args == NULL || args[currentDefArg] == NULL)
				{
					ptr.invalidate();
					return ptr;
				}

				paramName = args[currentDefArg++];
			}
			else
			{
				if (!popToken(ptr, Equal, true))
				{
					ptr.invalidate();
					return ptr;
				}
			}

			ITemplatizerBloc*	bloc = parseBloc(ptr);

			if (bloc == NULL)
			{
				ptr.invalidate();
				return ptr;
			}

			Params[paramName] = bloc;

			if (!popToken(ptr, Comma, true))
				break;
		}
		while (true);

		if (!popToken(ptr, CloseParenth, true))
		{
			ptr.invalidate();
			return ptr;
		}
	}

	return ptr;
}


/*
 * Parse bloc internal data
 */
CTemplatizerParser	ITemplatizerBloc::parseInternal(CTemplatizerParser ptr)
{
	ITemplatizerBloc*	bloc = NULL;
	do
	{
		bloc = parseBloc(ptr);

		if (bloc != NULL)
			Blocs.push_back(bloc);
	}
	while (bloc != NULL && *ptr != '\0' && !isBlocEnd(ptr));

	return ptr;
}


/*
 * Parse bloc internal data
 */
CTemplatizerParser	CTemplatizerTextBloc::parseInternal(CTemplatizerParser ptr)
{
	Text = "";

	ptr = skipSpace(ptr);

	if (*ptr != '"')
		return CTemplatizerParser();

	++ptr;

	while (*ptr != '\0' && *ptr != '"')
	{
		if (*ptr == '\\' && ptr[1] != '\0')
		{
			++ptr;
			switch (*ptr)
			{
			case 'n':
				Text += '\n';
				break;
			case 't':
				Text += '\t';
				break;
			case '\r':
				++ptr;
				if (*ptr == '\n')
					++ptr;
				break;
			case '\n':
				++ptr;
				if (*ptr == '\r')
					++ptr;
				break;
			default:
				Text += *ptr;
				break;
			}
			++ptr;
		}
		else
		{
			if (*ptr != '\r')
				Text += *ptr;
			++ptr;
		}
	}
	if (*ptr != '"')
		return CTemplatizerParser();

	return ++ptr;
}


/*
 * Parse bloc internal data
 */
CTemplatizerParser	CTemplatizerCommentBloc::parseInternal(CTemplatizerParser ptr)
{
	ptr = skipSpace(ptr);

	if (!popToken(ptr, CommentStart, true))
		return NULL;

	while (*ptr != '\0' && !isNextToken(ptr, CommentEnd, false))
		++ptr;

	if (!popToken(ptr, CommentEnd, false))
	{
		ptr.invalidate();
		return ptr;
	}

	return ptr;
}

