/////////////////////////////////////////////////////////////////////////////
// SimpleBrowser.cpp: Web browser control
/////////////////////////////////////////////////////////////////////////////

#include "stdpch.h"
#include "comdef.h"
#include "mshtml.h"
#include "mshtmcid.h"
#include "mshtmhst.h"
#include "exdispid.h"

#include "SimpleBrowser.h"


#define Unused(parameter) parameter					// avoid compile warnings
													// about unused parameters


class CMyDocHostUIHandler : public IDocHostUIHandler
{
public:		

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **)
	{
		return E_NOTIMPL;
	}
	ULONG STDMETHODCALLTYPE AddRef()
	{
		return 1;
	}        
    ULONG STDMETHODCALLTYPE Release()
	{
		return 1;
	}     

    virtual HRESULT STDMETHODCALLTYPE ShowContextMenu( 
        /* [in] */ DWORD dwID,
        /* [in] */ POINT __RPC_FAR *ppt,
        /* [in] */ IUnknown __RPC_FAR *pcmdtReserved,
        /* [in] */ IDispatch __RPC_FAR *pdispReserved)
	{
		return S_OK;
	}
    
    virtual HRESULT STDMETHODCALLTYPE GetHostInfo( 
        /* [out][in] */ DOCHOSTUIINFO __RPC_FAR *pInfo)
	{		
		pInfo->dwFlags |= DOCHOSTUIFLAG_NO3DBORDER|DOCHOSTUIFLAG_SCROLL_NO;		
		pInfo->dwDoubleClick = DOCHOSTUIDBLCLK_DEFAULT;		
		return S_OK;
		
	}
    
    virtual HRESULT STDMETHODCALLTYPE ShowUI( 
        /* [in] */ DWORD dwID,
        /* [in] */ IOleInPlaceActiveObject __RPC_FAR *pActiveObject,
        /* [in] */ IOleCommandTarget __RPC_FAR *pCommandTarget,
        /* [in] */ IOleInPlaceFrame __RPC_FAR *pFrame,
        /* [in] */ IOleInPlaceUIWindow __RPC_FAR *pDoc)
	{
		return E_NOTIMPL;
	}
    
    virtual HRESULT STDMETHODCALLTYPE HideUI( void)
	{
		return E_NOTIMPL;
	}
    
    virtual HRESULT STDMETHODCALLTYPE UpdateUI( void)
	{
		return E_NOTIMPL;
	}
    
    virtual HRESULT STDMETHODCALLTYPE EnableModeless( 
        /* [in] */ BOOL fEnable)
	{
		return E_NOTIMPL;
	}
    
    virtual HRESULT STDMETHODCALLTYPE OnDocWindowActivate( 
        /* [in] */ BOOL fActivate)
	{
		return E_NOTIMPL;
	}
    
    virtual HRESULT STDMETHODCALLTYPE OnFrameWindowActivate( 
        /* [in] */ BOOL fActivate)
	{
		return E_NOTIMPL;
	}
    
    virtual HRESULT STDMETHODCALLTYPE ResizeBorder( 
        /* [in] */ LPCRECT prcBorder,
        /* [in] */ IOleInPlaceUIWindow __RPC_FAR *pUIWindow,
        /* [in] */ BOOL fRameWindow)
	{
		return E_NOTIMPL;
	}
    
    virtual HRESULT STDMETHODCALLTYPE TranslateAccelerator( 
        /* [in] */ LPMSG lpMsg,
        /* [in] */ const GUID __RPC_FAR *pguidCmdGroup,
        /* [in] */ DWORD nCmdID)
	{
		return E_NOTIMPL;
	}
    
    virtual HRESULT STDMETHODCALLTYPE GetOptionKeyPath( 
        /* [out] */ LPOLESTR __RPC_FAR *pchKey,
        /* [in] */ DWORD dw)
	{
		return E_NOTIMPL;
	}
    
    virtual HRESULT STDMETHODCALLTYPE GetDropTarget( 
        /* [in] */ IDropTarget __RPC_FAR *pDropTarget,
        /* [out] */ IDropTarget __RPC_FAR *__RPC_FAR *ppDropTarget)
	{
		return E_NOTIMPL;
	}
    
    virtual HRESULT STDMETHODCALLTYPE GetExternal( 
        /* [out] */ IDispatch __RPC_FAR *__RPC_FAR *ppDispatch)
	{
		return E_NOTIMPL;
	}
    
    virtual HRESULT STDMETHODCALLTYPE TranslateUrl( 
        /* [in] */ DWORD dwTranslate,
        /* [in] */ OLECHAR __RPC_FAR *pchURLIn,
        /* [out] */ OLECHAR __RPC_FAR *__RPC_FAR *ppchURLOut)
	{
		return E_NOTIMPL;
	}
    
    virtual HRESULT STDMETHODCALLTYPE FilterDataObject( 
        /* [in] */ IDataObject __RPC_FAR *pDO,
        /* [out] */ IDataObject __RPC_FAR *__RPC_FAR *ppDORet)
	{
		return E_NOTIMPL;
	}	        
};



