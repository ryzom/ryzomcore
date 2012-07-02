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

#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"
#include "nel/misc/string_common.h"
#include "nel/misc/path.h"
#include "nel/misc/sstring.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/factory.h"
#include "nel/ligo/primitive.h"
#include "nel/ligo/primitive_utils.h"
#include "nel/ligo/ligo_config.h"
#include "../../../common/src/game_share/string_manager_sender.h"
#include "nel/misc/string_conversion.h"
#include <string>
#include <numeric>

class CMissionData;
class IStep;

const std::string NL("\n\r");

/// Exception thows when the parser meet somethink invalid.
struct EParseException
{
	EParseException(NLLIGO::IPrimitive *prim, const char *why)
		:	Primitive(prim),
		Why(why)
	{
	}
	
	NLLIGO::IPrimitive	*Primitive;
	std::string		Why;
};


// utility to untag a variable (tag are the '$' added before and after the var name)
inline void untagVar(std::string &var)
{
	if (!var.empty())
	{
		if (var[0] != '$')
			var = "";
		else
		{
			var = var.substr(1);
			if (var[var.size()-1] != '$')
				var = "";
			else
				var = var.substr(0, var.size()-1);
		}
	}
}

// utility to 'tabulate' the lines in a string
void tabulateLine(std::string &text, uint nbTabs);


/* Interface class for variables types*/
class IVar
{
public:

	typedef NLLIGO::IPrimitive *	TCtorParam;

	enum TVarType
	{
		vt_integer,
		vt_npc,
		vt_item,
		vt_place,
		vt_text
	};
	
	IVar(TVarType type, NLLIGO::IPrimitive *prim)
	{
		_VarType = type;
		_VarName = getPrimProperty(prim, "var_name");
	}
	
	// force virtual destructor
	virtual ~IVar() {}
	
	/** Return the variable type. Should return a string to 
	 *	limit coupling with implementation ?
	 */
	TVarType getVarType() const
	{
		return _VarType;
	}

	/**	Return the type of the variable as defined in the string manager.
	 *	Can return  NB_ENTITY_TYPES for compiler variable that don't match 
	 *	to a type in the string manager.
	 */
	virtual STRING_MANAGER::TParamType getStringManagerType()=0;

	/// Return the name of the variable.
	const std::string getVarName() const
	{
		return _VarName;
	}
	
	/// Evaluate the content of the variable with an optional sub part.
	virtual std::string evalVar(const std::string &subPart) = 0;
	
	/** Factory method to create new variable. Caller become responsible
	 *	for deleting the allocated variable.
	 */
	static IVar *createVar(CMissionData &md, NLLIGO::IPrimitive *prim);

	/// Generate variable declaration in mission script
	virtual std::string genDecl(CMissionData &md) = 0;

	/// Generate the phrase
	virtual std::string genPhrase()
	{
		static std::string emptyString;
		return emptyString;
	}
	
protected:
	/// Helper function to read a primitive property.
	std::string getPrimProperty(NLLIGO::IPrimitive *prim, const std::string &propName)
	{
		std::string *s;
		if (prim->getPropertyByName(propName.c_str(), s))
			return *s;
		else
			return "";
	}

	/// Helper function to read a primitive array property
	std::vector<std::string> getPrimPropertyArray(NLLIGO::IPrimitive *prim, const std::string &propName)
	{
		std::vector<std::string> *sv;
		if (prim->getPropertyByName(propName.c_str(), sv))
			return *sv;
		else
			return std::vector<std::string>();
	}
protected:
	/// Variable type.
	TVarType			_VarType;
	/// Variable name.
	std::string			_VarName;
};

class CMissionData;


/** Class for text management.
 *	Handle different expression of text such
 *	phrase identifier, reference to text variable
 *	or literal string.
 *	The class also handle the parameter list for
 *	the phrase.
 */
class CPhrase
{
public:
	/// Structure to store phrase parameters infos.
	struct TParamInfo
	{
		/// The name of the parameter
		std::string					ParamName;
		/// Compiler param
		std::string					CompilerParam;
		/// The type of the parameters (as defined in the string manager).
		STRING_MANAGER::TParamType	ParamType;

