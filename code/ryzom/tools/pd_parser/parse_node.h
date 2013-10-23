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

#ifndef RY_PD_PARSE_NODE_H
#define RY_PD_PARSE_NODE_H

#include "tokenizer.h"
#include "cpp_output.h"
#include "templatizer.h"

extern CTokenizer						Tokenizer;

extern std::string						getFunctionPrefix;
extern std::string						setFunctionPrefix;
extern std::string						newFunction;
extern std::string						deleteFunction;

extern std::string						indexVariable;
extern std::string						valueVariable;
extern std::string						keyVariable;
extern std::string						objectVariable;

extern std::string						staticCreateFunction;
extern std::string						staticRemoveFunction;
extern std::string						staticSetUserFactoryFunction;
extern std::string						staticLoadFunction;
extern std::string						staticUnloadFunction;
extern std::string						staticGetFunction;

extern std::string						initFunction;
extern std::string						destroyFunction;
extern std::string						registerFunction;
extern std::string						registerAttributesFunction;
extern std::string						unregisterFunction;
extern std::string						unregisterAttributesFunction;
extern std::string						fetchFunction;
extern std::string						setParentFunction;
extern std::string						setUnnotifiedParentFunction;
extern std::string						getTableFunction;
extern std::string						unlinkFunction;

extern std::string						staticInitFactoryFunction;
extern std::string						staticFactoryFunction;
extern std::string						staticFetchFunction;
extern std::string						staticInitFunction;

extern std::string						logStartFunction;
extern std::string						logStopFunction;


class CFileNode;
class CDbNode;
class CTypeNode;
class CClassNode;
class CIndexNode;
class CEnumNode;
class CDimensionNode;
class CLogMsgNode;

//
class CParseNode
{
public:

	CParseNode() : Parent(NULL), FileNode(NULL), DbNode(NULL), Env(NULL)	{}

	virtual ~CParseNode()
	{
		uint i;
		for (i=0; i<Nodes.size(); ++i)
			delete Nodes[i];
	}

	CParseNode*					Parent;
	std::vector<CParseNode*>	Nodes;
	CTokenizer::CToken			StartToken;
	std::string					Name;
	std::string					Description;



	///
	virtual	bool	prolog()	{ return true; }
	virtual	bool	epilog()	{ return true; }

	///
	bool			execute()
	{
		if (!prolog())
			return false;

		uint	i;
		for (i=0; i<Nodes.size(); ++i)
			if (!Nodes[i]->execute())
				return false;

		return epilog();
	}



	/// Issue error
	void			error(const std::string &errMsg, const char *errType = "semantic")
	{
		Tokenizer.error(StartToken, errType, errMsg.c_str() );
	}

	///
	CParseNode*		getNode(const std::string &name)
	{
		uint	i;
		for (i=0; i<Nodes.size(); ++i)
			if (Nodes[i]->Name == name)
				return Nodes[i];
		return NULL;
	}

	///
	void			getFileLine(uint &line, uint &col, std::string &file)
	{
		Tokenizer.getFileLine(StartToken, line, col, file);
	}

	///
	CFileNode*		getFileNode();
	CDbNode*		getDbNode();

	CFileNode*		FileNode;
	CDbNode*		DbNode;

	CCppOutput&		hOutput();
	CCppOutput&		cppOutput();
	CCppOutput&		inlineOutput();

	CTypeNode*		getTypeNode(const std::string &name, bool genError = true);
	CEnumNode*		getEnumNode(const std::string &name, bool genError = true);
	CDimensionNode*	getDimensionNode(const std::string &name, bool genError = true);
	CIndexNode*		getIndexNode(const std::string &name, bool genError = true);
	CClassNode*		getClassNode(const std::string &name, bool genError = true);

	CTemplatizerEnv*	Env;
	template<typename T>
	void			setEnv(const std::string& var, const T& val)
	{
		nlassert(Env != NULL);
		Env->set(var, val);
	}
	void			define(const std::string& var)
	{
		nlassert(Env != NULL);
		Env->set(var, 1);
	}
	void			define(bool isdef, const std::string& var)
	{
		nlassert(Env != NULL);
		if (isdef)
			Env->set(var, 1);
	}
};


//
//
//
class CDbNode : public CParseNode
{
public:
	CDbNode() : DbXml(false), DbSummary(false)	{}

	//
	bool			addTypeNode(const std::string &name, const std::string &displayName = std::string(""), const std::string &defaultValue = std::string(""));

