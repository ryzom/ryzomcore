/* Simple.c
 
This is a Win32 C application (ie, no MFC, WTL, nor even any C++ -- just plain C) that demonstrates
how to embed a browser "control" (actually, an OLE object) in your own window (in order to display a
web page, or an HTML file on disk).

This example opens two windows, each containing a browser object. The first window displays an HTML
string in memory. The second window displays Microsoft's web site. We also disable the pop-up context
menu, and resize the browser to fill our window.

This is very loosely based upon a C++ example written by Chris Becke. I used that to learn the minimum
of what I needed to know about hosting the browser object. Then I wrote this example from the ground up
in C.

This requires IE 5.0 (or better) -- due to the IDocHostUIHandler interface, or a browser that supports
the same level of OLE in-place activation.
*/

// Adapted to C++ by Nicolas Vizerie for GameForge's Ryzom installer




#include "web_page_embedder.h"

#include <windows.h>
#include <exdisp.h>		// Defines of stuff like IWebBrowser2. This is an include file with Visual C 6 and above
#include <mshtml.h>		// Defines of stuff like IHTMLDocument2. This is an include file with Visual C 6 and above
#include <mshtmhst.h>	// Defines of stuff like IDocHostUIHandler. This is an include file with Visual C 6 and above
#include <crtdbg.h>		// for _ASSERT()


#include "nel/misc/debug.h"

#define NOTIMPLEMENTED return E_NOTIMPL;

struct CMyOleClientSite;

////////////////////////////
// WEB BROWSER EVENT SINK //
////////////////////////////

struct CWebBrowserEventSink : public IDispatch
{
	CWebPageEmbedder::CBrowserEventSink *Callback;
	DWORD							   SinkCookie;

	CWebBrowserEventSink() : Callback(NULL), SinkCookie(0)
	{}
	~CWebBrowserEventSink()
	{
		if (Callback)
		{
			Callback->onDestroy();
		}
	}

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void FAR *FAR *ppvObject)
	{
		if (memcmp(&riid, &IID_IUnknown, sizeof(GUID)) == 0)
		{
			*ppvObject = this;
			return S_OK;
		}
		
		else if (memcmp(&riid, &IID_IDispatch, sizeof(GUID)))
		{
			*ppvObject = this;
			return S_OK;
		}

		*ppvObject = NULL;
		return E_NOINTERFACE;
	}

	ULONG STDMETHODCALLTYPE AddRef()
	{
		return 1;
	}

	ULONG STDMETHODCALLTYPE Release()
	{
		return 1;
	}
	
	HRESULT STDMETHODCALLTYPE Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pDispParams, VARIANT FAR *pVarResult, EXCEPINFO FAR *pExcepInfo, UINT FAR *puArgErr)
	{
		if (!pDispParams)
			return E_INVALIDARG;
		if (Callback)
		{
			Callback->onInvoke(dispIdMember, riid, lcid, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr);
		}		
		return S_OK;
	}

	// NOT IMPLEMENTED
	HRESULT STDMETHODCALLTYPE GetTypeInfoCount(UINT FAR *pctinfo) {	NOTIMPLEMENTED }	
	HRESULT STDMETHODCALLTYPE GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo FAR *FAR *ppTInfo) { NOTIMPLEMENTED }	
	HRESULT STDMETHODCALLTYPE GetIDsOfNames(REFIID riid, LPOLESTR FAR *rgszNames, UINT cNames, LCID lcid, DISPID FAR *rgDispId)	{ NOTIMPLEMENTED }	
};

//////////////////////
// IOleInPlaceFrame //
//////////////////////

struct CMyOleInPlaceFrame : public IOleInPlaceFrame
{
	HWND				Wnd;
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID FAR* ppvObj)
	{
		NOTIMPLEMENTED;
	}

	ULONG STDMETHODCALLTYPE AddRef()
	{
		return 1;
	}

	ULONG STDMETHODCALLTYPE Release()
	{
		return 1;
	}

	HRESULT STDMETHODCALLTYPE GetWindow(HWND FAR* lphwnd)
	{		
		*lphwnd = Wnd;
		return(S_OK);
	}

	HRESULT STDMETHODCALLTYPE SetActiveObject(IOleInPlaceActiveObject *pActiveObject, LPCOLESTR pszObjName)
	{
		return(S_OK);
	}

	HRESULT STDMETHODCALLTYPE SetMenu(HMENU hmenuShared, HOLEMENU holemenu, HWND hwndActiveObject)
	{
		return(S_OK);
	}

	HRESULT STDMETHODCALLTYPE SetStatusText(LPCOLESTR pszStatusText)
	{
		return(S_OK);
	}

	HRESULT STDMETHODCALLTYPE EnableModeless(BOOL fEnable)
	{
		return(S_OK);
	}


	HRESULT STDMETHODCALLTYPE ContextSensitiveHelp(BOOL fEnterMode) { NOTIMPLEMENTED; }	
	HRESULT STDMETHODCALLTYPE GetBorder(LPRECT lprectBorder) { NOTIMPLEMENTED; }	
	HRESULT STDMETHODCALLTYPE RequestBorderSpace(LPCBORDERWIDTHS pborderwidths)	{ NOTIMPLEMENTED; }	
	HRESULT STDMETHODCALLTYPE SetBorderSpace(LPCBORDERWIDTHS pborderwidths) { NOTIMPLEMENTED; }		
	HRESULT STDMETHODCALLTYPE InsertMenus(HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths)	{ NOTIMPLEMENTED; }		
	HRESULT STDMETHODCALLTYPE RemoveMenus(HMENU hmenuShared) { NOTIMPLEMENTED; }			
	HRESULT STDMETHODCALLTYPE TranslateAccelerator(LPMSG lpmsg, WORD wID) {	NOTIMPLEMENTED; }	
};


