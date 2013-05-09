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


#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>
#include "nel/misc/types_nl.h"
#include "nel/gui/proc.h"
#include "nel/gui/link_data.h"

namespace NLGUI
{
	class CInterfaceElement;
	class CViewBase;
	class CInterfaceGroup;
	class CInterfaceAnim;
	class CCtrlSheetSelection;
	class CInterfaceLink;

	/// Interface for the GUI XML parser class.
	class IParser
	{
	public:
		IParser();
		virtual ~IParser();

		static IParser* createParser();

		virtual void addParentPositionAssociation( CInterfaceElement *element, const std::string &parentID ) = 0;
        virtual void addParentSizeAssociation( CInterfaceElement *element, const std::string &parentID )     = 0;
        virtual void addParentSizeMaxAssociation( CInterfaceElement *element, const std::string &parentID )  = 0;
        virtual void addLuaClassAssociation( CInterfaceGroup *group, const std::string &luaScript )          = 0;
		virtual std::string getParentPosAssociation( CInterfaceElement *element ) const = 0;
		virtual std::string getParentSizeAssociation( CInterfaceElement *element ) const = 0;
		virtual std::string getParentSizeMaxAssociation( CInterfaceElement *element ) const = 0;
		virtual std::string getLuaClassAssociation( CInterfaceGroup *group ) const = 0;
        virtual CInterfaceGroup* createGroupInstance( const std::string &templateName, const std::string &parentID, const std::pair< std::string, std::string > *templateParams, uint numParams, bool updateLinks = true ) = 0;
        virtual CInterfaceGroup* createGroupInstance( const std::string &templateName, const std::string &parentID, std::vector< std::pair< std::string, std::string > > &templateParams, bool updateLinks = true ) = 0;
        virtual bool parseGroupChildren( xmlNodePtr cur, CInterfaceGroup * parentGroup, bool reload ) = 0;
        virtual uint getProcedureNumActions( const std::string &procName ) const = 0;
        virtual bool getProcedureAction( const std::string &procName, uint actionIndex, std::string &ah, std::string &params ) const = 0;
        virtual const std::string&  getDefine(const std::string &id) const = 0;
        virtual CInterfaceAnim* getAnim( const std::string &name ) const = 0;
        virtual CProcedure* getProc( const std::string &name ) = 0;
		virtual const TProcedureMap& getProcMap() const = 0;
		virtual bool parseInterface( const std::vector< std::string > &xmlFileNames, bool reload, bool isFilename = true, bool checkInData = false ) = 0;
		virtual void initLUA() = 0;
		virtual void uninitLUA() = 0;
		virtual bool isLuaInitialized() const = 0;
		virtual bool loadLUA( const std::string &luaFile, std::string &error ) = 0;
		virtual void reloadAllLuaFileScripts() = 0;
		virtual void removeAllTemplates() = 0;
		virtual bool solveDefine( const std::string &propVal, std::string &newPropVal, std::string &defError ) = 0;
		virtual CInterfaceElement* createUIElement( const std::string &templateName, const std::string &parentID, const std::pair< std::string,std::string> *templateParams, uint numParams, bool updateLinks ) = 0;
		virtual CInterfaceElement* createUIElement( const std::string &templateName, const std::string &parentID, std::vector< std::pair< std::string, std::string > > &templateParams, bool updateLinks = true ) = 0;
		virtual bool isDefineExist( const std::string &id ) const = 0;
		virtual CCtrlSheetSelection	&getCtrlSheetSelection() = 0;
		virtual bool addLink( CInterfaceLink *link, const std::string &id ) = 0;
		virtual bool removeLink( const std::string &id ) = 0;
		virtual void removeAll() = 0;
		virtual bool hasProc( const std::string &name ) const = 0;
		virtual bool addProc( const std::string &name ) = 0;
		virtual bool removeProc( const std::string &name ) = 0;
		virtual void setEditorMode( bool b ) = 0;
		virtual const std::map< uint32, SLinkData >& getLinkMap() const = 0;
		virtual uint32 addLinkData( SLinkData &linkData ) = 0;
		virtual void removeLinkData( uint32 id ) = 0;
		virtual bool getLinkData( uint32 id, SLinkData &linkData ) = 0;
		virtual void updateLinkData( uint32 id, const SLinkData &linkData ) = 0;
		virtual bool serializeVariables( xmlNodePtr parentNode ) const = 0;
		virtual bool serializeProcs( xmlNodePtr parentNode ) const = 0;
		virtual bool serializePointerSettings( xmlNodePtr parentNode ) const = 0;
		virtual bool serializeKeySettings( xmlNodePtr parentNode ) const = 0;
		virtual CViewBase* createClass( const std::string &name ) = 0;
	};
}

#endif

