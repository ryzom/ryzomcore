
var g_theApp = parent.g_theApp;

var layerRef="";
var layerStyleRef = "";
var styleSwitch = "";
var FILEProtocol = "file://";
var HTTPProtocol = "http://";

if (navigator.appName == "Netscape")
	{
	layerStyleRef="layer.";
	layerRef="document.layers";
	styleSwitch="";
	}
else
	{
	layerStyleRef="layer.style.";
	layerRef="document.all";
	styleSwitch=".style";
	}

function CImage(id)
	{
	this.id = id;

	if ( g_theApp.isIE )
		this.image = eval('document.images.' + this.id);
	else
		this.image = eval('document.images["' + this.id + '"]');

	this.put_Source = SetSource;
	this.get_Source = GetSource;
	this.put_Title = put_Title;
	this.get_Title = get_Title;
	}

function SetSource(newsrc)
	{
	if ( this.image )
		this.image.src = newsrc;
	}

function GetSource()
	{
	if ( this.image )
		return this.image.src;
	}

function put_Title(txt)
	{
	if ( this.image && g_theApp.isIE )
		this.image.title = txt;
	}

function get_Title(txt)
	{
	if ( this.image && g_theApp.isIE )
		return this.image.title;
	}

function CDiv(id, doc)
	{
	this.id = id;
	this.layer = FindLayer(id, doc);

	this.Show = Show;
	this.Hide = Hide;
	this.IsHidden = IsHidden;
	this.put_innerHTML = put_innerHTML;
	}

function Show()
	{
	if ( this.layer )
		eval('this.' + layerStyleRef + 'visibility' + '= "visible"');
	}

function Hide()
	{
	if ( this.layer )
		eval('this.' + layerStyleRef + 'visibility' + '= "hidden"');
	}

function IsHidden()
	{
	if ( this.layer && 
		 (-1 != eval('this.' + layerStyleRef + 'visibility').indexOf("hid")) )
		return true;
	
	return false;
	}

function put_innerHTML(txt)
	{
	if ( this.layer )
		{
		if ( g_theApp.isIE )
			this.layer.innerHTML = txt;
		else
			{
			this.layer.document.writeln(txt);
			this.layer.document.close();
			}
		}
	}

function SetZoomControl(f)
	{
	if ( !parent.g_NavBarLoaded )
		return;

	var formZoom = FindForm("zoomForm", parent.frmZoomBox.document);
	if (formZoom != null)
		{
		s = formZoom.zoomFactor;

		if ( -1 != f )
			f *= 100;

		for ( i = 0 ; i < s.options.length ; i++ )
			{
			if ( s.options[i].value == f )
				{
				s.selectedIndex = i;
				break;
				}
			}
		}
	}

function zoom_onchange(val)
{
	if ( g_theApp.ActiveViewMgr )
	{
		if ( g_theApp.ActiveViewMgr.ZoomIsPresent == true)
		{
			g_theApp.ActiveViewMgr.put_Zoom(parseInt(val));
		}
	}
}

function CViewMgr()
	{
//Set all zoom functions to null assuming the addons related
//to the data will provide their own functions.
	this.onResize = null;
	this.put_Zoom = null;
	this.get_Zoom = null;
	this.ApplyZoom = null;
	
	//General functions.
	this.onLoad = ViewMgrOnLoad;
	this.put_Location = ViewMgrDefaultFind; //MUST BE SET FOR FIND FEATURE 
	this.ZoomIsPresent = false;
	}

function ViewMgrOnLoad()
	{
	this.id = "ConvertedImage";
	this.zoomFactor = -1;
	this.zoomLast = -1;
	this.origWH = 1;
	this.origWidth = 100;

	if ( g_theApp.isIE )
	{
		p = document.all;
		this.s = document.all(this.id).style;
		

		if ( this.s )
		{
			this.s.position = "absolute";
			this.origWidth = this.s.pixelWidth;
			this.origWH = this.s.pixelWidth / this.s.pixelHeight;
		}
	}
	else
	{
		this.s = null;
	}
	SetZoomControl(this.zoomFactor);
}

function ViewMgrDefaultFind()
{
	return;
}

function handleResize()
	{
	location.reload();
	return false;
	}

function IsFrame(frameName)
	{
	return window.name == frameName;
	}

function UpdNavBar()
	{
	if (g_theApp.PageUpdateFunc != null)
		g_theApp.PageUpdateFunc ();

	if ( parent.g_NavBarLoaded )
		//parent.frmZoomBox.UpdateNavBar();
		parent.frmNavBar.UpdateNavBar();
	}

function UpdZoom()
	{
	if (g_theApp.ZoomResetFunc != null)
		g_theApp.ZoomResetFunc ();
	}

