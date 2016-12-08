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


#include "log_query.h"
#include <time.h>

using namespace std;
using namespace NLMISC;


bool ConversionTable[LGS::TSupportedParamType::nb_enum_items][LGS::TSupportedParamType::nb_enum_items] =
{
//	the type of the log param->	
//						spt_uint32,	spt_uint64,	spt_sint32,	spt_float,	spt_string,	spt_entityId,	spt_sheetId,	spt_itemId,
	/* v The type of the query param v*/
	/*spt_uint32*/	{	true,		true,		true,		true,		true,		false,			true,			false	},
	/*spt_uint64*/	{	true,		true,		true,		true,		true,		true,			false,			true	},
	/*spt_sint32*/	{	true,		true,		true,		true,		true,		false,			false,			false	},
	/*spt_float*/	{	true,		true,		true,		true,		true,		false,			false,			false	},
	/*spt_string*/	{	true,		true,		true,		true,		true,		true,			true,			true	},
	/*spt_entityId*/{	false,		false,		false,		false,		true,		true,			false,			false	},
	/*spt_sheetId*/	{	false,		false,		false,		false,		true,		false,			true,			false	},
	/*spt_itemId*/	{	false,		false,		false,		false,		true,		false,			false,				true	},

};



CQueryParser::CQueryParser(const TLogDefinitions &logDefs)
	:	_LogDefs(logDefs)
{
	// init the keywords table
	_Keywords.insert(std::make_pair(std::string("or"), tt_or));
	_Keywords.insert(std::make_pair(std::string("and"), tt_and));
	_Keywords.insert(std::make_pair(std::string("like"), tt_like));
	_Keywords.insert(std::make_pair(std::string("full_context"), tt_full_context));
	_Keywords.insert(std::make_pair(std::string("output_prefix"), tt_output_prefix));
	_Keywords.insert(std::make_pair(std::string("yesterday"), tt_yesterday));
	_Keywords.insert(std::make_pair(std::string("secs"), tt_secs));
	_Keywords.insert(std::make_pair(std::string("mins"), tt_mins));
	_Keywords.insert(std::make_pair(std::string("hours"), tt_hours));
	_Keywords.insert(std::make_pair(std::string("days"), tt_days));
	_Keywords.insert(std::make_pair(std::string("weeks"), tt_weeks));
	_Keywords.insert(std::make_pair(std::string("months"), tt_months));
	_Keywords.insert(std::make_pair(std::string("years"), tt_years));
}

LGS::TSupportedParamType CQueryParser::parseParamType(const std::string &typeName)
{
	if (typeName == "uint32")
		return LGS::TSupportedParamType::spt_uint32;
	else if (typeName == "uint64")
		return LGS::TSupportedParamType::spt_uint64;
	else if (typeName == "sint32")
		return LGS::TSupportedParamType::spt_sint32;
	else if (typeName == "float")
		return LGS::TSupportedParamType::spt_float;
	else if (typeName == "string")
		return LGS::TSupportedParamType::spt_string;
	else if (typeName == "entityId")
		return LGS::TSupportedParamType::spt_entityId;
	else if (typeName == "sheetId")
		return LGS::TSupportedParamType::spt_sheetId;
	else if (typeName == "itemId")
		return LGS::TSupportedParamType::spt_itemId;
	else
		return LGS::TSupportedParamType::invalid_val;
}