		TParamInfo()
			: ParamType(STRING_MANAGER::NB_PARAM_TYPES)
		{
		}

		TParamInfo(const std::string &name, STRING_MANAGER::TParamType type, const std::string &compilerParam = "")
			: ParamName(name),
			ParamType(type),
			CompilerParam(compilerParam)
		{
		}
	};

	typedef std::vector<std::vector<TParamInfo> >	TPredefParams;

private:

	/** Number of variant for this phrase.
	 *	Zero denote that there is no variant, thus no need to add
	 *	a postfix number after the phrase identifier for literal
	 *	generated phrase.
	 */
	uint32					_NumEntry;

	/** The phrase id. This is the identifier that must be use with the 
	 *	string manager.
	 */
	std::string				_PhraseId;
	/** String literal value. When the user fill a quoted string, then
	 *	this property receive the string value of the quoted string.
	 */
	std::vector<std::string> _PhraseLiterals;
	/** Suffixe for literal string identifier */
	std::string				_Suffixe;
	/// The list of default parameters.
	TPredefParams			_DefaultParams;
	/// The list of additional parameters.
	std::vector<TParamInfo>	_AdditionalParams;
public:
	

	/// init the phrase
	void initPhrase (CMissionData &md,
				NLLIGO::IPrimitive *prim, 
				const std::vector<std::string> &texts, 
				uint32 numEntry = 0, 
				const TPredefParams &predefParams = TPredefParams());

	/// generate the common string script for any script instruction.
	std::string genScript(CMissionData &md);

	/** generate the phrase file content. Return empty string 
	 *	if the phrase is not a literal.
	 */
	std::string genPhrase();

	/// Test if the phrase is empty
	bool isEmpty();

	/// Test if the phrase contains some additionnal parameters
	bool asAdditionnalParams();
};


/* Class for jumps */
struct TJumpInfo
{
	std::string		StepName;
	std::string		JumpName;
	bool			Discardable;

	TJumpInfo(const std::string &stepName, const std::string &jumpName = std::string(), bool discardable = true)
		: StepName(stepName),
		JumpName(jumpName),
		Discardable(discardable)
	{}

	bool operator <(const TJumpInfo &other) const
	{
		return StepName < other.StepName;
	}
};


/* Class for mission information storing */
class CMissionData : public NLMISC::CRefCount
{
public:
	
	CMissionData();
	~CMissionData();


//	void setAlias(const std::string &alias)	{ _Alias = alias; }
//	const std::string &getAlias()			{ return _Alias; }

	void setMissionName(const std::string &missionName);
	const std::string &getMissionName();

	const std::string &getGiverPrimitive()	{ return _GiverPrimitive; }
	void setGiverPrimitive(const std::string &giverPrimitive)	{ _GiverPrimitive = giverPrimitive; }

	const std::string &getGiverName()		{ return _MissionGiver; }
	void setGiverName(const std::string &giverName)	{ _MissionGiver = giverName; }
	
	bool addVariable(NLLIGO::IPrimitive *prim, IVar *var);

	IVar *getVariable(const std::string &varName);
	
	bool addStep(IStep *step);
	void addStepName(const std::string &name, IStep *step) { _StepsByNames[name] = step; }

	IStep *getNextStep(IStep *current);

	IStep *getStepByName(const std::string &stepName);

	std::string generateMissionScript(const std::string &primFileName);

	std::string generatePhraseFile();
	
	std::string generateDotScript();

	void parseMissionHeader(NLLIGO::IPrimitive *prim);
	void initHeaderPhrase(NLLIGO::IPrimitive *prim);
	void parsePrerequisites(NLLIGO::IPrimitive *prim);
	
	std::string replaceVar(NLLIGO::IPrimitive *prim, const std::string &str);
	std::vector<std::string> replaceVar(NLLIGO::IPrimitive *prim, const std::vector<std::string> &strs);

	std::string getProperty(NLLIGO::IPrimitive *prim, const std::string &propertyName, bool replaceVar, bool canFail);
	std::vector<std::string> getPropertyArray(NLLIGO::IPrimitive *prim, const std::string &propertyName, bool replaceVar, bool canFail);


