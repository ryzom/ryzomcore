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


#include "parser.h"
#include "cpp_output.h"
#include "templatizer.h"

#include "parser_rules.h"

#include <nel/misc/path.h>
#include <nel/misc/sha1.h>

using namespace std;
using namespace NLMISC;
//using namespace RY_PDS;


bool					GenerateCpp = true;
bool					GenerateXmlDescription = true;
bool					VerboseMode = false;
bool					GenerateDebugMessages = false;
bool					GenerateHAuto = false;
bool					GenerateOnlyLogs = false;


CTokenizer				Tokenizer;
CTemplatizer			Templatizer;
CHashKey				HashKey;

string					CurrentEnum;
CEnumNode				*CurrentEnumNode = NULL;

string					DbDescriptionVersion("0.0");

string	formatDescription(const string &str)
{
	string	result;

	uint	pos = 0;

	while (pos < str.size() && (str[pos] == ' ' || str[pos] == '\t' || str[pos] == '\r' || str[pos] == '\n'))
		++pos;

	bool	first = true;

	while (pos < str.size())
	{
		if (!first)
			result += "\n";
		first = false;

		result += "* ";

		while (pos < str.size() && str[pos] != '\n')
			result += str[pos++];

		while (pos < str.size() && (str[pos] == ' ' || str[pos] == '\t' || str[pos] == '\r' || str[pos] == '\n'))
			++pos;
	}

	return result;
}

string	strReplace(string str, const string &search, const string &replace)
{
	std::string::size_type	pos = 0;
	while ((pos = str.find(search)) != string::npos)
		str.replace(pos, search.size(), replace);
	return str;
}

string	xmlSpecialChars(string str)
{
	str = strReplace(str, "&", "&amp;");
	str = strReplace(str, "<", "&lt;");
	str = strReplace(str, ">", "&gt;");
	str = strReplace(str, "\"", "&quot;");
	str = strReplace(str, "'", "&apos;");

	return str;
}

string	getFullStdPathNoExt(const string &path)
{
	string	dir = NLMISC::toLower(NLMISC::CFile::getPath(path));
	string	file = NLMISC::toLower(NLMISC::CFile::getFilenameWithoutExtension(path));

	return dir.empty() ? file : NLMISC::CPath::standardizePath(dir)+file;
}

string	getFullStdPath(const string &path)
{
	string	dir = NLMISC::toLower(NLMISC::CFile::getPath(path));
	string	file = NLMISC::toLower(NLMISC::CFile::getFilename(path));

	return dir.empty() ? file : NLMISC::CPath::standardizePath(dir)+file;
}

string	appendArg(const std::string& firstArgs, const std::string& nextArg)
{
	if (firstArgs.empty())
		return nextArg;
	return firstArgs + ", " + nextArg;
}

/*
 * Start parsing
 */
CParseNode	*parse(const string &file)
{
	HashKey = getSHA1(file);

	Tokenizer.readFile(file);

	if (Tokenizer.tokenize())
		return parseMain(Tokenizer);
	return NULL;
}



/*
 * Execute nodes
 */

// DB Node
bool	CDbNode::prolog()
{
	addTypeNode("CSheetId", "NLMISC::CSheetId", "NLMISC::CSheetId::Unknown");
	addTypeNode("CEntityId", "NLMISC::CEntityId", "NLMISC::CEntityId::Unknown");
	addTypeNode("double", "double", "0.0");
	addTypeNode("float", "float", "0.0f");
	addTypeNode("sint64", "sint64", "(sint64)0");
	addTypeNode("uint64", "uint64", "(uint64)0");
	addTypeNode("sint32", "sint32", "0");
	addTypeNode("uint32", "uint32", "0");
	addTypeNode("sint16", "sint16", "0");
	addTypeNode("uint16", "uint16", "0");
	addTypeNode("sint8", "sint8", "0");
	addTypeNode("uint8", "uint8", "0");
	addTypeNode("ucchar", "ucchar", "0");
	addTypeNode("char", "char", "0");
	addTypeNode("bool", "bool", "false");

	return true;
}



bool				inlineAccessors					= false;
string				getFunctionPrefix				= "get";
string				setFunctionPrefix				= "set";
string				newFunction						= "addTo";
string				deleteFunction					= "deleteFrom";

string				indexVariable					= "__i";
string				valueVariable					= "__v";
string				keyVariable						= "__k";
string				objectVariable					= "__o";

bool				inlineUserInitDefaultCode		= false;
string				userInitFunction				= "init";
string				userReleaseFunction				= "release";

bool				inlineStaticPublic				= false;
string				staticCreateFunction			= "create";
string				staticRemoveFunction			= "remove";
string				staticSetUserFactoryFunction	= "setFactory";
string				staticLoadFunction				= "load";
string				staticUnloadFunction			= "unload";
string				staticSetLoadCbFunction			= "setLoadCallback";
string				staticGetFunction				= "get";
string				staticCastFunction				= "cast";
string				staticConstCastFunction			= "cast";
string				staticBeginFunction				= "begin";
string				staticEndFunction				= "end";

bool				inlineInternal					= false;
string				initFunction					= "pds__init";
string				destroyFunction					= "pds__destroy";
string				registerFunction				= "pds__register";
string				registerAttributesFunction		= "pds__registerAttributes";
string				unregisterFunction				= "pds__unregister";
string				unregisterAttributesFunction	= "pds__unregisterAttributes";
string				fetchFunction					= "pds__fetch";
string				setParentFunction				= "pds__setParent";
string				setUnnotifiedParentFunction		= "pds__setParentUnnotified";
string				getTableFunction				= "pds__getTable";
string				unlinkFunction					= "pds__unlink";
string				notifyInitFunction				= "pds__notifyInit";
string				notifyReleaseFunction			= "pds__notifyRelease";

string				clearFunction					= "clear";
string				storeFunction					= "store";
string				applyFunction					= "apply";
string				declareTokensFunction			= "pds__declareTokens";


bool				inlineStaticInternal			= false;
string				staticInitFactoryFunction		= "pds_static__setFactory";
string				staticFactoryFunction			= "pds_static__factory";
string				staticFetchFunction				= "pds_static__fetch";
string				staticInitFunction				= "pds_static__init";
string				staticNotifyLoadFailure			= "pds_static__notifyFailure";
string				staticLoadCbAttribute			= "__pds__LoadCallback";

bool				inlineLog						= false;
string				logStartFunction				= "pds__startLog";
string				logStopFunction					= "pds__stopLog";

string				PDSNamespace					= "RY_PDS";
string				CPDSLibName						= PDSNamespace+"::CPDSLib";
string				objectIndexName					= PDSNamespace+"::CObjectIndex";
string				nullIndexName					= PDSNamespace+"::CObjectIndex::null()";
string				TTableIndexName					= PDSNamespace+"::TTableIndex";
string				TRowIndexName					= PDSNamespace+"::TRowIndex";
string				TColumnIndexName				= PDSNamespace+"::TColumnIndex";
string				INVALID_TABLE_INDEXName			= PDSNamespace+"::INVALID_TABLE_INDEX";
string				INVALID_ROW_INDEXName			= PDSNamespace+"::INVALID_ROW_INDEX";
string				indexAllocatorName				= PDSNamespace+"::CIndexAllocator";
string				pdBaseDataName					= PDSNamespace+"::IPDBaseData";
//string				pdBaseInheritDataName			= PDSNamespace+"::IPDBaseInheritData";
string				CPDataName						= PDSNamespace+"::CPData";
string				TPDFactoryName					= PDSNamespace+"::TPDFactory";
string				TPDFetchName					= PDSNamespace+"::TPDFetch";
string				TPDFetchFailureName				= PDSNamespace+"::TPDFetchFailure";

bool				inlineLogFunctions				= false;

string				pdslibFunc(const std::string& func)
{
	//return CPDSLibName+"::"+func;
	return "PDSLib."+func;
}

bool	CDbNode::epilog()
{
	uint	i;

	Env = Templatizer.RootEnv;
	setEnv("db", Name);

	string	fullfile = getFullStdPathNoExt(MainFile.empty() ? Name : MainFile);
	string	filename = NLMISC::toLower(NLMISC::CFile::getFilenameWithoutExtension(fullfile));

	setEnv("filename", filename);
	setEnv("fullfilename", fullfile);
	setEnv("headerfilename", strReplace(strupr(filename+".h"), ".", "_"));

	DbHpp.setFileHeader(filename+".h", "Initialisation of the "+Name+" database, declarations\n"+Description);
	DbCpp.setFileHeader(filename+".cpp", "Initialisation of the "+Name+" database, implementation\n"+Description);
	DbHppInline.setFileHeader(filename+"_inline.h", "Initialisation of the "+Name+" database, inline implementation\n"+Description);

	DbSummary << "\n\n";
	DbSummary << "Summary of " << Name << " database classes of database\n";
	DbSummary << "-----------------------------------------------------------------------------\n\n";
	DbSummary << "This file is automatically generated.\n";
	DbSummary << "This is a reminder of all classes generated and methods implemented.\n\n\n";

	DbSummary << "Database " << Name << " is managed through 4 functions located in " << fullfile << ".h:\n\n";

	DbSummary << Name << "::init():\n";
	DbSummary << "Initialises database context and connection towards database server (refered as PDS).\n";
	DbSummary << "All user factories must have been set before call to this function.\n";
	DbSummary << "Call this function in service init method.\n\n";

	DbSummary << Name << "::ready():\n";
	DbSummary << "Tells whether the whole database engine is ready to work.\n";
	DbSummary << "You must not update any value nor call update() unless ready() is true.\n\n";

	DbSummary << Name << "::update():\n";
	DbSummary << "Updates the database engine and sends updates to the PDS.\n";
	DbSummary << "Call this function each tick, provided ready() returned true.\n\n";

	DbSummary << Name << "::release():\n";
	DbSummary << "Releases the database engine. Drops all data, closes the connection to the PDS.\n";
	DbSummary << "Call this function in service release method.\n\n";

	DbSummary << "\n\n";
	DbSummary << "Summary of generated classes for " << Name << "\n\n";

	DbHpp << "\n#ifndef " << strReplace(strupr(filename+".h"), ".", "_") << "\n";
	DbHpp << "#define " << strReplace(strupr(filename+".h"), ".", "_") << "\n";
	DbHpp << "\n";
	DbHpp << "#include <nel/misc/types_nl.h>\n";
	DbHpp << "#include <pd_lib/pd_lib.h>\n";
	DbHpp << "\n";
	DbHpp << "namespace " << Name << "\n{\n\n";
	DbHpp.unindent();

	if (!Pch.empty())
	{
		DbCpp << "\n#include \"" << Pch << "\"\n\n";
	}

	DbCpp << "#include \"" << filename << ".h\"\n\n";
	DbCpp << "namespace " << Name << "\n{\n\n";
	DbCpp.unindent();

	DbCpp << "RY_PDS::CPDSLib	PDSLib;\n";

	DbHppInline << "namespace " << Name << "\n{\n\n";
	DbHppInline.unindent();


	uint	maxClassId = 0;
	for (i=0; i<ClassNodes.size(); ++i)
	{
		CClassNode	*classnd = ClassNodes[i];
		++maxClassId;
	}

	uint	maxTypeId = 0;
	for (i=0; i<TypeNodes.size(); ++i)
	{
		CTypeNode	*typend = TypeNodes[i];
		++maxTypeId;
	}

	xmlDescription.clear();
	xmlDescription.push_back("<?xml version='1.0'?>");
	xmlDescription.push_back("<dbdescription version='"+DbDescriptionVersion+"'>");
	//xmlDescription.push_back("<db name='"+Name+"' types='"+toString(maxTypeId)+"' classes='"+toString(maxClassId)+"' hashkey='"+HashKey.toString()+"'>");
	xmlDescription.push_back("<db name='"+Name+"' types='"+toString(maxTypeId)+"' classes='"+toString(maxClassId)+"'>");

	pass1();

	pass2();

	pass3();

	// Check dependencies order
	vector<CClassNode*>	classesOrder;
	vector<CFileNode*>	filesOrder;
	buildClassOrder(classesOrder, filesOrder);

	// generate all file prologs
	for (i=0; i<FileNodes.size(); ++i)
	{
		CFileNode	*child = FileNodes[i];
		child->generateProlog();
	}

	pass4();

	//
	initDb.init("init");
	initDb.IsInline = false;
	initDb.Proto = "uint32 overrideDbId";
	initDb.Type = "void";
	initDb.Description = "Initialise the whole database engine.\nCall this function at service init.";

	readyDb.init("ready");
	readyDb.IsInline = false;
	readyDb.Proto = "";
	readyDb.Type = "bool";
	readyDb.Description = "Tells if database engine is ready to work.\nEngine may not be ready because PDS is down, not yet ready\nor message queue to PDS is full.";

	updateDb.init("update");
	updateDb.IsInline = false;
	updateDb.Proto = "";
	updateDb.Type = "void";
	updateDb.Description = "Update the database engine.\nCall this method once per tick, only if engine is ready (see also ready() above).";

	logChatDb.init("logChat");
	logChatDb.IsInline = false;
	logChatDb.Proto = "const ucstring& sentence, const NLMISC::CEntityId& from, const std::vector<NLMISC::CEntityId>& to";
	logChatDb.Type = "void";
	logChatDb.Description = "Logs chat sentence with sender and receipiants.";

	logTellDb.init("logTell");
	logTellDb.IsInline = false;
	logTellDb.Proto = "const ucstring& sentence, const NLMISC::CEntityId& from, const NLMISC::CEntityId& to";
	logTellDb.Type = "void";
	logTellDb.Description = "Logs tell sentence with sender and single recipient (might be player or group).";

	releaseDb.init("release");
	releaseDb.IsInline = false;
	releaseDb.Proto = "";
	releaseDb.Type = "void";
	releaseDb.Description = "Release the whole database engine.\nCall this function at service release.";

	//
	generateClassesDeclaration();

	//
	generateClassesContent(classesOrder);

	//
	generateLogContent();

	xmlDescription.push_back("</db>");
	xmlDescription.push_back("</dbdescription>");;

	initDb.add("std::string\txmlDescription;");

	DbXml.setXmlMode();

	for (i=0; i<xmlDescription.size(); ++i)
	{
		initDb.add("xmlDescription += \""+xmlDescription[i]+"\\n\";");
		DbXml << xmlDescription[i] << "\n";
	}

	DbXml.flush(fullfile+".xml");

	DbHpp << "/// \\name Public API for " << Name << " database\n";
	DbHpp << "// @{\n";
	DbHpp.unindent();

	initDb.add(pdslibFunc("init")+"(xmlDescription, overrideDbId);");
	initDb.flush(DbHpp, DbCpp, DbHppInline);

	readyDb.add("return "+pdslibFunc("PDSReady")+"();");
	readyDb.flush(DbHpp, DbCpp, DbHppInline);

	updateDb.add(pdslibFunc("update")+"();");
	updateDb.flush(DbHpp, DbCpp, DbHppInline);

	logChatDb.add(pdslibFunc("logChat")+"(sentence, from, to);");
	logChatDb.flush(DbHpp, DbCpp, DbHppInline);

	logTellDb.add("std::vector<NLMISC::CEntityId>\tids;");
	logTellDb.add("ids.push_back(to);");
	logTellDb.add(pdslibFunc("logChat")+"(sentence, from, ids);");
	logTellDb.flush(DbHpp, DbCpp, DbHppInline);

	releaseDb.add(pdslibFunc("release")+"();");
	releaseDb.flush(DbHpp, DbCpp, DbHppInline);

	DbHpp << "\n// @}\n\n";

	DbHpp << "extern RY_PDS::CPDSLib	PDSLib;\n";

	DbHpp.indent();
	DbHpp << "\n} // End of " << Name <<"\n";

	generateIncludes(filesOrder);

	DbHpp << "\n#include \"" << filename << "_inline.h\"\n\n";
	DbHpp << "\n#endif\n";
	DbHpp.flush(fullfile+".h");

	DbCpp.indent();
	DbCpp << "\n} // End of " << Name <<"\n";
	DbCpp.flush(fullfile+".cpp");

	DbHppInline.indent();
	DbHppInline << "\n} // End of " << Name <<"\n";
	DbHppInline.flush(fullfile+"_inline.h");

	DbSummary.flush(fullfile+"_summary.txt");

	for (i=0; i<FileNodes.size(); ++i)
	{
		CFileNode	*child = FileNodes[i];
		child->generateEpilog();
	}

	return true;
}



void	CDbNode::pass1()
{
	/*
	 * PASS 1
	 * - all type names and class names are known
	 * -> look for classes in set or backreferences
	 */

	uint	i;

	uint	classId = 0;
	uint	typeId = 0;

	for (i=0; i<ClassNodes.size(); ++i)
	{
		CClassNode	*classnd = ClassNodes[i];
		classnd->checkClassReferences();
		classnd->Id = classId++;

		classnd->getFileNode()->IncludeStandard = true;
		classnd->getFileNode()->IncludeDbFile = true;
	}

	for (i=0; i<TypeNodes.size(); ++i)
	{
		CTypeNode	*typend = TypeNodes[i];

		typend->Id = typeId++;

		uint	tsize = 0;
		if (typend->StorageType == "bool")			tsize = 1;
		if (typend->StorageType == "char")			tsize = 1;
		if (typend->StorageType == "ucchar")		tsize = 2;
		if (typend->StorageType == "uint8")			tsize = 1;
		if (typend->StorageType == "sint8")			tsize = 1;
		if (typend->StorageType == "uint16")		tsize = 2;
		if (typend->StorageType == "sint16")		tsize = 2;
		if (typend->StorageType == "uint32")		tsize = 4;
		if (typend->StorageType == "sint32")		tsize = 4;
		if (typend->StorageType == "uint64")		tsize = 8;
		if (typend->StorageType == "sint64")		tsize = 8;
		if (typend->StorageType == "float")			tsize = 4;
		if (typend->StorageType == "double")		tsize = 8;
		if (typend->StorageType == "CEntityId")		tsize = 8;
		if (typend->StorageType == "CSheetId")		tsize = 4;

		typend->Size = tsize;

		string	xmlnode = "<typedef name='"+typend->Name+"' id='"+toString(typend->Id)+"' size='"+toString(tsize)+"' storage='"+typend->StorageType+"'";

		if (typend->isEnum())
		{
			CEnumNode	*enumnd = static_cast<CEnumNode*>(typend);

			xmlnode += " type='enum'>";
			xmlDescription.push_back(xmlnode);
			uint	j;
			for (j=0; j<enumnd->Values.size(); ++j)
				xmlDescription.push_back("<enumvalue name='"+enumnd->Values[j].first+"' value='"+toString(enumnd->Values[j].second)+"'/>");
			xmlDescription.push_back("</typedef>");
		}
		else if (typend->isDimension())
		{
			CDimensionNode	*dimnd = static_cast<CDimensionNode*>(typend);

			xmlnode += " type='dimension' dimension='"+toString(dimnd->Dimension)+"'/>";
			xmlDescription.push_back(xmlnode);
		}
		else
		{
			xmlnode += " type='type'/>";
			xmlDescription.push_back(xmlnode);
		}
	}
}

void	CDbNode::pass2()
{
	/*
	 * PASS 2
	 * - class hierarchy, backreferences and in set information are known
	 * -> fill up attributes
	 */

	uint	i;

	for (i=0; i<ClassNodes.size(); ++i)
	{
		CClassNode	*child = ClassNodes[i];

		if (!child->Inherited.empty() && child->MappedFlag && !child->HasParent)
			child->error("class cannot inherit another class and be mapped. Try to map base class instead.");

		if (child->MappedFlag && child->getClassKey() == NULL)
			child->error("class is mapped and has no key defined");

		child->ForceReference = (child->HasInheritance || child->MappedFlag || child->DerivatedFlag || (child->HasParent && !child->ParentIsHidden));

		if (child->ForceReference && child->ParentIsHidden)
			child->error("Parent attribute cannot be hidden because class has inheritance, is mapped or is derivated.");

		child->getFileNode()->IncludePDSLib = true;

		if (!child->Inherited.empty())
		{
			CClassNode	*icln = child;
			CClassNode	*lastMapped = (child->MappedFlag ? child : NULL);
			while (!icln->Inherited.empty())
			{
				icln = getClassNode(icln->Inherited);
				if (icln->MappedFlag)
				{
					if (lastMapped != NULL)
						lastMapped->error("class cannot be remapped since parent "+icln->Name+" is already mapped");
					lastMapped = icln;
				}
			}

			/*
			if (icln->MappedFlag)
				child->MapClass = icln;
			*/
			child->MapClass = lastMapped;
		}
		else if (child->MappedFlag)
		{
			child->MapClass = child;
		}

		child->fillAttributes();
	}
}


void	CDbNode::pass3()
{
	/*
	 * PASS 3
	 * - attributes are known
	 * -> fill up backrefs and array/set forwardrefs
	 */

	uint	i;

	for (i=0; i<ClassNodes.size(); ++i)
	{
		CClassNode	*child = ClassNodes[i];
		child->fillRefs();
	}

	for (i=0; i<ClassNodes.size(); ++i)
	{
		CClassNode	*child = ClassNodes[i];
		child->computeFriends();
	}
}

void	CDbNode::pass4()
{
	/*
	 * PASS 4
	 * - everything is ok in database descriptor
	 * -> output c++ code
	 */

	uint	i;

	for (i=0; i<FileNodes.size(); ++i)
	{
		CFileNode*	file = FileNodes[i];
		file->Hpp << "\n\n//\n// Typedefs & Enums\n//\n\n";
	}

	for (i=0; i<TypeNodes.size(); ++i)
	{
		CTypeNode	*type = TypeNodes[i];
		if (type->ExternFlag || type->InternFlag)
			continue;

		type->generateContent();

	}
}



void	CDbNode::generateClassesDeclaration()
{
	uint	i;

	DbHpp << "\n//\n";
	DbHpp << "// Global Forward Declarations\n";
	DbHpp << "//\n\n";

	for (i=0; i<ClassNodes.size(); ++i)
	{
		DbHpp << "class " << ClassNodes[i]->Name << ";\n";
	}

	DbHpp << "\n";
	DbHpp << "//\n\n";
}

