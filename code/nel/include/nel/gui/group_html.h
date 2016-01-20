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

#ifndef CL_GROUP_HTML_H
#define CL_GROUP_HTML_H

#include <curl/curl.h>

#include "nel/misc/types_nl.h"
#include "nel/gui/interface_group.h"
#include "nel/gui/group_scrolltext.h"
#include "nel/gui/group_tree.h"
#include "nel/gui/ctrl_button.h"
#include "nel/gui/group_table.h"
#include "nel/gui/libwww_types.h"

typedef std::map<std::string, std::string>	TStyle;

namespace NLGUI
{
	class CCtrlButton;
	class CCtrlScroll;
	class CGroupList;
	class CDBGroupComboBox;
	class CGroupParagraph;

	extern std::string CurrentCookie;
	extern std::string HTTPCurrentDomain;

	// HTML group
	/**
	 * Widget to have a resizable scrolltext and its scrollbar
	 * \author Cyril 'Hulud' Corvazier
	 * \author Nevrax France
	 * \date 2002
	 */
	class CGroupHTML : public CGroupScrollText
	{
	public:
        DECLARE_UI_CLASS( CGroupHTML )

		/// Web browser options for CGroupHTML
		struct SWebOptions
		{
		public:
			/// Id of the browser ( e.g.: Chrome, Firefox, Ryzom )
			std::string appName;
			/// Version of the browser
			std::string appVersion;
			/// Language code of the browser( e.g.: en, hu )
			std::string languageCode;
			/// List of domains the widget can consider secure.
			std::vector< std::string > trustedDomains;
			/// Maximum concurrent MultiCurl connections per CGroupHTML instance
			sint32 curlMaxConnections;

			SWebOptions(): curlMaxConnections(2)
			{
			}
		};

		static SWebOptions options;
		
		class CStyleParams
		{
		public:
			CStyleParams () : TextColor(255,255,255,255)
			{
				FontSize=10;
				FontWeight=400;
				FontOblique=false;
				Underlined=false;
				StrikeThrough=false;
				Width=-1;
				Height=-1;
				MaxWidth=-1;
				MaxHeight=-1;
			}
			uint FontSize;
			uint FontWeight;
			bool FontOblique;
			NLMISC::CRGBA TextColor;
			bool Underlined;
			bool StrikeThrough;
			sint32 Width;
			sint32 Height;
			sint32 MaxWidth;
			sint32 MaxHeight;
		};

		// Constructor
		CGroupHTML(const TCtorParam &param);
		~CGroupHTML();

		std::string getProperty( const std::string &name ) const;
		void setProperty( const std::string &name, const std::string &value );
		xmlNodePtr serialize( xmlNodePtr parentNode, const char *type ) const;

		// CInterfaceGroup Interface
		virtual bool parse (xmlNodePtr cur, CInterfaceGroup *parentGroup);
		virtual void draw ();

		// Events
		virtual bool handleEvent (const NLGUI::CEventDescriptor& eventDesc);

		// Browse
		virtual void browse (const char *url);

		// parse html string using libxml2 parser
		virtual bool parseHtml(std::string htmlString);

		// Refresh
		void refresh();

		// submit form
		void submitForm (uint formId, const char *submitButtonType, const char *submitButtonName, const char *submitButtonValue, sint32 x, sint32 y);

		// Browse error
		void browseError (const char *msg);

		// stop browse
		void stopBrowse ();

		bool isBrowsing();

		void clean() { stopBrowse(); updateRefreshButton(); removeContent(); }

		// Update coords
		void updateCoords();

		// New paragraph
		void newParagraph(uint beginSpace);

		// End of the paragraph
		void endParagraph();

		// Timeout
		void	setTimeout(float tm) {_TimeoutValue= std::max(0.f, tm);}
		float	getTimeout() const {return (float)_TimeoutValue;}

