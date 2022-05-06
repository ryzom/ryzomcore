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

// Scintilla source code edit control
/** @file ScintillaWin.cxx
 ** Windows specific subclass of ScintillaBase.
 **/
// Copyright 1998-2002 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#include <limits.h>

#define _WIN32_WINNT  0x0400
#include <windows.h>
#include <commctrl.h>
#include <richedit.h>

#include "Platform.h"

#include "Scintilla.h"
#include "SString.h"
#ifdef SCI_LEXER
#include "SciLexer.h"
#include "PropSet.h"
#include "Accessor.h"
#include "KeyWords.h"
#endif
#include "ContractionState.h"
#include "SVector.h"
#include "CellBuffer.h"
#include "CallTip.h"
#include "KeyMap.h"
#include "Indicator.h"
#include "LineMarker.h"
#include "Style.h"
#include "AutoComplete.h"
#include "ViewStyle.h"
#include "Document.h"
#include "Editor.h"
#include "ScintillaBase.h"
#include "UniConversion.h"

#ifdef SCI_LEXER
#include "ExternalLexer.h"
#endif

#ifndef SPI_GETWHEELSCROLLLINES
#define SPI_GETWHEELSCROLLLINES   104
#endif

// These undefinitions are required to work around differences between different versions
// of the mingw headers, some of which define these twice, in both winuser.h and imm.h.
#ifdef __MINGW_H
#undef WM_IME_STARTCOMPOSITION
#undef WM_IME_ENDCOMPOSITION
#undef WM_IME_COMPOSITION
#undef WM_IME_KEYLAST
#undef WM_IME_SETCONTEXT
#undef WM_IME_NOTIFY
#undef WM_IME_CONTROL
#undef WM_IME_COMPOSITIONFULL
#undef WM_IME_SELECT
#undef WM_IME_CHAR
#undef WM_IME_KEYDOWN
#undef WM_IME_KEYUP
#endif

#ifndef WM_IME_STARTCOMPOSITION
#include <imm.h>
#endif

#include <commctrl.h>
#ifndef __BORLANDC__
#ifndef __DMC__
#include <zmouse.h>
#endif
#endif
#include <ole2.h>

#ifndef MK_ALT
#define MK_ALT 32
#endif

/** TOTAL_CONTROL ifdef surrounds code that will only work when ScintillaWin
 * is derived from ScintillaBase (all features) rather than directly from Editor
 * (lightweight editor).
 */
#define TOTAL_CONTROL

// GCC has trouble with the standard COM ABI so do it the old C way with explicit vtables.

const char scintillaClassName[] = "Scintilla";
const char callClassName[] = "CallTip";

class ScintillaWin; 	// Forward declaration for COM interface subobjects

/**
 */
class FormatEnumerator {
public:
	void **vtbl;
	int ref;
	int pos;
	CLIPFORMAT formats[2];
	int formatsLen;
	FormatEnumerator(int pos_, CLIPFORMAT formats_[], int formatsLen_);
};

/**
 */
class DropSource {
public:
	void **vtbl;
	ScintillaWin *sci;
	DropSource();
};

/**
 */
class DataObject {
public:
	void **vtbl;
	ScintillaWin *sci;
	DataObject();
};

/**
 */
class DropTarget {
public:
	void **vtbl;
	ScintillaWin *sci;
	DropTarget();
};

/**
 */
class ScintillaWin :
	public ScintillaBase {

	bool lastKeyDownConsumed;

	bool capturedMouse;

	unsigned int linesPerScroll;	///< Intellimouse support
	int wheelDelta; ///< Wheel delta from roll

	bool hasOKText;

	CLIPFORMAT cfColumnSelect;

	DropSource ds;
	DataObject dob;
	DropTarget dt;

	static HINSTANCE hInstance;

	ScintillaWin(HWND hwnd);
	ScintillaWin(const ScintillaWin &) : ScintillaBase() {}
	virtual ~ScintillaWin();
	ScintillaWin &operator=(const ScintillaWin &) { return *this; }

	virtual void Initialise();
	virtual void Finalise();
	HWND MainHWND();

	static sptr_t DirectFunction(
		    ScintillaWin *sci, UINT iMessage, uptr_t wParam, sptr_t lParam);
	static sptr_t PASCAL SWndProc(
		    HWND hWnd, UINT iMessage, WPARAM wParam, sptr_t lParam);
	static sptr_t PASCAL CTWndProc(
		    HWND hWnd, UINT iMessage, WPARAM wParam, sptr_t lParam);

	virtual void StartDrag();
	sptr_t WndPaint(uptr_t wParam);
	sptr_t HandleComposition(uptr_t wParam, sptr_t lParam);
	virtual sptr_t WndProc(unsigned int iMessage, uptr_t wParam, sptr_t lParam);
	virtual sptr_t DefWndProc(unsigned int iMessage, uptr_t wParam, sptr_t lParam);
	virtual void SetTicking(bool on);
	virtual void SetMouseCapture(bool on);
	virtual bool HaveMouseCapture();
	virtual void ScrollText(int linesToMove);
	virtual void SetVerticalScrollPos();
	virtual void SetHorizontalScrollPos();
	virtual bool ModifyScrollBars(int nMax, int nPage);
	virtual void NotifyChange();
	virtual void NotifyFocus(bool focus);
	virtual int GetCtrlID();
	virtual void NotifyParent(SCNotification scn);
	virtual void NotifyDoubleClick(Point pt, bool shift);
	virtual void Copy();
	virtual bool CanPaste();
	virtual void Paste();
	virtual void CreateCallTipWindow(PRectangle rc);
	virtual void AddToPopUp(const char *label, int cmd = 0, bool enabled = true);
	virtual void ClaimSelection();

	// DBCS
	void ImeStartComposition();
	void ImeEndComposition();

	void AddCharBytes(char b0, char b1=0);

	void GetIntelliMouseParameters();
	void CopySelTextToClipboard();
	void ScrollMessage(WPARAM wParam);
	void HorizontalScrollMessage(WPARAM wParam);
	void RealizeWindowPalette(bool inBackGround);
	void FullPaint();

public:
	/// Implement IUnknown
	STDMETHODIMP QueryInterface(REFIID riid, PVOID *ppv);
	STDMETHODIMP_(ULONG)AddRef();
	STDMETHODIMP_(ULONG)Release();

	/// Implement IDropTarget
	STDMETHODIMP DragEnter(LPDATAOBJECT pIDataSource, DWORD grfKeyState,
	                       POINTL pt, PDWORD pdwEffect);
	STDMETHODIMP DragOver(DWORD grfKeyState, POINTL pt, PDWORD pdwEffect);
	STDMETHODIMP DragLeave();
	STDMETHODIMP Drop(LPDATAOBJECT pIDataSource, DWORD grfKeyState,
	                  POINTL pt, PDWORD pdwEffect);

	/// Implement important part of IDataObject
	STDMETHODIMP GetData(FORMATETC *pFEIn, STGMEDIUM *pSTM);

	// External Lexers
#ifdef SCI_LEXER
	void SetLexerLanguage(const char *languageName);
	void SetLexer(uptr_t wParam);
#endif

	static bool Register(HINSTANCE hInstance_);
	static bool Unregister();

	friend class DropSource;
	friend class DataObject;
	friend class DropTarget;
	bool DragIsRectangularOK(CLIPFORMAT fmt) {
		return drag.rectangular && (fmt == cfColumnSelect);
	}
};

HINSTANCE ScintillaWin::hInstance = 0;

ScintillaWin::ScintillaWin(HWND hwnd) {

	lastKeyDownConsumed = false;

	capturedMouse = false;
	linesPerScroll = 0;
	wheelDelta = 0;   // Wheel delta from roll

	hasOKText = false;

	// There does not seem to be a real standard for indicating that the clipboard
	// contains a rectangular selection, so copy Developer Studio.
	cfColumnSelect = static_cast<CLIPFORMAT>(
		::RegisterClipboardFormat("MSDEVColumnSelect"));

	wMain = hwnd;

	dob.sci = this;
	ds.sci = this;
	dt.sci = this;

	Initialise();
}

ScintillaWin::~ScintillaWin() {}

void ScintillaWin::Initialise() {
	// Initialize COM.  If the app has already done this it will have
	// no effect.  If the app hasnt, we really shouldnt ask them to call
	// it just so this internal feature works.
	::OleInitialize(NULL);
}

void ScintillaWin::Finalise() {
	ScintillaBase::Finalise();
	SetTicking(false);
	::RevokeDragDrop(MainHWND());
	::OleUninitialize();
}

HWND ScintillaWin::MainHWND() {
	return reinterpret_cast<HWND>(wMain.GetID());
}

void ScintillaWin::StartDrag() {
	DWORD dwEffect = 0;
	dropWentOutside = true;
	IDataObject *pDataObject = reinterpret_cast<IDataObject *>(&dob);
	IDropSource *pDropSource = reinterpret_cast<IDropSource *>(&ds);
	//Platform::DebugPrintf("About to DoDragDrop %x %x\n", pDataObject, pDropSource);
	HRESULT hr = ::DoDragDrop(
	                 pDataObject,
	                 pDropSource,
	                 DROPEFFECT_COPY | DROPEFFECT_MOVE, &dwEffect);
	//Platform::DebugPrintf("DoDragDrop = %x\n", hr);
	if (SUCCEEDED(hr)) {
		if ((hr == DRAGDROP_S_DROP) && (dwEffect == DROPEFFECT_MOVE) && dropWentOutside) {
			// Remove dragged out text
			ClearSelection();
		}
	}
	inDragDrop = false;
	SetDragPosition(invalidPosition);
}