function UpdCPViewer()
	{
	if (g_theApp.CPResetFunc != null)
		g_theApp.CPResetFunc ();
	}

function UpdTitleBar()
	{
	if ( parent.g_TitleBarLoaded )
		parent.frmTitleBar.UpdateTitleBar();
	}

function GetCurPageNum()	{ return g_theApp.CurrentPageIX; }
function GetNumPages()		{ return g_theApp.FileList.length; }

function GoToNextPage()		{ GoToPage(g_theApp.CurrentPageIX + 1); }
function GoToPrevPage()		{ GoToPage(g_theApp.CurrentPageIX - 1); }
function GoToFirstPage()	{ GoToPage(0); }
function GoToLastPage()		{ GoToPage(gDocTable.length - 1) };

function GoToPage(ix)
	{
	var entry;

	if ( (g_theApp != null) &&
		 (ix != g_theApp.CurrentPageIX) && 
		 (null != (entry = g_theApp.FileList[ix])) )
		{
		var newPage;

		if (SupportsPriOutputType ())
			{
			newPage = entry.PriImage;

			if ( "" == newPage )
				newPage = newPage = entry.SecImage;
			}
		else
			newPage = entry.SecImage;

		if (frames["frmPageView"] != null)
			{
			frames["frmPageView"].window.location = newPage;
			}
		else
			{
			parent.frmPageView.location = newPage;
			}

		PageUpdated (ix);
		}
	}

function PageUpdated (ix)
{
	g_theApp.CurrentPageIX = ix;
	NotifyPageSyncs(ix);
}

function GoToPageByName(pageName)
{
	var pageIndex = PageIndexFromName (pageName);
	if (pageIndex >= 0)
	{
		GoToPage (pageIndex);
	}
}

function GoToPageByID(pageID)
{
	var pageIndex = PageIndexFromID (pageID);
	if (pageIndex >= 0)
	{
		GoToPage (pageIndex);
	}
}

function PageIndexFromName (pageName)
{
	if (g_theApp != null)
	{
		var entry;

		var count;
		var fileEntry;
		var bFoundEntry = false;
		for (count = 0; 
			 count < g_theApp.FileList.length && !bFoundEntry; 
			 count++)
		{
			fileEntry = g_theApp.FileList[count];
			if (pageName == fileEntry.PageName)
			{
				return count;
			}
		}
	}

	return -1;
}

function PageIndexFromID (pageID)
{
	if (g_theApp != null)
	{
		var entry;

		var count;
		var fileEntry;
		var bFoundEntry = false;
		for (count = 0; 
			 count < g_theApp.FileList.length && !bFoundEntry; 
			 count++)
		{
			fileEntry = g_theApp.FileList[count];
			if (pageID == fileEntry.PageID)
			{
				return count;
			}
		}
	}
	return -1;
}

function ZoomAvailable()
{
	if (SupportsPriOutputType () && g_theApp.FileList[0].PriImage != "")
	{
		return g_theApp.PriFormatSupportsZoom;
	}
	else
	{
		return (g_theApp.SecFormatSupportsZoom && g_theApp.FileList[0].SecImage != "");
	}
}

function NotifyPageSyncs(ix)
	{
	UpdNavBar();
	UpdTitleBar();
	UpdZoom();
	UpdCPViewer();
	}

function HasPrevSld()	{ return (GetCurPageNum() > 0); }
function HasNextSld()	{ return ((GetCurPageNum() + 1) < GetNumPages()); }

function CancelDrag()
	{
	window.event.cancelBubble=true;
	window.event.returnValue=false
	}

function html_escape(txt)
	{
	var result = "";

	for ( var i = 0 ; i < txt.length ; i++ )
		{
		if ( txt.charAt(i) == '&' )
			result += "&amp;";
		else if ( txt.charAt(i) == '<' )
			result += "&lt;";
		else if ( txt.charAt(i) == '>' )
			result += "&gt;";
		else
			result += txt.charAt(i);
		}

	return result;
	}

function FindForm(form, doc)
	{
	if ( g_theApp.isIE )
		return doc.forms[form];
	else if ( null != doc )
		{
		if ( null != doc.forms )
			{
			for ( i = 0 ; i < doc.forms.length ; i++ )
				{
				if ( form == doc.forms[i].name )
					return doc.forms[i];
				}
			}

		if ( null != doc.layers )
			{
			for ( i = 0 ; i < doc.layers.length ; i++ )
				{
				result = FindForm(form, doc.layers[i].document);

				if ( null != result )
					return result;
				}
			}
		}

	return null;
	}