void	CDbNode::generateIncludes(vector<CFileNode*>& filesOrder)
{
	uint	i;

	DbHpp << "\n//\n";
	DbHpp << "// Includes\n";
	DbHpp << "//\n\n";

	for (i=0; i<filesOrder.size(); ++i)
	{
		if (!filesOrder[i]->SeparatedFlag)
		{
			filesOrder[i]->setEnv("as", getFileNoExtPath(filesOrder[i]->IncludeAs));
			DbHpp << "#include \"" << getFileNoExtPath(filesOrder[i]->IncludeAs) << ".h\"\n";
		}
	}

	DbHpp << "\n";

	for (i=0; i<filesOrder.size(); ++i)
	{
		if (filesOrder[i]->IncludeDbFile && !filesOrder[i]->SeparatedFlag)
		{
			filesOrder[i]->define("incinline");
			DbHpp << "#include \"" << getFileNoExtPath(filesOrder[i]->IncludeAs) << "_inline.h\"\n";
		}
	}

	DbHpp << "\n";
	DbHpp << "//\n\n";
}

void	CDbNode::generateClassesContent(vector<CClassNode*>& classesOrder)
{
	uint	i;

	//
	// output classes content
	//
	for (i=0; i<classesOrder.size(); ++i)
	{
		CClassNode	*cln = classesOrder[i];

		initDb.add(pdslibFunc("registerClassMapping")+"("+toString(cln->Id)+", \""+cln->Name+"\");");
		cln->generateContent();
	}
}

void	CDbNode::buildClassOrder(vector<CClassNode*>& classesOrder, vector<CFileNode*>& filesOrder)
{
	set<CClassNode*>		checkedClasses;

	uint	i;

	for (i=0; i<ClassNodes.size(); ++i)
	{
		CClassNode	*child = ClassNodes[i];

		if (checkedClasses.find(child) != checkedClasses.end())
			continue;

		set<CClassNode*>	beingChecked;
		child->checkDependencies(beingChecked, checkedClasses, classesOrder);
	}

	set<CFileNode*>	checkedFiles;
	filesOrder.clear();

	for (i=0; i<FileNodes.size(); ++i)
	{
		CFileNode		*child = FileNodes[i];

		if (checkedFiles.find(child) != checkedFiles.end())
			continue;

		set<CFileNode*>	beingChecked;
		child->checkDependencies(beingChecked, checkedFiles, filesOrder);
	}

	for (i=0; i<filesOrder.size(); ++i)
		filesOrder[i]->Env = Env->nextArrayNode("files");

	for (i=0; i<classesOrder.size(); ++i)
		classesOrder[i]->Env = classesOrder[i]->getFileNode()->Env->nextArrayNode("classes");
}


void	CDbNode::generateLogContent()
{
	uint	logid = 0;

	uint	i;

	for (i=0; i<LogNodes.size(); ++i)
	{
		CLogMsgNode	*child = LogNodes[i];
		child->Id = logid;
		logid += (uint)child->Logs.size();

		child->generateContent();
	}
}

// get file path from this file
string	CDbNode::getFileNoExtPath(const std::string& file)
{
	string	thisPath = NLMISC::CFile::getPath(NLMISC::toLower(getDbFile()));
	string	filePath = NLMISC::CFile::getPath(NLMISC::toLower(file));
	string	fileName = NLMISC::CFile::getFilename(NLMISC::toLower(file));

	if (thisPath == filePath)
		return CFile::getFilenameWithoutExtension(fileName);
	else
		return CPath::standardizePath(filePath)+CFile::getFilenameWithoutExtension(NLMISC::toLower(file));
}




// File Node
bool	CFileNode::prolog()
{
	CDbNode*	db = getDbNode();
	db->FileNodes.push_back(this);

	if (GenerateOnlyLogs)
		Generate = false;

	return true;
}

bool	CFileNode::epilog()
{
	return true;
}

bool	CFileNode::generateProlog()
{
	CDbNode*	db = getDbNode();

	if (!db->Description.empty())
		Hpp << db->Description << "\n";

	string	filename = NLMISC::toLower(CFile::getFilenameWithoutExtension(Name));

	setEnv("fullfilename", getFullStdPathNoExt(Name));
	setEnv("filename", filename);
	setEnv("headerfilename", strReplace(strupr(filename+".h"), ".", "_"));
	setEnv("description", Description);

	Hpp.setFileHeader(filename+".h", Description);
	Cpp.setFileHeader(filename+".cpp", Description);
	HppInline.setFileHeader(filename+"_inline.h", Description);

	Hpp << "\n#ifndef " << strReplace(strupr(filename+".h"), ".", "_") << "\n";
	Hpp << "#define " << strReplace(strupr(filename+".h"), ".", "_") << "\n\n";


	Hpp << "#include <nel/misc/types_nl.h>\n";
	Hpp << "#include <nel/misc/debug.h>\n";
	Hpp << "#include <nel/misc/common.h>\n";

	if (GenerateHAuto)
	{
		Hpp << "#include <nel/misc/hierarchical_timer.h>\n";
	}

	if (IncludeStandard)
	{
		define("incstd");
		Hpp << "#include <nel/misc/entity_id.h>\n";
		Hpp << "#include <nel/misc/sheet_id.h>\n";
	}

	Hpp << "#include <vector>\n";
	Hpp << "#include <map>\n";

	if (IncludePDSLib)
	{
		define("incpdslib");
		Hpp << "#include <pd_lib/pd_lib.h>\n";
		Hpp << "#include <game_share/persistent_data.h>\n";
	}
	Hpp << "\n";

	if (SeparatedFlag)
	{
		string	fullfile = getFullStdPathNoExt(db->MainFile.empty() ? db->Name : db->MainFile);
		string	filename = NLMISC::toLower(NLMISC::CFile::getFilenameWithoutExtension(fullfile));
		Hpp << "#include \"" << filename << ".h\"\n";
		Hpp << "\n";
	}

	Hpp << "// User #includes\n";
	uint	i;
	for (i=0; i<IncludeNodes.size(); ++i)
	{
		Env->nextArrayNode("incuser")->set("as", IncludeNodes[i]->Name);
		Hpp << "#include \"" << IncludeNodes[i]->Name << "\"\n";
	}

	Hpp << "\nnamespace " << db->Name << "\n{\n\n";
	Hpp.unindent();

	if (!db->Description.empty())
		HppInline << db->Description;
	HppInline << "namespace " << db->Name << "\n{\n\n";
	HppInline.unindent();
	Hpp << "//\n// Forward declarations\n//\n\n";

	if (!db->Pch.empty())
	{
		Cpp << "\n";
		Cpp << "#include \""+db->Pch+"\"";
	}

	Cpp << "\n";
	if (SeparatedFlag || !IncludeDbFile)
	{
		Cpp << "#include \"" << filename << ".h\"\n";
	}
	else
	{
		Cpp << "#include \"" << getFileNoExtPath(getDbNode()->getDbFile()) << ".h\"\n";
	}

	Cpp << "\n";
	Cpp << "namespace " << db->Name << "\n{\n\n";
	Cpp.unindent();

	return true;
}

bool	CFileNode::generateEpilog()
{
	CDbNode*	db = getDbNode();

	string	fullfile = getFullStdPathNoExt(Name);
	string	filename = NLMISC::toLower(CFile::getFilenameWithoutExtension(Name));

	Hpp.indent();
	Hpp << "\n} // End of " << db->Name <<"\n";

	if (!IncludeDbFile || SeparatedFlag)
	{
		// add inline #include
		Hpp << "\n\n//\n// Inline implementations\n//\n\n";
		Hpp << "#include \"" << filename << "_inline.h\"\n";
	}

	Hpp << "\n#endif\n";

	HppInline.indent();
	HppInline << "\n} // End of " << db->Name <<"\n";

	Cpp.indent();
	Cpp << "\n} // End of " << db->Name <<"\n";

	if (Generate)
		writeFile();

	return true;
}

string	CFileNode::getFileNoExtPath(const string& file)
{
	string	thisPath = NLMISC::CFile::getPath(NLMISC::toLower(Name));
	string	filePath = NLMISC::CFile::getPath(NLMISC::toLower(file));
	string	fileName = NLMISC::CFile::getFilename(NLMISC::toLower(file));

	if (thisPath == filePath)
		return CFile::getFilenameWithoutExtension(fileName);
	else
		return CFile::getFilenameWithoutExtension(NLMISC::toLower(file));
}

void	CFileNode::writeFile()
{
	string	fullfile = getFullStdPathNoExt(Name);

	Hpp.flush(fullfile+".h");
	Cpp.flush(fullfile+".cpp");
	HppInline.flush(fullfile+"_inline.h");
}

void	CFileNode::checkDependencies(set<CFileNode*> &beingChecked,
									 set<CFileNode*> &checkedFiles,
									 vector<CFileNode*> &filesOrder)
{
	if (beingChecked.find(this) != beingChecked.end())
		error("circular dependency in file '"+Name+"'");

	if (checkedFiles.find(this) != checkedFiles.end())
		return;

	beingChecked.insert(this);
	checkedFiles.insert(this);

	set<CFileNode*>::iterator	it;
	for (it=Dependencies.begin(); it!=Dependencies.end(); ++it)
	{
		CFileNode	*fileNode = *it;
		if (fileNode == this)
			continue;

		fileNode->checkDependencies(beingChecked, checkedFiles, filesOrder);
	}

	filesOrder.push_back(this);
}







// Type Node
bool	CTypeNode::prolog()
{
	CDbNode*	db = getDbNode();
	db->TypeNodes.push_back(this);

	return true;
}

bool	CTypeNode::generateContent()
{
	hOutput() << "/** " << Name << "\n";
	if (!Description.empty())
	{
		hOutput() << Description << "\n";
	}
	uint	line, col;
	string	file;
	getFileLine(line, col, file);
	hOutput() << "defined at " << file << ":" << line << "\n";
	hOutput() << "*/\n";

	hOutput() << "typedef " << getCppType() << " " << Name << ";\n\n";


	if (ToCppType != NULL)
	{
		CCppCodeNode	*tocpp = static_cast<CCppCodeNode*>(ToCppType);

		CFunctionGenerator	toCppFunc;

		toCppFunc.init(storageToCpp());
		toCppFunc.setType(getName());
		toCppFunc.IsInline = true;
		toCppFunc.Proto = StorageType+" _v";

		toCppFunc.add(getName()+"\t__res;");
		toCppFunc.add(strReplace(strReplace(tocpp->RawCode, "$("+CppType+")", "__res"), "$("+StorageType+")", "_v"));
		toCppFunc.add("return __res;");
		toCppFunc.flush(hOutput(), cppOutput(), inlineOutput());
	}
	if (ToStorageType != NULL)
	{
		CCppCodeNode	*tostorage = static_cast<CCppCodeNode*>(ToStorageType);

		CFunctionGenerator	toStorageFunc;

		toStorageFunc.init(cppToStorage());
		toStorageFunc.setType(StorageType);
		toStorageFunc.IsInline = true;
		toStorageFunc.Proto = getName()+" _v";

		toStorageFunc.add(StorageType+"\t__res;");
		toStorageFunc.add(strReplace(strReplace(tostorage->RawCode, "$("+StorageType+")", "__res"), "$("+CppType+")", "_v"));
		toStorageFunc.add("return __res;");
		toStorageFunc.flush(hOutput(), cppOutput(), inlineOutput());
	}

	hOutput() << "\n";

	return true;
}



// Include Node
bool	CIncludeNode::prolog()
{
	CFileNode*	file = getFileNode();
	file->IncludeNodes.push_back(this);
	return true;
}


// Include Node
bool	CUsePchNode::prolog()
{
	CDbNode*	db = getDbNode();
	db->Pch = Name;
	return true;
}


// CppCode Node
bool	CCppCodeNode::prolog()
{
	return true;
}






// Dimension Nodes
bool	CDimensionNode::prolog()
{
	CDbNode*	db = getDbNode();
	db->TypeNodes.push_back(this);

	if (Dimension < 256)
		StorageType = "uint8";
	else if (Dimension < 65536)
		StorageType = "uint16";
	else
		StorageType = "uint32";

	CppType = "uint32";

	return true;
}

bool	CDimensionNode::epilog()
{
	return true;
}

bool	CDimensionNode::generateContent()
{
	hOutput() << "/** " << Name << "\n";
	if (!Description.empty())
	{
		hOutput() << Description << "\n";
	}
	uint	line, col;
	string	file;
	getFileLine(line, col, file);
	hOutput() << "defined at " << file << ":" << line << "\n";
	hOutput() << "*/\n";

	hOutput() << "typedef " << CppType << " " << Name << ";\n";
	hOutput() << "const " << getName() << "\t" << getSizeName() << " = " << Dimension << ";\n\n";

	return true;
}

// Enum Nodes
bool	CEnumNode::prolog()
{
	CDbNode*	db = getDbNode();
	db->TypeNodes.push_back(this);

	if (Name.empty() || Name[0] != 'T')
		error("enum name '"+Name+"' is invalid, must begin with a 'T'");

	CurrentValue = 0;
	CurrentEnum = Name;
	CurrentEnumNode = this;

	MinValue = 0;
	MaxValue = 0;

	return true;
}

bool	CEnumNode::epilog()
{
	uint	i;

	for (i=0; i<Nodes.size(); ++i)
	{
		CEnumNode	*nd = dynamic_cast<CEnumNode*>(Nodes[i]);
		if (!nd)
			continue;
		Values.insert(Values.end(), nd->Values.begin(), nd->Values.end());
	}

	for (i=0; i<Values.size(); ++i)
	{
		if (MinValue > Values[i].second)
			MinValue = Values[i].second;
		if (MaxValue < Values[i].second)
			MaxValue = Values[i].second;
	}

	CurrentEnumNode = NULL;
	return true;
}

bool	CEnumSimpleValueNode::prolog()
{
	CEnumNode	*parent = dynamic_cast<CEnumNode*>(Parent);
	if (parent != NULL)
	{
		CurrentValue = parent->CurrentValue;
	}
	else
	{
		CurrentValue = 0;
	}
	uint	i;
	for (i=0; i<Names.size(); ++i)
	{
		CurrentEnumNode->Values.push_back(make_pair<string, uint32>(Names[i], CurrentValue));
	}
	if (parent != NULL)
		++(parent->CurrentValue);

	return true;
}

bool	CEnumSimpleValueNode::epilog()
{
	return true;
}

bool	CEnumRangeNode::prolog()
{
	CEnumNode	*parent = dynamic_cast<CEnumNode*>(Parent);
	if (parent != NULL)
	{
		CurrentValue = parent->CurrentValue;
	}
	else
	{
		CurrentValue = 0;
	}

	CurrentEnumNode->Values.push_back(make_pair<string, uint32>(Name, CurrentValue));

	return true;
}

bool	CEnumRangeNode::epilog()
{
	CEnumNode	*parent = dynamic_cast<CEnumNode*>(Parent);
	if (parent != NULL)
	{
		parent->CurrentValue = CurrentValue;
	}

	uint	i;
	for (i=0; i<Nodes.size(); ++i)
	{
		CEnumNode	*nd = dynamic_cast<CEnumNode*>(Nodes[i]);
		if (!nd)
			continue;
		Values.insert(Values.end(), nd->Values.begin(), nd->Values.end());
	}

	if (!EndRange.empty())
	{
		CurrentEnumNode->Values.push_back(make_pair<string, uint32>(EndRange, CurrentValue));
	}

	return true;
}


bool	CEnumNode::generateContent()
{
	hOutput() << "/** " << Name << "\n";
	if (!Description.empty())
	{
		hOutput() << Description << "\n";
	}
	uint	line, col;
	string	file;
	getFileLine(line, col, file);
	hOutput() << "defined at " << file << ":" << line << "\n";
	hOutput() << "*/\n";

	string	enumTruncName = Name.substr(1);
	CClassGenerator	gen;
	gen.init("C"+enumTruncName);
	gen.createPublic("enum", "Enum values", "");
	gen.createPublic("conv", "Conversion methods", "Use these methods to convert from enum value to string (and vice versa)");
	gen.createPrivate("init", "Enum initialisation", "");

	uint	j;
	gen.addOther("enum "+Name+"\n", "enum");
	gen.addOther("{\n", "enum");
	for (j=0; j<Values.size(); ++j)
		gen.addOther(Values[j].first+" = "+toString(Values[j].second)+",\n", "enum");
	gen.addOther(getUnscopedUseSize()+" = "+toString(MaxValue-MinValue+1)+",\n", "enum");
	gen.addOther(getUnknownValue()+" = "+toString(MaxValue-MinValue+1)+",\n", "enum");
	if (!EndRange.empty())
		gen.addOther(EndRange+" = "+toString(MaxValue-MinValue+1)+",\n", "enum");
	gen.addOther("};\n", "enum");

	gen.startMethod("const std::string&", "toString", Name+" v", "conv", false, true, true);
	gen.add("if (v < 0 || v >= "+getUnscopedUseSize()+")");
	gen.add("{");
	gen.add("nlwarning(\""+Name+"::toString(): value '%u' is not matched, \\\"Unknown\\\" string returned\", v);");
	gen.add("return _UnknownString;");
	gen.add("}");
	//gen.add(checkCode("v"));
	gen.add("if (!_Initialised)");
	gen.add("{");
	gen.add("init();");
	gen.add("}");
	gen.add("return _StrTable[v];");

	gen.startMethod(getName(), "fromString", "const std::string& v", "conv", false, true, true);
	gen.add("if (!_Initialised)");
	gen.add("{");
	gen.add("init();");
	gen.add("}");
	gen.add("if(v==_UnknownString)");
	gen.add("{");
	gen.add("return Unknown;");
	gen.add("}");
	gen.add("const std::map<std::string, "+Name+">::const_iterator\tit = _ValueMap.find(NLMISC::toLower(v));");
	gen.add("if (it == _ValueMap.end())");
	gen.add("{");
	gen.add("nlwarning(\""+Name+"::toString(): string '%s' is not matched, 'Unknown' enum value returned\", v.c_str());");
	gen.add("return "+getUnknownValue()+";");
	gen.add("}");
	gen.add("return (*it).second;");


	gen.startMethod("void", "init", "", "init", false, false, true);
	gen.add("_StrTable.clear();");
	gen.add("_ValueMap.clear();");
	gen.add("_StrTable.resize("+toString(getSize()+1)+");");
	gen.add("uint\ti;");
	gen.add("for (i=0; i<"+toString(Values.size())+"; ++i)");
	gen.add("{");
	gen.add("_StrTable["+Name+"Convert[i].Value] = "+Name+"Convert[i].Name;");
	gen.add("_ValueMap[NLMISC::toLower(std::string("+Name+"Convert[i].Name))] = "+Name+"Convert[i].Value;");
	gen.add("}");

	gen.add("_Initialised = true;");


	gen.addAttribute("bool", "_Initialised", "init", true, "false");
	gen.addAttribute("std::string", "_UnknownString", "init", true, "\""+getUnknownValue()+"\"");
	gen.addAttribute("std::vector<std::string>", "_StrTable", "init", true);
	gen.addAttribute("std::map<std::string, "+Name+">", "_ValueMap", "init", true, "", false, "std::map<std::string, "+getName()+">");

	cppOutput() << "static const struct { char* Name; " << getName() << " Value; } " << Name << "Convert[] =\n";
	cppOutput() << "{\n";
	for (j=0; j<Values.size(); ++j)
		cppOutput() << "{ \"" << Values[j].first << "\", C"+enumTruncName+"::"+Values[j].first+" },\n";
	cppOutput() << "};\n";

	gen.flush(hOutput(), cppOutput(), inlineOutput());

	return true;
}





// Class Node
bool	CClassNode::prolog()
{
	CDbNode*	db = getDbNode();
	db->ClassNodes.push_back(this);

	return true;
}

bool	CClassNode::epilog()
{
	return true;
}

string	CClassNode::getUserCode(const string& name)
{
	uint	i;
	for (i=0; i<Nodes.size(); ++i)
	{
		CCppCodeNode	*code = dynamic_cast<CCppCodeNode*>(Nodes[i]);
		if (!code)
			continue;

		if (code->Name == name)
			return code->RawCode;
	}

	return "";
}


void	CClassNode::checkClassReferences()
{
	if (!Implements.empty())
	{
		Gen.Inherit += (HasRowAccess ? string(", ") : string(""))+"public "+Implements;
		getDbNode()->Implemented.insert(Implements);
	}


	if (!Inherited.empty())
	{
		HasInheritance = true;
		CClassNode	*nd = getClassNode(Inherited);
		nd->HasInheritance = true;
		nd->ChildrenClasses.push_back(Name);

		Dependencies.insert(nd);
		getFileNode()->Dependencies.insert(nd->getFileNode());
	}

	CClassNode*	inherit = this;
	while (inherit != NULL)
	{
		if (MappedFlag)
			inherit->PDSMapped = true;

		inherit->Legacy.insert(this);
		inherit = getClassNode(inherit->Inherited, false);
	}

	uint	i;
	uint	id = 0;
	for (i=0; i<Nodes.size(); ++i)
	{
		CDeclarationNode	*decl = dynamic_cast<CDeclarationNode*>(Nodes[i]);
		if (!decl)
			continue;

		decl->Id = id++;

		if (decl->ParentFlag)
		{
			if (HasParent)
				decl->error("class '"+Name+"' already has a parent");

			//if (MappedFlag)
			//	decl->error("class '"+Name+"' can't have a parent and be mapped at the same time");

			CClassNode*	inherit = this;
			while (inherit != NULL)
			{
				inherit->PDSMapped = false;
				inherit = getClassNode(inherit->Inherited, false);
			}

			ParentClass = decl->ParentClass;
			HasParent = true;

			ParentIsHidden = decl->HiddenFlag;

			decl->getClassNode(decl->ParentClass)->IsBackReferenced = true;
		}
		else if (decl->SetFlag)
		{
			decl->getClassNode(decl->Type)->IsInSet = true;
		}
	}
}