// Avoid warnings everywhere for old style casts by concentrating them here
static WORD LoWord(DWORD l) {
	return LOWORD(l);
}

static WORD HiWord(DWORD l) {
	return HIWORD(l);
}

static int InputCodePage() {
	HKL inputLocale = ::GetKeyboardLayout(0);
	LANGID inputLang = LOWORD(inputLocale);
	char sCodePage[10];
	int res = ::GetLocaleInfo(MAKELCID(inputLang, SORT_DEFAULT),
	  LOCALE_IDEFAULTANSICODEPAGE, sCodePage, sizeof(sCodePage));
	if (!res)
		return 0;
	return atoi(sCodePage);
}

/** Map the key codes to their equivalent SCK_ form. */
static int KeyTranslate(int keyIn) {
//PLATFORM_ASSERT(!keyIn);
	switch (keyIn) {
		case VK_DOWN:		return SCK_DOWN;
		case VK_UP:		return SCK_UP;
		case VK_LEFT:		return SCK_LEFT;
		case VK_RIGHT:		return SCK_RIGHT;
		case VK_HOME:		return SCK_HOME;
		case VK_END:		return SCK_END;
		case VK_PRIOR:		return SCK_PRIOR;
		case VK_NEXT:		return SCK_NEXT;
		case VK_DELETE:	return SCK_DELETE;
		case VK_INSERT:		return SCK_INSERT;
		case VK_ESCAPE:	return SCK_ESCAPE;
		case VK_BACK:		return SCK_BACK;
		case VK_TAB:		return SCK_TAB;
		case VK_RETURN:	return SCK_RETURN;
		case VK_ADD:		return SCK_ADD;
		case VK_SUBTRACT:	return SCK_SUBTRACT;
		case VK_DIVIDE:		return SCK_DIVIDE;
		default:			return keyIn;
	}
}

LRESULT ScintillaWin::WndPaint(uptr_t wParam) {
	//ElapsedTime et;

	// Redirect assertions to debug output and save current state
	bool assertsPopup = Platform::ShowAssertionPopUps(false);
	paintState = painting;
	PAINTSTRUCT ps;
	PAINTSTRUCT *pps;

	bool IsOcxCtrl = (wParam != 0); // if wParam != 0, it contains
								   // a PAINSTRUCT* from the OCX
	if (IsOcxCtrl) {
		pps = reinterpret_cast<PAINTSTRUCT*>(wParam);
	} else {
		pps = &ps;
		::BeginPaint(MainHWND(), pps);
	}
	AutoSurface surfaceWindow(pps->hdc, IsUnicodeMode());
	if (surfaceWindow) {
		rcPaint = PRectangle(pps->rcPaint.left, pps->rcPaint.top, pps->rcPaint.right, pps->rcPaint.bottom);
		PRectangle rcText = GetTextRectangle();
		paintingAllText = rcPaint.Contains(rcText);
		if (paintingAllText) {
			//Platform::DebugPrintf("Performing full text paint\n");
		} else {
			//Platform::DebugPrintf("Performing partial paint %d .. %d\n", rcPaint.top, rcPaint.bottom);
		}
		Paint(surfaceWindow, rcPaint);
		surfaceWindow->Release();
	}
	if(!IsOcxCtrl)
		::EndPaint(MainHWND(), pps);
	if (paintState == paintAbandoned) {
		// Painting area was insufficient to cover new styling or brace highlight positions
		FullPaint();
	}
	paintState = notPainting;

	// Restore debug output state
	Platform::ShowAssertionPopUps(assertsPopup);

	//Platform::DebugPrintf("Paint took %g\n", et.Duration());
	return 0l;
}

static BOOL IsNT() {
	OSVERSIONINFO osv = {sizeof(OSVERSIONINFO),0,0,0,0,""};
	::GetVersionEx(&osv);
	return osv.dwPlatformId == VER_PLATFORM_WIN32_NT;
}

sptr_t ScintillaWin::HandleComposition(uptr_t wParam, sptr_t lParam) {
#ifdef __DMC__
	// Digital Mars compiler does not include Imm library
	return 0;
#else
	sptr_t ret;
	if ((lParam & GCS_RESULTSTR) && (IsNT())) {
		HIMC hIMC = ::ImmGetContext(MainHWND());
		if (hIMC) {
			const int maxLenInputIME = 200;
			wchar_t wcs[maxLenInputIME];
			LONG bytes = ::ImmGetCompositionStringW(hIMC,
				GCS_RESULTSTR, wcs, (maxLenInputIME-1)*2);
			int wides = bytes / 2;
			if (IsUnicodeMode()) {
				char utfval[maxLenInputIME * 3];
				unsigned int len = UTF8Length(wcs, wides);
				UTF8FromUCS2(wcs, wides, utfval, len);
				utfval[len] = '\0';
				AddCharUTF(utfval, len);
			} else {
				char dbcsval[maxLenInputIME * 2];
				int size = ::WideCharToMultiByte(InputCodePage(),
					0, wcs, wides, dbcsval, sizeof(dbcsval) - 1, 0, 0);
				for (int i=0; i<size; i++) {
					AddChar(dbcsval[i]);
				}
			}
			::ImmReleaseContext(MainHWND(), hIMC);
		}
		ret = 0;
	} else {
		ret = ::DefWindowProc(MainHWND(), WM_IME_COMPOSITION, wParam, lParam);
	}
	if ((lParam & GCS_RESULTSTR)) {
		HIMC hIMC = ::ImmGetContext(MainHWND());
		Point pos = LocationFromPosition(currentPos);
		COMPOSITIONFORM CompForm;
		CompForm.dwStyle = CFS_POINT;
		CompForm.ptCurrentPos.x = pos.x;
		CompForm.ptCurrentPos.y = pos.y;
		::ImmSetCompositionWindow(hIMC, &CompForm);
		::ImmReleaseContext(MainHWND(), hIMC);
		DropCaret();
	}
	return ret;
#endif
}

// Translate message IDs from WM_* and EM_* to SCI_* so can partly emulate Windows Edit control
static unsigned int SciMessageFromEM(unsigned int iMessage) {
	switch (iMessage) {
	case EM_CANPASTE: return SCI_CANPASTE;
	case EM_CANUNDO: return SCI_CANUNDO;
	case EM_EMPTYUNDOBUFFER: return SCI_EMPTYUNDOBUFFER;
	case EM_FINDTEXTEX: return SCI_FINDTEXT;
	case EM_FORMATRANGE: return SCI_FORMATRANGE;
	case EM_GETFIRSTVISIBLELINE: return SCI_GETFIRSTVISIBLELINE;
	case EM_GETLINECOUNT: return SCI_GETLINECOUNT;
	case EM_GETSELTEXT: return SCI_GETSELTEXT;
	case EM_GETTEXTRANGE: return SCI_GETTEXTRANGE;
	case EM_HIDESELECTION: return SCI_HIDESELECTION;
	case EM_LINEINDEX: return SCI_POSITIONFROMLINE;
	case EM_LINESCROLL: return SCI_LINESCROLL;
	case EM_REPLACESEL: return SCI_REPLACESEL;
	case EM_SCROLLCARET: return SCI_SCROLLCARET;
	case EM_SETREADONLY: return SCI_SETREADONLY;
	case WM_CLEAR: return SCI_CLEAR;
	case WM_COPY: return SCI_COPY;
	case WM_CUT: return SCI_CUT;
	case WM_GETTEXT: return SCI_GETTEXT;
	case WM_GETTEXTLENGTH: return SCI_GETTEXTLENGTH;
	case WM_PASTE: return SCI_PASTE;
	case WM_UNDO: return SCI_UNDO;
	}
	return iMessage;
}