	//
	std::vector<CFileNode*>		FileNodes;
	std::vector<CTypeNode*>		TypeNodes;
	std::vector<CClassNode*>	ClassNodes;
	std::vector<CLogMsgNode*>	LogNodes;
	CCppOutput					DbXml;
	CCppOutput					DbHpp;
	CCppOutput					DbHppInline;
	CCppOutput					DbCpp;

	CCppOutput					DbSummary;

	std::string					MainFile;

	std::string					Pch;

	std::vector<std::string>	xmlDescription;
	CFunctionGenerator			initDb;
	CFunctionGenerator			readyDb;
	CFunctionGenerator			updateDb;
	CFunctionGenerator			releaseDb;
	CFunctionGenerator			logChatDb;
	CFunctionGenerator			logTellDb;

	std::set<std::string>		Implemented;

	///
	virtual bool	prolog();
	virtual bool	epilog();

	void			pass1();
	void			pass2();
	void			pass3();
	void			pass4();

	void			buildClassOrder(std::vector<CClassNode*>& classesOrder, std::vector<CFileNode*>& filesOrder);

	void			generateClassesDeclaration();
	void			generateIncludes(std::vector<CFileNode*>& filesOrder);

	void			generateClassesContent(std::vector<CClassNode*>& classesOrder);

	void			generateLogContent();

	std::string		getDbFile()				{ return MainFile.empty() ? Name : MainFile; }

	// get file path from this file
	std::string		getFileNoExtPath(const std::string& file);
};

//
//
//
class CIncludeNode;
class CFileNode : public CParseNode
{
public:

	CFileNode() : SeparatedFlag(false), IncludeStandard(false), IncludePDSLib(false), IncludeDbFile(false), Generate(true)	{ }

	std::string		IncludeAs;

	bool			SeparatedFlag;

	CCppOutput		Hpp;
	CCppOutput		HppInline;
	CCppOutput		Cpp;

	bool						IncludeStandard;
	bool						IncludePDSLib;
	bool						IncludeDbFile;
	std::vector<CIncludeNode*>	IncludeNodes;

	bool						Generate;

	std::set<CFileNode*>		Dependencies;

	void	checkDependencies(std::set<CFileNode*> &beingChecked,
							  std::set<CFileNode*> &checkedFiles,
							  std::vector<CFileNode*> &filesOrder);

	///
	virtual bool	prolog();
	virtual bool	epilog();

	//
	virtual bool	generateProlog();
	virtual bool	generateEpilog();

	//
	void			writeFile();

	// get file path from this file
	std::string		getFileNoExtPath(const std::string& file);
};

//
//
//
class CIncludeNode : public CParseNode
{
public:

	///
	virtual bool	prolog();
};

//
//
//
class CUsePchNode : public CParseNode
{
public:

	///
	virtual bool	prolog();
};

//
//
//
class CCppCodeNode : public CParseNode
{
public:

	std::string		RawCode;

	///
	virtual bool	prolog();
};

//
//
//
class CTypeNode : public CParseNode
{
public:
	CTypeNode() :
		ToCppType(NULL), 
		ToStorageType(NULL), 
		ExternFlag(false),
		InternFlag(false),
		Id(0) 
	{
	}

	std::string		CppType;
	std::string		StorageType;
	std::string		DisplayName;

	CParseNode*		ToCppType;
	CParseNode*		ToStorageType;

	bool			ExternFlag;
	bool			InternFlag;

	std::string		Temp;

	uint32			Size;
	uint			Id;

	std::string		DefaultValue;

	virtual bool		isEnum()		{ return false; }
	virtual bool		isDimension()	{ return false; }
	virtual bool		isIndex()		{ return false; }

	virtual std::string	checkCode(const std::string& var)	{ return ""; }

	std::string			storageToCpp()
	{
		if (ToCppType != NULL)
			return "__pds_cnv_type_"+NLMISC::toString(Id)+"_s2c";
		else
			return "("+CppType+")";
	}

	std::string			cppToStorage()
	{
		if (ToStorageType != NULL)
			return "__pds_cnv_type_"+NLMISC::toString(Id)+"_c2s";
		else
			return "("+StorageType+")";
	}



	std::string			castToCpp(const std::string& var)
	{
		if (CppType != StorageType)
		{
			return storageToCpp()+"("+var+")";
		}
		else
		{
			return var;
		}
	}

	std::string			castToStorage(const std::string& var)
	{
		if (CppType != StorageType)
		{
			return cppToStorage()+"("+var+")";
		}
		else
		{
			return var;
		}
	}

