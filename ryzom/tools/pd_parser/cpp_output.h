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

#ifndef RY_PD_CPP_OUTPUT_H
#define RY_PD_CPP_OUTPUT_H

#include <nel/misc/types_nl.h>
#include <nel/misc/debug.h>
#include <nel/misc/file.h>
#include <nel/misc/path.h>
#include <nel/misc/algo.h>

#include <string>

inline std::string	lcFirst(std::string str)
{
	if (!str.empty() && str[0]>='A' && str[0]<='Z')
		str[0] += 'a'-'A';
	return str;
}

inline std::string	padTab(std::string str, uint sz)
{
	uint	i;
	uint	csize = 0;

	for (i=0; i<str.size(); ++i)
	{
		if (str[i] == '\t')
			csize = (csize+4)&0xfffc;
		else
			++csize;
	}

	while (csize < sz-4)
	{
		str += '\t';
		csize += 4;
	}
	str += '\t';
	return str;
}

inline std::string	removeDefaultArgs(const std::string& s)
{
	std::string	res;

	uint	i;
	bool	foundEqual = false;
	for (i=0; i<s.size(); ++i)
	{
		if (s[i] == '=')
			foundEqual = true;
		if (foundEqual && s[i] == ',')
			foundEqual = false;
		if (!foundEqual)
			res += s[i];
	}

	return res;
}

class CCppOutput;

class CClassGenerator
{
public:

	enum TSection
	{
		Public = 0,
		Protected = 1,
		Private = 2
	};

	struct SMethod
	{
		SMethod() : IsConst(false), IsInline(false), IsStatic(false), IsVirtual(false), IsAttribute(false), IsSeparator(false)	{}
		std::string	Type;
		std::string	Name;
		std::string	Proto;
		std::string	Body;
		std::string	ConstructorInit;
		std::string	Description;
		bool		IsConst;
		bool		IsInline;
		bool		IsStatic;
		bool		IsVirtual;
		bool		IsAttribute;
		bool		IsSeparator;
	};

	struct SSection
	{
		TSection				Type;
		std::string				Name;
		std::vector<SMethod>	Methods;
		std::string				DisplayName;
		std::string				Description;
		std::string				Other;
	};

	class SMethodId
	{
	public:
		SMethodId(sint section=-1, sint method=-1, CClassGenerator *generator=NULL) : Section(section), Method(method), Generator(generator)	{}

		sint					Section;
		sint					Method;
		CClassGenerator			*Generator;

		void					add(const std::string &instruction);
		void					setDescription(const std::string &description);

	};

	/// name
	std::string					Name;

	///
	std::string					Inherit;

	/// sections
	std::vector<SSection>		Sections;

	/// in body
	std::string					Body;

	///
	sint						CurrentSection;

	///
	SMethodId					CurrentMethod;

	///
	bool						StartInline;

	void			init(const std::string &name)
	{
		clear();
		Name = name;
	}

	void			clear()
	{
		Name.clear();
		Sections.clear();
		Body.clear();
		Inherit.clear();
		CurrentSection = -1;
	}


	void			setSection(const std::string &name)
	{
		uint	i;
		for (i=0; i<Sections.size(); ++i)
		{
			if (Sections[i].Name == name)
			{
				CurrentSection = i;
				return;
			}
		}
	}

	void			createSection(TSection type, const std::string &name, const std::string &displayname=std::string(""), const std::string &description=std::string(""))
	{
		SSection	newSection;
		newSection.Type = type;
		newSection.Name = name;
		newSection.DisplayName = displayname;
		newSection.Description = description;
		CurrentSection = (sint)Sections.size();
		Sections.push_back(newSection);
	}

	void			createPublic(const std::string &name, const std::string &displayname=std::string(""), const std::string &description=std::string(""))
	{
		createSection(Public, name, displayname, description);
	}

	void			createProtected(const std::string &name, const std::string &displayname=std::string(""), const std::string &description=std::string(""))
	{
		createSection(Protected, name, displayname, description);
	}

	void			createPrivate(const std::string &name, const std::string &displayname=std::string(""), const std::string &description=std::string(""))
	{
		createSection(Private, name, displayname, description);
	}



