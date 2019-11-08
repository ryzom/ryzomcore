#ifndef RY_WEB_PAGE_H
#define RY_WEB_PAGE_H

#include "nel/misc/types_nl.h"

#include <windows.h>
#include <oaidl.h>
#include <exdispid.h>



/** Helper to embbed a Web page in a control, and receive events from it
  * Original C code by Chris Becke
  * Adapted to C++ by Nicolas Vizerie for GameForge's Ryzom installer
  *
  * NB : Ole needs to be initialized to use use this class
  */  
struct CWebPageEmbedder
{
	// callback to receive event from the web browser, as defined by the DWebBrowserEvents2 interface
	struct CBrowserEventSink
	{
		// Called when an event is fired (parameters are just forwarded from IDispatch::Invoke)
		virtual void onInvoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pDispParams, VARIANT FAR *pVarResult, EXCEPINFO FAR *pExcepInfo, UINT FAR *puArgErr) = 0;
		virtual void onDestroy() {}
	};

	// Create a new class of window. Window with such a class will embed a browser in them
	static void registerWebPageWindowClass(HINSTANCE hInstance, const char *name);

	// Set the event sink for a browser window
	static void setBrowserEventSink(HWND hwnd, CBrowserEventSink *eventSink);

	/******************************* DisplayHTMLPage() ****************************
	 * Displays a URL, or HTML file on disk.
	 *
	 * hwnd =		Handle to the window hosting the browser object.
	 * webPageName =	Pointer to nul-terminated name of the URL/file.
	 *
	 * RETURNS: 0 if success, or non-zero if an error.
	 *
	 */
	static long displayHTMLPage(HWND hwnd, const char *webPageName);	

	// Test if a web page is currently busing downloading
	static bool isBusy(HWND hwnd);
};



#endif