/////////////////////
// IOleInPlaceSite //
/////////////////////

struct CMyOleInPlaceSite : public IOleInPlaceSite
{
	CMyOleInPlaceFrame InPlaceFrame;
	CMyOleClientSite   *Parent;


	CMyOleInPlaceSite() : Parent(NULL)
	{		
	}

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID FAR* ppvObj);
	

	ULONG STDMETHODCALLTYPE AddRef()
	{
		return 1;
	}

	ULONG STDMETHODCALLTYPE Release()
	{
		return 1;
	}

	HRESULT STDMETHODCALLTYPE GetWindow(HWND FAR* lphwnd)
	{		
		*lphwnd = InPlaceFrame.Wnd;
		return(S_OK);
	}

	HRESULT STDMETHODCALLTYPE ContextSensitiveHelp(BOOL fEnterMode)
	{
		NOTIMPLEMENTED;
	}

	HRESULT STDMETHODCALLTYPE CanInPlaceActivate()
	{
		// Tell the browser we can in place activate
		return(S_OK);
	}

	HRESULT STDMETHODCALLTYPE OnInPlaceActivate()
	{
		// Tell the browser we did it ok
		return(S_OK);
	}

	HRESULT STDMETHODCALLTYPE OnUIActivate()
	{
		return(S_OK);
	}

	HRESULT STDMETHODCALLTYPE GetWindowContext(LPOLEINPLACEFRAME FAR* lplpFrame, LPOLEINPLACEUIWINDOW FAR* lplpDoc, LPRECT lprcPosRect, LPRECT lprcClipRect, LPOLEINPLACEFRAMEINFO lpFrameInfo)
	{	
		*lplpFrame = &InPlaceFrame;

		// We have no OLEINPLACEUIWINDOW
		*lplpDoc = 0;

		// Fill in some other info for the browser
		lpFrameInfo->fMDIApp = FALSE;
		lpFrameInfo->hwndFrame = InPlaceFrame.Wnd;
		lpFrameInfo->haccel = 0;
		lpFrameInfo->cAccelEntries = 0;

		// Give the browser the dimensions of where it can draw. We give it our entire window to fill.
		// We do this in OnPosRectChange() which is called right when a window is first
		// created anyway, so no need to duplicate it here.
		//	GetClientRect(lpFrameInfo->hwndFrame, lprcPosRect);
		//	GetClientRect(lpFrameInfo->hwndFrame, lprcClipRect);

		return(S_OK);
	}

	HRESULT STDMETHODCALLTYPE Scroll(SIZE scrollExtent)
	{
		NOTIMPLEMENTED;
	}

	HRESULT STDMETHODCALLTYPE OnUIDeactivate(BOOL fUndoable)
	{
		return(S_OK);
	}

	HRESULT STDMETHODCALLTYPE OnInPlaceDeactivate()
	{
		return(S_OK);
	}

	HRESULT STDMETHODCALLTYPE DiscardUndoState()
	{
		NOTIMPLEMENTED;
	}

	HRESULT STDMETHODCALLTYPE DeactivateAndUndo()
	{
		NOTIMPLEMENTED;
	}

	// Called when the position of the browser object is changed, such as when we call the IWebBrowser2's put_Width(),
	// put_Height(), put_Left(), or put_Right().
	HRESULT STDMETHODCALLTYPE OnPosRectChange(LPCRECT lprcPosRect);	
};


///////////////////////
// IDocHostUIHandler //
///////////////////////