void	CClassNode::fillAttributes()
{
	if (HasParent && !IsBackReferenced && !HasInheritance && !IsInSet && !DerivatedFlag && !MappedFlag)
		error("class '"+Name+"' has a parent whereas it is not backreferenced, has no inherited link and is not mapped");

	uint	i;
	for (i=0; i<Nodes.size(); ++i)
	{
		CDeclarationNode	*decl = dynamic_cast<CDeclarationNode*>(Nodes[i]);
		if (!decl)
			continue;

		uint	j;
		for (j=0; j<Nodes.size(); ++j)
			if (j != i && dynamic_cast<CDeclarationNode*>(Nodes[j]) != NULL && Nodes[j]->Name == decl->Name)
				decl->error("attribute '"+decl->Name+"' already defined");

		if (decl->ParentFlag)
		{
			decl->DeclarationType = BackRef;
		}
		else if (decl->ArrayFlag)
		{
			CClassNode	*classNd = NULL;
			CTypeNode	*typeNd = NULL;

			if ( (classNd = decl->getClassNode(decl->Type, false)) )
			{
				if (classNd->IsBackReferenced || classNd->ForceReference)
				{
					if (decl->ForwardRefAttribute.empty())
						decl->error("no forward reference to parent in array declaration, class '"+decl->Type+"' is backref'd or has inheritance");

					classNd->IsInArrayRef = true;
					decl->IsRef = true;
					decl->DeclarationType = ArrayRef;

					if (classNd->ParentIsHidden)
						Dependencies.insert(classNd);
				}
				else
				{
					if (!decl->ForwardRefAttribute.empty())
						decl->error("forward reference declared whereas subclass is not backreferenced and has no inheritance link");

					Dependencies.insert(classNd);

					classNd->IsInArray = true;
					decl->IsRef = false;
					decl->DeclarationType = ArrayClass;
				}
			}
			else if ( (typeNd = decl->getTypeNode(decl->Type, false)) )
			{
				decl->DeclarationType = ArrayType;
			}
			else
			{
				decl->error("type or class '"+decl->Type+"' not found");
			}
		}
		else if (decl->SetFlag)
		{
			decl->IsRef = true;
			decl->DeclarationType = Set;
		}
		else
		{
			CClassNode	*classNd = NULL;
			CTypeNode	*typeNd = NULL;

			if ( (classNd = decl->getClassNode(decl->Type, false)) )
			{
				if (classNd->IsBackReferenced || classNd->ForceReference)
				{
					if (decl->ForwardRefAttribute.empty())
						decl->error("no forward reference to parent in array declaration, class '"+decl->Type+"' is backref'd or has inheritance");

					decl->IsRef = true;

					decl->DeclarationType = ForwardRef;
				}
				else
				{
					if (!decl->ForwardRefAttribute.empty())
						decl->error("forward reference declared whereas subclass is not backreferenced and has no inheritance link");

					Dependencies.insert(classNd);

					decl->IsRef = false;

					decl->DeclarationType = SimpleClass;
				}
			}
			else if ( (typeNd = decl->getTypeNode(decl->Type, false)) )
			{
				decl->IsType = true;

				decl->DeclarationType = SimpleType;
			}
			else
			{
				decl->error("type or class '"+decl->Type+"' not found");
			}
		}
	}

	CDeclarationNode	*declNd = getClassKey();
	if (declNd != NULL)
	{
		if (!declNd->IsType)
			error("attribute '"+declNd->Name+"' can't be a key, only simple type allowed");

		declNd->IsKey = true;
	}
}

void	CClassNode::fillRefs()
{
	uint	i;
	for (i=0; i<Nodes.size(); ++i)
	{
		CDeclarationNode	*decl = dynamic_cast<CDeclarationNode*>(Nodes[i]);
		if (!decl)
			continue;

		switch (decl->DeclarationType)
		{
		case BackRef:
			{
				// check parent is a valid class
				CClassNode			*cln = decl->getClassNode(decl->ParentClass);
				CDeclarationNode	*dln = dynamic_cast<CDeclarationNode*>(cln->getNode(decl->ParentField));
				if (!dln)	decl->error("attribute '"+decl->ParentField+"' not found in class '"+decl->ParentClass+"'");

				if (!dln->ArrayFlag && !dln->SetFlag && !ForceReference)
					decl->error("back reference 'parent "+decl->ParentClass+":"+decl->ParentField+" "+decl->Name+"' is not forwarded in class '"+decl->ParentClass+"' with an array or a set");

				if (dln->Type != Name || dln->ForwardRefAttribute != decl->Name)
					decl->error("back reference 'parent "+decl->ParentClass+":"+decl->ParentField+" "+decl->Name+"' is not correctly forwarded in class '"+decl->ParentClass+"'");

				Friends.insert(cln->Name);
			}
			break;

		case Set:
			{
				CClassNode			*cln = decl->getClassNode(decl->Type);
				CDeclarationNode	*dln = dynamic_cast<CDeclarationNode*>(cln->getNode(decl->ForwardRefAttribute));
				if (!dln)	decl->error("attribute '"+decl->ForwardRefAttribute+"' not found in class '"+decl->Type+"'");

				if (!dln->ParentFlag)
					decl->error("set '"+decl->Type+":"+decl->ForwardRefAttribute+"<> "+decl->Name+"' is not backref'd in class '"+decl->Type+"'");

				if (dln->ParentClass != Name || dln->ParentField != decl->Name)
					decl->error("set '"+decl->Type+":"+decl->ForwardRefAttribute+"<> "+decl->Name+"' is not correctly backref'd in class '"+decl->Type+"'");

				if (cln->getClassKey() == NULL)
					decl->error("class '"+decl->Type+"' has no key defined, whereas it is used in a set");

				cln->Friends.insert(Name);
				ForwardFriends.insert(cln->Name);
			}
			break;

		case ArrayRef:
			{
				if (decl->ForwardRefAttribute.empty())
					decl->error("No forward reference defined");

				CClassNode			*cln = decl->getClassNode(decl->Type);
				CDeclarationNode	*dln = dynamic_cast<CDeclarationNode*>(cln->getNode(decl->ForwardRefAttribute));
				CTypeNode			*tln = decl->getTypeNode(decl->ArrayIndex);
				getFileNode()->Dependencies.insert(tln->getFileNode());
				if (!dln)	decl->error("attribute '"+decl->ForwardRefAttribute+"' not found in class '"+decl->Type+"'");

				if (!dln->ParentFlag)
					decl->error("array '"+decl->Type+":"+decl->ForwardRefAttribute+"["+decl->ArrayIndex+"] "+decl->Name+"' is not backref'd in class '"+decl->Type+"'");

				if (dln->ParentClass != Name || dln->ParentField != decl->Name)
					decl->error("array '"+decl->Type+":"+decl->ForwardRefAttribute+"["+decl->ArrayIndex+"] "+decl->Name+"' is not correctly backref'd in class '"+decl->Type+"'");

				if (cln->getClassKey() == NULL)
					decl->error("class '"+decl->Type+"' has no key defined, whereas it is used in an array of ref");
				CDeclarationNode	*kdn = dynamic_cast<CDeclarationNode*>(cln->getClassKey());
				if (!kdn)	decl->error("attribute '"+cln->ClassKey+"' not found in class '"+cln->Name+"'");

				if (kdn->Type != decl->ArrayIndex)
					decl->error("type in array definition mismatch class '"+cln->Name+"' key definition");

				cln->Friends.insert(Name);
				ForwardFriends.insert(cln->Name);
			}
			break;

		case ForwardRef:
			{
				if (decl->ForwardRefAttribute.empty())
					decl->error("No forward reference defined");

				CClassNode			*cln = decl->getClassNode(decl->Type);
				CDeclarationNode	*dln = dynamic_cast<CDeclarationNode*>(cln->getNode(decl->ForwardRefAttribute));
				if (!dln)	decl->error("attribute '"+decl->ForwardRefAttribute+"' not found in class '"+decl->Type+"'");

				if (!dln->ParentFlag)
					decl->error("set '"+decl->Type+":"+decl->ForwardRefAttribute+"<> "+decl->Name+"' is not backref'd in class '"+decl->Type+"'");

				if (dln->ParentClass != Name || dln->ParentField != decl->Name)
					decl->error("set '"+decl->Type+":"+decl->ForwardRefAttribute+"<> "+decl->Name+"' is not correctly backref'd in class '"+decl->Type+"'");

				cln->Friends.insert(Name);
				ForwardFriends.insert(cln->Name);
			}
			break;

		case ArrayType:
			{
				CTypeNode			*tln = decl->getTypeNode(decl->ArrayIndex);
				getFileNode()->Dependencies.insert(tln->getFileNode());
			}
		case SimpleType:
			break;

		case ArrayClass:
			{
				CTypeNode			*tln = decl->getTypeNode(decl->ArrayIndex);
				getFileNode()->Dependencies.insert(tln->getFileNode());
			}
		case SimpleClass:
			{
				CClassNode			*cln = decl->getClassNode(decl->Type);
				cln->Friends.insert(Name);
			}
			break;

		default:
			decl->error("Can't decide declaration type");
			break;
		}
	}
}

void	CClassNode::computeFriends()
{
	bool	added;

	do
	{
		added = false;
		set<string>::iterator	itf, itsf;

		for (itf=Friends.begin(); !added && itf!=Friends.end(); ++itf)
		{
			CClassNode*	pfriend = getClassNode(*itf);

			for (itsf=pfriend->Friends.begin(); !added && itsf!=pfriend->Friends.end(); ++itsf)
			{
				const string&	sfriend = *itsf;
				if (Friends.find(*itsf) == Friends.end())
				{
					Friends.insert(*itsf);
					added = true;
				}
			}
		}
	}
	while (added);
}


void	CClassNode::checkDependencies(set<CClassNode*> &beingChecked,
									  set<CClassNode*> &checkedClasses,
									  vector<CClassNode*> &classesOrder)
{
	if (beingChecked.find(this) != beingChecked.end())
		error("circular dependency in class '"+Name+"'");

	if (checkedClasses.find(this) != checkedClasses.end())
		return;

	beingChecked.insert(this);
	checkedClasses.insert(this);

	set<CClassNode*>::iterator	it;
	for (it=Dependencies.begin(); it!=Dependencies.end(); ++it)
	{
		CClassNode	*classNode = *it;

		classNode->checkDependencies(beingChecked, checkedClasses, classesOrder);
	}

	classesOrder.push_back(this);
}


//
void	CClassNode::buildInit()
{
	CDbNode	*db = getDbNode();

	if (!Init.empty())
		return;

	if (!Inherited.empty())
	{
		CClassNode	*mother = getClassNode(Inherited);
		if (mother)
		{
			mother->buildInit();
			Init = mother->Init;

			uint	i;
			for (i=0; i<Init.size(); ++i)
			{
				if (!InitProto.empty())
					InitProto += ", ";
				if (!InitCallArgs.empty())
					InitCallArgs += ", ";

				CTypeNode*	typeNode = getTypeNode(Init[i]->Type);
				InitProto += "const "+typeNode->getName()+" &"+Init[i]->Name;
				InitCallArgs += Init[i]->Name;
			}
		}
	}

	if (!ClassKey.empty())
	{
		CDeclarationNode	*decl = dynamic_cast<CDeclarationNode*>(getNode(ClassKey));
		if (decl)
		{
			Init.push_back(decl);

			if (!InitProto.empty())
				InitProto += ", ";

			CTypeNode*	typeNode = getTypeNode(decl->Type);
			InitProto += "const "+typeNode->getName()+" &"+decl->Name;
		}
	}
}






void	CClassNode::computeAttributesColumns()
{
	if (Columns >= 0)
		return;

	CDbNode	*db = getDbNode();

	Columns = 0;
	if (!Inherited.empty())
	{
		CClassNode	*mother = getClassNode(Inherited);
		mother->computeAttributesColumns();
		Columns = mother->Columns;

		Attributes = mother->Attributes;
	}

	uint	attribId = (uint)Attributes.size();
	uint	i;
	for (i=0; i<Nodes.size(); ++i)
	{
		CDeclarationNode	*decl = dynamic_cast<CDeclarationNode*>(Nodes[i]);
		if (decl == NULL)
			continue;

		Attributes.push_back(decl);

		decl->Column = Columns;
		decl->Id = attribId++;
		CColumn		col;

		// All for backref, set, forwardref, type
		switch (decl->DeclarationType)
		{
		case ForwardRef:
			decl->Columns = 1;
			col.Name = decl->Name;
			col.Type = ForwardRef;
			col.TypeStr = "forwardref";
			col.TypeId = decl->getClassNode(decl->Type)->Id;
			col.ByteSize = 8;
			decl->ColumnList.push_back(col);
			break;
		case BackRef:
			decl->Columns = 1;
			col.Name = decl->Name;
			col.Type = BackRef;
			col.TypeStr = "backref";
			col.TypeId = decl->getClassNode(decl->ParentClass)->Id;
			col.ByteSize = 8;
			decl->ColumnList.push_back(col);
			break;
		case SimpleType:
			decl->Columns = 1;
			col.Name = decl->Name;
			col.Type = SimpleType;
			col.TypeStr = "type";
			col.TypeId = decl->getTypeNode(decl->Type)->Id;
			col.ByteSize = decl->getTypeNode(decl->Type)->Size;
			decl->ColumnList.push_back(col);
			break;
		case Set:
			decl->Columns = 1;
			col.Name = decl->Name;
			col.Type = Set;
			col.TypeStr = "set";
			col.TypeId = decl->getClassNode(decl->Type)->Id;
			col.ByteSize = 4;
			decl->ColumnList.push_back(col);
			break;
		case SimpleClass:
			{
				CClassNode	*sub = decl->getClassNode(decl->Type);
				sub->computeAttributesColumns();
				decl->Columns = sub->Columns;

				uint	i, j;
				for (i=0; i<sub->Attributes.size(); ++i)
				{
					CDeclarationNode	*attrib = sub->Attributes[i];

					for (j=0; j<attrib->ColumnList.size(); ++j)
					{
						col.Name = decl->Name+"."+attrib->ColumnList[j].Name;
						col.Type = attrib->ColumnList[j].Type;
						col.TypeStr = attrib->ColumnList[j].TypeStr;
						col.TypeId = attrib->ColumnList[j].TypeId;
						col.ByteSize = attrib->ColumnList[j].ByteSize;
						decl->ColumnList.push_back(col);
					}
				}
			}
			break;
		case ArrayRef:
			{
				CIndexNode	*indexNd = decl->getIndexNode(decl->ArrayIndex);
				uint		numInEnum = indexNd->getSize();
				decl->Columns = numInEnum;

				uint	i;
				for (i=0; i<numInEnum; ++i)
				{
					col.Name = decl->Name+"["+indexNd->getIndexName(i)+"]";
					col.Type = ForwardRef;
					col.TypeStr = "forwardref";
					col.TypeId = decl->getClassNode(decl->Type)->Id;
					col.ByteSize = 8;
					decl->ColumnList.push_back(col);
				}
			}
			break;
		case ArrayType:
			{
				CIndexNode	*indexNd = decl->getIndexNode(decl->ArrayIndex);
				uint		numInEnum = indexNd->getSize();
				decl->Columns = numInEnum;

				uint	i;
				for (i=0; i<numInEnum; ++i)
				{
					col.Name = decl->Name+"["+indexNd->getIndexName(i)+"]";
					col.Type = SimpleType;
					col.TypeStr = "type";
					col.TypeId = decl->getTypeNode(decl->Type)->Id;
					col.ByteSize = decl->getTypeNode(decl->Type)->Size;
					decl->ColumnList.push_back(col);
				}
			}
			break;
		case ArrayClass:
			{
				CIndexNode	*indexNd = decl->getIndexNode(decl->ArrayIndex);

				CClassNode	*sub = decl->getClassNode(decl->Type);
				sub->computeAttributesColumns();

				uint		numInEnum = indexNd->getSize();
				decl->Columns = numInEnum*sub->Columns;

				uint	i, j, k;
				for (k=0; k<numInEnum; ++k)
				{
					for (i=0; i<sub->Attributes.size(); ++i)
					{
						CDeclarationNode	*attrib = sub->Attributes[i];

						for (j=0; j<attrib->ColumnList.size(); ++j)
						{
							col.Name = decl->Name+"["+indexNd->getIndexName(k)+"]."+attrib->ColumnList[j].Name;
							col.Type = attrib->ColumnList[j].Type;
							col.TypeStr = attrib->ColumnList[j].TypeStr;
							col.TypeId = attrib->ColumnList[j].TypeId;
							col.ByteSize = attrib->ColumnList[j].ByteSize;
							decl->ColumnList.push_back(col);
						}
					}
				}
			}
			break;
		}

		Columns += decl->Columns;
	}
}