CQueryParser::TParserResult CQueryParser::parseQuery(const std::string &queryStr, bool parseOnlyOption)
{
	// Query are formated as follow
	// query : options expr;
	// options:	(options)*;
	// option	:	'full-context'
	//			|	??
	//			;
	// expr : andExpr ('or' andExpr)?
	//		;
	// andExpr:	atom ('and' atom)?;
	// atom	:	'(' expr ')'
	//		|	predicate;
	// operator	:	'<'
	//			|	'<='
	//			|	'>'
	//			|	'>='
	//			|	'='
	//			|	'=='
	//			|	'!='
	//			|	'like';
	// predicate :	(ID | ('<' TYPE '>')) operator constant;
	// constant	:	STRING
	//			|	INT
	//			|	LONG
	//			|	FLOAT
	//			|	DATE
	//			|	entityId
	//			|	itemId
	//			|	sint;
	// date		:	DATE
	//			|	'yesterday'
	//			|	INT 'days'
	//			|	INT 'weeks'
	//			|	INT 'months'
	//			|	INT 'years';
	// longInt	:	INT ('l'|'L');
	// sint		:	'-' INT;
	// entityId	:	'(' INT ':' INT ':' INT ':' INT ')';
	// itemId	:	'[' INT ':' INT ':' INT ']';
	// ID	:	('a'-'z' | 'A'-'Z' | '_')('0'-'9'|'a'-'z' | 'A'-'Z' | '_')*;
	// STRING	:	'\"' (~'\"')* '\"';
	// INT		:	('0x')? ('0'-'9')+
	// LONG		:	INT ('l'|'L');
	// FLOAT	:	INT '.' INT;
	// DATE		:	INT '-' INT '-' INT (INT ':' INT)?;
	// TYPE		:	'{' ID '}'
	

	try 
	{
		TParserResult	pr;	

		iterator first = queryStr.begin();

		parseOptions(pr, first, queryStr.end());

		if (parseOnlyOption)
		{
			return pr;
		}

		auto_ptr<TQueryNode> rootNode(parseExpr(first, queryStr.end()));

		// make sure we have consumed all the stream
		iterator rew = first;
		TToken tok = getNextToken(first, queryStr.end());
		if (tok.TokenType != tt_EOF)
			throw EInvalidQuery(tok.It, "Not all the query content have been read");

		pr.QueryTree = rootNode;

		return pr;
	}
	catch (const EInvalidQuery &iq)
	{
		nlwarning("Error will parsing query near char %u : %s", iq.It - queryStr.begin(), iq.ErrorStr);

		throw iq;
	}

}

void CQueryParser::parseOptions(CQueryParser::TParserResult &parserResult, CQueryParser::iterator &it, CQueryParser::iterator end)
{
	// read options until the last one
	while (parseOption(parserResult, it, end));
}

bool CQueryParser::parseOption(CQueryParser::TParserResult &parserResult, CQueryParser::iterator &it, CQueryParser::iterator end)
{
	iterator rew = it;

	TToken tok = getNextToken(it, end);

	switch (tok.TokenType)
	{
	case tt_full_context:
		parserResult.FullContext = true;
		return true;
	case tt_output_prefix:
		{
			// read a output prefix
			tok = getNextToken(it, end);
			if (tok.TokenType != tt_EQUAL)
				throw EInvalidQuery(tok.It, "Output prefix option must be followed by an equal sign '='");

			tok = getNextToken(it, end);
			if (tok.TokenType != tt_ID && tok.TokenType != tt_STRING)
				throw EInvalidQuery(tok.It, "Output prefix option must be followed by a the prefix value after the '=' sign");

			// ok, store the prefix
			if (tok.TokenType == tt_STRING)
				parserResult.OutputPrefix = tok.Text.substr(1, tok.Text.size()-1);
			else
				parserResult.OutputPrefix = tok.Text;


			return true;
		}
	}

	// not an option
	it = rew;
	return false;
}