	SMethodId		startMethod(const std::string &type,
								const std::string &name,
								const std::string &proto,
								const std::string &section=std::string(""),
								bool isConst=false,
								bool isInline=true,
								bool isStatic=false,
								bool isAttribute = false,
								const std::string &init=std::string(""),
								bool isVirtual = false,
								bool isSep = false)
	{
		if (!section.empty())
			setSection(section);

		SSection	&_section = Sections[CurrentSection];

		SMethod		newMethod;

		newMethod.Name = name;
		newMethod.Type = type;
		newMethod.Proto = proto;
		newMethod.IsConst = isConst;
		newMethod.IsInline = isInline;
		newMethod.IsStatic = isStatic;
		newMethod.IsVirtual = isVirtual;
		newMethod.IsAttribute = isAttribute;
		newMethod.ConstructorInit = init;
		newMethod.IsSeparator = isSep;

		_section.Methods.push_back(newMethod);

		CurrentMethod = SMethodId(CurrentSection, (sint)_section.Methods.size()-1, this);

		return CurrentMethod;
	}

	SMethod			&get(SMethodId id)
	{
		return Sections[id.Section].Methods[id.Method];
	}

	SMethodId		startConstructor(const std::string &proto, const std::string &section=std::string(""), bool isInline=true, const std::string &init=std::string(""))
	{
		return startMethod("", "", proto, section, false, isInline, false, false, init);
	}

	SMethodId		startDestructor(const std::string &section=std::string(""), bool isInline=true, bool isVirtual=false)
	{
		return startMethod("", "~", "", section, false, isInline, false, false, "", isVirtual);
	}

	SMethodId		startRaw(const std::string &section=std::string(""), bool isInline=true)
	{
		return startMethod("", "", "", section, false, isInline, false, true, "", false, true);
	}

	void			separator(const std::string &section=std::string(""))
	{
		startMethod("", "", "", section, false, false, false, false, "", false, true);
	}

	void			add(const std::string &instruction, SMethodId methodId = SMethodId())
	{
		if (methodId.Method == -1)
			methodId = CurrentMethod;

		if (instruction.empty())
			return;

		Sections[methodId.Section].Methods[methodId.Method].Body += instruction+"\n";
	}

	void			setDescription(const std::string &description, SMethodId methodId = SMethodId())
	{
		if (methodId.Method == -1)
			methodId = CurrentMethod;

		Sections[methodId.Section].Methods[methodId.Method].Description += description;
	}

	void			addOther(const std::string &other, const std::string &section=std::string(""))
	{
		if (!section.empty())
			setSection(section);

		SSection	&_section = Sections[CurrentSection];

		_section.Other += other;
	}

	void			addAttribute(const std::string &type, const std::string &name, const std::string &section=std::string(""), bool isStatic=false, const std::string& value="", bool isFuncPtr=false, const std::string funcPtrProto="")
	{
		startMethod(type, name, funcPtrProto, section, false, false, isStatic, true, value, isFuncPtr);
	}


	void			flush(CCppOutput &hFile, CCppOutput &cppFile, CCppOutput &hInlineFile);

};








class CFunctionGenerator
{
public:

	CFunctionGenerator() : IsInline(false)	{}

	/// function name
	std::string	Name;

	/// return type
	std::string	Type;

	/// function prototype
	std::string	Proto;

	/// function body
	std::string	Body;

	/// is an inlined function
	bool		IsInline;

	/// function description
	std::string	Description;

	void			init(const std::string &name)
	{
		clear();
		Name = name;
	}

	void			setType(const std::string &type)
	{
		Type = type;
	}

	void			setProto(const std::string &proto)
	{
		Proto = proto;
	}

	void			clear()
	{
		Name.clear();
		Type.clear();
		Proto.clear();
		Body.clear();
		IsInline = false;
	}

	void			add(const std::string &instruction)
	{
		if (!instruction.empty())
			Body += instruction+"\n";
	}


	void			flush(CCppOutput &hFile, CCppOutput &cppFile, CCppOutput &hInlineFile);

};









class CCppOutput
{
public:

	/// Constructor
	CCppOutput(bool cppmode = true) :	_CppMode(cppmode), 
										_XmlMode(false), _NewLine(true), _Indent(0), _DescriptionMode(false), _Clean(true),
										_XmlInNode(false), _XmlLastSlash(false), _XmlGetNodeName(false), _XmlCloseNode(false), _XmlRootNode(false) {}

	/// Clear
	void	clear();

	/// Set Xml Mode
	void	setXmlMode()	{ _XmlMode = true; _NewLine = false; }

	/// Flush to file
	void	flush(const std::string &fileName);

	/// Force indent
	void	indent();

	/// Force unindent
	void	unindent();

	/// Stream
	template<typename T>
	CCppOutput	& operator << (const T& t)
	{
		addToStream(NLMISC::toString(t).c_str());
		return *this;
	}

