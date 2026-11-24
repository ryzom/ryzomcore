// NeL - MMORPG Framework <https://wiki.ryzom.dev/>
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

#include "std3d.h"
#include "nel/3d/glsl_block_struct_reads.h"

#include <cstring>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "nel/misc/common.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D {

namespace {

// The largest array of structs inside a struct that is written out element by element
const sint s_MaxStructArrayInStruct = 64;
// Structs do not contain themselves: a guard against a source that is not GLSL
const uint s_MaxStructDepth = 16;
// The longest value a function that is added may return, in characters
const size_t s_MaxAccessorValue = 16384;
// How deep reads may stand in the indices of reads
const uint s_MaxReadDepth = 32;

// What the names that are added begin with
const char *const s_FunctionPrefix = "nlBlockRead";
const char *const s_IndexPrefix = "nlBlockIndex";

// What may stand before the type of a declaration
const char *const s_Qualifiers[] = {
	"const", "in", "out", "inout", "uniform", "highp", "mediump", "lowp", "precise", "invariant", "flat",
	"smooth", "centroid", "attribute", "varying"
};
// The words that begin a statement, or stand where a type could
const char *const s_Statements[] = {
	"return", "else", "do", "case", "default", "discard", "break", "continue", "if", "for", "while",
	"switch", "layout"
};
// The types that are no structs
const char *const s_BasicTypes[] = {
	"float", "int", "uint", "bool",
	"vec2", "vec3", "vec4", "ivec2", "ivec3", "ivec4", "uvec2", "uvec3", "uvec4", "bvec2", "bvec3", "bvec4",
	"mat2", "mat3", "mat4", "mat2x2", "mat2x3", "mat2x4", "mat3x2", "mat3x3", "mat3x4", "mat4x2", "mat4x3", "mat4x4"
};

bool isIdentifierStart(char c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool isDigit(char c)
{
	return c >= '0' && c <= '9';
}

bool isLineEnd(char c)
{
	return c == '\n' || c == '\r';
}

bool isSpace(char c)
{
	return c == ' ' || c == '\t' || c == '\f' || c == '\v' || isLineEnd(c);
}

bool isBasicType(const std::string &type)
{
	for (size_t i = 0; i < sizeof(s_BasicTypes) / sizeof(s_BasicTypes[0]); ++i)
	{
		if (type == s_BasicTypes[i])
			return true;
	}
	return false;
}

// The line ends in a range of a text: "\n", "\r\n" and a "\r" alone each are one
uint countLineEnds(const std::string &s, size_t begin, size_t end)
{
	uint count = 0;
	for (size_t i = begin; i < end; ++i)
	{
		if (s[i] == '\n' || (s[i] == '\r' && !(i + 1 < s.size() && s[i + 1] == '\n')))
			++count;
	}
	return count;
}

struct CToken
{
	enum TKind
	{
		// white space or a comment
		Space,
		// a preprocessor line
		Directive,
		Identifier,
		Number,
		// one character of anything else
		Punctuation
	};
	TKind Kind;
	size_t Begin;
	size_t End;
};

// A field of a struct, or a member of a uniform block
struct CField
{
	std::string Type;
	std::string Name;
	bool IsArray;
	// the number of elements when the source has it as a plain number, -1 otherwise
	sint Size;
};

struct CStructDef
{
	std::vector<CField> Fields;
	// false for a struct whose fields were not understood, or that the source declares twice
	bool Known;
};

// A member of a uniform block that holds structs
struct CBlockMember
{
	CField Field;
	// the instance name of the block, empty when its members are global names
	std::string Instance;
	size_t Block;
};

// What follows the name of a block member in a read: an index or a field
struct CStep
{
	bool IsIndex;
	// the significant tokens of the brackets of an index
	size_t Open;
	size_t Close;
	std::string Field;
};

class CRewriter
{
public:
	CRewriter(const std::string &source);
	bool run(std::string &result, std::string &message);

private:
	void lex();
	void matchBrackets();
	void scanDeclarations();
	bool parseFields(size_t open, size_t close, std::vector<CField> &fields) const;
	sint dimensionSize(size_t open) const;
	void remarkDefines();
	void render(size_t a, size_t b, std::string &out, uint depth);
	bool isDeclaration(size_t k) const;
	bool isStructBody(size_t close) const;
	bool findAccessor(const CBlockMember &member, const std::vector<CStep> &steps, const std::string &type, std::string &name);
	bool construct(const std::string &type, const std::string &expr, std::string &out, uint depth);

	std::string text(size_t k) const { return m_Source.substr(m_Tokens[m_Sig[k]].Begin, m_Tokens[m_Sig[k]].End - m_Tokens[m_Sig[k]].Begin); }
	bool isIdentifier(size_t k) const { return k < m_Sig.size() && m_Tokens[m_Sig[k]].Kind == CToken::Identifier; }
	bool isIdentifier(size_t k, const char *word) const { return isIdentifier(k) && m_Source.compare(m_Tokens[m_Sig[k]].Begin, m_Tokens[m_Sig[k]].End - m_Tokens[m_Sig[k]].Begin, word) == 0; }
	bool isPunctuation(size_t k, char c) const { return k < m_Sig.size() && m_Tokens[m_Sig[k]].Kind == CToken::Punctuation && m_Source[m_Tokens[m_Sig[k]].Begin] == c; }
	bool isOneOf(size_t k, const char *const *words, size_t count) const;
	bool hasDirective(size_t a, size_t b) const;
	size_t tokenBegin(size_t k) const { return m_Tokens[m_Sig[k]].Begin; }
	size_t tokenEnd(size_t k) const { return m_Tokens[m_Sig[k]].End; }
	uint lineAt(size_t offset) const { return 1 + countLineEnds(m_Source, 0, offset); }
	uint line(size_t k) const { return lineAt(tokenBegin(k)); }
	void fail(const std::string &why);
	void remark(const std::string &what);

	const std::string &m_Source;
	std::vector<CToken> m_Tokens;
	// the tokens that are no space and no directive, as indices into m_Tokens
	std::vector<size_t> m_Sig;
	// for each significant token that is a bracket, the one it goes with
	std::vector<size_t> m_Match;
	// the significant tokens inside the declaration of a struct or of a uniform block
	std::vector<uint8> m_InDeclaration;
	// the source has a name that begins like the ones that are added
	bool m_PrefixTaken;
	std::map<std::string, CStructDef> m_Structs;
	// by "instance.member", with an empty instance for the blocks without one
	std::map<std::string, CBlockMember> m_Members;
	std::set<std::string> m_Instances;
	// the functions to add behind the declaration of each block
	std::vector<std::string> m_BlockAccessors;
	// the function that reads an expression, and the names that are given
	std::map<std::string, std::string> m_AccessorByExpr;
	std::set<std::string> m_AccessorNames;
	// the token that ends the declaration of a block, and the block
	std::map<size_t, size_t> m_BlockEnds;
	// where in the output the functions of a block go
	std::vector<std::pair<size_t, size_t>> m_Insertions;
	bool m_Failed;
	std::string m_Failure;
	std::set<std::string> m_RemarksMade;
	std::string m_Remarks;
};

CRewriter::CRewriter(const std::string &source)
    : m_Source(source)
    , m_PrefixTaken(false)
    , m_Failed(false)
{
}

bool CRewriter::isOneOf(size_t k, const char *const *words, size_t count) const
{
	for (size_t i = 0; i < count; ++i)
	{
		if (isIdentifier(k, words[i]))
			return true;
	}
	return false;
}

// True when a preprocessor line stands between the significant tokens a and b
bool CRewriter::hasDirective(size_t a, size_t b) const
{
	for (size_t i = m_Sig[a] + 1; i < m_Sig[b]; ++i)
	{
		if (m_Tokens[i].Kind == CToken::Directive)
			return true;
	}
	return false;
}

void CRewriter::fail(const std::string &why)
{
	if (!m_Failed)
	{
		m_Failed = true;
		m_Failure = why;
	}
}

void CRewriter::remark(const std::string &what)
{
	if (!m_RemarksMade.insert(what).second)
		return;
	if (!m_Remarks.empty())
		m_Remarks += "; ";
	m_Remarks += what;
}

void CRewriter::lex()
{
	const std::string &s = m_Source;
	const size_t n = s.size();
	const size_t functionPrefixSize = strlen(s_FunctionPrefix);
	const size_t indexPrefixSize = strlen(s_IndexPrefix);
	size_t i = 0;
	// a '#' is a directive when nothing but space is before it on its line
	bool lineStart = true;
	while (i < n)
	{
		CToken t;
		t.Begin = i;
		const char c = s[i];
		if (isSpace(c))
		{
			while (i < n && isSpace(s[i]))
			{
				if (isLineEnd(s[i]))
					lineStart = true;
				++i;
			}
			t.Kind = CToken::Space;
		}
		else if (c == '/' && i + 1 < n && s[i + 1] == '/')
		{
			while (i < n && !isLineEnd(s[i]))
				++i;
			t.Kind = CToken::Space;
		}
		else if (c == '/' && i + 1 < n && s[i + 1] == '*')
		{
			const size_t close = s.find("*/", i + 2);
			if (close == std::string::npos)
			{
				fail(NLMISC::toString("line %u: a comment is not closed", lineAt(i)));
				return;
			}
			i = close + 2;
			t.Kind = CToken::Space;
		}
		else if (c == '#' && lineStart)
		{
			// up to the end of the line, and of the next one where a backslash ends the line
			while (i < n)
			{
				if (isLineEnd(s[i]))
				{
					if (s[i - 1] != '\\')
						break;
					if (s[i] == '\r' && i + 1 < n && s[i + 1] == '\n')
						++i;
				}
				++i;
			}
			t.Kind = CToken::Directive;
			lineStart = false;
			if (s.find(s_FunctionPrefix, t.Begin) < i || s.find(s_IndexPrefix, t.Begin) < i)
				m_PrefixTaken = true;
		}
		else if (isIdentifierStart(c))
		{
			while (i < n && (isIdentifierStart(s[i]) || isDigit(s[i])))
				++i;
			t.Kind = CToken::Identifier;
			lineStart = false;
			if (s.compare(t.Begin, functionPrefixSize, s_FunctionPrefix) == 0 || s.compare(t.Begin, indexPrefixSize, s_IndexPrefix) == 0)
				m_PrefixTaken = true;
		}
		else if (isDigit(c) || (c == '.' && i + 1 < n && isDigit(s[i + 1])))
		{
			const bool hex = c == '0' && i + 1 < n && (s[i + 1] == 'x' || s[i + 1] == 'X');
			++i;
			while (i < n)
			{
				const char d = s[i];
				if (isIdentifierStart(d) || isDigit(d) || d == '.')
					++i;
				else if ((d == '+' || d == '-') && !hex && (s[i - 1] == 'e' || s[i - 1] == 'E'))
					++i;
				else
					break;
			}
			t.Kind = CToken::Number;
			lineStart = false;
		}
		else
		{
			++i;
			t.Kind = CToken::Punctuation;
			lineStart = false;
		}
		t.End = i;
		if (t.Kind != CToken::Space && t.Kind != CToken::Directive)
			m_Sig.push_back(m_Tokens.size());
		m_Tokens.push_back(t);
	}
}

void CRewriter::matchBrackets()
{
	const size_t n = m_Sig.size();
	m_Match.assign(n, 0);
	std::vector<size_t> open;
	for (size_t k = 0; k < n; ++k)
	{
		if (m_Tokens[m_Sig[k]].Kind != CToken::Punctuation)
			continue;
		const char c = m_Source[tokenBegin(k)];
		if (c == '(' || c == '[' || c == '{')
		{
			open.push_back(k);
		}
		else if (c == ')' || c == ']' || c == '}')
		{
			const char oc = open.empty() ? '\0' : m_Source[tokenBegin(open.back())];
			if ((c == ')' && oc != '(') || (c == ']' && oc != '[') || (c == '}' && oc != '{'))
			{
				fail(NLMISC::toString("line %u: '%c' closes no bracket", line(k), c));
				return;
			}
			m_Match[open.back()] = k;
			m_Match[k] = open.back();
			open.pop_back();
		}
	}
	if (!open.empty())
		fail(NLMISC::toString("line %u: '%c' is not closed", line(open.back()), m_Source[tokenBegin(open.back())]));
}

sint CRewriter::dimensionSize(size_t open) const
{
	if (m_Match[open] != open + 2 || m_Tokens[m_Sig[open + 1]].Kind != CToken::Number)
		return -1;
	const std::string digits = text(open + 1);
	if (digits.size() > 6)
		return -1;
	sint value = 0;
	for (size_t i = 0; i < digits.size(); ++i)
	{
		if (!isDigit(digits[i]))
			return -1;
		value = value * 10 + (digits[i] - '0');
	}
	// a number that starts with 0 is octal
	if (digits.size() > 1 && digits[0] == '0')
		return -1;
	return value;
}

// The fields between the braces of a struct or of a uniform block
bool CRewriter::parseFields(size_t open, size_t close, std::vector<CField> &fields) const
{
	size_t k = open + 1;
	while (k < close)
	{
		// what may stand before the type
		for (;;)
		{
			if (isIdentifier(k, "layout") && isPunctuation(k + 1, '('))
				k = m_Match[k + 1] + 1;
			else if (isIdentifier(k, "highp") || isIdentifier(k, "mediump") || isIdentifier(k, "lowp") || isIdentifier(k, "precise") || isIdentifier(k, "const"))
				++k;
			else
				break;
		}
		if (k >= close)
			break;
		if (!isIdentifier(k) || isIdentifier(k, "struct"))
			return false;
		CField type;
		type.Type = text(k);
		type.IsArray = false;
		type.Size = -1;
		++k;
		if (k < close && isPunctuation(k, '['))
		{
			type.IsArray = true;
			type.Size = dimensionSize(k);
			k = m_Match[k] + 1;
		}
		for (;;)
		{
			if (k >= close || !isIdentifier(k))
				return false;
			CField field = type;
			field.Name = text(k);
			++k;
			if (k < close && isPunctuation(k, '['))
			{
				// an array of arrays is not followed
				if (field.IsArray)
					return false;
				field.IsArray = true;
				field.Size = dimensionSize(k);
				k = m_Match[k] + 1;
			}
			if (k < close && isPunctuation(k, '['))
				return false;
			fields.push_back(field);
			if (k < close && isPunctuation(k, ','))
			{
				++k;
				continue;
			}
			if (k < close && isPunctuation(k, ';'))
			{
				++k;
				break;
			}
			return false;
		}
	}
	return true;
}

// The structs and the uniform blocks that the source declares at its top level
void CRewriter::scanDeclarations()
{
	const size_t n = m_Sig.size();
	m_InDeclaration.assign(n, 0);
	for (size_t k = 0; k < n; ++k)
	{
		if (isPunctuation(k, '{') || isPunctuation(k, '(') || isPunctuation(k, '['))
		{
			// the body of a function, or brackets of anything else
			k = m_Match[k];
			continue;
		}
		if (isIdentifier(k, "struct") && isIdentifier(k + 1) && isPunctuation(k + 2, '{'))
		{
			const size_t open = k + 2;
			const size_t close = m_Match[open];
			const std::string name = text(k + 1);
			CStructDef def;
			// with a preprocessor line between its braces, the fields of a struct depend on it
			def.Known = parseFields(open, close, def.Fields) && !def.Fields.empty() && !hasDirective(open, close);
			if (m_Structs.count(name))
				def.Known = false;
			m_Structs[name] = def;
			for (size_t i = open; i <= close; ++i)
				m_InDeclaration[i] = 1;
			k = close;
			continue;
		}
		if (isIdentifier(k, "uniform") && isIdentifier(k + 1) && isPunctuation(k + 2, '{'))
		{
			const size_t open = k + 2;
			const size_t close = m_Match[open];
			const std::string blockName = text(k + 1);
			for (size_t i = open; i <= close; ++i)
				m_InDeclaration[i] = 1;
			std::vector<CField> fields;
			bool understood = parseFields(open, close, fields) && !hasDirective(open, close);
			std::string instance;
			size_t end = close + 1;
			if (isIdentifier(end))
			{
				instance = text(end);
				m_InDeclaration[end] = 1;
				++end;
				// an array of blocks is not followed
				if (isPunctuation(end, '['))
					understood = false;
			}
			while (end < n && !isPunctuation(end, ';'))
				++end;
			if (end >= n)
			{
				fail(NLMISC::toString("line %u: the uniform block '%s' has no end", line(k), blockName.c_str()));
				return;
			}
			if (!understood)
			{
				remark(NLMISC::toString("line %u: the uniform block '%s' is not understood, reads of structs out of it are not rewritten", line(k), blockName.c_str()));
				k = end;
				continue;
			}
			const size_t block = m_BlockAccessors.size();
			bool holdsStructs = false;
			for (size_t i = 0; i < fields.size(); ++i)
			{
				if (!m_Structs.count(fields[i].Type))
				{
					// a type that is a name of the preprocessor may be a struct
					if (!isBasicType(fields[i].Type))
						remark(NLMISC::toString("line %u: the type '%s' of '%s' in the uniform block '%s' is not known, reads of it are not rewritten", line(k), fields[i].Type.c_str(), fields[i].Name.c_str(), blockName.c_str()));
					continue;
				}
				const std::string key = instance + "." + fields[i].Name;
				if (m_Members.count(key))
				{
					// two blocks of which the preprocessor leaves one
					fail(NLMISC::toString("line %u: the uniform block '%s' declares '%s', which another uniform block declares too", line(k), blockName.c_str(), fields[i].Name.c_str()));
					return;
				}
				CBlockMember member;
				member.Field = fields[i];
				member.Instance = instance;
				member.Block = block;
				m_Members[key] = member;
				holdsStructs = true;
			}
			if (holdsStructs)
			{
				m_BlockAccessors.push_back(std::string());
				m_BlockEnds[end] = block;
				if (!instance.empty())
					m_Instances.insert(instance);
			}
			k = end;
			continue;
		}
	}
}

// True when the '}' at token close ends the body of a struct: "struct { ... }" or "struct T { ... }"
bool CRewriter::isStructBody(size_t close) const
{
	const size_t open = m_Match[close];
	return (open >= 1 && isIdentifier(open - 1, "struct")) || (open >= 2 && isIdentifier(open - 1) && isIdentifier(open - 2, "struct"));
}

// True when the name at token k is being declared: as a variable, a parameter, a function or a type
bool CRewriter::isDeclaration(size_t k) const
{
	const size_t qualifierCount = sizeof(s_Qualifiers) / sizeof(s_Qualifiers[0]);
	const size_t statementCount = sizeof(s_Statements) / sizeof(s_Statements[0]);
	if (k == 0)
		return false;
	const size_t before = k - 1;
	// "struct name"
	if (isIdentifier(before, "struct"))
		return true;
	// "struct T { ... } name"
	if (isPunctuation(before, '}'))
		return isStructBody(before);
	// "Type name" and "Type[2] name"
	size_t typeToken = before;
	if (isPunctuation(typeToken, ']') && m_Match[typeToken] > 0)
		typeToken = m_Match[typeToken] - 1;
	if (isIdentifier(typeToken))
		return !isOneOf(typeToken, s_Qualifiers, qualifierCount) && !isOneOf(typeToken, s_Statements, statementCount);
	if (!isPunctuation(before, ','))
		return false;

	// A later name of a declaration, "Type a, name": back to where the statement begins
	size_t start = 0;
	size_t j = before;
	while (j > 0)
	{
		--j;
		if (isPunctuation(j, ']'))
		{
			j = m_Match[j];
			continue;
		}
		if (isPunctuation(j, ')'))
		{
			// behind the condition of a statement begins the statement it governs
			const size_t open = m_Match[j];
			if (open > 0 && (isIdentifier(open - 1, "if") || isIdentifier(open - 1, "while") || isIdentifier(open - 1, "for") || isIdentifier(open - 1, "switch")))
			{
				start = j + 1;
				break;
			}
			j = open;
			continue;
		}
		if (isPunctuation(j, '}'))
		{
			// "struct T { ... } a, name"
			if (isStructBody(j))
				return true;
			start = j + 1;
			break;
		}
		if (isPunctuation(j, ';') || isPunctuation(j, '{') || isPunctuation(j, ':') || isIdentifier(j, "else") || isIdentifier(j, "do"))
		{
			start = j + 1;
			break;
		}
		if (isPunctuation(j, '['))
			return false;
		if (isPunctuation(j, '('))
		{
			// the arguments of a call, or the first part of a for statement
			if (!(j > 0 && isIdentifier(j - 1, "for")))
				return false;
			start = j + 1;
			break;
		}
	}
	for (;;)
	{
		if (isIdentifier(start, "layout") && isPunctuation(start + 1, '('))
			start = m_Match[start + 1] + 1;
		else if (isOneOf(start, s_Qualifiers, qualifierCount))
			++start;
		else
			break;
	}
	if (!isIdentifier(start) || isOneOf(start, s_Statements, statementCount))
		return false;
	size_t first = start + 1;
	if (isPunctuation(first, '['))
		first = m_Match[first] + 1;
	return isIdentifier(first);
}

// A value of a struct type, written as its constructor over the members of expr
bool CRewriter::construct(const std::string &type, const std::string &expr, std::string &out, uint depth)
{
	std::map<std::string, CStructDef>::const_iterator it = m_Structs.find(type);
	if (it == m_Structs.end() || !it->second.Known)
	{
		fail(NLMISC::toString("the fields of the struct '%s' are not understood", type.c_str()));
		return false;
	}
	if (depth > s_MaxStructDepth || out.size() > s_MaxAccessorValue)
	{
		fail(NLMISC::toString("the struct '%s' has too many structs inside it to write a read of it out", type.c_str()));
		return false;
	}
	const std::vector<CField> &fields = it->second.Fields;
	out += type;
	out += "(";
	for (size_t i = 0; i < fields.size(); ++i)
	{
		const CField &field = fields[i];
		if (i != 0)
			out += ", ";
		const std::string fieldExpr = expr + "." + field.Name;
		if (!m_Structs.count(field.Type))
		{
			// a number, a vector, a matrix, or an array of those
			out += fieldExpr;
		}
		else if (!field.IsArray)
		{
			if (!construct(field.Type, fieldExpr, out, depth + 1))
				return false;
		}
		else
		{
			if (field.Size <= 0 || field.Size > s_MaxStructArrayInStruct)
			{
				fail(NLMISC::toString("the field '%s' of the struct '%s' is an array of structs whose size is no number from 1 to %d", field.Name.c_str(), type.c_str(), (sint)s_MaxStructArrayInStruct));
				return false;
			}
			out += field.Type + "[" + NLMISC::toString(field.Size) + "](";
			for (sint e = 0; e < field.Size; ++e)
			{
				if (e != 0)
					out += ", ";
				if (!construct(field.Type, fieldExpr + "[" + NLMISC::toString(e) + "]", out, depth + 1))
					return false;
			}
			out += ")";
		}
	}
	out += ")";
	return true;
}

// The function that reads the struct at the end of the steps, made when there is none yet
bool CRewriter::findAccessor(const CBlockMember &member, const std::vector<CStep> &steps, const std::string &type, std::string &name)
{
	std::vector<std::string> parts;
	if (!member.Instance.empty())
		parts.push_back(member.Instance);
	parts.push_back(member.Field.Name);
	std::string expr = member.Instance.empty() ? member.Field.Name : member.Instance + "." + member.Field.Name;
	std::string params;
	uint indices = 0;
	for (size_t i = 0; i < steps.size(); ++i)
	{
		if (steps[i].IsIndex)
		{
			const std::string index = s_IndexPrefix + NLMISC::toString(indices);
			++indices;
			expr += "[" + index + "]";
			if (!params.empty())
				params += ", ";
			params += "int " + index;
		}
		else
		{
			parts.push_back(steps[i].Field);
			expr += "." + steps[i].Field;
		}
	}
	std::map<std::string, std::string>::const_iterator known = m_AccessorByExpr.find(expr);
	if (known != m_AccessorByExpr.end())
	{
		name = known->second;
		return true;
	}
	if (m_PrefixTaken)
	{
		fail(NLMISC::toString("the source has a name that begins with '%s' or '%s', as the names do that the rewrite adds", s_FunctionPrefix, s_IndexPrefix));
		return false;
	}

	// The name is the path of the read, without the underscores its names begin or end with and
	// without two of them in a row: WebGL allows no name with two underscores in a row
	std::string base = s_FunctionPrefix;
	for (size_t i = 0; i < parts.size(); ++i)
	{
		std::string part;
		for (size_t c = 0; c < parts[i].size(); ++c)
		{
			if (parts[i][c] != '_' || (!part.empty() && part[part.size() - 1] != '_'))
				part += parts[i][c];
		}
		while (!part.empty() && part[part.size() - 1] == '_')
			part.resize(part.size() - 1);
		if (!part.empty())
			base += "_" + part;
	}
	// two paths may give the same name ("a.b_c" and "a_b.c"): the later one gets a number
	name = base;
	for (uint number = 2; m_AccessorNames.count(name); ++number)
		name = base + "_" + NLMISC::toString(number);

	std::string value;
	if (!construct(type, expr, value, 0))
		return false;
	m_AccessorNames.insert(name);
	m_AccessorByExpr[expr] = name;
	m_BlockAccessors[member.Block] += " " + type + " " + name + "(" + params + ") { return " + value + "; }";
	return true;
}

// The text from token a up to token b, with the reads of whole structs in it rewritten
void CRewriter::render(size_t a, size_t b, std::string &out, uint depth)
{
	if (a >= b)
		return;
	size_t copyFrom = tokenBegin(a);
	size_t k = a;
	while (k < b && !m_Failed)
	{
		std::map<size_t, size_t>::const_iterator blockEnd = m_BlockEnds.find(k);
		if (blockEnd != m_BlockEnds.end())
		{
			// the functions of the block go behind its declaration, on the same line
			out.append(m_Source, copyFrom, tokenEnd(k) - copyFrom);
			copyFrom = tokenEnd(k);
			m_Insertions.push_back(std::make_pair(out.size(), blockEnd->second));
			++k;
			continue;
		}
		if (!isIdentifier(k) || m_InDeclaration[k] || (k > 0 && isPunctuation(k - 1, '.')))
		{
			++k;
			continue;
		}

		// A name that is a block member holding structs, or the instance name of its block
		const std::string word = text(k);
		std::map<std::string, CBlockMember>::const_iterator found = m_Members.find("." + word);
		size_t memberToken = k;
		if (found == m_Members.end() && m_Instances.count(word))
		{
			if (isDeclaration(k))
			{
				fail(NLMISC::toString("line %u: '%s' is the instance name of a uniform block and is declared here as something else", line(k), word.c_str()));
				return;
			}
			if (isPunctuation(k + 1, '.') && isIdentifier(k + 2))
			{
				found = m_Members.find(word + "." + text(k + 2));
				memberToken = k + 2;
			}
			else
			{
				remark(NLMISC::toString("line %u: the instance name '%s' of a uniform block stands without a member behind it, a read of a struct through it is not rewritten", line(k), word.c_str()));
			}
		}
		if (found == m_Members.end())
		{
			++k;
			continue;
		}
		const CBlockMember &member = found->second;
		if (isDeclaration(k))
		{
			fail(NLMISC::toString("line %u: '%s' is a member of a uniform block and is declared here as something else", line(k), word.c_str()));
			return;
		}

		// Follow the indices and the fields behind the name
		std::vector<CStep> steps;
		std::string type = member.Field.Type;
		bool isArray = member.Field.IsArray;
		size_t pos = memberToken + 1;
		bool wholeStruct = false;
		for (;;)
		{
			if (isArray)
			{
				if (isPunctuation(pos, '['))
				{
					CStep step;
					step.IsIndex = true;
					step.Open = pos;
					step.Close = m_Match[pos];
					steps.push_back(step);
					pos = step.Close + 1;
					isArray = false;
					continue;
				}
				// the array as a whole: its length, or all of its structs
				if (m_Structs.count(type) && !(isPunctuation(pos, '.') && isIdentifier(pos + 1, "length")))
					remark(NLMISC::toString("line %u: an array of structs of '%s' is read as a whole, which is not rewritten", line(k), word.c_str()));
				break;
			}
			std::map<std::string, CStructDef>::const_iterator def = m_Structs.find(type);
			if (def == m_Structs.end())
				break; // a number, a vector or a matrix: a read of a member
			if (!(isPunctuation(pos, '.') && isIdentifier(pos + 1)))
			{
				wholeStruct = true;
				break;
			}
			if (!def->second.Known)
			{
				fail(NLMISC::toString("line %u: the fields of the struct '%s' are not understood", line(pos), type.c_str()));
				return;
			}
			const std::string fieldName = text(pos + 1);
			const CField *field = nullptr;
			for (size_t i = 0; i < def->second.Fields.size(); ++i)
			{
				if (def->second.Fields[i].Name == fieldName)
					field = &def->second.Fields[i];
			}
			if (!field)
			{
				fail(NLMISC::toString("line %u: the struct '%s' has no field '%s'", line(pos + 1), type.c_str(), fieldName.c_str()));
				return;
			}
			CStep step;
			step.IsIndex = false;
			step.Open = 0;
			step.Close = 0;
			step.Field = fieldName;
			steps.push_back(step);
			type = field->Type;
			isArray = field->IsArray;
			pos += 2;
		}
		if (!wholeStruct)
		{
			// the tokens of its indices are looked at as the loop goes on
			++k;
			continue;
		}

		// The read is the tokens k to pos - 1
		if (hasDirective(k, pos - 1))
		{
			fail(NLMISC::toString("line %u: a preprocessor line stands inside the read of a struct of '%s'", line(k), word.c_str()));
			return;
		}
		if (depth >= s_MaxReadDepth)
		{
			fail(NLMISC::toString("line %u: reads of structs stand too deep inside the indices of each other", line(k)));
			return;
		}
		std::string name;
		if (!findAccessor(member, steps, type, name))
			return;
		std::string call = name + "(";
		bool firstIndex = true;
		for (size_t i = 0; i < steps.size(); ++i)
		{
			if (!steps[i].IsIndex)
				continue;
			if (steps[i].Close == steps[i].Open + 1)
			{
				fail(NLMISC::toString("line %u: an index of '%s' is empty", line(steps[i].Open), word.c_str()));
				return;
			}
			if (!firstIndex)
				call += ", ";
			firstIndex = false;
			// an index with a comma in it is one expression, and would be two arguments
			bool comma = false;
			for (size_t t = steps[i].Open + 1; t < steps[i].Close; ++t)
			{
				if (isPunctuation(t, '(') || isPunctuation(t, '[') || isPunctuation(t, '{'))
					t = m_Match[t];
				else if (isPunctuation(t, ','))
					comma = true;
			}
			// the index may be unsigned: the parameter is an int
			call += comma ? "int((" : "int(";
			render(steps[i].Open + 1, steps[i].Close, call, depth + 1);
			call += comma ? "))" : ")";
		}
		call += ")";
		if (m_Failed)
			return;
		// the line ends that stood between the tokens of the read, so that the lines keep their numbers
		const uint lineEnds = countLineEnds(m_Source, tokenBegin(k), tokenEnd(pos - 1));
		const uint lineEndsKept = countLineEnds(call, 0, call.size());
		for (uint i = lineEndsKept; i < lineEnds; ++i)
			call += "\n";
		out.append(m_Source, copyFrom, tokenBegin(k) - copyFrom);
		out += call;
		copyFrom = tokenEnd(pos - 1);
		k = pos;
	}
	if (!m_Failed)
		out.append(m_Source, copyFrom, tokenEnd(b - 1) - copyFrom);
}

// The body of a #define is not looked into: one that names a block member that holds structs, or
// the instance name of its block, is said
void CRewriter::remarkDefines()
{
	std::set<std::string> names;
	for (std::map<std::string, CBlockMember>::const_iterator it = m_Members.begin(); it != m_Members.end(); ++it)
		names.insert(it->second.Instance.empty() ? it->second.Field.Name : it->second.Instance);
	for (size_t i = 0; i < m_Tokens.size(); ++i)
	{
		if (m_Tokens[i].Kind != CToken::Directive)
			continue;
		const std::string directive = m_Source.substr(m_Tokens[i].Begin, m_Tokens[i].End - m_Tokens[i].Begin);
		size_t p = 1;
		while (p < directive.size() && (directive[p] == ' ' || directive[p] == '\t'))
			++p;
		if (directive.compare(p, 6, "define") != 0)
			continue;
		for (std::set<std::string>::const_iterator it = names.begin(); it != names.end(); ++it)
		{
			const std::string &name = *it;
			size_t at = directive.find(name, p + 6);
			while (at != std::string::npos)
			{
				const bool wordBefore = isIdentifierStart(directive[at - 1]) || isDigit(directive[at - 1]);
				const size_t after = at + name.size();
				const bool wordAfter = after < directive.size() && (isIdentifierStart(directive[after]) || isDigit(directive[after]));
				if (!wordBefore && !wordAfter)
				{
					remark(NLMISC::toString("line %u: a #define names '%s', reads of structs in its body are not rewritten", lineAt(m_Tokens[i].Begin), name.c_str()));
					break;
				}
				at = directive.find(name, after);
			}
		}
	}
}

bool CRewriter::run(std::string &result, std::string &message)
{
	result = m_Source;
	message.clear();

	lex();
	if (!m_Failed)
		matchBrackets();
	if (!m_Failed)
		scanDeclarations();
	if (m_Failed)
	{
		message = m_Failure;
		return false;
	}
	if (m_Members.empty())
	{
		// no uniform block holds structs
		message = m_Remarks;
		return message.empty();
	}

	remarkDefines();

	std::string out;
	out.append(m_Source, 0, tokenBegin(0));
	render(0, m_Sig.size(), out, 0);
	if (m_Failed)
	{
		message = m_Failure;
		return false;
	}
	out.append(m_Source, tokenEnd(m_Sig.size() - 1), std::string::npos);
	for (size_t i = m_Insertions.size(); i > 0; --i)
		out.insert(m_Insertions[i - 1].first, m_BlockAccessors[m_Insertions[i - 1].second]);
	result.swap(out);
	message = m_Remarks;
	return message.empty();
}

} // namespace

bool rewriteGlslBlockStructReads(const std::string &source, std::string &result, std::string &message)
{
	// a source without a struct has nothing to rewrite
	if (source.find("struct") == std::string::npos)
	{
		result = source;
		message.clear();
		return true;
	}
	CRewriter rewriter(source);
	return rewriter.run(result, message);
}

} // NL3D