sptr_t ScintillaWin::WndProc(unsigned int iMessage, uptr_t wParam, sptr_t lParam) {
	//Platform::DebugPrintf("S M:%x WP:%x L:%x\n", iMessage, wParam, lParam);
	iMessage = SciMessageFromEM(iMessage);
	switch (iMessage) {

	case WM_CREATE:
		ctrlID = ::GetDlgCtrlID(reinterpret_cast<HWND>(wMain.GetID()));
		// Get Intellimouse scroll line parameters
		GetIntelliMouseParameters();
		::RegisterDragDrop(MainHWND(), reinterpret_cast<IDropTarget *>(&dt));
		break;

	case WM_COMMAND:
#ifdef TOTAL_CONTROL
		if (LoWord(wParam) == idAutoComplete) {
			int cmd = HiWord(wParam);
			if (cmd == LBN_DBLCLK) {
				AutoCompleteCompleted();
			} else {
				if (cmd != LBN_SETFOCUS)
					::SetFocus(MainHWND());
			}
		}
		Command(LoWord(wParam));
#endif
		break;

	case WM_PAINT:
		return WndPaint(wParam);

	case WM_VSCROLL:
		ScrollMessage(wParam);
		break;

	case WM_HSCROLL:
		HorizontalScrollMessage(wParam);
		break;

	case WM_SIZE: {
			//Platform::DebugPrintf("Scintilla WM_SIZE %d %d\n", LoWord(lParam), HiWord(lParam));
			ChangeSize();
		}
		break;

	case WM_MOUSEWHEEL:
		// Don't handle datazoom.
		// (A good idea for datazoom would be to "fold" or "unfold" details.
		// i.e. if datazoomed out only class structures are visible, when datazooming in the control
		// structures appear, then eventually the individual statements...)
		if (wParam & MK_SHIFT) {
			return ::DefWindowProc(MainHWND(), iMessage, wParam, lParam);
		}

		// Either SCROLL or ZOOM. We handle the wheel steppings calculation
		wheelDelta -= static_cast<short>(HiWord(wParam));
		if (abs(wheelDelta) >= WHEEL_DELTA && linesPerScroll > 0) {
			int linesToScroll = linesPerScroll;
			if (linesPerScroll == WHEEL_PAGESCROLL)
				linesToScroll = LinesOnScreen() - 1;
			if (linesToScroll == 0) {
				linesToScroll = 1;
			}
			linesToScroll *= (wheelDelta / WHEEL_DELTA);
			if (wheelDelta >= 0)
				wheelDelta = wheelDelta % WHEEL_DELTA;
			else
				wheelDelta = - (-wheelDelta % WHEEL_DELTA);

			if (wParam & MK_CONTROL) {
				// Zoom! We play with the font sizes in the styles.
				// Number of steps/line is ignored, we just care if sizing up or down
				if (linesToScroll < 0) {
					KeyCommand(SCI_ZOOMIN);
				} else {
					KeyCommand(SCI_ZOOMOUT);
				}
			} else {
				// Scroll
				ScrollTo(topLine + linesToScroll);
			}
		}
		return 0;

	case WM_TIMER:
		Tick();
		break;

	case WM_GETMINMAXINFO:
		return ::DefWindowProc(MainHWND(), iMessage, wParam, lParam);

	case WM_LBUTTONDOWN:
		//Platform::DebugPrintf("Buttdown %d %x %x %x %x %x\n",iMessage, wParam, lParam,
		//	Platform::IsKeyDown(VK_SHIFT),
		//	Platform::IsKeyDown(VK_CONTROL),
		//	Platform::IsKeyDown(VK_MENU));
		ButtonDown(Point::FromLong(lParam), ::GetTickCount(),
			(wParam & MK_SHIFT) != 0, 
			(wParam & MK_CONTROL) != 0, 
			Platform::IsKeyDown(VK_MENU));
		::SetFocus(MainHWND());
		break;

	case WM_MOUSEMOVE:
		ButtonMove(Point::FromLong(lParam));
		break;

	case WM_LBUTTONUP:
		ButtonUp(Point::FromLong(lParam), 
			::GetTickCount(), 
			(wParam & MK_CONTROL) != 0);
		break;

	case WM_SETCURSOR:
		if (LoWord(lParam) == HTCLIENT) {
			if (inDragDrop) {
				DisplayCursor(Window::cursorUp);
			} else {
				// Display regular (drag) cursor over selection
				POINT pt;
				::GetCursorPos(&pt);
				::ScreenToClient(MainHWND(), &pt);
				if (PointInSelMargin(Point(pt.x, pt.y))) {
					DisplayCursor(Window::cursorReverseArrow);
				} else if (PointInSelection(Point(pt.x, pt.y))) {
					DisplayCursor(Window::cursorArrow);
				} else {
					DisplayCursor(Window::cursorText);
				}
			}
			return TRUE;
		} else {
			return ::DefWindowProc(MainHWND(), iMessage, wParam, lParam);
		}

	case WM_CHAR:
		if (!iscntrl(wParam&0xff) || !lastKeyDownConsumed) {
			if (IsUnicodeMode()) {
				AddCharBytes(static_cast<char>(wParam & 0xff));
			} else {
				AddChar(static_cast<char>(wParam & 0xff));
			}
		}
		return 1;

	case WM_SYSKEYDOWN:
	case WM_KEYDOWN: {
		//Platform::DebugPrintf("S keydown %d %x %x %x %x\n",iMessage, wParam, lParam, ::IsKeyDown(VK_SHIFT), ::IsKeyDown(VK_CONTROL));
			lastKeyDownConsumed = false;
			int ret = KeyDown(KeyTranslate(wParam),
				Platform::IsKeyDown(VK_SHIFT),
				Platform::IsKeyDown(VK_CONTROL),
				Platform::IsKeyDown(VK_MENU),
				&lastKeyDownConsumed);
			if (!ret && !lastKeyDownConsumed) {
				return ::DefWindowProc(MainHWND(), iMessage, wParam, lParam);
			}
			break;
		}

	case WM_IME_KEYDOWN:
		return ::DefWindowProc(MainHWND(), iMessage, wParam, lParam);

	case WM_KEYUP:
		//Platform::DebugPrintf("S keyup %d %x %x\n",iMessage, wParam, lParam);
		return ::DefWindowProc(MainHWND(), iMessage, wParam, lParam);

	case WM_SETTINGCHANGE:
		//Platform::DebugPrintf("Setting Changed\n");
		InvalidateStyleData();
		// Get Intellimouse scroll line parameters
		GetIntelliMouseParameters();
		break;

	case WM_GETDLGCODE:
		return DLGC_HASSETSEL | DLGC_WANTALLKEYS;

	case WM_KILLFOCUS:
		SetFocusState(false);
		//RealizeWindowPalette(true);
		break;

	case WM_SETFOCUS:
		SetFocusState(true);
		RealizeWindowPalette(false);
		break;

	case WM_SYSCOLORCHANGE:
		//Platform::DebugPrintf("Setting Changed\n");
		InvalidateStyleData();
		break;

	case WM_PALETTECHANGED:
		if (wParam != reinterpret_cast<uptr_t>(MainHWND())) {
			//Platform::DebugPrintf("** Palette Changed\n");
			RealizeWindowPalette(true);
		}
		break;

	case WM_QUERYNEWPALETTE:
		//Platform::DebugPrintf("** Query palette\n");
		RealizeWindowPalette(false);
		break;

	case WM_IME_STARTCOMPOSITION: 	// dbcs
		ImeStartComposition();
		return ::DefWindowProc(MainHWND(), iMessage, wParam, lParam);

	case WM_IME_ENDCOMPOSITION: 	// dbcs
		ImeEndComposition();
		return ::DefWindowProc(MainHWND(), iMessage, wParam, lParam);

	case WM_IME_COMPOSITION:
		return HandleComposition(wParam, lParam);

	case WM_IME_CHAR: {
			AddCharBytes(HIBYTE(wParam), LOBYTE(wParam));
			return 0;
		}

	case WM_CONTEXTMENU:
#ifdef TOTAL_CONTROL
		if (displayPopupMenu) {
			Point pt = Point::FromLong(lParam);
			if ((pt.x == -1) && (pt.y == -1)) {
				// Caused by keyboard so display menu near caret
				pt = LocationFromPosition(currentPos);
				POINT spt = {pt.x, pt.y};
				::ClientToScreen(MainHWND(), &spt);
				pt = Point(spt.x, spt.y);
			}
			ContextMenu(pt);
			return 0;
		}
#endif
		return ::DefWindowProc(MainHWND(), iMessage, wParam, lParam);

	case WM_INPUTLANGCHANGE:
		//::SetThreadLocale(LOWORD(lParam));
		return ::DefWindowProc(MainHWND(), iMessage, wParam, lParam);

	case WM_INPUTLANGCHANGEREQUEST:
		return ::DefWindowProc(MainHWND(), iMessage, wParam, lParam);

	case WM_ERASEBKGND:
		return 1;   // Avoid any background erasure as whole window painted.

	case WM_CAPTURECHANGED:
		capturedMouse = false;
		return 0;

        // These are not handled in Scintilla and its faster to dispatch them here.
        // Also moves time out to here so profile doesn't count lots of empty message calls.
	case WM_MOVE:
	case WM_MOUSEACTIVATE:
	case WM_NCHITTEST:
	case WM_NCCALCSIZE:
	case WM_NCPAINT:
	case WM_NCMOUSEMOVE:
	case WM_NCLBUTTONDOWN:
	case WM_IME_SETCONTEXT:
	case WM_IME_NOTIFY:
	case WM_SYSCOMMAND:
	case WM_WINDOWPOSCHANGING:
	case WM_WINDOWPOSCHANGED:
		return ::DefWindowProc(MainHWND(), iMessage, wParam, lParam);

	case EM_LINEFROMCHAR:
		if (static_cast<int>(wParam) < 0) {
			wParam = SelectionStart();
		}
		return pdoc->LineFromPosition(wParam);

	case EM_EXLINEFROMCHAR:
		return pdoc->LineFromPosition(lParam);

	case EM_GETSEL:
		if (wParam) {
			*reinterpret_cast<int *>(wParam) = SelectionStart();
		}
		if (lParam) {
			*reinterpret_cast<int *>(lParam) = SelectionEnd();
		}
		return MAKELONG(SelectionStart(), SelectionEnd());

	case EM_EXGETSEL: {
			if (lParam == 0) {
				return 0;
			}
			CharacterRange *pCR = reinterpret_cast<CharacterRange *>(lParam);
			pCR->cpMin = SelectionStart();
			pCR->cpMax = SelectionEnd();
		}
		break;

	case EM_SETSEL: {
			int nStart = static_cast<int>(wParam);
			int nEnd = static_cast<int>(lParam);
			if (nStart == 0 && nEnd == -1) {
				nEnd = pdoc->Length();
			}
			if (nStart == -1) {
				nStart = nEnd;	// Remove selection
			}
			if (nStart > nEnd) {
				SetSelection(nEnd, nStart);
			} else {
				SetSelection(nStart, nEnd);
			}
			EnsureCaretVisible();
		}
		break;

	case EM_EXSETSEL: {
			if (lParam == 0) {
				return 0;
			}
			CharacterRange *pCR = reinterpret_cast<CharacterRange *>(lParam);
			selType = selStream;
			if (pCR->cpMax == 0 && pCR->cpMax == -1) {
				SetSelection(pCR->cpMin, pdoc->Length());
			} else {
				SetSelection(pCR->cpMin, pCR->cpMax);
			}
			EnsureCaretVisible();
			return pdoc->LineFromPosition(SelectionStart());
		}

	case SCI_GETDIRECTFUNCTION:
		return reinterpret_cast<sptr_t>(DirectFunction);

	case SCI_GETDIRECTPOINTER:
		return reinterpret_cast<sptr_t>(this);

	case SCI_GRABFOCUS:
		::SetFocus(MainHWND());
		break;

	default:
		return ScintillaBase::WndProc(iMessage, wParam, lParam);
	}
	return 0l;
}