class CMyOleInPlaceSite : public IOleInPlaceSite
{
public:
	HWND hWnd;

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **)
	{
		return E_NOTIMPL;
	}
	ULONG STDMETHODCALLTYPE AddRef()
	{
		return 1;
	}        
    ULONG STDMETHODCALLTYPE Release()
	{
		return 1;
	}
	virtual /* [input_sync] */ HRESULT STDMETHODCALLTYPE GetWindow( 
            /* [out] */ HWND __RPC_FAR *phwnd)
	{
		*phwnd = hWnd;
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE ContextSensitiveHelp( 
            /* [in] */ BOOL fEnterMode)
	{
		return E_NOTIMPL;
	}
    
	virtual HRESULT STDMETHODCALLTYPE CanInPlaceActivate( void) 
	{ 
		return S_OK; 
	}
    virtual HRESULT STDMETHODCALLTYPE OnInPlaceActivate( void) 
	{ 
		return S_OK; 
	}
    virtual HRESULT STDMETHODCALLTYPE OnUIActivate( void) { return E_NOTIMPL; }
    virtual HRESULT STDMETHODCALLTYPE GetWindowContext( 
            /* [out] */ IOleInPlaceFrame __RPC_FAR *__RPC_FAR *ppFrame,
            /* [out] */ IOleInPlaceUIWindow __RPC_FAR *__RPC_FAR *ppDoc,
            /* [out] */ LPRECT lprcPosRect,
            /* [out] */ LPRECT lprcClipRect,
            /* [out][in] */ LPOLEINPLACEFRAMEINFO lpFrameInfo)
	{ 
		return E_NOTIMPL; 
	}
        
    virtual HRESULT STDMETHODCALLTYPE Scroll( 
            /* [in] */ SIZE scrollExtant)
	{ 
		return E_NOTIMPL; 
	}
        
    virtual HRESULT STDMETHODCALLTYPE OnUIDeactivate(/* [in] */ BOOL fUndoable)
	{ 
		return E_NOTIMPL; 
	}
        
    virtual HRESULT STDMETHODCALLTYPE OnInPlaceDeactivate( void)
	{ 
		return E_NOTIMPL; 
	}
        
    virtual HRESULT STDMETHODCALLTYPE DiscardUndoState( void)
	{ 
		return E_NOTIMPL; 
	}
        
    virtual HRESULT STDMETHODCALLTYPE DeactivateAndUndo( void)
	{ 
		return E_NOTIMPL; 
	}
        
    virtual HRESULT STDMETHODCALLTYPE OnPosRectChange(/* [in] */ LPCRECT lprcPosRect)
	{ 
		return E_NOTIMPL; 
	}

};

class CMyOleClientSite : public IOleClientSite
{
public:
	CMyOleInPlaceSite _OleInPlaceSite;

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **dest)
	{
		if (memcmp(&riid, &IID_IUnknown, sizeof(GUID)) == 0 || memcmp(&riid, &IID_IOleClientSite, sizeof(GUID)) == 0)
		{
			*dest = this;
			return S_OK;
		}
		if (memcmp(&riid, &IID_IOleInPlaceSite, sizeof(GUID)) == 0)
		{
			*dest = (void **) &_OleInPlaceSite;
			return S_OK;
		}
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

    virtual HRESULT STDMETHODCALLTYPE SaveObject(void)
	{
		return E_NOTIMPL;
	}
    
    virtual HRESULT STDMETHODCALLTYPE GetMoniker( 
        /* [in] */ DWORD dwAssign,
        /* [in] */ DWORD dwWhichMoniker,
        /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmk)
	{
		return E_NOTIMPL;
	}
    
    virtual HRESULT STDMETHODCALLTYPE GetContainer( 
        /* [out] */ IOleContainer __RPC_FAR *__RPC_FAR *ppContainer)
	{
			// Tell the browser that we are a simple object and don't support a container
		//*ppContainer = 0;

		return(NOERROR);
	}
    
    virtual HRESULT STDMETHODCALLTYPE ShowObject( void)
	{
		return(NOERROR);		
	}
    
    virtual HRESULT STDMETHODCALLTYPE OnShowWindow(
        /* [in] */ BOOL fShow)
	{
		return E_NOTIMPL;
	}
    
    virtual HRESULT STDMETHODCALLTYPE RequestNewObjectLayout( void)
	{
		return E_NOTIMPL;
	}
    
};