	/// file description
	void	setFileHeader(const std::string& filename,  const std::string& description)
	{
		if (_Clean)
		{
			*this << "/** \\file " << filename << "\n";
			if (!description.empty())
				*this << description << "\n";
			*this << "\n";
			*this << "$Id: cpp_output.h,v 1.15 2004/12/13 17:19:01 legros Exp $\n";
			*this << "*/\n\n";
		}
	}



private:

	bool			_CppMode;
	bool			_XmlMode;
	bool			_NewLine;
	sint			_Indent;
	std::string		_Buffer;
	bool			_DescriptionMode;
	bool			_Clean;

	std::vector<std::string>	_XmlNodes;
	bool						_XmlInNode;
	bool						_XmlLastSlash;
	bool						_XmlGetNodeName;
	bool						_XmlCloseNode;
	bool						_XmlRootNode;
	std::string					_XmlNodeName;

	/// Stream operator
	CCppOutput	addToStream (const char* str)
	{
		_Clean = false;
		if (_CppMode)
		{
			while (*str != '\0')
			{
				if (!_DescriptionMode && *str == '}')
					unindent();

				bool	triggerDescriptionStart = false;
				bool	triggerDescriptionEnd = false;

				if (!strncmp(str, "/**", 3))
				{
					// start description
					triggerDescriptionStart = true;
				}

				if (_DescriptionMode && !strncmp(str, "*/", 2))
				{
					// stop description
					triggerDescriptionEnd = true;
				}

				bool	skipChar = false;
				if (*str != '\r')
				{
					if (_NewLine)
					{
						if ((*str == ' ' || *str == '\t') && !_DescriptionMode)
						{
							skipChar = true;
						}
						else
						{
							sint	i;
							for (i=0; i<_Indent; ++i)
								_Buffer += '\t';
							_NewLine = false;

							if (_DescriptionMode)
							{
								_Buffer += " ";
								if (!triggerDescriptionEnd)
									_Buffer += "* ";

								if (!triggerDescriptionEnd && *str == '*')
								{

									skipChar = true;
								}
							}
						}
					}

					if (!_DescriptionMode && *str == '{')
						indent();
				}

				if (*str == '\n')
				{
					_NewLine = true;
				}

				if (!skipChar)
					_Buffer += *str;

				if (triggerDescriptionStart)
					_DescriptionMode = true;
				if (triggerDescriptionEnd)
					_DescriptionMode = false;

				++str;
			}
		}
		else if (_XmlMode)
		{
			while (*str != '\0')
			{
				bool	skipChar = false;
				bool	requireIndent = false;

				if (*str == '<')
				{
					//
					_XmlCloseNode = (str[1] == '/');
					_XmlRootNode = (str[1] == '?');
					_XmlInNode = true;
					_XmlGetNodeName = true;
					_XmlNodeName.clear();

					requireIndent = (!_XmlCloseNode && !_XmlRootNode);
					if (_XmlCloseNode)
						unindent();
				}
				else if (*str == '>')
				{
					_XmlInNode = false;

					if (!_XmlCloseNode && !_XmlRootNode)
					{
						_XmlNodes.push_back(_XmlNodeName);
					}

					if (_XmlCloseNode || _XmlLastSlash)
					{
						if (_XmlLastSlash)
							unindent();

						nlassert(!_XmlNodes.empty() && _XmlNodes.back() == _XmlNodeName);
						_XmlNodes.pop_back();
					}
				}
				else if (*str == '\n')
				{
					_NewLine = true;
				}

				if (_NewLine)
				{
					if (*str == ' ' || *str == '\t')
					{
						skipChar = true;
					}
					else if (*str != '\n' && *str != '\r')
					{
						sint	i;
						for (i=0; i<_Indent; ++i)
							_Buffer += '\t';
						_NewLine = false;
					}
				}

				_XmlLastSlash = (*str == '/');

				if (_XmlInNode && _XmlGetNodeName && *str  != '/' && *str != '<')
				{
					if (!isalpha(*str))
						_XmlGetNodeName = false;

					if (_XmlGetNodeName)
						_XmlNodeName += *str;
				}

				if (!skipChar)
					_Buffer += *str;

				++str;

				if (requireIndent)
					indent();
			}
		}
		else
		{
			_Buffer += str;
		}

		return *this;
	}

};

//
// Inline implementation
//

inline void	CCppOutput::clear()
{
	_Buffer.clear();
}

