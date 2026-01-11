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

#include "stdgeorgesconvert.h"

#include "nel/misc/file.h"
#include "nel/misc/i_xml.h"
#include "nel/misc/o_xml.h"

#include "nel/georges/type.h"
#include "nel/georges/form.h"
#include "nel/georges/form_dfn.h"

#include "form_body_elt_atom.h"
#include "form_body_elt_list.h"
#include "form_body_elt_struct.h"
#include "form_file.h"

using namespace NLMISC;
using namespace std;

// Return the string value of a field in the structure
NLOLDGEORGES::CFormBodyElt *getValue (const char *key, const NLOLDGEORGES::CFormBodyEltStruct& str)
{
	for (uint elm=0; elm<str.vpbodyelt.size(); elm++)
	{
		if (str.vpbodyelt[elm]->sxname == key)
			return str.vpbodyelt[elm];
	}

	// Not found
	return NULL;
}

bool convertTypeFile (const char *oldFileName, const char *newFileName)
{
	// Load the form file

	// File stream
	CIFile file;
	if (file.open (oldFileName))
	{
		// Catch errors
		try
		{
			// Init the xml stream
			CIXml xmlFile;
			xmlFile.init (file);

			// Read the old file format
			NLOLDGEORGES::CFormFile oldFileFormat;
			oldFileFormat.serial (xmlFile);

			// Get a form pointer
			if (oldFileFormat.lform.size () != 1)
			{
				// Warning
				nlwarning ("Invalid old type file format in file %s", oldFileName);
				return false;
			}
			else
			{
				NLOLDGEORGES::CFormBodyEltStruct &str = oldFileFormat.lform.begin ()->body;

				// New file format
				NLGEORGES::CType newType;

				// Get type
				NLOLDGEORGES::CFormBodyEltAtom *eltType = dynamic_cast<NLOLDGEORGES::CFormBodyEltAtom*> (getValue ("Type", str));
				NLOLDGEORGES::CFormBodyEltAtom *eltEnum = dynamic_cast<NLOLDGEORGES::CFormBodyEltAtom*> (getValue ("Enum", str));
				NLOLDGEORGES::CFormBodyEltAtom *eltFormula = dynamic_cast<NLOLDGEORGES::CFormBodyEltAtom*> (getValue ("Formula", str));
				NLOLDGEORGES::CFormBodyEltAtom *eltDefaultValue = dynamic_cast<NLOLDGEORGES::CFormBodyEltAtom*> (getValue ("DefaultValue", str));
				NLOLDGEORGES::CFormBodyEltAtom *eltLowlimit = dynamic_cast<NLOLDGEORGES::CFormBodyEltAtom*> (getValue ("Lowlimit", str));
				NLOLDGEORGES::CFormBodyEltAtom *eltHighlimit = dynamic_cast<NLOLDGEORGES::CFormBodyEltAtom*> (getValue ("Highlimit", str));
				NLOLDGEORGES::CFormBodyEltList *eltPredef = dynamic_cast<NLOLDGEORGES::CFormBodyEltList*> (getValue ("Predef", str));

				// All needed is present ?
				if (eltType && eltPredef)
				{
					// Convert enum
					bool _enum = ( eltEnum && (eltEnum->sxvalue == "true") );

					// Have predef ?
					bool predef = eltPredef->vpbodyelt.size() != 0;

					// Convert the type
					eltType->sxvalue = strlwr (eltType->sxvalue);
					if (eltType->sxvalue == "uint")
					{
						newType.Type = NLGEORGES::CType::UnsignedInt;
						newType.UIType = _enum ? NLGEORGES::CType::NonEditableCombo : NLGEORGES::CType::EditSpin;
					}
					else if (eltType->sxvalue == "sint")
					{
						newType.Type = NLGEORGES::CType::SignedInt;
						newType.UIType = _enum ? NLGEORGES::CType::NonEditableCombo : NLGEORGES::CType::EditSpin;
					}
					else if (eltType->sxvalue == "string")
					{
						newType.Type = NLGEORGES::CType::String;
						newType.UIType = _enum ? NLGEORGES::CType::NonEditableCombo : NLGEORGES::CType::Edit;
					}
					else if (eltType->sxvalue == "double")
					{
						newType.Type = NLGEORGES::CType::Double;
						newType.UIType = _enum ? NLGEORGES::CType::NonEditableCombo : NLGEORGES::CType::EditSpin;
					}
					else if (eltType->sxvalue == "filename")
					{
						newType.Type = NLGEORGES::CType::String;
						newType.UIType = NLGEORGES::CType::FileBrowser;
					}
					else
					{
						// Warning
						nlwarning ("Invalid old type file format in file %s", oldFileName);
						return false;
					}

					// Convert predef
					if (predef)
					{
						// Resize the predef array
						newType.Definitions.resize (eltPredef->vpbodyelt.size());

						// For each predef
						uint pre;
						for (pre=0; pre<eltPredef->vpbodyelt.size(); pre++)
						{
							NLOLDGEORGES::CFormBodyEltStruct *eltPre = dynamic_cast<NLOLDGEORGES::CFormBodyEltStruct*> (eltPredef->vpbodyelt[pre]);
							if (eltPre)
							{
								// Get the label
								NLOLDGEORGES::CFormBodyEltAtom *eltLabel = dynamic_cast<NLOLDGEORGES::CFormBodyEltAtom*> (getValue ("Designation", *eltPre));

								// Get the value
								NLOLDGEORGES::CFormBodyEltAtom *eltValue = dynamic_cast<NLOLDGEORGES::CFormBodyEltAtom*> (getValue ("Substitute", *eltPre));

								// Ok ?
								if (eltLabel && eltValue)
								{
									newType.Definitions[pre].Label = eltLabel->sxvalue;
									newType.Definitions[pre].Value = eltValue->sxvalue;
								}
								else
								{
									// Warning
									nlwarning ("Invalid old type file format in file %s", oldFileName);
									return false;
								}
							}
							else
							{
								// Warning
								nlwarning ("Invalid old type file format in file %s", oldFileName);
								return false;
							}
						}
					}

					// Default value
					newType.Default = eltDefaultValue ? eltDefaultValue->sxvalue : "";

					// Min
					newType.Min = eltLowlimit ? eltLowlimit->sxvalue : "";

					// Max
					newType.Max = eltHighlimit ? eltHighlimit->sxvalue : "";

					// Some log
					newType.Header.addLog ("File converted from old format");
					newType.Header.setComments ("Converted from old format");

					// ** Type ok, save it

					// Output file
					COFile output;
					if (output.open (newFileName))
					{
						try
						{
							// Xml file
							COXml outputXml;
							outputXml.init (&output);

							// Write the document
							newType.write (outputXml.getDocument ());

							// Flush the document
							outputXml.flush ();
						}
						catch (Exception &e)
						{
							// Warning
							nlwarning ("Error while writing xmlFile %s (%s)", oldFileName, e.what ());
							return false;
						}
					}
					else
					{
						// Warning
						nlwarning ("Can't open the TYPE file %s for writing", oldFileName);
						return false;
					}
				}
				else
				{
					// Warning
					nlwarning ("Invalid old type file format in file %s", oldFileName);
					return false;
				}
			}
		}
		catch (Exception &e)
		{
			// Warning
			nlwarning ("Error while loading xmlFile %s (%s)", oldFileName, e.what ());
			return false;
		}
	}
	else
	{
		// Warning
		nlwarning ("Can't open the TYPE file %s for reading", oldFileName);
		return false;
	}

	return true;
}