/////////////////////////////////////////////////////////////////////////////
// Construction and creation
/////////////////////////////////////////////////////////////////////////////

SimpleBrowser::SimpleBrowser()
	: _Browser(NULL),
	  _BrowserDispatch(NULL)
{
	_DocHostUIHandler = new CMyDocHostUIHandler;
	_OleClientSite = new CMyOleClientSite;
}

SimpleBrowser::~SimpleBrowser()
{
    // release browser interfaces

	if (_Browser != NULL) {
		_Browser->Release();
		_Browser = NULL;
	}

	if (_BrowserDispatch != NULL) {
		_BrowserDispatch->Release();
		_BrowserDispatch = NULL;
	}

	delete _DocHostUIHandler;
	delete _OleClientSite;
}

// Standard create

BOOL SimpleBrowser::Create(DWORD dwStyle, 
                           const RECT& rect, 
                           CWnd* pParentWnd, 
						   UINT nID)
{
    BOOL results = TRUE;

    _Browser     = NULL;

    // create this window

    CWnd *window = this;
	results = window->Create(AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW),
	                         NULL,
							 dwStyle,
							 rect,
							 pParentWnd,
							 nID);
    if (!results) return FALSE;

	window->ShowWindow(SW_SHOW);

    // create browser control window as child of this window; 
	// this window sinks events from control

	/*
	CRect browser_window_rect(0,0,(rect.right - rect.left),(rect.bottom - rect.top));

    results = _BrowserWindow.CreateControl(CLSID_WebBrowser,
                                           NULL,
                                           (WS_VISIBLE | WS_TABSTOP),
                                           browser_window_rect,
                                           this,
                                           AFX_IDW_PANE_FIRST);
    if (!results) {
        DestroyWindow();
        return FALSE;
    }
	

    // get control interfaces

    LPUNKNOWN unknown = _BrowserWindow.GetControlUnknown();

    HRESULT hr = unknown->QueryInterface(IID_IWebBrowser2,(void **)&_Browser);
	*/
	
	((CMyOleClientSite *) _OleClientSite)->_OleInPlaceSite.hWnd = window->m_hWnd;

	// Create WebBrowser--store pointer in class member variable m_spWebBrowser
	if (CoCreateInstance(CLSID_WebBrowser, NULL, CLSCTX_INPROC, IID_IWebBrowser2, (void**)&_Browser) != S_OK)
	{
		DestroyWindow();
		return FALSE;
	}
	
	IOleObject *spOleObj;

	// Query WebBrowser for IOleObject pointer
	if (_Browser->QueryInterface(IID_IOleObject, (void**)&spOleObj) != S_OK)
	{
		DestroyWindow();
		return FALSE;
	}
			
	// Set client site
	spOleObj->SetClientSite(_OleClientSite);

	// In-place activate the WebBrowser control
	CRect browser_window_rect(0,0,(rect.right - rect.left),(rect.bottom - rect.top));		
	HRESULT r = spOleObj->DoVerb(OLEIVERB_INPLACEACTIVATE, NULL, _OleClientSite, 0, window->m_hWnd, &browser_window_rect);

	spOleObj->Release();


    
    if (_Browser->QueryInterface(IID_IDispatch,(void **)&_BrowserDispatch) != S_OK)
	{
		DestroyWindow();
		return FALSE;
	}
    
    
	// navigate to blank page; wait for document creation
	if (_Browser != NULL) {
	
		Navigate(_T("about:blank"));
		

		//IHTMLDocument2 *document = getDocument();
		//if (document != NULL) 
		//{
		//	// Add a IDocHostUIHandler interface
		//	/*ICustomDoc *customDoc;
		//	hr = document->QueryInterface(IID_ICustomDoc,(void **)&customDoc);
		//	if (SUCCEEDED(hr))
		//	{
		//		customDoc->SetUIHandler(_DocHostUIHandler);			
		//		customDoc->Release();
		//	}*/			
		//	document->Release();	
		//}

	}

    return TRUE;
}

/*
IHTMLDocument2 *SimpleBrowser::getDocument()
{
	ASSERT(_Browser);
	IHTMLDocument2 *document = NULL;
	HRESULT			hr       = S_OK;

	while ((document == NULL) && (hr == S_OK)) {

		Sleep(0);

		IDispatch *document_dispatch = NULL;

		hr = _Browser->get_Document(&document_dispatch);

		// if dispatch interface available, retrieve document interface
		
		if (SUCCEEDED(hr) && (document_dispatch != NULL)) {

			// retrieve document interface
			
			hr = document_dispatch->QueryInterface(IID_IHTMLDocument2,(void **)&document);

			document_dispatch->Release();

		}
		
	}
	return document;
}
*/