struct CMyDocHostUIHandler : public IDocHostUIHandler
{
	IUnknown *Parent;
	CMyDocHostUIHandler() : Parent(NULL)
	{		
	}

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID FAR* ppvObj)
	{
		// delegate to my parent
		nlassert(Parent);
		return Parent->QueryInterface(riid, ppvObj);
	}	

	ULONG STDMETHODCALLTYPE AddRef()
	{
		return 1;
	}

	ULONG STDMETHODCALLTYPE Release()
	{
		return 1;
	}

	// Called when the browser object is about to display its context menu.
	HRESULT STDMETHODCALLTYPE ShowContextMenu(DWORD dwID, POINT __RPC_FAR *ppt, IUnknown __RPC_FAR *pcmdtReserved, IDispatch __RPC_FAR *pdispReserved)
	{
		// If desired, we can pop up your own custom context menu here. Of course,
		// we would need some way to get our window handle, so what we'd probably
		// do is like what we did with our IOleInPlaceFrame object. We'd define and create
		// our own IDocHostUIHandlerEx object with an embedded IDocHostUIHandler at the
		// start of it. Then we'd add an extra HWND field to store our window handle.
		// It could look like this:
		//
		// typedef struct _IDocHostUIHandlerEx {
		//		IDocHostUIHandler	ui;		// The IDocHostUIHandler must be first!
		//		HWND				window;
		// } IDocHostUIHandlerEx;
		//
		// Of course, we'd need a unique IDocHostUIHandlerEx object for each window, so
		// EmbedBrowserObject() would have to allocate one of those too. And that's
		// what we'd pass to our browser object (and it would in turn pass it to us
		// here, instead of 'This' being a IDocHostUIHandler *).

		// We will return S_OK to tell the browser not to display its default context menu,
		// or return S_FALSE to let the browser show its default context menu. For this
		// example, we wish to disable the browser's default context menu.
		return(S_OK);
	}

	// Called at initialization of the browser object UI. We can set various features of the browser object here.
	HRESULT STDMETHODCALLTYPE GetHostInfo(DOCHOSTUIINFO __RPC_FAR *pInfo)
	{
		pInfo->cbSize = sizeof(DOCHOSTUIINFO);

		// Set some flags. We don't want any 3D border. You can do other things like hide
		// the scroll bar (DOCHOSTUIFLAG_SCROLL_NO), display picture display (DOCHOSTUIFLAG_NOPICS),
		// disable any script running when the page is loaded (DOCHOSTUIFLAG_DISABLE_SCRIPT_INACTIVE),
		// open a site in a new browser window when the user clicks on some link (DOCHOSTUIFLAG_OPENNEWWIN),
		// and lots of other things. See the MSDN docs on the DOCHOSTUIINFO struct passed to us.
		pInfo->dwFlags = DOCHOSTUIFLAG_NO3DBORDER|DOCHOSTUIFLAG_SCROLL_NO|DOCHOSTUIFLAG_DIALOG|DOCHOSTUIFLAG_DIALOG;

		// Set what happens when the user double-clicks on the object. Here we use the default.
		pInfo->dwDoubleClick = DOCHOSTUIDBLCLK_DEFAULT;

		return(S_OK);
	}

	// Called when the browser object shows its UI. This allows us to replace its menus and toolbars by creating our
	// own and displaying them here.
	HRESULT STDMETHODCALLTYPE ShowUI(DWORD dwID, IOleInPlaceActiveObject __RPC_FAR *pActiveObject, IOleCommandTarget __RPC_FAR *pCommandTarget, IOleInPlaceFrame __RPC_FAR *pFrame, IOleInPlaceUIWindow __RPC_FAR *pDoc)
	{
		// We've already got our own UI in place so just return S_OK to tell the browser
		// not to display its menus/toolbars. Otherwise we'd return S_FALSE to let it do
		// that.
		return(S_OK);
	}

	// Called when browser object hides its UI. This allows us to hide any menus/toolbars we created in ShowUI.
	HRESULT STDMETHODCALLTYPE HideUI()
	{
		return(S_OK);
	}

	// Called when the browser object wants to notify us that the command state has changed. We should update any
	// controls we have that are dependent upon our embedded object, such as "Back", "Forward", "Stop", or "Home"
	// buttons.
	HRESULT STDMETHODCALLTYPE UpdateUI()
	{
		// We update our UI in our window message loop so we don't do anything here.
		return(S_OK);
	}

	// Called from the browser object's IOleInPlaceActiveObject object's EnableModeless() function. Also
	// called when the browser displays a modal dialog box.
	HRESULT STDMETHODCALLTYPE EnableModeless(BOOL fEnable)
	{
		return(S_OK);
	}

	// Called from the browser object's IOleInPlaceActiveObject object's OnDocWindowActivate() function.
	// This informs off of when the object is getting/losing the focus.
	HRESULT STDMETHODCALLTYPE OnDocWindowActivate(BOOL fActivate)
	{
		return(S_OK);
	}

	// Called from the browser object's IOleInPlaceActiveObject object's OnFrameWindowActivate() function.
	HRESULT STDMETHODCALLTYPE OnFrameWindowActivate(BOOL fActivate)
	{
		return(S_OK);
	}

	// Called from the browser object's IOleInPlaceActiveObject object's ResizeBorder() function.
	HRESULT STDMETHODCALLTYPE ResizeBorder(LPCRECT prcBorder, IOleInPlaceUIWindow __RPC_FAR *pUIWindow, BOOL fRameWindow)
	{
		return(S_OK);
	}

	// Called from the browser object's TranslateAccelerator routines to translate key strokes to commands.
	HRESULT STDMETHODCALLTYPE TranslateAccelerator(LPMSG lpMsg, const GUID __RPC_FAR *pguidCmdGroup, DWORD nCmdID)
	{
		// We don't intercept any keystrokes, so we do nothing here. But for example, if we wanted to
		// override the TAB key, perhaps do something with it ourselves, and then tell the browser
		// not to do anything with this keystroke, we'd do:
		//
		//	if (pMsg && pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_TAB)
		//	{
		//		// Here we do something as a result of a TAB key press.
		//
		//		// Tell the browser not to do anything with it.
		//		return(S_FALSE);
		//	}
		//
		//	// Otherwise, let the browser do something with this message.
		//	return(S_OK);

		// For our example, we want to make sure that the user can invoke some key to popup the context
		// menu, so we'll tell it to ignore all messages.
		return(S_FALSE);
	}

	// Called by the browser object to find where the host wishes the browser to get its options in the registry.
	// We can use this to prevent the browser from using its default settings in the registry, by telling it to use
	// some other registry key we've setup with the options we want.
	HRESULT STDMETHODCALLTYPE GetOptionKeyPath(LPOLESTR __RPC_FAR *pchKey, DWORD dw)
	{
		// Let the browser use its default registry settings.
		return(S_FALSE);
	}

	// Called by the browser object when it is used as a drop target. We can supply our own IDropTarget object,
	// IDropTarget functions, and IDropTarget VTable if we want to determine what happens when someone drags and
	// drops some object on our embedded browser object.
	HRESULT STDMETHODCALLTYPE GetDropTarget(IDropTarget __RPC_FAR *pDropTarget, IDropTarget __RPC_FAR *__RPC_FAR *ppDropTarget)
	{
		// Return our IDropTarget object associated with this IDocHostUIHandler object. I don't
		// know why we don't do this via UI_QueryInterface(), but we don't.

		// NOTE: If we want/need an IDropTarget interface, then we would have had to setup our own
		// IDropTarget functions, IDropTarget VTable, and create an IDropTarget object. We'd want to put
		// a pointer to the IDropTarget object in our own custom IDocHostUIHandlerEx object (like how
		// we may add an HWND field for the use of UI_ShowContextMenu). So when we created our
		// IDocHostUIHandlerEx object, maybe we'd add a 'idrop' field to the end of it, and
		// store a pointer to our IDropTarget object there. Then we could return this pointer as so:
		//
		// *pDropTarget = ((IDocHostUIHandlerEx FAR *)This)->idrop;
		// return(S_OK);

		// But for our purposes, we don't need an IDropTarget object, so we'll tell whomever is calling
		// us that we don't have one.
		return(S_FALSE);
	}

	// Called by the browser when it wants a pointer to our IDispatch object. This object allows us to expose
	// our own automation interface (ie, our own COM objects) to other entities that are running within the
	// context of the browser so they can call our functions if they want. An example could be a javascript
	// running in the URL we display could call our IDispatch functions. We'd write them so that any args passed
	// to them would use the generic datatypes like a BSTR for utmost flexibility.
	HRESULT STDMETHODCALLTYPE GetExternal(IDispatch __RPC_FAR *__RPC_FAR *ppDispatch)
	{
		// Return our IDispatch object associated with this IDocHostUIHandler object. I don't
		// know why we don't do this via UI_QueryInterface(), but we don't.

		// NOTE: If we want/need an IDispatch interface, then we would have had to setup our own
		// IDispatch functions, IDispatch VTable, and create an IDispatch object. We'd want to put
		// a pointer to the IDispatch object in our custom _IDocHostUIHandlerEx object (like how
		// we may add an HWND field for the use of UI_ShowContextMenu). So when we defined our
		// _IDocHostUIHandlerEx object, maybe we'd add a 'idispatch' field to the end of it, and
		// store a pointer to our IDispatch object there. Then we could return this pointer as so:
		//
		// *ppDispatch = ((_IDocHostUIHandlerEx FAR *)This)->idispatch;
		// return(S_OK);

		// But for our purposes, we don't need an IDispatch object, so we'll tell whomever is calling
		// us that we don't have one. Note: We must set ppDispatch to 0 if we don't return our own
		// IDispatch object.
		*ppDispatch = 0;
		return(S_FALSE);
	}

	// Called by the browser object to give us an opportunity to modify the URL to be loaded.
	HRESULT STDMETHODCALLTYPE TranslateUrl(DWORD dwTranslate, OLECHAR __RPC_FAR *pchURLIn, OLECHAR __RPC_FAR *__RPC_FAR *ppchURLOut)
	{
		// We don't need to modify the URL. Note: We need to set ppchURLOut to 0 if we don't
		// return an OLECHAR (buffer) containing a modified version of pchURLIn.
		*ppchURLOut = 0;
		return(S_FALSE);
	}

	// Called by the browser when it does cut/paste to the clipboard. This allows us to block certain clipboard
	// formats or support additional clipboard formats.
	HRESULT STDMETHODCALLTYPE FilterDataObject(IDataObject __RPC_FAR *pDO, IDataObject __RPC_FAR *__RPC_FAR *ppDORet)
	{
		// Return our IDataObject object associated with this IDocHostUIHandler object. I don't
		// know why we don't do this via UI_QueryInterface(), but we don't.

		// NOTE: If we want/need an IDataObject interface, then we would have had to setup our own
		// IDataObject functions, IDataObject VTable, and create an IDataObject object. We'd want to put
		// a pointer to the IDataObject object in our custom _IDocHostUIHandlerEx object (like how
		// we may add an HWND field for the use of UI_ShowContextMenu). So when we defined our
		// _IDocHostUIHandlerEx object, maybe we'd add a 'idata' field to the end of it, and
		// store a pointer to our IDataObject object there. Then we could return this pointer as so:
		//
		// *ppDORet = ((_IDocHostUIHandlerEx FAR *)This)->idata;
		// return(S_OK);

		// But for our purposes, we don't need an IDataObject object, so we'll tell whomever is calling
		// us that we don't have one. Note: We must set ppDORet to 0 if we don't return our own
		// IDataObject object.
		*ppDORet = 0;
		return(S_FALSE);
	}

};