bool convertDfnFile (const char *oldFileName, const char *newFileName)
{
	// Load the form file

	// File stream
	CIFile file;
	if (file.open (oldFileName))
	{
		// Catch errors
		try
		{
			// Init the xml stream
			CIXml xmlFile;
			xmlFile.init (file);

			// Read the old file format
			NLOLDGEORGES::CFormFile oldFileFormat;
			oldFileFormat.serial (xmlFile);

			// Get a form pointer
			if (oldFileFormat.lform.size () != 1)
			{
				// Warning
				nlwarning ("Invalid old dfn file format in file %s", oldFileName);
				return false;
			}
			else
			{
				NLOLDGEORGES::CFormBodyEltStruct &str = oldFileFormat.lform.begin ()->body;

				// New file format
				NLGEORGES::CFormDfn newDfn;
				newDfn.Entries.resize (str.vpbodyelt.size());

				// For each element of the struct
				for (uint elm=0; elm<str.vpbodyelt.size(); elm++)
				{
					// Get the atom
					NLOLDGEORGES::CFormBodyEltAtom *eltAtom = dynamic_cast<NLOLDGEORGES::CFormBodyEltAtom*> (str.vpbodyelt[elm]);
					if (eltAtom)
					{
						// Get name and value
						string name = eltAtom->sxname;
						string value = eltAtom->sxvalue;
						string value_lwr = strlwr (value);

						// Is a list ?
						bool isArray = ( value_lwr.find ( "list<" ) != -1 );

						// Get list value
						if ( isArray )
						{
							unsigned int ipos = value_lwr.find( ">" );
							if( ipos < 0 )
							{
								// Warning
								nlwarning ("Invalid old dfn file format in file %s", oldFileName);
								return false;
							}
							value = value.substr ( 6, ipos-7 );
							value_lwr = value_lwr.substr ( 6, ipos-7 );
						}

						// Is a typ ?
						bool type = ( value_lwr.find ( ".typ" ) != -1 );

						// Is a dfn ?
						bool dfn = ( value_lwr.find ( ".dfn" ) != -1 );

						// Valid ?
						if (type || dfn)
						{
							// Create a new array
							newDfn.Entries[elm].TypeElement = type ? NLGEORGES::UFormDfn::EntryType : NLGEORGES::UFormDfn::EntryDfn;
							newDfn.Entries[elm].Name = name;
							newDfn.Entries[elm].Filename = value;
							newDfn.Entries[elm].Array = isArray;
						}
						else
						{
							// Warning
							nlwarning ("Invalid old dfn file format in file %s", oldFileName);
							return false;
						}
					}
					else
					{
						// Warning
						nlwarning ("Invalid old dfn file format in file %s", oldFileName);
						return false;
					}
				}
				
				// Some log
				newDfn.Header.addLog ("File converted from old format");
				newDfn.Header.setComments ("Converted from old format");

				// ** Type ok, save it

				// Output file
				COFile output;
				if (output.open (newFileName))
				{
					try
					{
						// Xml file
						COXml outputXml;
						outputXml.init (&output);

						// Write the document
						newDfn.write (outputXml.getDocument ());

						// Flush the document
						outputXml.flush ();
					}
					catch (Exception &e)
					{
						// Warning
						nlwarning ("Error while writing xmlFile %s (%s)", oldFileName, e.what ());
						return false;
					}
				}
				else
				{
					// Warning
					nlwarning ("Can't open the dfn file %s for writing", oldFileName);
					return false;
				}
			}
		}
		catch (Exception &e)
		{
			// Warning
			nlwarning ("Error while loading xmlFile %s (%s)", oldFileName, e.what ());
			return false;
		}
	}
	else
	{
		// Warning
		nlwarning ("Can't open the dfn file %s for reading", oldFileName);
		return false;
	}

	return true;
}