TQueryNode *CQueryParser::buildPredicate(const LGS::TParamValue &refVal, const TToken &operatorType, const TToken &leftValue, CQueryParser::iterator &it, CQueryParser::iterator end)
{
	TQueryNode *node = NULL;
	if (leftValue.TokenType == tt_ID)
	{
		switch (operatorType.TokenType)
		{
		case tt_LESS:
			node = new TPredicateNode<TLessOp>(refVal, leftValue.Text, _LogDefs);
			break;
		case tt_LESS_EQUAL:
			node = new TPredicateNode<TLessEqualOp>(refVal, leftValue.Text, _LogDefs);
			break;
		case tt_GREATER:
			node = new TPredicateNode<TGreaterOp>(refVal, leftValue.Text, _LogDefs);
			break;
		case tt_GREATER_EQUAL:
			node = new TPredicateNode<TGreaterEqualOp>(refVal, leftValue.Text, _LogDefs);
			break;
		case tt_EQUAL:
			node = new TPredicateNode<TEqualOp>(refVal, leftValue.Text, _LogDefs);
			break;
		case tt_NOT_EQUAL:
			node = new TPredicateNode<TNotEqualOp>(refVal, leftValue.Text, _LogDefs);
			break;
		case tt_like:
			node = new TPredicateNode<TLikeOp>(refVal, leftValue.Text, _LogDefs);
			break;
		default:
			throw EInvalidQuery(operatorType.It, "Invalid operator for predicate");
			
		}
		if (!node->init())
		{
			nlwarning("No log match a parameter named %s with a predicate of type %s", leftValue.Text.c_str(), refVal.getType().toString().c_str());
		}
	}
	else if (leftValue.TokenType == tt_TYPE)
	{
		LGS::TSupportedParamType type = parseParamType(leftValue.Text);
		if (type == LGS::TSupportedParamType::invalid_val)
			throw EInvalidQuery(leftValue.It, "Invalid type name");

		switch (operatorType.TokenType)
		{
		case tt_LESS:
			node = new TTypePredicateNode<TLessOp>(refVal, type, _LogDefs);
			break;
		case tt_LESS_EQUAL:
			node = new TTypePredicateNode<TLessEqualOp>(refVal, type, _LogDefs);
			break;
		case tt_GREATER:
			node = new TTypePredicateNode<TGreaterOp>(refVal, type, _LogDefs);
			break;
		case tt_GREATER_EQUAL:
			node = new TTypePredicateNode<TGreaterEqualOp>(refVal, type, _LogDefs);
			break;
		case tt_EQUAL:
			node = new TTypePredicateNode<TEqualOp>(refVal, type, _LogDefs);
			break;
		case tt_NOT_EQUAL:
			node = new TTypePredicateNode<TNotEqualOp>(refVal, type, _LogDefs);
			break;
		case tt_like:
			node = new TTypePredicateNode<TLikeOp>(refVal, type, _LogDefs);
			break;
		default:
			throw EInvalidQuery(operatorType.It, "Invalid operator for predicate");
			
		}
		if (!node->init())
		{
			nlwarning("No log match a parameter of type %s with a predicate of type %s", leftValue.Text.c_str(), refVal.getType().toString().c_str());
		}
	}
	return node;
}

sint32 CQueryParser::parseSint(CQueryParser::iterator &it, CQueryParser::iterator end)
{
	iterator rew = it;
	skipWS(it, end);
	iterator start = it;
	TToken tok = getNextToken(it, end);
	if (tok.TokenType != tt_DASH)
		throw EInvalidQuery(rew, "Invalid start character for sint, must be '-'");

	rew = it;
	tok = getNextToken(it, end);
	if (tok.TokenType != tt_INT)
		throw EInvalidQuery(rew, "Invalid content for sint, must be an int value");

	sint32 val;
	NLMISC::fromString(string(start, it), val);

	return val;
}


std::string CQueryParser::parseItemId(CQueryParser::iterator &it, CQueryParser::iterator end)
{
	iterator rew = it;
	it = skipWS(it, end);
	iterator start = it;
	TToken tok = getNextToken(it, end);
	if (tok.TokenType != tt_OPEN_BRACKET)
		throw EInvalidQuery(tok.It, "Invalid start character for item id, must be '['");

	rew = it;
	tok = getNextToken(it, end);
	if (tok.TokenType != tt_INT)
		throw EInvalidQuery(tok.It, "Invalid first element for item id, must be an int value");

	rew = it;
	tok = getNextToken(it, end);
	if (tok.TokenType != tt_COLON)
		throw EInvalidQuery(rew, "Invalid separator for item id, must be a ':' char");

	rew = it;
	tok = getNextToken(it, end);
	if (tok.TokenType != tt_INT)
		throw EInvalidQuery(tok.It, "Invalid second element for item id, must be an int value");

	rew = it;
	tok = getNextToken(it, end);
	if (tok.TokenType != tt_COLON)
		throw EInvalidQuery(tok.It, "Invalid separator for item id, must be a ':' char");

	rew = it;
	tok = getNextToken(it, end);
	if (tok.TokenType != tt_INT)
		throw EInvalidQuery(tok.It, "Invalid third element for item id, must be an int value");

	tok = getNextToken(it, end);
	if (tok.TokenType != tt_CLOSE_BRACKET)
		throw EInvalidQuery(tok.It, "Invalid end character for item id, must be ']'");

	return string(start, it);
}