	std::string			getCppType()
	{
		if (CppType == "CEntityId")
			return "NLMISC::CEntityId";
		else if (CppType == "CSheetId")
			return "NLMISC::CSheetId";
		else
			return CppType;
	}




	std::string			castToPDS(const std::string& var)
	{
		if (isEnum())
		{
			return "(uint32)"+var;
		}
		else if (ExternFlag)
		{
			return "("+StorageType+")"+var;
		}
		else
		{
			//return "("+getName()+")";
			return castToStorage(var);
		}
	}

	std::string			castFromUser(const std::string& var)
	{
		return "("+getName()+")"+var;
	}



	virtual std::string	getName()		{ return DisplayName.empty() ? Name : DisplayName; }

	std::string			getDefaultValue()
	{
		if (DefaultValue.empty())
		{
			if (CppType == "CEntityId")	return "NLMISC::CEntityId::Unknown";
			if (CppType == "CSheetId")	return "NLMISC::CSheetId::Unknown";
			return castFromUser("0");
		}
		else
		{
			return DefaultValue;
		}
	}



	virtual std::string	getPrintfFmt()
	{
		if (CppType == "CEntityId" || CppType == "CSheetId")
			return "%s";
		if (CppType == "double" || CppType == "float")
			return "%f";
		if (CppType == "sint64")
			return "%\"NL_I64\"d";
		if (CppType == "uint64")
			return "%\"NL_I64\"u";
		if (CppType == "sint32" || CppType == "sint16" || CppType == "sint8")
			return "%d";
		if (CppType == "uint32" || CppType == "uint16" || CppType == "uint8")
			return "%u";
		if (CppType == "char" || CppType == "ucchar")
			return "%c";
		if (CppType == "bool")
			return "%s";
		return "%d";
	}

	virtual std::string getPrintfVal(const std::string& var)
	{
		if (CppType == "CEntityId" || CppType == "CSheetId")
			return var+".toString()";
		if (CppType == "double" || CppType == "float" ||
			CppType == "sint64" || CppType == "uint64" ||
			CppType == "sint32" || CppType == "sint16" || CppType == "sint8" ||
			CppType == "uint32" || CppType == "uint16" || CppType == "uint8" ||
			CppType == "char" || CppType == "ucchar")
			return var;
		if (CppType == "bool")
			return "("+var+" ? \"true\" : \"false\")";
		return "(uint32)"+var;
	}


	///
	virtual bool	prolog();

	///
	virtual bool	generateContent();

	virtual void	generateApplyCode(CClassGenerator::SMethodId& method, const std::string& token, const std::string& value)
	{
		method.add("__pdr.pop("+token+", "+value+");");
	}

	virtual void	generateStoreCode(CClassGenerator::SMethodId& method, const std::string& token, const std::string& value)
	{
		method.add("__pdr.push("+token+", "+value+");");
	}
};







//
//
//
class CDeclarationNode;
class CCallContext;

enum TDeclarationType
{
	SimpleType,
	SimpleClass,
	ForwardRef,
	BackRef,
	ArrayType,
	ArrayClass,
	ArrayRef,
	Set
};

struct CColumn
{
	std::string				Name;
	uint32					ByteSize;
	TDeclarationType		Type;
	std::string				TypeStr;
	uint					TypeId;
};

class CClassNode : public CParseNode
{
public:

	CClassNode() : 
		IsBackReferenced(false),
		HasInheritance(false),
		IsInSet(false),
		IsInArray(false),
		IsInArrayRef(false),
		MappedFlag(false),
		DerivatedFlag(false),
		HasParent(false),
		ParentIsHidden(false),
		ForceReference(false),
		PDSMapped(false),
		MapClass(NULL),
		Columns(-1),
		Id(0),
		HasRowAccess(false),
		HasTableAccess(false),
		indexUsedInInit(false),
		indexUsedInDestroy(false),
		indexUsedInFetch(false),
		tableAndRowIndicesUsedInFetch(false),
		indexUsedInRegister(false),
		indexUsedInUnregister(false),
		indexUsedInNotifyInit(false),
		indexUsedInNotifyRelease(false)
	{
	}

	std::string					Inherited;
	std::string					ClassKey;
	std::string					Implements;
	std::set<CClassNode*>		Dependencies;

	std::vector<std::string>	ChildrenClasses;
	bool						IsBackReferenced;
	bool						HasInheritance;
	bool						IsInSet;
	bool						IsInArray;
	bool						IsInArrayRef;
	bool						MappedFlag;
	bool						DerivatedFlag;
	bool						HasParent;
	bool						ParentIsHidden;
	bool						ForceReference;
	std::string					ParentClass;
	std::string					Reserve;