// Create in place of dialog control

BOOL SimpleBrowser::CreateFromControl(CWnd *pParentWnd,UINT nID)
{
	BOOL result = FALSE;

	ASSERT(pParentWnd != NULL);

	CWnd *control = pParentWnd->GetDlgItem(nID);

	if (control != NULL) {

		// get control location

		CRect		rect;

		control->GetWindowRect(&rect);
		pParentWnd->ScreenToClient(&rect);

		// destroy control, since the browser will take its place

		control->DestroyWindow();

		// create browser

		result = Create((WS_CHILD | WS_VISIBLE),
		                rect,
						pParentWnd,
						nID);

	}

	return result;
}

// Destruction

void SimpleBrowser::PostNcDestroy()
{
    // release browser interfaces

	if (_Browser != NULL) {
		_Browser->Release();
		_Browser = NULL;
	}

	if (_BrowserDispatch != NULL) {
		_BrowserDispatch->Release();
		_BrowserDispatch = NULL;
	}
}

/////////////////////////////////////////////////////////////////////////////
// Controls
/////////////////////////////////////////////////////////////////////////////

// Navigate to URL

void SimpleBrowser::Navigate(LPCTSTR URL)
{
	if (_Browser != NULL) {

		CString		url(URL);

		_variant_t	flags(0L,VT_I4);
		_variant_t	target_frame_name("");
		_variant_t	post_data("");
		_variant_t	headers("");

		_Browser->Navigate(url.AllocSysString(),
						   &flags,
						   &target_frame_name,
						   &post_data,
						   &headers);

	}
}

// Navigate to HTML document in resource

void SimpleBrowser::NavigateResource(int resource_ID)
{
	if (_Browser != NULL) {

		CString resource_string;

		// load HTML document from resource

		HRSRC resource_handle = FindResource(AfxGetResourceHandle(),
											 MAKEINTRESOURCE(resource_ID),
											 RT_HTML);

		if (resource_handle != NULL) {

			HGLOBAL resource = LoadResource(AfxGetResourceHandle(),
											resource_handle);

			if (resource != NULL) {

				LPVOID resource_memory = LockResource(resource);

				if (resource_memory != NULL) {

					DWORD resource_size = SizeofResource(AfxGetResourceHandle(),
														 resource_handle);

					// identify the resource document as MBCS (e.g. ANSI)
					// or UNICODE

					bool     UNICODE_document = false;

					wchar_t *UNICODE_memory   = (wchar_t *)resource_memory;
					int      UNICODE_size     = resource_size / sizeof(wchar_t);

					if (UNICODE_size >= 1) {

						// check for UNICODE byte order mark

						if (*UNICODE_memory == L'\xFEFF') {
							UNICODE_document = true;
							UNICODE_memory  += 1;
							UNICODE_size    -= 1;
						}

						// otherwise, check for UNICODE leading tag

						else if (UNICODE_size >= 5) {

							if ((UNICODE_memory[0]           == L'<') &&
							    (towupper(UNICODE_memory[1]) == L'H') &&
							    (towupper(UNICODE_memory[2]) == L'T') &&
							    (towupper(UNICODE_memory[3]) == L'M') &&
							    (towupper(UNICODE_memory[4]) == L'L')) {
								UNICODE_document = true;
							}

						}

						// Note: This logic assumes that the UNICODE resource document is 
						//       in little-endian byte order, which would be typical for 
						//       any HTML document used as a resource in a Windows application.

					}

					// convert resource document if required

#if !defined(UNICODE)

					if (UNICODE_document) {

						char *MBCS_buffer = resource_string.GetBufferSetLength(resource_size + 1);

						int MBCS_length = ::WideCharToMultiByte(CP_ACP,
						                                        0,
											                    UNICODE_memory,
											                    UNICODE_size,
											                    MBCS_buffer,
											                    resource_size + 1,
											                    NULL,
											                    NULL);

						resource_string.ReleaseBuffer(MBCS_length);

					}

					else {

						resource_string = CString((char *)resource_memory,resource_size);

					}

#else

					if (UNICODE_document) {

						resource_string = CString(UNICODE_memory,UNICODE_size);

					}

					else {

						wchar_t *UNICODE_buffer = resource_string.GetBufferSetLength(resource_size + 1);

						int UNICODE_length = ::MultiByteToWideChar(CP_ACP,
						                                           0,
											                       (const char *)resource_memory,
											                       resource_size,
											                       UNICODE_buffer,
											                       (resource_size + 1));

						resource_string.ReleaseBuffer(UNICODE_length);

					}

#endif


				}

			}

		}

		Clear();
		Write(resource_string);

	}
}

// Append string to current document

