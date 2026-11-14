// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2026  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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
#include "rt_scenario_check.h"

#include "object.h"

#include "nel/misc/common.h"
#include "nel/misc/string_common.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <map>
#include <vector>

using namespace NLMISC;

namespace R2 {

namespace {

/// Largest part of a text of the tree that is copied into the description of a refusal
const uint32 s_MaxQuotedTextSize = 64;

/// Largest sheet id (a sheet id is 32 bits)
const double s_MaxSheetId = 4294967295.0;

/// Largest weight of an action (it is read as a signed 32 bit value)
const double s_MaxActionWeight = 2147483647.0;

// The names of the attributes that are read. They are kept as strings, so that looking one up
// does not make a string each time.
const std::string s_Empty;
const std::string s_KeyAction("Action");
const std::string s_KeyActDescription("ActDescription");
const std::string s_KeyActionsId("ActionsId");
const std::string s_KeyActs("Acts");
const std::string s_KeyAiActivity("AiActivity");
const std::string s_KeyAiMovement("AiMovement");
const std::string s_KeyAiProfilParams("AiProfilParams");
const std::string s_KeyAiProfilesParams("AiProfilesParams");
const std::string s_KeyEquipment("Equipment");
const std::string s_KeyEvent("Event");
const std::string s_KeyGroupsByName("GroupsByName");
const std::string s_KeyGrpParameters("GrpParameters");
const std::string s_KeyStatesByName("StatesByName");
const std::string s_KeyAiStates("AiStates");
const std::string s_KeyAngle("Angle");
const std::string s_KeyChatParameters("ChatParameters");
const std::string s_KeyChildren("Children");
const std::string s_KeyComment("Comment");
const std::string s_KeyDescription("Description");
const std::string s_KeyEntryPoint("EntryPoint");
const std::string s_KeyEvents("Events");
const std::string s_KeyGrp("Grp");
const std::string s_KeyId("Id");
const std::string s_KeyIsland("Island");
const std::string s_KeyKeywords("Keywords");
const std::string s_KeyLocationId("LocationId");
const std::string s_KeyLocations("Locations");
const std::string s_KeyName("Name");
const std::string s_KeyNpcs("Npcs");
const std::string s_KeyParameters("Parameters");
const std::string s_KeyPlotItems("PlotItems");
const std::string s_KeyPreActDescription("PreActDescription");
const std::string s_KeyPt("Pt");
const std::string s_KeyPts("Pts");
const std::string s_KeySeason("Season");
const std::string s_KeySheet("Sheet");
const std::string s_KeySheetClient("SheetClient");
const std::string s_KeySheetId("SheetId");
const std::string s_KeyText("Text");
const std::string s_KeyTexts("Texts");
const std::string s_KeyUserTriggers("UserTriggers");
const std::string s_KeyWeatherValue("WeatherValue");
const std::string s_KeyWeight("Weight");
const std::string s_KeyX("x");
const std::string s_KeyY("y");
const std::string s_KeyZ("z");

/// A text of the tree for the description of a refusal: shortened, and without control characters
std::string quoteText(const std::string &text)
{
	std::string out;
	uint32 size = (uint32)std::min((size_t)s_MaxQuotedTextSize, text.size());
	out.reserve(size + 5);
	out += '\'';
	for (uint32 i = 0; i < size; ++i)
	{
		char c = text[i];
		out += ((uint8)c < 0x20 || (uint8)c == 0x7f) ? '?' : c;
	}
	out += '\'';
	if (text.size() > size)
		out += "...";
	return out;
}

const CObject *getAttrPtr(const CObject *table, const std::string &name)
{
	return table->getAttr(name).getPtr();
}

bool isTable(const CObject *object)
{
	return object && object->isTable(s_Empty);
}

/// The text of a string or of a ref id of the tree, without a copy; NULL for anything else
const std::string *getText(const CObject *object)
{
	const CObjectString *text = dynamic_cast<const CObjectString *>(object);
	return text ? &text->getValue() : nullptr;
}

/// A number of the tree as a double. Nothing is converted to an integer type here, so a number of
/// any size, an infinity or a NaN can be read and compared.
bool getNumber(const CObject *object, double &value)
{
	const CObjectNumber *number = dynamic_cast<const CObjectNumber *>(object);
	if (!number)
		return false;
	value = number->getNumberValue();
	return true;
}

/// A number of the tree that is neither an infinity nor a NaN and has no fractional part
bool getWholeNumber(const CObject *object, double &value)
{
	if (!getNumber(object, value))
		return false;
	return isValidDouble(value) && value == floor(value);
}

std::string describeNumber(double value)
{
	if (isValidDouble(value) && value == floor(value) && fabs(value) < 1e15)
		return toString("%.0f", value);
	return toString("%g", value);
}

/// What a value of the tree is, for the description of a refusal
std::string describeValue(const CObject *object)
{
	if (!object)
		return "missing";
	double number;
	if (getNumber(object, number))
		return "the number " + describeNumber(number);
	if (object->isTable(s_Empty))
		return toString("a table of %u values", object->getSize());
	const std::string *text = getText(object);
	if (text)
		return "the text " + quoteText(*text);
	return "a value of an unknown type";
}

// The types of action of a scenario: the ones the translator of the editor gives an action
// (Translator.createAction in r2_translator.lua, which writes most of what the editor offers as a
// "code" action), and the ones the animation module has a translation of its own for (facing,
// condition_if, condition_if_else). The AI service has more types of action, which are for the
// primitives of a shard: they send messages to other services, or work on the outposts and the
// dynamic system of a continent. The list is sorted.
const char *const s_ActionTypes[] = {
	"begin_state",
	"code",
	"condition_if",
	"condition_if_else",
	"dynamic_if",
	"emot",
	"facing",
	"modify_variable",
	"multi_actions",
	"npc_say",
	"null_action",
	"punctual_state",
	"punctual_state_end",
	"random_select",
	"say",
	"set_timer_t0",
	"set_timer_t1",
	"set_timer_t2",
	"set_timer_t3",
	"sit_down",
	"stand_up",
	"switch_actions",
	"trigger_event_0",
	"trigger_event_1",
	"trigger_event_2",
	"trigger_event_3",
	"trigger_event_4",
	"trigger_event_5",
	"trigger_event_6",
	"trigger_event_7",
	"trigger_event_8",
	"trigger_event_9",
};

// The native functions of the AI service the scripts of a scenario call, by their signature
// (name_parameters_results; f is a number, s a text, c a group). These are the functions the
// translator of the editor writes scripts with (r2_translator.lua, r2_logic.lua), and
// setConditionSuccess, which the AI service puts around the condition of a dynamic_if action. The
// other native functions are for the scripts of a shard: they run commands of the service, read
// files, act on players by their entity id or on the data of other services. The list is sorted.
const char *const s_ScriptFunctions[] = {
	"activateEasterEgg_fffsffffsss_",
	"addPersistentProfileParameter_s_",
	"addProfileParameter_s_",
	"break__",
	"context__c",
	"deactivateEasterEgg_fff_",
	"debug_f_",
	"debug_s_",
	"despawn_f_",
	"dssMessage_fsss_",
	"dssStartAct_ff_",
	"emote_css_",
	"endScenarioTiming_f_",
	"facing_cscs_",
	"giveMissionItems_ssc_",
	"giveReward_ssssc_",
	"isAlived__f",
	"npcSay_css_",
	"postNextState_s_",
	"receiveMissionItems_ssc_",
	"removePersistentProfileParameter_s_",
	"rndm_ff_f",
	"setActivity_s_",
	"setAggro_ff_",
	"setBotAttackable_f_",
	"setConditionSuccess_f_",
	"setEvent_f_",
	"setFactionProp_ss_",
	"setHPScale_f_",
	"setMode_s_",
	"setPlayerAttackable_f_",
	"setScenarioPoints_ff_",
	"setTimer_ff_",
	"sitDown__",
	"spawn__",
	"standUp__",
	"startScenarioTiming_f_",
	"stopMoving__",
	"talkTo_sc_",
	"teleportNear_fffc_",
	"timerAdd_ff_",
	"timerDisable_f_",
	"timerIsEnabled_f_f",
	"timerIsSuspended_f_f",
	"timerResume_f_",
	"timerSetRyzomDaytime_fff_",
	"timerSuspend_f_",
	"wander__",
};

struct CNameLess
{
	bool operator()(const char *a, const std::string &b) const { return b.compare(a) > 0; }
	bool operator()(const std::string &a, const char *b) const { return a.compare(b) < 0; }
	bool operator()(const char *a, const char *b) const { return strcmp(a, b) < 0; }
};

/// True when name is the name of one of the functions of s_ScriptFunctions, whatever its parameters
bool isScriptFunctionName(const std::string &name)
{
	// the signatures of a name follow each other in the list: the first that is not before "name_"
	std::string prefix = name + "_";
	const char *const *end = s_ScriptFunctions + sizeof(s_ScriptFunctions) / sizeof(s_ScriptFunctions[0]);
	const char *const *found = std::lower_bound(s_ScriptFunctions, end, prefix, CNameLess());
	return found != end && strncmp(*found, prefix.c_str(), prefix.size()) == 0;
}

/**
 * The tokens of a script of the AI service, as its lexer makes them (script_parser.lex), for what
 * is looked for here: the calls of native functions. The compiler removes the comments of each line
 * before the lexer sees the text (CCompiler::compileCode); that is done here too.
 */
class CScriptTokens
{
public:
	enum TToken
	{
		TokenEnd,
		TokenName,
		TokenContextName,
		TokenStringName,
		TokenKeyword,
		TokenLeftParenthesis,
		TokenRightParenthesis,
		TokenPoint,
		TokenSeparator,
		TokenLeftBrace,
		TokenRightBrace,
		TokenSemicolon,
		/// A text between quotes; getTextSize() tells its size
		TokenText,
		/// A number or an operator
		TokenOther
	};