bool	CClassNode::generateContent()
{
	nlassert(Env != NULL);

	uint	line, col;
	string	file;
	getFileLine(line, col, file);

	setEnv("name", Name);
	if (!Description.empty())
		setEnv("description", Description);
	setEnv("deffile", file);
	setEnv("defline", line);

	computeAttributesColumns();

	uint	j;

	//
	// generate description
	//
	hOutput() << "/** " << Name << "\n";
	if (!Description.empty())
		hOutput() << Description << "\n";

	hOutput() << "defined at " << file << ":" << line << "\n";
	hOutput() << "*/\n";

	CCppOutput&	DbSummary = getDbNode()->DbSummary;
	DbSummary << "Class " << getDbNode()->Name << "::" << Name << ":\n";
	DbSummary << "----------------------------------------------------------\n";
	DbSummary << "located in file \"" << getFullStdPathNoExt(getFileNode()->Name) << ".h\"\n";
	DbSummary << "defined in file \"" << getFullStdPath(file) << "\"\n";
	DbSummary << "The class contains:\n\n";

	Gen.init(Name);
	Gen.createPublic("methods", "Accessors and Mutators methods", "Use these methods to change a value, add or delete elements.");
	Gen.createPublic("map", "Public Management methods", "Use these methods to create, load, unload and get\nan object from database.");
	Gen.createPublic("user", "User defined attributes and methods", "This code was verbatim copied from source file");
	Gen.createPublic("construct", "Public constructor", "This constructor is public to allow direct instanciation of the class");
	Gen.createPublic("persist", "Persistent methods declaration", "");
	Gen.createProtected("userinit", "User defined init and release methods", "Overload those methods to implement init and release behaviours");
	Gen.createProtected("attributes", "Attributes", "Don't modify those value manually, use accessors and mutators above");
	Gen.createProtected("internal", "Internal Management methods");
	Gen.createProtected("inherit map");
	Gen.createProtected("factories", "Default Factory and Fetch methods");
	Gen.createProtected("friends");

	// EGS Compat
	// -- begin

	Gen.startRaw("persist", false);
	ApplyId = Gen.startMethod("void", applyFunction, "CPersistentDataRecord &__pdr", "persist", false, inlineInternal, false, false, "", HasInheritance);
	StoreId = Gen.startMethod("void", storeFunction, "CPersistentDataRecord &__pdr", "persist", true, inlineInternal, false, false, "", HasInheritance);

	ClearId = Gen.startMethod("void", clearFunction, "", "map", false, inlineStaticPublic, false, false, "", HasInheritance);
	Gen.setDescription("Clear whole object content but key (delete subobjects if there are, key is left unmodified), default clear value is 0.");

	StoreId.add("uint16\t__Tok_MapKey = __pdr.addString(\"__Key__\");");
	StoreId.add("uint16\t__Tok_MapVal = __pdr.addString(\"__Val__\");");
	StoreId.add("uint16\t__Tok_ClassName = __pdr.addString(\"__Class__\");");
	ApplyId.add("uint16\t__Tok_MapKey = __pdr.addString(\"__Key__\");");
	ApplyId.add("uint16\t__Tok_MapVal = __pdr.addString(\"__Val__\");");
	ApplyId.add("uint16\t__Tok_ClassName = __pdr.addString(\"__Class__\");");

	if (!Inherited.empty())
	{
		StoreId.add("uint16\t__Tok_Parent = __pdr.addString(\"__Parent__\");");
		ApplyId.add("uint16\t__Tok_Parent = __pdr.addString(\"__Parent__\");");
	}

	for (j=0; j<Attributes.size(); ++j)
	{
		CDeclarationNode*	decl = Attributes[j];
		if (decl->Parent != this)
			continue;

		if (decl->DeclarationType == BackRef)
		{
			ApplyId.add(decl->cppName()+" = NULL;");
		}
		else
		{
			StoreId.add("uint16\t"+decl->tokenName()+" = __pdr.addString(\""+decl->Name+"\");");
			ApplyId.add("uint16\t"+decl->tokenName()+" = __pdr.addString(\""+decl->Name+"\");");
		}
	}

	ApplyId.add("while (!__pdr.isEndOfStruct())");
	ApplyId.add("{");
	ApplyId.add(	"if (false) {}");

	if (!Inherited.empty())
	{
		StoreId.add("__pdr.pushStructBegin(__Tok_Parent);");
		StoreId.add(Inherited+"::store(__pdr);");
		StoreId.add("__pdr.pushStructEnd(__Tok_Parent);");

		ApplyId.add("else if (__pdr.peekNextToken() == __Tok_Parent)");
		ApplyId.add("{");
		ApplyId.add(	"__pdr.popStructBegin(__Tok_Parent);");
		ApplyId.add(	Inherited+"::apply(__pdr);");
		ApplyId.add(	"__pdr.popStructEnd(__Tok_Parent);");
		ApplyId.add("}");
	}

	// -- end


	for (j=0; j<Nodes.size(); ++j)
	{
		CCppCodeNode*	cpp = dynamic_cast<CCppCodeNode*>(Nodes[j]);
		if (cpp == NULL || !cpp->Name.empty())
			continue;

		Gen.addOther(cpp->RawCode, "user");
	}

	HasRowAccess = false;
	HasTableAccess = false;

	if (!Inherited.empty())
	{
		setEnv("inherit", Inherited);
		setEnv("inheritclass", Inherited);

		Gen.Inherit += "public "+Inherited;
		HasRowAccess = true;
		HasTableAccess = true;
	}
	else if (HasInheritance || IsBackReferenced || IsInSet || ForceReference)
	{
		setEnv("inherit", pdBaseDataName);

		Gen.Inherit += "public "+pdBaseDataName;
		HasTableAccess = true;
		HasRowAccess = true;
	}

	if (Legacy.size() > 1 && HasTableAccess)
	{
		CClassGenerator::SMethodId	castId = Gen.startMethod(Name+"*", staticCastFunction, pdBaseDataName+"* obj", "map", false, inlineStaticPublic, true, false, "", false, false);
		Gen.setDescription("Cast base object to "+Name);
		castId.add("switch (obj->getTable())");
		castId.add("{");
		std::set<CClassNode*>::iterator	itl;
		for (itl=Legacy.begin(); itl!=Legacy.end(); ++itl)
		{
			CClassNode*	child = (*itl);
			castId.add("case "+toString(child->Id)+":");
		}
		castId.add("return static_cast<"+Name+"*>(obj);");
		castId.add("}");
		castId.add("return NULL;");

		CClassGenerator::SMethodId	constCastId = Gen.startMethod("const "+Name+"*", staticConstCastFunction, "const "+pdBaseDataName+"* obj", "map", false, inlineStaticPublic, true, false, "", false, false);
		Gen.setDescription("Cast base object to const "+Name);
		constCastId.add("switch (obj->getTable())");
		constCastId.add("{");
		for (itl=Legacy.begin(); itl!=Legacy.end(); ++itl)
		{
			CClassNode*	child = (*itl);
			constCastId.add("case "+toString(child->Id)+":");
		}
		constCastId.add("return static_cast<const "+Name+"*>(obj);");
		constCastId.add("}");
		constCastId.add("return NULL;");
	}
	else if (Legacy.size() == 1 && HasTableAccess)
	{
		CClassGenerator::SMethodId	castId = Gen.startMethod(Name+"*", staticCastFunction, pdBaseDataName+"* obj", "map", false, inlineStaticPublic, true, false, "", false, false);
		std::set<CClassNode*>::iterator	itl = Legacy.begin();
		Gen.setDescription("Cast base object to "+Name);
		castId.add("return (obj->getTable() == "+toString((*itl)->Id)+") ? static_cast<"+Name+"*>(obj) : NULL;");

		CClassGenerator::SMethodId	constCastId = Gen.startMethod("const "+Name+"*", staticConstCastFunction, "const "+pdBaseDataName+"* obj", "map", false, inlineStaticPublic, true, false, "", false, false);
		Gen.setDescription("Cast base object to const "+Name);
		constCastId.add("return (obj->getTable() == "+toString((*itl)->Id)+") ? static_cast<const "+Name+"*>(obj) : NULL;");
	}
	else
	{
	}

	if (HasRowAccess)
		define("hasrowaccess");
	if (HasTableAccess)
		define("hastableaccess");

	if (!Implements.empty())
	{
		setEnv("implements", Implements);
		Gen.Inherit += (HasRowAccess ? string(", ") : string(""))+"public "+Implements;
	}

	//
	// generate init method
	//
	buildInit();

	setEnv("initproto", InitProto);

	InitId = Gen.startMethod("void", initFunction, InitProto, "internal", false, inlineInternal, false, false, "", HasInheritance);
	if (!Inherited.empty())
	{
		InitId.add(Inherited + "::" + initFunction + "(" + InitCallArgs + ");");
		setEnv("initcallargs", InitCallArgs);
	}

	DestroyId				= Gen.startMethod("void", destroyFunction, "", "internal", false, inlineInternal, false, false, "", HasInheritance);
	FetchId					= Gen.startMethod("void", fetchFunction, CPDataName+" &data", "internal", false, inlineInternal, false, false, "", HasInheritance);
	RegisterId				= Gen.startMethod("void", registerFunction, "", "internal", false, inlineInternal, false, false, "", HasInheritance);
	RegisterAttributesId	= Gen.startMethod("void", registerAttributesFunction, "", "internal", false, inlineInternal, false, false, "", HasInheritance);
	UnregisterId			= Gen.startMethod("void", unregisterFunction, "", "internal", false, inlineInternal, false, false, "", HasInheritance);
	UnregisterAttributesId	= Gen.startMethod("void", unregisterAttributesFunction, "", "internal", false, inlineInternal, false, false, "", HasInheritance);
	SetParentId;
	SetUnnotifiedParentId;
	if (HasParent)
	{
		SetParentId = Gen.startMethod("void", setParentFunction, ParentClass+"* __parent", "internal", false, inlineInternal);
		SetUnnotifiedParentId = Gen.startMethod("void", setUnnotifiedParentFunction, ParentClass+"* __parent", "internal", false, inlineInternal);
	}

	if (HasRowAccess && GenerateDebugMessages)
	{
		DestroyId.add("if (RY_PDS::PDVerbose)\tnldebug(\""+Name+": destroy %u:%u\", "+getId()+", __BaseRow);");
		FetchId.add("if (RY_PDS::PDVerbose)\tnldebug(\""+Name+": fetch %u:%u\", "+getId()+", __BaseRow);");
	}

	if (!Inherited.empty())
	{
		FetchId.add(Inherited+"::"+fetchFunction+"(data);");
	}

	if (DerivatedFlag)
	{
		UserInitId		= Gen.startMethod("void", userInitFunction, "", "userinit", false, inlineUserInitDefaultCode, false, false, "", true);
		UserReleaseId	= Gen.startMethod("void", userReleaseFunction, "", "userinit", false, inlineUserInitDefaultCode, false, false, "", true);
	}

	NotifyInitId		= Gen.startMethod("void", notifyInitFunction, "", "internal", false, inlineInternal, false, false, "", HasInheritance);
	string	initUCode = getUserCode("onInit");
	if (!Inherited.empty())
		Gen.add(Inherited+"::"+notifyInitFunction+"();");
	if (DerivatedFlag)
		Gen.add(userInitFunction+"();");
	if (!initUCode.empty())
	{
		Gen.add("{");
		uint	line, col;
		string	file;
		getFileLine(line, col, file);
		Gen.add("// "+Name+" init user code, defined at "+file+":"+toString(line));
		Gen.add(initUCode);
		Gen.add("}");
	}

	NotifyReleaseId		= Gen.startMethod("void", notifyReleaseFunction, "", "internal", false, inlineInternal, false, false, "", HasInheritance);
	string	releaseUCode = getUserCode("onRelease");
	if (!releaseUCode.empty())
	{
		Gen.add("{");
		uint	line, col;
		string	file;
		getFileLine(line, col, file);
		Gen.add("// "+Name+" release user code, defined at "+file+":"+toString(line));
		Gen.add(releaseUCode);
		Gen.add("}");
	}
	if (DerivatedFlag)
		Gen.add(userReleaseFunction+"();");
	if (!Inherited.empty())
		Gen.add(Inherited+"::"+notifyReleaseFunction+"();");
	else if (HasRowAccess)
		Gen.add(pdslibFunc("release")+"("+getId()+", __BaseRow);");

	if (!Inherited.empty())
	{
		DestroyId.add(Inherited+"::"+destroyFunction+"();");
		ClearId.add(Inherited+"::"+clearFunction+"();");
	}

	//
	// Generate XML description
	//
	string	xmlnode;
	xmlnode += "<classdef";
	xmlnode += " name='"+Name+"'";
	xmlnode += " id='"+toString(Id)+"'";
	if (!Inherited.empty())
	{
		CClassNode	*inh = getClassNode(Inherited);
		xmlnode += " inherit='"+toString(inh->Id)+"'";
	}
	CDeclarationNode	*dln = getClassKey();
	if (dln != NULL)
	{
		xmlnode += " key='"+toString(dln->Id)+"'";
	}
	if (MapClass && !MapClass->HasParent)
	{
		xmlnode += " mapped='"+toString(MapClass->Id)+"'";
	}
	if (HasRowAccess)
	{
/*
		if (!Reserve.empty())
		{
			xmlnode += " allocate='"+(Reserve)+"'";
		}
		else
		{
			xmlnode += " allocate='10000'";
		}
*/
	}
	xmlnode += " columns='"+toString(Columns)+"'";
	xmlnode += ">";
	getDbNode()->xmlDescription.push_back(xmlnode);

	indexUsedInInit = false;
	indexUsedInDestroy = false;
	indexUsedInFetch = false;
	tableAndRowIndicesUsedInFetch = false;
	indexUsedInRegister = false;
	indexUsedInUnregister = false;

	// generate code for init of new index
	if (HasRowAccess)
	{
		RegisterId.add("__BaseRow = _IndexAllocator.allocate();");
		if (GenerateDebugMessages)
		{
			if (MapClass != NULL)
			{
				CDeclarationNode*	key = MapClass->getKey();
				RegisterId.add("if (RY_PDS::PDVerbose)\tnldebug(\""+Name+": register %u:%u, key="+key->displayPrintfPrefix()+"\", "+getId()+", __BaseRow, "+key->displayCppCode()+");");
			}
			else
			{
				RegisterId.add("if (RY_PDS::PDVerbose)\tnldebug(\""+Name+": register %u:%u\", "+getId()+", __BaseRow);");
			}
		}

		string	oeid;
		if (useEntityId())
		{
			oeid = ", "+getClassKey()->cppName();
		}

		RegisterId.add(pdslibFunc("allocateRow")+"("+getId()+", __BaseRow, "+(PDSMapped ? MapClass->getKey()->toUint64() : "0")+oeid+");");
		RegisterId.add(registerAttributesFunction + "();");

		RegisterAttributesId.add("if (RY_PDS::PDVerbose)\tnldebug(\""+Name+": registerAttributes %u:%u\", "+getId()+", __BaseRow);");

		// send key to the pds (if key exists)
		if (!ClassKey.empty())
		{
			CDeclarationNode	*keyNode = getKey();
			CTypeNode			*keyTypeNode = getTypeNode(keyNode->Type);
			///// TYPE CAST
			RegisterAttributesId.add(pdslibFunc("set")+"("+getId()+", __BaseRow, ("+TColumnIndexName+")("+toString(keyNode->Column)+"), "+keyTypeNode->castToPDS(getKey()->cppName())+");");
		}

		if (!Inherited.empty())
		{
			RegisterAttributesId.add(Inherited + "::" + registerAttributesFunction + "();");
		}

		UnregisterAttributesId.add("if (RY_PDS::PDVerbose)\tnldebug(\""+Name+": unregisterAttributes %u:%u\", "+getId()+", __BaseRow);");

		if (!Inherited.empty())
		{
			UnregisterAttributesId.add(Inherited + "::" + unregisterAttributesFunction + "();");
		}

		if (HasParent)
		{
			UnregisterAttributesId.add(setParentFunction+"(NULL);");
		}

		if (GenerateDebugMessages)
		{
			if (MapClass != NULL)
			{
				CDeclarationNode*	key = MapClass->getKey();
				UnregisterId.add("if (RY_PDS::PDVerbose)\tnldebug(\""+Name+": unregister %u:%u, key="+key->displayPrintfPrefix()+"\", "+getId()+", __BaseRow, "+key->displayCppCode()+");");
			}
			else
				UnregisterId.add("if (RY_PDS::PDVerbose)\tnldebug(\""+Name+": unregister %u:%u\", "+getId()+", __BaseRow);");
		}

		UnregisterId.add(unregisterAttributesFunction + "();");

		oeid = "";
		if (useEntityId())
		{
			oeid = ", "+getClassKey()->cppName();
		}
		
		UnregisterId.add(pdslibFunc("deallocateRow")+"("+getId()+", __BaseRow"+oeid+");");
		UnregisterId.add("_IndexAllocator.deallocate(__BaseRow);");
		//UnregisterId.add(destroyFunction+"();");
	}

	//
	// add attributes and methods
	// - attributes are in private part
	// - read accessor are public
	// - write accessor are public or delegated in public accessor objects
	//
	for (j=0; j<Nodes.size(); ++j)
	{
		CDeclarationNode	*dln = dynamic_cast<CDeclarationNode*>(Nodes[j]);
		if (!dln)
			continue;

		dln->Env = Env->nextArrayNode("dcl");

		dln->generateContent();

	}

	uint	columnId = 0;

	for (j=0; j<Attributes.size(); ++j)
	{
		CDeclarationNode	*dln = Attributes[j];
		getDbNode()->xmlDescription.push_back("<attribute "+dln->XmlNode+"/>");

		uint	k;
		for (k=0; k<dln->ColumnList.size(); ++k)
		{
			CColumn		&column = dln->ColumnList[k];
			//getDbNode()->xmlDescription.push_back("<column id='"+toString(k)+"' name='"+column.Name+"' type='"+column.TypeStr+"' typeid='"+toString(column.TypeId)+"' size='"+toString(column.ByteSize)+"' columnid='"+toString(columnId)+"'/>");
			++columnId;
		}

		//getDbNode()->xmlDescription.push_back("</attribute>");
	}

	getDbNode()->xmlDescription.push_back("</classdef>");

	if (HasTableAccess)
	{
		Gen.startConstructor("", "construct");
		Gen.add("__BaseTable = "+toString(Id)+";");

		Gen.startDestructor("construct", true, DerivatedFlag || HasInheritance);
	}

	// when inited/fetched a mapped class, map id to object
	if (MappedFlag)
	{
		InitId.add("_Map["+getKey()->getFunc()+"()] = this;");
		FetchId.add("_Map["+getKey()->getFunc()+"()] = this;");
		DestroyId.add("_Map.erase("+getKey()->getFunc()+"());");
	}

	//
	// generate IPDBaseData API
	//

	if (MappedFlag || DerivatedFlag || HasInheritance || ForceReference)
	{
		if (DerivatedFlag)
		{
			Gen.startMethod("void", staticSetUserFactoryFunction, TPDFactoryName+" userFactory", "map", false, inlineStaticPublic, true);
			Gen.setDescription("Set user factory for this class (as class is indicated as derived, a home made constructor must be provided)");
			Gen.add(staticInitFactoryFunction+"(userFactory);");
		}

		Gen.startMethod("void", staticInitFactoryFunction, TPDFactoryName+" userFactory", "factories", false, inlineStaticInternal, true);
		Gen.add("if (!_FactoryInitialised)");
		Gen.add("{");
		Gen.add(pdslibFunc("registerClass")+"(" + toString(Id) + ", userFactory, "+staticFetchFunction+", "+((MappedFlag && !HasParent) ? staticNotifyLoadFailure : string("NULL"))+");");
		Gen.add("_FactoryInitialised = true;");
		Gen.add("}");

		Gen.addAttribute("bool", "_FactoryInitialised", "factories", true);

		if (MappedFlag || HasInheritance || ForceReference)
		{
			//
			// create: create an object, then init attributes and register
			//

			Gen.startMethod(Name+"*", staticCreateFunction, InitProto, "map", false, inlineStaticPublic, true);
			Gen.setDescription("Create an object of the "+Name+" class, and declare it to the PDS.");
			if (GenerateDebugMessages)
			{
				Gen.add("if (RY_PDS::PDVerbose)\tnldebug(\""+Name+": create\");");
			}
			if (DerivatedFlag)
			{
				Gen.add(Name + "\t*"+objectVariable+" = static_cast<" + Name + "*>("+pdslibFunc("create")+"("+toString(Id)+"));");
			}
			else
			{
				Gen.add(Name + "\t*"+objectVariable+" = static_cast<" + Name + "*>("+staticFactoryFunction+"());");
			}

			string	str = objectVariable+"->" + initFunction + "(";
			uint	i;
			for (i=0; i<Init.size(); ++i)
				str += (i != 0 ? ", " : "") + Init[i]->Name;
			str += ");";
			Gen.add(str);
			Gen.add(objectVariable+"->" + registerFunction + "();");
/*
			if (MappedFlag)
			{
				Gen.add("_Map["+MapClass->ClassKey+"] = "+objectVariable+";");
			}
*/
			Gen.add(objectVariable+"->"+notifyInitFunction+"();");
			Gen.add("return "+objectVariable+";");
		}

		if (MappedFlag)
		{
			CDeclarationNode	*dln = (MapClass != NULL ? MapClass->getKey() : NULL);
			CTypeNode			*keyType = getTypeNode(dln->Type);

			// only authorize remove/load/unload for mapped objects that are roots
			if (!HasParent)
			{
				Gen.startMethod("void", staticRemoveFunction, "const "+keyType->getName()+"& "+dln->Name, "map", false, inlineStaticPublic, true);
				Gen.setDescription("Destroy an object from the PDS. Caution! Object will no longer exist in database.\nAlso children (that is objects that belong to this object) are also destroyed.");
				if (GenerateDebugMessages)
				{
					Gen.add("if (RY_PDS::PDVerbose)\tnldebug(\""+Name+": remove "+dln->displayPrintfPrefix()+"\", "+dln->displayCppCode(dln->Name)+");");
				}
				Gen.add("std::map<" + keyType->getName() + "," + Name + "*>::iterator\tit = _Map.find("+dln->Name+");");
				Gen.add("if (it != _Map.end())");
				Gen.add("{");
				Gen.add(Name + "*\t__o = (*it).second;");
				Gen.add("__o->"+notifyReleaseFunction+"();");
				Gen.add("__o->"+unregisterFunction+"();");
				Gen.add("__o->"+destroyFunction+"();");
				Gen.add("delete __o;");
				//Gen.add("_Map.erase(it);");
				Gen.add("}");

				Gen.startMethod("void", staticLoadFunction, "const "+keyType->getName()+"& "+dln->Name, "map", false, inlineStaticPublic, true);
				Gen.setDescription("Retrieve an object from the database.\nData are sent asynchronously, so the load callback is called when data are ready.\nUse get() to access to the loaded object.");
				if (GenerateDebugMessages)
				{
					Gen.add("if (RY_PDS::PDVerbose)\tnldebug(\""+Name+": load "+dln->displayPrintfPrefix()+"\", "+dln->displayCppCode(dln->Name)+");");
				}
				Gen.add(pdslibFunc("load")+"("+toString(Id)+", "+dln->toUint64(dln->Name)+");");

				Gen.startMethod("void", staticSetLoadCbFunction, "void (*callback)(const "+keyType->getName()+"& key, "+Name+"* object)", "map", false, inlineStaticPublic, true);
				Gen.setDescription("Setup load callback so client is warned that load succeded or failed.");
				Gen.add(staticLoadCbAttribute+" = callback;");

				Gen.startMethod("void", staticNotifyLoadFailure, "uint64 key", "factories", false, inlineStaticInternal, true);
				Gen.add("if ("+staticLoadCbAttribute+" != NULL)");
				Gen.add("{");
				Gen.add(staticLoadCbAttribute+"("+keyType->castFromUser("key")+", NULL);");
				Gen.add("}");

				Gen.addAttribute("void", staticLoadCbAttribute, "factories", true, "NULL", true, "const "+keyType->getName()+"& key, "+Name+"* object");


				//
				Gen.startMethod("void", staticUnloadFunction, "const " + keyType->getName() + " &" + dln->Name, "map", false, inlineStaticPublic, true);
				Gen.setDescription("Unload an object from the client memory. Object still exists in database and can be retrieved again using load.");
				if (GenerateDebugMessages)
				{
					Gen.add("if (RY_PDS::PDVerbose)\tnldebug(\""+Name+": unload "+dln->displayPrintfPrefix()+"\", "+dln->displayCppCode(dln->Name)+");");
				}
				Gen.add("std::map<" + keyType->getName() + "," + Name + "*>::iterator\tit = _Map.find("+dln->Name+");");
				Gen.add("if (it != _Map.end())");
				Gen.add("{");
				Gen.add(Name + "*\t__o = (*it).second;");
				Gen.add("__o->"+notifyReleaseFunction+"();");
				Gen.add("__o->"+destroyFunction+"();");
				Gen.add("delete __o;");
				//Gen.add("_Map.erase(it);");
				Gen.add("}");
			}

			std::string	mapType = "std::map<"+keyType->getName()+", "+Name+"*>";

			Gen.startMethod(Name+"*", staticGetFunction, "const " + keyType->getName() + " &" + dln->Name, "map", false, inlineStaticPublic, true);
			Gen.setDescription("Get an object in client. Object must have been previously loaded from database with a load.");
			if (GenerateDebugMessages)
			{
				Gen.add("if (RY_PDS::PDVerbose)\tnldebug(\""+Name+": get "+dln->displayPrintfPrefix()+"\", "+dln->displayCppCode(dln->Name)+");");
			}
			Gen.add(mapType+"::iterator\t__it = _Map.find("+dln->Name+");");
			if (GenerateDebugMessages)
			{
				Gen.add("if (__it == _Map.end())");
				Gen.add("nlwarning(\""+Name+": unable to get %\"NL_I64\"u, not found in map.\", "+dln->toUint64(dln->Name)+");");
			}
			Gen.add("return (__it != _Map.end()) ? (*__it).second : NULL;");

			Gen.startMethod(mapType+"::iterator", staticBeginFunction, "", "map", false, inlineStaticPublic, true);
			Gen.setDescription("Return the begin iterator of the global map of "+Name);
			Gen.add("return _Map.begin();");

			Gen.startMethod(mapType+"::iterator", staticEndFunction, "", "map", false, inlineStaticPublic, true);
			Gen.setDescription("Return the end iterator of the global map of "+Name);
			Gen.add("return _Map.end();");
		}
	}


	//
	// generate internal management functions
	//

	if (HasRowAccess || MappedFlag)
	{
		if (MappedFlag)
		{
			CDeclarationNode	*dln = (MappedFlag ? getKey() : NULL);
			CTypeNode			*keyType = getTypeNode(dln->Type);
			Gen.addAttribute("std::map<" + keyType->getName() + "," + Name + "*>", "_Map", "inherit map", true);
		}

		Gen.addAttribute(indexAllocatorName, "_IndexAllocator", "factories", true);

		Gen.startMethod("void", staticInitFunction, "", "internal", false, inlineInternal, true, false, "", false);
		Gen.add(pdslibFunc("setIndexAllocator")+"("+toString(Id)+", _IndexAllocator);");
		if (MappedFlag || DerivatedFlag || HasInheritance || ForceReference)
		{
			if (DerivatedFlag)
			{
				// check factory has been set
				Gen.add("nlassertex(_FactoryInitialised, (\"User Factory for class "+Name+" not set!\"));");
				Gen.add("// factory must have been set by user before database init called!");
				Gen.add("// You must provide a factory for the class "+Name+" as it is marked as derived");
				Gen.add("// Call "+getDbNode()->Name+"::"+Name+"::"+staticSetUserFactoryFunction+"() with a factory before any call to "+getDbNode()->Name+"::init()!");
			}
			else
			{
				Gen.add(staticInitFactoryFunction+"("+staticFactoryFunction+");");
			}
		}

		getDbNode()->initDb.add(Name+"::"+staticInitFunction+"();");

		if (ForceReference)
		{
			if (!DerivatedFlag)	// forbid factory function for derivated classes
			{
				Gen.startMethod(pdBaseDataName+"*", staticFactoryFunction, "", "factories", false, inlineStaticInternal, true);
				Gen.add("return new " + Name + "();");
			}

			if (Inherited.empty())
			{
				Gen.startMethod("void", staticFetchFunction, pdBaseDataName+" *object, "+CPDataName+" &data", "factories", false, inlineStaticInternal, true);
				Gen.add(Name + "\t*"+objectVariable+" = static_cast<" + Name + "*>(object);");
				Gen.add(objectVariable+"->"+fetchFunction+"(data);");
				if (MappedFlag)
				{
					//Gen.add("_Map["+objectVariable+"->"+getKey()->getFunc()+"()] = "+objectVariable+";");
					if (!HasParent)
					{
						Gen.add("if ("+staticLoadCbAttribute+" != NULL)");
						Gen.add("{");
						Gen.add(staticLoadCbAttribute+"("+objectVariable+"->"+getKey()->getFunc()+"(), "+objectVariable+");");
						Gen.add("}");
					}
				}
				Gen.add(objectVariable+"->"+notifyInitFunction+"();");
			}
		}
	}

	// EGS Compat
	// -- begin

	ApplyId.add(	"else");
	ApplyId.add(	"{");
	ApplyId.add(		"nlwarning(\"Skipping unrecognised token: %s\", __pdr.peekNextTokenName().c_str());");
	ApplyId.add(		"__pdr.skipData();");
	ApplyId.add(	"}");
	ApplyId.add("}");

	if (MappedFlag && !HasParent)
	{
		ApplyId.add(notifyInitFunction+"();");
	}


	//EGSImplId.add("\n#include \"game_share/persistent_data_template.h\"");
	//EGSImplId.add("#undef PERSISTENT_CLASS");
	//EGSImplId.add("#undef PERSISTENT_DATA");

	// -- end

	set<string>::iterator	itf;
	for (itf=Friends.begin(); itf!=Friends.end(); ++itf)
		if (*itf != Name)
			Gen.addOther("friend class "+(*itf)+";\n", "friends");
	for (itf=ForwardFriends.begin(); itf!=ForwardFriends.end(); ++itf)
		if (*itf != Name)
			Gen.addOther("friend class "+(*itf)+";\n", "friends");
	Gen.addOther("friend class "+CPDSLibName+";\n", "friends");
	CDbNode*	dbNode = getDbNode();
	Gen.addOther("friend void "+dbNode->Name+"::init(uint32);\n", "friends");

	Gen.flush(hOutput(), cppOutput(), inlineOutput());

	DbSummary << "\n\n";

	return true;
}