		// Some constants
		NLMISC::CRGBA	BgColor;
		NLMISC::CRGBA	ErrorColor;
		NLMISC::CRGBA	LinkColor;
		NLMISC::CRGBA	TextColor;
		NLMISC::CRGBA	H1Color;
		NLMISC::CRGBA	H2Color;
		NLMISC::CRGBA	H3Color;
		NLMISC::CRGBA	H4Color;
		NLMISC::CRGBA	H5Color;
		NLMISC::CRGBA	H6Color;
		bool			ErrorColorGlobalColor;
		bool			LinkColorGlobalColor;
		bool			TextColorGlobalColor;
		bool			H1ColorGlobalColor;
		bool			H2ColorGlobalColor;
		bool			H3ColorGlobalColor;
		bool			H4ColorGlobalColor;
		bool			H5ColorGlobalColor;
		bool			H6ColorGlobalColor;
		uint			TextFontSize;
		uint			H1FontSize;
		uint			H2FontSize;
		uint			H3FontSize;
		uint			H4FontSize;
		uint			H5FontSize;
		uint			H6FontSize;
		uint			TDBeginSpace;
		uint			PBeginSpace;
		uint			LIBeginSpace;
		uint			ULBeginSpace;
		uint			LIIndent;
		uint			ULIndent;
		float			LineSpaceFontFactor;
		std::string		DefaultButtonGroup;
		std::string		DefaultFormTextGroup;
		std::string		DefaultFormTextAreaGroup;
		std::string		DefaultFormSelectGroup;
		std::string		DefaultCheckBoxBitmapNormal;
		std::string		DefaultCheckBoxBitmapPushed;
		std::string		DefaultCheckBoxBitmapOver;
		std::string		DefaultRadioButtonBitmapNormal;
		std::string		DefaultRadioButtonBitmapPushed;
		std::string		DefaultRadioButtonBitmapOver;
		std::string		DefaultBackgroundBitmapView;
		std::string		CurrentLinkTitle;

		struct TFormField {
		public:
			TFormField(const std::string &k, const std::string &v)
				:name(k),value(v)
			{}
			std::string name;
			std::string value;
		};

		struct SFormFields {
		public:
			SFormFields()
			{
			}

			void clear()
			{
				Values.clear();
			}

			void add(const std::string &key, const std::string &value)
			{
				Values.push_back(TFormField(key, value));
			}

			std::vector<TFormField> Values;
		};

		// Browser home
		std::string		Home;

		// Undo browse: Browse the precedent url browsed. no op if none
		void browseUndo ();
		// Redo browse: Browse the precedent url undoed. no op if none
		void browseRedo ();
		// clear undo/redo
		void clearUndoRedo();


		std::string getURL() const { return _URL; }
		void		setURL(const std::string &url);


		int luaBrowse(CLuaState &ls);
		int luaRefresh(CLuaState &ls);
		int luaRemoveContent(CLuaState &ls);
		int luaInsertText(CLuaState &ls);
		int luaAddString(CLuaState &ls);
		int luaAddImage(CLuaState &ls);
		int luaBeginElement(CLuaState &ls);
		int luaEndElement(CLuaState &ls);
		int luaShowDiv(CLuaState &ls);
		int luaParseHtml(CLuaState &ls);

		REFLECT_EXPORT_START(CGroupHTML, CGroupScrollText)
			REFLECT_LUA_METHOD("browse", luaBrowse)
			REFLECT_LUA_METHOD("refresh", luaRefresh)
			REFLECT_LUA_METHOD("removeContent", luaRemoveContent)
			REFLECT_LUA_METHOD("insertText", luaInsertText)
			REFLECT_LUA_METHOD("addString", luaAddString)
			REFLECT_LUA_METHOD("addImage", luaAddImage)
			REFLECT_LUA_METHOD("beginElement", luaBeginElement)
			REFLECT_LUA_METHOD("endElement", luaEndElement)
			REFLECT_LUA_METHOD("showDiv", luaShowDiv)
			REFLECT_LUA_METHOD("parseHtml", luaParseHtml)
			REFLECT_STRING("url", getURL, setURL)
			REFLECT_FLOAT("timeout", getTimeout, setTimeout)
		REFLECT_EXPORT_END

	protected :

		// \name callback from libwww

		// Begin of the parsing of a HTML document
		virtual void beginBuild ();

		// End of the parsing of a HTML document
		virtual void endBuild ();

