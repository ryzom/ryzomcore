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

#ifndef GEORGES_INTERFACE
#define GEORGES_INTERFACE

#ifndef GEORGES_EXPORT
#define GEORGES_EXPORT __declspec( dllimport ) 
#endif // GEORGES_EXPORT

// Increment this version number each time you distribute a new version of the dll.
#define GEORGES_VERSION 3

#include <string>
#include <vector>

// Should be the same than in NLGEORGES::CType
enum TType
{
	UnsignedInt=0,
	SignedInt,
	Double,
	String,
	Color,
	TypeCount
};

// Should be the same than in NLGEORGES::CType
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

/**
 * IGeorges
 *
 * \author Matthieu "TrapII" Besson
 * \author Nevrax France
 * \date 2001
 */
class IGeorges
{
public:
	virtual ~IGeorges() {}

	// Init the UI
	virtual void initUI (int m_nCmdShow, bool exeStandalone, HWND parent=NULL)=0;

	// Init the UI Light version
	virtual void initUILight (int m_nCmdShow, int x, int y, int cx, int cy)=0;

	// Go
	virtual void go ()=0;

	// Release the UI
	virtual void releaseUI ()=0;

	// Get the main frame
	virtual void *getMainFrame ()=0;
	
	//virtual void SetDocumentWorkDirectory( const std::string& _sxworkdirectory ) = 0;
	//virtual void SetDocumentRootDirectory( const std::string& _sxrootdirectory ) = 0;

//	virtual void NewDocument() = 0;

//	virtual void NewDocument (const std::string& _sxdfnname) = 0;

	virtual void LoadDocument (const std::string& _sxfullname) = 0;

//	virtual void SaveDocument (const std::string& _sxfullname) = 0;

//	virtual void CloseDocument() = 0;

	// Directories Management
	virtual void SetDirDfnTyp		(const std::string &_sxDirectory) = 0;
	virtual void SetDirPrototype	(const std::string &_sxDirectory) = 0;
	virtual void SetDirLevel		(const std::string &_sxDirectory) = 0;

	virtual std::string GetDirDfnTyp	() = 0;
/*	virtual std::string GetDirPrototype	() = 0;
	virtual std::string GetDirLevel		() = 0;*/

	// Put a text in the right cell
	virtual void PutGroupText (const std::vector<std::string>& _vText, bool append) = 0;
	virtual void PutText (const std::string& _sText) = 0;
	virtual void LineUp () = 0;
	virtual void LineDown () = 0;

	virtual BOOL PreTranslateMessage (MSG *pMsg) = 0;

/*	virtual void SaveAllDocument() = 0;
	virtual void CloseAllDocument() = 0;

	virtual void SetTypPredef( const std::string& _sxfilename, const std::vector< std::string >& _pvsx ) = 0;

	virtual void MakeDfn( const std::string& _sxfullname, const std::vector< std::pair< std::string, std::string > >* const _pvdefine = 0 ) = 0;
*/
	virtual void MakeTyp( const std::string& filename, TType type, TUI ui, const std::string& _min, const std::string& _max, const std::string& _default, const std::vector< std::pair< std::string, std::string > >* const _pvpredef ) = 0;

	virtual void createInstanceFile (const std::string &_sxFullnameWithoutExt, const std::string &_dfnname) = 0;

	// -----------------------------------------------------------------------

	// Get interface (autoconstruct like a factory)
	static GEORGES_EXPORT IGeorges *getInterface (int version = GEORGES_VERSION);

	// Release interface
	static GEORGES_EXPORT void releaseInterface (IGeorges *pGeorges);
};


// To export the names in a good format that can be human readable and not with the heavy style
// of the C++ we have to do it in 'old-school' mode (so in C). But this is just a bind to
// the static factory constructor/destructor
extern "C" 
{
	GEORGES_EXPORT IGeorges *IGeorgesGetInterface (int version = GEORGES_VERSION);
	GEORGES_EXPORT void IGeorgesReleaseInterface (IGeorges *pG);
}

#endif LOGIC_EDITOR_INTERFACE