void	CClassNode::generateContentInCall(CCallContext *context)
{
	uint	j;
	for (j=0; j<Nodes.size(); ++j)
	{
		CDeclarationNode	*dln = dynamic_cast<CDeclarationNode*>(Nodes[j]);
		if (!dln)
			continue;

		dln->generateContent(context);
	}
}









// Declaration Node
bool	CDeclarationNode::prolog()
{
	return true;
}

bool	CDeclarationNode::epilog()
{
	return true;
}



void	CDeclarationNode::generateContent(CCallContext *context)
{
	ClassNode = static_cast<CClassNode*>(Parent);

	nlassert(Env != NULL);

	setEnv("name", Name);

	XmlNode = "name='"+Name+"' id='"+toString(Id)+"' columnid='"+toString(Column)+"' columns='"+toString(Columns)+"'";

	if (context == NULL)
	{
		CCppOutput&	DbSummary = getDbNode()->DbSummary;
		DbSummary << "Attribute " << Name << ":\n";
	}

	switch (DeclarationType)
	{
	case SimpleType:
		generateTypeContent(context);
		break;

	case SimpleClass:
		generateClassContent(context);
		break;

	case BackRef:
		generateBackRefContent();
		break;

	case ForwardRef:
		generateForwardRefContent();
		break;

	case ArrayType:
		generateArrayTypeContent(context);
		break;

	case ArrayClass:
		generateArrayClassContent(context);
		break;

	case ArrayRef:
		generateArrayRefContent(context);
		break;

	case Set:
		generateSetContent(context);
		break;

	default:
		error("Can't decide declaration type");
		break;
	}

	if (context == NULL)
	{
		ClassNode->Gen.separator("methods");
	}
	
}



std::string		CDeclarationNode::getAccessorName(CCallContext *context, const std::string& accessortype, const std::string& sep)
{
	return context->getRootCaller()->Name + sep +accessortype + context->getCallString();
}


void	CDeclarationNode::generateTypeContent(CCallContext *context)
{
	CClassGenerator&	Gen = ClassNode->Gen;
	CCppOutput&			DbSummary = getDbNode()->DbSummary;

	CClassGenerator::SMethodId	&InitId = ClassNode->InitId;
	CClassGenerator::SMethodId	&ClearId = ClassNode->ClearId;
	CClassGenerator::SMethodId	&DestroyId = ClassNode->DestroyId;
	CClassGenerator::SMethodId	&FetchId = ClassNode->FetchId;
	CClassGenerator::SMethodId	&RegisterId = ClassNode->RegisterId;
	CClassGenerator::SMethodId	&RegisterAttributesId = ClassNode->RegisterAttributesId;
	CClassGenerator::SMethodId	&UnregisterId = ClassNode->UnregisterId;
	CClassGenerator::SMethodId	&UnregisterAttributesId = ClassNode->UnregisterAttributesId;
	CClassGenerator::SMethodId	&SetParentId = ClassNode->SetParentId;
	CClassGenerator::SMethodId	&SetUnnotifiedParentId = ClassNode->SetUnnotifiedParentId;

	string			UCodeContext;
	if (context != NULL)
		UCodeContext = context->getUserCodeContext();

	string			onGetUser = getUserCode("onGet", UCodeContext);
	string			onSetUser = getUserCode("onSet", UCodeContext);
	string			onChangeUser = getUserCode("onChange", UCodeContext);

	CTypeNode	*tnd = getTypeNode(Type);
	XmlNode += " type='type' typeid='"+toString(tnd->Id)+"'";

	setEnv("decltype", "type");
	setEnv("type", tnd->getName());
	define(IsKey, "iskey");
	setEnv("defaultvalue", tnd->getDefaultValue());
	setEnv("checkcode", tnd->checkCode(Name));

	CCallContext	ctx(this);
	if (context != NULL)
		ctx = context->getSubContext(this);
	CClassGenerator	&gen = ctx.getRootCaller()->Gen;

	// EGS Compat
	// -- begin

	CClassGenerator::SMethodId&	StoreId = ClassNode->StoreId;
	CClassGenerator::SMethodId&	ApplyId = ClassNode->ApplyId;


	ApplyId.add("else if (__pdr.peekNextToken() == "+tokenName()+")");
	ApplyId.add("{");
	tnd->generateApplyCode(ApplyId, tokenName(), cppName());
	ApplyId.add("}");

	tnd->generateStoreCode(StoreId, tokenName(), cppName());

	// -- end

	CTemplatizerEnv*	env = ctx.getRootDeclaration()->Env->nextArrayNode("accessors");

	env->set("name", Name);
	env->set("type", tnd->getName());
	env->set("defaultvalue", tnd->getDefaultValue());
	env->set("checkcode", tnd->checkCode(Name));
	env->define(ctx.getRootCaller()->HasRowAccess, "rowaccess");
	env->define(IsKey, "iskey");
	env->set("rootcallerid", ctx.getRootCaller()->getId());
	env->set("callstr", ctx.getCallString());
	env->set("callargs", ctx.getCallArgList());
	env->set("callpath", ctx.getCallPath());
	env->set("column", ctx.getColumn());
	env->set("valuevar", valueVariable);
	env->set("castcpp", tnd->castToCpp(valueVariable));
	env->set("castpds", tnd->castToPDS(valueVariable));

	vector<string>	checks = ctx.getCheckCode();
	for (uint i=0; i<checks.size(); ++i)
		env->nextArrayNode("checks")->set("check", checks[i]);
	env->nextArrayNode("checks")->set("check", tnd->checkCode(valueVariable));

	if (!onGetUser.empty())		env->set("onget", onGetUser);
	if (!onSetUser.empty())		env->set("onset", onSetUser);
	if (!onChangeUser.empty())	env->set("onchange", onChangeUser);

	//
	// generate read accessor
	//
	if (ctx.getRootCaller()->HasRowAccess)
	{
		string			arglist = ctx.getCallArgList();

		gen.startMethod(tnd->getName(), getFunctionPrefix+ctx.getCallString(), arglist, "methods", true, inlineAccessors);

		DbSummary << "\t" << getFunctionPrefix+ctx.getCallString() << "\n";

		uint	i;
		vector<string>	checks = ctx.getCheckCode();
		for (i=0; i<checks.size(); ++i)
		{
			gen.add(checks[i]);
		}

		if (!onGetUser.empty())
		{
			gen.add("{");
			gen.add(onGetUser);
			gen.add("}");
		}
		gen.add("return "+ctx.getCallPath()+";");
	}

	//
	// generate write accessor
	//
	if (ctx.getRootCaller()->HasRowAccess && !IsKey)
	{
		string	arglist = ctx.getCallArgList();

		if (!arglist.empty())
			arglist += ", ";
		arglist += tnd->getName()+" "+valueVariable;
		gen.startMethod("void", setFunctionPrefix+ctx.getCallString(), appendArg(arglist, "bool forceWrite=false"), "methods", false, inlineAccessors);

		DbSummary << "\t" << setFunctionPrefix << ctx.getCallString() << "\n";

		if (GenerateHAuto)
		{
			gen.add("H_AUTO("+getAccessorName(&ctx, setFunctionPrefix, "_")+")");
		}

		if (VerboseMode)
		{
			string	verbStr;
			string	callStr;

			verbStr = "nlinfo(\"" + ctx.getRootCaller()->Name + "(%d:%d)::" +setFunctionPrefix + ctx.getCallString() + "(";

			callStr = ctx.getDebugCallStringFmt();
			if (!callStr.empty())
				callStr += ", ";

			callStr += valueVariable+"="+tnd->getPrintfFmt();

			verbStr += callStr;
			verbStr += ")\", __BaseTable, __BaseRow, ";

			callStr = ctx.getDebugCallStringVal();
			if (!callStr.empty())
				callStr += ", ";

			callStr += tnd->getPrintfVal(valueVariable);

			verbStr += callStr;
			verbStr += ");";

			gen.add(verbStr);
		}

		uint	i;
		vector<string>	checks = ctx.getCheckCode();
		for (i=0; i<checks.size(); ++i)
		{
			gen.add(checks[i]);
		}
		gen.add(tnd->checkCode(valueVariable));

		///// TYPE CAST
		if (!onChangeUser.empty())
		{
			gen.add("if ("+ctx.getCallPath()+" != "+tnd->castToCpp(valueVariable)+")");
			gen.add("{");
			gen.add(onChangeUser);
			gen.add("}");
		}
		if (!onSetUser.empty())
		{
			gen.add("{");
			gen.add(onSetUser);
			gen.add("}");
		}
		if (WriteTriggerFlag)
		{
			gen.add("if (forceWrite && ("+ctx.getCallPath()+" != "+tnd->castToCpp(valueVariable)+"))");
		}
		else
		{
			gen.add("if (("+ctx.getCallPath()+" != "+tnd->castToCpp(valueVariable)+") || forceWrite)");
		}
		gen.add("{");
		bool		useEntityId = ctx.hasRootEntityIdKey();
		if (useEntityId)
		{
			gen.add(pdslibFunc("set")+"("+ctx.getRootCaller()->getId()+", __BaseRow, ("+TColumnIndexName+")("+ctx.getColumn()+"), "+tnd->castToPDS(valueVariable)+", "+ctx.getRootCaller()->getKey()->cppName()+");");
		}
		else
		{
			gen.add(pdslibFunc("set")+"("+ctx.getRootCaller()->getId()+", __BaseRow, ("+TColumnIndexName+")("+ctx.getColumn()+"), "+tnd->castToPDS(valueVariable)+");");
		}
		gen.add("}");
		gen.add(ctx.getCallPath()+" = "+tnd->castToCpp(valueVariable)+";");
	}

	//
	// generate attribute
	//
	Gen.addAttribute(tnd->getName(), cppName(), "attributes");


	string defaultValue;
	if (!DefaultValue.empty())
		defaultValue = DefaultValue;
	else
		defaultValue = tnd->getDefaultValue();

	//
	// generate init
	//
	if (!IsKey)
	{
		InitId.add(cppName()+" = "+defaultValue+";");
	}
	else
	{
		InitId.add(tnd->checkCode(Name));
		InitId.add(cppName()+" = "+Name+";");
	}

	//
	// generate create code
	//


	//
	// generate fetch code
	//
	if (tnd->isEnum())
		FetchId.add("data.serialEnum("+cppName()+");");
	else if (tnd->CppType != tnd->StorageType)
	{
		FetchId.add("{");
		FetchId.add(tnd->StorageType+"\t_v;");
		FetchId.add("data.serial(_v);");
		FetchId.add(cppName()+" = "+tnd->castToCpp("_v")+";");
		FetchId.add("}");
	}
	else
	{
		FetchId.add("data.serial("+cppName()+");");
	}

	//
	// generate clear code
	//
	if (ctx.getRootCaller()->HasRowAccess && !IsKey)
	{
		ctx.getRootCaller()->ClearId.add(ctx.getCallPath()+" = "+defaultValue+";");
		ctx.getRootCaller()->ClearId.add(pdslibFunc("set")+"("+ctx.getRootCaller()->getId()+", __BaseRow, ("+TColumnIndexName+")("+ctx.getColumn()+"), "+tnd->castToPDS(defaultValue)+");");
	}
}








void	CDeclarationNode::generateClassContent(CCallContext *context)
{
	setEnv("decltype", "class");

	CClassGenerator	&Gen = ClassNode->Gen;
	CCppOutput&	DbSummary = getDbNode()->DbSummary;

	CClassGenerator::SMethodId	&InitId = ClassNode->InitId;
	CClassGenerator::SMethodId	&ClearId = ClassNode->ClearId;
	CClassGenerator::SMethodId	&DestroyId = ClassNode->DestroyId;
	CClassGenerator::SMethodId	&FetchId = ClassNode->FetchId;
	CClassGenerator::SMethodId	&RegisterId = ClassNode->RegisterId;
	CClassGenerator::SMethodId	&RegisterAttributesId = ClassNode->RegisterAttributesId;
	CClassGenerator::SMethodId	&UnregisterId = ClassNode->UnregisterId;
	CClassGenerator::SMethodId	&UnregisterAttributesId = ClassNode->UnregisterAttributesId;
	CClassGenerator::SMethodId	&SetParentId = ClassNode->SetParentId;
	CClassGenerator::SMethodId	&SetUnnotifiedParentId = ClassNode->SetUnnotifiedParentId;

	CClassNode	*cnd = getClassNode(Type);
	XmlNode += " type='class' classid='"+toString(cnd->Id)+"'";

	setEnv("type", Type);

	// EGS Compat
	// -- begin

	CClassGenerator::SMethodId&	StoreId = ClassNode->StoreId;
	CClassGenerator::SMethodId&	ApplyId = ClassNode->ApplyId;

	StoreId.add("__pdr.pushStructBegin("+tokenName()+");");
	StoreId.add(cppName()+".store(__pdr);");
	StoreId.add("__pdr.pushStructEnd("+tokenName()+");");

	ApplyId.add("else if (__pdr.peekNextToken() == "+tokenName()+")");
	ApplyId.add("{");
	ApplyId.add("__pdr.popStructBegin("+tokenName()+");");
	ApplyId.add(cppName()+".apply(__pdr);");
	ApplyId.add("__pdr.popStructEnd("+tokenName()+");");
	ApplyId.add("}");

	// -- end


	//
	// export class accessors into root caller
	//
	CCallContext	ctx;
	if (context != NULL)
		ctx = *context;
	ctx.Context.push_back(this);
	cnd->generateContentInCall(&ctx);

	//
	// generate attribute
	//
	Gen.addAttribute(Type, cppName(), "attributes");

	//
	// generate init
	//
	InitId.add(cppName()+"."+initFunction+"();");

	//
	// generate create code
	//

	//
	// generate fetch code
	//
	FetchId.add(cppName()+"."+fetchFunction+"(data);");

	//
	// generate clear code
	//
	//ClearId.add(cppName()+"."+clearFunction+"();");
}






void	CDeclarationNode::generateBackRefContent()
{
	setEnv("decltype", "backref");
	define(HiddenFlag, "hidden");

	CClassGenerator	&Gen = ClassNode->Gen;
	CCppOutput&	DbSummary = getDbNode()->DbSummary;

	CClassGenerator::SMethodId	&InitId = ClassNode->InitId;
	CClassGenerator::SMethodId	&ClearId = ClassNode->ClearId;
	CClassGenerator::SMethodId	&DestroyId = ClassNode->DestroyId;
	CClassGenerator::SMethodId	&FetchId = ClassNode->FetchId;
	CClassGenerator::SMethodId	&RegisterId = ClassNode->RegisterId;
	CClassGenerator::SMethodId	&RegisterAttributesId = ClassNode->RegisterAttributesId;
	CClassGenerator::SMethodId	&UnregisterId = ClassNode->UnregisterId;
	CClassGenerator::SMethodId	&UnregisterAttributesId = ClassNode->UnregisterAttributesId;
	CClassGenerator::SMethodId	&SetParentId = ClassNode->SetParentId;
	CClassGenerator::SMethodId	&SetUnnotifiedParentId = ClassNode->SetUnnotifiedParentId;

	CClassNode			*cnd = getClassNode(ParentClass);
	CDeclarationNode	*dnd = cnd->getDeclarationNode(ParentField);
	CDeclarationNode	*knd = (ClassNode->ClassKey.empty() ? NULL : ClassNode->getKey());
	XmlNode += " type='backref' classid='"+toString(cnd->Id)+"' backreferentid='"+toString(dnd->Id)+"'";
	if (knd != NULL)
		XmlNode += " key='"+toString(knd->Id)+"'";

	setEnv("type", ParentClass);

	//
	// generate read accessor
	//
	Gen.startMethod(ParentClass+"*", getFunc(), "", "methods", false, inlineAccessors);
	Gen.add("return "+cppName()+";");

	Gen.startMethod("const "+ParentClass+"*", getFunc(), "", "methods", true, inlineAccessors);
	Gen.add("return "+cppName()+";");

	DbSummary << "\t" << getFunc() << "\n";

	//
	// generate write accessor
	//

	//
	// generate attribute
	//
	Gen.addAttribute(ParentClass+"*", cppName(), "attributes");

	//
	// generate init
	//
	InitId.add(cppName()+" = NULL;");

	//
	// generate create code
	//

	//
	// generate fetch code
	//
	FetchId.add(cppName()+" = NULL;");

	//
	// generate set parent code
	//

	bool	useId = ClassNode->useEntityId();
	bool	parentUseId = cnd->useEntityId();

	if (parentUseId)
		SetParentId.add("NLMISC::CEntityId\tprevId;");

	if (!HiddenFlag)
	{
		//SetParentId.add(pdslibFunc("set")+"("+ClassNode->getId()+", getRow(), ("+TColumnIndexName+")("+toString(Column)+"), (__parent != NULL ? "+objectIndexName+"("+(cnd->HasInheritance ? toString("__parent->getTable()") : toString(cnd->Id))+", __parent->getRow()) : "+nullIndexName+"));");

		SetParentId.add("if ("+cppName()+" != NULL)");
		SetParentId.add("{");

		if (parentUseId)
			SetParentId.add("prevId = "+cppName()+"->"+getFunctionPrefix+cnd->getKey()->Name+"();");

		if (ClassNode->getClassKey() == NULL)
		{
			SetParentId.add(cppName()+"->"+dnd->unlinkFunc()+"();");
		}
		else
		{
			SetParentId.add(cppName()+"->"+dnd->unlinkFunc()+"("+ClassNode->getKey()->cppName()+");");
		}
		SetParentId.add("}");
	}
	else
	{
		if (parentUseId)
		{
			SetParentId.add("if ("+cppName()+" != NULL)");
			SetParentId.add("{");
			SetParentId.add("prevId = "+cppName()+"->"+cnd->getKey()->cppName()+";");
			SetParentId.add("}");
		}
	}

	string	oeid;
	string	peid;

	if (useId)
		oeid = ", "+ClassNode->getKey()->cppName();

	if (parentUseId)
		peid = ", ("+cppName()+" != NULL ? "+cppName()+"->"+getFunctionPrefix+cnd->getKey()->Name+"() : NLMISC::CEntityId::Unknown), prevId";

	SetParentId.add(cppName()+" = __parent;");
	SetParentId.add(pdslibFunc("setParent")+"("+ClassNode->getId()+", getRow(), ("+TColumnIndexName+")("+toString(Column)+"), (__parent != NULL ? "+objectIndexName+"("+(cnd->HasInheritance ? toString("__parent->getTable()") : toString(cnd->Id))+", __parent->getRow()) : "+nullIndexName+")"+oeid+peid+");");

	SetUnnotifiedParentId.add(cppName()+" = __parent;");

}