bool convertFormAtom (const NLOLDGEORGES::CFormBodyEltAtom &_atom, NLGEORGES::CFormElmAtom &dst)
{
	// Copy the value
	dst.setValue (((string)_atom.sxvalue).c_str ());

	// Ok
	return true;
}

bool convertFormStruct (const NLOLDGEORGES::CFormBodyEltStruct &_struct, NLGEORGES::CFormElmStruct &dst, NLGEORGES::CForm *form);

bool convertFormArray (const NLOLDGEORGES::CFormBodyEltList &listSrc, NLGEORGES::CFormElmArray &array, NLGEORGES::CForm *form)
{
	// Resize the destination struct
	array.Elements.resize (listSrc.vpbodyelt.size ());

	// For each sub-element
	uint i;
	for (i=0; i<listSrc.vpbodyelt.size (); i++)
	{
		// Cast
		const NLOLDGEORGES::CFormBodyEltAtom *_atom = dynamic_cast<const NLOLDGEORGES::CFormBodyEltAtom*> (listSrc.vpbodyelt[i]);
		if (_atom)
		{
			// New atom
			NLGEORGES::CFormElmAtom *atomDst = new NLGEORGES::CFormElmAtom (form, NULL, NULL, 0xffffffff);
			array.Elements[i] = atomDst;

			// Convert the atom
			if (!convertFormAtom (*_atom, *atomDst))
				return false;
		}
		else
		{
			const NLOLDGEORGES::CFormBodyEltStruct *_struct = dynamic_cast<const NLOLDGEORGES::CFormBodyEltStruct*> (listSrc.vpbodyelt[i]);
			if (_struct)
			{
				// New struct
				NLGEORGES::CFormElmStruct *_newStruct = new NLGEORGES::CFormElmStruct (form, NULL, NULL, 0xffffffff);
				array.Elements[i] = _newStruct;

				// Build this struct
				if (!convertFormStruct (*_struct, *_newStruct, form))
					return false;
			}
			else
			{
				const NLOLDGEORGES::CFormBodyEltList *_list = dynamic_cast<const NLOLDGEORGES::CFormBodyEltList*> (listSrc.vpbodyelt[i]);
				if (_list)
				{
					// New array
					NLGEORGES::CFormElmArray *_array = new NLGEORGES::CFormElmArray (form, NULL, NULL, NULL, NULL, 0xffffffff);
					array.Elements[i] = _array;

					// Build this struct
					if (!convertFormArray (*_list, *_array, form))
						return false;
				}
				else
					return false;
			}
		}
	}

	return true;
}