sptr_t ScintillaWin::DefWndProc(unsigned int iMessage, uptr_t wParam, sptr_t lParam) {
	return ::DefWindowProc(MainHWND(), iMessage, wParam, lParam);
}

void ScintillaWin::SetTicking(bool on) {
	if (timer.ticking != on) {
		timer.ticking = on;
		if (timer.ticking) {
			timer.tickerID = reinterpret_cast<TickerID>(::SetTimer(MainHWND(), 1, timer.tickSize, NULL));
		} else {
			::KillTimer(MainHWND(), reinterpret_cast<uptr_t>(timer.tickerID));
			timer.tickerID = 0;
		}
	}
	timer.ticksToWait = caret.period;
}

void ScintillaWin::SetMouseCapture(bool on) {
	if (mouseDownCaptures) {
		if (on) {
			::SetCapture(MainHWND());
		} else {
			::ReleaseCapture();
		}
	}
	capturedMouse = on;
}

bool ScintillaWin::HaveMouseCapture() {
	// Cannot just see if GetCapture is this window as the scroll bar also sets capture for the window
	return capturedMouse;
	//return capturedMouse && (::GetCapture() == MainHWND());
}

void ScintillaWin::ScrollText(int linesToMove) {
	//Platform::DebugPrintf("ScintillaWin::ScrollText %d\n", linesToMove);
	::ScrollWindow(MainHWND(), 0,
		vs.lineHeight * linesToMove, 0, 0);
	::UpdateWindow(MainHWND());
}

// Change the scroll position but avoid repaint if changing to same value
static void ChangeScrollPos(HWND w, int barType, int pos) {
	SCROLLINFO sci = {
		sizeof(sci),0,0,0,0,0,0
	};
	sci.fMask = SIF_POS;
	::GetScrollInfo(w, barType, &sci);
	if (sci.nPos != pos) {
		sci.nPos = pos;
		::SetScrollInfo(w, barType, &sci, TRUE);
	}
}

void ScintillaWin::SetVerticalScrollPos() {
	ChangeScrollPos(MainHWND(), SB_VERT, topLine);
}

void ScintillaWin::SetHorizontalScrollPos() {
	ChangeScrollPos(MainHWND(), SB_HORZ, xOffset);
}

bool ScintillaWin::ModifyScrollBars(int nMax, int nPage) {
	bool modified = false;
	SCROLLINFO sci = {
		sizeof(sci),0,0,0,0,0,0
	};
	sci.fMask = SIF_PAGE | SIF_RANGE;
	::GetScrollInfo(MainHWND(), SB_VERT, &sci);
	if ((sci.nMin != 0) || 
		(sci.nMax != nMax) ||
	        (sci.nPage != static_cast<unsigned int>(nPage)) ||
	        (sci.nPos != 0)) {
		//Platform::DebugPrintf("Scroll info changed %d %d %d %d %d\n",
		//	sci.nMin, sci.nMax, sci.nPage, sci.nPos, sci.nTrackPos);
		sci.fMask = SIF_PAGE | SIF_RANGE;
		sci.nMin = 0;
		sci.nMax = nMax;
		sci.nPage = nPage;
		sci.nPos = 0;
		sci.nTrackPos = 1;
		::SetScrollInfo(MainHWND(), SB_VERT, &sci, TRUE);
		modified = true;
	}

	PRectangle rcText = GetTextRectangle();
	int horizEndPreferred = scrollWidth;
	if (horizEndPreferred < 0)
		horizEndPreferred = 0;
	if (!horizontalScrollBarVisible || (wrapState != eWrapNone))
		horizEndPreferred = 0;
	unsigned int pageWidth = rcText.Width();
	sci.fMask = SIF_PAGE | SIF_RANGE;
	::GetScrollInfo(MainHWND(), SB_HORZ, &sci);
	if ((sci.nMin != 0) || 
		(sci.nMax != horizEndPreferred) ||
		(sci.nPage != pageWidth) ||
	        (sci.nPos != 0)) {
		sci.fMask = SIF_PAGE | SIF_RANGE;
		sci.nMin = 0;
		sci.nMax = horizEndPreferred;
		sci.nPage = pageWidth;
		sci.nPos = 0;
		sci.nTrackPos = 1;
		::SetScrollInfo(MainHWND(), SB_HORZ, &sci, TRUE);
		modified = true;
		if (scrollWidth < static_cast<int>(pageWidth)) {
			HorizontalScrollTo(0);
		}
	}
	return modified;
}

void ScintillaWin::NotifyChange() {
	::SendMessage(::GetParent(MainHWND()), WM_COMMAND,
	        MAKELONG(GetCtrlID(), SCEN_CHANGE),
		reinterpret_cast<LPARAM>(MainHWND()));
}

void ScintillaWin::NotifyFocus(bool focus) {
	::SendMessage(::GetParent(MainHWND()), WM_COMMAND,
	        MAKELONG(GetCtrlID(), focus ? SCEN_SETFOCUS : SCEN_KILLFOCUS),
		reinterpret_cast<LPARAM>(MainHWND()));
}

int ScintillaWin::GetCtrlID() {
	return ::GetDlgCtrlID(reinterpret_cast<HWND>(wMain.GetID()));
}

void ScintillaWin::NotifyParent(SCNotification scn) {
	scn.nmhdr.hwndFrom = MainHWND();
	scn.nmhdr.idFrom = GetCtrlID();
	::SendMessage(::GetParent(MainHWND()), WM_NOTIFY,
	              GetCtrlID(), reinterpret_cast<LPARAM>(&scn));
}

void ScintillaWin::NotifyDoubleClick(Point pt, bool shift) {
	//Platform::DebugPrintf("ScintillaWin Double click 0\n");
	ScintillaBase::NotifyDoubleClick(pt, shift);
	// Send myself a WM_LBUTTONDBLCLK, so the container can handle it too.
	::SendMessage(MainHWND(),
			  WM_LBUTTONDBLCLK,
			  shift ? MK_SHIFT : 0,
			  MAKELPARAM(pt.x, pt.y));
}

void ScintillaWin::Copy() {
	//Platform::DebugPrintf("Copy\n");
	if (currentPos != anchor) {
		::OpenClipboard(MainHWND());
		::EmptyClipboard();
		CopySelTextToClipboard();
		if (selType == selRectangle) {
			::SetClipboardData(cfColumnSelect, 0);
		}
		::CloseClipboard();
	}
}

bool ScintillaWin::CanPaste() {
	if (!Editor::CanPaste())
		return false;
	if (::IsClipboardFormatAvailable(CF_TEXT))
		return true;
	if (IsUnicodeMode())
		return ::IsClipboardFormatAvailable(CF_UNICODETEXT) != 0;
	return false;
}

void ScintillaWin::Paste() {
	pdoc->BeginUndoAction();
	int selStart = SelectionStart();
	ClearSelection();
	::OpenClipboard(MainHWND());
	bool isRectangular = ::IsClipboardFormatAvailable(cfColumnSelect) != 0;
	HGLOBAL hmemUSelection = 0;
	if (IsUnicodeMode()) {
		hmemUSelection = ::GetClipboardData(CF_UNICODETEXT);
		if (hmemUSelection) {
			wchar_t *uptr = static_cast<wchar_t *>(::GlobalLock(hmemUSelection));
			if (uptr) {
				unsigned int bytes = ::GlobalSize(hmemUSelection);
				unsigned int len = UTF8Length(uptr, bytes/2);
				char *putf = new char[len+1];
				if (putf) {
					UTF8FromUCS2(uptr, bytes/2, putf, len);
					if (isRectangular) {
						PasteRectangular(selStart, putf, len);
					} else {
						if (pdoc->InsertString(currentPos, putf, len)) {
							SetEmptySelection(currentPos + len);
						}
					}
					delete []putf;
				}
			}
			::GlobalUnlock(hmemUSelection);
		}
	}
	if (!hmemUSelection) {
		HGLOBAL hmemSelection = ::GetClipboardData(CF_TEXT);
		if (hmemSelection) {
			char *ptr = static_cast<char *>(
				::GlobalLock(hmemSelection));
			if (ptr) {
				unsigned int bytes = ::GlobalSize(hmemSelection);
				unsigned int len = bytes;
				for (unsigned int i = 0; i < bytes; i++) {
					if ((len == bytes) && (0 == ptr[i]))
						len = i;
				}
				if (isRectangular) {
					PasteRectangular(selStart, ptr, len);
				} else {
					pdoc->InsertString(currentPos, ptr, len);
					SetEmptySelection(currentPos + len);
				}
			}
			::GlobalUnlock(hmemSelection);
		}
	}
	::CloseClipboard();
	pdoc->EndUndoAction();
	NotifyChange();
	Redraw();
}