////////////////////
// IOleClientSite //
////////////////////
struct CMyOleClientSite : public IOleClientSite
{
	// contained interfaces implementation
	CMyOleInPlaceSite			InPlaceSite;		// My IOleInPlaceSite object. A convenient place to put it.
	CMyDocHostUIHandler			DocHostUIHandler;	// My IDocHostUIHandler object. Must be first within my _IDocHostUIHandlerEx.
	IOleObject					*BrowserObject;	
	//
	IConnectionPointContainer	*ConnectionPointContainer;
	IConnectionPoint			*ConnectionPoint;
	CWebBrowserEventSink		EventSink;
	//
	

	CMyOleClientSite()
	{		
		InPlaceSite.Parent = this;
		DocHostUIHandler.Parent = this;
		BrowserObject = NULL;
	}

	~CMyOleClientSite()
	{
		if (ConnectionPoint)
		{
			ConnectionPoint->Unadvise(EventSink.SinkCookie);
			ConnectionPoint->Release();
		}
		if (ConnectionPointContainer)
		{
			ConnectionPointContainer->Release();
		}
	}

	/************************* Site_QueryInterface() *************************
	 * The browser object calls this when it wants a pointer to one of our
	 * IOleClientSite, IDocHostUIHandler, or IOleInPlaceSite structures. They
	 * are all accessible via the _IOleClientSiteEx struct we allocated in
	 * EmbedBrowserObject() and passed to DoVerb() and OleCreate().
	 *
	 * This =		A pointer to whatever _IOleClientSiteEx struct we passed to
	 *				OleCreate() or DoVerb().
	 * riid =		A GUID struct that the browser passes us to clue us as to
	 *				which type of struct (object) it would like a pointer
	 *				returned for.
	 * ppvObject =	Where the browser wants us to return a pointer to the
	 *				appropriate struct. (ie, It passes us a handle to fill in).
	 *
	 * RETURNS: S_OK if we return the struct, or E_NOINTERFACE if we don't have
	 * the requested struct.
	 */

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void ** ppvObject)
	{		
		if (!memcmp(&riid, &IID_IUnknown, sizeof(GUID)) || !memcmp(&riid, &IID_IOleClientSite, sizeof(GUID)))
			*ppvObject = this;
		else if (!memcmp(&riid, &IID_IOleInPlaceSite, sizeof(GUID)))
			*ppvObject = &InPlaceSite;
		else if (!memcmp(&riid, &IID_IDocHostUIHandler, sizeof(GUID)))
			*ppvObject = &DocHostUIHandler;		
		else
		{
			*ppvObject = 0;
			return(E_NOINTERFACE);
		}
		return(S_OK);
	}

	ULONG STDMETHODCALLTYPE AddRef()
	{		
		return 1;
	}

	ULONG STDMETHODCALLTYPE Release()
	{		
		return 1;
	}

	HRESULT STDMETHODCALLTYPE SaveObject()
	{
		NOTIMPLEMENTED;
	}

	HRESULT STDMETHODCALLTYPE GetMoniker(DWORD dwAssign, DWORD dwWhichMoniker, IMoniker ** ppmk)
	{
		NOTIMPLEMENTED;
	}

	HRESULT STDMETHODCALLTYPE GetContainer(LPOLECONTAINER FAR* ppContainer)
	{
		// Tell the browser that we are a simple object and don't support a container
		*ppContainer = 0;

		return(E_NOINTERFACE);
	}

	HRESULT STDMETHODCALLTYPE ShowObject()
	{
		return(NOERROR);
	}

	HRESULT STDMETHODCALLTYPE OnShowWindow(BOOL fShow)
	{
		NOTIMPLEMENTED;
	}

	HRESULT STDMETHODCALLTYPE RequestNewObjectLayout()
	{
		NOTIMPLEMENTED;
	}
};