	bool isThereAJumpTo(const std::string &stepName);

	bool isGuildMission() const
	{
		return _Guild;
	}

private:

	std::string genPreRequisites();

	// forbidden copy constructor !
	CMissionData(const CMissionData &other)
	{
		nlstop;
	}

//	std::string		_Alias;	
	std::string		_MissionName;
	std::string		_MissionGiver;
	std::string		_GiverPrimitive;
	bool		_MonoInstance;
	bool		_RunOnce;
	bool		_Replayable;
	bool		_Solo;
	bool		_Guild;
	bool		_NotInJournal;
	bool		_AutoRemoveFromJournal; // When mission ends (fail or success) automatically remove it from the journal
	std::string _MissionCategory;
	uint32		_PlayerReplayTimer;
	uint32		_GlobalReplayTimer;
	bool		_NotProposed;
	bool		_NonAbandonnable;
	bool		_NeedValidation;
	bool		_FailIfInventoryIsFull;
	std::string _MissionIcon;

	std::vector<std::string>	_MissionTitleRaw;
	CPhrase						_MissionTitle;
	std::vector<std::string>	_MissionDescriptionRaw;
	CPhrase						_MissionDescription;

	bool						_MissionAuto;
	std::vector<std::string>	_MissionAutoMenuRaw;
	CPhrase						_MissionAutoMenu;

	////// Pre requisites /////////
	struct TReqSkill
	{
		std::string		Skill;
		std::string		MinLevel;
		std::string		MaxLevel;
	};

	struct TReqFame
	{
		std::string		Faction;
		std::string		Fame;
	};
	std::vector<TReqSkill>		_ReqSkills;
	std::vector<std::string>	_ReqMissionDone;
	std::vector<std::string>	_ReqMissionNotDone;
	std::vector<std::string>	_ReqMissionRunning;
	std::vector<std::string>	_ReqMissionNotRunning;
	std::vector<std::string>	_ReqWearItem;
	std::vector<std::string>	_ReqOwnItem;
	std::string					_ReqTitle;
	std::vector<TReqFame>		_ReqFames;
	bool						_ReqGuild;
	std::string					_ReqGrade;
	std::string					_ReqTeamSize;
	std::vector<std::string>	_ReqBrick;
	std::string					_ReqSeason;
//	bool						_ReqEncycloTasksDone;
	std::string					_ReqEncyclo;
	std::string					_ReqEncycloNeg;
	std::string					_ReqEventFaction;

	/// The list of parent missions
	std::set<std::string>				_ParentMissions;
	/// The list of variable by name
	std::map<std::string, IVar*>		_Variables;
	/// The list of variable in primitive order
	std::vector<IVar*>					_VariablesOrder;
	/// the list of step in execution order
	std::vector<IStep*>					_Steps;
	/// The list of step sorted by step name
	std::map<std::string, IStep *>		_StepsByNames;

	std::set<TJumpInfo>	_JumpPoints;

};

typedef NLMISC::CSmartPtr<CMissionData>	TMissionDataPtr;

/** This class manage the compilation of missions contained under a 
 *	primitive file node.
 */
class CMissionCompiler
{
public:

	/** Generate the dot language script for the missions under the specified node.
	 *	This method is primarily used for world editor mission display.
	 */
//	std::vector<std::string>	generateDotScript(NLLIGO::NLLIGO::IPrimitive *rootPrim);
	bool generateDotScript(NLLIGO::IPrimitive *missionPrim, std::string &dotScript, std::string &log);

	/** compile one mission. The primitive node must be
	 *	a mission tree root node.
	 *	fileName must receive the primitive file name. It is used
	 */
	bool compileMission(NLLIGO::IPrimitive *rootPrim, const std::string &primFileName);

	/** Compile all the missions found under the given primitive node.
	 *	The primitive tree is searched recursively ti find all the 
	 *	mission tree nodes.
	 *	All the compiled missions are stored internaly in precompiled form.
	 */
	bool compileMissions(NLLIGO::IPrimitive *rootPrim, const std::string &primFileName);

	/** Install the generated script into the destination primitive files */
	bool installCompiledMission(NLLIGO::CLigoConfig &ligoConfig, const std::string &primFileName);