	bool						PDSMapped;

	CClassNode					*MapClass;

	std::vector<CDeclarationNode*>	Init;
	std::string					InitProto;
	std::string					InitCallArgs;

	sint						Columns;

	uint						Id;

	std::vector<CDeclarationNode*>	Attributes;

	bool						HasRowAccess;
	bool						HasTableAccess;

	std::set<std::string>		Friends;
	std::set<std::string>		ForwardFriends;

	std::set<CClassNode*>		Legacy;

	///
	virtual bool	prolog();
	virtual bool	epilog();

	CDeclarationNode*	getDeclarationNode(const std::string &name);
	CDeclarationNode*	getKey();
	CDeclarationNode*	getClassKey();
	CDeclarationNode*	getDeclaration(const std::string& name);

	bool				useEntityId();

	void			checkClassReferences();
	void			fillAttributes();
	void			fillRefs();
	void			computeFriends();

	void	checkDependencies(std::set<CClassNode*> &beingChecked,
							  std::set<CClassNode*> &checkedClasses,
							  std::vector<CClassNode*> &classesOrder);

	void	buildInit();

	void	computeAttributesColumns();

	std::string					getId()	{ return (HasInheritance ? NLMISC::toString("__BaseTable") : NLMISC::toString(Id)); }

	std::string					getUserCode(const std::string& name);

	//

	CClassGenerator		Gen;

	CClassGenerator::SMethodId	InitId;
	CClassGenerator::SMethodId	DestroyId;
	CClassGenerator::SMethodId	FetchId;
	CClassGenerator::SMethodId	RegisterId;
	CClassGenerator::SMethodId	RegisterAttributesId;
	CClassGenerator::SMethodId	UnregisterId;
	CClassGenerator::SMethodId	UnregisterAttributesId;
	CClassGenerator::SMethodId	SetParentId;
	CClassGenerator::SMethodId	SetUnnotifiedParentId;
	CClassGenerator::SMethodId	NotifyInitId;
	CClassGenerator::SMethodId	NotifyReleaseId;

	CClassGenerator::SMethodId	UserInitId;
	CClassGenerator::SMethodId	UserReleaseId;

	//
	CClassGenerator::SMethodId	ClearId;
	CClassGenerator::SMethodId	StoreId;
	CClassGenerator::SMethodId	ApplyId;

	bool	indexUsedInInit;
	bool	indexUsedInDestroy;
	bool	indexUsedInFetch;
	bool	tableAndRowIndicesUsedInFetch;
	bool	indexUsedInRegister;
	bool	indexUsedInUnregister;
	bool	indexUsedInNotifyInit;
	bool	indexUsedInNotifyRelease;

	bool	generateContent();
	void	generateContentInCall(CCallContext *context);
};

class CDeclarationNode : public CParseNode
{
public:

	CDeclarationNode() : 
		InitFillFlag(false),
		WriteTriggerFlag(false),
		ParentFlag(false),
		HiddenFlag(false),
		MirroredFlag(false),
		ArrayFlag(false),
		SetFlag(false),
		IsRef(false),
		IsType(false),
		IsKey(false),
		Id(0),
		Column(-1),
		Columns(-1)
	{
	}

	bool			InitFillFlag;
	bool			WriteTriggerFlag;
	bool			ParentFlag;
	bool			HiddenFlag;
	bool			MirroredFlag;

	std::string		ParentClass;
	std::string		ParentField;

	std::string		Type;

	bool			ArrayFlag;
	std::string		ArrayIndex;
	bool			SetFlag;
	std::string		ForwardRefAttribute;

	bool			IsRef;
	bool			IsType;
	bool			IsKey;

	std::string		DefaultValue;

	struct CUserCode
	{
		std::string		Event;
		std::string		CodeSpecializer;
		std::string		UserCode;
	};

	std::vector<CUserCode>	UserCodes;

	std::string		getUserCode(const std::string &name, const std::string &specialize = std::string(""))
	{
		uint	i;
		// first look for a specialized code
		for (i=0; i<UserCodes.size(); ++i)
			if (UserCodes[i].Event == name && UserCodes[i].CodeSpecializer == specialize)
				return UserCodes[i].UserCode;
		// then look for a default code
		for (i=0; i<UserCodes.size(); ++i)
			if (UserCodes[i].Event == name && UserCodes[i].CodeSpecializer == "")
				return UserCodes[i].UserCode;
		return "";
	}

	TDeclarationType	DeclarationType;