		// A new text block has been parsed
		virtual void addText (const char * buf, int len);

		// A link has been parsed
		virtual void addLink (uint element_number, const std::vector<bool> &present, const std::vector<const char *> &value);

		// A new begin HTML element has been parsed (<IMG> for exemple)
		virtual void beginElement (uint element_number, const std::vector<bool> &present, const std::vector<const char *> &value);

		// A new end HTML element has been parsed (</IMG> for exemple)
		virtual void endElement (uint element_number);

		// A new begin unparsed element has been found
		virtual void beginUnparsedElement(const char *buffer, int length);

		// A new end unparsed element has been found
		virtual void endUnparsedElement(const char *buffer, int length);

		// Add GET params to the url
		virtual void addHTTPGetParams (std::string &url, bool trustedDomain);

		// Add POST params to the libwww list
		virtual void addHTTPPostParams (SFormFields &formfields, bool trustedDomain);

		// the current request is terminated
		virtual void requestTerminated();

		// libxml2 html parser functions
		void htmlElement(xmlNode *node, int element_number);
		void htmlWalkDOM(xmlNode *a_node);

		// Get Home URL
		virtual std::string	home();

		// Parse style html tag
		TStyle parseStyle(const std::string &str_styles);

		// Handle some work at each pass
		virtual void handle ();

		// \name internal methods

		// Add a group in the current parent group
		void addGroup (CInterfaceGroup *group, uint beginSpace);

		// Get the current parent group
		CInterfaceGroup *getCurrentGroup();

		// Update current paragraph dependent data
		void paragraphChange ();

		// Clear the contexts info
		void clearContext();

		// Translate a char
		bool translateChar(ucchar &output, ucchar input, ucchar lastChar) const;

		// Add a string in the current paragraph
		void addString(const ucstring &str);

		// Add an image in the current paragraph
		void addImage(const char *image, bool globalColor, bool reloadImg=false, const CStyleParams &style = CStyleParams());

		// Add a text area in the current paragraph
		CInterfaceGroup *addTextArea (const std::string &templateName, const char *name, uint rows, uint cols, bool multiLine, const ucstring &content, uint maxlength);

		// Add a combo box in the current paragraph
		CDBGroupComboBox *addComboBox(const std::string &templateName, const char *name);

		// Add a button in the current paragraph. actionHandler, actionHandlerParams and tooltip can be NULL.
		CCtrlButton *addButton(CCtrlButton::EType type, const std::string &name, const std::string &normalBitmap, const std::string &pushedBitmap,
			const std::string &overBitmap, bool useGlobalColor, const char *actionHandler, const char *actionHandlerParams, const char *tooltip,
			const CStyleParams &style = CStyleParams());

		// Set the background color
		void setBackgroundColor (const NLMISC::CRGBA &bgcolor);

		// Set the background
		void setBackground (const std::string &bgtex, bool scale, bool tile);

		// Force the current string to be in a single string
		void flushString();

		// Set the title
		void setTitle (const ucstring &title);

		// Lookup a url in local file system
		bool lookupLocalFile (std::string &result, const char *url, bool isUrl);

		// Delete page content and prepare next page
		void removeContent ();

		// Current URL for relative links in page
		std::string		_URL;
		// Current URL
		std::string		_DocumentUrl;
		// Valid base href was found
		bool            _IgnoreBaseUrlTag;
		// Fragment from loading url
		std::string		_UrlFragment;
		std::map<std::string,NLGUI::CInterfaceElement *> _Anchors;
		std::vector<std::string> _AnchorName;

		// Parser context
		bool			_ReadingHeadTag;
		bool			_IgnoreHeadTag;

		// Current DOMAIN
		bool			_TrustedDomain;

		// Title prefix
		ucstring		_TitlePrefix;

		// Title string
		ucstring		_TitleString;

		// Need to browse next update coords..
		bool			_BrowseNextTime;
		bool			_PostNextTime;
		uint			_PostFormId;
		std::string		_PostFormSubmitType;
		std::string		_PostFormSubmitButton;
		std::string		_PostFormSubmitValue;
		sint32			_PostFormSubmitX;
		sint32			_PostFormSubmitY;