	/// script is the parameter of the action as the client uploaded it; isCondition tells that it is
	/// the condition of a dynamic_if action
	CScriptTokens(const std::string &script, bool isCondition)
	    : m_Position(0)
	    , m_TextSize(0)
	{
		// The lines of a condition are the lines of the text that are not empty: the animation module
		// makes the parameters of an action of them, and the AI service puts the last one inside an
		// if. The lines of a script are all kept, as the compiler joins the lines it is given. The
		// animation module gives it none that is empty, so a line that begins with '#' after empty
		// lines begins the script there, and is a comment: here it is read as script, which is what
		// it is when the empty lines are compiled with it.
		std::vector<std::string> lines;
		std::string::size_type begin = 0;
		while (begin < script.size())
		{
			std::string::size_type end = script.find('\n', begin);
			if (end == std::string::npos)
				end = script.size();
			if (end != begin || !isCondition)
				lines.push_back(script.substr(begin, end - begin));
			begin = end + 1;
		}

		if (isCondition && !lines.empty())
		{
			// The animation module puts the prefix of the session before each line of a condition that
			// has a colon in it, and the AI service puts the last line between the parentheses of an
			// if. Any name does for the prefix here.
			if (script.find(':') != std::string::npos)
			{
				for (uint32 i = 0; i < lines.size(); ++i)
					lines[i] = "r2_0000_" + lines[i];
			}
			lines.back() = "if (" + lines.back() + "){()setConditionSuccess(1);} else { ()setConditionSuccess(0); }";
		}

		// The compiler cuts a line that has "//" in it at the first "//" outside a text between
		// quotes, and joins the lines again. It looks for the quotes as this does: the character
		// after a '/' that is alone is copied without being looked at.
		m_Code.reserve(script.size() + 128);
		for (uint32 l = 0; l < lines.size(); ++l)
		{
			const std::string &line = lines[l];
			if (line.find("//") == std::string::npos)
			{
				m_Code += line;
			}
			else
			{
				bool inQuote = false;
				std::string::size_type i = 0;
				while (i < line.size())
				{
					if (line[i] == '"')
						inQuote = !inQuote;
					if (!inQuote && line[i] == '/')
					{
						++i;
						if (i == line.size())
						{
							m_Code += '/';
							break;
						}
						if (line[i] == '/')
							break;
						m_Code += '/';
					}
					m_Code += line[i];
					++i;
				}
			}
			m_Code += "\n ";
		}
	}

	/// The next token; text receives the name of a name token
	TToken next(std::string &text)
	{
		const std::string &code = m_Code;
		std::string::size_type size = code.size();
		while (m_Position < size)
		{
			char c = code[m_Position];
			if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
			{
				++m_Position;
			}
			else if (c == '/' && m_Position + 1 < size && code[m_Position + 1] == '/')
			{
				// a comment the compiler left, to the end of the line
				m_Position = skipLine(m_Position);
			}
			else if (c == '#' && m_Position == 0)
			{
				// a comment line: the lexer has this for a line that begins with '#', and after the
				// compiler joined the lines only the first one begins where a line begins
				m_Position = skipLine(m_Position);
			}
			else if (c == '"')
			{
				// A text ends at a quote. A quote after a backslash can be inside the text: the lexer
				// takes the longest text it can, so the text ends at the last quote that only has
				// such quotes before it on the line. Without an end, the quote is no token at all.
				std::string::size_type last = std::string::npos;
				for (std::string::size_type i = m_Position + 1; i < size && code[i] != '\n'; ++i)
				{
					if (code[i] == '"')
					{
						last = i;
						if (code[i - 1] != '\\' || i - 1 == m_Position)
							break;
					}
				}
				if (last == std::string::npos)
				{
					++m_Position;
				}
				else
				{
					m_TextSize = (uint32)(last - m_Position - 1);
					m_Position = last + 1;
					return TokenText;
				}
			}
			else if (isNameStart(c) || ((c == '@' || c == '$') && m_Position + 1 < size && isNameStart(code[m_Position + 1])))
			{
				std::string::size_type begin = m_Position;
				++m_Position;
				while (m_Position < size && isNameChar(code[m_Position]))
					++m_Position;
				if (c == '@')
					return TokenContextName;
				if (c == '$')
					return TokenStringName;
				text.assign(code, begin, m_Position - begin);
				return isKeyword(text) ? TokenKeyword : TokenName;
			}
			else if (c >= '0' && c <= '9')
			{
				// a number: its digits are no name, whatever follows them is read as the next token
				while (m_Position < size && code[m_Position] >= '0' && code[m_Position] <= '9')
					++m_Position;
				return TokenOther;
			}
			else
			{
				++m_Position;
				switch (c)
				{
				case '(': return TokenLeftParenthesis;
				case ')': return TokenRightParenthesis;
				case '.': return TokenPoint;
				case ',': return TokenSeparator;
				case '{': return TokenLeftBrace;
				case '}': return TokenRightBrace;
				case ';': return TokenSemicolon;
				case '=':
				case ':':
				case '<':
				case '>':
				case '+':
				case '-':
				case '*':
				case '/':
					return TokenOther;
				case '!':
					if (m_Position < size && code[m_Position] == '=')
					{
						++m_Position;
						return TokenOther;
					}
					break;
				case '&':
				case '|':
					if (m_Position < size && code[m_Position] == c)
					{
						++m_Position;
						return TokenOther;
					}
					break;
				default:
					// the lexer has no token for this character: the tokens around it follow each other
					break;
				}
			}
		}
		return TokenEnd;
	}