function FindLayer(layer, doc)
	{
	var result = null;

	if ( g_theApp.isIE )
		return doc.all(layer);
	else if ( (null != doc) && (null != doc.layers) )
		{
		for ( i = 0 ; i < doc.layers.length ; i++ )
			{
			result = doc.layers[i];

			if ( layer == result.name )
				return result;

			result = FindLayer(layer, result.document);

			if ( null != result )
				return result;
			}
		}

	return null;
	}

function Unquote (str)
{
	var nStartIndex = 0;
	var nEndIndex = str.length;

	if (str.charAt (0) == '"')
	{
		nStartIndex = 1;
	}

	if (str.charAt (nEndIndex - 1) == '"')
	{
		nEndIndex -= 1;
	}

	return str.substring (nStartIndex, nEndIndex);
}

function ConvertXorYCoordinate(PosValue, OldMin, OldMax, NewMin, NewMax, MapBackwards)
{
//This is a simple conversion routine that changes from one system to another.
	var OldMid = (OldMax - OldMin) / 2;
	var NewMid = (NewMax - NewMin) / 2;
	var ConvertResult = 1 * PosValue;
	ConvertResult = ConvertResult - (OldMin + OldMid);
	ConvertResult = ConvertResult / OldMid;
	if(MapBackwards != 0)
	{
		ConvertResult = 0 - ConvertResult;
	}
	ConvertResult = ConvertResult * NewMid;
	ConvertResult = ConvertResult + (NewMin + NewMid);
	return ConvertResult;
}

function GoToURL (defURL)
{
	if ((g_theApp == null) || !SupportsXML () || (g_theApp.objParser == null))
	{
		if (defURL.indexOf ("javascript:") == 0)
		{
			// This is actually a function call, not a URL. 
			eval (defURL);
			return;
		}

		parent.location = defURL;
	}
}

var el;
function showMenu(pageID, shapeID) {

	if (SupportsXML ())
	{
		var shapeXML = FindShapeXML (pageID, shapeID);
		if (shapeXML != null)
		{
			CreateHLMenu (shapeXML);

			//ContextElement=window.event.srcElement;
			parent.frmPageView.menu1.style.leftPos += 10;
			parent.frmPageView.menu1.style.posLeft = event.clientX;
			parent.frmPageView.menu1.style.posTop = event.clientY;
			parent.frmPageView.menu1.style.display = "";

			var clientWidth = event.srcElement.document.body.clientWidth;
			var clientHeight = event.srcElement.document.body.clientHeight;

			var menuWidth = parseInt (parent.frmPageView.menu1.style.width);
			var margin = 10;

			// Figure out where to place the menu (X). 
			var menuX = event.clientX;
			if (event.clientX + parent.frmPageView.menu1.clientWidth > clientWidth)
			{
				menuX = clientWidth - parent.frmPageView.menu1.clientWidth - margin;
				if (menuX < margin)
				{
					menuX = margin;
				}
			}

			// Figure out where to place the menu (Y). 
			var menuY = event.clientY;
			if (event.clientY + parent.frmPageView.menu1.clientHeight > clientHeight)
			{
				menuY = clientHeight - parent.frmPageView.menu1.clientHeight - margin;
				if (menuY < margin)
				{
					menuY = margin;
				}
			}

			parent.frmPageView.menu1.style.posLeft = menuX;
			parent.frmPageView.menu1.style.posTop = menuY;

			parent.frmPageView.menu1.setCapture();

			event.cancelBubble = true;
		}
	}
}

function toggleMenu() {
	el=event.srcElement;
	if (el.className=="menuItem") {
	  el.className="highlightItem";
	} else if (el.className=="highlightItem") {
	  el.className="menuItem";
	}
}

function clickMenu()
{
	if (parent.frmPageView.menu1.style.display != "none")
	{
		parent.frmPageView.menu1.releaseCapture();
		parent.frmPageView.menu1.style.display="none";
		el=event.srcElement;
		if (el.doFunction != null) {
		 eval(el.doFunction);
		}
	}
}