std::string CQueryParser::parseEntityId(CQueryParser::iterator &it, CQueryParser::iterator end)
{
	iterator rew = it;
	it = skipWS(it, end);
	iterator start = it;
	TToken tok = getNextToken(it, end);
	if (tok.TokenType != tt_OPEN_PAR)
		throw EInvalidQuery(tok.It, "Invalid start character for entity id, must be '('");

	rew = it;
	tok = getNextToken(it, end);
	if (tok.TokenType != tt_INT)
		throw EInvalidQuery(tok.It, "Invalid first element for entity id, must be an hexa value");

	rew = it;
	tok = getNextToken(it, end);
	if (tok.TokenType != tt_COLON)
		throw EInvalidQuery(tok.It, "Invalid separator for entity id, must be a ':' char");

	rew = it;
	tok = getNextToken(it, end);
	if (tok.TokenType != tt_INT && tok.TokenType != tt_NAKED_HEXA
		&& !(tok.TokenType == tt_ID && isNakedHexa(tok.Text)))
		throw EInvalidQuery(tok.It, "Invalid second element for entity id, must be an int or naked hexa value");

	rew = it;
	tok = getNextToken(it, end);
	if (tok.TokenType != tt_COLON)
		throw EInvalidQuery(tok.It, "Invalid separator for entity id, must be a ':' char");

	rew = it;
	tok = getNextToken(it, end);
	if (tok.TokenType != tt_INT && tok.TokenType != tt_NAKED_HEXA
		&& !(tok.TokenType == tt_ID && isNakedHexa(tok.Text)))
		throw EInvalidQuery(tok.It, "Invalid third element for entity id, must be an int or naked hexa value");

	rew = it;
	tok = getNextToken(it, end);
	if (tok.TokenType != tt_COLON)
		throw EInvalidQuery(tok.It, "Invalid separator for entity id, must be a ':' char");

	rew = it;
	tok = getNextToken(it, end);
	if (tok.TokenType != tt_INT && tok.TokenType != tt_NAKED_HEXA
		&& !(tok.TokenType == tt_ID && isNakedHexa(tok.Text)))
		throw EInvalidQuery(tok.It, "Invalid fourth element for entity id, must be an int or naked hexa value");

	rew = it;
	tok = getNextToken(it, end);
	if (tok.TokenType != tt_CLOSE_PAR)
		throw EInvalidQuery(tok.It, "Invalid end character for entity id, must be ')'");

	return string(start, it);
}

bool CQueryParser::isNakedHexa(const std::string &text)
{
	for (uint i=0; i<text.size(); ++i)
	{
		char c = text[i];
		if (!((c >= '0' && c <= '9')
				|| (c >= 'a' && c <= 'f')
				|| (c >= 'A' && c <= 'F')
			))
			return false;
	}

	return true;
}