		// Browsing..
		bool			_Browsing;
		bool			_Connecting;
		double			_TimeoutValue;			// the timeout in seconds
		double			_ConnectingTimeout;
		sint			_RedirectsRemaining;

		// minimal embeded lua script support
		// Note : any embeded script is executed immediately after the closing
		// element has been found
		// True when the <lua> element has been encountered
		bool			_ParsingLua;
		bool			_IgnoreText;
		// the script to execute
		std::string		_LuaScript;
		bool			_LuaHrefHack;

		bool			_Object;
		std::string		_ObjectScript;

		// Data container for active curl transfer
		class CCurlWWWData *	_CurlWWW;

		// Current paragraph
		std::string		_DivName;
		CGroupParagraph*	_Paragraph;
		inline CGroupParagraph *getParagraph()
		{
			return _Paragraph;
			/*if (_Paragraph.empty())
				return NULL;
			return _Paragraph.back();*/
		}

		// PRE mode
		std::vector<bool>	_PRE;
		inline bool getPRE() const
		{
			if (_PRE.empty())
				return false;
			return _PRE.back();
		}

		// DL list
		class HTMLDListElement {
		public:
			HTMLDListElement()
				: DT(false), DD(false)
			{ }

		public:
			bool DT;
			bool DD;
		};
		std::vector<HTMLDListElement>	_DL;

		// OL and UL
		class HTMLOListElement {
		public:
			HTMLOListElement(int start, std::string type)
				: Value(start),Type(type), First(true)
			{ }

			std::string getListMarkerText() const;
		public:
			sint32 Value;
			std::string Type;
			bool First;
		};
		std::vector<HTMLOListElement> _UL;

		// A mode
		std::vector<bool>	_A;
		inline bool getA() const
		{
			if (_A.empty())
				return false;
			return _A.back();
		}

		// IL mode
		bool _LI;

		// Current text color
		std::vector<NLMISC::CRGBA>	_TextColor;
		inline const NLMISC::CRGBA &getTextColor() const
		{
			if (_TextColor.empty())
				return TextColor;
			return _TextColor.back();
		}

		// Current global color flag
		std::vector<bool>	_GlobalColor;
		inline bool getGlobalColor() const
		{
			if (_GlobalColor.empty())
				return false;
			return _GlobalColor.back();
		}

		// Current font size
		std::vector<uint>			_FontSize;
		inline uint getFontSize() const
		{
			if (_FontSize.empty())
				return TextFontSize;
			return _FontSize.back();
		}
		inline uint getFontSizeSmaller() const
		{
			if (getFontSize() < 5)
				return 3;
			return getFontSize()-2;
		}

		std::vector<uint>			_FontWeight;
		inline uint getFontWeight() const
		{
			if (_FontWeight.empty())
				return 400;
			return _FontWeight.back();
		}

		std::vector<bool>			_FontOblique;
		inline bool getFontOblique() const
		{
			if (_FontOblique.empty())
				return false;
			return _FontOblique.back();
		}

		std::vector<bool>			_FontUnderlined;
		inline bool getFontUnderlined() const
		{
			if (_FontUnderlined.empty())
				return false;
			return _FontUnderlined.back();
		}

		std::vector<bool>			_FontStrikeThrough;
		inline bool getFontStrikeThrough() const
		{
			if (_FontStrikeThrough.empty())
				return false;
			return _FontStrikeThrough.back();
		}

		// Current link
		std::vector<std::string>	_Link;
		inline const char *getLink() const
		{
			if (_Link.empty())
				return "";
			return _Link.back().c_str();
		}

		std::vector<std::string>	_LinkTitle;
		inline const char *getLinkTitle() const
		{
			if (_LinkTitle.empty())
				return "";
			return _LinkTitle.back().c_str();
		}
		std::vector<std::string>		_LinkClass;
		inline const char *getLinkClass() const
		{
			if (_LinkClass.empty())
				return "";
			return _LinkClass.back().c_str();
		}

		std::vector<bool>				_BlockLevelElement;
		inline bool isBlockLevelElement() const
		{
			if (_BlockLevelElement.empty())
				return false;
			return _BlockLevelElement.back();
		}