void SimpleBrowser::Write(LPCTSTR string)
{
	if (_Browser != NULL) {

		// get document interface

		
		IHTMLDocument2 *document = GetDocument();
		
		if (document != NULL) {

			// construct text to be written to browser as SAFEARRAY

			SAFEARRAY *safe_array = SafeArrayCreateVector(VT_VARIANT,0,1);
			
			VARIANT	*variant;
			
			SafeArrayAccessData(safe_array,(LPVOID *)&variant);
			
			variant->vt      = VT_BSTR;
			variant->bstrVal = CString(string).AllocSysString();
			
			SafeArrayUnaccessData(safe_array);

			// write SAFEARRAY to browser document

			document->write(safe_array);
			
			document->Release();
			document = NULL;

		}
		

	}
}

void SimpleBrowser::Clear()
{
	if (_Browser != NULL) {

		// if document interface available, close/re-open document to clear display

		IHTMLDocument2	*document	= GetDocument();
		HRESULT			hr			= S_OK;
		
		if (document != NULL) {

			// close and re-open document to empty contents

			document->close();
			
			VARIANT		open_name;
			VARIANT		open_features;
			VARIANT		open_replace;
			IDispatch	*open_window	= NULL;

			::VariantInit(&open_name);

			open_name.vt      = VT_BSTR;
			open_name.bstrVal = ::SysAllocString(L"_self");

			::VariantInit(&open_features);
			::VariantInit(&open_replace);
			
			hr = document->open(::SysAllocString(L"text/html"),
			                    open_name,
			                    open_features,
			                    open_replace,
			                    &open_window);

			if (hr == S_OK) {
				Refresh();
			}

			if (open_window != NULL) {
				open_window->Release();
			}

		}

		// otherwise, navigate to about:blank and wait for document ready

		else {

			Navigate(_T("about:blank"));

			IHTMLDocument2 *document = NULL;
			HRESULT			hr       = S_OK;

			while ((document == NULL) && (hr == S_OK)) {

				Sleep(0);

				IDispatch *document_dispatch = NULL;

				hr = _Browser->get_Document(&document_dispatch);

				// if dispatch interface available, retrieve document interface
				
				if (SUCCEEDED(hr) && (document_dispatch != NULL)) {

					// retrieve document interface
					
					hr = document_dispatch->QueryInterface(IID_IHTMLDocument2,(void **)&document);

					document_dispatch->Release();

				}
				
			}
			
			if (document != NULL) {
				document->Release();	
			}

		}
		
	}
}

// Navigation operations

void SimpleBrowser::GoBack()
{
	if (_Browser != NULL) {
		_Browser->GoBack();
	}
}

void SimpleBrowser::GoForward()
{
	if (_Browser != NULL) {
		_Browser->GoForward();
	}
}

void SimpleBrowser::GoHome()
{
	if (_Browser != NULL) {
		_Browser->GoHome();
	}
}

void SimpleBrowser::Refresh()
{
	if (_Browser != NULL) {
		_Browser->Refresh();
	}
}

void SimpleBrowser::Stop()
{
	if (_Browser != NULL) {
		_Browser->Stop();
	}
}

// Print contents

void SimpleBrowser::Print(LPCTSTR header,LPCTSTR footer)
{
	if (_Browser != NULL) {

		// construct two element SAFEARRAY;
		// first element is header string, second element is footer string

		HRESULT hr;

		VARIANT		header_variant;
		VariantInit(&header_variant);
		V_VT(&header_variant)   = VT_BSTR;
		V_BSTR(&header_variant) = CString(header).AllocSysString();

		VARIANT		footer_variant;
		VariantInit(&footer_variant);
		V_VT(&footer_variant)   = VT_BSTR;
		V_BSTR(&footer_variant) = CString(footer).AllocSysString();

		long index;

		SAFEARRAYBOUND	parameter_array_bound[1];
		SAFEARRAY		*parameter_array = NULL;

		parameter_array_bound[0].cElements = 2;
		parameter_array_bound[0].lLbound   = 0;

		parameter_array = SafeArrayCreate(VT_VARIANT,1,parameter_array_bound);

		index = 0;
		hr    = SafeArrayPutElement(parameter_array,&index,&header_variant);

		index = 1;
		hr    = SafeArrayPutElement(parameter_array,&index,&footer_variant);

		VARIANT	parameter;
		VariantInit(&parameter);
		V_VT(&parameter)    = VT_ARRAY | VT_BYREF;
		V_ARRAY(&parameter) = parameter_array;

		// start printing browser contents

		hr = _Browser->ExecWB(OLECMDID_PRINT,OLECMDEXECOPT_DODEFAULT,&parameter,NULL);

			// Note: There is no simple way to determine that printing has completed. 
			//       In fact, if the browser is destroyed while printing is in progress, 
			//       only part of the contents will be printed.

		// release SAFEARRAY

		if (!SUCCEEDED(hr)) {
			VariantClear(&header_variant);
			VariantClear(&footer_variant);
			if (parameter_array != NULL) {
				SafeArrayDestroy(parameter_array);
			}
		}

	}
}