LGS::TParamValue CQueryParser::parseConstant(CQueryParser::iterator &it, CQueryParser::iterator end)
{
	iterator rew = it;
	TToken constantTok = getNextToken(it, end);

	switch (constantTok.TokenType)
	{
	case tt_INT:
		{
			uint32 num;
			NLMISC::fromString(constantTok.Text, num);
			rew = it;
			TToken dateTag = getNextToken(it, end);
			switch (dateTag.TokenType)
			{
			case tt_secs:
				num = CTime::getSecondsSince1970()-num;
				break;
			case tt_mins:
				num = CTime::getSecondsSince1970()-num*60;
				break;
			case tt_hours:
				num = CTime::getSecondsSince1970()-num*(60*60);
				break;
			case tt_days:
				num = CTime::getSecondsSince1970()-num*(60*60*24);
				break;
			case tt_weeks:
				num = CTime::getSecondsSince1970()-num*(60*60*24*7);
				break;
			case tt_months:
				num = CTime::getSecondsSince1970()-num*(60*60*24*30);
				break;
			case tt_years:
				num = CTime::getSecondsSince1970()-num*(60*60*24*365);
				break;
			default:
				// rewind the last token
				it = rew;
			}

			return LGS::TParamValue(num);
		}
	case tt_STRING:
		return LGS::TParamValue(constantTok.Text.substr(1, constantTok.Text.size()-1));
	case tt_FLOAT:
		return LGS::TParamValue(float(atof(constantTok.Text.c_str())));
	case tt_LONG:
		return LGS::TParamValue(uint64(atol(constantTok.Text.c_str())));
	case tt_DATE:
		{
			struct tm date;
			memset(&date, 0, sizeof(date));
			int nbParam = sscanf(constantTok.Text.c_str(), "%i-%i-%i %i:%i:%i", 
				&date.tm_year, 
				&date.tm_mon,
				&date.tm_mday,
				&date.tm_hour,
				&date.tm_min,
				&date.tm_sec);

			// adjust the year offset
			date.tm_year -= 1900;
			// adjust month
			date.tm_mon -= 1;
			// let the CRunt time compute the daylight saving offset
			date.tm_isdst  = -1;

			time_t t = mktime(&date);

			return LGS::TParamValue(uint32(t));
		}
	case tt_OPEN_PAR:
		{
			it = rew;
			std::string eids = parseEntityId(it, end);
			CEntityId eid(eids);
			return LGS::TParamValue(eid);
		}
	case tt_OPEN_BRACKET:
		{
			it = rew;
			std::string itemIdStr = parseItemId(it, end);
			INVENTORIES::TItemId itemId(itemIdStr);
			return LGS::TParamValue(itemId);
		}
	case tt_DASH:
		{
			it = rew;
			sint32 i = parseSint(it, end);
			return LGS::TParamValue(i);
		}
	case tt_ID:
		{
			return LGS::TParamValue(CSheetId(constantTok.Text));
		}
	case tt_yesterday:
		{
			uint32 now = CTime::getSecondsSince1970();
			// return date of yesterday
			return LGS::TParamValue(now - 60*60*24);
		}
	default:
		throw EInvalidQuery(constantTok.It, "Invalid constant value on right of operator for predicate");
	}

	return LGS::TParamValue();
}

TQueryNode* CQueryParser::parsePredicate(CQueryParser::iterator &it, CQueryParser::iterator end)
{
	iterator rew = it;
	TToken idTok = getNextToken(it, end);

	if (idTok.TokenType == tt_OPEN_BRACE)
	{
		// try to read a type 
		idTok = getNextToken(it, end);
		if (idTok.TokenType != tt_ID)
			throw EInvalidQuery(idTok.It, "Param type predicate must follow the form '{type}', invalid type identifier");
		TToken tok = getNextToken(it, end);
		if (tok.TokenType != tt_CLOSE_BRACE)
			throw EInvalidQuery(tok.It, "Param type predicate must follow the form '{type}', invalid close char (not a closing brace '}')");

		idTok.TokenType = tt_TYPE;
	}

	if (idTok.TokenType != tt_ID && idTok.TokenType != tt_TYPE)
		throw EInvalidQuery(idTok.It, "Invalid left parameter for predicate");

	TToken opTok = getNextToken(it, end);

	switch (opTok.TokenType)
	{
	case tt_LESS:
	case tt_LESS_EQUAL:
	case tt_GREATER:
	case tt_GREATER_EQUAL:
	case tt_EQUAL:
	case tt_NOT_EQUAL:
	case tt_like:
		{
			LGS::TParamValue refVal = parseConstant(it, end);
			return buildPredicate(refVal, opTok, idTok, it, end);
		}
	default:
		throw EInvalidQuery(opTok.It, "Invalid operator for predicate");
	}

	return NULL;
}