	std::string		XmlNode;
	std::vector<CColumn>	ColumnList;

	uint			Id;

	sint			Column;
	sint			Columns;

	CClassNode*		ClassNode;
	CClassNode*		getParentClass()	{ return dynamic_cast<CClassNode*>(Parent); }

	///
	virtual bool	prolog();
	virtual bool	epilog();

	//
	std::string		getFunc() const		{ return lcFirst(getFunctionPrefix+Name); }
	std::string		setFunc() const		{ return lcFirst(setFunctionPrefix+Name); }
	std::string		newFunc() const		{ return lcFirst(newFunction+Name); }
	std::string		deleteFunc() const	{ return lcFirst(deleteFunction+Name); }
	std::string		unlinkFunc() const	{ return lcFirst(unlinkFunction+Name); }
	std::string		cppName() const		{ return "_"+Name; }
	std::string		tokenName() const	{ return "__Tok"+Name; }

	//
	std::string		displayPrintfPrefix();
	std::string		displayCppCode(std::string replVar = "");
	std::string		toUint64(std::string replVar = "");

	//
	void			generateContent(CCallContext *context = NULL);

	void			generateTypeContent(CCallContext *context = NULL);
	void			generateClassContent(CCallContext *context = NULL);
	void			generateBackRefContent();
	void			generateForwardRefContent();
	void			generateArrayTypeContent(CCallContext *context = NULL);
	void			generateArrayClassContent(CCallContext *context = NULL);
	void			generateArrayRefContent(CCallContext *context = NULL);
	void			generateSetContent(CCallContext *context = NULL);


	void			generateArrayApplyCode();
	void			generateArrayStoreCode();
	void			generateArrayEndCode();

	void			generateClassPtrApplyCode(const std::string& value);
	void			generateClassPtrStoreCode(const std::string& value);

	std::string		getAccessorName(CCallContext *context, const std::string& accessortype, const std::string& sep = "::");
};



//
//
//
class CIndexNode : public CTypeNode
{
public:

	CIndexNode()
	{
		CppType = "uint32";
		StorageType = "uint32";
	}

	virtual bool		isIndex()		{ return true; }

	virtual std::string	getSizeName()	{ return Name+"Size"; }
	virtual uint		getSize() = 0;

	virtual std::string	getIndexName(uint32 value) const
	{
		return NLMISC::toString(value);
	}

	virtual std::string	getToStringCode(const std::string& var) const
	{
		return "NLMISC::toString("+var+")";
	}

	virtual std::string	getFromStringCode(const std::string& var) const
	{
		return "atoi("+var+")";
	}

	virtual std::string	checkCode(const std::string& var)	{ return "nlassert("+var+"<"+getSizeName()+");"; }
};


//
//
//
class CEnumNode : public CIndexNode
{
public:
	CEnumNode() :
		CurrentValue(0),
		MinValue(0),
		MaxValue(0)
	{
	}

	uint32			CurrentValue;

	uint32			MinValue;
	uint32			MaxValue;
	std::vector<std::pair<std::string, uint32> >	Values;
	std::string		EndRange;

	virtual bool		isEnum()		{ return true; }
	virtual std::string	getName()
	{
		std::string	trunc = Name.substr(1);
		return "C"+trunc+"::"+(DisplayName.empty() ? Name : DisplayName);
	}

	virtual uint		getSize()		{ return MaxValue-MinValue; }

	bool	prolog();
	bool	epilog();

	///
	virtual bool	generateContent();

	std::string		getIndexName(uint32 value) const
	{
		std::string	result;

		uint	i;
		for (i=0; i<Values.size(); ++i)
			if (Values[i].second == value)
				result = Values[i].first;

		return result;
	}

	std::string		getUnscopedUseSize() const
	{
		return "___"+Name+"_useSize";
	}

	std::string		getUnknownValue() const
	{
		return "Unknown";
	}

	std::string		getUseSize() const
	{
		std::string	trunc = Name.substr(1);
		return "C"+trunc+"::"+getUnscopedUseSize();
	}

	std::string		getSizeName()
	{
		std::string	trunc = Name.substr(1);
		return "C"+trunc+"::"+getUnscopedUseSize();
	}

	virtual std::string	getToStringCode(const std::string& var) const
	{
		std::string	trunc = Name.substr(1);
		return "C"+trunc+"::toString("+var+")";
	}

	virtual std::string	getFromStringCode(const std::string& var) const
	{
		std::string	trunc = Name.substr(1);
		return "C"+trunc+"::fromString("+var+")";
	}

