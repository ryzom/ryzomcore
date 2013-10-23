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

#ifndef __SCRIPT_COMPILER__
#define __SCRIPT_COMPILER__

#include "script_vm.h"

// Forward declarations
class CStateInstance;

namespace AICOMP
{

/****************************************************************************/
/* Compiler classes                                                         */
/****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
// Helper classes                                                           //
//////////////////////////////////////////////////////////////////////////////

struct EScriptError : public NLMISC::Exception
{
	EScriptError(const std::string &text, size_t index);
	size_t getIndex() const;
	size_t _Index;
};

//////////////////////////////////////////////////////////////////////////////
// Native functions access                                                  //
//////////////////////////////////////////////////////////////////////////////

typedef void (*FScrptNativeFunc)(CStateInstance* si, AIVM::CScriptStack& stack);

class CScriptNativeFuncParams : public NLMISC::CRefCount
{
public:
	CScriptNativeFuncParams(const std::string &str, FScrptNativeFunc func);
	virtual ~CScriptNativeFuncParams() { }
	size_t _nbInParams;
	size_t _nbOutParams;
	bool _va;
	FScrptNativeFunc _func;
};

//////////////////////////////////////////////////////////////////////////////
// Node classes                                                             //
//////////////////////////////////////////////////////////////////////////////

class CCodeNode : public NLMISC::CRefCount
{
public:
	CCodeNode(const std::string &type, const std::string &name, NLMISC::CSmartPtr<CCodeNode> firstChildNode=NULL);
	
	virtual	void dump(size_t indent=0);
	virtual	std::string	getFullName() const;
	
	virtual ~CCodeNode() { }
	std::string	_Type;
	std::string	_Name;
	NLMISC::CSmartPtr<CCodeNode>	_NextNode;
	NLMISC::CSmartPtr<CCodeNode>	_FirstChildNode;
};

//////////////////////////////////////////////////////////////////////////////

class CCodeTokenNode : public CCodeNode
{
public:
	CCodeTokenNode(const std::string &type, const std::string &name, NLMISC::CSmartPtr<CCodeNode> firstChildNode);
	virtual ~CCodeTokenNode() { }
	
	void dump(size_t indent=0);
	virtual std::string getFullName() const;
};

//////////////////////////////////////////////////////////////////////////////
// Token classes                                                            //
//////////////////////////////////////////////////////////////////////////////

class CTokenTestResult
{
public:
	enum TBasicTokenTestRes
	{
		BRULE_INVALID = 0,
		BRULE_VALID,
	};
	
	CTokenTestResult();
	CTokenTestResult(const NLMISC::CSmartPtr<CCodeNode> &codeNode);
	CTokenTestResult(const NLMISC::CSmartPtr<CCodeNode> &codeNode, TBasicTokenTestRes res);
	virtual ~CTokenTestResult();
	bool isValid() const;
	CCodeNode *getCode () const;
private:
	TBasicTokenTestRes _res;
	NLMISC::CSmartPtr<CCodeNode> _codeNode;
};

//////////////////////////////////////////////////////////////////////////////

class CBasicToken : public NLMISC::CRefCount
{
public:
	typedef std::map<char,NLMISC::CSmartPtr<CBasicToken> > TBasicTokenList;
public:
	virtual ~CBasicToken() { }

	typedef	std::vector<NLMISC::CSmartPtr<CBasicToken> > TTokenList;
	
	virtual size_t init (TTokenList &tokenList, const std::string &str, size_t firstIndex, size_t lastIndex) = 0;
	virtual CBasicToken *createNew () const = 0;
	virtual void dump(size_t indent) const = 0;				
	virtual CTokenTestResult buildNode(const std::string &code, size_t &index) const = 0;
	
	static size_t initTokens(TTokenList &tokenList, const std::string &str, size_t firstIndex, size_t lastIndex);
	
	static NLMISC::CSmartPtr<CBasicToken> getNewToken(char c);
	static void	insertBasicToken(char id, NLMISC::CSmartPtr<CBasicToken> token);
	static TBasicTokenList _BasicTokens;
};

//////////////////////////////////////////////////////////////////////////////

class CToken : public NLMISC::CRefCount
{
public:
	CToken(const std::string &tokenName, const std::string &tokenDesc);
	
	typedef	std::vector<NLMISC::CSmartPtr<CBasicToken> > TTokenContainer;
	
	void dump() const;
	
	virtual ~CToken() { }
	
	CTokenTestResult buildTree(const std::string &code, size_t &index);
	
	const std::string &getName() const;
	
	std::string _tokenName;
	std::string _tokenDesc;
	std::vector<NLMISC::CSmartPtr<CBasicToken> > _Tokens;
};

//////////////////////////////////////////////////////////////////////////////
// Rule classes                                                             //
//////////////////////////////////////////////////////////////////////////////

class CSubRule;

typedef std::vector<NLMISC::CSmartPtr<CSubRule> > TSubRuleList;

class CRule : public NLMISC::CRefCount
{
public:
	CRule(const	std::string	&name, const	std::string	&decl);
	virtual ~CRule() { }

	void setDesc(const std::string &decl);
		
	TSubRuleList	_subRules;
	std::string		_Name;
};

//////////////////////////////////////////////////////////////////////////////

class CSubRule : public NLMISC::CRefCount
{
public:
	CSubRule(CRule *parent, const std::string &decl);
	
	std::vector<std::string> _tokens;
	NLMISC::CSmartPtr<CRule> _Parent;
	
	std::vector<std::string> _ExecOpCodes;
};		

//////////////////////////////////////////////////////////////////////////////

class CSubRuleTracer;
typedef std::vector<NLMISC::CSmartPtr<CSubRuleTracer> > TSubRuleTracerList;

class CSubRuleTracer : public NLMISC::CRefCount, public NLMISC::CDbgRefCount<CSubRuleTracer>
{
public:
	CSubRuleTracer(size_t tokenStartIndex, size_t currentTokenIndex, const std::string &name, const std::string &textValue);
	CSubRuleTracer(NLMISC::CSmartPtr<CSubRule> subRule, size_t tokenStartIndex, size_t currentTokenIndex, const std::string &name, const std::string &textValue);
	CSubRuleTracer(const CSubRuleTracer &otherSRT);			
			
	virtual ~CSubRuleTracer();

	NLMISC::CSmartPtr<CSubRuleTracer> getValidTracer() const;
		
	void updatePreviousNext();
	
	void checkRule(CSubRule *rule, size_t index, size_t currentToken, TSubRuleTracerList &childTracers);
	void checkRules(size_t currentToken);
	
	NLMISC::CSmartPtr<CSubRuleTracer> codifyTree();

	void generateCode(NLMISC::CSmartPtr<AIVM::CByteCode> &cByteCode) const;

	/// Returns a chain specifying the params this way: ffsf for (float, float, string, float)
	const CSubRuleTracer *getChildForName(const std::string &name) const;
	void getSignature(std::string &signature, bool inOtherWiseOut) const;
	size_t getNbChildNamed(const std::string &name) const;
	
	CSubRuleTracer *getHigherParent	();
	CSubRuleTracer *getNextLower() const;
	
	void removePrevious(CSubRuleTracer *tracer);
	void removeNext    (CSubRuleTracer *tracer);
	void removeParent  (CSubRuleTracer *tracer);
	void removeChild   (CSubRuleTracer *tracer);

	void detachFromEveryBody();
	
	void iterateToMarkValidTracer();		
	void removeInvalidTracers();
	
	void flushErrors();

	void dump(size_t indent) const;
	
	
	std::string	_Name;
	std::string	_TextValue;
	
	size_t	_index; ///< Index in the subrule.
	
	size_t	_tokenStartIndex;
	size_t	_tokenIndex;

	bool	_Valid;
	
	//////////////////////////////////////////////////////////////////////////////
	//	All Links are Directs.
	
	NLMISC::CSmartPtr<CSubRule> _subRule;

	typedef std::set<NLMISC::CSmartPtr<CSubRuleTracer> > TTracersSet;
	typedef std::map<size_t, TTracersSet> TOrderedTracers;

	static TOrderedTracers _PreviousTracers;
	static TOrderedTracers _NextTracers;

	TSubRuleTracerList _parentTracers;
	TSubRuleTracerList _childTracers;
};

//////////////////////////////////////////////////////////////////////////////

class CCompiler
{
public:	
	typedef std::map<NLMISC::TStringId, NLMISC::CSmartPtr<CScriptNativeFuncParams> > TNativeFuncMap;
	typedef std::vector<NLMISC::CSmartPtr<CRule> > TRuleList;
	typedef std::map<std::string, AIVM::CScriptVM::EOpcode> TOpcodeMap;
public:
	CCompiler();
	virtual ~CCompiler() { }
	
	NLMISC::CSmartPtr<AIVM::CByteCode const> compileCode(std::string const& sourceCode, std::string const& fullName) const;
	NLMISC::CSmartPtr<AIVM::CByteCode const> compileCode(std::vector<std::string> const& sourceCodeLines, std::string const& fullName) const;
	
	// New compiler using lex & yacc
	NLMISC::CSmartPtr<AIVM::CByteCode const> compileCodeYacc(std::string const& sourceCode, std::string const& fullName, bool dump, bool win32report) const;
	
	// Old compiler
	NLMISC::CSmartPtr<AIVM::CByteCode const> compileCodeOld(std::string const& sourceCode, std::string const& fullName, bool dump) const;
	
	// Dump the source and the bytecode for debugging
	void dumpByteCode(std::string const& sourceCode, std::string const& fullName, NLMISC::CSmartPtr<AIVM::CByteCode const> &byteCode, std::string const& directory) const;
	
	NLMISC::CSmartPtr<CSubRuleTracer> buildCodeTree(const std::string &code) const;
	
	static void registerNativeFunc	();
	
	static void addToken(std::string const& tokenName, std::string const& tokenDesc);
	static void addRule(std::string const& ruleName, std::string const& ruleDesc);
	static void addOpcode(std::string const& str, AIVM::CScriptVM::EOpcode const& op);
	static void addNativeFunc(std::string const& signature, FScrptNativeFunc const& foo);
	static void addDeprecatedNativeFunc(std::string const& signature);
	
	static CToken*							getToken			(std::string const& tokenName);
	static NLMISC::CSmartPtr<CRule>			getRule				(std::string const& ruleName);
	static bool								getNextToken		(std::string const& text, size_t& index, std::string& tokenName, std::string& textValue) throw (EScriptError);
	static std::string const&				getOpcodeName		(AIVM::CScriptVM::EOpcode const& op);
	static AIVM::CScriptVM::EOpcode			getOpcodeAndValue	(std::string const& str, std::string& value);
	static CScriptNativeFuncParams*			getNativeFunc		(std::string const& funcName, std::string const& inparams, std::string const& outparams);
	
	static CCompiler& getInstance()
	{
		if (!_Instance)
			_Instance = new CCompiler();
		return *_Instance;
	}
	static TNativeFuncMap const& getFunctionList() { return _NativeFunctions; }
	
	typedef	std::vector<NLMISC::CSmartPtr<CToken> >	TTokenList;
	
	static TRuleList _Rules;
	static TOpcodeMap _Opcodes;
private:
	static TNativeFuncMap	_NativeFunctions;
	static TTokenList		_Tokens;		
	static CCompiler*		_Instance;
};

/****************************************************************************/
/* Inlined functions                                                        */
/****************************************************************************/

inline
EScriptError::EScriptError(const std::string &text, size_t index)
: NLMISC::Exception(text)
, _Index(index)
{
}
inline
size_t EScriptError::getIndex() const
{
	return _Index;
}

inline
CTokenTestResult::CTokenTestResult()
: _res(BRULE_VALID)
{
}
inline
CTokenTestResult::CTokenTestResult(const NLMISC::CSmartPtr<CCodeNode> &codeNode)
: _res(BRULE_VALID)
, _codeNode(codeNode)
{
}
inline
CTokenTestResult::CTokenTestResult(const NLMISC::CSmartPtr<CCodeNode> &codeNode, TBasicTokenTestRes res)
: _res(BRULE_VALID)
, _codeNode(codeNode)
{
}
inline
CTokenTestResult::~CTokenTestResult()
{
}
inline
bool CTokenTestResult::isValid() const
{
	return _res==BRULE_VALID;
}
inline
CCodeNode *CTokenTestResult::getCode () const
{
	return _codeNode;
}

/****************************************************************************/
/* For LEX & YACC                                                           */
/****************************************************************************/

#define AICOMP_MAX_SIGNATURE 32

// Yacc base node
struct CBaseYacc
{
	char				Signature[AICOMP_MAX_SIGNATURE];
	bool				isFloat () const { return strcmp (Signature, "f") == 0; }
	bool				isString () const { return strcmp (Signature, "s") == 0; }
	bool				isBool () const { return strcmp (Signature, "b") == 0; }
	const char			*getType () const { return isFloat ()?"float":isBool ()?"bool":isString ()?"string":"void"; }
};

// Yacc node for current byte code
struct CByteCodeYacc : public CBaseYacc
{
	std::vector<size_t>	*ByteCode;
};

// Yacc node for a single opcode
struct COpcodeYacc : public CBaseYacc
{
	int					Line;
	size_t				Opcode;
};

// Yacc node for operators
struct COperatorYacc : public COpcodeYacc
{
	const char			*Operator;
};

// Yacc node for byte code tree node, keeping the tree structure
struct CByteCodeListYacc : public CBaseYacc
{
	std::list<std::vector<size_t> * > *ByteCodeList;
};

// Case structure
struct CCase : public CBaseYacc
{
	int					Line;
	size_t				Case;
	std::vector<size_t>	*ByteCode;
};

// Yacc node for case
struct CCaseYacc : public CBaseYacc
{
	CCase				*Case;
};

// Yacc node for switch
struct CSwitchYacc : public CBaseYacc
{
	std::map<size_t, CCase *>	*Cases;
};

}; // namespace
	
// From Lex&Yacc
extern bool aiCompile (std::vector<size_t> &dest, const char *script, const char *scriptName, bool win32report);

#endif
