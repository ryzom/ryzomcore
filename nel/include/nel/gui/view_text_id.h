// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
// Copyright (C) 2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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



#ifndef NL_VIEW_TEXT_ID_H
#define NL_VIEW_TEXT_ID_H

#include "nel/misc/types_nl.h"
#include "nel/gui/view_text.h"

namespace NLMISC{
	class CCDBNodeLeaf;
}

namespace NLGUI
{

	// ***************************************************************************
	class IOnReceiveTextId
	{
	public:
		virtual ~IOnReceiveTextId() {}
		// the deriver may change the input text
		virtual	void	onReceiveTextId(std::string &str) =0;
	};

	// ***************************************************************************
	/**
	 * class implementing a text view that take the text from an id
	 * \author Matthieu 'TrapII' Besson
	 * \author Nevrax France
	 * \date 2002
	 */
	class CViewTextID : public CViewText
	{
	public:
        DECLARE_UI_CLASS( CViewTextID )
		
		/// Interface for classes which can provide text to CViewTextId
		class IViewTextProvider
		{
		public:
			virtual ~IViewTextProvider(){}
			virtual bool getString(uint32 stringId, std::string &result) = 0;
			virtual bool getDynString(uint32 dynStringId, std::string &result) = 0;
		};

		CViewTextID(const TCtorParam &param) : CViewText(param)
		{
			_StringModifier= NULL;
			_IsDBLink = false;
			_TextId = 0xFFFFFFFF;
			_Initialized = false;
			_DynamicString = false;
			_IsTextFormatTaged= false;
		}

		// ctor with a text id
		CViewTextID (const std::string& id, uint32 nID, sint FontSize=12,
				NLMISC::CRGBA Color=NLMISC::CRGBA(255,255,255), bool Shadow=false) :
					CViewText (id, std::string(""), FontSize, Color, Shadow)
		{
			_StringModifier= NULL;
			_IsDBLink = false;
			_TextId = nID;
			_Initialized = false;
			_DynamicString = false;
			_IsTextFormatTaged= false;
		}

		// ctor with a db path entry
		CViewTextID (const std::string& id,
					 const std::string &idDBPath,
					 sint FontSize=12,
					 NLMISC::CRGBA Color=NLMISC::CRGBA(255,255,255),
					 bool Shadow=false);


		~CViewTextID();

		std::string getProperty( const std::string &name ) const;
		void setProperty( const std::string &name, const std::string &value );
		xmlNodePtr serialize( xmlNodePtr parentNode, const char *type ) const;
		virtual bool parse(xmlNodePtr cur, CInterfaceGroup * parentGroup);
		virtual void checkCoords();

		bool	parseTextIdOptions(xmlNodePtr cur);

		uint32 getTextId () const;
		void setTextId (uint32 id);

		/** set a text id from a db path
		  * \return true if the link could be done
		  */
		bool setDBTextID(const std::string &dbPath);
		// set a text from a db leaf
		void setDBLeaf(NLMISC::CCDBNodeLeaf *leaf);

		std::string getTextIdDbLink() const;
		void setTextIdDbLink(const std::string &link);

		void	setDynamicString(bool state) {_DynamicString= state;}
		bool	getDynamicString() const {return _DynamicString;}

		// modify name when received
		void				setOnReceiveTextId(IOnReceiveTextId *callBack) {_StringModifier= callBack;}
		IOnReceiveTextId	*getOnReceiveTextId() const {return _StringModifier;}

		REFLECT_EXPORT_START(CViewTextID, CViewText)
			REFLECT_UINT32("textid", getTextId, setTextId);
			REFLECT_STRING("textid_dblink", getTextIdDbLink, setTextIdDbLink);
		REFLECT_EXPORT_END

		static void setTextProvider( IViewTextProvider *provider ){ textProvider = provider; }

	protected:

		bool					_IsDBLink;
		CInterfaceProperty		_DBTextId;
		uint32					_TextId;

		bool					_Initialized;

		// If true, use a dynamic string (CStringManagerClient::getDynString), else use a server string id (CStringManagerClient::getString)
		bool					_DynamicString;

		// If true, setTextFormatted() is used instead of setText()
		bool					_IsTextFormatTaged;

		// Optional utf-8 string modifier
		IOnReceiveTextId		*_StringModifier;
		std::string				_DBPath;
		static IViewTextProvider* getTextProvider(){ return textProvider; }

	private:
		static IViewTextProvider *textProvider;

	};

}

#endif // NL_VIEW_TEXT_ID_H

/* End of view_text_id.h */
