// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010-2019  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
// Copyright (C) 2019-2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include "nel/misc/types_nl.h"
#include "nel/gui/interface_group.h"
#include "nel/gui/group_scrolltext.h"
#include "nel/gui/group_tree.h"
#include "nel/gui/ctrl_button.h"
#include "nel/gui/group_table.h"
#include "nel/gui/html_element.h"
#include "nel/gui/html_parser.h"
#include "nel/gui/css_style.h"

// forward declaration
typedef void CURLM;

namespace NLGUI
{
	class CCtrlButton;
	class CCtrlTextButton;
	class CCtrlScroll;
	class CGroupList;
	class CGroupMenu;
	class CDBGroupComboBox;
	class CGroupParagraph;

	extern std::string CurrentCookie;

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

			SWebOptions(): curlMaxConnections(5)
			{
			}
		};

		static SWebOptions options;

		// ImageDownload system
		enum TDataType {ImgType= 0, BnpType, StylesheetType};
		enum TImageType {NormalImage=0, OverImage};

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

		// load css from local file and insert into active stylesheet collection
		void parseStylesheetFile(const std::string &fname);

		// parse html string using libxml2 parser
		bool parseHtml(const std::string &htmlString);

		// Refresh
		void refresh();

		// submit form
		void submitForm(uint button, sint32 x, sint32 y);

		// Browse error
		void browseError (const char *msg);

		bool isBrowsing();

		// Update coords
		void updateCoords();

		// New paragraph
		void newParagraph(uint beginSpace);

		// End of the paragraph
		void endParagraph();

		// add image download (used by view_bitmap.cpp to load web images)
		void addImageDownload(const std::string &url, CViewBase *img, const CStyleParams &style = CStyleParams(), const TImageType type = NormalImage, const std::string &placeholder = "web_del.tga");
		// remove image from download list if present
		void removeImageDownload(CViewBase *img);
		std::string localImageName(const std::string &url);

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
		std::string		DefaultFormSelectBoxMenuGroup;
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
		// Get Home URL
		virtual std::string	home() const;

		// Undo browse: Browse the precedent url browsed. no op if none
		void browseUndo ();
		// Redo browse: Browse the precedent url undoed. no op if none
		void browseRedo ();
		// disable refresh button
		void clearRefresh();
		// clear undo/redo
		void clearUndoRedo();


		std::string getURL() const { return _URL; }
		void		setURL(const std::string &url);

		std::string getHTML() const { return _DocumentHtml; }
		void		setHTML(const std::string &html);

		void		setHome(const std::string &home);

		int luaClearRefresh(CLuaState &ls);
		int luaClearUndoRedo(CLuaState &ls);
		int luaBrowse(CLuaState &ls);
		int luaRefresh(CLuaState &ls);
		int luaRemoveContent(CLuaState &ls);
		int luaInsertText(CLuaState &ls);
		int luaAddString(CLuaState &ls);
		int luaAddImage(CLuaState &ls);
		int luaShowDiv(CLuaState &ls);
		int luaParseHtml(CLuaState &ls);
		int luaRenderHtml(CLuaState &ls);
		int luaSetBackground(CLuaState &ls);

		REFLECT_EXPORT_START(CGroupHTML, CGroupScrollText)
			REFLECT_LUA_METHOD("browse", luaBrowse)
			REFLECT_LUA_METHOD("refresh", luaRefresh)
			REFLECT_LUA_METHOD("clearUndoRedo", luaClearUndoRedo)
			REFLECT_LUA_METHOD("clearRefresh", luaClearRefresh)
			REFLECT_LUA_METHOD("removeContent", luaRemoveContent)
			REFLECT_LUA_METHOD("insertText", luaInsertText)
			REFLECT_LUA_METHOD("addString", luaAddString)
			REFLECT_LUA_METHOD("addImage", luaAddImage)
			REFLECT_LUA_METHOD("showDiv", luaShowDiv)
			REFLECT_LUA_METHOD("parseHtml", luaParseHtml)
			REFLECT_LUA_METHOD("renderHtml", luaRenderHtml)
			REFLECT_LUA_METHOD("setBackground", luaSetBackground)
			REFLECT_STRING("url", getURL, setURL)
			REFLECT_STRING("html", getHTML, setHTML)
			REFLECT_STRING("home", home, setHome)
			REFLECT_FLOAT("timeout", getTimeout, setTimeout)
			REFLECT_STRING("title", getTitle, setTitle)
		REFLECT_EXPORT_END

	protected :

		// \name callback from libwww

		// Begin of the rendering of a HTML document
		virtual void beginBuild ();

		// End of the rendering of a HTML document
		virtual void endBuild ();

		// A new text block has been parsed
		virtual void addText (const char * buf, int len);

		// A new begin HTML element has been parsed (<IMG> for exemple)
		virtual void beginElement(CHtmlElement &elm);

		// A new end HTML element has been parsed (</IMG> for exemple)
		virtual void endElement(CHtmlElement &elm);

		// Add GET params to the url
		virtual void addHTTPGetParams (std::string &url, bool trustedDomain);

		// Add POST params to the libwww list
		virtual void addHTTPPostParams (SFormFields &formfields, bool trustedDomain);

		// parse dom node and all child nodes recursively
		void renderDOM(CHtmlElement &elm);

		// Clear style stack and restore default style
		void resetCssStyle();

		// Parse style html tag
		TStyle parseStyle(const std::string &str_styles);

		// Handle some work at each pass
		virtual void handle ();

		// \name internal methods

		// Add a group in the current parent group
		void addHtmlGroup (CInterfaceGroup *group, uint beginSpace);

		// Get the current parent group
		CInterfaceGroup *getCurrentGroup();

		// Update current paragraph dependent data
		void paragraphChange ();

		// Clear the contexts info
		void clearContext();

		// Translate a char
		bool translateChar(u32char &output, u32char input, u32char lastChar) const;

		// Add a string in the current paragraph
		void addString(const std::string &str);

		// Add an image in the current paragraph
		void addImage(const std::string &id, const std::string &img, bool reloadImg=false, const CStyleParams &style = CStyleParams());

		// Add a text area in the current paragraph
		CInterfaceGroup *addTextArea (const std::string &templateName, const char *name, uint rows, uint cols, bool multiLine, const std::string &content, uint maxlength);

		// Add a combo box in the current paragraph
		CDBGroupComboBox *addComboBox(const std::string &templateName, const char *name);
		CGroupMenu *addSelectBox(const std::string &templateName, const char *name);

		// Add a button in the current paragraph. actionHandler, actionHandlerParams and tooltip can be NULL.
		CCtrlButton *addButton(CCtrlButton::EType type, const std::string &name, const std::string &normalBitmap, const std::string &pushedBitmap,
			const std::string &overBitmap, const char *actionHandler, const char *actionHandlerParams, const std::string &tooltip,
			const CStyleParams &style = CStyleParams());

		// Set the background color
		void setBackgroundColor (const NLMISC::CRGBA &bgcolor);

		// Set the background
		void setBackground (const std::string &bgtex, bool scale, bool tile);

		// Force the current string to be in a single string
		void flushString();

		// Set the title
		void setTitle (const std::string &title);
		std::string getTitle() const;
		void setContainerTitle (const std::string &title);

		// Lookup a url in local file system
		bool lookupLocalFile (std::string &result, const char *url, bool isUrl);

		// Delete page content and prepare next page
		void removeContent ();

		// Counter to number html elements without id attribute
		uint32			getNextAutoIdSeq() { return _AutoIdSeq++; }
		uint32			_AutoIdSeq;

		// Current URL for relative links in page
		std::string		_URL;
		// Current URL
		std::string		_DocumentUrl;
		std::string		_DocumentDomain;
		std::string		_DocumentHtml; // not updated only set by first render
		// If true, then render _DocumentHtml on next update (replaces content)
		bool			_RenderNextTime;
		// true if renderer is waiting for css files to finish downloading (link rel=stylesheet)
		bool			_WaitingForStylesheet;
		// list of css file urls that are queued up for download
		std::vector<CHtmlParser::StyleLink> _StylesheetQueue;
		// <style> and downloaded <link rel=stylesheet> elements
		std::vector<std::string> _HtmlStyles;

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
		std::string		_TitlePrefix;

		// Title string
		std::string		_TitleString;

		// Need to browse next update coords..
		bool			_BrowseNextTime;
		bool			_PostNextTime;
		uint			_PostFormId;
		std::string		_PostFormAction;
		std::string		_PostFormSubmitType;
		std::string		_PostFormSubmitButton;
		std::string		_PostFormSubmitValue;
		sint32			_PostFormSubmitX;
		sint32			_PostFormSubmitY;

		// Browsing..
		bool			_Browsing;
		double			_TimeoutValue;			// the timeout in seconds
		double			_ConnectingTimeout;
		sint			_RedirectsRemaining;
		// Automatic page refresh
		double			_LastRefreshTime;
		double			_NextRefreshTime;
		std::string		_RefreshUrl;

		// minimal embeded lua script support
		// Note : any embeded script is executed immediately after the closing
		// element has been found
		// True when the <lua> element has been encountered
		bool			_ParsingLua;
		bool			_IgnoreText;
		bool			_IgnoreChildElements;
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

		class HTMLMeterElement {
		public:
			enum EValueRegion {
				VALUE_OPTIMUM = 0,
				VALUE_SUB_OPTIMAL,
				VALUE_EVEN_LESS_GOOD
			};
		public:
			HTMLMeterElement()
				: value(0.f), min(0.f), max(1.f), low(0.f), high(1.f), optimum(0.5f)
			{}

			// read attributes from html element
			void readValues(const CHtmlElement &elm);

			// return value ratio to min-max
			float getValueRatio() const;

			// return optimum region based current value
			EValueRegion getValueRegion() const;

			// return meter bar color
			NLMISC::CRGBA getBarColor(const CHtmlElement &elm, CCssStyle &style) const;

			// return meter value bar color based value and optimum range
			NLMISC::CRGBA getValueColor(const CHtmlElement &elm, CCssStyle &style) const;

			float value;
			float min;
			float max;
			float low;
			float high;
			float optimum;
		};

		class HTMLProgressElement
		{
		public:
			HTMLProgressElement()
				: value(0.f), max(1.f)
			{}

			// read attributes from html element
			void readValues(const CHtmlElement &elm);

			// return value ratio to min-max
			float getValueRatio() const;

			// return meter bar color
			NLMISC::CRGBA getBarColor(const CHtmlElement &elm, CCssStyle &style) const;

			// return meter value bar color based value and optimum range
			NLMISC::CRGBA getValueColor(const CHtmlElement &elm, CCssStyle &style) const;

			float value;
			float max;
		};

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

		// style from browser.css
		CCssStyle _BrowserStyle;
		// local file for browser.css
		std::string _BrowserCssFile;

		// Keep track of current element style
		CCssStyle _Style;
		CHtmlElement _HtmlDOM;
		CHtmlElement *_CurrentHTMLElement;
		// Backup of CurrentHTMLElement->nextSibling before ::beginElement() is called
		// for luaParseHtml() to insert nodes into right place in right order
		CHtmlElement *_CurrentHTMLNextSibling;

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
					SelectBox = NULL;
					sbRBRef = NULL;
					sbMultiple = false;
					sbOptionDisabled = -1;
					InitialSelection = 0;
				}

				// Variable name
				std::string		Name;

				// Variable value
				std::string		Value;

				// Text area group
				CInterfaceGroup *TextArea;

				// Checkbox
				CCtrlButton *Checkbox;

				// Combobox group
				CDBGroupComboBox *ComboBox;

				// Combobox with multiple selection or display size >= 2
				CGroupMenu *SelectBox;

				// Single or multiple selections for SelectBox
				bool sbMultiple;

				// Marks OPTION element as disabled
				// Only valid when parsing html
				sint sbOptionDisabled;

				// First radio button in SelectBox if single selection
				CCtrlBaseButton *sbRBRef;

				// select values (for the <select> tag)
				std::vector<std::string> SelectValues;
				sint					 InitialSelection; // initial selection for the combo box
			};

			// <form> element "id" attribute
			std::string id;

			// The action the form has to perform
			std::string Action;

			// The text area associated with the form
			std::vector<CEntry>	Entries;
		};
		std::vector<CForm>	_Forms;
		// if <FORM> element has been closed or not
		bool				_FormOpen;

		// submit buttons added to from
		struct SFormSubmitButton
		{
			SFormSubmitButton(const std::string &form, const std::string &name, const std::string &value, const std::string &type, const std::string &formAction="")
				: form(form), name(name), value(value), type(type), formAction(formAction)
			{ }

			std::string form; // form 'id'
			std::string name; // submit button name
			std::string value; // submit button value
			std::string type; // button type, ie 'image'

			std::string formAction; // override form action attribute (url)
		};

		// submit buttons added to form
		std::vector<SFormSubmitButton> _FormSubmit;

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
				Height = 0;
			}
			NLMISC::CRGBA	BgColor;
			std::string		Style;
			CGroupCell::TAlign	Align;
			CGroupCell::TVAlign	VAlign;
			sint32	LeftMargin;
			bool	NoWrap;
			sint32	Height;
		};
		std::vector<CCellParams>	_CellParams;

		// Indentation
		std::vector<uint>	_Indent;
		inline uint getIndent() const {
			if (_Indent.empty())
				return 0;
			return _Indent.back();
		}



		// Current node is a title
		bool			_Title;

		// Current node must be localized
		bool			_Localize;

		// Current node is a text area
		bool			_TextArea;
		std::string		_TextAreaTemplate;
		std::string		_TextAreaContent;
		std::string		_TextAreaName;
		uint			_TextAreaRow;
		uint			_TextAreaCols;
		uint			_TextAreaMaxLength;

		// current mode is in select option
		bool			_SelectOption;
		std::string		_SelectOptionStr;

		// Current node is a object
		std::string		_ObjectType;
		std::string		_ObjectData;
		std::string		_ObjectMD5Sum;
		std::string		_ObjectAction;
		std::string		_TextAreaScript;

		// Get last char
		u32char getLastChar() const;

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

		// load and render local html file (from bnp for example)
		void doBrowseLocalFile(const std::string &filename);

		// load remote content using either GET or POST
		void doBrowseRemoteUrl(std::string url, const std::string &referer, bool doPost = false, const SFormFields &formfields = SFormFields());

		// render html string as new browser page
		bool renderHtmlString(const std::string &html);

		// initialize formfields list from form elements on page
		void buildHTTPPostParams (SFormFields &formfields);

	private:
		friend class CHtmlParser;

		// move src->Children into CurrentHtmlElement.parent.children element
		void spliceFragment(std::list<CHtmlElement>::iterator src);

		// decode all HTML entities
		static std::string decodeHTMLEntities(const std::string &str);

		struct CDataImageDownload
		{
		public:
			CDataImageDownload(CViewBase *img, CStyleParams style, TImageType type): Image(img), Style(style), Type(type)
			{
			}
		public:
			CViewBase * Image;
			CStyleParams Style;
			TImageType Type;
		};

		struct CDataDownload
		{
		public:
			CDataDownload(const std::string &u, const std::string &d, TDataType t, CViewBase *i, const std::string &s, const std::string &m, const CStyleParams &style = CStyleParams(), const TImageType imagetype = NormalImage)
				: data(NULL), fp(NULL), url(u), dest(d), type(t), luaScript(s), md5sum(m), redirects(0), ConnectionTimeout(60)
			{
				if (t == ImgType) imgs.push_back(CDataImageDownload(i, style, imagetype));
			}
			~CDataDownload();

		public:
			CCurlWWWData *data;
			std::string url;
			std::string dest;
			std::string luaScript;
			std::string md5sum;
			TDataType type;
			uint32 redirects;
			FILE *fp;
			std::vector<CDataImageDownload> imgs;
			uint32 ConnectionTimeout;
		};

		std::list<CDataDownload> Curls;
		CURLM *MultiCurl;
		int RunningCurls;

		bool startCurlDownload(CDataDownload &download);
		void finishCurlDownload(const CDataDownload &download);
		void pumpCurlQueue();

		void initImageDownload();
		void checkImageDownload();
		std::string getAbsoluteUrl(const std::string &url);

		bool isTrustedDomain(const std::string &domain);
		void setImage(CViewBase *view, const std::string &file, const TImageType type);
		void setImageSize(CViewBase *view, const CStyleParams &style = CStyleParams());

		void setTextButtonStyle(CCtrlTextButton *ctrlButton, const CStyleParams &style);
		void setTextStyle(CViewText *pVT, const CStyleParams &style);

		// BnpDownload system
		void initBnpDownload();
		void checkBnpDownload();
		bool addBnpDownload(std::string url, const std::string &action, const std::string &script, const std::string &md5sum);
		std::string localBnpName(const std::string &url);

		// add css file from <link href=".." rel="stylesheet"> to download queue
		void addStylesheetDownload(std::vector<CHtmlParser::StyleLink> links);

		// stop all curl downalods (html and data)
		void releaseDownloads();
		void checkDownloads();

		// _CurlWWW download finished
		void htmlDownloadFinished(bool success, const std::string &error);
		// images, stylesheets, etc finished downloading
		void dataDownloadFinished(bool success, const std::string &error, CDataDownload &data);

		// HtmlType download finished
		void htmlDownloadFinished(const std::string &content, const std::string &type, long code);

		// stylesheet finished downloading. if local file does not exist, then it failed (404)
		void cssDownloadFinished(const std::string &url, const std::string &local);

		// read common table/tr/td parameters and push them to _CellParams
		void getCellsParameters(const CHtmlElement &elm, bool inherit);

		// render _HtmlDOM
		void renderDocument();

		// :before, :after rendering
		void renderPseudoElement(const std::string &pseudo, const CHtmlElement &elm);

		// apply background from current style (for html, body)
		void applyBackground(const CHtmlElement &elm);

		void insertFormImageButton(const std::string &name,
			const std::string &tooltip,
			const std::string &src,
			const std::string &over,
			const std::string &formId,
			const std::string &formAction = "",
			uint32 minWidth = 0,
			const std::string &templateName = "");

		void insertFormTextButton(const std::string &name,
			const std::string &tooltip,
			const std::string &value,
			const std::string &formId,
			const std::string &formAction = "",
			uint32 minWidth = 0,
			const std::string &templateName = "");

		// HTML elements
		void htmlA(const CHtmlElement &elm);
		void htmlAend(const CHtmlElement &elm);
		void htmlBASE(const CHtmlElement &elm);
		void htmlBODY(const CHtmlElement &elm);
		void htmlBR(const CHtmlElement &elm);
		void htmlBUTTON(const CHtmlElement &elm);
		void htmlBUTTONend(const CHtmlElement &elm);
		void htmlDD(const CHtmlElement &elm);
		void htmlDDend(const CHtmlElement &elm);
		//void htmlDEL(const CHtmlElement &elm);
		void htmlDIV(const CHtmlElement &elm);
		void htmlDIVend(const CHtmlElement &elm);
		void htmlDL(const CHtmlElement &elm);
		void htmlDLend(const CHtmlElement &elm);
		void htmlDT(const CHtmlElement &elm);
		void htmlDTend(const CHtmlElement &elm);
		//void htmlEM(const CHtmlElement &elm);
		void htmlFONT(const CHtmlElement &elm);
		void htmlFORM(const CHtmlElement &elm);
		void htmlFORMend(const CHtmlElement &elm);
		void htmlH(const CHtmlElement &elm);
		void htmlHend(const CHtmlElement &elm);
		void htmlHEAD(const CHtmlElement &elm);
		void htmlHEADend(const CHtmlElement &elm);
		void htmlHR(const CHtmlElement &elm);
		void htmlHTML(const CHtmlElement &elm);
		void htmlI(const CHtmlElement &elm);
		void htmlIend(const CHtmlElement &elm);
		void htmlIMG(const CHtmlElement &elm);
		void htmlINPUT(const CHtmlElement &elm);
		void htmlLI(const CHtmlElement &elm);
		void htmlLIend(const CHtmlElement &elm);
		void htmlLUA(const CHtmlElement &elm);
		void htmlLUAend(const CHtmlElement &elm);
		void htmlMETA(const CHtmlElement &elm);
		void htmlMETER(const CHtmlElement &elm);
		void htmlOBJECT(const CHtmlElement &elm);
		void htmlOBJECTend(const CHtmlElement &elm);
		void htmlOL(const CHtmlElement &elm);
		void htmlOLend(const CHtmlElement &elm);
		void htmlOPTION(const CHtmlElement &elm);
		void htmlOPTIONend(const CHtmlElement &elm);
		void htmlP(const CHtmlElement &elm);
		void htmlPend(const CHtmlElement &elm);
		void htmlPRE(const CHtmlElement &elm);
		void htmlPREend(const CHtmlElement &elm);
		void htmlPROGRESS(const CHtmlElement &elm);
		void htmlSCRIPT(const CHtmlElement &elm);
		void htmlSCRIPTend(const CHtmlElement &elm);
		void htmlSELECT(const CHtmlElement &elm);
		void htmlSELECTend(const CHtmlElement &elm);
		//void htmlSMALL(const CHtmlElement &elm);
		//void htmlSPAN(const CHtmlElement &elm);
		//void htmlSTRONG(const CHtmlElement &elm);
		void htmlSTYLE(const CHtmlElement &elm);
		void htmlSTYLEend(const CHtmlElement &elm);
		void htmlTABLE(const CHtmlElement &elm);
		void htmlTABLEend(const CHtmlElement &elm);
		void htmlTD(const CHtmlElement &elm);
		void htmlTDend(const CHtmlElement &elm);
		void htmlTEXTAREA(const CHtmlElement &elm);
		void htmlTEXTAREAend(const CHtmlElement &elm);
		void htmlTH(const CHtmlElement &elm);
		void htmlTHend(const CHtmlElement &elm);
		void htmlTITLE(const CHtmlElement &elm);
		void htmlTITLEend(const CHtmlElement &elm);
		void htmlTR(const CHtmlElement &elm);
		void htmlTRend(const CHtmlElement &elm);
		//void htmlU(const CHtmlElement &elm);
		void htmlUL(const CHtmlElement &elm);
		void htmlULend(const CHtmlElement &elm);
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