	/// Publish the modified to the path parameter
	bool publishFiles(const std::string &serverPathPrim, const std::string &serverPathText, const std::string &localPathText);

	/// Search for text in the file : add it if it's not in
	bool includeText(const std::string filename, const std::string text);
	
	/// Parse the pre requisite node of a mission.
	bool parsePreRequisite(CMissionData &md, NLLIGO::IPrimitive *preReq);

	/// Parse the steps of a missions.
	bool parseSteps(CMissionData &md, NLLIGO::IPrimitive *steps, IStep *parent=NULL);
	bool parseOneStep(CMissionData &md, NLLIGO::IPrimitive *stepToParse, IStep *parent, bool bEndOfBranch);

	/// Helper to retrive a property in a primitive node.
	std::string getProp(NLLIGO::IPrimitive *prim, const std::string &propName);
	/// Helper to retreive the class name of a primitive node.
	std::string getClass(NLLIGO::IPrimitive *prim);
	/// Parse the variable of a missions.
	bool parseVariables(CMissionData &md, NLLIGO::IPrimitive *variables);


	/// Get full paths of files to publish
	uint getFileToPublishCount() { return (uint)_FilesToPublish.size(); }
	std::string getFileToPublish(uint index) { nlassert(index < _FilesToPublish.size()); return _FilesToPublish[index]; }

	
	std::vector <TMissionDataPtr> &getMissions()
	{
		return _CompiledMission;
	}
	uint getMissionsCount()
	{
		return (uint)_CompiledMission.size();
	}
	TMissionDataPtr	getMission(uint index)
	{
		nlassert(index < _CompiledMission.size());
		return _CompiledMission[index];
	}

	
private:

	/// Storage for loaded primitive
	struct TLoadedPrimitive
	{
		NLLIGO::CPrimitives	*PrimDoc;
		std::string			FullFileName;

		TLoadedPrimitive()
			: PrimDoc(NULL)
		{}
		
		TLoadedPrimitive(NLLIGO::CPrimitives *primDoc, const std::string &fullFileName)
			: PrimDoc(primDoc),
			FullFileName(fullFileName)
		{
		}
	};

	/// Storage for precompiled missions.
	std::vector <TMissionDataPtr>	_CompiledMission;

	/// Storage for files to publish
	std::vector<std::string>		_FilesToPublish;
};


// Class to easily handle var and var name pair
struct TCompilerVarName
{
	typedef std::vector <std::vector<struct TCompilerVarName> > TPredefParams;
	// Default parameter name
	std::string			_DefaultName;
	// Type of script parameter 
	STRING_MANAGER::TParamType	_ParamType;
	// the name of the compiler var, like 'the_creature' in $the_creature$ = chab1
	std::string	_VarName;
	// the value of the compiler var line 'chab1' in $the_creature$ = chab1
	std::string	_VarValue;

	void init(const std::string &defaultName, STRING_MANAGER::TParamType type, CMissionData &md, NLLIGO::IPrimitive *prim, const std::string propName);

	void initWithText(const std::string &defaultName, STRING_MANAGER::TParamType type, CMissionData &md, NLLIGO::IPrimitive *prim, const std::string &text);	
	
	CPhrase::TParamInfo getParamInfo() const;

	bool empty() const;

	operator const std::string  () const;

	operator CPhrase::TParamInfo() const;

	//static void getPhrasePredef(const TCompilerVarName::TPredefParams& compilerParams, CPhrase::TPredefParams& params);	
	
	static std::vector<TCompilerVarName> getPropertyArrayWithText(const std::string &defaultName, STRING_MANAGER::TParamType type, CMissionData &md, NLLIGO::IPrimitive *prim, const std::string & arrayProperyName);
	static std::vector<TCompilerVarName> getPropertyArrayWithTextStaticDefaultName(const std::string &defaultName, STRING_MANAGER::TParamType type, CMissionData &md, NLLIGO::IPrimitive *prim, const std::string & arrayProperyName);


};

std::string operator+(const TCompilerVarName& left, const std::string & right);
std::string operator+(const std::string & left, const TCompilerVarName& right);