		// Divs (i.e. interface group)
		std::vector<class CInterfaceGroup*>	_Divs;
		inline CInterfaceGroup *getDiv() const
		{
			if (_Divs.empty())
				return NULL;
			return _Divs.back();
		}

		// Tables
		std::vector<class CGroupTable*>	_Tables;
		inline CGroupTable *getTable() const
		{
			if (_Tables.empty())
				return NULL;
			return _Tables.back();
		}

		// Cells
		std::vector<class CGroupCell*>	_Cells;

		// TR
		std::vector<bool>	_TR;
		inline bool getTR() const
		{
			if (_TR.empty())
				return false;
			return _TR.back();
		}

		// Forms
		class CForm
		{
		public:

			class CEntry
			{
			public:
				CEntry ()
				{
					TextArea = NULL;
					Checkbox = NULL;
					ComboBox = NULL;
					InitialSelection = 0;
				}

				// Variable name
				std::string		Name;

				// Variable value
				ucstring		Value;

				// Text area group
				CInterfaceGroup *TextArea;

				// Checkbox
				CCtrlButton *Checkbox;

				// Combobox group
				CDBGroupComboBox *ComboBox;

				// select values (for the <select> tag)
				std::vector<std::string> SelectValues;
				sint					 InitialSelection; // initial selection for the combo box
			};

			// The action the form has to perform
			std::string Action;

			// The text area associated with the form
			std::vector<CEntry>	Entries;
		};
		std::vector<CForm>	_Forms;
		std::vector<CInterfaceGroup *>	_Groups;

		// Cells parameters
		class CCellParams
		{
		public:
			CCellParams () : BgColor(0,0,0,0)
			{
				Align = CGroupCell::Left;
				VAlign = CGroupCell::Middle;
				LeftMargin = 0;
				NoWrap = false;
			}
			NLMISC::CRGBA	BgColor;
			std::string		Style;
			CGroupCell::TAlign	Align;
			CGroupCell::TVAlign	VAlign;
			sint32	LeftMargin;
			bool	NoWrap;
		};
		std::vector<CCellParams>	_CellParams;

		// Indentation
		uint	_Indent;

		// Current node is a title
		bool			_Title;

		// Current node must be localized
		bool			_Localize;

		// Current node is a text area
		bool			_TextArea;
		std::string		_TextAreaTemplate;
		ucstring		_TextAreaContent;
		std::string		_TextAreaName;
		uint			_TextAreaRow;
		uint			_TextAreaCols;
		uint			_TextAreaMaxLength;

		// current mode is in select option
		bool			_SelectOption;
		ucstring		_SelectOptionStr;

		// Current node is a object
		std::string		_ObjectType;
		std::string		_ObjectData;
		std::string		_ObjectMD5Sum;
		std::string		_ObjectAction;
		std::string		_TextAreaScript;

		// Get last char
		ucchar getLastChar() const;

		// Current link view
		class CViewLink			*_CurrentViewLink;
		class CViewBitmap		*_CurrentViewImage;

		// Current group table
		class CGroupCell *_CurrentCell;

		// The main group
		class CGroupListAdaptor *_GroupListAdaptor;

		// For auto selecting the node in a BrowseTree bound to this HTML web page
		std::string		_BrowseTree;
		// select the tree node that has the correct url
		const std::string &selectTreeNodeRecurs(CGroupTree::SNode *node, const std::string &url);
		// search if the action / params match the url. look recurs into procedures
		bool	actionLaunchUrlRecurs(const std::string &ah, const std::string &params, const std::string &url);

		void	registerAnchor(CInterfaceElement* elm);

		// Browse undo and redo
		enum	{MaxUrlUndoRedo= 256};
		std::string		_BrowseUndoButton;
		std::string		_BrowseRedoButton;
		std::string		_BrowseRefreshButton;
		// _BrowseUrl is different from _URL, in that _URL may change in handle()
		std::string		_AskedUrl;
		std::deque<std::string>		_BrowseUndo;
		std::deque<std::string>		_BrowseRedo;
		void	pushUrlUndoRedo(const std::string &url);
		void	doBrowse(const char *url, bool force = false);
		void	doBrowseAnchor(const std::string &anchor);
		void	updateUndoRedoButtons();
		void	updateRefreshButton();