HRESULT STDMETHODCALLTYPE CMyOleInPlaceSite::QueryInterface(REFIID riid, LPVOID FAR* ppvObj)
{
	// delegate to my parent
	nlassert(Parent);
	return Parent->QueryInterface(riid, ppvObj);
}


// Called when the position of the browser object is changed, such as when we call the IWebBrowser2's put_Width(),
	// put_Height(), put_Left(), or put_Right().
HRESULT STDMETHODCALLTYPE CMyOleInPlaceSite::OnPosRectChange(LPCRECT lprcPosRect)
{		
	IOleInPlaceObject	*inplace;
	// We need to get the browser's IOleInPlaceObject object so we can call its SetObjectRects
	// function.		
	if (!Parent->BrowserObject->QueryInterface(IID_IOleInPlaceObject, (void**)&inplace))
	{
		// Give the browser the dimensions of where it can draw.
		inplace->SetObjectRects(lprcPosRect, lprcPosRect);
		inplace->Release();
	}

	return(S_OK);
}












/*************************** UnEmbedBrowserObject() ************************
 * Called to detach the browser object from our host window, and free its
 * resources, right before we destroy our window.
 *
 * hwnd =		Handle to the window hosting the browser object.
 *
 * NOTE: The pointer to the browser object must have been stored in the
 * window's USERDATA field. In other words, don't call UnEmbedBrowserObject().
 * with a HWND that wasn't successfully passed to EmbedBrowserObject().
 */

static void UnEmbedBrowserObject(HWND hwnd)
{	
	CMyOleClientSite *clientSite = (CMyOleClientSite *) GetWindowLong(hwnd, GWL_USERDATA);
	
	if (clientSite)
	{
		clientSite->BrowserObject->Close(OLECLOSE_NOSAVE);				
		clientSite->BrowserObject->Release();		
		SetWindowLong(hwnd, GWL_USERDATA, 0);
		delete clientSite;
	}	
}



/******************************* DisplayHTMLPage() ****************************
 * Displays a URL, or HTML file on disk.
 *
 * hwnd =		Handle to the window hosting the browser object.
 * webPageName =	Pointer to nul-terminated name of the URL/file.
 *
 * RETURNS: 0 if success, or non-zero if an error.
 *
 * NOTE: EmbedBrowserObject() must have been successfully called once with the
 * specified window, prior to calling this function. You need call
 * EmbedBrowserObject() once only, and then you can make multiple calls to
 * this function to display numerous pages in the specified window.
 */