	/// The bytes between the quotes of the text next() returned last
	uint32 getTextSize() const { return m_TextSize; }

private:
	static bool isNameStart(char c) { return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_'; }
	static bool isNameChar(char c) { return isNameStart(c) || (c >= '0' && c <= '9'); }

	static bool isKeyword(const std::string &name)
	{
		static const char *const keywords[] = { "if", "else", "while", "print", "log", "switch", "case", "rand", "onchildren" };
		for (uint32 i = 0; i < sizeof(keywords) / sizeof(keywords[0]); ++i)
		{
			if (name == keywords[i])
				return true;
		}
		return false;
	}

	std::string::size_type skipLine(std::string::size_type position) const
	{
		std::string::size_type end = m_Code.find('\n', position);
		return end == std::string::npos ? m_Code.size() : end + 1;
	}

	std::string m_Code;
	std::string::size_type m_Position;
	uint32 m_TextSize;
};

/**
 * Looks for the calls of native functions in a script. A call is written "(results) name(...)" or
 * "(results) group.name(...)", where the results are variables between parentheses, or nothing.
 * Returns false at the first call of a function that is not one of a scenario, with its name.
 *
 * This is not the compiler of the AI service. That service refuses the same functions when it
 * compiles the scripts of a scenario, by their signature; here the client gets its answer when it
 * uploads, and a text this function does not read as the compiler does is refused there.
 */
bool checkScriptCalls(const std::string &script, bool isCondition, std::string &refusal)
{
	// The limits are far above what the translator of the editor writes (rt_scenario_check.h)
	if (script.size() > RtScenarioMaxScriptSize)
	{
		refusal = toString("has %u bytes, a script has %u at most", (uint32)script.size(), RtScenarioMaxScriptSize);
		return false;
	}

	// The lexer of the AI service reads a script up to its first zero byte, and has no token for the
	// other control characters: a script has the tab and the ends of its lines, and no other
	for (std::string::size_type i = 0; i < script.size(); ++i)
	{
		uint8 c = (uint8)script[i];
		if (c < 0x20 && c != '\t' && c != '\n' && c != '\r')
		{
			refusal = toString("has the control character 0x%02x at byte %u, a script has the tab and the ends of its lines only", (uint32)c, (uint32)i);
			return false;
		}
	}

	std::vector<uint8> tokens;
	std::vector<std::string> names;
	{
		CScriptTokens lexer(script, isCondition);
		std::string text;
		uint32 depth = 0;
		// the tokens since the last ';', '{' or '}'
		uint32 statementTokens = 0;
		// The "else" that follow one another, for each brace that is open: an "else" comes after the
		// ';' or the '}' that ends a statement, and anything else there ends the "if" it would be of
		std::vector<uint32> elseChains(1, 0);
		bool statementEnded = false;
		for (;;)
		{
			CScriptTokens::TToken token = lexer.next(text);
			if (token == CScriptTokens::TokenEnd)
				break;
			if (token == CScriptTokens::TokenSemicolon || token == CScriptTokens::TokenLeftBrace || token == CScriptTokens::TokenRightBrace)
			{
				statementTokens = 0;
			}
			else
			{
				++statementTokens;
				if (statementTokens > RtScenarioMaxScriptStatementTokens)
				{
					refusal = toString("has a statement of more than %u tokens (names, numbers, texts, operators and parentheses between two of ';', '{' and '}')", RtScenarioMaxScriptStatementTokens);
					return false;
				}
			}
			if (token == CScriptTokens::TokenKeyword && text == "else")
			{
				++elseChains.back();
				if (elseChains.back() > RtScenarioMaxScriptElseChain)
				{
					refusal = toString("has more than %u 'else' that follow one another", RtScenarioMaxScriptElseChain);
					return false;
				}
			}
			else if (statementEnded)
			{
				elseChains.back() = 0;
			}
			statementEnded = token == CScriptTokens::TokenSemicolon || token == CScriptTokens::TokenRightBrace;
			if (token == CScriptTokens::TokenLeftBrace)
				elseChains.push_back(0);
			else if (token == CScriptTokens::TokenRightBrace && elseChains.size() > 1)
				elseChains.pop_back();
			if (token == CScriptTokens::TokenText && lexer.getTextSize() > RtScenarioMaxScriptTextSize)
			{
				refusal = toString("has a text of %u bytes between quotes, a text of a script has %u at most", lexer.getTextSize(), RtScenarioMaxScriptTextSize);
				return false;
			}
			if (token == CScriptTokens::TokenLeftParenthesis || token == CScriptTokens::TokenLeftBrace)
			{
				++depth;
				if (depth > RtScenarioMaxScriptDepth)
				{
					refusal = toString("has parentheses and braces more than %u deep", RtScenarioMaxScriptDepth);
					return false;
				}
			}
			else if ((token == CScriptTokens::TokenRightParenthesis || token == CScriptTokens::TokenRightBrace) && depth != 0)
			{
				--depth;
			}
			// The translator of the editor writes no loop, no block the AI service picks one statement
			// of at random and no block for the groups below a group
			if (token == CScriptTokens::TokenKeyword && (text == "while" || text == "rand" || text == "onchildren"))
			{
				refusal = "uses " + quoteText(text) + ", which a script of a scenario does not have";
				return false;
			}
			tokens.push_back((uint8)token);
			names.push_back(token == CScriptTokens::TokenName ? text : s_Empty);
		}
	}
	// the tokens that are looked at after the last one
	uint32 count = (uint32)tokens.size();
	tokens.resize(count + 4, (uint8)CScriptTokens::TokenEnd);
	// the names that are the function of a native call
	std::vector<bool> isNativeCall(count + 4, false);

	for (uint32 i = 0; i < count; ++i)
	{
		if (tokens[i] != CScriptTokens::TokenLeftParenthesis)
			continue;

		// the results: variables, each with the group it is of or without, between commas
		uint32 j = i + 1;
		bool isResults = true;
		while (tokens[j] != CScriptTokens::TokenRightParenthesis)
		{
			if ((tokens[j] == CScriptTokens::TokenName || tokens[j] == CScriptTokens::TokenContextName) && tokens[j + 1] == CScriptTokens::TokenPoint)
				j += 2;
			if (tokens[j] != CScriptTokens::TokenName && tokens[j] != CScriptTokens::TokenStringName && tokens[j] != CScriptTokens::TokenContextName)
			{
				isResults = false;
				break;
			}
			++j;
			if (tokens[j] == CScriptTokens::TokenSeparator)
			{
				++j;
			}
			else if (tokens[j] != CScriptTokens::TokenRightParenthesis)
			{
				isResults = false;
				break;
			}
		}
		if (!isResults)
			continue;

		// the group the function is called on, if any, and the function
		++j;
		if ((tokens[j] == CScriptTokens::TokenName || tokens[j] == CScriptTokens::TokenContextName) && tokens[j + 1] == CScriptTokens::TokenPoint)
			j += 2;
		if (tokens[j] == CScriptTokens::TokenName && tokens[j + 1] == CScriptTokens::TokenLeftParenthesis)
		{
			if (!isScriptFunctionName(names[j]))
			{
				refusal = "calls the function " + quoteText(names[j]) + ", which is not a function of a scenario";
				return false;
			}
			isNativeCall[j] = true;
		}
	}

	// A name before a parenthesis that is not a native call defines or calls a function of the script
	// itself: the translator of the editor writes none
	for (uint32 i = 0; i < count; ++i)
	{
		if (tokens[i] == CScriptTokens::TokenName && tokens[i + 1] == CScriptTokens::TokenLeftParenthesis && !isNativeCall[i])
		{
			refusal = "defines or calls the script function " + quoteText(names[i]) + ", which a script of a scenario does not have";
			return false;
		}
	}
	return true;
}

/// True for the id of a state, a group, an npc or an event: the service makes their names in the AI
/// service from it ("r2_<session>_<id>"), and the scripts name them by it
bool isAiName(const std::string &id)
{
	if (id.empty() || id.size() > RtScenarioMaxIdSize)
		return false;
	for (std::string::size_type i = 0; i < id.size(); ++i)
	{
		char c = id[i];
		if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_'))
			return false;
	}
	return true;
}

const char *const s_IdRule = ": an id has letters, digits and '_', 64 at most";

/// True for what the translator of the editor names a state by where an event or an action names
/// one: its id, or its name, which is made of an id, a point or a bar, and a word
bool isAiStateName(const std::string &name)
{
	if (name.empty() || name.size() > RtScenarioMaxIdSize * 2)
		return false;
	for (std::string::size_type i = 0; i < name.size(); ++i)
	{
		char c = name[i];
		if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_' || c == '.' || c == '|'))
			return false;
	}
	return true;
}

const char *const s_StateNameRule = ": the name of a state or a group has letters, digits, '_', '.' and '|', 128 at most";

/// The lines of a text that are not empty: the animation module makes the parameters of an action
/// and the properties of a group or an npc of them
void splitLines(const std::string &text, std::vector<std::string> &lines)
{
	std::string::size_type begin = 0;
	while (begin < text.size())
	{
		std::string::size_type end = text.find('\n', begin);
		if (end == std::string::npos)
			end = text.size();
		if (end != begin)
			lines.push_back(text.substr(begin, end - begin));
		begin = end + 1;
	}
}

/// The lines of the parameters of an npc_say action, as the animation module reads them (it uses
/// NLMISC::CSString::splitLines there): without the '\r' of the text, and with the lines that are
/// empty, but for one the text would end with
void splitSayLines(const std::string &text, std::vector<std::string> &lines)
{
	std::string line;
	bool open = false;
	for (std::string::size_type i = 0; i < text.size(); ++i)
	{
		char c = text[i];
		if (c == '\r')
			continue;
		if (c == '\n')
		{
			lines.push_back(line);
			line.clear();
			open = false;
		}
		else
		{
			line += c;
			open = true;
		}
	}
	if (open)
		lines.push_back(line);
}

bool isOneOf(const std::string &text, const char *const *list, uint32 count)
{
	for (uint32 i = 0; i < count; ++i)
	{
		if (text == list[i])
			return true;
	}
	return false;
}

/// True for a text made of these characters only, and not empty
bool isMadeOf(const std::string &text, const char *characters)
{
	return !text.empty() && text.find_first_not_of(characters) == std::string::npos;
}

const char *const s_NameCharacters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_";
const char *const s_EmoteCharacters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_ -";

/// A number as the translator writes one into a parameter: digits, with a fraction or not
bool isPlainNumber(const std::string &text)
{
	if (text.empty() || text.size() > 16 || text[0] < '0' || text[0] > '9')
		return false;
	std::string::size_type point = text.find('.');
	if (point != std::string::npos && (point + 1 == text.size() || text.find('.', point + 1) != std::string::npos))
		return false;
	return text.find_first_not_of("0123456789.") == std::string::npos;
}

// The types of event the translator of the editor gives an event (Translator.createEvent in
// r2_translator.lua). The AI service has others, for the primitives of a shard.
const char *const s_EventTypes[] = {
	"bot_killed", "bot_target_killed", "destination_reached", "destination_reached_all", "end_of_state",
	"group_despawned", "group_eliminated", "group_spawned", "group_under_attack",
	"player_arrived_trigger_zone", "player_left_trigger_zone", "player_target_npc", "start_of_state",
	"timer_t0_triggered", "timer_t1_triggered", "timer_t2_triggered", "timer_t3_triggered",
	"user_event_0", "user_event_1", "user_event_2", "user_event_3", "user_event_4",
	"user_event_5", "user_event_6", "user_event_7", "user_event_8", "user_event_9",
	"variable_changed", "variable_v0_changed", "variable_v1_changed", "variable_v2_changed", "variable_v3_changed"
};

// What the translator writes into the parameters of a group (Translator.setGroupParameters and
// createAiGroup in r2_translator.lua); "aggro range: <number>" is tested apart
const char *const s_GroupParameters[] = {
	"ring", "bot_attackable", "player_attackable", "respawn time:-1", "denied_astar_flags:WaterAndNoGo"
};

// The movement the translator of the editor gives a state: of a step of an activity
// (Logic.getAiMovement in r2_logic.lua), of the states around a sequence ("idle"), and for the
// state of an npc without activities what its behavior has in the scenario (r2.Utils.setState in
// r2_utils.lua), which is nothing in a scenario of this editor. The AI service finds a profile by
// this word, among all the profiles it has: some are for the groups of a shard, and some are not
// for a state at all.
const char *const s_StateMovements[] = {
	"", "follow_route", "idle", "stand_on_start_point", "stand_on_vertices", "wander"
};

// The activity the translator of the editor gives a state: nothing for the npcs that have a profile
// of the editor, "no_change" for the others (Translator.getAiActivity in r2_translator.lua), and
// "normal" for the state of a feature
const char *const s_StateActivities[] = {
	"", "no_change", "normal"
};

struct CWalkFrame
{
	const CObject *Table;
	uint32 Next;
	uint32 Size;
	/// False below a table whose id was already taken: the instance map of the animation module
	/// leaves such a table and what it holds out
	bool Register;
};

/// A table of the tree that has an id, and what was found about it so far
struct CIdEntry
{
	const CObject *Table;
	/// A state names it as one of its groups
	bool NamedAsGroup;
	/// A group names it as one of its npcs
	bool NamedAsNpc;
	/// An event names it as one of its actions, or it is an action below another one
	bool NamedAsAction;