void	CDeclarationNode::generateClassPtrApplyCode(const std::string& value)
{
	CClassGenerator::SMethodId&	ApplyId = ClassNode->ApplyId;

	ApplyId.add(value+" = NULL;");
	ApplyId.add("if (__pdr.peekNextToken() == __Tok_ClassName)");
	ApplyId.add("{");
	ApplyId.add(	"std::string\t__className;");
	ApplyId.add(	"__pdr.pop(__Tok_ClassName, __className);");
	ApplyId.add(	value+" = "+Type+"::cast("+pdslibFunc("create")+"(__className));");
	ApplyId.add(	"if ("+value+" != NULL)");
	ApplyId.add(	"{");
	ApplyId.add(		"__pdr.popStructBegin("+tokenName()+");");
	ApplyId.add(		value+"->apply(__pdr);");
	ApplyId.add(		value+"->"+setUnnotifiedParentFunction+"(this);");
	ApplyId.add(		"__pdr.popStructEnd("+tokenName()+");");
	ApplyId.add(	"}");
	ApplyId.add(	"else");
	ApplyId.add(	"{");
	ApplyId.add(		"__pdr.skipStruct();");
	ApplyId.add(	"}");
	ApplyId.add("}");
}

void	CDeclarationNode::generateClassPtrStoreCode(const std::string& value)
{
	CClassGenerator::SMethodId&	StoreId = ClassNode->StoreId;

	StoreId.add("if ("+value+" != NULL)");
	StoreId.add("{");
	StoreId.add(	"std::string\t__className = "+pdslibFunc("getClassName")+"("+value+");");
	StoreId.add(	"__pdr.push(__Tok_ClassName, __className);");
	StoreId.add(	"__pdr.pushStructBegin("+tokenName()+");");
	StoreId.add(	value+"->store(__pdr);");
	StoreId.add(	"__pdr.pushStructEnd("+tokenName()+");");
	StoreId.add("}");
}



void	CDeclarationNode::generateForwardRefContent()
{
	setEnv("decltype", "forwardref");

	CClassGenerator	&Gen = ClassNode->Gen;
	CCppOutput&	DbSummary = getDbNode()->DbSummary;

	CClassGenerator::SMethodId	&InitId = ClassNode->InitId;
	CClassGenerator::SMethodId	&ClearId = ClassNode->ClearId;
	CClassGenerator::SMethodId	&DestroyId = ClassNode->DestroyId;
	CClassGenerator::SMethodId	&FetchId = ClassNode->FetchId;
	CClassGenerator::SMethodId	&RegisterId = ClassNode->RegisterId;
	CClassGenerator::SMethodId	&RegisterAttributesId = ClassNode->RegisterAttributesId;
	CClassGenerator::SMethodId	&UnregisterId = ClassNode->UnregisterId;
	CClassGenerator::SMethodId	&UnregisterAttributesId = ClassNode->UnregisterAttributesId;
	CClassGenerator::SMethodId	&SetParentId = ClassNode->SetParentId;
	CClassGenerator::SMethodId	&NotifyInitId = ClassNode->NotifyInitId;
	CClassGenerator::SMethodId	&NotifyReleaseId = ClassNode->NotifyReleaseId;

	CClassNode			*cnd = getClassNode(Type);
	CDeclarationNode	*dnd = cnd->getDeclarationNode(ForwardRefAttribute);
	CDeclarationNode	*knd = (cnd->ClassKey.empty() ? NULL : cnd->getKey());
	XmlNode += " type='forwardref' classid='"+toString(cnd->Id)+"' forwardreferedid='"+toString(dnd->Id)+"'";
	if (knd != NULL)
		XmlNode += " key='"+toString(knd->Id)+"'";

	setEnv("type", Type);

	// EGS Compat
	// -- begin

	CClassGenerator::SMethodId&	StoreId = ClassNode->StoreId;
	CClassGenerator::SMethodId&	ApplyId = ClassNode->ApplyId;

	StoreId.add("// store "+Name);
	StoreId.add("__pdr.pushStructBegin("+tokenName()+");");
	generateClassPtrStoreCode(cppName());
	StoreId.add("__pdr.pushStructEnd("+tokenName()+");");

	ApplyId.add("// apply "+Name);
	ApplyId.add("else if (__pdr.peekNextToken() == "+tokenName()+")");
	ApplyId.add("{");
	ApplyId.add(	"__pdr.popStructBegin("+tokenName()+");");
	generateClassPtrApplyCode(cppName());
	ApplyId.add(	"__pdr.popStructEnd("+tokenName()+");");
	ApplyId.add("}");

	// -- end


	//
	// generate read accessor
	//
	Gen.startMethod(Type+"*", getFunc(), "", "methods", false, inlineAccessors);
	Gen.add("return "+cppName()+";");

	Gen.startMethod("const "+Type+"*", getFunc(), "", "methods", true, inlineAccessors);
	Gen.add("return "+cppName()+";");

	DbSummary << "\t" << getFunc() << "\n";

	//
	// generate write accessor
	//
	Gen.startMethod("void", setFunc(), Type+"* "+valueVariable, "methods", false, inlineAccessors);
	Gen.add("if ("+cppName()+" != NULL)");
	Gen.add("{");
	Gen.add(cppName()+"->"+setParentFunction+"(NULL);");
	Gen.add("}");
	Gen.add(valueVariable+"->"+setParentFunction+"(this);");
	Gen.add(cppName()+" = "+valueVariable+";");

	DbSummary << "\t" << setFunc() << "\n";

	//
	// generate attribute
	//
	Gen.addAttribute(Type+"*", cppName(), "attributes");

	//
	// generate init
	//
	InitId.add(cppName()+" = NULL;");

	//
	// generate destroy code
	//
	DestroyId.add("if ("+cppName()+" != NULL)");
	DestroyId.add("{");
	DestroyId.add(Type+"*\t__o = "+cppName()+";");
	DestroyId.add("__o->"+destroyFunction+"();");
	DestroyId.add("delete __o;");
	DestroyId.add("}");

	//
	// generate create code
	//

	//
	// generate fetch code
	//
	FetchId.add("// read table and row, create an object, affect to the ref, and fetch it");
	if (!ClassNode->tableAndRowIndicesUsedInFetch)
	{
		FetchId.add(TTableIndexName+"\ttableIndex;\n"+TRowIndexName+"\trowIndex;");
		ClassNode->tableAndRowIndicesUsedInFetch = true;
	}
	FetchId.add(cppName()+" = NULL;");
	FetchId.add("data.serial(tableIndex, rowIndex);");
	FetchId.add("if (rowIndex != "+INVALID_ROW_INDEXName+" && tableIndex != "+INVALID_TABLE_INDEXName+")");
	FetchId.add("{");
	FetchId.add(cppName()+" = static_cast<"+Type+"*>("+pdslibFunc("create")+"(tableIndex));");
	FetchId.add(pdslibFunc("setRowIndex")+"(rowIndex, "+cppName()+");");
	FetchId.add(cppName()+"->"+fetchFunction+"(data);");
	FetchId.add(cppName()+"->"+setUnnotifiedParentFunction+"(this);");
	FetchId.add("}");

	//
	// generate register/unregister code
	//

	UnregisterAttributesId.add("if ("+cppName()+" != NULL)");
	UnregisterAttributesId.add("{");
	UnregisterAttributesId.add(Type+"*\t"+objectVariable+" = "+cppName()+";");
	UnregisterAttributesId.add(objectVariable+"->"+unregisterFunction+"();");
	UnregisterAttributesId.add(objectVariable+"->"+destroyFunction+"();");
	UnregisterAttributesId.add("delete "+objectVariable+";");
	UnregisterAttributesId.add("}");

	//
	// generate init/release notification
	//

	NotifyInitId.add("if ("+cppName()+" != NULL)");
	NotifyInitId.add("{");
	NotifyInitId.add(cppName()+"->"+notifyInitFunction+"();");
	NotifyInitId.add("}");

	NotifyReleaseId.add("if ("+cppName()+" != NULL)");
	NotifyReleaseId.add("{");
	NotifyReleaseId.add(cppName()+"->"+notifyReleaseFunction+"();");
	NotifyReleaseId.add("}");

	//
	// generate unlink code
	//
	string	unlinkProto;
	if (cnd->getClassKey() != NULL)
	{
		CDeclarationNode*	kd = cnd->getClassKey();
		CTypeNode*			keyType = getTypeNode(kd->Type);
		unlinkProto = keyType->getName()+" dummy";
	}
	Gen.startMethod("void", unlinkFunc(), unlinkProto, "internal", false, inlineInternal);
	Gen.add("{");
	Gen.add(cppName()+" = NULL;");
	Gen.add("}");

	//
	// generate clear code
	//
	//ClearId.add(cppName()+"->"+setParentFunction+"(NULL);");
	ClearId.add(Type+"*\t"+objectVariable+" = "+cppName()+";");
	ClearId.add(objectVariable+"->"+unregisterFunction+"();");
	ClearId.add(objectVariable+"->"+destroyFunction+"();");
	ClearId.add("delete "+objectVariable+";");
}



void	CDeclarationNode::generateArrayApplyCode()
{
	CClassGenerator::SMethodId&	ApplyId = ClassNode->ApplyId;

	ApplyId.add("// apply "+Name);
	ApplyId.add("else if (__pdr.peekNextToken() == "+tokenName()+")");
	ApplyId.add("{");
	ApplyId.add(	"__pdr.popStructBegin("+tokenName()+");");
	ApplyId.add(	"uint\tindex = 0;");
	ApplyId.add(	"while (!__pdr.isEndOfStruct())");
	ApplyId.add(	"{");

	CIndexNode	*ind = getIndexNode(ArrayIndex);
	if (ind->isEnum())
	{
		ApplyId.add("std::string\tindexname;");
		ApplyId.add("__pdr.pop(__Tok_MapKey, indexname);");
		ApplyId.add("index = "+ind->getFromStringCode("indexname")+";");
	}
}

void	CDeclarationNode::generateArrayStoreCode()
{
	CClassGenerator::SMethodId&	StoreId = ClassNode->StoreId;

	StoreId.add("// store "+Name);
	StoreId.add("__pdr.pushStructBegin("+tokenName()+");");
	CIndexNode	*ind = getIndexNode(ArrayIndex);

	StoreId.add("for (uint index=0; index<"+ind->getSizeName()+"; ++index)");
	StoreId.add("{");

	if (ind->isEnum())
	{
		StoreId.add("std::string\tindexname = "+ind->getToStringCode(ind->castFromUser("index"))+";");
		StoreId.add("__pdr.push(__Tok_MapKey, indexname);");
	}
}


void	CDeclarationNode::generateArrayEndCode()
{
	CClassGenerator::SMethodId&	StoreId = ClassNode->StoreId;
	CClassGenerator::SMethodId&	ApplyId = ClassNode->ApplyId;

	ApplyId.add(		"++index;");
	ApplyId.add(	"}");
	ApplyId.add(	"__pdr.popStructEnd("+tokenName()+");");
	ApplyId.add("}");

	StoreId.add("}");
	StoreId.add("__pdr.pushStructEnd("+tokenName()+");");
}



void	CDeclarationNode::generateArrayTypeContent(CCallContext *context)
{
	setEnv("decltype", "arraytype");

	CClassGenerator	&Gen = ClassNode->Gen;
	CCppOutput&	DbSummary = getDbNode()->DbSummary;

	CClassGenerator::SMethodId	&InitId = ClassNode->InitId;
	CClassGenerator::SMethodId	&ClearId = ClassNode->ClearId;
	CClassGenerator::SMethodId	&DestroyId = ClassNode->DestroyId;
	CClassGenerator::SMethodId	&FetchId = ClassNode->FetchId;
	CClassGenerator::SMethodId	&RegisterId = ClassNode->RegisterId;
	CClassGenerator::SMethodId	&RegisterAttributesId = ClassNode->RegisterAttributesId;
	CClassGenerator::SMethodId	&UnregisterId = ClassNode->UnregisterId;
	CClassGenerator::SMethodId	&UnregisterAttributesId = ClassNode->UnregisterAttributesId;
	CClassGenerator::SMethodId	&SetParentId = ClassNode->SetParentId;
	CClassGenerator::SMethodId	&SetUnnotifiedParentId = ClassNode->SetUnnotifiedParentId;

	string			UCodeContext;
	if (context != NULL)
		UCodeContext = context->getUserCodeContext();

	string			onGetUser = getUserCode("onGet", UCodeContext);
	string			onSetUser = getUserCode("onSet", UCodeContext);
	string			onChangeUser = getUserCode("onChange", UCodeContext);

	CTypeNode	*tnd = getTypeNode(Type);
	CIndexNode	*ind = getIndexNode(ArrayIndex);
	XmlNode += " type='arraytype' typeid='"+toString(tnd->Id)+"' indexid='"+toString(ind->Id)+"'";

	CCallContext	ctx(this);
	if (context != NULL)
		ctx = context->getSubContext(this);
	CClassGenerator	&gen = ctx.getRootCaller()->Gen;

	setEnv("type", tnd->getName());
	setEnv("indexsize", ind->getSizeName());

	// EGS Compat
	// -- begin

	CClassGenerator::SMethodId&	StoreId = ClassNode->StoreId;
	CClassGenerator::SMethodId&	ApplyId = ClassNode->ApplyId;

	generateArrayApplyCode();
	generateArrayStoreCode();

	ApplyId.add(tnd->getName()+"\tvalue;");
	tnd->generateApplyCode(ApplyId, "__Tok_MapVal", "value");
	ApplyId.add("if (index != "+toString(ind->getSize())+")");
	ApplyId.add("{");
	ApplyId.add(	cppName()+"[index] = value;");
	ApplyId.add("}");

	tnd->generateStoreCode(ApplyId, "__Tok_MapVal", cppName()+"[index]");

	generateArrayEndCode();

	// -- end


	//
	// generate read accessor
	//
	if (ctx.getRootCaller()->HasRowAccess)
	{
		string	arglist = ctx.getCallArgList();
		ctx.getRootCaller()->Gen.startMethod(tnd->getName(), getFunctionPrefix+ctx.getCallString(), arglist, "methods", true, inlineAccessors);

		DbSummary << "\t" << getFunctionPrefix+ctx.getCallString() << "\n";

		uint	i;
		vector<string>	checks = ctx.getCheckCode();
		for (i=0; i<checks.size(); ++i)
			gen.add(checks[i]);

		if (!onGetUser.empty())
		{
			gen.add("{");
			gen.add(onGetUser);
			gen.add("}");
		}
		gen.add("return "+ctx.getCallPath()+";");
	}

	//
	// generate write accessor
	//
	if (ctx.getRootCaller()->HasRowAccess)
	{
		string	arglist = ctx.getCallArgList();
		if (!arglist.empty())
			arglist += ", ";
		arglist += tnd->getName()+" "+valueVariable;
		gen.startMethod("void", setFunctionPrefix+ctx.getCallString(), appendArg(arglist, "bool forceWrite=false"), "methods", false, inlineAccessors);

		DbSummary << "\t" << setFunctionPrefix+ctx.getCallString() << "\n";

		if (GenerateHAuto)
		{
			gen.add("H_AUTO("+getAccessorName(&ctx, getFunctionPrefix, "_")+")");
		}

		if (VerboseMode)
		{
			string	verbStr;
			string	callStr;

			verbStr = "nlinfo(\"" + ctx.getRootCaller()->Name + "(%d:%d)::" +setFunctionPrefix + ctx.getCallString() + "(";

			callStr = ctx.getDebugCallStringFmt();
			if (!callStr.empty())
				callStr += ", ";

			callStr += valueVariable+"="+tnd->getPrintfFmt();

			verbStr += callStr;
			verbStr += ")\", __BaseTable, __BaseRow, ";

			callStr = ctx.getDebugCallStringVal();
			if (!callStr.empty())
				callStr += ", ";

			callStr += tnd->getPrintfVal(valueVariable);

			verbStr += callStr;
			verbStr += ");";

			gen.add(verbStr);
		}

		uint	i;
		vector<string>	checks = ctx.getCheckCode();
		for (i=0; i<checks.size(); ++i)
			gen.add(checks[i]);
		gen.add(tnd->checkCode(valueVariable));

		if (!onChangeUser.empty())
		{
			gen.add("if ("+ctx.getCallPath()+" != "+tnd->castToCpp(valueVariable)+")");
			gen.add("{");
			gen.add(onChangeUser);
			gen.add("}");
		}
		if (!onSetUser.empty())
		{
			gen.add("{");
			gen.add(onGetUser);
			gen.add("}");
		}
		if (WriteTriggerFlag)
		{
			gen.add("if (forceWrite && ("+ctx.getCallPath()+" != "+tnd->castToCpp(valueVariable)+"))");
		}
		else
		{
			gen.add("if (("+ctx.getCallPath()+" != "+tnd->castToCpp(valueVariable)+") || forceWrite)");
		}
		gen.add("{");
		bool		useEntityId = ctx.hasRootEntityIdKey();
		if (useEntityId)
		{
			gen.add(pdslibFunc("set")+"("+ctx.getRootCaller()->getId()+", __BaseRow, ("+TColumnIndexName+")("+ctx.getColumn()+"), "+tnd->castToPDS(valueVariable)+", "+ctx.getRootCaller()->getKey()->cppName()+");");
		}
		else
		{
			gen.add(pdslibFunc("set")+"("+ctx.getRootCaller()->getId()+", __BaseRow, ("+TColumnIndexName+")("+ctx.getColumn()+"), "+tnd->castToPDS(valueVariable)+");");
		}
		//gen.add(pdslibFunc("set")+"("+ctx.getRootCaller()->getId()+", __BaseRow, ("+TColumnIndexName+")("+ctx.getColumn()+"), "+tnd->castToPDS(valueVariable)+");");
		gen.add("}");
		gen.add(ctx.getCallPath()+" = "+tnd->castToCpp(valueVariable)+";");
	}

	//
	// generate attribute
	//
	Gen.addAttribute(tnd->getName(), cppName()+"["+ind->getSizeName()+"]", "attributes");

	//
	// generate init
	//
	if (!ClassNode->indexUsedInInit)
	{
		InitId.add("uint\ti;");
		ClassNode->indexUsedInInit = true;
	}

	InitId.add("for (i=0; i<"+ind->getSizeName()+"; ++i)\t"+cppName()+"[i] = "+tnd->getDefaultValue()+";");

	//
	// generate create code
	//

	//
	// generate fetch code
	//
	if (!ClassNode->indexUsedInFetch)
	{
		FetchId.add("uint\ti;");
		ClassNode->indexUsedInFetch = true;
	}

	if (tnd->isEnum())
		FetchId.add("for (i=0; i<"+ind->getSizeName()+"; ++i)\tdata.serialEnum("+cppName()+"[i]);");
	else if (tnd->CppType != tnd->StorageType)
	{
		FetchId.add("for (i=0; i<"+ind->getSizeName()+"; ++i)");
		FetchId.add("{");
		FetchId.add(tnd->StorageType+"\t_v;");
		FetchId.add("data.serial(_v);");
		FetchId.add(cppName()+"[i] = "+tnd->castToCpp("_v")+";");
		FetchId.add("}");
	}
	else
	{
		FetchId.add("for (i=0; i<"+ind->getSizeName()+"; ++i)\tdata.serial("+cppName()+"[i]);");
	}

	//
	// generate clear code
	//
	if (ctx.getRootCaller()->HasRowAccess)
	{
		string	forIndex = "__i"+toString(ctx.getContextIndex());
		ctx.getRootCaller()->ClearId.add("for (uint "+forIndex+"=0; "+forIndex+"<"+ind->getSizeName()+"; ++"+forIndex+")");
		ctx.getRootCaller()->ClearId.add("{");
		ctx.getRootCaller()->ClearId.add(ctx.getCallPath()+" = "+tnd->getDefaultValue()+";");
		ctx.getRootCaller()->ClearId.add(pdslibFunc("set")+"("+ctx.getRootCaller()->getId()+", __BaseRow, ("+TColumnIndexName+")("+ctx.getColumn()+"), "+tnd->castToPDS(tnd->getDefaultValue())+");");
		ctx.getRootCaller()->ClearId.add("}");
	}
}