long CWebPageEmbedder::displayHTMLPage(HWND hwnd, const char *webPageName)
{
	CMyOleClientSite *clientSite = (CMyOleClientSite *) GetWindowLong(hwnd, GWL_USERDATA);
	if (!clientSite) return -1;


	IWebBrowser2	*webBrowser2;
	VARIANT			myURL;
	
	
	// We want to get the base address (ie, a pointer) to the IWebBrowser2 object embedded within the browser
	// object, so we can call some of the functions in the former's table.
	if (!clientSite->BrowserObject->QueryInterface(IID_IWebBrowser2, (void**)&webBrowser2))
	{
		// Ok, now the pointer to our IWebBrowser2 object is in 'webBrowser2', and so its VTable is
		// webBrowser2->lpVtbl.

		// Our URL (ie, web address, such as "http://www.microsoft.com" or an HTM filename on disk
		// such as "c:\myfile.htm") must be passed to the IWebBrowser2's Navigate2() function as a BSTR.
		// A BSTR is like a pascal version of a double-byte character string. In other words, the
		// first unsigned short is a count of how many characters are in the string, and then this
		// is followed by those characters, each expressed as an unsigned short (rather than a
		// char). The string is not nul-terminated. The OS function SysAllocString can allocate and
		// copy a UNICODE C string to a BSTR. Of course, we'll need to free that BSTR after we're done
		// with it. If we're not using UNICODE, we first have to convert to a UNICODE string.
		//
		// What's more, our BSTR needs to be stuffed into a VARIENT struct, and that VARIENT struct is
		// then passed to Navigate2(). Why? The VARIENT struct makes it possible to define generic
		// 'datatypes' that can be used with all languages. Not all languages support things like
		// nul-terminated C strings. So, by using a VARIENT, whose first field tells what sort of
		// data (ie, string, float, etc) is in the VARIENT, COM interfaces can be used by just about
		// any language.
		VariantInit(&myURL);
		myURL.vt = VT_BSTR;

#ifndef UNICODE
		{
		wchar_t		*buffer;
		DWORD		size;

		size = MultiByteToWideChar(CP_ACP, 0, webPageName, -1, 0, 0);
		if (!(buffer = (wchar_t *)GlobalAlloc(GMEM_FIXED, sizeof(wchar_t) * size))) goto badalloc;
		MultiByteToWideChar(CP_ACP, 0, webPageName, -1, buffer, size);
		myURL.bstrVal = SysAllocString(buffer);
		GlobalFree(buffer);
		}
#else
		myURL.bstrVal = SysAllocString(webPageName);
#endif
		if (!myURL.bstrVal)
		{
badalloc:	webBrowser2->Release();
			return(-6);
		}

		// Call the Navigate2() function to actually display the page.
		webBrowser2->Navigate2(&myURL, 0, 0, 0, 0);

		// Free any resources (including the BSTR we allocated above).
		VariantClear(&myURL);

		// We no longer need the IWebBrowser2 object (ie, we don't plan to call any more functions in it,
		// so we can release our hold on it). Note that we'll still maintain our hold on the browser
		// object.
		webBrowser2->Release();

		// Success
		return(0);
	}

	return(-5);
}





/******************************* ResizeBrowser() ****************************
 * Resizes the browser object for the specified window to the specified
 * width and height.
 *
 * hwnd =			Handle to the window hosting the browser object.
 * width =			Width.
 * height =			Height.
 *
 * NOTE: EmbedBrowserObject() must have been successfully called once with the
 * specified window, prior to calling this function. You need call
 * EmbedBrowserObject() once only, and then you can make multiple calls to
 * this function to resize the browser object.
 */

static void ResizeBrowser(HWND hwnd, DWORD width, DWORD height)
{	
	CMyOleClientSite *clientSite = (CMyOleClientSite *) GetWindowLong(hwnd, GWL_USERDATA);	
	if (!clientSite) return;
	
	// We want to get the base address (ie, a pointer) to the IWebBrowser2 object embedded within the browser
	// object, so we can call some of the functions in the former's table.
	IWebBrowser2	*webBrowser2;	
	if (!clientSite->BrowserObject->QueryInterface(IID_IWebBrowser2, (void**)&webBrowser2))
	{
		// Ok, now the pointer to our IWebBrowser2 object is in 'webBrowser2', and so its VTable is
		// webBrowser2->lpVtbl.

		// Call are put_Width() and put_Height() to set the new width/height.
		webBrowser2->put_Width(width);
		webBrowser2->put_Height(height);

		// We no longer need the IWebBrowser2 object (ie, we don't plan to call any more functions in it,
		// so we can release our hold on it). Note that we'll still maintain our hold on the browser
		// object.
		webBrowser2->Release();
	}
}


void CWebPageEmbedder::setBrowserEventSink(HWND hwnd,CBrowserEventSink *eventSink)
{
	CMyOleClientSite *clientSite = (CMyOleClientSite *) GetWindowLong(hwnd, GWL_USERDATA);	
	if (!clientSite) return;
	clientSite->EventSink.Callback = eventSink;
}


/***************************** EmbedBrowserObject() **************************
 * Puts the browser object inside our host window, and save a pointer to this
 * window's browser object in the window's GWL_USERDATA field.
 *
 * hwnd =		Handle of our window into which we embed the browser object.
 *
 * RETURNS: 0 if success, or non-zero if an error.
 *
 * NOTE: We tell the browser object to occupy the entire client area of the
 * window.
 *
 * NOTE: No HTML page will be displayed here. We can do that with a subsequent
 * call to either DisplayHTMLPage() or DisplayHTMLStr(). This is merely once-only
 * initialization for using the browser object. In a nutshell, what we do
 * here is get a pointer to the browser object in our window's GWL_USERDATA
 * so we can access that object's functions whenever we want, and we also pass
 * the browser a pointer to our IOleClientSite struct so that the browser can
 * call our functions in our struct's VTable.
 */