	CIdEntry(const CObject *table)
	    : Table(table)
	    , NamedAsGroup(false)
	    , NamedAsNpc(false)
	    , NamedAsAction(false)
	{
	}
};

/// An order of the ids: by their size, then by their bytes. Any order does, as long as it is one.
struct CIdLess
{
	bool operator()(const std::string *a, const std::string *b) const
	{
		if (a->size() != b->size())
			return a->size() < b->size();
		return memcmp(a->data(), b->data(), a->size()) < 0;
	}
};

/// The tables of the tree by their id. The ids are not copied: they are texts of the tree, which
/// does not change while it is checked. A tree can have half a million of them, and whoever sends
/// the tree chooses them: they are kept in order, so that finding one takes the same number of
/// steps whatever the ids are.
typedef std::map<const std::string *, CIdEntry, CIdLess> TIds;

struct CActCounts
{
	/// Size of the Npcs table: what the service has always counted
	uint32 Npcs;
	/// Bots the translation makes: one for each npc of a group of a state
	uint32 Bots;
	/// Groups the states name
	uint32 Groups;
	/// States, groups, bots and events: each takes an alias in the primitive file of the act
	uint32 Aliases;
	/// Actions the events start, with the actions below them
	uint32 Actions;

	CActCounts()
	    : Npcs(0)
	    , Bots(0)
	    , Groups(0)
	    , Aliases(0)
	    , Actions(0)
	{
	}
};

/**
 * Every table is looked at once: a group, an npc and an action can be named once. An attribute is
 * found by going through its table, so the lookups of one table cost its size a few times, and those
 * of the whole check the size of the tree a few times.
 *
 * A refusal is described where it is found, and the functions it returns through put the place in
 * front of it: nothing is formatted for a tree that passes.
 */

class CRtScenarioChecker
{
public:
	CRtScenarioChecker(const CRtScenarioLimits &limits)
	    : m_Limits(limits)
	    , m_Verdict(RtScenarioOk)
	    , m_Located(false)
	    , m_ActionNamed(false)
	    , m_LocationCount(0)
	    , m_PrimNodes(0)
	    , m_ActIndex(0)
	{
	}

	TRtScenarioVerdict run(const CObject *root);

	const std::string &getError() const { return m_Error; }

private:
	bool walkTree(const CObject *root);
	bool registerId(const CObject *table);
	CIdEntry *findId(const std::string &id);

	bool checkPlotItems(const CObject *root);
	bool checkPlotItem(const CObject *plotItem);
	bool checkLocations(const CObject *root);
	bool checkLocation(const CObject *location);
	bool checkTexts(const CObject *root);
	bool checkActs(const CObject *root);
	bool checkAct(const CObject *act, CActCounts &counts);
	bool checkAiState(const CObject *state, CActCounts &counts);
	bool checkGroup(const CObject *group, CActCounts &counts);
	bool checkBot(const CObject *bot);
	bool checkEvent(const CObject *event, CActCounts &counts);
	bool checkAction(const CObject *action, const std::string *id, CIdEntry *entry, uint32 &count);
	bool checkActionContent(const CObject *action, uint32 &count);
	bool checkActionParameters(const CObject *action, const std::string &type);
	bool checkGroupTexts(const CObject *group);
	bool checkBotTexts(const CObject *bot);
	bool checkNameLines(const CObject *table, const std::string &name, bool states);
	bool checkUserTriggers(const CObject *act);
	bool checkPoint(const CObject *point);

	bool requireTable(const CObject *table, const std::string &name, const CObject *&value);
	bool requireText(const CObject *table, const std::string &name, const std::string *&text);
	bool requireText(const CObject *table, const std::string &name);
	bool requireWholeNumber(const CObject *table, const std::string &name, double minValue, double maxValue, double &value);
	bool checkRangeIfNumber(const CObject *table, const std::string &name, double minValue, double maxValue);

	/// Counts the nodes of the primitives the translation makes. The size of the tree does not bound
	/// them exactly: an event that holds nothing is one value, and two nodes.
	bool addPrimNodes(uint32 count);

	/// Records the refusal, with what is wrong (" is ...", or ".Attribute is ..."), and returns false
	bool refuse(TRtScenarioVerdict verdict, const std::string &what);
	bool invalid(const std::string &what) { return refuse(RtScenarioInvalid, what); }
	/// Records a refusal that is about the act that is being checked as a whole
	bool refuseAct(TRtScenarioVerdict verdict, const std::string &what);
	/// Puts a part of the place in front of the refusal, and returns false
	bool prepend(const std::string &text);
	/// Puts the place in front of the refusal, unless a function nearer to it did, and returns false
	bool locate(const std::string &where);