void ScintillaWin::CreateCallTipWindow(PRectangle) {
#ifdef TOTAL_CONTROL
	ct.wCallTip = ::CreateWindow(callClassName, "ACallTip",
				     WS_VISIBLE | WS_CHILD, 100, 100, 150, 20,
				     MainHWND(), reinterpret_cast<HMENU>(idCallTip),
				     reinterpret_cast<HINSTANCE>(::GetWindowLong(MainHWND(),GWL_HINSTANCE)),
				     &ct);
	ct.wDraw = ct.wCallTip;
#endif
}

void ScintillaWin::AddToPopUp(const char *label, int cmd, bool enabled) {
#ifdef TOTAL_CONTROL
	HMENU hmenuPopup = reinterpret_cast<HMENU>(popup.GetID());
	if (!label[0])
		::AppendMenu(hmenuPopup, MF_SEPARATOR, 0, "");
	else if (enabled)
		::AppendMenu(hmenuPopup, MF_STRING, cmd, label);
	else
		::AppendMenu(hmenuPopup, MF_STRING | MF_DISABLED | MF_GRAYED, cmd, label);
#endif
}

void ScintillaWin::ClaimSelection() {
	// Windows does not have a primary selection
}

#ifdef SCI_LEXER

/*

  Initial Windows-Only implementation of the external lexer
  system in ScintillaWin class. Intention is to create a LexerModule
  subclass (?) to have lex and fold methods which will call out to their
  relevant DLLs...

*/

void ScintillaWin::SetLexer(uptr_t wParam) {
	lexLanguage = wParam;
	lexCurrent = LexerModule::Find(lexLanguage);
	if (!lexCurrent)
		lexCurrent = LexerModule::Find(SCLEX_NULL);
}

void ScintillaWin::SetLexerLanguage(const char *languageName) {
	lexLanguage = SCLEX_CONTAINER;
	lexCurrent = LexerModule::Find(languageName);
	if (!lexCurrent)
		lexCurrent = LexerModule::Find(SCLEX_NULL);
	if (lexCurrent)
		lexLanguage = lexCurrent->GetLanguage();
}

#endif

/// Implement IUnknown

STDMETHODIMP_(ULONG)FormatEnumerator_AddRef(FormatEnumerator *fe);
STDMETHODIMP FormatEnumerator_QueryInterface(FormatEnumerator *fe, REFIID riid, PVOID *ppv) {
	//Platform::DebugPrintf("EFE QI");
	*ppv = NULL;
	if (riid == IID_IUnknown)
		*ppv = reinterpret_cast<IEnumFORMATETC *>(fe);
	if (riid == IID_IEnumFORMATETC)
		*ppv = reinterpret_cast<IEnumFORMATETC *>(fe);
	if (!*ppv)
		return E_NOINTERFACE;
	FormatEnumerator_AddRef(fe);
	return S_OK;
}
STDMETHODIMP_(ULONG)FormatEnumerator_AddRef(FormatEnumerator *fe) {
	return ++fe->ref;
}
STDMETHODIMP_(ULONG)FormatEnumerator_Release(FormatEnumerator *fe) {
	fe->ref--;
	if (fe->ref > 0)
		return fe->ref;
	delete fe;
	return 0;
}
/// Implement IEnumFORMATETC
STDMETHODIMP FormatEnumerator_Next(FormatEnumerator *fe, ULONG celt, FORMATETC *rgelt, ULONG *pceltFetched) {
	//Platform::DebugPrintf("EFE Next %d %d", fe->pos, celt);
	if (rgelt == NULL) return E_POINTER;
	// We only support one format, so this is simple.
	unsigned int putPos = 0;
	while ((fe->pos < fe->formatsLen) && (putPos < celt)) {
		rgelt->cfFormat = fe->formats[fe->pos];
		rgelt->ptd = 0;
		rgelt->dwAspect = DVASPECT_CONTENT;
		rgelt->lindex = -1;
		rgelt->tymed = TYMED_HGLOBAL;
		fe->pos++;
		putPos++;
	}
	if (pceltFetched)
		*pceltFetched = putPos;
	return putPos ? S_OK : S_FALSE;
}
STDMETHODIMP FormatEnumerator_Skip(FormatEnumerator *fe, ULONG celt) {
	fe->pos += celt;
	return S_OK;
}
STDMETHODIMP FormatEnumerator_Reset(FormatEnumerator *fe) {
	fe->pos = 0;
	return S_OK;
}
STDMETHODIMP FormatEnumerator_Clone(FormatEnumerator *fe, IEnumFORMATETC **ppenum) {
	FormatEnumerator *pfe = new FormatEnumerator(fe->pos, fe->formats, fe->formatsLen);
	return FormatEnumerator_QueryInterface(pfe, IID_IEnumFORMATETC,
	                                       reinterpret_cast<void **>(ppenum));
}

static void *vtFormatEnumerator[] = {
	FormatEnumerator_QueryInterface,
	FormatEnumerator_AddRef,
	FormatEnumerator_Release,
	FormatEnumerator_Next,
	FormatEnumerator_Skip,
	FormatEnumerator_Reset,
	FormatEnumerator_Clone
};

FormatEnumerator::FormatEnumerator(int pos_, CLIPFORMAT formats_[], int formatsLen_) {
	vtbl = vtFormatEnumerator;
	ref = 0;   // First QI adds first reference...
	pos = pos_;
	formatsLen = formatsLen_;
	for (int i=0;i<formatsLen;i++)
		formats[i] = formats_[i];
}

/// Implement IUnknown
STDMETHODIMP DropSource_QueryInterface(DropSource *ds, REFIID riid, PVOID *ppv) {
	return ds->sci->QueryInterface(riid, ppv);
}
STDMETHODIMP_(ULONG)DropSource_AddRef(DropSource *ds) {
	return ds->sci->AddRef();
}
STDMETHODIMP_(ULONG)DropSource_Release(DropSource *ds) {
	return ds->sci->Release();
}

/// Implement IDropSource
STDMETHODIMP DropSource_QueryContinueDrag(DropSource *, BOOL fEsc, DWORD grfKeyState) {
	if (fEsc)
		return DRAGDROP_S_CANCEL;
	if (!(grfKeyState & MK_LBUTTON))
		return DRAGDROP_S_DROP;
	return S_OK;
}

STDMETHODIMP DropSource_GiveFeedback(DropSource *, DWORD) {
	return DRAGDROP_S_USEDEFAULTCURSORS;
}

static void *vtDropSource[] = {
	DropSource_QueryInterface,
	DropSource_AddRef,
	DropSource_Release,
	DropSource_QueryContinueDrag,
	DropSource_GiveFeedback
};

DropSource::DropSource() {
	vtbl = vtDropSource;
	sci = 0;
}

/// Implement IUnkown
STDMETHODIMP DataObject_QueryInterface(DataObject *pd, REFIID riid, PVOID *ppv) {
	//Platform::DebugPrintf("DO QI %x\n", pd);
	return pd->sci->QueryInterface(riid, ppv);
}
STDMETHODIMP_(ULONG)DataObject_AddRef(DataObject *pd) {
	return pd->sci->AddRef();
}
STDMETHODIMP_(ULONG)DataObject_Release(DataObject *pd) {
	return pd->sci->Release();
}
/// Implement IDataObject
STDMETHODIMP DataObject_GetData(DataObject *pd, FORMATETC *pFEIn, STGMEDIUM *pSTM) {
	return pd->sci->GetData(pFEIn, pSTM);
}

STDMETHODIMP DataObject_GetDataHere(DataObject *, FORMATETC *, STGMEDIUM *) {
	//Platform::DebugPrintf("DOB GetDataHere\n");
	return E_NOTIMPL;
}

STDMETHODIMP DataObject_QueryGetData(DataObject *pd, FORMATETC *pFE) {
	if (pd->sci->DragIsRectangularOK(pFE->cfFormat) &&
	    pFE->ptd == 0 &&
	    (pFE->dwAspect & DVASPECT_CONTENT) != 0 &&
	    pFE->lindex == -1 &&
	    (pFE->tymed & TYMED_HGLOBAL) != 0
	) {
		return S_OK;
	}

	bool formatOK = (pFE->cfFormat == CF_TEXT) ||
		((pFE->cfFormat == CF_UNICODETEXT) && pd->sci->IsUnicodeMode()) ||
		(pFE->cfFormat == CF_HDROP);
	if (!formatOK ||
	    pFE->ptd != 0 ||
	    (pFE->dwAspect & DVASPECT_CONTENT) == 0 ||
	    pFE->lindex != -1 ||
	    (pFE->tymed & TYMED_HGLOBAL) == 0
	) {
		//Platform::DebugPrintf("DOB QueryGetData No %x\n",pFE->cfFormat);
		//return DATA_E_FORMATETC;
		return S_FALSE;
	}
	//Platform::DebugPrintf("DOB QueryGetData OK %x\n",pFE->cfFormat);
	return S_OK;
}