static long EmbedBrowserObject(HWND hwnd)
{		
	CMyOleClientSite *clientSite = NULL;
	
	LPCLASSFACTORY				classFactory = 0;
	// Get a pointer to the browser object's IClassFactory object via CoGetClassObject()	
	if (CoGetClassObject(CLSID_WebBrowser, CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER, NULL, IID_IClassFactory, (void **)&classFactory) == S_OK && classFactory)
	{		
		IOleObject					*browserObject = NULL;
		// Call the IClassFactory's CreateInstance() to create a browser object
		if (classFactory->CreateInstance(0, IID_IOleObject, (void **) &browserObject) == S_OK)
		{
			// Free the IClassFactory. We need it only to create a browser object instance
			classFactory->Release();

			CMyOleClientSite *clientSite = new CMyOleClientSite;
			clientSite->BrowserObject = browserObject;
			clientSite->InPlaceSite.InPlaceFrame.Wnd = hwnd;
			SetWindowLong(hwnd, GWL_USERDATA, (LONG) clientSite);

			// Give the browser a pointer to my IOleClientSite object
			if (browserObject->SetClientSite(clientSite) == S_OK)
			{
				// We can now call the browser object's SetHostNames function. SetHostNames lets the browser object know our
				// application's name and the name of the document in which we're embedding the browser. (Since we have no
				// document name, we'll pass a 0 for the latter). When the browser object is opened for editing, it displays
				// these names in its titlebar.
				//	
				// We are passing 3 args to SetHostNames. You'll note that the first arg to SetHostNames is the base
				// address of our browser control. This is something that you always have to remember when working in C
				// (as opposed to C++). When calling a VTable function, the first arg to that function must always be the
				// structure which contains the VTable. (In this case, that's the browser control itself). Why? That's
				// because that function is always assumed to be written in C++. And the first argument to any C++ function
				// must be its 'this' pointer (ie, the base address of its class, which in this case is our browser object
				// pointer). In C++, you don't have to pass this first arg, because the C++ compiler is smart enough to
				// produce an executable that always adds this first arg. In fact, the C++ compiler is smart enough to
				// know to fetch the function pointer from the VTable, so you don't even need to reference that. In other
				// words, the C++ equivalent code would be:
				//
				// browserObject->SetHostNames(L"My Host Name", 0);
				//
				// So, when you're trying to convert C++ code to C, always remember to add this first arg whenever you're
				// dealing with a VTable (ie, the field is usually named 'lpVtbl') in the standard objects, and also add
				// the reference to the VTable itself.
				//
				// Oh yeah, the L is because we need UNICODE strings. And BTW, the host and document names can be anything
				// you want.
				browserObject->SetHostNames(L"My Host Name", 0);

				RECT rect;		
				GetClientRect(hwnd, &rect);

				IWebBrowser2 *webBrowser2 = NULL;

				// Let browser object know that it is embedded in an OLE container.
				if (!OleSetContainedObject((struct IUnknown *)browserObject, TRUE) &&

					// Set the display area of our browser control the same as our window's size
					// and actually put the browser object into our window.
					!browserObject->DoVerb(OLEIVERB_SHOW, NULL, clientSite, -1, hwnd, &rect) &&

					// Ok, now things may seem to get even trickier, One of those function pointers in the browser's VTable is
					// to the QueryInterface() function. What does this function do? It lets us grab the base address of any
					// other object that may be embedded within the browser object. And this other object has its own VTable
					// containing pointers to more functions we can call for that object.
					//
					// We want to get the base address (ie, a pointer) to the IWebBrowser2 object embedded within the browser
					// object, so we can call some of the functions in the former's table. For example, one IWebBrowser2 function
					// we intend to call below will be Navigate2(). So we call the browser object's QueryInterface to get our
					// pointer to the IWebBrowser2 object.
				!browserObject->QueryInterface(IID_IWebBrowser2, (void**)&webBrowser2))
				{
					// Ok, now the pointer to our IWebBrowser2 object is in 'webBrowser2', and so its VTable is
					// webBrowser2->lpVtbl.

					// Let's call several functions in the IWebBrowser2 object to position the browser display area
					// in our window. The functions we call are put_Left(), put_Top(), put_Width(), and put_Height().
					// Note that we reference the IWebBrowser2 object's VTable to get pointers to those functions. And
					// also note that the first arg we pass to each is the pointer to the IWebBrowser2 object.
					webBrowser2->put_Left(0);
					webBrowser2->put_Top(0);
					webBrowser2->put_Width(rect.right);
					webBrowser2->put_Height(rect.bottom);

					
					/*
					HWND wnd;
					if (SUCCEEDED(webBrowser2->get_HWND((long *) &wnd)))
					{
						::SetWindowPos(wnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE);
					}
					*/

					// We no longer need the IWebBrowser2 object (ie, we don't plan to call any more functions in it
					// right now, so we can release our hold on it). Note that we'll still maintain our hold on the
					// browser object until we're done with that object.
					webBrowser2->Release();

					
					// Establish event sink on the browser to see when the frame has finished to load
					if (webBrowser2->QueryInterface(IID_IConnectionPointContainer, (void **) &clientSite->ConnectionPointContainer) == S_OK)
					{						
						if (clientSite->ConnectionPointContainer->FindConnectionPoint(DIID_DWebBrowserEvents2, &clientSite->ConnectionPoint) == S_OK)
						{						
							if (clientSite->ConnectionPoint->Advise(&clientSite->EventSink, &clientSite->EventSink.SinkCookie) == S_OK)
							{
								// Success
								return(0);								
							}							
							else
							{
								UnEmbedBrowserObject(hwnd);			
								SetWindowLong(hwnd, GWL_USERDATA, 0);
								return(-6);
							}							
						}
						else
						{							
							UnEmbedBrowserObject(hwnd);			
							SetWindowLong(hwnd, GWL_USERDATA, 0);
							return(-5);
						}
					}					
				}
			}

			// Something went wrong setting up the browser!
			UnEmbedBrowserObject(hwnd);			
			SetWindowLong(hwnd, GWL_USERDATA, 0);
			return(-4);
		}

		classFactory->Release();
		delete clientSite;
		SetWindowLong(hwnd, GWL_USERDATA, 0);		

		// Can't create an instance of the browser!
		return(-3);
	}

	delete clientSite;
	SetWindowLong(hwnd, GWL_USERDATA, 0);

	// Can't get the web browser's IClassFactory!
	return(-2);
}


