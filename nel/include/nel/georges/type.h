// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2014  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef NL_TYPE_H
#define NL_TYPE_H

#include "nel/misc/types_nl.h"
#include "nel/georges/u_type.h"
#include "nel/georges/u_form_elm.h"
#include "header.h"

namespace NLGEORGES
{

class UFormElm;

/**
  * This class is a basic type used by atomic form element.
  */
class CType : public UType
{
	friend class CFormLoader;
public:

	CType ();
	virtual ~CType ();

	// ** UI
	enum TUI
	{
		Edit,				// Default, for all types
		EditSpin,			// For number types
		NonEditableCombo,	// For all types
		FileBrowser,		// Browse file
		BigEdit,			// Edit a huge text
		ColorEdit,			// Edit a color
		IconWidget,			// Draw an icon
		UITypeCount
	};

	// Is a UI compatible with a type ?
	static bool uiCompatible (TType type, TUI ui);

	// ** IO functions
	void				write (xmlDocPtr doc) const;

	// Header
	CFileHeader			Header;

	// Type of the type
	UType::TType		Type;

	// Type fo user interface
	TUI					UIType;

	// Default value
	std::string			Default;

	// Min value
	std::string			Min;

	// Max value
	std::string			Max;

	// Increment step value
	std::string			Increment;

	// Evaluate a node
	bool				getValue (std::string &result, const class CForm *form, const class CFormElmAtom *node, const class CFormDfn &parentDfn,
									uint parentIndex, UFormElm::TEval evaluate, uint32 *where, uint32 round, const std::string &formName) const;

	// Definitions
	class CDefinition
	{
	public:
		// Label of the definition
		std::string		Label;

		// Value of the definition
		std::string		Value;
	};

	// Array of definition
	std::vector<CDefinition>	Definitions;

	// Get the type names
	static const char *getTypeName (UType::TType type);
	static const char *getUIName (TUI type);

	// From UType
	virtual TType				getType () const;
	virtual const std::string	&getDefault () const;
	virtual const std::string	&getMin () const;
	virtual const std::string	&getMax () const;
	virtual const std::string	&getIncrement () const;
	virtual uint				getNumDefinition () const;
	virtual bool				getDefinition (uint index, std::string &label, std::string &value) const;
	virtual const std::string	&getComment () const;
	virtual	void				getDependencies (std::set<std::string> &dependencies) const;

private:

	// Error handling
	virtual void		warning (bool exception, const std::string &formName, const std::string &formFilename, const std::string &function, const char *format, ... ) const;
	virtual void		warning2 (bool exception, const std::string &function, const char *format, ... ) const;

	// Type names
	static const char	*TypeNames[];
	static const char	*UITypeNames[];

	// CFormLoader call it
	void				read (xmlNodePtr root);

};

} // NLGEORGES

#endif // NL_TYPE_H
