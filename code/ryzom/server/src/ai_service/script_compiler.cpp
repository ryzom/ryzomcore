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

#include "stdpch.h"

#include "script_vm.h"
#include "script_compiler.h"

// Compiler special keywords (tokens and rules)
static std::string const s_kw_NUMBER       = "NUMBER";
static std::string const s_kw_CHAIN        = "CHAIN";
static std::string const s_kw_NAME         = "NAME";
static std::string const s_kw_STRNAME      = "STRNAME";
static std::string const s_kw_POINT        = "POINT";
static std::string const s_kw_readConstVar = "readConstVar";
static std::string const s_kw_lineOrClose  = "lineOrClose";
static std::string const s_kw_params       = "params";
static std::string const s_kw_tuple        = "tuple";
static std::string const s_kw_expeclose    = "expeclose";
static std::string const s_kw_exp          = "exp";
static std::string const s_kw_somme        = "somme";
static std::string const s_kw_produit      = "produit";
static std::string const s_kw_facteur      = "facteur";
static std::string const s_kw_case         = "case";

using namespace std;
using namespace NLMISC;

//////////////////////////////////////////////////////////////////////////
// A Small Custom Compiler For AI.
// (Token and Grammar are Upgradable, Error returns have to be upgraded).
// Ask to Stephane Le Dorze for Explanations.
//////////////////////////////////////////////////////////////////////////

namespace AICOMP
{

	
/****************************************************************************/
/* Script related classes                                                   */
/****************************************************************************/

//////////////////////////////////////////////////////////////////////////////
// CScriptNativeFuncParams                                                  //
//////////////////////////////////////////////////////////////////////////////

CScriptNativeFuncParams::CScriptNativeFuncParams(const std::string &str, FScrptNativeFunc func)
	: _func(func)
	, _va(false)
{
	const size_t lastPartIndex2=str.find_last_of("_", string::npos);
	nlassert(lastPartIndex2!=0 && lastPartIndex2!=string::npos);
	const size_t lastPartIndex=str.find_last_of("_", lastPartIndex2-1);
	nlassert(lastPartIndex!=0 && lastPartIndex!=string::npos);
	
	_nbInParams=lastPartIndex2-lastPartIndex-1;
	_nbOutParams=str.size()-lastPartIndex2-1;
	if (_nbInParams==1 && str[lastPartIndex+1] == 'v')
	{
		_nbInParams = ~0;
		_va = true;
	}
	if (_nbOutParams==1 && str[lastPartIndex2+1] == 'v')
	{
		_nbOutParams = ~0;
		_va = true;
	}
}

//////////////////////////////////////////////////////////////////////////////
// CJumpRememberer                                                          //
//////////////////////////////////////////////////////////////////////////////

class CJumpRememberer
{
public:
	CJumpRememberer(size_t codeBlockIndex);
	size_t _where;
	size_t _codeBlockIndex;
};

//////////////////////////////////////////////////////////////////////////////
// CJumpTable                                                               //
//////////////////////////////////////////////////////////////////////////////

class CJumpTable
{
public:
	CJumpTable(const CSmartPtr<AIVM::CByteCode> &byteCode);
	virtual ~CJumpTable();
	void add(CJumpRememberer jump);
	void newCodeBlock();
private:
	vector<size_t> _codeOffsets;
	vector<CJumpRememberer> _jumps;
	CSmartPtr<AIVM::CByteCode> _byteCode;
};

//////////////////////////////////////////////////////////////////////////////
// CCaseTracer                                                              //
//////////////////////////////////////////////////////////////////////////////

class CCaseTracer : public CRefCount
{
public:
	CCaseTracer(const CSmartPtr<CSubRuleTracer> &tracer, const string &sourceName);
	
	CSmartPtr<CSubRuleTracer> _tracer;
	CSmartPtr<const AIVM::CByteCode> _code;
	size_t _sortValue;
};

/****************************************************************************/
/* TOKEN SPECIALIZATION                                                     */
/****************************************************************************/

/* used:
	[]		-> or for character.
	-		-> from before to after (char) (inside []).
	"c"		-> character c (or \c)
	
	{xx}	-> the token 'xx'.
	()		-> enclosing
	
	|		-> or
	
	+		-> card suffixe 1..n
	*		-> card suffixe 0..n
	?		-> card suffixe 0..1
	{m,n}	-> card suffixe m..n
*/

//////////////////////////////////////////////////////////////////////////////
// CBracketToken                                                            //
//////////////////////////////////////////////////////////////////////////////

// [a-zA-Z0-9] (readOK)
class CBracketToken : public CBasicToken
{
public:
	CBasicToken *createNew() const;
	size_t init(TTokenList &tokenList, const string &str, size_t firstIndex, size_t lastIndex);
	CTokenTestResult buildNode(const std::string &code, size_t &index) const;
	void dump(size_t indent) const;
	
private:
	string _Body;
};

//////////////////////////////////////////////////////////////////////////////
// CCharToken                                                               //
//////////////////////////////////////////////////////////////////////////////

// "c"
class CCharToken : public CBasicToken
{
public:
	CBasicToken *createNew() const;
	size_t init(TTokenList &tokenList, const string &str, size_t firstIndex, size_t lastIndex);
	CTokenTestResult buildNode(const std::string &code, size_t &index) const;
	void dump(size_t indent) const;
	
private:
	char _c;
};

//////////////////////////////////////////////////////////////////////////////
// CTokenToken                                                              //
//////////////////////////////////////////////////////////////////////////////

// {mytoken}
class CTokenToken : public CBasicToken
{
public:
	CBasicToken *createNew() const;
	size_t init(TTokenList &tokenList, const string &str, size_t firstIndex, size_t lastIndex);
	CTokenTestResult buildNode(const std::string &code, size_t &index) const;
	void dump(size_t indent) const;
	
private:
	string _tokenName;
};

//////////////////////////////////////////////////////////////////////////////
// CParenthesisToken                                                        //
//////////////////////////////////////////////////////////////////////////////

// (..)
class CParenthesisToken : public CBasicToken
{
public:
	CBasicToken *createNew() const;
	size_t init(TTokenList &tokenList, const string &str, size_t firstIndex, size_t lastIndex);
	CTokenTestResult buildNode(const std::string &code, size_t &index) const;
	void dump(size_t indent) const;
	
private:
	TTokenList _tokenList;
};

//////////////////////////////////////////////////////////////////////////////
// COrToken                                                                 //
//////////////////////////////////////////////////////////////////////////////

// | (readOK)
class COrToken : public CBasicToken
{
public:
	CBasicToken *createNew() const;
	size_t init(TTokenList &tokenList, const string &str, size_t firstIndex, size_t lastIndex);
	CTokenTestResult buildNode(const std::string &code, size_t &index) const;
	void dump(size_t indent) const;
	
private:
	CSmartPtr<CBasicToken> firstToken;
	CSmartPtr<CBasicToken> secondToken;
};

//////////////////////////////////////////////////////////////////////////////
// CCardToken                                                               //
//////////////////////////////////////////////////////////////////////////////

// * + ? (readOK)
class CCardToken : public CBasicToken
{
public:
	enum TCardType
	{
		CARD_ZERO_ONE=0,
		CARD_ZERO_MANY,
		CARD_ONE_MANY,
	};
	