STDMETHODIMP DataObject_GetCanonicalFormatEtc(DataObject *pd, FORMATETC *, FORMATETC *pFEOut) {
	//Platform::DebugPrintf("DOB GetCanon\n");
	if (pd->sci->IsUnicodeMode())
		pFEOut->cfFormat = CF_UNICODETEXT;
	else
		pFEOut->cfFormat = CF_TEXT;
	pFEOut->ptd = 0;
	pFEOut->dwAspect = DVASPECT_CONTENT;
	pFEOut->lindex = -1;
	pFEOut->tymed = TYMED_HGLOBAL;
	return S_OK;
}

STDMETHODIMP DataObject_SetData(DataObject *, FORMATETC *, STGMEDIUM *, BOOL) {
	//Platform::DebugPrintf("DOB SetData\n");
	return E_FAIL;
}

STDMETHODIMP DataObject_EnumFormatEtc(DataObject *pd, DWORD dwDirection, IEnumFORMATETC **ppEnum) {
	//Platform::DebugPrintf("DOB EnumFormatEtc %d\n", dwDirection);
	if (dwDirection != DATADIR_GET) {
		*ppEnum = 0;
		return E_FAIL;
	}
	FormatEnumerator *pfe;
	if (pd->sci->IsUnicodeMode()) {
		CLIPFORMAT formats[] = {CF_UNICODETEXT, CF_TEXT};
		pfe = new FormatEnumerator(0, formats, 2);
	} else {
		CLIPFORMAT formats[] = {CF_TEXT};
		pfe = new FormatEnumerator(0, formats, 1);
	}
	return FormatEnumerator_QueryInterface(pfe, IID_IEnumFORMATETC,
	                                       reinterpret_cast<void **>(ppEnum));
}

STDMETHODIMP DataObject_DAdvise(DataObject *, FORMATETC *, DWORD, IAdviseSink *, PDWORD) {
	//Platform::DebugPrintf("DOB DAdvise\n");
	return E_FAIL;
}

STDMETHODIMP DataObject_DUnadvise(DataObject *, DWORD) {
	//Platform::DebugPrintf("DOB DUnadvise\n");
	return E_FAIL;
}

STDMETHODIMP DataObject_EnumDAdvise(DataObject *, IEnumSTATDATA **) {
	//Platform::DebugPrintf("DOB EnumDAdvise\n");
	return E_FAIL;
}

static void *vtDataObject[] = {
	DataObject_QueryInterface,
	DataObject_AddRef,
	DataObject_Release,
	DataObject_GetData,
	DataObject_GetDataHere,
	DataObject_QueryGetData,
	DataObject_GetCanonicalFormatEtc,
	DataObject_SetData,
	DataObject_EnumFormatEtc,
	DataObject_DAdvise,
	DataObject_DUnadvise,
	DataObject_EnumDAdvise
};

DataObject::DataObject() {
	vtbl = vtDataObject;
	sci = 0;
}

/// Implement IUnknown
STDMETHODIMP DropTarget_QueryInterface(DropTarget *dt, REFIID riid, PVOID *ppv) {
	//Platform::DebugPrintf("DT QI %x\n", dt);
	return dt->sci->QueryInterface(riid, ppv);
}
STDMETHODIMP_(ULONG)DropTarget_AddRef(DropTarget *dt) {
	return dt->sci->AddRef();
}
STDMETHODIMP_(ULONG)DropTarget_Release(DropTarget *dt) {
	return dt->sci->Release();
}

/// Implement IDropTarget by forwarding to Scintilla
STDMETHODIMP DropTarget_DragEnter(DropTarget *dt, LPDATAOBJECT pIDataSource, DWORD grfKeyState,
                                  POINTL pt, PDWORD pdwEffect) {
	return dt->sci->DragEnter(pIDataSource, grfKeyState, pt, pdwEffect);
}
STDMETHODIMP DropTarget_DragOver(DropTarget *dt, DWORD grfKeyState, POINTL pt, PDWORD pdwEffect) {
	return dt->sci->DragOver(grfKeyState, pt, pdwEffect);
}
STDMETHODIMP DropTarget_DragLeave(DropTarget *dt) {
	return dt->sci->DragLeave();
}
STDMETHODIMP DropTarget_Drop(DropTarget *dt, LPDATAOBJECT pIDataSource, DWORD grfKeyState,
                             POINTL pt, PDWORD pdwEffect) {
	return dt->sci->Drop(pIDataSource, grfKeyState, pt, pdwEffect);
}

static void *vtDropTarget[] = {
	DropTarget_QueryInterface,
	DropTarget_AddRef,
	DropTarget_Release,
	DropTarget_DragEnter,
	DropTarget_DragOver,
	DropTarget_DragLeave,
	DropTarget_Drop
};

DropTarget::DropTarget() {
	vtbl = vtDropTarget;
	sci = 0;
}

/**
 * DBCS: support Input Method Editor (IME).
 * Called when IME Window opened.
 */
void ScintillaWin::ImeStartComposition() {
#ifndef __DMC__
	// Digital Mars compiler does not include Imm library
	if (caret.active) {
		// Move IME Window to current caret position
		HIMC hIMC = ::ImmGetContext(MainHWND());
		Point pos = LocationFromPosition(currentPos);
		COMPOSITIONFORM CompForm;
		CompForm.dwStyle = CFS_POINT;
		CompForm.ptCurrentPos.x = pos.x;
		CompForm.ptCurrentPos.y = pos.y;

		::ImmSetCompositionWindow(hIMC, &CompForm);

		// Set font of IME window to same as surrounded text.
		if (stylesValid) {
			// Since the style creation code has been made platform independent,
			// The logfont for the IME is recreated here.
			int styleHere = (pdoc->StyleAt(currentPos)) & 31;
			LOGFONT lf = {0,0,0,0,0,0,0,0,0,0,0,0,0,""};
			int sizeZoomed = vs.styles[styleHere].size + vs.zoomLevel;
			if (sizeZoomed <= 2)	// Hangs if sizeZoomed <= 1
				sizeZoomed = 2;
			AutoSurface surface(IsUnicodeMode());
			int deviceHeight = sizeZoomed;
			if (surface) {
				deviceHeight = (sizeZoomed * surface->LogPixelsY()) / 72;
			}
			// The negative is to allow for leading
			lf.lfHeight = -(abs(deviceHeight));
			lf.lfWeight = vs.styles[styleHere].bold ? FW_BOLD : FW_NORMAL;
			lf.lfItalic = static_cast<BYTE>(vs.styles[styleHere].italic ? 1 : 0);
			lf.lfCharSet = DEFAULT_CHARSET;
			lf.lfFaceName[0] = '\0';
			if (vs.styles[styleHere].fontName)
				strcpy(lf.lfFaceName, vs.styles[styleHere].fontName);

			::ImmSetCompositionFont(hIMC, &lf);
		}
		::ImmReleaseContext(MainHWND(), hIMC);
		// Caret is displayed in IME window. So, caret in Scintilla is useless.
		DropCaret();
	}
#endif
}

/** Called when IME Window closed. */
void ScintillaWin::ImeEndComposition() {
	ShowCaretAtCurrentPosition();
}

void ScintillaWin::AddCharBytes(char b0, char b1) {
	int inputCodePage = InputCodePage();
	if (inputCodePage && IsUnicodeMode()) {
		char utfval[4]="\0\0\0";
		char ansiChars[3];
		ansiChars[0] = b0;
		ansiChars[1] = b1;
		ansiChars[2] = '\0';
		wchar_t wcs[2];
		::MultiByteToWideChar(inputCodePage, 0, ansiChars, 2, wcs, 1);
		unsigned int len = UTF8Length(wcs, 1);
		UTF8FromUCS2(wcs, 1, utfval, len);
		utfval[len] = '\0';
		AddCharUTF(utfval,len);
	} else if (b1) {
		char dbcsChars[3];
		dbcsChars[0] = b0;
		dbcsChars[1] = b1;
		dbcsChars[2] = '\0';
		AddCharUTF(dbcsChars, strlen(dbcsChars), true);
	} else {
		AddChar(b0);
	}
}

void ScintillaWin::GetIntelliMouseParameters() {
	// This retrieves the number of lines per scroll as configured inthe Mouse Properties sheet in Control Panel
	::SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &linesPerScroll, 0);
}

void ScintillaWin::CopySelTextToClipboard() {
	SelectionText selectedText;
	CopySelectionRange(&selectedText);
	if (selectedText.len == 0)
		return;

	HGLOBAL hand = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
		selectedText.len + 1);
	if (hand) {
		char *ptr = static_cast<char *>(::GlobalLock(hand));
		memcpy(ptr, selectedText.s, selectedText.len);
		ptr[selectedText.len] = '\0';
		::GlobalUnlock(hand);
	}
	::SetClipboardData(CF_TEXT, hand);

	if (IsUnicodeMode()) {
		int uchars = UCS2Length(selectedText.s, selectedText.len);
		HGLOBAL uhand = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
			2 * (uchars + 1));
		if (uhand) {
			wchar_t *uptr = static_cast<wchar_t *>(::GlobalLock(uhand));
			UCS2FromUTF8(selectedText.s, selectedText.len, uptr, uchars);
			uptr[uchars] = 0;
			::GlobalUnlock(uhand);
		}
		::SetClipboardData(CF_UNICODETEXT, uhand);
	}
}