		// For Killing request. Associate each CGroupHTML object with a unique ID.
		uint32				_GroupHtmlUID;
		static uint32		_GroupHtmlUIDPool;
		typedef std::map<uint32, NLMISC::CRefPtr<CGroupHTML> >	TGroupHtmlByUIDMap;
		static TGroupHtmlByUIDMap _GroupHtmlByUID;

		// read style attribute
		void getStyleParams(const std::string &styleString, CStyleParams &style, bool inherit = true);
		void applyCssMinMax(sint32 &width, sint32 &height, sint32 minw=0, sint32 minh=0, sint32 maxw=0, sint32 maxh=0);

		// load and render local html file (from bnp for example)
		void doBrowseLocalFile(const std::string &filename);

		// load remote content using either GET or POST
		void doBrowseRemoteUrl(const std::string &url, const std::string &referer, bool doPost = false, const SFormFields &formfields = SFormFields());

		// render html string as new browser page
		bool renderHtmlString(const std::string &html);

		// initialize formfields list from form elements on page
		void buildHTTPPostParams (SFormFields &formfields);

	private:
		// decode all HTML entities
		static ucstring decodeHTMLEntities(const ucstring &str);

		// ImageDownload system
		enum TDataType {ImgType= 0, BnpType};
		
		struct CDataImageDownload
		{
		public:
			CDataImageDownload(CViewBase *img, CStyleParams style): Image(img), Style(style)
			{
			}
		public:
			CViewBase * Image;
			CStyleParams Style;
		};

		struct CDataDownload
		{
		public:
			CDataDownload(CURL *c, const std::string &u, const std::string &d, FILE *f, TDataType t, CViewBase *i, const std::string &s, const std::string &m, const CStyleParams &style = CStyleParams()) : curl(c), url(u), dest(d), luaScript(s), md5sum(m), type(t), fp(f)
			{
				if (t == ImgType) imgs.push_back(CDataImageDownload(i, style));
			}

		public:
			CURL *curl;
			std::string url;
			std::string dest;
			std::string luaScript;
			std::string md5sum;
			TDataType type;
			FILE *fp;
			std::vector<CDataImageDownload> imgs;
		};

		std::vector<CDataDownload> Curls;
		CURLM *MultiCurl;
		int RunningCurls;

		void initImageDownload();
		void checkImageDownload();
		void addImageDownload(const std::string &url, CViewBase *img, const CStyleParams &style = CStyleParams());
		std::string localImageName(const std::string &url);
		std::string getAbsoluteUrl(const std::string &url);

		bool isTrustedDomain(const std::string &domain);
		void setImage(CViewBase *view, const std::string &file);
		void setImageSize(CViewBase *view, const CStyleParams &style = CStyleParams());

		// BnpDownload system
		void initBnpDownload();
		void checkBnpDownload();
		bool addBnpDownload(const std::string &url, const std::string &action, const std::string &script, const std::string &md5sum);
		std::string localBnpName(const std::string &url);

		void releaseDownloads();
		void checkDownloads();

		// HtmlType download finished
		void htmlDownloadFinished(const std::string &content, const std::string &type, long code);

		// cURL transfer callbacks
		static size_t curlHeaderCallback(char *buffer, size_t size, size_t nmemb, void *pCCurlWWWData);
		static size_t curlDataCallback(char *buffer, size_t size, size_t nmemb, void *pCCurlWWWData);
		static size_t curlProgressCallback(void *pCCurlWWWData, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);
	};

	// adapter group that store y offset for inputs inside an html form
	class CGroupHTMLInputOffset : public CInterfaceGroup
	{
	public:
        DECLARE_UI_CLASS( CGroupHTMLInputOffset )

		sint32 Offset;
		CGroupHTMLInputOffset(const TCtorParam &param);
		xmlNodePtr serialize( xmlNodePtr parentNode, const char *type ) const;
		virtual bool parse (xmlNodePtr cur, CInterfaceGroup *parentGroup);
	};
}

#endif