void	CDeclarationNode::generateArrayClassContent(CCallContext *context)
{
	setEnv("decltype", "arrayclass");

	CClassGenerator	&Gen = ClassNode->Gen;
	CCppOutput&	DbSummary = getDbNode()->DbSummary;

	CClassGenerator::SMethodId	&InitId = ClassNode->InitId;
	CClassGenerator::SMethodId	&ClearId = ClassNode->ClearId;
	CClassGenerator::SMethodId	&DestroyId = ClassNode->DestroyId;
	CClassGenerator::SMethodId	&FetchId = ClassNode->FetchId;
	CClassGenerator::SMethodId	&RegisterId = ClassNode->RegisterId;
	CClassGenerator::SMethodId	&RegisterAttributesId = ClassNode->RegisterAttributesId;
	CClassGenerator::SMethodId	&UnregisterId = ClassNode->UnregisterId;
	CClassGenerator::SMethodId	&UnregisterAttributesId = ClassNode->UnregisterAttributesId;
	CClassGenerator::SMethodId	&SetParentId = ClassNode->SetParentId;
	CClassGenerator::SMethodId	&SetUnnotifiedParentId = ClassNode->SetUnnotifiedParentId;

	CClassNode	*sub = getClassNode(Type);
	CIndexNode	*ind = getIndexNode(ArrayIndex);
	XmlNode += " type='arrayclass' classid='"+toString(sub->Id)+"' indexid='"+toString(ind->Id)+"'";

	setEnv("type", Type);
	setEnv("indexsize", ind->getSizeName());

	// EGS Compat
	// -- begin

	CClassGenerator::SMethodId&	StoreId = ClassNode->StoreId;
	CClassGenerator::SMethodId&	ApplyId = ClassNode->ApplyId;

	generateArrayApplyCode();
	generateArrayStoreCode();

//	StoreId.add("__pdr.pushStructBegin("+tokenName()+");");
	StoreId.add("__pdr.pushStructBegin(__Tok_MapVal);");
	StoreId.add(cppName()+"[index].store(__pdr);");
	StoreId.add("__pdr.pushStructEnd(__Tok_MapVal);");
//	StoreId.add("__pdr.pushStructEnd("+tokenName()+");");

	ApplyId.add("if (index < "+ind->getSizeName()+")");
	ApplyId.add("{");
//	ApplyId.add("__pdr.popStructBegin("+tokenName()+");");
	ApplyId.add("__pdr.popStructBegin(__pdr.peekNextToken());");
	ApplyId.add(cppName()+"[index].apply(__pdr);");
	ApplyId.add("__pdr.popStructEnd(__pdr.peekNextToken());");
//	ApplyId.add("__pdr.popStructEnd("+tokenName()+");");
	ApplyId.add("}");
	ApplyId.add("else");
	ApplyId.add("{");
	ApplyId.add(	"__pdr.skipStruct();");
	ApplyId.add("}");

	generateArrayEndCode();

	// -- end


	//
	// export class accessors into root caller
	//

	CCallContext	ctx;
	if (context != NULL)
		ctx = *context;
	ctx.Context.push_back(this);

	if (ctx.getRootCaller()->HasRowAccess)
	{
		string	forIndex = "__i"+toString(ctx.getContextIndex());
		ctx.getRootCaller()->ClearId.add("for (uint "+forIndex+"=0; "+forIndex+"<"+ind->getSizeName()+"; ++"+forIndex+")");
		ctx.getRootCaller()->ClearId.add("{");
	}
	
	sub->generateContentInCall(&ctx);

	if (ctx.getRootCaller()->HasRowAccess)
	{
		ctx.getRootCaller()->ClearId.add("}");
	}

	//
	// generate attribute
	//
	Gen.addAttribute(Type, cppName()+"["+ind->getSizeName()+"]", "attributes");

	//
	// generate init
	//

	//
	// generate create code
	//

	//
	// generate fetch code
	//
	if (!ClassNode->indexUsedInFetch)
	{
		FetchId.add("uint\ti;");
		ClassNode->indexUsedInFetch = true;
	}
	FetchId.add("for (i=0; i<"+ind->getSizeName()+"; ++i)\t"+cppName()+"[i]."+fetchFunction+"(data);");

	//
	// generate clear code
	//
}






void	CDeclarationNode::generateArrayRefContent(CCallContext *context)
{
	setEnv("decltype", "arrayref");

	CClassGenerator	&Gen = ClassNode->Gen;
	CCppOutput&	DbSummary = getDbNode()->DbSummary;

	CClassGenerator::SMethodId	&InitId = ClassNode->InitId;
	CClassGenerator::SMethodId	&ClearId = ClassNode->ClearId;
	CClassGenerator::SMethodId	&DestroyId = ClassNode->DestroyId;
	CClassGenerator::SMethodId	&FetchId = ClassNode->FetchId;
	CClassGenerator::SMethodId	&RegisterId = ClassNode->RegisterId;
	CClassGenerator::SMethodId	&RegisterAttributesId = ClassNode->RegisterAttributesId;
	CClassGenerator::SMethodId	&UnregisterId = ClassNode->UnregisterId;
	CClassGenerator::SMethodId	&UnregisterAttributesId = ClassNode->UnregisterAttributesId;
	CClassGenerator::SMethodId	&SetParentId = ClassNode->SetParentId;
	CClassGenerator::SMethodId	&SetUnnotifiedParentId = ClassNode->SetUnnotifiedParentId;
	CClassGenerator::SMethodId	&NotifyInitId = ClassNode->NotifyInitId;
	CClassGenerator::SMethodId	&NotifyReleaseId = ClassNode->NotifyReleaseId;

	CClassNode	*cnd = getClassNode(Type);

	bool		useReference = cnd->ForceReference;

	CDeclarationNode	*dnd = cnd->getDeclarationNode(ForwardRefAttribute);
	CIndexNode			*ind = getIndexNode(ArrayIndex);
	CDeclarationNode	*knd = cnd->getKey();
	XmlNode += " type='arrayref' classid='"+toString(cnd->Id)+"' forwardreferedid='"+toString(dnd->Id)+"' key='"+toString(knd->Id)+"' indexid='"+toString(ind->Id)+"' allownull='"+(useReference ? "true" : "false")+"'";

	string		arrayType = Type+(useReference ? "*" : "");
	string		access = (useReference ? "->" : ".");

	setEnv("type", Type);
	setEnv("indexsize", ind->getSizeName());
	define(useReference, "useref");

	string			UCodeContext;
	if (context != NULL)
		UCodeContext = context->getUserCodeContext();

	string			onChangeUser = getUserCode("onChange", UCodeContext);

	// EGS Compat
	// -- begin

	CClassGenerator::SMethodId&	StoreId = ClassNode->StoreId;
	CClassGenerator::SMethodId&	ApplyId = ClassNode->ApplyId;

	generateArrayStoreCode();
	generateArrayApplyCode();

	StoreId.add("__pdr.pushStructBegin("+tokenName()+");");

	ApplyId.add("if (index < "+ind->getSizeName()+")");
	ApplyId.add("{");
	ApplyId.add(	"if (__pdr.peekNextToken() == "+tokenName()+")");
	ApplyId.add(	"{");
	ApplyId.add(		"__pdr.popStructBegin("+tokenName()+");");

	if (useReference)
	{
		generateClassPtrStoreCode(cppName()+"[index]");

		generateClassPtrApplyCode(cppName()+"[index]");
	}
	else
	{
		StoreId.add(cppName()+"[index].store(__pdr);");

		ApplyId.add(cppName()+"[index].apply(__pdr);");
		ApplyId.add(cppName()+"[index]."+setUnnotifiedParentFunction+"(this);");
	}

	StoreId.add("__pdr.pushStructEnd("+tokenName()+");");

	ApplyId.add(		"__pdr.popStructEnd("+tokenName()+");");
	ApplyId.add(	"}");
	ApplyId.add("}");
	ApplyId.add("else");
	ApplyId.add("{");
	ApplyId.add(	"__pdr.skipStruct();");
	ApplyId.add("}");

	generateArrayEndCode();

	// -- end


	//
	// generate read accessor
	//
	Gen.startMethod((useReference ? arrayType : Type+"&"), getFunc(), ind->getName()+" "+indexVariable, "methods", false, inlineAccessors);
	Gen.add(ind->checkCode(indexVariable));
	Gen.add("return "+cppName()+"["+indexVariable+"];");

	Gen.startMethod(string("const ")+(useReference ? arrayType : Type+"&"), getFunc(), ind->getName()+" "+indexVariable, "methods", true, inlineAccessors);
	Gen.add(ind->checkCode(indexVariable));
	Gen.add("return "+cppName()+"["+indexVariable+"];");

	DbSummary << "\t" << getFunc() << "\n";

	//
	// generate write accessor
	//
	if (useReference)
	{
		Gen.startMethod("void", setFunc(), arrayType+" "+valueVariable, "methods", false, inlineAccessors);

		DbSummary << "\t" << setFunc() << "\n";

		Gen.add("if ("+valueVariable+" == NULL)\treturn;");
		Gen.add(ind->getName()+"\t"+keyVariable+" = "+valueVariable+"->"+cnd->getKey()->getFunc()+"();");
		Gen.add(ind->checkCode(keyVariable));
		Gen.add(Type+"*\t__prev = "+cppName()+"["+keyVariable+"];");
		Gen.add("if (__prev != NULL)");
		Gen.add("{");
		Gen.add("__prev->"+setParentFunction+"(NULL);");
		if (cnd->MapClass == NULL)
		{
			Gen.add("__prev->"+unregisterFunction+"();");
			Gen.add("__prev->"+destroyFunction+"();");
			Gen.add("delete __prev;");
		}
		Gen.add("}");
		Gen.add(valueVariable+"->"+setParentFunction+"(this);");
		Gen.add(cppName()+"["+keyVariable+"] = "+valueVariable+";");
	}

	if (cnd->MapClass == NULL && useReference && !cnd->HasInheritance && !cnd->DerivatedFlag)
	{
		Gen.startMethod(Type+"*", newFunc(), ind->getName()+" "+indexVariable, "methods", false, inlineAccessors);
		DbSummary << "\t" << newFunc() << "\n";
		Gen.add(ind->checkCode(indexVariable));
		Gen.add(Type+"*\t"+objectVariable+" = new "+Type+"();");
		Gen.add(objectVariable+"->"+initFunction+"("+indexVariable+");");
		Gen.add(objectVariable+"->"+registerFunction+"();");
		Gen.add(setFunc()+"("+objectVariable+");");
	}

	if (cnd->MapClass == NULL && useReference)
	{
		Gen.startMethod("void", deleteFunc(), ind->getName()+" "+indexVariable, "methods", false, inlineAccessors);
		DbSummary << "\t" << deleteFunc() << "\n";
		Gen.add(ind->checkCode(indexVariable));
		Gen.add(Type+"*\t"+objectVariable+" = "+cppName()+"["+indexVariable+"];");
		Gen.add(objectVariable+"->"+setParentFunction+"(NULL);");
		Gen.add(objectVariable+"->"+unregisterFunction+"();");
		Gen.add(objectVariable+"->"+destroyFunction+"();");
		Gen.add("delete "+objectVariable+";");
	}

	//
	// generate attribute
	//
	Gen.addAttribute(arrayType, cppName()+"["+ind->getSizeName()+"]", "attributes");

	//
	// generate init
	//
	if (!ClassNode->indexUsedInInit)
	{
		InitId.add("uint\ti;");
		ClassNode->indexUsedInInit = true;
	}
	if (useReference)
	{
		InitId.add("for (i=0; i<"+ind->getSizeName()+"; ++i)\t"+cppName()+"[i] = NULL;");
	}
	else
	{
		InitId.add("for (i=0; i<"+ind->getSizeName()+"; ++i)\t"+cppName()+"[i]."+initFunction+"(("+ind->getName()+")i);");
	}

	//
	// generate destroy code
	//
	if (!ClassNode->indexUsedInDestroy)
	{
		DestroyId.add("uint\ti;");
		ClassNode->indexUsedInDestroy = true;
	}
	DestroyId.add("for (i=0; i<"+ind->getSizeName()+"; ++i)");
	DestroyId.add("{");
	if (useReference)
	{
		DestroyId.add("if ("+cppName()+"[i] != NULL)");
		DestroyId.add("{");
		DestroyId.add(Type+"*\t"+objectVariable+" = "+cppName()+"[i];");
		DestroyId.add(objectVariable+"->"+destroyFunction+"();");
		DestroyId.add("delete "+objectVariable+";");
		DestroyId.add("}");
	}
	else
	{
		DestroyId.add(cppName()+"[i]."+destroyFunction+"();");
	}
	DestroyId.add("}");

	//
	// generate create code
	//

	//
	// generate fetch code
	//
	if (useReference)
	{
		if (!ClassNode->indexUsedInFetch)
		{
			FetchId.add("uint\ti;");
			ClassNode->indexUsedInFetch = true;
		}
		if (!ClassNode->tableAndRowIndicesUsedInFetch)
		{
			FetchId.add(TTableIndexName+"\ttableIndex;\n"+TRowIndexName+"\trowIndex;");
			ClassNode->tableAndRowIndicesUsedInFetch = true;
		}
		FetchId.add("for (i=0; i<"+ind->getSizeName()+"; ++i)");
		FetchId.add("{");
		FetchId.add(cppName()+"[i] = NULL;");
		FetchId.add("// read table and row, create an object, affect to the ref, and fetch it");
		FetchId.add("data.serial(tableIndex, rowIndex);");
		FetchId.add("if (rowIndex != "+INVALID_ROW_INDEXName+" && tableIndex != "+INVALID_TABLE_INDEXName+")");
		FetchId.add("{");
		FetchId.add(cppName()+"[i] = static_cast<"+Type+"*>("+pdslibFunc("create")+"(tableIndex));");
		FetchId.add(pdslibFunc("setRowIndex")+"(rowIndex, "+cppName()+"[i]);");
		FetchId.add(cppName()+"[i]->"+fetchFunction+"(data);");
		FetchId.add(cppName()+"[i]->"+setUnnotifiedParentFunction+"(this);");
		FetchId.add("}");
		FetchId.add("}");
	}
	else
	{
		if (!ClassNode->indexUsedInFetch)
		{
			FetchId.add("uint\ti;");
			ClassNode->indexUsedInFetch = true;
		}
		if (!ClassNode->tableAndRowIndicesUsedInFetch)
		{
			FetchId.add(TTableIndexName+"\ttableIndex;\n"+TRowIndexName+"\trowIndex;");
			ClassNode->tableAndRowIndicesUsedInFetch = true;
		}
		FetchId.add("for (i=0; i<"+ind->getSizeName()+"; ++i)");
		FetchId.add("{");
		FetchId.add("// read table and row, create an object, affect to the ref, and fetch it");
		FetchId.add("data.serial(tableIndex, rowIndex);");
		FetchId.add("if (rowIndex != "+INVALID_ROW_INDEXName+" && tableIndex != "+INVALID_TABLE_INDEXName+")");
		FetchId.add("{");
		FetchId.add(pdslibFunc("setRowIndex")+"(rowIndex, &"+cppName()+"[i]);");
		FetchId.add(cppName()+"[i]."+fetchFunction+"(data);");
		FetchId.add(cppName()+"[i]."+setUnnotifiedParentFunction+"(this);");
		FetchId.add("}");
		FetchId.add("}");
	}

	//
	// generate register/unregister code
	//
	if (!useReference)
	{
		if (!ClassNode->indexUsedInRegister)
		{
			RegisterAttributesId.add("uint\ti;");
			ClassNode->indexUsedInRegister = true;
		}
		RegisterAttributesId.add("for (i=0; i<"+ind->getSizeName()+"; ++i)");
		RegisterAttributesId.add("{");
		RegisterAttributesId.add(cppName()+"[i]."+registerFunction+"();");
		RegisterAttributesId.add(cppName()+"[i]."+setParentFunction+"(this);");
		RegisterAttributesId.add("}");

		if (!ClassNode->indexUsedInUnregister)
		{
			UnregisterAttributesId.add("uint\ti;");
			ClassNode->indexUsedInUnregister = true;
		}
		UnregisterAttributesId.add("for (i=0; i<"+ind->getSizeName()+"; ++i)");
		UnregisterAttributesId.add("{");
		UnregisterAttributesId.add(cppName()+"[i]."+unregisterFunction+"();");
		UnregisterAttributesId.add("}");
	}
	else
	{
		if (!ClassNode->indexUsedInUnregister)
		{
			UnregisterAttributesId.add("uint\ti;");
			ClassNode->indexUsedInUnregister = true;
		}
		UnregisterAttributesId.add("for (i=0; i<"+ind->getSizeName()+"; ++i)");
		UnregisterAttributesId.add("{");
		UnregisterAttributesId.add("if ("+cppName()+"[i] != NULL)");
		UnregisterAttributesId.add("{");
		UnregisterAttributesId.add(Type+"*\t"+objectVariable+" = "+cppName()+"[i];");
		UnregisterAttributesId.add(objectVariable+"->"+unregisterFunction+"();");
		UnregisterAttributesId.add(objectVariable+"->"+destroyFunction+"();");
		UnregisterAttributesId.add("delete "+objectVariable+";");
		UnregisterAttributesId.add("}");
		UnregisterAttributesId.add("}");
	}

	//
	// generate init/release notification
	//

	if (!ClassNode->indexUsedInNotifyInit)
	{
		NotifyInitId.add("uint\ti;");
		ClassNode->indexUsedInNotifyInit = true;
	}
	NotifyInitId.add("for (i=0; i<"+ind->getSizeName()+"; ++i)");
	NotifyInitId.add("{");

	if (!ClassNode->indexUsedInNotifyRelease)
	{
		NotifyReleaseId.add("uint\ti;");
		ClassNode->indexUsedInNotifyRelease = true;
	}
	NotifyReleaseId.add("for (i=0; i<"+ind->getSizeName()+"; ++i)");
	NotifyReleaseId.add("{");

	if (!useReference)
	{
		NotifyInitId.add(cppName()+"[i]."+notifyInitFunction+"();");

		NotifyReleaseId.add(cppName()+"[i]."+notifyReleaseFunction+"();");
	}
	else
	{
		NotifyInitId.add("if ("+cppName()+"[i] != NULL)");
		NotifyInitId.add("{");
		NotifyInitId.add(cppName()+"[i]->"+notifyInitFunction+"();");
		NotifyInitId.add("}");

		NotifyReleaseId.add("if ("+cppName()+"[i] != NULL)");
		NotifyReleaseId.add("{");
		NotifyReleaseId.add(cppName()+"[i]->"+notifyReleaseFunction+"();");
		NotifyReleaseId.add("}");
	}

	NotifyInitId.add("}");
	NotifyReleaseId.add("}");


	//
	// generate unlink code
	//
	if (useReference)
	{
		Gen.startMethod("void", unlinkFunc(), ind->getName()+" "+keyVariable, "internal", false, inlineInternal);
		Gen.add(cppName()+"["+keyVariable+"] = NULL;");
	}

	//
	// generate clear code
	//
	ClearId.add("for (uint i=0; i<"+ind->getSizeName()+"; ++i)");
	ClearId.add("{");
	if (!useReference)
	{
		ClearId.add(cppName()+"[i]."+clearFunction+"();");
	}
	else
	{
		//ClearId.add(cppName()+"[i]->"+setParentFunction+"(NULL);");
		ClearId.add(Type+"*\t"+objectVariable+" = "+cppName()+"[i];");
		ClearId.add(objectVariable+"->"+unregisterFunction+"();");
		ClearId.add(objectVariable+"->"+destroyFunction+"();");
		ClearId.add("delete "+objectVariable+";");
	}
	ClearId.add("}");
}