inline bool	searchForId(char* buffer, char** start, char** end)
{
	const char*	id = "$Id:";
	uint	len = (uint)strlen(id);
	for (; *buffer != '\0'; ++buffer)
	{
		if (strncmp(buffer, id, len) == 0)
		{
			*start = buffer;
			*end = buffer+1;
			while (**end != '\0' && **end != '$')
				++(*end);
			if (**end != (char)'$')
				return false;

			++(*end);
			return true;
		}
	}

	return false;
}

inline void	CCppOutput::flush(const std::string &fileName)
{
	NLMISC::COFile	f;

	bool	overwrite = true;

	if (NLMISC::CFile::fileExists(fileName))
	{
		NLMISC::CIFile	fi;
		if (fi.open(fileName))
		{
			std::string	buffer;
			buffer.resize(fi.getFileSize(), '*');
			fi.serialBuffer((uint8*)(&(buffer[0])), fi.getFileSize());

			// search for $Id: cpp_output.h,v 1.15 2004/12/13 17:19:01 legros Exp $ string in file...
			char	*searchidstart, *searchidend;
			char	*replaceidstart, *replaceidend;

			if (searchForId(&(buffer[0]), &replaceidstart, &replaceidend) &&
				searchForId(&(_Buffer[0]), &searchidstart, &searchidend))
			{
				std::string	replace = std::string(replaceidstart, replaceidend-replaceidstart);
				std::string	search = std::string(searchidstart, searchidend-searchidstart);
				NLMISC::strFindReplace(_Buffer, search, replace);
			}

			overwrite = (buffer != _Buffer);
		}
	}

	if (overwrite)
	{
		if (f.open(fileName))
		{
			f.serialBuffer((uint8*)(_Buffer.c_str()), (uint)_Buffer.size());
			f.close();
		}
		else
		{
			nlwarning("Unable to open file '%s' for output", fileName.c_str());
		}
	}
	else
	{
		nldebug("File '%s' did not changed, skipped", fileName.c_str());
	}

	clear();
}

inline void	CCppOutput::indent()
{
	++_Indent;
}


inline void	CCppOutput::unindent()
{
	--_Indent;
	if (_Indent < 0)
		_Indent = 0;
}