// Miscellaneous

bool SimpleBrowser::GetBusy()
{
    bool busy = false;

	if (_Browser != NULL) {

		VARIANT_BOOL    variant_bool;

		HRESULT hr = _Browser->get_Busy(&variant_bool);
		if (SUCCEEDED(hr)) {
			busy = (variant_bool == VARIANT_TRUE);    
		}

	}

    return busy;
}

CString SimpleBrowser::GetLocationName()
{
    CString location_name = _T("");

	if (_Browser != NULL) {

		BSTR location_name_BSTR = NULL;

		HRESULT hr = _Browser->get_LocationName(&location_name_BSTR);
		if (SUCCEEDED(hr)) {
			location_name = location_name_BSTR;
		}

		::SysFreeString(location_name_BSTR);

	}

    return location_name;
}

CString SimpleBrowser::GetLocationURL()
{
    CString location_URL = _T("");

	if (_Browser != NULL) {

		BSTR location_URL_BSTR = NULL;

		HRESULT hr = _Browser->get_LocationURL(&location_URL_BSTR);
		if (SUCCEEDED(hr)) {
			location_URL = location_URL_BSTR;
		}

		::SysFreeString(location_URL_BSTR);

	}

    return location_URL;
}

READYSTATE SimpleBrowser::GetReadyState()
{
    READYSTATE readystate = READYSTATE_UNINITIALIZED;

    if (_Browser != NULL) {
		_Browser->get_ReadyState(&readystate);
	}

    return readystate;
}

bool SimpleBrowser::GetSilent()
{
    bool silent = false;

	if (_Browser != NULL) {

		VARIANT_BOOL silent_variant;

		HRESULT hr = _Browser->get_Silent(&silent_variant);
		if (SUCCEEDED(hr)) {
			silent = (silent_variant == VARIANT_TRUE);
		}

	}

    return silent;
}

void SimpleBrowser::PutSilent(bool silent)
{
	if (_Browser != NULL) {

		VARIANT_BOOL silent_variant;

		if (silent) silent_variant = VARIANT_TRUE;
		else        silent_variant = VARIANT_FALSE;

		_Browser->put_Silent(silent_variant);
	}
}

IHTMLDocument2 *SimpleBrowser::GetDocument()
{
	IHTMLDocument2 *document = NULL;
	
	if (_Browser != NULL) {
	
		// get browser document's dispatch interface

		IDispatch *document_dispatch = NULL;

		HRESULT hr = _Browser->get_Document(&document_dispatch);

		if (SUCCEEDED(hr) && (document_dispatch != NULL)) {

			// get the actual document interface

			hr = document_dispatch->QueryInterface(IID_IHTMLDocument2,
			                                       (void **)&document);

			// release dispatch interface
					
			document_dispatch->Release();
		
		}
		
	}
	
	return document;
}

/////////////////////////////////////////////////////////////////////////////
// Message handlers
/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(SimpleBrowser,CWnd)
    //{{AFX_MSG_MAP(SimpleBrowser)
	ON_WM_SIZE()	
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// Resize control window as this window is resized

void SimpleBrowser::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	if (_Browser != NULL) {
        CRect rect(0,0,cx,cy);	
		//_BrowserWindow.MoveWindow(&rect);
	}
}

/////////////////////////////////////////////////////////////////////////////
// Event handlers
/////////////////////////////////////////////////////////////////////////////

BEGIN_EVENTSINK_MAP(SimpleBrowser,CWnd)
    ON_EVENT(SimpleBrowser,AFX_IDW_PANE_FIRST,
	         DISPID_BEFORENAVIGATE2,
             _OnBeforeNavigate2, 
			 VTS_DISPATCH VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PBOOL)
    ON_EVENT(SimpleBrowser,AFX_IDW_PANE_FIRST,
	         DISPID_DOCUMENTCOMPLETE,
             _OnDocumentComplete, 
			 VTS_DISPATCH VTS_PVARIANT)
    ON_EVENT(SimpleBrowser,AFX_IDW_PANE_FIRST,
	         DISPID_DOWNLOADBEGIN,
             _OnDownloadBegin, 
			 VTS_NONE)
    ON_EVENT(SimpleBrowser,AFX_IDW_PANE_FIRST,
	         DISPID_PROGRESSCHANGE,
             _OnProgressChange, 
			 VTS_I4 VTS_I4)
    ON_EVENT(SimpleBrowser,AFX_IDW_PANE_FIRST,
	         DISPID_DOWNLOADCOMPLETE,
             _OnDownloadComplete, 
			 VTS_NONE)
    ON_EVENT(SimpleBrowser,AFX_IDW_PANE_FIRST,
	         DISPID_NAVIGATECOMPLETE2,
             _OnNavigateComplete2, 
			 VTS_DISPATCH VTS_PVARIANT)
    ON_EVENT(SimpleBrowser,AFX_IDW_PANE_FIRST,
	         DISPID_STATUSTEXTCHANGE,
             _OnStatusTextChange, 
			 VTS_BSTR)
    ON_EVENT(SimpleBrowser,AFX_IDW_PANE_FIRST,
	         DISPID_TITLECHANGE,
             _OnTitleChange, 
			 VTS_BSTR)