	virtual void	generateApplyCode(CClassGenerator::SMethodId& method, const std::string& token, const std::string& value)
	{
		method.add("{");
		method.add("std::string\tvaluename;");
		method.add("__pdr.pop("+token+", valuename);");
		method.add(value+" = "+getFromStringCode("valuename")+";");
		method.add("}");
	}

	virtual void	generateStoreCode(CClassGenerator::SMethodId& method, const std::string& token, const std::string& value)
	{
		method.add("{");
		method.add("std::string\tvaluename = "+getToStringCode(value)+";");
		method.add("__pdr.push("+token+", valuename);");
		method.add("}");
	}
};

class CEnumSimpleValueNode : public CEnumNode
{
public:

	std::vector<std::string>	Names;

	bool	prolog();
	bool	epilog();
};

class CEnumRangeNode : public CEnumNode
{
public:

	bool	prolog();
	bool	epilog();
};




//
//
//
class CDimensionNode : public CIndexNode
{
public:

public:
	CDimensionNode()
	{
	}

	virtual bool		isDimension()	{ return true; }
	std::string			getSizeName()	{ return Name+"Size"; }

	bool	prolog();
	bool	epilog();

	sint	Dimension;

	virtual uint		getSize()		{ return Dimension; }

	///
	virtual bool	generateContent();
};


//
//
//
class CLogMsgNode : public CParseNode
{
public:

	CLogMsgNode() : Context(false)
	{
	}

	bool	prolog();
	bool	epilog()
	{
		return true;
	}

	std::vector<std::pair<std::string, std::string> >	Params;
	std::vector<std::string>							Logs;

	bool												Context;

	uint			Id;

	void	generateContent();
};

class CExtLogTypeNode : public CParseNode
{
public:

	std::string	ExtLogType;

};




//
//
//
class CCallContext
{
public:

	std::vector<CDeclarationNode*>	Context;

	std::string				RootTable;
	std::string				RootRow;
	uint					Column;

	CCallContext(CDeclarationNode* decl = NULL)
	{
		if (decl != NULL)
			Context.push_back(decl);
	}

	bool	hasRootEntityIdKey()
	{
		CDeclarationNode*	k = getRootCaller()->getClassKey();
		if (k != NULL)
		{
			 return (getRootCaller()->getTypeNode(getRootCaller()->getKey()->Type)->CppType == "CEntityId");
		}
		return false;
	}

	/**
	 * Generates the name of the pointed variable
	 * For instance: PhysicalScoresBase, where PhysicalScores is an array and Base is an attribute in this array
	 * Actually concats all fields names
	 */
	std::string				getCallString()
	{
		uint	i;
		std::string	res;
		for (i=0; i<Context.size(); ++i)
			res += Context[i]->Name;
		return res;
	}

	CClassNode*				getRootCaller()
	{
		return (CClassNode*)Context[0]->Parent;
	}

	CDeclarationNode*		getRootDeclaration()
	{
		return Context[0];
	}

	/**
	 * Generates the access prototype argument list
	 * Actually only generates indexes for arrays
	 */
	std::string				getDebugCallStringFmt()
	{
		uint		i, idx=0;
		std::string	res;

		for (i=0; i<Context.size(); ++i)
		{
			CDeclarationNode*	decl = Context[i];

			if (decl->DeclarationType == ArrayClass || decl->DeclarationType == ArrayType)
			{
				if (!res.empty())
					res += ", ";

				CTypeNode*	tnd = decl->getTypeNode(decl->ArrayIndex);
				res += indexVariable+NLMISC::toString(idx++)+"="+tnd->getPrintfFmt();
			}
		}

		return res;
	}


	/**
	 * Generates the access prototype argument list
	 * Actually only generates indexes for arrays
	 */
	std::string				getDebugCallStringVal()
	{
		uint		i, idx=0;
		std::string	res;

		for (i=0; i<Context.size(); ++i)
		{
			CDeclarationNode*	decl = Context[i];

			if (decl->DeclarationType == ArrayClass || decl->DeclarationType == ArrayType)
			{
				if (!res.empty())
					res += ", ";

				CTypeNode*	tnd = decl->getTypeNode(decl->ArrayIndex);
				res += tnd->getPrintfVal(indexVariable+NLMISC::toString(idx++));
			}
		}

		return res;
	}