TQueryNode* CQueryParser::parseAtom(CQueryParser::iterator &it, CQueryParser::iterator end)
{
	iterator rew = it;
	TQueryNode *node = NULL;
	TToken tok = getNextToken(it, end);

	if (tok.TokenType == tt_OPEN_PAR)
	{
		node = parseExpr(it, end);

		// must have a close token
		tok = getNextToken(it, end);
		if (tok.TokenType != tt_CLOSE_PAR)
			throw EInvalidQuery(tok.It, "Missing closing parenthesis");
	}
	else
	{
		it = rew;
		node =  parsePredicate(it, end);
	}

	return node;
}

	
TQueryNode *CQueryParser::parseAndExpr(CQueryParser::iterator &it, CQueryParser::iterator end)
{
	TQueryNode *node = parseAtom(it, end);

	iterator rew = it;

	TToken tok = getNextToken(it, end);

	while (tok.TokenType != tt_EOF)
	{
		if (tok.TokenType == tt_and)
		{
			TQueryNode *left = node;
			// create a 'or' root node
			node = new TCombineNode<TAndCombiner>;

			// parse another expression
			TQueryNode *right = parseAtom(it, end);

			node->LeftNode = left;
			node->RightNode = right;

			rew = it;
			// parse an optional 'and'
			tok = getNextToken(it, end);
		}
		else
		{
			it = rew;
			break;
		}
	}

	return node;
}


TQueryNode *CQueryParser::parseExpr(CQueryParser::iterator &it, CQueryParser::iterator end)
{
	TQueryNode *node = parseAndExpr(it, end);

	iterator rew = it;

	TToken tok = getNextToken(it, end);

	while (tok.TokenType != tt_EOF)
	{
		if (tok.TokenType == tt_or)
		{
			TQueryNode *left = node;
			// create a 'or' root node
			node = new TCombineNode<TOrCombiner>;

			// parse another expression
			TQueryNode *right = parseAndExpr(it, end);

			node->LeftNode = left;
			node->RightNode = right;

			rew = it;
			// parse an optional 'or'
			tok = getNextToken(it, end);
		}
		else
		{
			it = rew;
			break;
		}
	}

	return node;

}




// parse an ID
bool CQueryParser::parseID(CQueryParser::iterator &it, CQueryParser::iterator end)
{
	iterator rew = it;
	if (it != end 
		&& (*it>='a' && *it<='z' 
		|| *it>='A' && *it<='Z' 
		|| *it == '_'))
	{
		++it;

		while (it != end 
			&& (*it>='a' && *it<='z' 
			|| *it>='A' && *it<='Z' 
			|| *it == '_'
			|| *it>='0' && *it<='9'
			|| *it == '.'))
			++it;
		return true;
	}

	it = rew;
	return false;
}

// parse an STRING
bool CQueryParser::parseSTRING(CQueryParser::iterator &it, CQueryParser::iterator end)
{
	iterator rew = it;

	if (it != end && *it == '"')
	{
		++it;
		char c = *it;

		while (it != end && *it != '"')
			++it;

		if (it != end && *it == '"')
		{
			++it;
			return true;
		}
	}

	it = rew;
	return false;
}

// Parse a date
bool CQueryParser::parseDATE(CQueryParser::iterator &it, CQueryParser::iterator end)
{
	iterator rew = it;

	// year is already parsed
//	if (!parseINT(it, end))
//		goto failed;
	if (getNextToken(it, end).TokenType != tt_DASH)
		goto failed;
	// month
	if (!parseINT(it, end))
		goto failed;
	if (getNextToken(it, end).TokenType != tt_DASH)
		goto failed;
	// day
	if (!parseINT(it, end))
		goto failed;

	// optional hour
	rew = it;
	it = skipWS(it, end);
	if (parseINT(it, end))
	{
		if (getNextToken(it, end).TokenType != tt_COLON)
			goto noHour;
		if (!parseINT(it, end))
			goto noHour;
		// optional sec
		rew = it;
		if (getNextToken(it, end).TokenType != tt_COLON)
			goto noHour;
		if (!parseINT(it, end))
			goto noHour;
	}
	else
	{
noHour:
		// no hour, rewind
		it = rew;
	}

	// ok, the date if correctly parsed
	return true;


failed:
	it = rew;
	return false;
}