END_EVENTSINK_MAP()

SimpleBrowser::Notification::Notification(HWND hwnd,UINT ID,NotificationType type)
{
	hdr.hwndFrom	= hwnd;
	hdr.idFrom		= ID;
	hdr.code		= type;
	URL				= _T("");
	frame			= _T("");
	post_data		= NULL;
	post_data_size	= 0;
	headers			= _T("");
	progress		= 0;
	progress_max	= 0;
	text			= _T("");
}

// Called before navigation begins; application may cancel if required

void SimpleBrowser::_OnBeforeNavigate2(LPDISPATCH lpDisp,
                                       VARIANT FAR *URL,
                                       VARIANT FAR *Flags,
                                       VARIANT FAR *TargetFrameName,
                                       VARIANT FAR *PostData,
                                       VARIANT FAR *Headers,
                                       VARIANT_BOOL *Cancel)
{
	Unused(Flags);	// Note: flags value is reserved

    if (lpDisp == _BrowserDispatch) {

		CString				URL_string;
		CString				frame;
		unsigned char		*post_data		= NULL;
		int					post_data_size	= 0;
		CString				headers;

        if ((URL       != NULL) && 
		    (V_VT(URL) == VT_BSTR)) {
            URL_string = V_BSTR(URL);
        }

		if ((TargetFrameName       != NULL) &&
            (V_VT(TargetFrameName) == VT_BSTR)) {
			frame = V_BSTR(TargetFrameName);
        }

		if ((PostData       != NULL)                    &&
		    (V_VT(PostData) == (VT_VARIANT | VT_BYREF))) {

			VARIANT *PostData_variant = V_VARIANTREF(PostData);

			if ((PostData_variant       != NULL) &&
			    (V_VT(PostData_variant) != VT_EMPTY)) {

				SAFEARRAY *PostData_safearray = V_ARRAY(PostData_variant);

				if (PostData_safearray != NULL) {

					char *post_data_array = NULL;

					SafeArrayAccessData(PostData_safearray,(void HUGEP **)&post_data_array);

					long		lower_bound = 1;
					long		upper_bound = 1;

					SafeArrayGetLBound(PostData_safearray,1,&lower_bound);
					SafeArrayGetUBound(PostData_safearray,1,&upper_bound);

					post_data_size = (int)(upper_bound - lower_bound + 1);

					post_data = new unsigned char[post_data_size];

					memcpy(post_data,post_data_array,post_data_size);

					SafeArrayUnaccessData(PostData_safearray);

				}

			}

		}

		bool cancel = OnBeforeNavigate2(URL_string,
		                                frame,
										post_data,post_data_size,
										headers);

		if (Cancel != NULL) {
			if (cancel) *Cancel = VARIANT_TRUE;
			else        *Cancel = VARIANT_FALSE;
		}

		delete []post_data;

    }    
}

bool SimpleBrowser::OnBeforeNavigate2(CString URL,
                                      CString frame,
									  void    *post_data,int post_data_size,
									  CString headers)
{
	bool cancel = false;
	
	CWnd *parent = GetParent();
	
	if (parent != NULL) {

		Notification	notification(m_hWnd,GetDlgCtrlID(),BeforeNavigate2);
		
		notification.URL			= URL;
		notification.frame			= frame;
		notification.post_data		= post_data;
		notification.post_data_size	= post_data_size;
		notification.headers		= headers;

		LRESULT result = parent->SendMessage(WM_NOTIFY,
		                                     notification.hdr.idFrom,
											 (LPARAM)&notification);
		
		if (result) {
			cancel = true;
		}

	}
	
    return cancel;
}

// Document loaded and initialized

void SimpleBrowser::_OnDocumentComplete(LPDISPATCH lpDisp,VARIANT *URL)
{
    if (lpDisp == _BrowserDispatch) {

		CString URL_string;
		
		if ((URL       != NULL) &&
            (V_VT(URL) == VT_BSTR)) {
			URL_string = V_BSTR(URL);
        }

		

        OnDocumentComplete(URL_string);

    }    
}