	/**
	 * Generates the access prototype argument list
	 * Actually only generates indexes for arrays
	 */
	std::string				getCallArgList()
	{
		std::string	res;
		uint		i, idx=0;

		for (i=0; i<Context.size(); ++i)
		{
			CDeclarationNode*	decl = Context[i];

			if (decl->DeclarationType == ArrayClass || decl->DeclarationType == ArrayType)
			{
				if (!res.empty())
					res += ", ";

				CTypeNode*	tnd = decl->getTypeNode(decl->ArrayIndex);
				res += tnd->getName()+ " "+indexVariable+NLMISC::toString(idx++);
			}
		}
		return res;
	}

	/**
	 * Generates the c++ path to the variable
	 * For instance: _PhysicalScores[__i0].Base, where PhysicalScores is an array and Base is an attribute in this array
	 */
	std::string				getCallPath()
	{
		std::string	res;
		uint		i, idx=0;

		for (i=0; i<Context.size(); ++i)
		{
			CDeclarationNode*	decl = Context[i];

			if (!res.empty())
				res += ".";

			res += decl->cppName();

			if (decl->DeclarationType == ArrayClass || decl->DeclarationType == ArrayType)
			{
				res += "["+indexVariable+NLMISC::toString(idx++)+"]";
			}
		}
		return res;
	}

	/**
	 * Generates the c++ path to the variable
	 * For instance: _PhysicalScores[__i0].Base, where PhysicalScores is an array and Base is an attribute in this array
	 */
	std::string				getUserCodeContext()
	{
		std::string	res;
		uint		i;

		if (Context.empty())
			return res;

		res = Context[0]->getParentClass()->Name;

		for (i=0; i<Context.size(); ++i)
			res += "."+Context[i]->Name;

		return res;
	}

	/**
	 * Generates check code for all types accessors
	 */
	std::vector<std::string>	getCheckCode()
	{
		std::vector<std::string>	res;
		uint		i, idx=0;

		for (i=0; i<Context.size(); ++i)
		{
			CDeclarationNode*	decl = Context[i];

			if (decl->DeclarationType == ArrayType || decl->DeclarationType == ArrayRef || decl->DeclarationType == ArrayClass)
			{
				CIndexNode*	ind = decl->getIndexNode(decl->ArrayIndex);
				std::string	code = ind->checkCode(indexVariable+NLMISC::toString(idx++));
				if (!code.empty())
					res.push_back(code);
			}
		}

		return res;
	}

	/**
	 * Get current context index
	 */
	uint					getContextIndex()
	{
		uint		i, idx=0;

		for (i=0; i<Context.size(); ++i)
		{
			CDeclarationNode*	decl = Context[i];
			if (decl->DeclarationType == ArrayClass || decl->DeclarationType == ArrayType)
				idx++;
		}
		return idx-1;
	}

	CCallContext			getSubContext(CDeclarationNode* decl)
	{
		CCallContext	ctx = *this;
		ctx.Context.push_back(decl);
		return ctx;
	}

	std::string				getColumn()
	{
		std::string	res;
		uint		i, idx=0;

		for (i=0; i<Context.size(); ++i)
		{
			CDeclarationNode*	decl = Context[i];
			if (decl->Column != 0)
			{
				if (!res.empty())
					res += "+";
				res += NLMISC::toString(decl->Column);
			}

			if (decl->DeclarationType == ArrayType || decl->DeclarationType == ArrayRef)
			{
				if (!res.empty())
					res += "+";
				res += indexVariable+NLMISC::toString(idx++);
			}
			else if (decl->DeclarationType == ArrayClass)
			{
				if (!res.empty())
					res += "+";
				CClassNode*	sub = decl->getClassNode(decl->Type);
				res += indexVariable+NLMISC::toString(idx++)+"*"+NLMISC::toString(sub->Columns);
			}
		}

		if (res.empty())
			res = "0";

		return res;
	}
};






//
// INLINES
//

inline CFileNode*	CParseNode::getFileNode()
{
	if (FileNode != NULL)
		return FileNode;

	CParseNode*	node = this;
	CFileNode*	fnode = NULL;

	while (node != NULL && (fnode = dynamic_cast<CFileNode*>(node)) == NULL)
		node = node->Parent;

	if (fnode == NULL)
		error("Can't find file node", "internal");

	FileNode = fnode;

	return fnode;
}

inline CDbNode*	CParseNode::getDbNode()
{
	if (DbNode != NULL)
		return DbNode;

	CParseNode*	node = this;
	CDbNode*	fnode = NULL;

	while (node != NULL && (fnode = dynamic_cast<CDbNode*>(node)) == NULL)
		node = node->Parent;

	if (fnode == NULL)
		error("Can't find db node", "internal");

	DbNode = fnode;

	return fnode;
}