bool convertFormStruct (const NLOLDGEORGES::CFormBodyEltStruct &structSrc, NLGEORGES::CFormElmStruct &dst, NLGEORGES::CForm *form)
{
	// Resize the destination struct
	dst.Elements.resize (structSrc.vpbodyelt.size ());

	// For each sub-element
	uint i;
	for (i=0; i<structSrc.vpbodyelt.size (); i++)
	{
		// Element name
		dst.Elements[i].Name = structSrc.vpbodyelt[i]->sxname;

		// Cast
		const NLOLDGEORGES::CFormBodyEltAtom *_atom = dynamic_cast<const NLOLDGEORGES::CFormBodyEltAtom*> (structSrc.vpbodyelt[i]);
		if (_atom)
		{
			// New atom
			NLGEORGES::CFormElmAtom *atomDst = new NLGEORGES::CFormElmAtom (form, NULL, NULL, 0xffffffff);
			dst.Elements[i].Element = atomDst;

			// Convert the atom
			if (!convertFormAtom (*_atom, *atomDst))
				return false;
		}
		else
		{
			const NLOLDGEORGES::CFormBodyEltStruct *_struct = dynamic_cast<const NLOLDGEORGES::CFormBodyEltStruct*> (structSrc.vpbodyelt[i]);
			if (_struct)
			{
				// New struct
				NLGEORGES::CFormElmStruct *_newStruct = new NLGEORGES::CFormElmStruct (form, NULL, NULL, 0xffffffff);
				dst.Elements[i].Element = _newStruct;

				// Build this struct
				if (!convertFormStruct (*_struct, *_newStruct, form))
					return false;
			}
			else
			{
				const NLOLDGEORGES::CFormBodyEltList *_list = dynamic_cast<const NLOLDGEORGES::CFormBodyEltList*> (structSrc.vpbodyelt[i]);
				if (_list)
				{
					// New array
					NLGEORGES::CFormElmArray *_array = new NLGEORGES::CFormElmArray (form, NULL, NULL, NULL, NULL, 0xffffffff);
					dst.Elements[i].Element = _array;

					// Build this struct
					if (!convertFormArray (*_list, *_array, form))
						return false;
				}
				else
					return false;
			}
		}
	}

	return true;
}