#ifndef __IHTMLElementRender_INTERFACE_DEFINED__
#define __IHTMLElementRender_INTERFACE_DEFINED__

/* interface IHTMLElementRender */
/* [uuid][unique][object] */ 


GUID IID_IHTMLElementRender = {0x3050f669, 0x98b5, 0x11cf, 0xbb, 0x82, 0x00, 0xaa, 0x00, 0xbd, 0xce, 0x0b};


class IHTMLElementRender : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE DrawToDC( 
        /* [in] */ HDC hDC) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE SetDocumentPrinter( 
        /* [in] */ BSTR bstrPrinterName,
        /* [in] */ HDC hDC) = 0;
    
};
    

#endif 	/* __IHTMLElementRender_INTERFACE_DEFINED__ */




/****************************** WindowProc() ***************************
 * Our message handler for our window to host the browser.
 */

LRESULT CALLBACK WebPageWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{	
		/*
		case WM_PRINT:
		case WM_PRINTCLIENT:
		{			
			CMyOleClientSite *clientSite = (CMyOleClientSite *) GetWindowLong(hwnd, GWL_USERDATA);	
			if (clientSite)
			{
				IWebBrowser2	   *webBrowser2 = NULL;
				IHTMLDocument2	   *doc = NULL;
				IHTMLElement	   *body =	NULL;
				IHTMLElementRender *renderer = NULL;
				if (SUCCEEDED(clientSite->BrowserObject->QueryInterface(IID_IWebBrowser2, (void**)&webBrowser2)))
				{
					if (SUCCEEDED(webBrowser2->QueryInterface(IID_IHTMLDocument2, (void**)&doc)))
					{
						if (SUCCEEDED(doc->get_body(&body)))
						{
							if (SUCCEEDED(body->QueryInterface(IID_IHTMLElementRender, (void **) &renderer)))
							{
								renderer->DrawToDC((HDC) wParam);
							}
						}
					}
				}
				if (renderer)	 renderer->Release();
				if (body)		 body->Release();
				if (doc)		 doc->Release();
				if (webBrowser2) webBrowser2->Release();
			}        
			return TRUE;
		}
		break;
		*/
		case WM_SIZE:
		{
			// Resize the browser object to fit the window
			ResizeBrowser(hwnd, LOWORD(lParam), HIWORD(lParam));
			return(0);
		}

		case WM_CREATE:
		{
			// Embed the browser object into our host window. We need do this only
			// once. Note that the browser object will start calling some of our
			// IOleInPlaceFrame and IOleClientSite functions as soon as we start
			// calling browser object functions in EmbedBrowserObject().
			if (EmbedBrowserObject(hwnd)) return(-1);

			
			// Success
			return(0);
		}

		case WM_DESTROY:
		{
			// Detach the browser object from this window, and free resources.
			UnEmbedBrowserObject(hwnd);

			
			

			return(TRUE);
		}

		/*
		case WM_SHOWWINDOW:
		{
			CMyOleClientSite *clientSite = (CMyOleClientSite *) GetWindowLong(hwnd, GWL_USERDATA);	
			if (clientSite)
			{
				IWebBrowser2	   *webBrowser2 = NULL;				
				if (SUCCEEDED(clientSite->BrowserObject->QueryInterface(IID_IWebBrowser2, (void**)&webBrowser2)))
				{
					HWND wnd.
					if (SUCCEEDED(webBrowser2->get_HWND(&wnd))
					{
						::SetWindowPos(wnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE);
					}
					webBrowser2->Release();
				}
			}							
		}
		break;
		*/


	}
	return(DefWindowProc(hwnd, uMsg, wParam, lParam));
}


void CWebPageEmbedder::registerWebPageWindowClass(HINSTANCE hInstance, const char *name)
{
	WNDCLASSEX		wc;

	// Register the class of our window to host the browser. 'WindowProc' is our message handler
	// and 'ClassName' is the class name. You can choose any class name you want.
	ZeroMemory(&wc, sizeof(WNDCLASSEX));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.hInstance = hInstance;
	wc.lpfnWndProc = WebPageWindowProc;
	wc.lpszClassName = name;
	RegisterClassEx(&wc);
}

bool CWebPageEmbedder::isBusy(HWND hwnd)
{
	CMyOleClientSite *clientSite = (CMyOleClientSite *) GetWindowLong(hwnd, GWL_USERDATA);	
	bool result = false;
	if (clientSite)
	{
		IWebBrowser2	   *webBrowser2 = NULL;				
		if (SUCCEEDED(clientSite->BrowserObject->QueryInterface(IID_IWebBrowser2, (void**)&webBrowser2)))
		{
			VARIANT_BOOL vb;
			if (SUCCEEDED(webBrowser2->get_Busy(&vb)))
			{
				result = vb != 0;	
			}
			webBrowser2->Release();
		}
	}
	return result;
}