inline CCppOutput&	CParseNode::hOutput()
{
	CFileNode	*fnode = getFileNode();
	return fnode->Hpp;
}

inline CCppOutput&	CParseNode::cppOutput()
{
	CFileNode	*fnode = getFileNode();
	return fnode->Cpp;
}

inline CCppOutput&	CParseNode::inlineOutput()
{
	CFileNode	*fnode = getFileNode();
	return fnode->HppInline;
}


inline CTypeNode	*CParseNode::getTypeNode(const std::string &name, bool genError)
{
	CDbNode*	db = getDbNode();
	uint		i;
	for (i=0; i<db->TypeNodes.size(); ++i)
		if (db->TypeNodes[i]->Name == name)
			return db->TypeNodes[i];
	if (genError)
		error("Can't find type '"+name+"'");
	return NULL;
}

inline CEnumNode	*CParseNode::getEnumNode(const std::string &name, bool genError)
{
	CEnumNode*	node = dynamic_cast<CEnumNode*>(getTypeNode(name, genError));
	if (node == NULL && genError)
		error("Can't find enum '"+name+"'");
	return node;
}

inline CDimensionNode	*CParseNode::getDimensionNode(const std::string &name, bool genError)
{
	CDimensionNode*	node = dynamic_cast<CDimensionNode*>(getTypeNode(name, genError));
	if (node == NULL && genError)
		error("Can't find dimension '"+name+"'");
	return node;
}

inline CIndexNode	*CParseNode::getIndexNode(const std::string &name, bool genError)
{
	CIndexNode*	node = dynamic_cast<CIndexNode*>(getTypeNode(name, genError));
	if (node == NULL && genError)
		error("Can't find index type '"+name+"' (neither enum nor dimension)");
	return node;
}

inline CClassNode	*CParseNode::getClassNode(const std::string &name, bool genError)
{
	CDbNode*	db = getDbNode();
	uint		i;
	for (i=0; i<db->ClassNodes.size(); ++i)
		if (db->ClassNodes[i]->Name == name)
			return db->ClassNodes[i];
	if (genError)
		error("Can't find class '"+name+"'");
	return NULL;
}

inline bool			CDbNode::addTypeNode(const std::string &name, const std::string &displayName, const std::string &defaultValue)
{
	CTypeNode		*node = new CTypeNode();

	node->Name = name;
	node->DisplayName = displayName;
	node->CppType = name;
	node->StorageType = name;

	node->ToCppType = NULL;
	node->ToStorageType = NULL;

	node->DefaultValue = defaultValue;

	node->ExternFlag = false;
	node->InternFlag = true;
	node->Parent = this;

	Nodes.insert(Nodes.begin(), node);

	return true;
}


inline CDeclarationNode*	CClassNode::getDeclarationNode(const std::string &name)
{
	CDeclarationNode	*node;
	if ((node = dynamic_cast<CDeclarationNode*>(getNode(name))) == NULL)
		error("declaration '"+name+"' not found");
	return node;
}

inline bool	CClassNode::useEntityId()
{
	 //return !ClassKey.empty() && getTypeNode(getKey()->Type)->CppType == "CEntityId";
	CDeclarationNode*	k = getClassKey();
	 return k!=NULL && getTypeNode(k->Type)->CppType == "CEntityId";
}

inline CDeclarationNode*	CClassNode::getClassKey()
{
	if (ClassKey.empty())
	{
		if (Inherited.empty())
			return NULL;
		CClassNode*	p = getClassNode(Inherited);
		return p->getClassKey();
	}
	else
	{
		return getKey();
	}
}


inline CDeclarationNode*	CClassNode::getKey()
{
	CDeclarationNode*	key = NULL;
	CClassNode*			classNode = this;
	while (classNode != NULL)
	{
		key = dynamic_cast<CDeclarationNode*>(classNode->getNode(classNode->ClassKey));
		if (key != NULL)
			return key;
		classNode = getClassNode(Inherited, false);
	}
	error("key declaration '"+ClassKey+"' not found");
	return NULL;
	//return getDeclarationNode(ClassKey);
}

inline CDeclarationNode*	CClassNode::getDeclaration(const std::string& name)
{
	CDeclarationNode*	decl = NULL;
	CClassNode*			classNode = this;
	while (classNode != NULL)
	{
		decl = dynamic_cast<CDeclarationNode*>(classNode->getNode(name));
		if (decl != NULL)
			return decl;
		classNode = getClassNode(classNode->Inherited, false);
	}
	error("declaration '"+name+"' not found");
	return NULL;
}



#endif