	const CRtScenarioLimits &m_Limits;
	TRtScenarioVerdict m_Verdict;
	std::string m_Error;
	bool m_Located;
	bool m_ActionNamed;
	/// The tables of the tree by their Id, as CInstanceMap("Id") finds them
	TIds m_Ids;
	uint32 m_LocationCount;
	uint32 m_PrimNodes;
	uint32 m_ActIndex;
};

// ****************************************************************************
bool CRtScenarioChecker::refuse(TRtScenarioVerdict verdict, const std::string &what)
{
	if (m_Verdict == RtScenarioOk)
	{
		m_Verdict = verdict;
		m_Error = what;
	}
	return false;
}

// ****************************************************************************
bool CRtScenarioChecker::refuseAct(TRtScenarioVerdict verdict, const std::string &what)
{
	if (m_Verdict == RtScenarioOk)
	{
		m_Verdict = verdict;
		m_Error = toString("Acts[%u]", m_ActIndex) + what;
		m_Located = true;
	}
	return false;
}

// ****************************************************************************
bool CRtScenarioChecker::prepend(const std::string &text)
{
	if (!m_Located)
		m_Error = text + m_Error;
	return false;
}

// ****************************************************************************
bool CRtScenarioChecker::locate(const std::string &where)
{
	if (!m_Located)
	{
		m_Error = where + m_Error;
		m_Located = true;
	}
	return false;
}

// ****************************************************************************
bool CRtScenarioChecker::addPrimNodes(uint32 count)
{
	if (count > m_Limits.MaxNodes - std::min(m_PrimNodes, m_Limits.MaxNodes))
		return invalid(toString(" makes the translated scenario larger than %u nodes", m_Limits.MaxNodes));
	m_PrimNodes += count;
	return true;
}

// ****************************************************************************
bool CRtScenarioChecker::registerId(const CObject *table)
{
	const std::string *id = getText(getAttrPtr(table, s_KeyId));
	if (!id)
		return true;
	return m_Ids.insert(std::make_pair(id, CIdEntry(table))).second;
}

// ****************************************************************************
CIdEntry *CRtScenarioChecker::findId(const std::string &id)
{
	TIds::iterator found = m_Ids.find(&id);
	return found != m_Ids.end() ? &found->second : nullptr;
}

// ****************************************************************************
// Visits every value once with an explicit stack: the depth and the number of values are checked
// before any function that recurses over the tree runs on it, and the tables are recorded by Id.
bool CRtScenarioChecker::walkTree(const CObject *root)
{
	// The frames of the open tables. They are reached through a pointer, and the array only grows
	// when a table is opened below the deepest one so far.
	std::vector<CWalkFrame> stack(64);
	CWalkFrame *frames = &stack[0];
	uint32 depth = 1;
	uint32 values = 1;

	frames[0].Table = root;
	frames[0].Next = 0;
	frames[0].Size = root->getSize();
	frames[0].Register = registerId(root);

	while (depth != 0)
	{
		CWalkFrame &top = frames[depth - 1];
		if (top.Next == top.Size)
		{
			--depth;
			continue;
		}

		const CObject *child = top.Table->getValueAtPos(top.Next).getPtr();
		++top.Next;
		if (!child)
			return invalid(" has a table that holds an empty value");

		++values;
		if (values > m_Limits.MaxNodes)
			return invalid(toString(" has more than %u values", m_Limits.MaxNodes));

		if (child->isTable(s_Empty))
		{
			if (depth >= m_Limits.MaxDepth)
				return invalid(toString(" has tables nested deeper than %u", m_Limits.MaxDepth));

			// a table that holds nothing has no id, and nothing to visit
			uint32 size = child->getSize();
			if (size != 0)
			{
				bool registered = top.Register && registerId(child);
				if (depth == stack.size())
				{
					stack.resize(stack.size() * 2);
					frames = &stack[0];
				}
				// 'top' is not used after this: the array can have moved
				CWalkFrame &frame = frames[depth];
				frame.Table = child;
				frame.Next = 0;
				frame.Size = size;
				frame.Register = registered;
				++depth;
			}
		}
	}
	return true;
}

// ****************************************************************************
bool CRtScenarioChecker::requireTable(const CObject *table, const std::string &name, const CObject *&value)
{
	value = getAttrPtr(table, name);
	if (!isTable(value))
		return invalid("." + name + " is " + describeValue(value) + ", a table is needed");
	return true;
}

// ****************************************************************************
bool CRtScenarioChecker::requireText(const CObject *table, const std::string &name, const std::string *&text)
{
	const CObject *value = getAttrPtr(table, name);
	text = getText(value);
	if (!text)
		return invalid("." + name + " is " + describeValue(value) + ", a text is needed");
	return true;
}

// ****************************************************************************
bool CRtScenarioChecker::requireText(const CObject *table, const std::string &name)
{
	const std::string *text;
	return requireText(table, name, text);
}

// ****************************************************************************
bool CRtScenarioChecker::requireWholeNumber(const CObject *table, const std::string &name, double minValue, double maxValue, double &value)
{
	const CObject *object = getAttrPtr(table, name);
	if (!getWholeNumber(object, value))
		return invalid("." + name + " is " + describeValue(object) + ", a whole number is needed");
	if (value < minValue || value > maxValue)
		return invalid("." + name + " is " + describeNumber(value) + ", outside " + describeNumber(minValue) + " to " + describeNumber(maxValue));
	return true;
}

// ****************************************************************************
// A value that is only read when it is a number: then it must be one in the range. The test is
// written so that a NaN fails it.
bool CRtScenarioChecker::checkRangeIfNumber(const CObject *table, const std::string &name, double minValue, double maxValue)
{
	double value;
	if (getNumber(getAttrPtr(table, name), value) && !(value >= minValue && value <= maxValue))
		return invalid("." + name + " is " + describeNumber(value) + ", outside " + describeNumber(minValue) + " to " + describeNumber(maxValue));
	return true;
}

// ****************************************************************************
bool CRtScenarioChecker::checkPoint(const CObject *point)
{
	// the coordinates are read from a table; anything else is left alone by the translation
	if (!isTable(point))
		return true;
	return checkRangeIfNumber(point, s_KeyX, m_Limits.MinX, m_Limits.MaxX)
	    && checkRangeIfNumber(point, s_KeyY, m_Limits.MinY, m_Limits.MaxY)
	    && checkRangeIfNumber(point, s_KeyZ, -RtScenarioMaxCoordinate, RtScenarioMaxCoordinate);
}

// ****************************************************************************
bool CRtScenarioChecker::checkPlotItem(const CObject *plotItem)
{
	if (!isTable(plotItem))
		return invalid(" is " + describeValue(plotItem) + ", a table is needed");

	// whether the sheet is one of the plot item sheets is for the service to say: it has the sheets
	double sheetId;
	if (!requireWholeNumber(plotItem, s_KeySheetId, 0.0, s_MaxSheetId, sheetId))
		return false;

	const std::string *const names[] = { &s_KeyName, &s_KeyDescription, &s_KeyComment };
	for (uint32 t = 0; t < sizeof(names) / sizeof(names[0]); ++t)
	{
		const std::string *text;
		if (!requireText(plotItem, *names[t], text))
			return false;
		if (text->size() > m_Limits.MaxPlotItemTextSize)
			return invalid("." + *names[t] + toString(" has %u bytes, the limit is %u", (uint32)text->size(), m_Limits.MaxPlotItemTextSize));
	}
	return true;
}

// ****************************************************************************
bool CRtScenarioChecker::checkPlotItems(const CObject *root)
{
	const CObject *plotItems;
	if (!requireTable(root, s_KeyPlotItems, plotItems))
		return locate("the scenario");

	uint32 count = plotItems->getSize();
	if (count > m_Limits.MaxPlotItems)
	{
		invalid(toString(" has %u plot items, the limit is %u", count, m_Limits.MaxPlotItems));
		return locate("PlotItems");
	}

	for (uint32 i = 0; i < count; ++i)
	{
		if (!checkPlotItem(plotItems->getValueAtPos(i).getPtr()))
			return locate(toString("PlotItems[%u]", i));
	}
	return true;
}

// ****************************************************************************
bool CRtScenarioChecker::checkLocation(const CObject *location)
{
	if (!isTable(location))
		return invalid(" is " + describeValue(location) + ", a table is needed");

	double season;
	return requireWholeNumber(location, s_KeySeason, 0.0, (double)RtScenarioMaxSeason, season)
	    && requireText(location, s_KeyIsland)
	    && requireText(location, s_KeyEntryPoint);
}

// ****************************************************************************
bool CRtScenarioChecker::checkLocations(const CObject *root)
{
	const CObject *locations;
	if (!requireTable(root, s_KeyLocations, locations))
		return locate("the scenario");

	m_LocationCount = locations->getSize();
	for (uint32 i = 0; i < m_LocationCount; ++i)
	{
		if (!checkLocation(locations->getValueAtPos(i).getPtr()))
			return locate(toString("Locations[%u]", i));
	}
	return true;
}

// ****************************************************************************
bool CRtScenarioChecker::checkTexts(const CObject *root)
{
	// a scenario without a text manager has no texts
	const CObject *textManager = getAttrPtr(root, s_KeyTexts);
	if (!textManager)
		return true;
	if (!textManager->isTable(s_Empty))
	{
		invalid(" is " + describeValue(textManager) + ", a table is needed");
		return locate("Texts");
	}

	const CObject *texts;
	if (!requireTable(textManager, s_KeyTexts, texts))
		return locate("Texts");

	uint32 count = texts->getSize();
	if (count > m_Limits.MaxTexts)
	{
		refuse(RtScenarioOverQuota, toString(" has %u texts, the limit is %u", count, m_Limits.MaxTexts));
		return locate("Texts.Texts");
	}

	for (uint32 i = 0; i < count; ++i)
	{
		const CObject *entry = texts->getValueAtPos(i).getPtr();
		if (!isTable(entry))
			invalid(" is " + describeValue(entry) + ", a table is needed");
		else if (requireText(entry, s_KeyId) && requireText(entry, s_KeyText))
			continue;
		return locate(toString("Texts.Texts[%u]", i));
	}
	return true;
}

// ****************************************************************************
bool CRtScenarioChecker::checkBot(const CObject *bot)
{
	const std::string *const names[] = { &s_KeyName, &s_KeyChatParameters, &s_KeyKeywords, &s_KeySheetClient, &s_KeySheet };
	for (uint32 t = 0; t < sizeof(names) / sizeof(names[0]); ++t)
	{
		if (!requireText(bot, *names[t]))
			return false;
	}

	if (!checkBotTexts(bot))
		return false;

	if (!checkPoint(getAttrPtr(bot, s_KeyPt)))
		return prepend(".Pt");
	return checkRangeIfNumber(bot, s_KeyAngle, -RtScenarioMaxAngle, RtScenarioMaxAngle);
}

// ****************************************************************************
bool CRtScenarioChecker::checkGroup(const CObject *group, CActCounts &counts)
{
	const CObject *children;
	if (!requireTable(group, s_KeyChildren, children))
		return false;

	if (!checkGroupTexts(group))
		return false;

	// the group and its parameters
	if (!addPrimNodes(2))
		return false;
	++counts.Aliases;

	uint32 count = children->getSize();
	for (uint32 i = 0; i < count; ++i)
	{
		const CObject *child = children->getValueAtPos(i).getPtr();
		const std::string *botId = getText(child);
		if (!botId)
			return invalid(toString(".Children[%u] is ", i) + describeValue(child) + ", the id of an npc is needed");
		if (!isAiName(*botId))
			return invalid(toString(".Children[%u] is the id ", i) + quoteText(*botId) + s_IdRule);

		// the count stops at the first bot over the limit, however long the list is
		++counts.Bots;
		++counts.Aliases;
		if (counts.Bots > m_Limits.MaxBotsPerAct)
			return refuseAct(RtScenarioOverQuota, toString(" has more than %u bots", m_Limits.MaxBotsPerAct));
		if (!addPrimNodes(1))
			return false;

		// An npc belongs to one group. The editor never names one twice, and the translation would
		// make a bot of it each time.
		CIdEntry *bot = findId(*botId);
		if (!bot)
			invalid(" is not in the scenario");
		else if (bot->NamedAsNpc)
			invalid(" is named a second time, an npc belongs to one group");
		else
		{
			bot->NamedAsNpc = true;
			if (checkBot(bot->Table))
				continue;
		}
		return prepend(": the npc " + quoteText(*botId));
	}
	return true;
}

// ****************************************************************************
bool CRtScenarioChecker::checkAiState(const CObject *state, CActCounts &counts)
{
	if (!isTable(state))
		return invalid(" is " + describeValue(state) + ", a table is needed");

	if (!requireText(state, s_KeyAiMovement))
		return false;

	// a state without an id has no name in the AI service
	const std::string *stateId = getText(getAttrPtr(state, s_KeyId));
	if (stateId && !isAiName(*stateId))
		return invalid(".Id is " + quoteText(*stateId) + s_IdRule);

	// The movement and the activity of a state are one of the words the translator of the editor
	// writes, or nothing. The editor gives a state no keywords and no parameters of a profile.
	const std::string *const words[] = { getText(getAttrPtr(state, s_KeyAiMovement)), getText(getAttrPtr(state, s_KeyAiActivity)) };
	for (uint32 w = 0; w < sizeof(words) / sizeof(words[0]); ++w)
	{
		if (words[w] && !(w == 0 ? isOneOf(*words[w], s_StateMovements, sizeof(s_StateMovements) / sizeof(s_StateMovements[0])) : isOneOf(*words[w], s_StateActivities, sizeof(s_StateActivities) / sizeof(s_StateActivities[0]))))
			return invalid(std::string(w == 0 ? ".AiMovement" : ".AiActivity") + " is " + quoteText(*words[w]) + (w == 0 ? ", which is not a movement of a state of a scenario" : ", which is not an activity of a state of a scenario"));
	}
	const std::string *const none[] = { getText(getAttrPtr(state, s_KeyKeywords)), getText(getAttrPtr(state, s_KeyAiProfilesParams)) };
	for (uint32 n = 0; n < sizeof(none) / sizeof(none[0]); ++n)
	{
		if (none[n] && none[n]->find_first_not_of("\n") != std::string::npos)
			return invalid(std::string(n == 0 ? ".Keywords" : ".AiProfilesParams") + " is " + quoteText(*none[n]) + ", a state of a scenario has none");
	}

	const CObject *points = getAttrPtr(state, s_KeyPts);
	if (points)
	{
		if (!points->isTable(s_Empty))
			return invalid(".Pts is " + describeValue(points) + ", a table is needed");
		uint32 pointCount = points->getSize();
		for (uint32 i = 0; i < pointCount; ++i)
		{
			if (!checkPoint(points->getValueAtPos(i).getPtr()))
				return prepend(toString(".Pts[%u]", i));
		}
	}

	const CObject *children;
	if (!requireTable(state, s_KeyChildren, children))
		return false;

	if (!addPrimNodes(1))
		return false;
	++counts.Aliases;

	uint32 count = children->getSize();
	for (uint32 i = 0; i < count; ++i)
	{
		const CObject *child = children->getValueAtPos(i).getPtr();
		const std::string *groupId = getText(child);
		if (!groupId)
			return invalid(toString(".Children[%u] is ", i) + describeValue(child) + ", the id of a group is needed");
		if (!isAiName(*groupId))
			return invalid(toString(".Children[%u] is the id ", i) + quoteText(*groupId) + s_IdRule);

		// A group belongs to one state. The editor never names one twice, and the translation would
		// make the group and its bots each time.
		CIdEntry *group = findId(*groupId);
		if (!group)
			invalid(" is not in the scenario");
		else if (group->NamedAsGroup)
			invalid(" is named a second time, a group belongs to one state");
		else
		{
			group->NamedAsGroup = true;
			++counts.Groups;
			if (counts.Groups > m_Limits.MaxGroupsPerAct)
				return refuseAct(RtScenarioOverQuota, toString(" has more than %u groups", m_Limits.MaxGroupsPerAct));
			if (checkGroup(group->Table, counts))
				continue;
		}
		return prepend(": the group " + quoteText(*groupId));
	}
	return true;
}

// ****************************************************************************
// An action and the actions below it; count receives how many they are. The actions are nested
// tables, so the recursion is as deep as the tree, which walkTree() has checked.
//
// An action belongs to one event, or to one action above it. The editor never names one twice, and
// the translation would make it and what is below it each time: an action that has an id, and so
// can be named, is refused the second time it is reached, before anything is read from it. id and
// entry are the id an event names the action by and what the id gave; they are NULL for an action
// that is below another one, whose own id is looked up here.
bool CRtScenarioChecker::checkAction(const CObject *action, const std::string *id, CIdEntry *entry, uint32 &count)
{
	bool ok = true;
	if (!entry)
	{
		if (!isTable(action))
			return invalid(": an action is " + describeValue(action) + ", a table is needed");
		id = getText(getAttrPtr(action, s_KeyId));
		if (id)
		{
			// the table that has the id, which another table of the same id is not
			entry = findId(*id);
			if (entry && entry->Table != action)
				entry = nullptr;
		}
	}

	if (entry && entry->NamedAsAction)
		ok = invalid(" is named a second time, an action belongs to one event or to one action above it");
	else
	{
		if (entry)
			entry->NamedAsAction = true;
		ok = checkActionContent(action, count);
	}

	// the action the refusal is about is named, not the ones it is below
	if (!ok && !m_ActionNamed)
	{
		m_ActionNamed = true;
		prepend(id ? ": the action " + quoteText(*id) : std::string(": an action"));
	}
	return ok;
}

// ****************************************************************************
bool CRtScenarioChecker::checkActionContent(const CObject *action, uint32 &count)
{
	const std::string *name;
	if (!requireText(action, s_KeyAction, name))
		return false;

	// The AI service has types of action that are not for a scenario: the type is one the editor makes
	if (!isRtScenarioActionType(*name))
		return invalid(".Action is " + quoteText(*name) + ", which is not an action of a scenario");

	// these actions read their parameters without a test of the type
	if (*name == "modify_variable" || *name == "condition_if" || *name == "condition_if_else" || *name == "dynamic_if")
	{
		if (!requireText(action, s_KeyParameters))
			return false;
	}

	// the parameters have the shape the translator gives them for the type
	if (!checkActionParameters(action, *name))
		return false;

	if (getAttrPtr(action, s_KeyWeight))
	{
		double weight;
		if (!requireWholeNumber(action, s_KeyWeight, 0.0, s_MaxActionWeight, weight))
			return false;
	}

	const CObject *children;
	if (!requireTable(action, s_KeyChildren, children))
		return false;

	count = 1;
	uint32 childCount = children->getSize();
	for (uint32 i = 0; i < childCount; ++i)
	{
		uint32 below = 0;
		if (!checkAction(children->getValueAtPos(i).getPtr(), nullptr, nullptr, below))
			return false;
		count += below;
	}
	return true;
}

// ****************************************************************************
// The parameters of an action go to the AI service line by line, some with the prefix of the
// session before them. The AI service reads them as the level designers of a shard write them:
// here they must be what the translator of the editor writes for the type of the action.
bool CRtScenarioChecker::checkActionParameters(const CObject *action, const std::string &type)
{
	const std::string *text = getText(getAttrPtr(action, s_KeyParameters));
	std::vector<std::string> lines;
	if (text)
		splitLines(*text, lines);

	if (type == "code" || type == "dynamic_if")
	{
		// a script of the AI service: of a size, without what the editor never writes, calling the
		// functions of a scenario
		std::string refusal;
		if (text && !checkScriptCalls(*text, type == "dynamic_if", refusal))
			return invalid(".Parameters " + refusal);
	}
	else if (type == "say")
	{
		// The translator writes "say: <phrase>". The AI service reads the word before the colon as
		// the channel the npc talks in: the npc of a scenario talks where it stands.
		if (text && text->compare(0, 4, "say:") != 0)
			return invalid(".Parameters is " + quoteText(*text) + ", a say action begins with 'say:'");
	}
	else if (type == "begin_state" || type == "punctual_state" || type.compare(0, 14, "trigger_event_") == 0)
	{
		// the names of states, or of groups
		for (uint32 i = 0; i < lines.size(); ++i)
		{
			if (!isAiStateName(lines[i]))
				return invalid(".Parameters has the line " + quoteText(lines[i]) + s_StateNameRule);
		}
	}
	else if (type == "npc_say")
	{
		// "<group>:<npc>" on a line and the id of the text on the next, which the translator leaves
		// empty for a text its user did not write. The animation module reads the lines in its own
		// way, and makes of anything less than two lines an action that has nothing to say, which
		// the AI service tells in its log each time the action runs.
		std::vector<std::string> said;
		if (text)
			splitSayLines(*text, said);
		if (said.size() < 2)
			return invalid(".Parameters is " + (text ? quoteText(*text) : describeValue(getAttrPtr(action, s_KeyParameters))) + ", an npc_say action has the npc on a line and the text on the next");
		std::string::size_type colon = said[0].find(':');
		if (colon != std::string::npos && !isAiName(said[0].substr(0, colon)))
			return invalid(".Parameters names the group " + quoteText(said[0].substr(0, colon)) + s_IdRule);
	}
	else if (type == "facing" || type == "emot")
	{
		// an emote and npcs, or two npcs; an npc is "<group>:<npc>" or its name alone
		if (type == "emot" && (lines.empty() || !isMadeOf(lines[0], s_EmoteCharacters) || lines[0].size() > RtScenarioMaxIdSize))
			return invalid(".Parameters is " + (text ? quoteText(*text) : std::string("missing")) + ", an emot action begins with the name of the emote");
		for (uint32 i = (type == "emot" ? 1 : 0); i < lines.size(); ++i)
		{
			std::string::size_type colon = lines[i].find(':');
			if (colon != std::string::npos && !isAiName(lines[i].substr(0, colon)))
				return invalid(".Parameters names the group " + quoteText(lines[i].substr(0, colon)) + s_IdRule);
		}
	}
	else if (type == "modify_variable" || type == "condition_if" || type == "condition_if_else")
	{
		// "<variable> <operator> <value>"
		std::vector<std::string> words;
		if (!lines.empty())
			NLMISC::explode(lines[0], std::string(" "), words, true);
		if (lines.size() != 1 || words.size() != 3)
			return invalid(".Parameters is " + (text ? quoteText(*text) : std::string("missing")) + ", a variable, an operator and a value are needed");
	}
	else if (type.compare(0, 11, "set_timer_t") == 0)
	{
		// a time, as a number
		if (lines.size() != 1 || !isPlainNumber(lines[0]))
			return invalid(".Parameters is " + (text ? quoteText(*text) : std::string("missing")) + ", a number is needed");
	}
	else if (type == "switch_actions")
	{
		// the variable whose value picks one of the actions below
		if (lines.size() != 1 || !isMadeOf(lines[0], s_NameCharacters) || lines[0].size() > RtScenarioMaxIdSize || (lines[0][0] >= '0' && lines[0][0] <= '9'))
			return invalid(".Parameters is " + (text ? quoteText(*text) : std::string("missing")) + ", the name of a variable is needed");
	}
	return true;
}

// ****************************************************************************
bool CRtScenarioChecker::checkNameLines(const CObject *table, const std::string &name, bool states)
{
	const std::string *text = getText(getAttrPtr(table, name));
	if (!text)
		return true;
	std::vector<std::string> lines;
	splitLines(*text, lines);
	for (uint32 i = 0; i < lines.size(); ++i)
	{
		if (states ? !isAiStateName(lines[i]) : !isAiName(lines[i]))
			return invalid("." + name + " has the line " + quoteText(lines[i]) + (states ? s_StateNameRule : s_IdRule));
	}
	return true;
}

// ****************************************************************************
// What the animation module copies of a group and of an npc into the primitive as the client wrote
// it. The translator of the editor writes a few fixed lines there; the AI service reads more, which
// is for the npcs of a shard.
bool CRtScenarioChecker::checkGroupTexts(const CObject *group)
{
	const std::string *text = getText(getAttrPtr(group, s_KeyGrpParameters));
	if (text)
	{
		std::vector<std::string> lines;
		splitLines(*text, lines);
		for (uint32 i = 0; i < lines.size(); ++i)
		{
			const std::string &line = lines[i];
			if (isOneOf(line, s_GroupParameters, sizeof(s_GroupParameters) / sizeof(s_GroupParameters[0])))
				continue;
			if (line.compare(0, 13, "aggro range: ") == 0 && isPlainNumber(line.substr(13)))
				continue;
			return invalid(".GrpParameters has the line " + quoteText(line) + ", which is not a parameter of a group of a scenario");
		}
	}
	text = getText(getAttrPtr(group, s_KeyAiProfilParams));
	if (text)
	{
		std::vector<std::string> lines;
		splitLines(*text, lines);
		for (uint32 i = 0; i < lines.size(); ++i)
		{
			if (lines[i] != "running")
				return invalid(".AiProfilParams has the line " + quoteText(lines[i]) + ", which is not a parameter of a group of a scenario");
		}
	}
	return true;
}

// ****************************************************************************
bool CRtScenarioChecker::checkBotTexts(const CObject *bot)
{
	// the editor gives an npc no chat (shops, missions, web pages) and no keywords
	const std::string *text = getText(getAttrPtr(bot, s_KeyChatParameters));
	if (text && text->find_first_not_of("\n") != std::string::npos)
		return invalid(".ChatParameters is " + quoteText(*text) + ", an npc of a scenario has none");
	text = getText(getAttrPtr(bot, s_KeyKeywords));
	if (text && text->find_first_not_of("\n") != std::string::npos)
		return invalid(".Keywords is " + quoteText(*text) + ", an npc of a scenario has none");

	// the three visual properties of an npc with a look of its own, or a word
	text = getText(getAttrPtr(bot, s_KeyEquipment));
	if (text)
	{
		std::vector<std::string> lines;
		splitLines(*text, lines);
		for (uint32 i = 0; i < lines.size(); ++i)
		{
			const std::string &line = lines[i];
			bool visual = line.size() > 4 && line.size() <= 4 + 32 && line[0] == 'V' && line[1] == 'P' && line[2] >= 'A' && line[2] <= 'C' && line[3] == ':'
			    && line.find_first_not_of(s_NameCharacters, 4) == std::string::npos;
			if (!visual && !(isMadeOf(line, s_NameCharacters) && line.size() <= RtScenarioMaxIdSize))
				return invalid(".Equipment has the line " + quoteText(line) + ", which is not the equipment of an npc of a scenario");
		}
	}
	return true;
}

// ****************************************************************************
bool CRtScenarioChecker::checkEvent(const CObject *event, CActCounts &counts)
{
	if (!isTable(event))
		return invalid(" is " + describeValue(event) + ", a table is needed");

	// the event, and the action that holds the others when there are several
	if (!addPrimNodes(2))
		return false;
	++counts.Aliases;

	// an event without an id has no name in the AI service
	const std::string *eventId = getText(getAttrPtr(event, s_KeyId));
	if (eventId && !isAiName(*eventId))
		return invalid(".Id is " + quoteText(*eventId) + s_IdRule);

	// the type of the event is one the translator of the editor gives an event
	const std::string *eventType = getText(getAttrPtr(event, s_KeyEvent));
	if (eventType && !isOneOf(*eventType, s_EventTypes, sizeof(s_EventTypes) / sizeof(s_EventTypes[0])))
		return invalid(".Event is " + quoteText(*eventType) + ", which is not an event of a scenario");

	// the states and the groups the event is for: the service puts the prefix of the session before
	// each line
	if (!checkNameLines(event, s_KeyStatesByName, true) || !checkNameLines(event, s_KeyGroupsByName, false))
		return false;

	const CObject *actionIds = getAttrPtr(event, s_KeyActionsId);
	if (!actionIds)
		return true;
	if (!actionIds->isTable(s_Empty))
		return invalid(".ActionsId is " + describeValue(actionIds) + ", a table is needed");

	uint32 count = actionIds->getSize();
	for (uint32 i = 0; i < count; ++i)
	{
		const CObject *value = actionIds->getValueAtPos(i).getPtr();
		const std::string *actionId = getText(value);
		if (!actionId)
			return invalid(toString(".ActionsId[%u] is ", i) + describeValue(value) + ", the id of an action is needed");

		CIdEntry *action = findId(*actionId);
		if (!action)
			return invalid(": the action " + quoteText(*actionId) + " is not in the scenario");

		uint32 actions = 0;
		if (!checkAction(action->Table, actionId, action, actions))
			return false;

		if (actions > m_Limits.MaxActionsPerAct - std::min(counts.Actions, m_Limits.MaxActionsPerAct))
			return refuseAct(RtScenarioOverQuota, toString(" has more than %u actions", m_Limits.MaxActionsPerAct));
		counts.Actions += actions;
		if (!addPrimNodes(actions))
			return false;
	}
	return true;
}

// ****************************************************************************
bool CRtScenarioChecker::checkUserTriggers(const CObject *act)
{
	const CObject *userTriggers;
	if (!requireTable(act, s_KeyUserTriggers, userTriggers))
		return false;

	uint32 count = userTriggers->getSize();
	if (count > m_Limits.MaxUserTriggersPerAct)
		return refuseAct(RtScenarioOverQuota, toString(" has %u user triggers, the limit is %u", count, m_Limits.MaxUserTriggersPerAct));

	for (uint32 i = 0; i < count; ++i)
	{
		const CObject *userTrigger = userTriggers->getValueAtPos(i).getPtr();
		if (!isTable(userTrigger))
			invalid(" is " + describeValue(userTrigger) + ", a table is needed");
		else if (requireText(userTrigger, s_KeyName) && requireText(userTrigger, s_KeyGrp))
			continue;
		return locate(toString("Acts[%u].UserTriggers[%u]", m_ActIndex, i));
	}
	return true;
}

// ****************************************************************************
bool CRtScenarioChecker::checkAct(const CObject *act, CActCounts &counts)
{
	if (!isTable(act))
		return invalid(" is " + describeValue(act) + ", a table is needed");

	// The location is an index into Locations. The base act has no location of its own and the
	// editor leaves its index at 0, which is read like any other when the weather and the season of
	// an act are looked up: it must exist too.
	double locationId;
	const CObject *locationObject = getAttrPtr(act, s_KeyLocationId);
	if (!getWholeNumber(locationObject, locationId))
		return invalid(".LocationId is " + describeValue(locationObject) + ", a whole number is needed");
	if (locationId < 0.0 || locationId >= (double)m_LocationCount)
		return invalid(".LocationId is " + describeNumber(locationId) + toString(", the scenario has %u locations", m_LocationCount));

	if (!requireText(act, s_KeyName)
	    || !requireText(act, s_KeyActDescription)
	    || !requireText(act, s_KeyPreActDescription))
		return false;

	// the weather is read as the whole part of a number
	const CObject *weatherObject = getAttrPtr(act, s_KeyWeatherValue);
	if (weatherObject)
	{
		double weather;
		if (!getNumber(weatherObject, weather) || !isValidDouble(weather))
			return invalid(".WeatherValue is " + describeValue(weatherObject) + ", a number is needed");
		if (weather < 0.0 || weather >= (double)RtScenarioMaxWeatherValue + 1.0)
			return invalid(".WeatherValue is " + describeNumber(weather) + toString(", outside 0 to %u", RtScenarioMaxWeatherValue));
	}

	const CObject *npcs;
	if (!requireTable(act, s_KeyNpcs, npcs))
		return false;
	counts.Npcs = npcs->getSize();

	// the two managers of an act take an alias each
	counts.Aliases = 2;

	const CObject *states;
	if (!requireTable(act, s_KeyAiStates, states))
		return false;
	uint32 stateCount = states->getSize();
	if (stateCount > m_Limits.MaxStatesPerAct)
		return refuseAct(RtScenarioOverQuota, toString(" has %u states, the limit is %u", stateCount, m_Limits.MaxStatesPerAct));
	for (uint32 i = 0; i < stateCount; ++i)
	{
		if (!checkAiState(states->getValueAtPos(i).getPtr(), counts))
			return locate(toString("Acts[%u].AiStates[%u]", m_ActIndex, i));
	}

	const CObject *events = getAttrPtr(act, s_KeyEvents);
	if (events)
	{
		if (!events->isTable(s_Empty))
			return invalid(".Events is " + describeValue(events) + ", a table is needed");
		uint32 eventCount = events->getSize();
		if (eventCount > m_Limits.MaxEventsPerAct)
			return refuseAct(RtScenarioOverQuota, toString(" has %u events, the limit is %u", eventCount, m_Limits.MaxEventsPerAct));
		for (uint32 i = 0; i < eventCount; ++i)
		{
			if (!checkEvent(events->getValueAtPos(i).getPtr(), counts))
				return locate(toString("Acts[%u].Events[%u]", m_ActIndex, i));
		}
	}

	if (counts.Aliases > m_Limits.MaxAliasesPerAct)
		return invalid(toString(" needs %u aliases, the primitive file of an act has %u", counts.Aliases, m_Limits.MaxAliasesPerAct));

	return checkUserTriggers(act);
}

// ****************************************************************************
bool CRtScenarioChecker::checkActs(const CObject *root)
{
	const CObject *acts;
	if (!requireTable(root, s_KeyActs, acts))
		return locate("the scenario");

	uint32 count = acts->getSize();
	if (count == 0)
	{
		invalid(" is empty, a scenario has at least its base act");
		return locate("Acts");
	}
	if (count > m_Limits.MaxActs)
	{
		invalid(toString(" has %u acts, the limit is %u", count, m_Limits.MaxActs));
		return locate("Acts");
	}

	// The base act (index 0) stays loaded while one of the other acts runs: the limit is for the
	// content of the two together.
	CActCounts base;
	for (m_ActIndex = 0; m_ActIndex < count; ++m_ActIndex)
	{
		CActCounts counts;
		if (!checkAct(acts->getValueAtPos(m_ActIndex).getPtr(), counts))
			return locate(toString("Acts[%u]", m_ActIndex));

		if (m_ActIndex == 0)
			base = counts;

		uint32 npcs = counts.Npcs + (m_ActIndex != 0 ? base.Npcs : 0);
		if (npcs > m_Limits.MaxBotsPerAct)
			return refuseAct(RtScenarioOverQuota, toString(" has %u npcs with those of the base act, the limit is %u", npcs, m_Limits.MaxBotsPerAct));

		uint32 bots = counts.Bots + (m_ActIndex != 0 ? base.Bots : 0);
		if (bots > m_Limits.MaxBotsPerAct)
			return refuseAct(RtScenarioOverQuota, toString(" has %u bots with those of the base act, the limit is %u", bots, m_Limits.MaxBotsPerAct));
	}
	return true;
}

// ****************************************************************************
TRtScenarioVerdict CRtScenarioChecker::run(const CObject *root)
{
	if (!root)
	{
		invalid("the upload holds no run time scenario (it is empty, or its compressed data could not be read)");
		return m_Verdict;
	}
	if (!root->isTable(s_Empty))
	{
		invalid("the scenario is " + describeValue(root) + ", a table is needed");
		return m_Verdict;
	}

	if (!walkTree(root))
	{
		locate("the scenario");
		return m_Verdict;
	}

	// the locations come before the acts, which index them
	if (checkPlotItems(root)
	    && checkLocations(root)
	    && checkTexts(root)
	    && checkActs(root))
	{
		return RtScenarioOk;
	}
	return m_Verdict;
}

} // namespace

// ****************************************************************************
bool isRtScenarioActionType(const std::string &action)
{
	return std::binary_search(s_ActionTypes, s_ActionTypes + sizeof(s_ActionTypes) / sizeof(s_ActionTypes[0]), action, CNameLess());
}

// ****************************************************************************
bool isRtScenarioScriptFunction(const std::string &signature)
{
	return std::binary_search(s_ScriptFunctions, s_ScriptFunctions + sizeof(s_ScriptFunctions) / sizeof(s_ScriptFunctions[0]), signature, CNameLess());
}

// ****************************************************************************
const char *const *getRtScenarioScriptFunctions(uint32 &count)
{
	count = sizeof(s_ScriptFunctions) / sizeof(s_ScriptFunctions[0]);
	return s_ScriptFunctions;
}

// ****************************************************************************
bool checkRtScenarioScript(const std::string &script, bool isCondition, std::string &refusal)
{
	return checkScriptCalls(script, isCondition, refusal);
}

// ****************************************************************************
TRtScenarioVerdict checkRtScenario(const CObject *rtScenario, const CRtScenarioLimits &limits, std::string &error)
{
	CRtScenarioChecker checker(limits);
	TRtScenarioVerdict verdict = checker.run(rtScenario);
	error = checker.getError();
	return verdict;
}

// ****************************************************************************
TRtScenarioVerdict checkRtScenarioStartingAct(const CObject *rtScenario, uint32 startingAct, std::string &error)
{
	error.clear();
	const CObject *acts = isTable(rtScenario) ? getAttrPtr(rtScenario, s_KeyActs) : nullptr;
	if (!isTable(acts))
	{
		error = "the scenario has no table of acts";
		return RtScenarioInvalid;
	}

	uint32 count = acts->getSize();
	if (startingAct == 0 || startingAct >= count)
	{
		error = toString("the starting act is %u, the scenario has %u acts with its base act, which is never started", startingAct, count);
		return RtScenarioInvalid;
	}
	return RtScenarioOk;
}

} // namespace R2