inline void	CClassGenerator::flush(CCppOutput &hFile, CCppOutput &cppFile, CCppOutput &hInlineFile)
{
	hFile << "class " << Name;
	if (!Inherit.empty())
		hFile << " : " << Inherit;
	hFile << "\n{\n";

	hInlineFile << "/* -----------------------------------------\n";
	hInlineFile << " * Inline implementation of " << Name << "\n";
	hInlineFile << " * ----------------------------------------- */\n";

	cppFile << "/* -----------------------------------------\n";
	cppFile << " * Static Implementation of " << Name << "\n";
	cppFile << " * ----------------------------------------- */\n";

	uint	i;
	for (i=0; i<Sections.size(); ++i)
	{
		static const char*	stypes[] = { "public", "protected", "private" };

		SSection	&section = Sections[i];

		if (section.Methods.empty() && section.Other.empty())
			continue;

		uint	j;
		for (j=0; j<section.Methods.size(); ++j)
			if (!section.Methods[j].IsSeparator)
				break;

		if (j == section.Methods.size() && section.Other.empty())
			continue;

		hFile.unindent();
		hFile << "\n" << stypes[section.Type] << ":\n\n";
		hFile.indent();

		if (!section.DisplayName.empty())
		{
			hFile << "/// \\name " << section.DisplayName << "\n// @{\n\n";
			hFile.unindent();
		}

		if (!section.Description.empty())
		{
			hFile << "/**\n" << section.Description << "\n*/\n\n";
		}

		while (!section.Methods.empty() && section.Methods.front().IsSeparator && !section.Methods.front().IsAttribute)
			section.Methods.erase(section.Methods.begin());

		while (!section.Methods.empty() && section.Methods.back().IsSeparator && !section.Methods.back().IsAttribute)
			section.Methods.pop_back();

		for (j=0; j<section.Methods.size(); ++j)
		{
			SMethod	&method = section.Methods[j];

			std::string	inlinekw = (method.IsInline ? std::string("inline ") : std::string(""));
			std::string	statickw = (method.IsStatic ? std::string("static ") : std::string(""));
			std::string	virtualkw = (method.IsVirtual ? std::string("virtual ") : std::string(""));
			std::string	constkw = (method.IsConst ? std::string(" const") : std::string(""));

			if (!method.Description.empty())
			{
				hFile << "\n/**\n" << method.Description << "\n*/\n";
			}

			// raw
			if (method.IsAttribute && method.IsSeparator)
			{
				if (method.IsInline)
				{
					hInlineFile << method.Body;
				}
				else
				{
					cppFile << method.Body;
				}
			}
			// separator
			else if (method.IsSeparator)
			{
				hFile << "\n";
			}
			// attribute
			else if (method.IsAttribute)
			{
				// pointer to function
				if (method.IsVirtual)
				{
					hFile << padTab(statickw+method.Type, 32) << "(*" <<method.Name << ")(" << method.Proto << ");\n";
					if (method.IsStatic)
					{
						cppFile << padTab(method.Type, 32) << "(*" << Name << "::" << method.Name << ")(" << method.Proto << ")";
						if (!method.ConstructorInit.empty())
							cppFile << " = " << method.ConstructorInit;
						cppFile << ";\n";
					}
				}
				else
				{
					hFile << padTab(statickw+method.Type, 32) << method.Name << ";\n";
					if (method.IsStatic)
					{
						if (!method.Proto.empty())
							cppFile << padTab(method.Proto, 32) << Name << "::" << method.Name;
						else
							cppFile << padTab(method.Type, 32) << Name << "::" << method.Name;
						if (!method.ConstructorInit.empty())
							cppFile << " = " << method.ConstructorInit;
						cppFile << ";\n";
					}
				}
			}
			// constructor
			else if (method.Type.empty())
			{
				hFile << virtualkw << method.Name << Name << "(" << method.Proto << ");\n";
				if (method.IsInline)
				{
					hInlineFile << inlinekw << Name << "::" << method.Name << Name << "(" << removeDefaultArgs(method.Proto) << ")";
					if (!method.ConstructorInit.empty())
						hInlineFile << " : " << method.ConstructorInit;
					hInlineFile << "\n";
					hInlineFile << "{\n";
					hInlineFile << method.Body;
					hInlineFile << "}\n";
				}
				else
				{
					cppFile << Name << "::" << method.Name << Name << "(" << removeDefaultArgs(method.Proto) << ")\n";
					cppFile << "{\n";
					cppFile << method.Body;
					cppFile << "}\n";
				}
			}
			// method
			else
			{
				hFile << padTab(statickw+virtualkw+method.Type, 32) << lcFirst(method.Name) << "(" << method.Proto << ")" << constkw << ";\n";
				if (method.IsInline)
				{
					hInlineFile << padTab(inlinekw+method.Type, 32) << Name << "::" << lcFirst(method.Name) << "(" + removeDefaultArgs(method.Proto) << ")" << constkw << "\n";
					hInlineFile << "{\n";
					hInlineFile << method.Body;
					hInlineFile << "}\n";
				}
				else
				{
					cppFile << padTab(method.Type, 32) << Name << "::" << lcFirst(method.Name) << "(" << removeDefaultArgs(method.Proto) << ")" << constkw << "\n";
					cppFile << "{\n";
					cppFile << method.Body;
					cppFile << "}\n";
				}
			}
		}

		hFile << section.Other;

		if (!section.DisplayName.empty())
		{
			hFile << "\n// @}\n\n";
			hFile.indent();
		}
	}

	hFile << "};\n\n";
	hInlineFile << "// End of inline implementation of " << Name << "\n\n";

	cppFile << Body;
	cppFile << "// End of static implementation of " << Name << "\n\n";
}

inline void	CClassGenerator::SMethodId::add(const std::string &instruction)
{
	nlassert(Generator != NULL);
	Generator->add(instruction, *this);
}

inline void	CClassGenerator::SMethodId::setDescription(const std::string &description)
{
	nlassert(Generator != NULL);
	Generator->add(description, *this);
}

inline void	CFunctionGenerator::flush(CCppOutput &hFile, CCppOutput &cppFile, CCppOutput &hInlineFile)
{
	std::string	inlinekw = (IsInline ? std::string("inline ") : std::string(""));

	if (!Description.empty())
	{
		hFile << "\n/**\n" << Description << "\n*/\n";
	}

	hFile << padTab(Type, 32) << lcFirst(Name) << "(" << Proto << ");\n";

	if (IsInline)
	{
		hInlineFile << padTab(inlinekw+Type, 32) << lcFirst(Name) << "(" << removeDefaultArgs(Proto) << ")\n";
		hInlineFile << "{\n";
		hInlineFile << Body;
		hInlineFile << "}\n\n";
	}
	else
	{
		cppFile << padTab(Type, 32) << lcFirst(Name) << "(" << removeDefaultArgs(Proto) << ")\n";
		cppFile << "{\n";
		cppFile << Body;
		cppFile << "}\n\n";
	}
}



#endif