void ScintillaWin::ScrollMessage(WPARAM wParam) {
	//DWORD dwStart = timeGetTime();
	//Platform::DebugPrintf("Scroll %x %d\n", wParam, lParam);

	SCROLLINFO sci;
	memset(&sci, 0, sizeof(sci));
	sci.cbSize = sizeof(sci);
	sci.fMask = SIF_ALL;

	::GetScrollInfo(MainHWND(), SB_VERT, &sci);

	//Platform::DebugPrintf("ScrollInfo %d mask=%x min=%d max=%d page=%d pos=%d track=%d\n", b,sci.fMask,
	//sci.nMin, sci.nMax, sci.nPage, sci.nPos, sci.nTrackPos);

	int topLineNew = topLine;
	switch (LoWord(wParam)) {
	case SB_LINEUP:
		topLineNew -= 1;
		break;
	case SB_LINEDOWN:
		topLineNew += 1;
		break;
	case SB_PAGEUP:
		topLineNew -= LinesToScroll(); break;
	case SB_PAGEDOWN: topLineNew += LinesToScroll(); break;
	case SB_TOP: topLineNew = 0; break;
	case SB_BOTTOM: topLineNew = MaxScrollPos(); break;
	case SB_THUMBPOSITION: topLineNew = sci.nTrackPos; break;
	case SB_THUMBTRACK: topLineNew = sci.nTrackPos; break;
	}
	ScrollTo(topLineNew);
}

void ScintillaWin::HorizontalScrollMessage(WPARAM wParam) {
	int xPos = xOffset;
	PRectangle rcText = GetTextRectangle();
	int pageWidth = rcText.Width() * 2 / 3;
	switch (LoWord(wParam)) {
	case SB_LINEUP:
		xPos -= 20;
		break;
	case SB_LINEDOWN:	// May move past the logical end
		xPos += 20;
		break;
	case SB_PAGEUP:
		xPos -= pageWidth;
		break;
	case SB_PAGEDOWN:
		xPos += pageWidth;
		if (xPos > scrollWidth - rcText.Width()) {	// Hit the end exactly
			xPos = scrollWidth - rcText.Width();
		}
		break;
	case SB_TOP:
		xPos = 0;
		break;
	case SB_BOTTOM:
		xPos = scrollWidth;
		break;
	case SB_THUMBPOSITION:
		xPos = HiWord(wParam);
		break;
	case SB_THUMBTRACK:
		xPos = HiWord(wParam);
		break;
	}
	HorizontalScrollTo(xPos);
}

void ScintillaWin::RealizeWindowPalette(bool inBackGround) {
	RefreshStyleData();
	HDC hdc = ::GetDC(MainHWND());
	AutoSurface surfaceWindow(hdc, IsUnicodeMode());
	if (surfaceWindow) {
		int changes = surfaceWindow->SetPalette(&palette, inBackGround);
		if (changes > 0)
			Redraw();
		surfaceWindow->Release();
	}
	::ReleaseDC(MainHWND(), hdc);
}

/**
 * Redraw all of text area.
 * This paint will not be abandoned.
 */
void ScintillaWin::FullPaint() {
	paintState = painting;
	rcPaint = GetTextRectangle();
	paintingAllText = true;
	HDC hdc = ::GetDC(MainHWND());
	AutoSurface surfaceWindow(hdc, IsUnicodeMode());
	if (surfaceWindow) {
		Paint(surfaceWindow, rcPaint);
		surfaceWindow->Release();
	}
	::ReleaseDC(MainHWND(), hdc);
	paintState = notPainting;
}

/// Implement IUnknown
STDMETHODIMP ScintillaWin::QueryInterface(REFIID riid, PVOID *ppv) {
	*ppv = NULL;
	if (riid == IID_IUnknown)
		*ppv = reinterpret_cast<IDropTarget *>(&dt);
	if (riid == IID_IDropSource)
		*ppv = reinterpret_cast<IDropSource *>(&ds);
	if (riid == IID_IDropTarget)
		*ppv = reinterpret_cast<IDropTarget *>(&dt);
	if (riid == IID_IDataObject)
		*ppv = reinterpret_cast<IDataObject *>(&dob);
	if (!*ppv)
		return E_NOINTERFACE;
	return S_OK;
}

STDMETHODIMP_(ULONG) ScintillaWin::AddRef() {
	return 1;
}

STDMETHODIMP_(ULONG) ScintillaWin::Release() {
	return 1;
}