bool convertFormFile (const char *oldFileName, const char *newFileName)
{
	// Load the form file

	// File stream
	CIFile file;
	if (file.open (oldFileName))
	{
		// Catch errors
		try
		{
			// Init the xml stream
			CIXml xmlFile;
			xmlFile.init (file);

			// Read the old file format
			NLOLDGEORGES::CFormFile oldFileFormat;
			oldFileFormat.serial (xmlFile);

			// Get a form pointer
			if (oldFileFormat.lform.size () != 1)
			{
				// Warning
				nlwarning ("Invalid old form file format in file %s", oldFileName);
				return false;
			}
			else
			{
				// Struct ref
				NLOLDGEORGES::CFormBodyEltStruct &str = oldFileFormat.lform.begin ()->body;

				// Build this struct
				NLGEORGES::CForm newForm;
				if (convertFormStruct (str, newForm.Elements, &newForm))
				{
					string parent = str.GetParent( 0 );
					if (!parent.empty())
					{
						newForm.ParentFilename = parent;
					}
					
					// Some log
					newForm.Header.addLog ("File converted from old format");
					newForm.Header.setComments ("Converted from old format");

					// Output file
					COFile output;
					if (output.open (newFileName))
					{
						try
						{
							// Xml file
							COXml outputXml;
							outputXml.init (&output);

							// Write the document
							newForm.write (outputXml.getDocument ());

							// Flush the document
							outputXml.flush ();
						}
						catch (Exception &e)
						{
							// Warning
							nlwarning ("Error while writing xmlFile %s (%s)", oldFileName, e.what ());
							return false;
						}
					}
					else
					{
						// Warning
						nlwarning ("Can't open the dfn file %s for writing", oldFileName);
						return false;
					}
				}
				else
				{
					// Warning
					nlwarning ("Invalid old form file format in file %s", oldFileName);
					return false;
				}
			}
		}
		catch (Exception &e)
		{
			// Warning
			nlwarning ("Error while loading xmlFile %s (%s)", oldFileName, e.what ());
			return false;
		}
	}
	else
	{
		// Warning
		nlwarning ("Can't open the form file %s for reading", oldFileName);
		return false;
	}

	return true;
}

using namespace NLOLDGEORGES;

int main (int argc, void **argv)
{
	new NLMISC::CApplicationContext;

	// Register classes
	NLMISC_REGISTER_CLASS (CFormBodyElt);
	NLMISC_REGISTER_CLASS (CFormBodyEltAtom);
	NLMISC_REGISTER_CLASS (CFormBodyEltList);
	NLMISC_REGISTER_CLASS (CFormBodyEltStruct);

	// Help ?
	if (argc!=3)
	{
		printf ("georges_convert [old_file] [new_file]");
		return 0;
	}
	else
	{
		strlwr ((const char*)argv[1]);
		bool isType = strstr ((const char*)argv[1], ".typ") != NULL;
		bool isDfn = strstr ((const char*)argv[1], ".dfn") != NULL;
		if (isType)
			return convertTypeFile ((const char*) argv[1], (const char*) argv[2]) ? 1 : 0;
		else if (isDfn)
			return convertDfnFile ((const char*) argv[1], (const char*) argv[2]) ? 1 : 0;
		else
			return convertFormFile ((const char*) argv[1], (const char*) argv[2]) ? 1 : 0;
	}
	
	return 1;
}