// parse an FLOAT
bool CQueryParser::parseFLOAT(CQueryParser::iterator &it, CQueryParser::iterator end)
{
	iterator rew = it;

	if (getNextToken(it, end).TokenType != tt_DOT)
		goto failed;

	if (!parseINT(it, end))
		goto failed;

	return true;

failed:
	it = rew;
	return false;
}

// Parse a long int (64 bits)
bool CQueryParser::parseLONG_INT(CQueryParser::iterator &it, CQueryParser::iterator end)
{
	iterator rew = it;

	TToken tok = getNextToken(it, end);
	if (tok.TokenType != tt_ID || (tok.Text != "l" && tok.Text != "L"))
		goto failed;

	// ok, this is a long int
	return true;

failed:
	it= rew;
	return false;
}

/// Parse a naked hexa (ie. without the '0x' prefix)
bool CQueryParser::parseNAKED_HEXA(iterator &it, iterator end)
{
	iterator rew = it;

	while (it != end && ((*it >= 'a' && *it <= 'f') || (*it >= 'A' && *it <= 'F')))
		++it;

	if (it == rew)
		// no character consumed, not a naked hexa
		goto failed;
	// the iterator have advanced, check the next character
	if (it != end && ((*it >= 'a' && *it <= 'z') || (*it >= 'A' && *it <= 'Z') || (*it == '_')))
	{
		// not a naked hexa
		goto failed;
	}

	// ok, this is a naked hexa
	return true;

failed:
	it= rew;
	return false;
}

// parse an INT
bool CQueryParser::parseINT(CQueryParser::iterator &it, CQueryParser::iterator end)
{
	iterator rew = it;

	if (it != end && *it >='0' && *it <='9')
	{
		++it;
		// check for 'hex' constant
		if (it != end && (*it == 'x' || *it == 'X'))
		{
			++it;

			if (it == end 
				|| !(*it >= '0' && *it<='9' 
					|| *it >= 'a' && *it<='f'
					|| *it >= 'A' && *it<='F'))
			{
				// need at least one hexdigit after the 0x prefix
				it = rew;
				return false;
			}
			// read the hex digit
			while (it != end 
				&& (	(*it >='0' && *it <= '9') 
						|| (*it >='a' && *it <= 'f') 
						|| (*it >='A' && *it <= 'F')))
				++it;
		}
		else
		{
			// read decimal digits
			while (it != end && *it >='0' && *it <= '9')
				++it;
		}

		return true;
	}

	it = rew;
	return false;
}