void	CDeclarationNode::generateSetContent(CCallContext *context)
{
	setEnv("decltype", "set");

	CClassGenerator	&Gen = ClassNode->Gen;
	CCppOutput&	DbSummary = getDbNode()->DbSummary;

	CClassGenerator::SMethodId	&InitId = ClassNode->InitId;
	CClassGenerator::SMethodId	&ClearId = ClassNode->ClearId;
	CClassGenerator::SMethodId	&DestroyId = ClassNode->DestroyId;
	CClassGenerator::SMethodId	&FetchId = ClassNode->FetchId;
	CClassGenerator::SMethodId	&RegisterId = ClassNode->RegisterId;
	CClassGenerator::SMethodId	&RegisterAttributesId = ClassNode->RegisterAttributesId;
	CClassGenerator::SMethodId	&UnregisterId = ClassNode->UnregisterId;
	CClassGenerator::SMethodId	&UnregisterAttributesId = ClassNode->UnregisterAttributesId;
	CClassGenerator::SMethodId	&SetParentId = ClassNode->SetParentId;
	CClassGenerator::SMethodId	&SetUnnotifiedParentId = ClassNode->SetUnnotifiedParentId;
	CClassGenerator::SMethodId	&NotifyInitId = ClassNode->NotifyInitId;
	CClassGenerator::SMethodId	&NotifyReleaseId = ClassNode->NotifyReleaseId;

	CClassNode			*sub = getClassNode(Type);
	CDeclarationNode	*key = sub->getKey();
	CTypeNode			*keyType = getTypeNode(key->Type);

	bool	useReference = sub->ForceReference;
	string	access = (useReference ? "->" : ".");

	string	onChangeUser = getUserCode("onChange", context != NULL ? context->getUserCodeContext() : "");


	string	setType = "std::map<"+keyType->getName()+", " + Type + (useReference ? "*" : "") + ">";

	CDeclarationNode	*dnd = sub->getDeclarationNode(ForwardRefAttribute);
	XmlNode += " type='set' classid='"+toString(sub->Id)+"' forwardreferedid='"+toString(dnd->Id)+"' key='"+toString(key->Id)+"' ";

	setEnv("type", Type);
	setEnv("keytype", keyType->getName());
	define(useReference, "useref");

	// EGS Compat
	// -- begin

	CClassGenerator::SMethodId&	StoreId = ClassNode->StoreId;
	CClassGenerator::SMethodId&	ApplyId = ClassNode->ApplyId;

	StoreId.add("// store "+Name);
	StoreId.add("__pdr.pushStructBegin("+tokenName()+");");
	StoreId.add("for ("+setType+"::const_iterator it="+cppName()+".begin(); it!="+cppName()+".end(); ++it)");
	StoreId.add("{");
	StoreId.add(	keyType->getName()+"\tkey = (*it).first;");
	keyType->generateStoreCode(StoreId, "__Tok_MapKey", "key");
	StoreId.add(	"__pdr.pushStructBegin(__Tok_MapVal);");

	if (useReference)
	{
		generateClassPtrStoreCode("(*it).second");
	}
	else
	{
		StoreId.add("(*it).second.store(__pdr);");
	}

	StoreId.add(	"__pdr.pushStructEnd(__Tok_MapVal);");
	StoreId.add("}");
	StoreId.add("__pdr.pushStructEnd("+tokenName()+");");
	StoreId.add("// end of store "+Name);

	ApplyId.add("// apply "+Name);
	ApplyId.add("else if (__pdr.peekNextToken() == "+tokenName()+")");
	ApplyId.add("{");
	ApplyId.add(	"__pdr.popStructBegin("+tokenName()+");");
	ApplyId.add(	"while (!__pdr.isEndOfStruct())");
	ApplyId.add(	"{");
	ApplyId.add(		keyType->getName()+"\tkey;");
	keyType->generateApplyCode(ApplyId, "__Tok_MapKey", "key");

	ApplyId.add(		"__pdr.popStructBegin(__Tok_MapVal);");
	if (useReference)
	{
		ApplyId.add(	Type+"*\tobj;");
		generateClassPtrApplyCode("obj");
		ApplyId.add(	"if (obj !=  NULL)");
		ApplyId.add(	"{");
		ApplyId.add(		cppName()+"[key] = obj;");
		ApplyId.add(	"}");
	}
	else
	{
		ApplyId.add(	Type+"&\tobj = "+cppName()+"[key];");
		ApplyId.add(	"obj.apply(__pdr);");
		ApplyId.add(	"obj."+setUnnotifiedParentFunction+"(this);");
	}
	ApplyId.add(		"__pdr.popStructEnd(__Tok_MapVal);");
	ApplyId.add(	"}");
	ApplyId.add(	"__pdr.popStructEnd("+tokenName()+");");
	ApplyId.add("}");
	ApplyId.add("// end of apply "+Name);

	// -- end


	//
	// generate read accessor
	//
	Gen.startMethod(Type+"*", getFunc(), "const "+keyType->getName()+"& "+keyVariable, "methods", false, inlineAccessors);
	DbSummary << "\t" << getFunc() << "\n";
	Gen.add(setType+"::iterator _it = "+cppName()+".find("+keyVariable+");");
	Gen.add("return (_it=="+cppName()+".end() ? NULL : "+(useReference ? "(*_it).second" : "&((*_it).second)")+");");

	Gen.startMethod("const "+Type+"*", getFunc(), "const "+keyType->getName()+"& "+keyVariable, "methods", true, inlineAccessors);
	Gen.add(setType+"::const_iterator _it = "+cppName()+".find("+keyVariable+");");
	Gen.add("return (_it=="+cppName()+".end() ? NULL : "+(useReference ? "(*_it).second" : "&((*_it).second)")+");");

	// generate map accessor
	string	stypedef = "T"+Name+"Map";
	//Gen.addOther("typedef "+setType+"\t"+stypedef+";");
	Gen.startMethod(setType+"::iterator", getFunc()+"Begin", "", "methods", false, inlineAccessors);
	Gen.add("return "+cppName()+".begin();");
	DbSummary << "\t" << getFunc() << "Begin" << "\n";
	Gen.startMethod(setType+"::iterator", getFunc()+"End", "", "methods", false, inlineAccessors);
	Gen.add("return "+cppName()+".end();");
	DbSummary << "\t" << getFunc() << "End" << "\n";
	Gen.startMethod(setType+"::const_iterator", getFunc()+"Begin", "", "methods", true, inlineAccessors);
	Gen.add("return "+cppName()+".begin();");
	Gen.startMethod(setType+"::const_iterator", getFunc()+"End", "", "methods", true, inlineAccessors);
	Gen.add("return "+cppName()+".end();");
	Gen.startMethod("const "+setType+" &", getFunc(), "", "methods", true, inlineAccessors);
	Gen.add("return "+cppName()+";"); 

	//
	// generate write accessor
	//
	if (useReference)
	{
		Gen.startMethod("void", setFunc(), Type+"* "+valueVariable, "methods", false, inlineAccessors);
		DbSummary << "\t" << setFunc() << "\n";
		Gen.add("if ("+valueVariable+" == NULL)\treturn;");
		Gen.add(keyType->getName()+"\t"+keyVariable+" = "+valueVariable+"->"+sub->getKey()->getFunc()+"();");
		Gen.add(setType+"::iterator\t_it = "+cppName()+".find("+keyVariable+");");
		Gen.add("if (_it != "+cppName()+".end())");
		Gen.add("{");
		Gen.add(Type+"*\t__prev = (*_it).second;");
		Gen.add("if (__prev == "+valueVariable+")\treturn;");
		Gen.add("__prev->"+setParentFunction+"(NULL);");
		Gen.add("__prev->"+unregisterFunction+"();");
		Gen.add("__prev->"+destroyFunction+"();");
		Gen.add("delete __prev;");
		Gen.add("}");
		Gen.add(valueVariable+"->"+setParentFunction+"(this);");
		Gen.add(cppName()+"["+keyVariable+"] = "+valueVariable+";");
		if (!onChangeUser.empty())
		{
			Gen.add("{");
			Gen.add(onChangeUser);
			Gen.add("}");
		}
	}

	if (sub->MapClass == NULL && !sub->HasInheritance && !sub->DerivatedFlag)
	{
		Gen.startMethod(Type+"*", newFunc(), "const "+keyType->getName()+" &"+keyVariable, "methods", false, inlineAccessors);
		DbSummary << "\t" << newFunc() << "\n";
		Gen.add(setType+"::iterator\t__it = "+cppName()+".find("+keyVariable+");");
		Gen.add("if (__it == "+cppName()+".end())");
		Gen.add("{");
		Gen.add("__it = "+cppName()+".insert("+setType+"::value_type("+keyVariable+", "+(useReference ? toString("new ") : toString(""))+Type+"())).first;");
		Gen.add(Type+"*\t"+objectVariable+" = "+(useReference ? toString("") : toString("&"))+"((*__it).second);");
		Gen.add(objectVariable+"->"+initFunction+"("+keyVariable+");");
		Gen.add(objectVariable+"->"+registerFunction+"();");
		Gen.add(objectVariable+"->"+setParentFunction+"(this);");
		Gen.add("}");
		if (!onChangeUser.empty())
		{
			Gen.add("{");
			Gen.add(onChangeUser);
			Gen.add("}");
		}
		Gen.add("return "+(useReference ? toString("") : toString("&"))+"((*__it).second);");
	}

	Gen.startMethod("void", deleteFunc(), "const "+keyType->getName()+" &"+keyVariable, "methods", false, inlineAccessors);
	DbSummary << "\t" << deleteFunc() << "\n";
	Gen.add(setType+"::iterator\t__it = "+cppName()+".find("+keyVariable+");");
	Gen.add("if (__it == "+cppName()+".end())\treturn;");
	if (useReference)
	{
		Gen.add(Type+"*\t"+objectVariable+" = (*__it).second;");
		Gen.add(objectVariable+"->"+unregisterFunction+"();");
		Gen.add(objectVariable+"->"+destroyFunction+"();");
		Gen.add("delete "+objectVariable+";");
	}
	else
	{
		Gen.add(Type+"&\t"+objectVariable+" = (*__it).second;");
		Gen.add(objectVariable+"."+unregisterFunction+"();");
		Gen.add(cppName()+".erase(__it);");
	}
	if (!onChangeUser.empty())
	{
		Gen.add("{");
		Gen.add(onChangeUser);
		Gen.add("}");
	}

	//
	// generate attribute
	//
	Gen.addAttribute(setType, cppName(), "attributes");

	//
	// generate init
	//

	//
	// generate destroy code
	//
	DestroyId.add("for ("+setType+"::iterator __it="+cppName()+".begin(); __it!="+cppName()+".end(); )");
	DestroyId.add("{");
	DestroyId.add(setType+"::iterator __itr=__it++;");
	if (useReference)
	{
		DestroyId.add(Type+"*\t"+objectVariable+" = ((*__itr).second);");
		DestroyId.add("if ("+objectVariable+" != NULL)");
		DestroyId.add("{");
		DestroyId.add(objectVariable+"->"+destroyFunction+"();");
		DestroyId.add("delete "+objectVariable+";");
		DestroyId.add("}");
	}
	else
	{
		DestroyId.add("((*__itr).second)."+destroyFunction+"();");
	}
	DestroyId.add("}");
	DestroyId.add(cppName()+".clear();");

	//
	// generate create code
	//

	//
	// generate fetch code
	//
	if (!ClassNode->tableAndRowIndicesUsedInFetch)
	{
		FetchId.add(TTableIndexName+"\ttableIndex;\n"+TRowIndexName+"\trowIndex;");
		ClassNode->tableAndRowIndicesUsedInFetch = true;
	}
	FetchId.add("do");
	FetchId.add("{");
	FetchId.add("// read table and row, create an object, affect to the ref, and fetch it");
	FetchId.add("data.serial(tableIndex, rowIndex);");
	FetchId.add("if (rowIndex == "+INVALID_ROW_INDEXName+" || tableIndex == "+INVALID_TABLE_INDEXName+")\tbreak;");

	FetchId.add(keyType->getName()+"\t"+keyVariable+";");
	if (keyType->isEnum())
	{
		FetchId.add("data.serialEnum("+keyVariable+");");
	}
	else if (keyType->CppType != keyType->StorageType)
	{
		FetchId.add("{");
		FetchId.add(keyType->StorageType+"\t_v;");
		FetchId.add("data.serial(_v);");
		FetchId.add(keyVariable+" = "+keyType->castToCpp("_v")+";");
		FetchId.add("}");
	}
	else
	{
		FetchId.add("data.serial("+keyVariable+");");
	}
	if (useReference)
	{
		FetchId.add(Type+"*\t"+objectVariable+" = static_cast<"+Type+"*>("+pdslibFunc("create")+"(tableIndex));");
		FetchId.add(cppName()+".insert(std::make_pair<"+keyType->getName()+","+Type+"*>("+keyVariable+", "+objectVariable+"));");
	}
	else
	{
		FetchId.add(cppName()+".insert(std::make_pair<"+keyType->getName()+","+Type+">("+keyVariable+", "+Type+"()));");
		FetchId.add(Type+"*\t"+objectVariable+" = &("+cppName()+"["+keyVariable+"]);");
	}
	FetchId.add(pdslibFunc("setRowIndex")+"(rowIndex, "+objectVariable+");");
	FetchId.add(objectVariable+"->"+fetchFunction+"(data);");
	FetchId.add(objectVariable+"->"+setUnnotifiedParentFunction+"(this);");
	FetchId.add("}");
	FetchId.add("while (true);");

	//
	// generate register/unregister code
	//

	UnregisterAttributesId.add("for ("+setType+"::iterator __it="+cppName()+".begin(); __it!="+cppName()+".end(); )");
	UnregisterAttributesId.add("{");
	UnregisterAttributesId.add(setType+"::iterator __itr=__it++;");
	if (useReference)
	{
		UnregisterAttributesId.add(Type+"*\t"+objectVariable+" = (*__itr).second;");
		UnregisterAttributesId.add(objectVariable+"->"+unregisterFunction+"();");
		UnregisterAttributesId.add(objectVariable+"->"+destroyFunction+"();");
		UnregisterAttributesId.add("delete "+objectVariable+";");
	}
	else
	{
		UnregisterAttributesId.add(Type+"&\t"+objectVariable+" = (*__itr).second;");
		UnregisterAttributesId.add(objectVariable+"."+unregisterFunction+"();");
	}
	UnregisterAttributesId.add("}");

	//
	// generate init/release notification
	//

	NotifyInitId.add("for ("+setType+"::iterator __it="+cppName()+".begin(); __it!="+cppName()+".end(); )");
	NotifyInitId.add("{");
	NotifyInitId.add(setType+"::iterator __itr=__it++;");

	NotifyReleaseId.add("for ("+setType+"::iterator __it="+cppName()+".begin(); __it!="+cppName()+".end(); )");
	NotifyReleaseId.add("{");
	NotifyReleaseId.add(setType+"::iterator __itr=__it++;");

	if (!useReference)
	{
		NotifyInitId.add("(*__itr).second."+notifyInitFunction+"();");
		NotifyReleaseId.add("(*__itr).second."+notifyReleaseFunction+"();");
	}
	else
	{
		NotifyInitId.add("(*__itr).second->"+notifyInitFunction+"();");
		NotifyReleaseId.add("(*__itr).second->"+notifyReleaseFunction+"();");
	}

	NotifyInitId.add("}");
	NotifyReleaseId.add("}");

	//
	// generate unlink code
	//
	if (useReference)
	{
		Gen.startMethod("void", unlinkFunc(), keyType->getName()+" "+keyVariable, "internal", false, inlineInternal);
		Gen.add(cppName()+".erase("+keyVariable+");");
	}

	//
	// generate clear code
	//
	ClearId.add("for ("+setType+"::iterator __it="+cppName()+".begin(); __it!="+cppName()+".end(); )");
	ClearId.add("{");
	ClearId.add(setType+"::iterator __itr=__it++;");
	if (!useReference)
	{
		ClearId.add(Type+"*\t"+objectVariable+" = &((*__itr).second);");
	}
	else
	{
		ClearId.add(Type+"*\t"+objectVariable+" = (*__itr).second;");
	}
	//ClearId.add(objectVariable+"->"+setParentFunction+"(NULL);");
	ClearId.add(objectVariable+"->"+unregisterFunction+"();");
	ClearId.add(objectVariable+"->"+destroyFunction+"();");
	if (useReference)
	{
		ClearId.add("delete "+objectVariable+";");
	}
	ClearId.add("}");
	ClearId.add(cppName()+".clear();");
}

//
string	CDeclarationNode::displayPrintfPrefix()
{
	if (DeclarationType != SimpleType)
		error("can't display other declaration than simple type", "internal");

	CTypeNode*	type = getTypeNode(Type);

	if (type->CppType == "CEntityId" || type->CppType == "CSheetId")
		return "%s";
	else if (type->CppType == "uint64" || type->CppType == "sint64")
		return "%\"NL_I64\"d";
	else if (type->CppType == "float" || type->CppType == "double")
		return "%g";
	else
		return "%d";
}

string	CDeclarationNode::displayCppCode(string replVar)
{
	if (DeclarationType != SimpleType)
		error("can't display other declaration than simple type", "internal");

	if (replVar.empty())
		replVar = cppName();

	CTypeNode*	type = getTypeNode(Type);

	if (type->CppType == "CEntityId" || type->CppType == "CSheetId")
		return replVar+".toString().c_str()";
	else 
		return replVar;
}


string	CDeclarationNode::toUint64(string replVar)
{
	if (DeclarationType != SimpleType)
		error("can't display other declaration than simple type", "internal");

	if (replVar.empty())
		replVar = cppName();

	CTypeNode*	type = getTypeNode(Type);

	if (type->CppType == "CEntityId")
		return replVar+".asUint64()";
	else if (type->CppType == "CSheetId")
		return "(uint64)("+replVar+".asInt())";
	else 
		return "(uint64)"+replVar;
}





bool	CLogMsgNode::prolog()
{
	getDbNode()->LogNodes.push_back(this);

	getFileNode()->IncludeDbFile = true;
	getFileNode()->IncludeStandard = true;
	getFileNode()->IncludePDSLib = true;

	return true;
}

void	CLogMsgNode::generateContent()
{
	//
	uint	j;

	CFunctionGenerator		logfunc;
	CClassGenerator			logclass;
	string					argcall;

	CFunctionGenerator&		initDb = getDbNode()->initDb;

	logfunc.init(Name);
	logfunc.setType("void");
	logfunc.IsInline = inlineLogFunctions;

	logclass.init("Log"+Name);
	logclass.createPublic("pub");
	logclass.createPrivate("priv");

	CClassGenerator::SMethodId	startlogid = logclass.startMethod("void", logStartFunction, "", "priv", false, inlineLogFunctions);
	CClassGenerator::SMethodId	stoplogid = logclass.startMethod("void", logStopFunction, "", "priv", false, inlineLogFunctions);
	startlogid.add(pdslibFunc("pushContext")+"();");
	stoplogid.add(pdslibFunc("popContext")+"();");

	map<string, CParseNode*>	params;

	for (j=0; j<Params.size(); ++j)
	{
		string	type = Params[j].first;
		string	name = Params[j].second;

		CTypeNode	*tnd;
		CClassNode	*cnd;
		if ( (tnd = getTypeNode(type, false)) )
		{
			pair<map<string, CParseNode*>::iterator, bool>	res = params.insert(make_pair<string, CParseNode*>(name, tnd));
			if (!res.second)
				error("log parameter '"+name+"' already defined");

			if (!logfunc.Proto.empty())
				logfunc.Proto += ", ";
			if (!argcall.empty())
				argcall += ", ";

			logfunc.Proto += tnd->getName()+" "+name;
			argcall += name;
		}
		else if ( (cnd = getClassNode(type, false)) )
		{
			pair<map<string, CParseNode*>::iterator, bool>	res = params.insert(make_pair<string, CParseNode*>(name, cnd));
			if (!res.second)
				error("log parameter '"+name+"' already defined");

			if (!logfunc.Proto.empty())
				logfunc.Proto += ", ";
			if (!argcall.empty())
				argcall += ", ";

			logfunc.Proto += "const "+type+"& "+name;
			argcall += name;
		}
		else if (type == "string")
		{
			CExtLogTypeNode*	extnd = new CExtLogTypeNode();
			extnd->ExtLogType = "string";
			pair<map<string, CParseNode*>::iterator, bool>	res = params.insert(make_pair<string, CParseNode*>(name, extnd));
			if (!res.second)
				error("log parameter '"+name+"' already defined");

			if (!logfunc.Proto.empty())
				logfunc.Proto += ", ";
			if (!argcall.empty())
				argcall += ", ";

			logfunc.Proto += "const std::string& "+name;
			argcall += name;
		}
		else
		{
			error("'"+type+"' not found as a class or a type");
		}
	}

	// 
	initDb.add("// Init "+Name+" log message and parameters");

	for (j=0; j<Logs.size(); ++j)
	{
		uint	logId = Id+j;

		logfunc.add(pdslibFunc("log")+"("+toString(logId)+");");
		startlogid.add(pdslibFunc("log")+"("+toString(logId)+");");

		const char	*cptr = Logs[j].c_str();
		string		log;

		// parse log line

		getDbNode()->xmlDescription.push_back("<logmsg id='"+toString(logId)+"' context='"+string(Context ? "true" : "false")+"'>");

		uint				paramNum = 0;

		initDb.add(pdslibFunc("initLog")+"("+toString(logId)+");");

		while (*cptr != '\0')
		{
			if (*cptr == '$')
			{
				++cptr;
				if (*cptr == '\0')
					error("log format corrupted in line \""+Logs[j]+"\"");

				if (*cptr == '$')
				{
					log += "$$";
					++cptr;
					continue;
				}

				string	param;
				string	var;

				while (*cptr!='\0' && *cptr!='$' && *cptr!='.')
					param += *(cptr++);

				if (*cptr == '\0')
					error("log format corrupted in line \""+Logs[j]+"\"");

				if (*cptr == '.')
				{
					++cptr;
					while (*cptr!='\0' && *cptr!='$')
						var += *(cptr++);
				}

				if (*cptr != '$')
					error("log format corrupted in line \""+Logs[j]+"\"");

				++cptr;

				map<string, CParseNode*>::iterator	it = params.find(param);

				if (it == params.end())
					error("'"+param+"' ot found in prototype, at line \""+Logs[j]+"\"");

				CTypeNode	*tnd = NULL;
				CExtLogTypeNode*	extnd = NULL;

				if (var.empty())
				{
					// is simple type
					tnd = dynamic_cast<CTypeNode*>((*it).second);
					extnd = dynamic_cast<CExtLogTypeNode*>((*it).second);
					if (tnd != NULL)
					{
						logfunc.add(pdslibFunc("logPush")+"("+tnd->castToPDS(param)+");");
						startlogid.add(pdslibFunc("logPush")+"("+tnd->castToPDS(param)+");");
					}
					else if (extnd != NULL && extnd->ExtLogType == "string")
					{
						logfunc.add(pdslibFunc("logPush")+"("+param+");");
						startlogid.add(pdslibFunc("logPush")+"("+param+");");
					}
					else
					{
						error("misuse of parameter '"+param+"' at line \""+Logs[j]+"\", missing attribute name");
					}
				}
				else
				{
					// is class
					CClassNode	*cnd = dynamic_cast<CClassNode*>((*it).second);
					if (!cnd)
						error("misuse of parameter '"+param+"' at line \""+Logs[j]+"\", should be a class");

					CDeclarationNode	*dnd = cnd->getDeclaration(var);

					if (!dnd->IsType)
						error("attribute '"+var+"' is not a simple type");

					tnd = getTypeNode(dnd->Type);

					logfunc.add(pdslibFunc("logPush")+"("+tnd->castToPDS(param+"."+dnd->getFunc()+"()")+");");
					startlogid.add(pdslibFunc("logPush")+"("+tnd->castToPDS(param+"."+dnd->getFunc()+"()")+");");
				}

				if (tnd != NULL)
				{
					getDbNode()->xmlDescription.push_back("<param id='"+toString(paramNum)+"' typeid='"+toString(tnd->Id)+"'/>");
					initDb.add(pdslibFunc("initLogParam")+"("+toString(logId)+", "+toString(paramNum)+", "+toString(tnd->Size)+");");
				}
				else
				{
					getDbNode()->xmlDescription.push_back("<param id='"+toString(paramNum)+"' typeid='"+extnd->ExtLogType+"'/>");
					initDb.add(pdslibFunc("initLogParam")+"("+toString(logId)+", "+toString(paramNum)+", "+toString(sizeof(uint16))+");");
				}


				log += "$"+toString(paramNum);

				++paramNum;
			}
			else
			{
				log += *(cptr++);
			}
		}

		getDbNode()->xmlDescription.push_back("<msg>"+xmlSpecialChars(log)+"</msg>");
		getDbNode()->xmlDescription.push_back("</logmsg>");
	}

	logclass.get(startlogid).Proto = logfunc.Proto;

	CClassGenerator::SMethodId	construct = logclass.startConstructor(logfunc.Proto, "pub", inlineLogFunctions, "");
	construct.add(logStartFunction+"("+argcall+");");
	CClassGenerator::SMethodId	destruct = logclass.startDestructor("pub", inlineLogFunctions);
	destruct.add(logStopFunction+"();");

	CFileNode*	file = getFileNode();

	if (Context)
	{
		logclass.flush(file->hOutput(), file->cppOutput(), file->inlineOutput());
		file->hOutput() << "\n";
	}
	else
	{
		logfunc.flush(file->hOutput(), file->cppOutput(), file->inlineOutput());
		file->hOutput() << "\n";
	}
}

