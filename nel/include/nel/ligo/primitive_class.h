// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#ifndef NL_PRIMITIVE_CLASS_H
#define NL_PRIMITIVE_CLASS_H

#include "nel/misc/types_nl.h"
#include "nel/misc/rgba.h"
#include <map>
#include <set>

// Forward declarations for libxml2
typedef struct _xmlNode xmlNode;
typedef xmlNode *xmlNodePtr;

namespace NLLIGO
{

class IPrimitive;
class IProperty;
class CLigoConfig;

/**
 * Class of primitive
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2002
 */
class CPrimitiveClass
{
public:

	// Type of the primitive
	enum TType
	{
		Node,
		Point,
		Path,
		Bitmap,
		Zone,
		Alias
	}						Type;

	/// Constructor
	CPrimitiveClass ();

	/// Class name
	std::string				Name;

	/// Filename extension (for type File)
	std::string				FileExtension;

	/// File type (for type File)
	std::string				FileType;

	/// Color
	NLMISC::CRGBA			Color;

	/// Auto init ?
	bool					AutoInit;

	/// Deletable ?
	bool					Deletable;

	/// Collision ?
	bool					Collision;

	/// Link children ?
	bool					LinkBrothers;

	/// Show arrow ?
	bool					ShowArrow;

	/// Numberize on copy ?
	bool					Numberize;

	/// Is primitive visible ?
	bool					Visible;

	/// Init parameters
	class CInitParameters
	{
		// A default value
		class CDefaultValue
		{
		public:
			std::string		Name;
			bool			GenID;

			bool			operator== (const CDefaultValue &other) const
			{
				return (Name == other.Name) && (GenID == other.GenID);
			}

			bool			operator< (const CDefaultValue &other) const
			{
				if (Name < other.Name)
					return true;
				else if (Name == other.Name)
				{
					return (GenID < other.GenID);
				}
				else
					return false;
			}
		};

	public:
		/// Parameter name
		std::string					Name;

		/// Default value
		std::vector<CDefaultValue>	DefaultValue;
	};

	// Parameter description
	class CParameter : public CInitParameters
	{
	public:
		CParameter () {}
		CParameter (const NLLIGO::IProperty &property, const char *propertyName);
		bool operator== (const CParameter &other) const;
		bool operator< (const CParameter &other) const;

		// Type
		enum TType
		{
			Boolean,
			ConstString,
			String,
			StringArray,
			ConstStringArray,
		}			Type;

		/// Is parameter visible ?
		bool		Visible;

		// Is a filename
		bool		Filename;

		// Make a look up ?
		bool		Lookup;

		/// Is parameter read only ?
		bool		ReadOnly;

		// File extension
		std::string	FileExtension;

		// Autonaming
		std::string	Autoname;

		// Folder
		std::string	Folder;

		// Size of multi line view
		uint		WidgetHeight;

		// Sort entry in combo box
		bool		SortEntries;

		// Editable
		bool		Editable;

		// Display horizontal slider in multiline edit box
		bool		DisplayHS;

		// Combobox value
		class CConstStringValue
		{
		public:
			bool operator== (const CConstStringValue &other) const;
			bool operator< (const CConstStringValue &other) const;
			std::vector<std::string>	Values;
			std::vector<std::string>	PrimitivePath;
			void	appendFilePath				(std::vector<std::string> &pathList)	const;
			void	appendPrimPath				(std::vector<std::string> &pathList, const	std::vector<const IPrimitive*>	&relativePrimPaths)	const;
			void	getPrimitivesForPrimPath	(std::vector<const IPrimitive*>	&relativePrimPaths, const	std::vector<const IPrimitive*>	&startPrimPath)	const;
		};

		// Map of combobox value per context
		std::map<std::string, CConstStringValue>	ComboValues;

		/// Get the autoname translation
		bool	translateAutoname (std::string &result, const IPrimitive &primitive, const CPrimitiveClass &primitiveClass) const;

		// Get a default value
		bool	getDefaultValue (std::string &result, const IPrimitive &primitive, const CPrimitiveClass &primitiveClass, std::string *fromWhere = NULL) const;
		bool	getDefaultValue (std::vector<std::string> &result, const IPrimitive &primitive, const CPrimitiveClass &primitiveClass, std::string *fromWhere = NULL) const;
	};

	/// Parameters
	std::vector<CParameter>	Parameters;

	// Child
	class CChild
	{
	public:
		/// Static child name
		std::string	Name;

		/// Child class name
		std::string	ClassName;

		/// Init parameters
		std::vector<CInitParameters>	Parameters;
	};

	// Static Children
	std::vector<CChild>			StaticChildren;

	// Dynamic Children
	std::vector<CChild>			DynamicChildren;

	// Generated Children
	std::vector<CChild>			GeneratedChildren;

	// Read
	bool	read (xmlNodePtr primitiveNode,
					const char *filename,
					const char *className,
					std::set<std::string> &contextStrings,
					std::map<std::string, std::string> &contextFilesLookup,
					NLLIGO::CLigoConfig &config,
					bool parsePrimitiveComboContent);
};

} // NLLIGO

#endif // NL_PRIMITIVE_CLASS_H

/* End of primitive_class.h */