	CBasicToken *createNew() const;
	size_t init(TTokenList &tokenList, const string &str, size_t firstIndex, size_t lastIndex);
	CTokenTestResult buildNode(const std::string &code, size_t &index) const;
	void dump(size_t indent) const;
	
private:
	CSmartPtr<CBasicToken> _childToken;
	TCardType _card;
};

/****************************************************************************/
/** Script related classes implementation ***********************************/
/****************************************************************************/

/****************************************************************************/
/* Tokens                                                                   */
/****************************************************************************/

//////////////////////////////////////////////////////////////////////////////
// CBracketToken implementation                                             //
//////////////////////////////////////////////////////////////////////////////

CBasicToken *CBracketToken::createNew() const
{
	return new CBracketToken();
}

size_t CBracketToken::init(TTokenList &tokenList, const string &str, size_t firstIndex, size_t lastIndex)
{
	const size_t index = str.find_first_of("]", firstIndex);
	nlassert(index!=string::npos);
	firstIndex++; // pass '['
	_Body = str.substr(firstIndex, index-firstIndex);
	tokenList.push_back(this);
	return index+1;
}

CTokenTestResult CBracketToken::buildNode(const std::string &code, size_t &index) const
{
	if (index>=code.size())
		return CTokenTestResult(NULL, CTokenTestResult::BRULE_INVALID);

	const char c = code.at(index);
	bool success = false;
	bool first = true;
	FOREACHC(p, string, _Body)
	{
		const char cp = *p;
		if (first)
		{
			if (cp==c)
				goto found;
			first=false;
			continue;
		}

		if (cp=='-')
		{
			if ( c>=*(p-1)
				&& c<=*(p+1))
				goto found;
			p++; // to pass the last letter of the range
			continue;
		}
		
		if (cp==c)
			goto found;
	}
	
	return CTokenTestResult(NULL, CTokenTestResult::BRULE_INVALID);
found:
	index++;
	string name;
	name+=c;
	CSmartPtr<CCodeNode> codeNode=new CCodeNode("Bracket",name);
	return CTokenTestResult(codeNode);
}

void CBracketToken::dump(size_t indent) const
{
	string str;
	str.resize(indent,' ');
	str+="["+_Body+"]";
	nldebug(str.c_str());
}

//////////////////////////////////////////////////////////////////////////////
// CCharToken implementation                                                //
//////////////////////////////////////////////////////////////////////////////

CBasicToken *CCharToken::createNew() const
{
	return new CCharToken();
}

size_t CCharToken::init(TTokenList &tokenList, const string &str, size_t firstIndex, size_t lastIndex)
{
	if (str.at(firstIndex)=='\\')
	{
		_c=str.at(firstIndex+1);
		tokenList.push_back(this);
		return firstIndex+2;
	}
	else
	{
		nlassert(str.at(firstIndex)=='"');
		nlassert(str.at(firstIndex+2)=='"');
		_c=str.at(firstIndex+1);
		tokenList.push_back(this);
		return firstIndex+3;
	}
}

CTokenTestResult CCharToken::buildNode(const std::string &code, size_t &index) const
{
	if ( index<code.size()
		&& code.at(index)==_c)
	{
		index++;
		string name;
		name+=_c;

		CSmartPtr<CCodeNode> codeNode=new CCodeNode("Char",name);
		return CTokenTestResult(codeNode);
	}
	return CTokenTestResult(NULL, CTokenTestResult::BRULE_INVALID);
}
	
void CCharToken::dump(size_t indent) const
{
	string str;
	str.resize(indent,' ');
	str+="'";
	str+=_c;
	str+="'";
	nldebug(str.c_str());
}

//////////////////////////////////////////////////////////////////////////////
// CTokenToken implementation                                               //
//////////////////////////////////////////////////////////////////////////////

CBasicToken *CTokenToken::createNew() const
{
	return new CTokenToken();
}

size_t CTokenToken::init(TTokenList &tokenList, const string &str, size_t firstIndex, size_t lastIndex)
{
	size_t index=str.find_first_of("}", firstIndex);
	nlassert(index!=string::npos);
	firstIndex++;
	_tokenName=str.substr(firstIndex, index-firstIndex);
	tokenList.push_back(this);
	return index+1;
}

CTokenTestResult CTokenToken::buildNode(const std::string &code, size_t &index) const
{
	CToken *const token=CCompiler::getInstance().getToken(_tokenName);
	nlassert(token!=NULL);
	const size_t lastIndex=index;
	CTokenTestResult res=token->buildTree(code,index);
	if (!res.isValid())
	{
		nlwarning("token %s not succeed index %d", _tokenName.c_str(), index);
		return res;
	}
	
	CSmartPtr<CCodeNode> codeNode=new CCodeTokenNode("token", _tokenName, res.getCode());
	return CTokenTestResult(codeNode);
}

void CTokenToken::dump(size_t indent) const
{
	string str;
	str.resize(indent,' ');
	str+="{"+_tokenName+"}";
	nldebug(str.c_str());
}

//////////////////////////////////////////////////////////////////////////////
// CParenthesisToken implementation                                         //
//////////////////////////////////////////////////////////////////////////////

CBasicToken *CParenthesisToken::createNew() const
{
	return new CParenthesisToken();
}

size_t CParenthesisToken::init(TTokenList &tokenList, const string &str, size_t firstIndex, size_t lastIndex)
{
	const size_t index=initTokens(_tokenList, str, firstIndex+1, lastIndex);
	nlassert(str.at(index)==')');
	tokenList.push_back(this);
	return index+1;
}

CTokenTestResult CParenthesisToken::buildNode(const std::string &code, size_t &index) const
{
	CSmartPtr<CCodeNode> codeNode;
	CSmartPtr<CCodeNode> nextCodeNode;
	size_t localIndex=index;
	
	FOREACHC(tokenIt, TTokenList, _tokenList)
	{
		CTokenTestResult res=(*tokenIt)->buildNode(code, localIndex); 			
		if (!res.isValid())
			return CTokenTestResult(NULL, CTokenTestResult::BRULE_INVALID);

		CSmartPtr<CCodeNode> localCodeNode=res.getCode();
		if (!localCodeNode)
			continue;

		// Chain result node.
		if (!codeNode)
			codeNode=localCodeNode;
		else
			nextCodeNode->_NextNode=localCodeNode;
		nextCodeNode=localCodeNode;
		while (nextCodeNode->_NextNode)
			nextCodeNode=nextCodeNode->_NextNode;
	}
	if (codeNode)
		index=localIndex;
	return codeNode;
}

void CParenthesisToken::dump(size_t indent) const
{
	string str;
	str.resize(indent,' ');
	
	nldebug((str+"(").c_str());
	FOREACHC(tokenIt,TTokenList,_tokenList)
		(*tokenIt)->dump(indent+1);
	nldebug((str+")").c_str());
}

//////////////////////////////////////////////////////////////////////////////
// COrToken implementation                                                  //
//////////////////////////////////////////////////////////////////////////////

CBasicToken *COrToken::createNew() const
{
	return new COrToken();
}

size_t COrToken::init(TTokenList &tokenList, const string &str, size_t firstIndex, size_t lastIndex)
{
	nlassert(tokenList.size()>0);
	const size_t orIndex=tokenList.size()-1;
	const size_t finalIndex=initTokens(tokenList, str, firstIndex+1, lastIndex);
	nlassert(tokenList.size()>1);
	
	// insert the or operation.
	firstToken=tokenList[orIndex];
	secondToken=tokenList[orIndex+1];
	tokenList[orIndex]=this;
	tokenList.erase(tokenList.begin()+orIndex+1);
	
	return finalIndex;
}

CTokenTestResult COrToken::buildNode(const std::string &code, size_t &index) const
{
	size_t firstIndex=index;
	CTokenTestResult firstRes=firstToken->buildNode 	(code, firstIndex);
	if (!firstRes.isValid())
	{
		size_t secondIndex=index;
		CTokenTestResult secondRes=secondToken->buildNode 	(code, secondIndex);
		index=secondIndex;
		return secondRes;
	}
	
	index=firstIndex;
	return firstRes;
}

void COrToken::dump(size_t indent) const
{
	firstToken->dump(indent+1);
	string str;
	str.resize(indent,' ');
	str+="|";
	nldebug(str.c_str());
	secondToken->dump(indent+1);
}

//////////////////////////////////////////////////////////////////////////////
// CCardToken implementation                                                //
//////////////////////////////////////////////////////////////////////////////

CBasicToken *CCardToken::createNew() const
{
	return new CCardToken();
}

size_t CCardToken::init(TTokenList &tokenList, const string &str, size_t firstIndex, size_t lastIndex)
{
	nlassert(tokenList.size()>0);

	switch(str.at(firstIndex))
	{
	case '?':
		_card = CARD_ZERO_ONE;
		break;
	case '*':
		_card = CARD_ZERO_MANY;
		break;
	case '+':
		_card = CARD_ONE_MANY;
		break;
	default:
		break;
	}
	
	_childToken = tokenList.back();
	tokenList.back()=this;

	return firstIndex+1;
}

CTokenTestResult CCardToken::buildNode(const std::string &code, size_t &index) const
{
	CSmartPtr<CCodeNode> firstCodeNode = NULL;
	CSmartPtr<CCodeNode> nextCodeNode = NULL;

	size_t localIndex = index;
	size_t nbNodes = 0;

	CTokenTestResult res;

	while ((res=_childToken->buildNode(code, localIndex)).isValid())
	{
		CSmartPtr<CCodeNode> newCodeNode=res.getCode();
		if (!newCodeNode)
			continue;
		
		if (_card==CARD_ZERO_ONE)
		{
			index=localIndex;
			return res;
		}

		if (!firstCodeNode)
			firstCodeNode = newCodeNode;
		else
			nextCodeNode->_NextNode=newCodeNode;
		nextCodeNode=newCodeNode;
		while (nextCodeNode->_NextNode)
			nextCodeNode = nextCodeNode->_NextNode;
		nbNodes++;
	}
	if (firstCodeNode)
		index = localIndex;
	return CTokenTestResult(firstCodeNode, (nbNodes==0 && _card==CARD_ONE_MANY)?CTokenTestResult::BRULE_INVALID:CTokenTestResult::BRULE_VALID);
}

void CCardToken::dump(size_t indent) const
{
	string str;
	str.resize(indent,' ');
	switch(_card)
	{
	case CARD_ZERO_ONE:
		str += "?(0..1)";
		break;
	case CARD_ZERO_MANY:
		str += "*(0..n)";
		break;
	case CARD_ONE_MANY:
		str += "+(1..n)";
		break;
	default:
		break;
	}
	nldebug(str.c_str());
	_childToken->dump(indent+1);
}

/****************************************************************************/
/* Nodes                                                                    */
/****************************************************************************/

//////////////////////////////////////////////////////////////////////////////
// CCodeNode implementation                                                 //
//////////////////////////////////////////////////////////////////////////////

CCodeNode::CCodeNode(const string &type, const string &name, CSmartPtr<CCodeNode> firstChildNode)
: _Type(type)
, _Name(name)
, _FirstChildNode(firstChildNode)
{
}

void CCodeNode::dump(size_t indent)
{
	string str;
	str.resize(indent,' ');
	str+=_Type+":"+_Name;
	// 		nlwarning(str.c_str());
	if (_NextNode)
		_NextNode->dump(indent);
}

string CCodeNode::getFullName() const
{
	if (_NextNode)
		return _Name+_NextNode->getFullName();
	return _Name;
}

//////////////////////////////////////////////////////////////////////////////
// CCodeTokenNode implementation                                            //
//////////////////////////////////////////////////////////////////////////////

CCodeTokenNode::CCodeTokenNode(const string &type, const string &name, CSmartPtr<CCodeNode> firstChildNode)
: CCodeNode(type, name, firstChildNode)
{
}

void CCodeTokenNode::dump(size_t indent)
{
	string str;
	str.resize(indent,' ');
	str+=_Type+":"+_Name;
	
	if (_FirstChildNode)
		str+=":"+_FirstChildNode->getFullName();
	
	nldebug(str.c_str());
	if (_FirstChildNode)
		_FirstChildNode->dump(indent+1);
	
	if (_NextNode)
		_NextNode->dump(indent);
}

string CCodeTokenNode::getFullName() const
{
	string returnName;
	if (_FirstChildNode)
		returnName+=_FirstChildNode->getFullName();
	if (_NextNode)
		returnName+=_NextNode->getFullName();
	return returnName;
}

/****************************************************************************/
/*                                                                          */
/****************************************************************************/

/*
 * Lexx
	x        the character "x"
	"x"      an "x", even if x is an operator.
	\x       an "x", even if x is an operator.
	[xy]     the character x or y.
	[x-z]    the characters x, y or z.
	[^x]     any character but x.
	.        any character but newline.
	^x       an x at the beginning of a line.
	<y>x     an x when Lex is in start condition y.
	x$       an x at the end of a line.
	x?       an optional x.
	x*       0,1,2, ... instances of x.
	x+       1,2,3, ... instances of x.
	x|y      an x or a y.
	(x)      an x.
	x/y      an x but only if followed by y.
	{xx}     the translation of xx from the
	definitions section.
	x{m,n}   m through n occurrences of x
*/

/* used:
	-		-> from before to after (char)
	[]		-> or for character.
	\c		-> character c
	"c"		-> character c
	{token}	-> match token ?
	()		-> enclosing
	|		-> or
	+		-> card suffixe 1..n
	*		-> card suffixe 0..n
	?		-> card suffixe 0..1
//	{m,n}	-> card suffixe 	m..n
*/


//////////////////////////////////////////////////////////////////////////////
// CRule implementation                                                     //
//////////////////////////////////////////////////////////////////////////////

CRule::CRule(const std::string &name, const string &decl)
: _Name(name)
{
	setDesc(decl);
}

void CRule::setDesc(const string &decl)
{
	const AIVM::CStringSeparator strSep(decl, "|");
	while (strSep.hasNext())
		_subRules.push_back(new CSubRule(this,strSep.get()));
}
	
//////////////////////////////////////////////////////////////////////////////
// CSubRule implementation                                                  //
//////////////////////////////////////////////////////////////////////////////

CSubRule::CSubRule(CRule *parent, const  string &decl)
	:_Parent(parent)
{
	const AIVM::CStringSeparator strSepTokenAction(decl, ",");

	// Tokens.
	if (strSepTokenAction.hasNext())
	{
		const string Tokens=strSepTokenAction.get();

		// Tokens.
		const AIVM::CStringSeparator strSep(Tokens, " ");
		while (strSep.hasNext())
			_tokens.push_back(strSep.get());
	}

	// Action.
	while (strSepTokenAction.hasNext())
		_ExecOpCodes.push_back(strSepTokenAction.get());
	
	nlassert(_tokens.size()>0);
}

//////////////////////////////////////////////////////////////////////////////
// CBasicToken implementation                                               //
//////////////////////////////////////////////////////////////////////////////
	
size_t CBasicToken::initTokens(TTokenList &tokenList, const string &str, size_t firstIndex, size_t lastIndex)
{
	size_t index=firstIndex;
	while (index<lastIndex)
	{
		CSmartPtr<CBasicToken> token=getNewToken(str.at(index));
		if (!token)
			break;
		index=token->init(tokenList, str, index, lastIndex);
	}
	return index;
}

CSmartPtr<CBasicToken> CBasicToken::getNewToken(char c)
{
	if (_BasicTokens.empty())
	{
		insertBasicToken('[', new CBracketToken());

		insertBasicToken('"', new CCharToken());
		insertBasicToken('\\', new CCharToken());
		
		insertBasicToken('{', new CTokenToken());
		insertBasicToken('(', new CParenthesisToken());
		insertBasicToken('|', new COrToken());

		insertBasicToken('*', new CCardToken());
		insertBasicToken('?', new CCardToken());
		insertBasicToken('+', new CCardToken());
	}
	TBasicTokenList::iterator it=_BasicTokens.find(c);
	if (it==_BasicTokens.end())
		return NULL;
	return it->second->createNew();
}

void CBasicToken::insertBasicToken(char id, CSmartPtr<CBasicToken> token)
{
	_BasicTokens.insert(make_pair(id,token));
}

	
//////////////////////////////////////////////////////////////////////////////
// CToken implementation                                                    //
//////////////////////////////////////////////////////////////////////////////
		
CToken::CToken(const  string 	&tokenName, const string &tokenDesc)
	:_tokenName(tokenName), _tokenDesc(tokenDesc)
{
	const size_t index = CBasicToken::initTokens(_Tokens, _tokenDesc, 0, _tokenDesc.size());
	nlassert(index==tokenDesc.size());
}

void CToken::dump() const
{
	const string str = "Token:"+_tokenName+" : "+_tokenDesc;
	nldebug(str.c_str());
	FOREACHC(tokenIt,TTokenContainer,_Tokens)
		(*tokenIt)->dump(0);
}

CTokenTestResult CToken::buildTree(const std::string &code, size_t &index)
{
	CSmartPtr<CCodeNode> masterNode;
	CSmartPtr<CCodeNode> currentNode;
	size_t 	localIndex = index;
				
	FOREACHC(tokenIt,std::vector<CSmartPtr<CBasicToken> >,_Tokens)
	{
		CTokenTestResult res = (*tokenIt)->buildNode(code,localIndex);
		
		if (!res.isValid())
			return CTokenTestResult(NULL, CTokenTestResult::BRULE_INVALID);
		
		CSmartPtr<CCodeNode> const newCodeNode = res.getCode();
		if (!newCodeNode)
			continue;
		
		// Chain result node.
		if (!masterNode)
			masterNode=newCodeNode;
		else
			currentNode->_NextNode=newCodeNode;
		currentNode=newCodeNode;
		while (currentNode->_NextNode)
			currentNode=currentNode->_NextNode;
	}
	
	if (masterNode)
		index=localIndex;
	return masterNode;
}

const std::string &CToken::getName() const
{
	return _tokenName;
}

//////////////////////////////////////////////////////////////////////////////
// CCompiler implementation                                                 //
//////////////////////////////////////////////////////////////////////////////

CCompiler::CCompiler()
{
	string cfgFile = NLMISC::CPath::lookup ("ais_script_compiler.cfg", false);
	if (!cfgFile.empty ())
	{
		NLMISC::CIFile file(cfgFile);
		const int bufferSize = 256;
		char buffer[bufferSize];
		
		while (!file.eof())
		{
			file.getline(buffer, bufferSize);
			if (buffer[0]=='#' || buffer[0]=='\0') // Skip lines beginning with a # and empty lines
				continue;
			string line = buffer;
			const string sep1 = ": ";
			const string sep2 = "=";
			string part1, part2, part3;
			
			string::size_type pos1 = line.find(sep1);
			if (pos1!=string::npos)
			{
				pos1+=sep1.size();
				string::size_type pos2 = line.find(sep2, pos1);
				if (pos2!=string::npos)
				{
					pos2+=sep2.size();
					if (pos1!=string::npos && pos2!=string::npos)
					{
						part1 = line.substr(0, pos1-sep1.size()); // begin to sep1
						part2 = line.substr(pos1, pos2-pos1-sep2.size()); // sep1 to sep2
						part3 = line.substr(pos2); // sep2 to end
					}
				}
			}
			
			if (part1.size()!=0 && part2.size()!=0 && part3.size()!=0)
			{
				if (part1=="token")
				{
					addToken(part2, part3);
				}
				else if (part1=="rule")
				{
					addRule(part2, part3);
				}
			}
			else
			{
				nlwarning("Invalid script line: \"%s\"", line.c_str());
			}
		}
	
	// Basic Opcodes ---------------------------------------------------------
#define REGISTER_OPCODE(__opcode) addOpcode(#__opcode,::AIVM::CScriptVM::__opcode)

	REGISTER_OPCODE(AND);
	REGISTER_OPCODE(OR);
	REGISTER_OPCODE(NOT);
	
	REGISTER_OPCODE(EQ);
	REGISTER_OPCODE(NEQ);
	REGISTER_OPCODE(INF);
	REGISTER_OPCODE(INFEQ);
	REGISTER_OPCODE(SUP);
	REGISTER_OPCODE(SUPEQ);
	
	REGISTER_OPCODE(ADD);
	REGISTER_OPCODE(SUB);
	REGISTER_OPCODE(MUL);
	REGISTER_OPCODE(DIV);
	
	REGISTER_OPCODE(PUSH_ON_STACK);
	REGISTER_OPCODE(POP);
	
	REGISTER_OPCODE(SET_VAR_VAL);
	REGISTER_OPCODE(SET_STR_VAR_VAL);
	REGISTER_OPCODE(SET_CTX_VAR_VAL);
	
	REGISTER_OPCODE(PUSH_VAR_VAL);
	REGISTER_OPCODE(PUSH_STR_VAR_VAL);
	REGISTER_OPCODE(PUSH_CTX_VAR_VAL);
	
//	REGISTER_OPCODE(SET_OTHER_VAR_VAL);
//	REGISTER_OPCODE(SET_OTHER_STR_VAR_VAL);
//	REGISTER_OPCODE(SET_OTHER_CTX_VAR_VAL);
	
//	REGISTER_OPCODE(PUSH_OTHER_VAR_VAL);
//	REGISTER_OPCODE(PUSH_OTHER_STR_VAR_VAL);
//	REGISTER_OPCODE(PUSH_OTHER_CTX_VAR_VAL);
	
	REGISTER_OPCODE(SET_CONTEXT_VAR_VAL);
	REGISTER_OPCODE(SET_CONTEXT_STR_VAR_VAL);
	REGISTER_OPCODE(SET_CONTEXT_CTX_VAR_VAL);
	
	REGISTER_OPCODE(PUSH_CONTEXT_VAR_VAL);
	REGISTER_OPCODE(PUSH_CONTEXT_STR_VAR_VAL);
	REGISTER_OPCODE(PUSH_CONTEXT_CTX_VAR_VAL);
	
	REGISTER_OPCODE(JUMP);
	REGISTER_OPCODE(JE);
	REGISTER_OPCODE(JNE);
	
	REGISTER_OPCODE(PUSH_PRINT_STRING);
	REGISTER_OPCODE(PUSH_PRINT_VAR);
	REGISTER_OPCODE(PUSH_PRINT_STR_VAR);
	REGISTER_OPCODE(PRINT_STRING);
	
	REGISTER_OPCODE(LOG_STRING);
	
	REGISTER_OPCODE(FUNCTION);
	REGISTER_OPCODE(CALL);
	
	REGISTER_OPCODE(PUSH_THIS);
	REGISTER_OPCODE(PUSH_GROUP);
	
	REGISTER_OPCODE(PUSH_STRING);
	REGISTER_OPCODE(ASSIGN_FUNC_FROM);
	
	REGISTER_OPCODE(NATIVE_CALL);
	REGISTER_OPCODE(RAND);
	REGISTER_OPCODE(RANDEND);
	
	REGISTER_OPCODE(RET);
	
	REGISTER_OPCODE(EOP);
	
	REGISTER_OPCODE(ONCHILDREN);
	REGISTER_OPCODE(SWITCH);
	
	REGISTER_OPCODE(INCR);
	REGISTER_OPCODE(DECR);
	
	REGISTER_OPCODE(CONCAT);
	REGISTER_OPCODE(FTOS);
	}
	
	// Natives Funcs ---------------------------------------------------------
	
	registerNativeFunc();
}

CSmartPtr<CSubRuleTracer> CCompiler::buildCodeTree (const string &code) const
{
	size_t tokenIndex=0;
	size_t index=0;
	
	CSmartPtr<CSubRuleTracer> lastInsertedTracer;
	CSmartPtr<CSubRuleTracer> firstInsertedTracer;
	
	// For each token of the code.
	while (index<code.size())
	{
		std::string tokenName;
		std::string textValue;
		const size_t lastIndex=index;
		if (!getNextToken(code, index, tokenName, textValue))
			break;
		
		tokenIndex++;
		
#ifdef DISPLAY_INFOS
		{
			string str(">> TOKEN: ");
			str+=tokenName;
			str+=" "+toString(tokenIndex);
			nldebug(str.c_str());
		}
#endif
		CSmartPtr<CSubRuleTracer> tracer=new CSubRuleTracer(lastIndex, index-1, tokenName, textValue);
		
		if (lastInsertedTracer.isNull())
		{
			lastInsertedTracer=tracer;
			firstInsertedTracer=tracer;
		}
		else
		{
			tracer->checkRules(index-1);
			lastInsertedTracer=tracer;
		}
	}
	return firstInsertedTracer;
}


CSmartPtr<const AIVM::CByteCode> CCompiler::compileCode (const std::vector<std::string> &sourceCodeLines, const string &fullName) const
{
	CSubRuleTracer::_PreviousTracers.clear();
	CSubRuleTracer::_NextTracers.clear();
	
	typedef const std::vector<std::string> TList; // because there a problem with const in the macro.
	string code="{ \n";
	// Concatenates lines, avoid parts after // ..
	FOREACHC(itArg, TList, sourceCodeLines)
	{
		const string &str=*itArg;
		size_t index = str.find("//",0);
		if (index == string::npos)
			code += str;
		else {
			// We have a potential comment. Now check if it is quoted or not
			bool inQuote = false;
			uint i = 0;
			for (;;)
			{
				if ('"' == str[i])
					inQuote = !inQuote;

				if ( !inQuote && ('/' == str[i]) )
				{
					++i;
					if ('/' == str[i])
						break;

					code += '/';
				}
				code += str[i];
				++i;
				if (str.size() == i)
					break;
			}
		}

		code+="\n "; // additional ..
	}
	code+="}";
	return compileCode(code, fullName);
}

CSmartPtr<const AIVM::CByteCode> CCompiler::compileCode (const string &sourceCode, const string &fullName) const
{
	bool debug = NLNET::IService::getInstance()->haveArg('d');
	CSmartPtr<const AIVM::CByteCode> byteCode = compileCodeYacc (sourceCode, fullName, debug, false);

	if (debug)
	{
		// Generate the old byte code
		CSmartPtr<const AIVM::CByteCode> oldbyteCode = compileCodeOld (sourceCode, fullName, debug);
	}

	return byteCode;
}

void CCompiler::dumpByteCode (const string &sourceCode, const string &fullName, CSmartPtr<const AIVM::CByteCode> &byteCode,
							  const string &directory) const
{
	// Build a valid filename
	string tmp = fullName;
	string::size_type pos;
	while ((pos=tmp.find (':')) != string::npos)
		tmp[pos] = '-';

	// Create the bytecode directory
	CFile::createDirectory (directory.c_str ());

	// Save the bytecode
	nlinfo ("saving bytecode for %s", tmp.c_str ());
	FILE *file = fopen ((directory+"/"+tmp+".bin").c_str(), "wb");
	if (file)
	{
		fwrite (&byteCode->_opcodes[0], sizeof(size_t), byteCode->_opcodes.size (), file);
		fclose (file);
	}
	else
		nlwarning ("can't open %s for writing", tmp.c_str ());

	// Create the source directory
	CFile::createDirectory ("iasources");

	// Save the source code
	file = fopen (("iasources/"+tmp+".src").c_str(), "w");
	if (file)
	{
		fwrite (sourceCode.c_str (), sourceCode.size(), 1, file);
		fclose (file);
	}
	else
		nlstopex(("can't open %s for writing", tmp.c_str ()));
}
	
CSmartPtr<const AIVM::CByteCode> CCompiler::compileCodeOld (const string &sourceCode, const string &fullName, bool debug) const
{
	CSmartPtr<AIVM::CByteCode> byteCode = new AIVM::CByteCode(fullName);

	nldebug("script compilation of %s", fullName.c_str());
	string code=sourceCode;
	try
	{ 			
		nldebug(">parsing source code ..");
		CSmartPtr<CSubRuleTracer> tracer=buildCodeTree (code);
		
		if (tracer!=NULL)
		{
			nldebug(">generating code tree ..");
			tracer=tracer->codifyTree 	(); 			// removes ambiguities, at this point a good or bad code is compiled.
			
			nldebug(">generating byte code ..");
			tracer->getHigherParent()->generateCode(byteCode);
		}
		else
		{
			nldebug(">empty source code ..");
		}
		
		nldebug("compilation success. (code size %d)",byteCode->_opcodes.size()*4);
		
		CSmartPtr<const AIVM::CByteCode> tmp=&(*byteCode);

		if (debug)
			dumpByteCode (sourceCode, fullName, tmp, "iaoldbytecode");

		return tmp;
	}
	catch (const Exception &e)
	{
		nlwarning("compilation failed for %s", fullName.c_str());
		nlwarning(e.what());
	}

	return NULL;
}

CSmartPtr<const AIVM::CByteCode> CCompiler::compileCodeYacc (const string &sourceCode, const string &fullName, bool debug, bool win32report) const
{
	CSmartPtr<AIVM::CByteCode> byteCode = new AIVM::CByteCode(fullName);

	if (aiCompile (byteCode->_opcodes, sourceCode.c_str (), fullName.c_str (), win32report))
	{
		CSmartPtr<const AIVM::CByteCode> tmp=&(*byteCode);
		
		if (debug)
			dumpByteCode (sourceCode, fullName, tmp, "ianewbytecode");

		return tmp;
	}
	else if (debug)
		nlstop;

	return NULL;
}
	
void CCompiler::addToken(const string &tokenName, const string &tokenDesc)
{
	CToken *token = new CToken(tokenName, tokenDesc);
	_Tokens.push_back(token);
}

CToken *CCompiler::getToken(const  string 	&tokenName)
{
	FOREACH (tokenIt, TTokenList, _Tokens)
	{
		if ((*tokenIt)->getName()==tokenName)
			return *tokenIt;
	}
	nlassert(false);
	return NULL;
}

/// Helper function
static void displayErrorLinesForIndex(const string &text, size_t &index)
{
	AIVM::CStringSeparator sep(text,"\n\r");
	size_t totalIndex = 0;
	string tmp;
	size_t lineIndex = 0;

	while (sep.hasNext())
	{
		tmp = sep.get();
		const size_t stringSize = tmp.size()+1; // +1 for the \n text.
		if (totalIndex+stringSize>index)
			break;
		totalIndex += stringSize;
		lineIndex++;
	}

	{
		string lineoStr("at line ");
		lineoStr += toString(lineIndex);
		nlwarning(lineoStr.c_str());
	}
	
	nlwarning(tmp.c_str());
	{
		string indexerStr;
		indexerStr.resize(index-totalIndex,' ');
		indexerStr += "^";
		nlwarning(indexerStr.c_str());
	}
}


bool CCompiler::getNextToken(const string &text, size_t &index, string &tokenName, string &textValue) throw (EScriptError)
{
	char c=text.at(index);
	while (c==' '||c=='\n'||c=='\r'||c=='\t') // to avoid blanks, returns and Tabs.
	{
		index++;
		if (index==text.size())
			return false;
		c = text.at(index);
	}
	const size_t firstIndex=index;
	
	static const string unknownS("unknown");
	FOREACH(tokenIt, TTokenList, _Tokens)
	{
		CTokenTestResult res=(*tokenIt)->buildTree(text, index);
		if (res.isValid()) // we found it !!
		{
			tokenName = (*tokenIt)->getName();
			textValue = text.substr(firstIndex, index-firstIndex);
			return true;
		}

	}
	displayErrorLinesForIndex(text, index);
	throw (EScriptError("(Unrecognized pattern)", index));
	nlassert(false);
	return false;
}

void CCompiler::addOpcode(const std::string &str,AIVM::CScriptVM::EOpcode const& op)
{
	nlassert(_Opcodes.find(str)==_Opcodes.end());
	_Opcodes.insert(make_pair(str, op));
}

const string &CCompiler::getOpcodeName(AIVM::CScriptVM::EOpcode const& op)
{
	static string unk("---");
	FOREACHC(opIt, TOpcodeMap, _Opcodes)
	{
		if (opIt->second==op)
			return opIt->first;
	}
	return unk;
}

void CCompiler::addNativeFunc(std::string const& signature, FScrptNativeFunc const& func)
{
	TStringId strId = CStringMapper::map(signature);
	nlassert(_NativeFunctions.find(strId)==_NativeFunctions.end());
	
	_NativeFunctions.insert(make_pair(strId, new CScriptNativeFuncParams(signature, func)));
}

void CCompiler::addDeprecatedNativeFunc(std::string const& signature)
{
	TStringId strId = CStringMapper::map(signature);
	nlassert(_NativeFunctions.find(strId)==_NativeFunctions.end());
	
	_NativeFunctions.insert(make_pair(strId, new CScriptNativeFuncParams(signature, NULL)));
}

CScriptNativeFuncParams* CCompiler::getNativeFunc(const std::string &funcName, const std::string &inparams, const std::string &outparams)
{
	std::string signature;
	TStringId strId;
	TNativeFuncMap::iterator it;
	// Normal params
	signature = funcName + "_" + inparams + "_" + outparams;
	strId = CStringMapper::map(signature);
	it = _NativeFunctions.find(strId);
	if (it!=_NativeFunctions.end())
		return it->second;
	// Variable arguments (va) as input
	signature = funcName + "_v_" + outparams;
	strId = CStringMapper::map(signature);
	it = _NativeFunctions.find(strId);
	if (it!=_NativeFunctions.end())
		return it->second;
	// VA as output
	signature = funcName + "_" + inparams + "_v";
	strId = CStringMapper::map(signature);
	it = _NativeFunctions.find(strId);
	if (it!=_NativeFunctions.end())
		return it->second;
	// VA as input and output
	signature = funcName + "_v_v";
	strId = CStringMapper::map(signature);
	it = _NativeFunctions.find(strId);
	if (it!=_NativeFunctions.end())
		return it->second;
	return NULL;
}

AIVM::CScriptVM::EOpcode CCompiler::getOpcodeAndValue(const std::string &str, std::string &value)
{
	AIVM::CScriptVM::EOpcode opcode=AIVM::CScriptVM::INVALID_OPCODE;
	AIVM::CStringSeparator sep(str," \t");
	if (sep.hasNext())
	{
		TOpcodeMap::iterator it=_Opcodes.find(sep.get());
		if (it!=_Opcodes.end())
			opcode=it->second;
	}
	if (sep.hasNext())
		value=sep.get();
	return opcode;
}

void CCompiler::addRule (const  string 	&ruleName, const string &ruleDesc)
{
	CRule *rule=getRule (ruleName);
	if (rule)
	{
		rule->setDesc(ruleDesc);
		return;
	}
	rule=new CRule(ruleName, ruleDesc);
	_Rules.push_back(rule);
}

CSmartPtr<CRule> CCompiler::getRule (const  string 	&ruleName)
{
	FOREACH(ruleIt, TRuleList, _Rules)
	{
		if ((*ruleIt)->_Name==ruleName)
			return *ruleIt;
	}
	return NULL;
}

CCompiler::TOpcodeMap			CCompiler::_Opcodes;
CCompiler::TRuleList			CCompiler::_Rules;
CCompiler::TTokenList			CCompiler::_Tokens;
CCompiler::TNativeFuncMap		CCompiler::_NativeFunctions;
CCompiler*						CCompiler::_Instance = NULL;
CBasicToken::TBasicTokenList	CBasicToken::_BasicTokens;

//////////////////////////////////////////////////////////////////////////////
// CJumpRememberer implementation                                           //
//////////////////////////////////////////////////////////////////////////////

CJumpRememberer::CJumpRememberer(size_t codeBlockIndex)
: _where(~0)
, _codeBlockIndex(codeBlockIndex)
{
}

//////////////////////////////////////////////////////////////////////////////
// CJumpTable implementation                                                //
//////////////////////////////////////////////////////////////////////////////

CJumpTable::CJumpTable(const CSmartPtr<AIVM::CByteCode> &byteCode)
: _byteCode(byteCode)
{
	_codeOffsets.push_back(0);
}

CJumpTable::~CJumpTable()
{
	newCodeBlock();

	FOREACHC(jumpIt, vector<CJumpRememberer>, _jumps)
	{
		nlassert(jumpIt->_where<_byteCode->_opcodes.size());
		nlassert(jumpIt->_codeBlockIndex<_codeOffsets.size());
		_byteCode->_opcodes[jumpIt->_where]=_codeOffsets[jumpIt->_codeBlockIndex]-jumpIt->_where;
	}
}

void CJumpTable::add(CJumpRememberer jump)
{
	jump._where=_byteCode->_opcodes.size();
	_jumps.push_back(jump);
}

void CJumpTable::newCodeBlock()
{
	_codeOffsets.push_back(_byteCode->_opcodes.size());
}

//////////////////////////////////////////////////////////////////////////////
// CCaseTracer implementation                                               //
//////////////////////////////////////////////////////////////////////////////

CCaseTracer::CCaseTracer(const CSmartPtr<CSubRuleTracer> &tracer, const string &sourceName)
: _tracer(tracer)
{
	const CSubRuleTracer *chldTracer=tracer->getChildForName(s_kw_readConstVar);
	const CSubRuleTracer *valChldTracer=NULL;

	breakable
	{
		if (valChldTracer=chldTracer->getChildForName(s_kw_CHAIN))
		{
			const string &strRef=valChldTracer->_TextValue;
			TStringId strId;
			if ( strRef.at(0)=='"'
				&& strRef.at(0)==strRef.at(strRef.size()-1))
				strId=CStringMapper::map(strRef.substr(1,strRef.size()-2));
			else
				strId=CStringMapper::map(strRef);
			_sortValue=*((size_t*)&strId);
			break;
		}
		if (valChldTracer=chldTracer->getChildForName(s_kw_NUMBER))
		{
			const string &strRef=valChldTracer->_TextValue;
			const float f=(float)atof(strRef.c_str()); 									
			_sortValue=*((size_t*)&f);
			break;
		}
		if (!valChldTracer)
			throw Exception("Invalid case parameter");
	}
	
	chldTracer=tracer->getChildForName(s_kw_lineOrClose);

	CSmartPtr<AIVM::CByteCode> bc=new AIVM::CByteCode(sourceName);
	chldTracer->generateCode(bc);
	_code=&(*bc);
}

typedef std::map<size_t, CSmartPtr<CCaseTracer> > TCaseTracerList;

//////////////////////////////////////////////////////////////////////////////
// CSubRuleTracer implementation                                            //
//////////////////////////////////////////////////////////////////////////////

CSubRuleTracer::CSubRuleTracer(size_t tokenStartIndex, size_t currentTokenIndex, const string &name, const string &textValue)
: _index(0)
, _tokenStartIndex(tokenStartIndex)
, _tokenIndex(currentTokenIndex)
, _Name(name)
, _TextValue(textValue)
, _Valid(false)
{
	updatePreviousNext();
}

CSubRuleTracer::CSubRuleTracer(NLMISC::CSmartPtr<CSubRule> subRule, size_t tokenStartIndex, size_t currentTokenIndex, const std::string &name, const std::string &textValue)
: _index(0)
, _tokenStartIndex(tokenStartIndex)
, _tokenIndex(currentTokenIndex)
, _Name(name)
, _TextValue(textValue)
, _Valid(false)
, _subRule(subRule)
{
	updatePreviousNext();
}

CSubRuleTracer::CSubRuleTracer(const CSubRuleTracer &otherSRT)
: _index(otherSRT._index)
, _tokenStartIndex(otherSRT._tokenStartIndex)
, _tokenIndex(otherSRT._tokenIndex)
, _Name(otherSRT._Name)
, _TextValue(otherSRT._TextValue)
, _Valid(otherSRT._Valid)
, _subRule(otherSRT._subRule)
{
	updatePreviousNext();
}

CSubRuleTracer::~CSubRuleTracer()
{
}
		
void CSubRuleTracer::updatePreviousNext()
{
	// Next registration.
	{
		CSubRuleTracer::TOrderedTracers::iterator it=CSubRuleTracer::_NextTracers.find(_tokenStartIndex);
		if (it==CSubRuleTracer::_NextTracers.end())
		{
			CSubRuleTracer::_NextTracers.insert(make_pair(_tokenStartIndex, CSubRuleTracer::TTracersSet()) );
			it=CSubRuleTracer::_NextTracers.find(_tokenStartIndex);
			nlassert(it!=CSubRuleTracer::_NextTracers.end());
		}
		CSubRuleTracer::TTracersSet &set=it->second;
		set.insert(this);
	}
	
	// Previous registration.
	{
		CSubRuleTracer::TOrderedTracers::iterator it=CSubRuleTracer::_PreviousTracers.find(_tokenIndex);
		if (it==CSubRuleTracer::_PreviousTracers.end())
		{
			CSubRuleTracer::_PreviousTracers.insert(make_pair(_tokenIndex, CSubRuleTracer::TTracersSet()) );
			it=CSubRuleTracer::_PreviousTracers.find(_tokenIndex);
			nlassert(it!=CSubRuleTracer::_PreviousTracers.end());
		}
		CSubRuleTracer::TTracersSet &set=it->second;
		set.insert(this);
	}
}

CSubRuleTracer::TOrderedTracers CSubRuleTracer::_PreviousTracers;
CSubRuleTracer::TOrderedTracers CSubRuleTracer::_NextTracers;
	
// Removers ------------------------------------------------------------------

void CSubRuleTracer::removeParent(CSubRuleTracer *tracer)
{
	FOREACH(itParent, TSubRuleTracerList, _parentTracers)
	{
		if ((*itParent)==tracer)
		{
			_parentTracers.erase(itParent);
			return;
		}
	}
	nlassert(false);
}
void CSubRuleTracer::removeChild(CSubRuleTracer *tracer)
{
	FOREACH(itChild, TSubRuleTracerList, _childTracers)
	{
		if ((*itChild)==tracer)
		{
			_childTracers.erase(itChild);
			return;
		}
	}
	nlassert(false);
}

// ---------------------------------------------------------------------------

void CSubRuleTracer::detachFromEveryBody()
{
	FOREACH(it, TSubRuleTracerList, _parentTracers)
		(*it)->removeChild(this);
	FOREACH(it, TSubRuleTracerList, _childTracers)
		(*it)->removeParent(this);
}

void CSubRuleTracer::iterateToMarkValidTracer()
{
	_Valid=true;
	FOREACH(childIt, TSubRuleTracerList, _childTracers)
		(*childIt)->iterateToMarkValidTracer();
}

CSmartPtr<CSubRuleTracer> CSubRuleTracer::getValidTracer() const
{
	CSmartPtr<CSubRuleTracer> sRT=new 	CSubRuleTracer(*this);

	sRT->_childTracers.reserve(_childTracers.size());
	FOREACHC(childIt, TSubRuleTracerList, _childTracers)
	{
		CSmartPtr<CSubRuleTracer> child=(*childIt)->getValidTracer();
		child->_parentTracers.push_back(sRT);
		sRT->_childTracers.push_back(child);
	}

	return sRT;
}
	
void CSubRuleTracer::flushErrors 	()
{
	FOREACH(itChild, TSubRuleTracerList, _childTracers)
	{
		nlassert((*itChild)->_parentTracers.size()==1); // if there is a problem, we can see it here .. :)
		(*itChild)->flushErrors();
	}
}

void CSubRuleTracer::removeInvalidTracers()
{
	TSubRuleTracerList parentList=_parentTracers; // copy to avoid problems.

	if ( !_Valid
		&& _childTracers.size()>0) // if not valid and not a base tracer.
	{
		detachFromEveryBody();
	}
	
	FOREACH(parentIt, TSubRuleTracerList, parentList)
		(*parentIt)->removeInvalidTracers();
}


CSmartPtr<CSubRuleTracer> CSubRuleTracer::codifyTree()
{
	if (getHigherParent()==this) // an error occured.
	{
		bool errorAppened=false;
		// Check.
		{
			CSubRuleTracer *tracer=this;
			while (tracer!=NULL)
			{
				if (tracer->_parentTracers.size()==0) // if there is a problem, we can see it here .. :)
				{
					errorAppened=true;
					nlwarning("an grammar error appeared that breaks this enclosing: ");
					tracer->dump(10);
				}
				tracer=tracer->getNextLower();
			}
		}
		// Flush errors.
		getHigherParent()->flushErrors();
		if (errorAppened)
			throw Exception("Script Grammar Error");
		return getHigherParent();
	}
	else
	{
		CSmartPtr<CSubRuleTracer> returnTracer=getHigherParent()->getValidTracer();

		// Flush errors.
		returnTracer->flushErrors();
		return returnTracer;
	}

}

CSubRuleTracer *CSubRuleTracer::getNextLower() const
{ 		
	CSubRuleTracer::TOrderedTracers::iterator it=CSubRuleTracer::_NextTracers.find(_tokenIndex+1);
	if (it!=CSubRuleTracer::_NextTracers.end())
	{
		CSubRuleTracer::TTracersSet &set=it->second;
		FOREACH(setIt, CSubRuleTracer::TTracersSet, set)
		{
			if ((*setIt)->_childTracers.size()==0)
				return (*setIt);
		}
	} 		
	return NULL;
}


CSubRuleTracer *CSubRuleTracer::getHigherParent()
{
	if (_parentTracers.size()>0)
		return _parentTracers.back()->getHigherParent();
	return this;
}

// Rule finding can be optimized here with an multimap tree.
void CSubRuleTracer::checkRules(size_t currentToken)
{
	FOREACH(ruleIt, CCompiler::TRuleList, CCompiler::_Rules )
	{
		FOREACH(subRuleIt, TSubRuleList, (*ruleIt)->_subRules)
		{
			TSubRuleTracerList childTracers;
			checkRule(*subRuleIt,0, currentToken, childTracers);
		}
	}
}

void CSubRuleTracer::checkRule(CSubRule *rule, size_t index, size_t currentToken, TSubRuleTracerList &childTracers)
{
	bool equal=false;
	string token=rule->_tokens[rule->_tokens.size()-1-index];
	if (token.at(0)=='+')
		equal=(_Name==token.substr(1,token.size()-1));
	else
		equal=(_Name==token);

	if (!equal) // if not equal, check if its equal to the previous one. (and if this one was 'multi')
	{
		if (index==0) // failed.
			return; 

		token=rule->_tokens[rule->_tokens.size()-index];
		if (token.at(0)!='+')
			return;

		equal=(_Name==token.substr(1,token.size()-1));
		if (!equal) // failed.
			return;
		index--;
	}

	childTracers.push_back(this);
	
	if (index==rule->_tokens.size()-1) // do we match a rule here ?
	{
		CSubRuleTracer *lastTracer=childTracers.back();

		CSmartPtr<CSubRuleTracer> newTracer=new CSubRuleTracer(rule, lastTracer->_tokenStartIndex, currentToken, rule->_Parent->_Name, "");
		
		// we have to copy this vector inverted ..
		newTracer->_childTracers.resize(childTracers.size());
		std::copy(childTracers.begin(), childTracers.end(), newTracer->_childTracers.rbegin());

		FOREACH(childIt, TSubRuleTracerList, childTracers)
			(*childIt)->_parentTracers.push_back(newTracer);

		// Recursively trying to complete others higher rules starting with this new Tracer. :)
		newTracer->checkRules(currentToken);
	}
	else
	{
		// _previousTracers cannot be affected by checkRule calls.

		if ((_tokenStartIndex-1)>=0)
		{
			CSubRuleTracer::TOrderedTracers::iterator it=CSubRuleTracer::_PreviousTracers.find(_tokenStartIndex-1);
			if (it!=CSubRuleTracer::_PreviousTracers.end())
			{
				CSubRuleTracer::TTracersSet &set=it->second;
				FOREACH(setIt, CSubRuleTracer::TTracersSet, set)
					(*setIt)->checkRule(rule, index+1, currentToken, childTracers);
			}
		}
	}
	nlassert(childTracers.back()==(CSubRuleTracer*)this);
	childTracers.pop_back();
}

void CSubRuleTracer::dump(size_t indent) const
{
	string str;
	str.resize(indent,' ');
	str+=_Name;
	str+="("+toString(_tokenStartIndex);
	str+=","+toString(_tokenIndex); 		
	str+="): "+_TextValue;
	
	nldebug(str.c_str());
	FOREACHC(itTracer, TSubRuleTracerList, _childTracers)
		(*itTracer)->dump(indent+1);
}

const CSubRuleTracer *CSubRuleTracer::getChildForName(const string &name) const
{
	FOREACHC(childIt, TSubRuleTracerList, _childTracers)
	{
		if ((*childIt)->_Name==name)
			return *childIt;
	}
	return NULL;
}

size_t CSubRuleTracer::getNbChildNamed(const string &name) const
{
	size_t nb=0;
	if (_Name==name)
		nb=1;

	FOREACHC(childIt, TSubRuleTracerList, _childTracers)
		nb+=(*childIt)->getNbChildNamed(name);
	return nb;
}

void CSubRuleTracer::getSignature(string &signature, bool inOtherWiseOut) const
{
	if (inOtherWiseOut)
	{
		if ( _Name==s_kw_exp
			|| _Name==s_kw_somme
			|| _Name==s_kw_produit
			|| _Name==s_kw_facteur
			|| _Name==s_kw_NAME
			|| _Name==s_kw_NUMBER)
		{
			signature+="f";
			return;
		}
		if ( _Name==s_kw_CHAIN
			|| _Name==s_kw_STRNAME)
		{
			signature+="s";
			return;
		}
		if ( _Name==s_kw_POINT)
		{
			signature.resize(signature.size()-1);
			return;
		}
	}
	else
	{
		if (_Name==s_kw_NAME)
		{
			signature+="f";
			return;
		}
		if (_Name==s_kw_STRNAME)
		{
			signature+="s";
			return;
		} 		
		if ( _Name==s_kw_POINT)
		{
			signature.resize(signature.size()-1);
			return;
		}
		if ( _Name==s_kw_exp
			|| _Name==s_kw_somme
			|| _Name==s_kw_produit
			|| _Name==s_kw_facteur
			|| _Name==s_kw_CHAIN
			|| _Name==s_kw_NUMBER)
		{
			signature+="!";
			return;
		}
	}
	FOREACHC(childIt, TSubRuleTracerList, _childTracers)
		(*childIt)->getSignature(signature,inOtherWiseOut);
}

void CSubRuleTracer::generateCode(CSmartPtr<AIVM::CByteCode> &cByteCode) const
{
	using namespace AIVM;

	nlassert(!cByteCode.isNull());
	typedef vector<CSmartPtr<CByteCode> > TCodePieceList;
	size_t 		codeBlockIndex=0;
	TCodePieceList codePieces;
	vector<size_t> &byteCode=cByteCode->_opcodes;
	CJumpTable 	jumpTable(cByteCode);
	
	size_t randCountMarkIndex=~0;
	
	{
		codePieces.resize(_childTracers.size());

		TCodePieceList::iterator itPiece=codePieces.begin();
		FOREACHC(childIt, TSubRuleTracerList, _childTracers)
		{
			*itPiece=new CByteCode(cByteCode->_sourceName);
			(*childIt)->generateCode(*itPiece);
			++itPiece;
		}

	}
	
	if (!_subRule.isNull())
	{

		FOREACHC(instrIt, vector<string>, _subRule->_ExecOpCodes)
		{
			const string str=*instrIt;
			string param;
			const CScriptVM::EOpcode op=CCompiler::getOpcodeAndValue(str, param);

			if (op!=CScriptVM::INVALID_OPCODE) // it could something else than an instruction.
			{
				switch(op)
				{
				case CScriptVM::JE:
				case CScriptVM::JNE:
				case CScriptVM::JUMP:
					byteCode.push_back(op); // + Jump offset.

					uint32 index;
					NLMISC::fromString(param, index);
					jumpTable.add(CJumpRememberer(index));
					byteCode.push_back(0); // Invalid
					break;
				default:
					byteCode.push_back(op);
					break;
				}
				jumpTable.newCodeBlock();
				continue;
			}
			
			breakable
			{
				if (str.find("Atof")!=string::npos)
				{
					uint32 index;
					NLMISC::fromString(param, index);
					--index;
					string &strRef=_childTracers[index]->_TextValue;
					const float f=(float)atof(strRef.c_str());
					byteCode.push_back(*((size_t*)&f));
					jumpTable.newCodeBlock();
					break;
				}
				
				if (str.find("String")!=string::npos)
				{
					uint32 index;
					NLMISC::fromString(param, index);
					--index;
					string &strRef=_childTracers[index]->_TextValue;
					TStringId strId;
					if ( strRef.at(0)=='"'
						&& strRef.at(0)==strRef.at(strRef.size()-1))
						strId=CStringMapper::map(strRef.substr(1,strRef.size()-2));
					else
						strId=CStringMapper::map(strRef);
					byteCode.push_back(*((size_t*)&strId));
					jumpTable.newCodeBlock();
					break;
				}
				
				if (str.find("CodeAllExceptFirstAndLast")!=string::npos)
				{
					size_t index=0;
					
					FOREACHC(CPIt, TCodePieceList, codePieces)
					{
						index++; 					// Not the first, not the last.
						if ( index==1
							|| index==codePieces.size())
							continue;
						
						if (byteCode.size()==0)
							byteCode=(*CPIt)->_opcodes;
						else
						{
							FOREACHC(codePieceIt, vector<size_t>, (*CPIt)->_opcodes)
								byteCode.push_back(*codePieceIt);
						}
					}
					
					jumpTable.newCodeBlock();
					break;
				}
				
				if (str.find("AllCode")!=string::npos)
				{
					
					FOREACHC(CPIt, TCodePieceList, codePieces)
					{
						if (byteCode.size()==0)
							byteCode=(*CPIt)->_opcodes;
						else
						{
							FOREACHC(codePieceIt, vector<size_t>, (*CPIt)->_opcodes)
								byteCode.push_back(*codePieceIt);
						}
					}
					
					jumpTable.newCodeBlock();
					break;
				}
				
				if (str.find("Code")!=string::npos)
				{
					uint32 index;
					NLMISC::fromString(param, index);
					--index;
					if (byteCode.size()==0)
						byteCode=codePieces[index]->_opcodes;
					else
					{
						FOREACHC(codePieceIt, vector<size_t>, codePieces[index]->_opcodes)
							byteCode.push_back(*codePieceIt);
					}
					jumpTable.newCodeBlock();
					break;
				} 				
				
				if (str.find("NativeCall")!=string::npos)
				{
					string funcName;
					string inParamsSig;
					string outParamsSig;
					// Extract signature
					{
						const CSubRuleTracer*paramTracer=getChildForName(s_kw_NAME);
						funcName=paramTracer->_TextValue;
						
						paramTracer=getChildForName(s_kw_params);
						if (!paramTracer)
							throw Exception("right params not found for the native call "+paramTracer->_TextValue);
						paramTracer->getSignature(inParamsSig, true);
						
						paramTracer=getChildForName(s_kw_tuple);
						if (!paramTracer)
							throw Exception("left params(tuple) not found for the native call "+paramTracer->_TextValue);
						paramTracer->getSignature(outParamsSig, false);
					}
					// Get a function description
					CScriptNativeFuncParams *funcParam=CCompiler::getNativeFunc(funcName, inParamsSig, outParamsSig);
					if (!funcParam)
					{
						string signature = funcName + "_" + inParamsSig + "_" + outParamsSig;
						throw Exception("Critical: unknown function name or bad parameters "+signature);
					}
					
					size_t mode = 0;
					if (funcParam->_va)
						mode |= 1; // :KLUDGE: Hardcoded 1 :TODO: replace with a named constant
					
					byteCode.push_back(CScriptVM::NATIVE_CALL);
				//	byteCode.push_back(*((size_t*)&funcParam));
					TStringId strId;
					strId = CStringMapper::map(funcName);
					byteCode.push_back(*((size_t*)&strId));
					byteCode.push_back(mode);
					strId = CStringMapper::map(inParamsSig);
					byteCode.push_back(*((size_t*)&strId));
					strId = CStringMapper::map(outParamsSig);
					byteCode.push_back(*((size_t*)&strId));
					
					jumpTable.newCodeBlock();
					break;
				}
				
				if (str.find("RandomSeq")!=string::npos)
				{
					const CSubRuleTracer *randomTracers=getChildForName(s_kw_expeclose); 					
					const size_t nbTracers=randomTracers->_childTracers.size()-2; // LA and RA are omitted.
					
					byteCode.push_back(CScriptVM::RAND);
					byteCode.push_back(nbTracers);
					
					byteCode.push_back(CScriptVM::JUMP);
					jumpTable.add(CJumpRememberer(nbTracers+1)); // Final Jump Position.
					byteCode.push_back(0); // Invalid
					
					size_t tracerInd=nbTracers;
					while (tracerInd>0)
					{
						byteCode.push_back(CScriptVM::JUMP);
						jumpTable.add(CJumpRememberer(tracerInd));
						byteCode.push_back(0); // Invalid
						tracerInd--;
					}
					byteCode.push_back(CScriptVM::RANDEND);
					jumpTable.newCodeBlock();
					
					{
						codePieces.resize(0);
						codePieces.resize(randomTracers->_childTracers.size());
						
						TCodePieceList::iterator itPiece=codePieces.begin();
						FOREACHC(childIt, TSubRuleTracerList, randomTracers->_childTracers)
						{
							*itPiece=new AIVM::CByteCode(cByteCode->_sourceName);
							(*childIt)->generateCode(*itPiece);
							++itPiece;
						}
					}
					
					tracerInd=nbTracers;
					while (tracerInd>0)
					{
						tracerInd--;
						if (byteCode.size()==0)
							byteCode=codePieces[tracerInd+1]->_opcodes;
						else
						{
							FOREACHC(codePieceIt, vector<size_t>, codePieces[tracerInd+1]->_opcodes)
								byteCode.push_back(*codePieceIt);
						}
						byteCode.push_back(CScriptVM::RET);
						jumpTable.newCodeBlock();
					}
					break;
				}
				
				if (str.find("SwitchSeq")!=string::npos)
				{
					// first, build a list of case statements.
					TCaseTracerList caseTracers;
					FOREACHC(chldIt, TSubRuleTracerList, _childTracers)
					{
						if ((*chldIt)->_Name==s_kw_case)
						{
							CSmartPtr<CCaseTracer> ptr=new CCaseTracer(*chldIt, cByteCode->_sourceName);
							caseTracers.insert(make_pair(ptr->_sortValue, ptr));
						}
					}
					
					byteCode.push_back(CScriptVM::SWITCH);
					byteCode.push_back(caseTracers.size());
					
					jumpTable.add(CJumpRememberer(caseTracers.size()+2)); // Final Jump Position.
					byteCode.push_back(0); // Invalid
					
					{
						size_t index=2;
						FOREACHC(caseIt, TCaseTracerList, caseTracers)
						{
							byteCode.push_back(caseIt->first);
							jumpTable.add(CJumpRememberer(index)); // Final Jump Position.
							byteCode.push_back(0); // Invalid
							index++;
						}
					}
					jumpTable.newCodeBlock();
					
					FOREACHC(caseIt, TCaseTracerList, caseTracers)
					{
						FOREACHC(codePieceIt, vector<size_t>, caseIt->second->_code->_opcodes)
							byteCode.push_back(*codePieceIt);
						byteCode.push_back(CScriptVM::RET);
						jumpTable.newCodeBlock();
					} 					
					break;
				} 				
				throw Exception("Unrecognized keyword "+str);
			}
		}
	}
}

/*
	if (outputFilename != "") output the byte code in a file
*/
bool compileExternalScript (const char *filename, const char *outputFilename)
{
	// Read the content
	bool result = false;
	FILE *file = fopen (filename, "r");
	if (file)
	{
		string content;
		char buffer[512];
		int read;
		while ((read = (int)fread (buffer, 1, sizeof(buffer)-1, file)) == sizeof(buffer)-1)
		{
			buffer[read] = 0;
			content += buffer;
		}
		buffer[read] = 0;
		content += buffer;
		CSmartPtr<const AIVM::CByteCode> byteCode = CCompiler::getInstance ().compileCodeYacc (content, filename, NLNET::IService::getInstance()->haveArg('d'), true);
		fclose (file);
		if (byteCode)
		{
			// Save the byte code ?
			if (strcmp (outputFilename, "") != 0)
			{
				FILE *output = fopen (filename, "wb");
				if (output)
				{
					size_t size = byteCode->_opcodes.size()*sizeof(size_t);
					if (fwrite (&byteCode->_opcodes[0], 1, size, output) == size)
						result = true;
					else
						nlwarning ("Error while writing %s", outputFilename);
					fclose (output);
				}
				else
					nlwarning ("Can't open the file %s for writing", outputFilename);
			}
			else
				result = true;
		}
	}
	else
		nlwarning ("Can't open the file %s for reading", filename);
	return result;
}

//////////////////////////////////////////////////////////////////////////////
/*
*/

}; // namespace

NLMISC_COMMAND(listNativeFunctions, "list native functions of that AIS", "")
{
	CLogStringWriter stringWriter(&log);
	AICOMP::CCompiler::TNativeFuncMap const& funcs = AICOMP::CCompiler::getInstance().getFunctionList();
	std::deque<std::string> names;
	FOREACHC(itFunc, AICOMP::CCompiler::TNativeFuncMap, funcs)
		names.push_back(CStringMapper::unmap(itFunc->first));
	std::sort(names.begin(), names.end());
	FOREACHC(itName, std::deque<std::string>, names)
		log.displayNL("%s", itName->c_str());
	return true;
}