function CreateHLMenu (shapeNode)
{
	// Create the HTML string. 
	var strHLMenuHTML = "";

	if (shapeNode != null)
	{
		// Look up all the Hyperlink nodes. 
		var hlColl = shapeNode.selectNodes ("Scratch/B/SolutionXML/HLURL:Hyperlinks/HLURL:Hyperlink");

		// Walk the list of Hyperlink nodes to generate the menu.
		var hlCount = hlColl.length;
		for (var count = 0; count < hlCount; count++)
		{
			var strDoFunction = "";
			var strDesc = "";
			var strAddress = "";

			var hlAddress = hlColl.item(count).selectSingleNode("HLURL:Address/textnode()");
			if (hlAddress != null && hlAddress.text.length > 0 && IsValidAddress (hlAddress.text))
			{
				strDoFunction = "'parent.location.href=";

				// Get the absolute URL. 
				var absoluteURL = hlColl.item(count).selectSingleNode("HLURL:AbsoluteURL/textnode()");
				if (g_theApp.DocHasBaseHL && absoluteURL != null && absoluteURL.text.length > 0)
				{
					// Use the absolute URL for our hyperlink. 
					strAddress = absoluteURL.text;
				}
				else
				{
					// Just use the address field. 
					strAddress = hlAddress.text;
				}

				strDoFunction += '"' + EscapePath (strAddress) + '"' + ";'";

				// Now try to get the description field. If empty, use the address as the description. 
				hlDesc = hlColl.item(count).selectSingleNode("HLURL:Description/textnode()");
				if (hlDesc != null && hlDesc.text.length > 0)
				{
					strDesc = hlDesc.text;
				}
				else
				{
					strDesc = strAddress;
				}
			}
			else // Address is not present, assume it's a link into a different page in this document. 
			{
				hlAddress = hlColl.item(count).selectSingleNode("HLURL:SubAddress/textnode()");
				if (hlAddress != null && hlAddress.text.length > 0)
				{
					strAddress = hlAddress.text;

					// Strip off the shape id (if present). 
					var pageShapeSep = strAddress.lastIndexOf ('/');
					if (pageShapeSep > 0)
					{
						strAddress = strAddress.substring (0, pageShapeSep);
					}

					strAddress = unescape(strAddress);

					var pageIndex = PageIndexFromName (strAddress);

					strDoFunction = "'GoToPage (" + pageIndex + ");'";

					// Just set the description to the page name as well. 
					strDesc = strAddress;
				}
			}

			if (strDoFunction.length > 0 && strDesc.length > 0)
			{
				strHLMenuHTML += "<div class='menuItem' doFunction=" + strDoFunction + ">";
				strHLMenuHTML += strDesc + "</div>";
			}
		}
	}

	parent.frmPageView.menu1.innerHTML = strHLMenuHTML;
}

function IsValidAddress (strAddress)
{
	var ret = false;

	if (strAddress != null && strAddress.length > 0)
	{
		var strFullPath = g_theApp.VisDocPath + g_theApp.VisDocName;

		if (strAddress != strFullPath &&
			strAddress != g_theApp.VisDocName)
		{
			// Points to something other than this file, go ahead 
			// and consider it valid. 
			ret = true;
		}
	}

	return ret;
}

function FindShapeXML (pageID, shapeID)
{
	var shapeObj = null;

	if (g_theApp != null && g_theApp.objParser != null)
	{
		// Get the Pages collection. 
		var pagesObj = g_theApp.objParser.selectSingleNode("VisioDocument/Pages");
		if(!pagesObj)
		{
			return null;
		}
		
		// Get the correct page. 
		var pageQuerryString = './/Page[@ID = "' + pageID + '"]';
		var pageObj = pagesObj.selectSingleNode(pageQuerryString);
		if(!pageObj)
		{
			return null;
		}

		// Get the correct shape.
		var shapeQuerryString = './/Shape[@ID = "' + shapeID + '"]';
		shapeObj = pageObj.selectSingleNode(shapeQuerryString);
	}

	return shapeObj;
}

function UpdateProps(pageID, shapeID)
{
	// Check to see if we should ignore this event. 
	if (window.event != null &&
		window.event.ctrlKey)
	{
		// If the control key is down, do nothing!
		return;
	}

	if (SupportsXML ())
	{
		var shape = FindShapeXML (pageID, shapeID);

		if (g_theApp.custPropEntryPoint != null)
			g_theApp.custPropEntryPoint (shape);
	}
}

function SupportsXML ()
{
	return (g_theApp != null && g_theApp.isIE && g_theApp.verIE >= 5.0);
}

function SupportsPriOutputType ()
{
	if (g_theApp.isIE)	// IE
	{
		return ((g_theApp.verIE >= g_theApp.PriFormatMinIE) && (g_theApp.PriFormatMinIE > 0.0));
	}
	else if (g_theApp.isNav)	// Nav
	{
		return ((g_theApp.verNav >= g_theApp.PriFormatMinNav) && (g_theApp.PriFormatMinNav > 0.0));
	}

	// Unsupported browser. 
	return false;
}

function EscapePath (strPath)
{
	var strResult = "";

	for ( var i = 0 ; i < strPath.length ; i++ )
	{
		if ( strPath.charAt(i) == '\\' )
		{
			strResult += "\\\\";
		}
		else
		{
			strResult += strPath.charAt(i);
		}
	}

	return strResult;
}