// The lexer
CQueryParser::TToken CQueryParser::getNextToken(CQueryParser::iterator &it, CQueryParser::iterator end)
{
	TToken ret;
	iterator rew = it;
	it = skipWS(it, end);

	ret.It = it;

	if (it == end)
	{
		ret.TokenType = tt_EOF;
		ret.Text = "";
		return ret;
	}

	iterator first = it;
	char c = *it;
	if (c>='a' && c<='z' 
		|| c>='A' && c<='Z' 
		|| c == '_')
	{
		// try to read an ID
		if (parseID(it, end))
		{
			ret.TokenType = tt_ID;
			ret.Text = std::string(first, it);
		}
	}
	else if (c == '"')
	{
		// try to read a string
		if (parseSTRING(it, end))
		{
			ret.TokenType = tt_STRING;
			ret.Text = std::string(first, it-1);
		}
	}
	else if (c >= '0' && c <= '9')
	{
		if (parseINT(it, end))
		{
			// try to read a long int
			if (parseLONG_INT(it, end))
			{
				ret.TokenType = tt_LONG;
				ret.Text = string(first, it);
			}
			// try to read a date
			else if (parseDATE(it, end))
			{
				ret.TokenType = tt_DATE;
				ret.Text = std::string(first, it);
			}
			// try to read a float
			else if (parseFLOAT(it, end))
			{
				ret.TokenType = tt_FLOAT;
				ret.Text = std::string(first, it);
			}
			// try to read a naked hexa 
			else if (parseNAKED_HEXA(it, end))
			{
				ret.TokenType = tt_NAKED_HEXA;
				ret.Text = std::string(first, it);
			}
			else /*if (parseINT(it, end))*/
			{
				// just a simple int
				ret.TokenType = tt_INT;
				ret.Text = std::string(first, it);
			}
		}
	}
	else if (c == '<')
	{
		// read less or less equal
		++it;
		if (it != end && *it == '=')
		{
			// consume the '='
			++it;
			ret.TokenType = tt_LESS_EQUAL;
			ret.Text = "<=";
		}
		else
		{
			ret.TokenType = tt_LESS;
			ret.Text = "<";
		}
	}
	else if (c == '>')
	{
		// read greater or greater equal
		++it;
		if (it != end && *it == '=')
		{
			// consume the '='
			++it;
			ret.TokenType = tt_GREATER_EQUAL;
			ret.Text = ">=";
		}
		else
		{
			ret.TokenType = tt_GREATER;
			ret.Text = ">";
		}
	}
	else if (c == '=')
	{
		// try to read '=='
		++it;
		if (it != end && *it == '=')
		{
			// we support '==' and '=', so consume the secondary '=' if any
			++it;
		}
		// equal
		ret.TokenType = tt_EQUAL;
		ret.Text = "==";
	}
	else if (c == '!')
	{
		// try to read '!='
		++it;
		if (it != end && *it == '=')
		{
			++it;
			// not equal
			ret.TokenType = tt_NOT_EQUAL;
			ret.Text = "!=";
		}
		else
		{
			// nothing recognised
			--it;
			// no valid alternative found !
			throw EInvalidQuery(it, "Invalid char following '!', need an '=' sign");
		}
	}
	else if (c == '(')
	{
		++it;
		ret.TokenType = tt_OPEN_PAR;
		ret.Text = "(";
	}
	else if (c == ')')
	{
		++it;
		ret.TokenType = tt_CLOSE_PAR;
		ret.Text = ")";
	}
	else if (c == '[')
	{
		++it;
		ret.TokenType = tt_OPEN_BRACKET;
		ret.Text = "[";
	}
	else if (c == ']')
	{
		++it;
		ret.TokenType = tt_CLOSE_BRACKET;
		ret.Text = "]";
	}
	else if (c == '{')
	{
		++it;
		ret.TokenType = tt_OPEN_BRACE;
		ret.Text = "{";
	}
	else if (c == '}')
	{
		++it;
		ret.TokenType = tt_CLOSE_BRACE;
		ret.Text = "}";
	}
	else if (c == '-')
	{
		++it;
		ret.TokenType = tt_DASH;
		ret.Text = "-";
	}
	else if (c == ':')
	{
		++it;
		ret.TokenType = tt_COLON;
		ret.Text = ":";
	}
	else if (c == '.')
	{
		++it;
		ret.TokenType = tt_DOT;
		ret.Text = ".";
	}
	else
	{
		// no valid alternative found !
		throw EInvalidQuery(it, "Can not found a valid lexer token");
	}

	// check for keyword
	if (_Keywords.find(ret.Text) != _Keywords.end())
	{
		// change the token type
		ret.TokenType = _Keywords[ret.Text];
	}

	return ret;
}

CQueryParser::iterator CQueryParser::skipWS(CQueryParser::iterator it, CQueryParser::iterator end)
{
	while (it != end)
	{
		switch (*it)
		{
		case ' ':
		case '\n':
		case '\r':
		case '\t':
			// this is a white space
			// break the switch and advance to next character
			break;
		default:
			// not a white space, return the iterator
			return it;
		}
		++it;
	}

	// no more character to read, return the iterator (should be end)
	return it;
}

CQueryParser	*createQueryParser(const TLogDefinitions &logDefs)
{
	return new CQueryParser(logDefs);
}