void SimpleBrowser::OnDocumentComplete(CString URL)
{
	CWnd *parent = GetParent();
	
	if (parent != NULL) {

		Notification	notification(m_hWnd,GetDlgCtrlID(),DocumentComplete);
		
		notification.URL = URL;

		LRESULT result = parent->SendMessage(WM_NOTIFY,
		                                     notification.hdr.idFrom,
											 (LPARAM)&notification);				
	}
}

// Navigation/download progress

void SimpleBrowser::_OnDownloadBegin()
{
    OnDownloadBegin();
}

void SimpleBrowser::OnDownloadBegin()
{
	CWnd *parent = GetParent();
	
	if (parent != NULL) {

		Notification	notification(m_hWnd,GetDlgCtrlID(),DownloadBegin);
		
		LRESULT result = parent->SendMessage(WM_NOTIFY,
		                                     notification.hdr.idFrom,
											 (LPARAM)&notification);
		
	}
}

void SimpleBrowser::_OnProgressChange(long progress,long progress_max)
{
    OnProgressChange((int)progress,(int)progress_max);
}

void SimpleBrowser::OnProgressChange(int progress,int progress_max)
{
	CWnd *parent = GetParent();
	
	if (parent != NULL) {

		Notification	notification(m_hWnd,GetDlgCtrlID(),ProgressChange);
		
		notification.progress     = progress;
		notification.progress_max = progress_max;

		LRESULT result = parent->SendMessage(WM_NOTIFY,
		                                     notification.hdr.idFrom,
											 (LPARAM)&notification);
		
	}
}

void SimpleBrowser::_OnDownloadComplete()
{
    OnDownloadComplete();
}

void SimpleBrowser::OnDownloadComplete()
{
	CWnd *parent = GetParent();
	
	if (parent != NULL) {

		Notification	notification(m_hWnd,GetDlgCtrlID(),DownloadComplete);
		
		LRESULT result = parent->SendMessage(WM_NOTIFY,
		                                     notification.hdr.idFrom,
											 (LPARAM)&notification);
		
	}
}

// Navigation to a link has completed

void SimpleBrowser::_OnNavigateComplete2(LPDISPATCH lpDisp,VARIANT *URL)
{
	/*
	IHTMLDocument2 *document = getDocument();
	if (document != NULL) 
	{					
		IHTMLElement *body;
		if (document->get_body(&body) == S_OK && body)
		{				
			body->setAttribute(L"scroll", _variant_t("no"), 0);			
		}		
		document->Release();		
	}
	*/
		

    if (lpDisp == _BrowserDispatch) {

		CString URL_string;
		
		if ((URL       != NULL) &&
            (V_VT(URL) == VT_BSTR)) {
			URL_string = V_BSTR(URL);
        }

		OnNavigateComplete2(URL_string);

    }    
}

void SimpleBrowser::OnNavigateComplete2(CString URL)
{
	CWnd *parent = GetParent();
	
	if (parent != NULL) {

		Notification	notification(m_hWnd,GetDlgCtrlID(),NavigateComplete2);
		
		notification.URL = URL;

		LRESULT result = parent->SendMessage(WM_NOTIFY,
		                                     notification.hdr.idFrom,
											 (LPARAM)&notification);		
		
	}
}

// Status text has changed

void SimpleBrowser::_OnStatusTextChange(BSTR bstrText)
{
    CString text;
	
	if (bstrText != NULL) {
		text = (LPCTSTR)bstrText;
	}

    OnStatusTextChange(text);
}

void SimpleBrowser::OnStatusTextChange(CString text)
{
	CWnd *parent = GetParent();
	
	if (parent != NULL) {

		Notification	notification(m_hWnd,GetDlgCtrlID(),StatusTextChange);
		
		notification.text = text;

		LRESULT result = parent->SendMessage(WM_NOTIFY,
		                                     notification.hdr.idFrom,
											 (LPARAM)&notification);
		
	}
}

// Title text has changed

void SimpleBrowser::_OnTitleChange(BSTR bstrText)
{
    CString text;
	
	if (bstrText != NULL) {
		text = (LPCTSTR)bstrText;
	}

    OnTitleChange(text);
}

void SimpleBrowser::OnTitleChange(CString text)
{
	CWnd *parent = GetParent();
	
	if (parent != NULL) {

		Notification	notification(m_hWnd,GetDlgCtrlID(),TitleChange);
		
		notification.text = text;

		LRESULT result = parent->SendMessage(WM_NOTIFY,
		                                     notification.hdr.idFrom,
											 (LPARAM)&notification);
		
	}
}