/// Implement IDropTarget
STDMETHODIMP ScintillaWin::DragEnter(LPDATAOBJECT pIDataSource, DWORD grfKeyState,
                                     POINTL, PDWORD pdwEffect) {
	if (pIDataSource == NULL)
		return E_POINTER;
	if (IsUnicodeMode()) {
		FORMATETC fmtu = {CF_UNICODETEXT, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
		HRESULT hrHasUText = pIDataSource->QueryGetData(&fmtu);
		hasOKText = (hrHasUText == S_OK);
	}
	if (!hasOKText) {
		FORMATETC fmte = {CF_TEXT, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
		HRESULT hrHasText = pIDataSource->QueryGetData(&fmte);
		hasOKText = (hrHasText == S_OK);
	}
	if (!hasOKText) {
		*pdwEffect = DROPEFFECT_NONE;
		return S_OK;
	}

	if (inDragDrop)	// Internal defaults to move
		*pdwEffect = DROPEFFECT_MOVE;
	else
		*pdwEffect = DROPEFFECT_COPY;
	if (grfKeyState & MK_ALT)
		*pdwEffect = DROPEFFECT_MOVE;
	if (grfKeyState & MK_CONTROL)
		*pdwEffect = DROPEFFECT_COPY;
	return S_OK;
}

STDMETHODIMP ScintillaWin::DragOver(DWORD grfKeyState, POINTL pt, PDWORD pdwEffect) {
	if (!hasOKText || pdoc->IsReadOnly()) {
		*pdwEffect = DROPEFFECT_NONE;
		return S_OK;
	}

	// These are the Wordpad semantics.
	if (inDragDrop)	// Internal defaults to move
		*pdwEffect = DROPEFFECT_MOVE;
	else
		*pdwEffect = DROPEFFECT_COPY;
	if (grfKeyState & MK_ALT)
		*pdwEffect = DROPEFFECT_MOVE;
	if (grfKeyState & MK_CONTROL)
		*pdwEffect = DROPEFFECT_COPY;
	// Update the cursor.
	POINT rpt = {pt.x, pt.y};
	::ScreenToClient(MainHWND(), &rpt);
	SetDragPosition(PositionFromLocation(Point(rpt.x, rpt.y)));

	return S_OK;
}

STDMETHODIMP ScintillaWin::DragLeave() {
	SetDragPosition(invalidPosition);
	return S_OK;
}

STDMETHODIMP ScintillaWin::Drop(LPDATAOBJECT pIDataSource, DWORD grfKeyState,
                                POINTL pt, PDWORD pdwEffect) {
	if (inDragDrop)	// Internal defaults to move
		*pdwEffect = DROPEFFECT_MOVE;
	else
		*pdwEffect = DROPEFFECT_COPY;
	if (grfKeyState & MK_ALT)
		*pdwEffect = DROPEFFECT_MOVE;
	if (grfKeyState & MK_CONTROL)
		*pdwEffect = DROPEFFECT_COPY;

	if (pIDataSource == NULL)
		return E_POINTER;

	SetDragPosition(invalidPosition);

	STGMEDIUM medium={0,{0},0};
	HRESULT hr = S_OK;

	char *data = 0;
	bool dataAllocated = false;

	if (IsUnicodeMode()) {
		FORMATETC fmtu = {CF_UNICODETEXT, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
		hr = pIDataSource->GetData(&fmtu, &medium);
		if (SUCCEEDED(hr) && medium.hGlobal) {
			wchar_t *udata = static_cast<wchar_t *>(::GlobalLock(medium.hGlobal));
			int tlen = ::GlobalSize(medium.hGlobal);
			// Convert UCS-2 to UTF-8
			int dataLen = UTF8Length(udata, tlen/2);
			data = new char[dataLen+1];
			if (data) {
				UTF8FromUCS2(udata, tlen/2, data, dataLen);
				dataAllocated = true;
			}
		}
	}

	if (!data) {
		FORMATETC fmte = {CF_TEXT, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
		hr = pIDataSource->GetData(&fmte, &medium);
		if (SUCCEEDED(hr) && medium.hGlobal) {
			data = static_cast<char *>(::GlobalLock(medium.hGlobal));
		}
	}

	if (!data) {
		//Platform::DebugPrintf("Bad data format: 0x%x\n", hres);
		return hr;
	}

	FORMATETC fmtr = {cfColumnSelect, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
	HRESULT hrRectangular = pIDataSource->QueryGetData(&fmtr);

	POINT rpt = {pt.x, pt.y};
	::ScreenToClient(MainHWND(), &rpt);
	int movePos = PositionFromLocation(Point(rpt.x, rpt.y));

	DropAt(movePos, data, *pdwEffect == DROPEFFECT_MOVE, hrRectangular == S_OK);

	::GlobalUnlock(medium.hGlobal);

	// Free data
	if (medium.pUnkForRelease != NULL)
		medium.pUnkForRelease->Release();
	else
		::GlobalFree(medium.hGlobal);

	if (dataAllocated)
		delete []data;

	return S_OK;
}

/// Implement important part of IDataObject
STDMETHODIMP ScintillaWin::GetData(FORMATETC *pFEIn, STGMEDIUM *pSTM) {
	bool formatOK = (pFEIn->cfFormat == CF_TEXT) ||
		((pFEIn->cfFormat == CF_UNICODETEXT) && IsUnicodeMode()) ||
		(pFEIn->cfFormat == CF_HDROP);
	if (!formatOK ||
	    pFEIn->ptd != 0 ||
	    (pFEIn->dwAspect & DVASPECT_CONTENT) == 0 ||
	    pFEIn->lindex != -1 ||
	    (pFEIn->tymed & TYMED_HGLOBAL) == 0
	) {
		//Platform::DebugPrintf("DOB GetData No %d %x %x fmt=%x\n", lenDrag, pFEIn, pSTM, pFEIn->cfFormat);
		return DATA_E_FORMATETC;
	}
	pSTM->tymed = TYMED_HGLOBAL;
	if (pFEIn->cfFormat == CF_HDROP) {
		pSTM->hGlobal = 0;
		pSTM->pUnkForRelease = 0;
		return S_OK;
	}
	//Platform::DebugPrintf("DOB GetData OK %d %x %x\n", lenDrag, pFEIn, pSTM);

	HGLOBAL hand;
	if (pFEIn->cfFormat == CF_UNICODETEXT) {
		int uchars = UCS2Length(drag.s, drag.len);
		hand = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, 2 * (uchars + 1));
		if (hand) {
			wchar_t *uptr = static_cast<wchar_t *>(::GlobalLock(hand));
			UCS2FromUTF8(drag.s, drag.len, uptr, uchars);
			uptr[uchars] = 0;
		}
	} else {
		hand = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, drag.len + 1);
		if (hand) {
			char *ptr = static_cast<char *>(::GlobalLock(hand));
			for (int i = 0; i < drag.len; i++) {
				ptr[i] = drag.s[i];
			}
			ptr[drag.len] = '\0';
		}
	}
	::GlobalUnlock(hand);
	pSTM->hGlobal = hand;
	pSTM->pUnkForRelease = 0;
	return S_OK;
}

bool ScintillaWin::Register(HINSTANCE hInstance_) {

	hInstance = hInstance_;
	bool result;
#if 0
	// Register the Scintilla class
	if (IsNT()) {
	//if (0) {
		// Register Scintilla as a wide character window
		WNDCLASSEXW wndclass;
		wndclass.cbSize = sizeof(wndclass);
		wndclass.style = CS_GLOBALCLASS | CS_HREDRAW | CS_VREDRAW;
		wndclass.lpfnWndProc = ::ScintillaWin::SWndProc;
		wndclass.cbClsExtra = 0;
		wndclass.cbWndExtra = sizeof(ScintillaWin *);
		wndclass.hInstance = hInstance;
		wndclass.hIcon = NULL;
		wndclass.hCursor = NULL;
		wndclass.hbrBackground = NULL;
		wndclass.lpszMenuName = NULL;
		wndclass.lpszClassName = L"Scintilla";
		wndclass.hIconSm = 0;
		result = ::RegisterClassExW(&wndclass);
	} else {
#endif
		// Register Scintilla as a normal character window
		WNDCLASSEX wndclass;
		wndclass.cbSize = sizeof(wndclass);
		wndclass.style = CS_GLOBALCLASS | CS_HREDRAW | CS_VREDRAW;
		wndclass.lpfnWndProc = ::ScintillaWin::SWndProc;
		wndclass.cbClsExtra = 0;
		wndclass.cbWndExtra = sizeof(ScintillaWin *);
		wndclass.hInstance = hInstance;
		wndclass.hIcon = NULL;
		wndclass.hCursor = NULL;
		wndclass.hbrBackground = NULL;
		wndclass.lpszMenuName = NULL;
		wndclass.lpszClassName = scintillaClassName;
		wndclass.hIconSm = 0;
		result = ::RegisterClassEx(&wndclass) != 0;
	//}

	if (result) {
		// Register the CallTip class
		WNDCLASSEX wndclassc;
		wndclassc.cbSize = sizeof(wndclass);
		wndclassc.style = CS_GLOBALCLASS | CS_HREDRAW | CS_VREDRAW;
		wndclassc.cbClsExtra = 0;
		wndclassc.cbWndExtra = sizeof(ScintillaWin *);
		wndclassc.hInstance = hInstance;
		wndclassc.hIcon = NULL;
		wndclassc.hbrBackground = NULL;
		wndclassc.lpszMenuName = NULL;
		wndclassc.lpfnWndProc = ScintillaWin::CTWndProc;
		wndclassc.hCursor = ::LoadCursor(NULL, IDC_ARROW);
		wndclassc.lpszClassName = callClassName;
		wndclassc.hIconSm = 0;
	
		result = ::RegisterClassEx(&wndclassc) != 0;
	}

	return result;
}

bool ScintillaWin::Unregister() {
	bool result = ::UnregisterClass(scintillaClassName, hInstance) != 0;
	if (::UnregisterClass(callClassName, hInstance) == 0)
		result = false;
	return result;
}

sptr_t PASCAL ScintillaWin::CTWndProc(
    HWND hWnd, UINT iMessage, WPARAM wParam, sptr_t lParam) {

	// Find C++ object associated with window.
	CallTip *ctp = reinterpret_cast<CallTip *>(GetWindowLong(hWnd, 0));
	// ctp will be zero if WM_CREATE not seen yet
	if (ctp == 0) {
		if (iMessage == WM_CREATE) {
			// Associate CallTip object with window
			CREATESTRUCT *pCreate = reinterpret_cast<CREATESTRUCT *>(lParam);
			::SetWindowLong(hWnd, 0,
			              reinterpret_cast<LONG>(pCreate->lpCreateParams));
			return 0;
		} else {
			return ::DefWindowProc(hWnd, iMessage, wParam, lParam);
		}
	} else {
		if (iMessage == WM_DESTROY) {
			::SetWindowLong(hWnd, 0, 0);
			return ::DefWindowProc(hWnd, iMessage, wParam, lParam);
		} else if (iMessage == WM_PAINT) {
			PAINTSTRUCT ps;
			::BeginPaint(hWnd, &ps);
			AutoSurface surfaceWindow(ps.hdc, ctp->unicodeMode);
			if (surfaceWindow) {
				ctp->PaintCT(surfaceWindow);
				surfaceWindow->Release();
			}
			::EndPaint(hWnd, &ps);
			return 0;
		} else {
			return ::DefWindowProc(hWnd, iMessage, wParam, lParam);
		}
	}
}

sptr_t ScintillaWin::DirectFunction(
    ScintillaWin *sci, UINT iMessage, uptr_t wParam, sptr_t lParam) {
	return sci->WndProc(iMessage, wParam, lParam);
}

sptr_t PASCAL ScintillaWin::SWndProc(
    HWND hWnd, UINT iMessage, WPARAM wParam, sptr_t lParam) {
	//Platform::DebugPrintf("S W:%x M:%x WP:%x L:%x\n", hWnd, iMessage, wParam, lParam);

	// Find C++ object associated with window.
	ScintillaWin *sci = reinterpret_cast<ScintillaWin *>(::GetWindowLong(hWnd, 0));
	// sci will be zero if WM_CREATE not seen yet
	if (sci == 0) {
		if (iMessage == WM_CREATE) {
			// Create C++ object associated with window
			sci = new ScintillaWin(hWnd);
			::SetWindowLong(hWnd, 0, reinterpret_cast<LONG>(sci));
			return sci->WndProc(iMessage, wParam, lParam);
		} else {
			return ::DefWindowProc(hWnd, iMessage, wParam, lParam);
		}
	} else {
		if (iMessage == WM_DESTROY) {
			sci->Finalise();
			delete sci;
			::SetWindowLong(hWnd, 0, 0);
			return ::DefWindowProc(hWnd, iMessage, wParam, lParam);
		} else {
			return sci->WndProc(iMessage, wParam, lParam);
		}
	}
}

extern void Platform_Initialise(void *hInstance);
extern void Platform_Finalise();

// This function is externally visible so it can be called from container when building statically.
// Must be called once only.
bool Scintilla_RegisterClasses(void *hInstance) {
	Platform_Initialise(hInstance);
	bool result = ScintillaWin::Register(reinterpret_cast<HINSTANCE>(hInstance));
#ifdef SCI_LEXER
	Scintilla_LinkLexers();
	LexerManager *lexMan = LexerManager::GetInstance();
	lexMan->Load();
#endif
	return result;
}

// This function is externally visible so it can be called from container when building statically.
bool Scintilla_ReleaseResources() {
	bool result = ScintillaWin::Unregister();
	Platform_Finalise();
	return result;
}

#ifndef STATIC_BUILD
extern "C" int APIENTRY DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID) {
	//Platform::DebugPrintf("Scintilla::DllMain %d %d\n", hInstance, dwReason);
	if (dwReason == DLL_PROCESS_ATTACH) {
		if (!Scintilla_RegisterClasses(hInstance))
			return FALSE;
	} else if (dwReason == DLL_PROCESS_DETACH) {
		Scintilla_ReleaseResources();
	}
	return TRUE;
}
#endif
