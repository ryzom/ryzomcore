// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#include "std_afx.h"
#include "sound_anim_view.h"
#include "sound_anim_dlg.h"
#include "object_viewer.h"

#include "nel/sound/sound_animation.h"
#include "nel/sound/sound_anim_manager.h"
#include "nel/sound/sound_anim_marker.h"

using namespace std;
using namespace NLMISC;
using namespace NL3D;
using namespace NLSOUND;


IMPLEMENT_DYNCREATE(CSoundAnimView, CWnd)



BEGIN_MESSAGE_MAP(CSoundAnimView, CWnd)
	//{{AFX_MSG_MAP(CSoundAnimView)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

bool			CSoundAnimView::_Registered = false;
CString			CSoundAnimView::_WndClass;
uint			CSoundAnimView::_WndId = 0;
const uint		CSoundAnimView::_ZoomCount = 7;
float			CSoundAnimView::_ZoomValue[] = { 0.1f, 0.25f, 0.5f, 1.0f, 2.0f, 4.0f, 10.0f };
const float		CSoundAnimView::_Scale = 200.0f;    // 1 second equals 200 pixels
CFont			CSoundAnimView::_Font;
CBrush			CSoundAnimView::_FillBrush;
CBrush			CSoundAnimView::_MarkerBrush;
CBrush			CSoundAnimView::_SelectBrush;
CPen			CSoundAnimView::_RedPen;


// ***************************************************************************

bool CSoundAnimView::registerClass()
{
	if (_Registered)
	{
		return true;
	}

	_WndClass = AfxRegisterWndClass(CS_VREDRAW | CS_HREDRAW, ::LoadCursor(NULL, IDC_ARROW), (HBRUSH) ::GetStockObject(WHITE_BRUSH));

	// Do some additional initialization of static veriables
	_Font.CreateFont(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Arial"));
	_FillBrush.CreateSolidBrush(RGB(230, 245, 245));
	_MarkerBrush.CreateSolidBrush(RGB(0, 0, 0));
	_SelectBrush.CreateSolidBrush(RGB(0, 210, 210));
	_RedPen.CreatePen(PS_SOLID, 1, RGB(255, 0, 0));

	_Registered = true;

	return true;
}


// ***************************************************************************

void CSoundAnimView::Create(CObjectViewer* objView, CAnimationDlg* animDlg, CSoundAnimDlg* sndDlg, const RECT& rect)
{
	registerClass();

	_ObjView = objView;
	_AnimationDlg = animDlg;
	_SoundAnimDlg = sndDlg;
	_Zoom = 1.0f;
	_Cursor = 0;
	_TimeStart = 0.0f;
	_TimeEnd = 0.0f;
	_TimeOffset = 0.0f;
	_PixelsTotal = 0;
	_PixelsOffset = 0;
	_PixelsViewH = rect.right - rect.left;
	_PixelsViewV = rect.bottom - rect.top;
	_Dragging = false;
	_SelectedAnim = 0;
	_SelectedMarker = 0;

	// Find the zoom index dynamically so we can change the _ZoomValue array
	// withou risk of using a bad _ZoomIndex
	for (uint i = 0; i < _ZoomCount; i++)
	{
		if (_ZoomValue[i] == 1.0f)
		{
			_ZoomIndex = i;
			break;
		}
	}


	CWnd::Create((LPCTSTR) _WndClass, _T("Sound Animation"), WS_CHILD | WS_VISIBLE, rect, (CWnd*) sndDlg, ++_WndId);
}

// ********************************************************

void CSoundAnimView::changeTimeScale()
{
	_PixelsTotal = timeToPixel(_TimeEnd - _TimeStart);
	_PixelsOffset = timeToPixel(_TimeOffset - _TimeStart);


	if (_PixelsTotal < _PixelsViewH)
	{
		_PixelsOffset = 0;
		_TimeOffset = _TimeStart;
		_SoundAnimDlg->updateScroll(0, 0, 0); 
	}
	else if (_PixelsOffset + _PixelsViewH > _PixelsTotal)
	{
		_PixelsOffset = _PixelsTotal - _PixelsViewH;
		_TimeOffset = _TimeStart + pixelToTime(_PixelsOffset);
		_SoundAnimDlg->updateScroll(_PixelsOffset, 0, _PixelsTotal - _PixelsViewH); 
	}
	else
	{
		_SoundAnimDlg->updateScroll(_PixelsOffset, 0, _PixelsTotal - _PixelsViewH); 		
	}

	Invalidate();
}

// ********************************************************

void CSoundAnimView::changeScroll(uint curpos)
{
	_PixelsOffset = curpos;
	_TimeOffset = _TimeStart + pixelToTime(_PixelsOffset);
	Invalidate();	
}

// ********************************************************

void CSoundAnimView::zoomIn()
{
	if (_ZoomIndex < _ZoomCount - 1)
	{
		_ZoomIndex++;
		_Zoom = _ZoomValue[_ZoomIndex];
	}

	changeTimeScale();
}

// ********************************************************

void CSoundAnimView::zoomOut()
{
	if (_ZoomIndex > 0)
	{
		_ZoomIndex--;
		_Zoom = _ZoomValue[_ZoomIndex];
	}

	changeTimeScale();
}

// ********************************************************

void CSoundAnimView::setAnimTime(float animStart, float animEnd)
{
	_TimeStart = animStart / _AnimationDlg->getSpeed();
	_TimeEnd = animEnd / _AnimationDlg->getSpeed();

	changeTimeScale();
}

// ********************************************************

void CSoundAnimView::mark()
{
	insertMarkerAt(_AnimationDlg->getTime());
}

// ********************************************************

void CSoundAnimView::save()
{
	CAnimationVector::iterator iter;
	for (iter = _Animations.begin(); iter != _Animations.end(); iter++)
	{
		CSoundAnimationHolder &h = *iter;
		CSoundAnimation* anim = h._Anim;

		if (anim->isDirty())
		{
			string filename = anim->getFilename();

			// Ask user for filename
			if (filename.empty())
			{
				filename.append(anim->getName()).append(".sound_anim");

				// Create a dialog
				TCHAR BASED_CODE szFilter[] = _T("NeL Sound Animations (*.sound_anim)|*.sound_anim|All Files (*.*)|*.*||");
				CFileDialog fileDlg(FALSE, _T(".sound_anim"), nlUtf8ToTStr(filename), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter);

				if (fileDlg.DoModal() == IDOK)
				{
					filename = tStrToUtf8(fileDlg.GetPathName());
				}
				else
				{
					continue;
				}
			}

			// Open the file
			try
			{
				CSoundAnimManager::instance()->saveAnimation(anim, filename);
			}
			catch (const Exception& e)
			{
				MessageBox(nlUtf8ToTStr(e.what()), _T("NeL object viewer"), MB_OK | MB_ICONEXCLAMATION);
			}
		}
	}


	Invalidate();
}

// ********************************************************

void CSoundAnimView::deleteMarker()
{
	if (_SelectedMarker != 0)
	{
		_SelectedAnim._Anim->removeMarker(_SelectedMarker);
		_SelectedAnim._Anim->setDirty(true);
		Invalidate();
	}
}

// ********************************************************

bool CSoundAnimView::getAnimationAt(CSoundAnimationHolder& holder, float time)
{
	CAnimationVector::iterator iter;
	for (iter = _Animations.begin(); iter != _Animations.end(); iter++)
	{
		CSoundAnimationHolder &h = *iter;
		if ((h._AnimStart <= time) && (time < h._AnimEnd))
		{
			holder = h;
			return true;
		}
	}

	return false;
}

// ********************************************************

void CSoundAnimView::refresh(BOOL update)
{
	_Animations.clear();

	uint selected = _ObjView->getEditedObject();
	if (selected == 0xffffffff)
	{
		return;
	}

	CSoundAnimManager* animManager = CSoundAnimManager::instance();

	// Make sure the sound anim manager is already instanciated
	if (animManager == 0)
	{
		return;
	}

	CInstanceInfo *instanceInfo = _ObjView->getInstance(selected);

	// Some animation in the list ?
	if (!instanceInfo->Saved.PlayList.empty())
	{
		// Accumul time
		float startTime = 0;
		float endTime = 0;

		// Get start time of the animation that starts before the current time
		for (uint index = 0; index < instanceInfo->Saved.PlayList.size(); index++)
		{
			// Pointer on the animation
			string& name = instanceInfo->Saved.PlayList[index];
			CAnimation *anim = instanceInfo->AnimationSet.getAnimation (instanceInfo->AnimationSet.getAnimationIdByName(name));

			// Add start time
			startTime = endTime;
			endTime = startTime + anim->getEndTime() - anim->getBeginTime();

			CSoundAnimation* soundAnim = animManager->findAnimation(name);

			if (soundAnim == 0)
			{
				bool needCreate = false;
				try
				{
					TSoundAnimId res = animManager->loadAnimation(name);
					if(res == CSoundAnimationNoId)
						needCreate = true;
					else
						soundAnim = animManager->findAnimation(name);
				}
				catch (const exception& e)
				{
					nlwarning("Couldn't find sound animation <%s>: %s", name.c_str(), e.what());
					needCreate = true;
				}
				if(needCreate)
				{
					animManager->createAnimation(name);
					soundAnim = animManager->findAnimation(name);
				}
			}

			CSoundAnimationHolder holder(soundAnim, startTime, endTime);

			_Animations.push_back(holder);
		}
	}

	Invalidate();
}

// ********************************************************

void CSoundAnimView::updateCursor()
{
	sint cursor = timeToPixel(_AnimationDlg->getTime());

	if (cursor != _Cursor)
	{
		RECT r;

		r.left = (cursor < _Cursor)? cursor : _Cursor;
		r.right = (cursor < _Cursor)? _Cursor + 1 : cursor + 1;
		r.top = 0;
		r.bottom = _PixelsViewV;

		// correct for offset
		r.left -= _PixelsOffset;
		r.right -= _PixelsOffset;

		InvalidateRect(&r);

		_Cursor = cursor;
	}
}

// ********************************************************

CSoundAnimMarker* CSoundAnimView::getMarkerAt(CPoint point)
{
	CSoundAnimationHolder holder;


	//nldebug("TIME=%f", pixelToTime(_PixelsOffset + point.x));

	if (getAnimationAt(holder, pixelToTime(_PixelsOffset + point.x)))
	{
		CSoundAnimation* anim = holder._Anim;
		float offset = holder._AnimStart;
		uint32 nmarkers = anim->countMarkers();

		for (uint32 i = 0; i < nmarkers; i++)
		{
			CSoundAnimMarker* marker = anim->getMarker(i);
			uint pixel = timeToPixel(offset + marker->getTime()) - _PixelsOffset;

			::CRect r;
			r.left = pixel - 4;
			r.right = pixel + 5;
			r.top = 0;
			r.bottom = _PixelsViewV;		

			if (r.PtInRect(point))
			{
				return marker;
			}
		}
	}
	return 0;
}

// ********************************************************

void CSoundAnimView::insertMarkerAt(float time)
{
	CSoundAnimationHolder holder;

	if (getAnimationAt(holder, time))
	{
		CSoundAnimation* anim = holder._Anim;

		CSoundAnimMarker* marker = new CSoundAnimMarker((float)(time - holder._AnimStart));
		anim->addMarker(marker);
		anim->setDirty(true);

		_SelectedMarker = marker;
		_SelectedAnim = holder;
		_SoundAnimDlg->selectMarker(marker);
	}

	Invalidate();
}

// ***************************************************************************

void CSoundAnimView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CSoundAnimMarker* marker = getMarkerAt(point);

	if (marker != 0)
	{	
		float time = pixelToTime(_PixelsOffset + point.x);
		_Dragging = true;
		_DragStartPoint = point;
		_DragStartTime = marker->getTime();
		_SelectedMarker = marker;
		getAnimationAt(_SelectedAnim, time);
	}
	else
	{
		_SelectedMarker = 0;
		_SelectedAnim = 0;
	}
}

// ***************************************************************************

void CSoundAnimView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	float time = pixelToTime(point.x);

	if (nFlags == MK_CONTROL)
	{
		insertMarkerAt(time);	
	}
	else if (nFlags == 0)
	{
		_SoundAnimDlg->selectMarker(_SelectedMarker);
		if (_Dragging) 
		{
			_Dragging = false;
		}
	}

	Invalidate();
}

// ********************************************************

void CSoundAnimView::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	float time = pixelToTime(point.x);
	CSoundAnimationHolder holder;

	if (getAnimationAt(holder, time))
	{
		nlwarning("CSoundAnimView::OnLButtonDblClk: x=%d, t=%.3f", point.x, time);
	}
}

// ***************************************************************************

void CSoundAnimView::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (_Dragging && (_SelectedMarker != 0) && (point.x != _DragStartPoint.x)) 
	{
		sint32 deltaPx = point.x - _DragStartPoint.x;
		float newTime = _DragStartTime + pixelToTime(point.x - _DragStartPoint.x);
		clamp(newTime, 0.0f, _SelectedAnim._AnimEnd - _SelectedAnim._AnimStart);
		_SelectedMarker->setTime(newTime);
		_SelectedAnim._Anim->setDirty(true);
		Invalidate();
	}
}

// ***************************************************************************

void CSoundAnimView::OnPaint()
{
	PAINTSTRUCT ps;
	CDC* dc = BeginPaint(&ps);

	RECT r;
	GetClientRect(&r);
	dc->Rectangle(&r);

	// Shift the origin according to the scroll offset
	CPoint p = dc->GetViewportOrg();
	p.Offset(- (sint) _PixelsOffset, 0);
	dc->SetViewportOrg(p);


	CAnimationVector::iterator iter;
	sint lastPixel = 0;

	dc->MoveTo(0, _PixelsViewV - 15);
	dc->LineTo(_PixelsTotal, _PixelsViewV - 15);

	for (iter = _Animations.begin(); iter != _Animations.end(); iter++)
	{
		CSoundAnimationHolder &holder = *iter;

		if (holder._Anim == NULL) continue;

		dc->MoveTo(timeToPixel(holder._AnimStart), 0);
		dc->LineTo(timeToPixel(holder._AnimStart), _PixelsViewV);

		r.left = timeToPixel(holder._AnimStart) + 3;
		r.right = timeToPixel(holder._AnimEnd) - 3;
		r.top = 0;
		r.bottom = 20;

		CGdiObject* oldFont = (CGdiObject*) dc->SelectObject(&_Font);
		_StringBuffer.erase();
		_StringBuffer.append(holder._Anim->getName());
		if (holder._Anim->isDirty())
		{
			_StringBuffer.append("*");
		}
		dc->DrawText(_StringBuffer.c_str(), &r, DT_VCENTER | DT_LEFT | DT_SINGLELINE);
		dc->SelectObject(oldFont);

		lastPixel = timeToPixel(holder._AnimEnd);

		// Draw the markers of the animation
		CSoundAnimation* anim = holder._Anim;
		float offset = holder._AnimStart;
		uint32 nmarkers = anim->countMarkers();
		for (uint32 i = 0; i < nmarkers; i++)
		{
			CSoundAnimMarker* marker = anim->getMarker(i);
			sint pixel = timeToPixel(offset + marker->getTime());

			r.left = pixel - 3;
			r.right = pixel + 3;
			r.top = _PixelsViewV - 20 ;
			r.bottom = _PixelsViewV - 10;		

			dc->FillRect(&r, (_SelectedMarker == marker)? &_SelectBrush : &_MarkerBrush);
		}
	}

	dc->MoveTo(lastPixel, 0);
	dc->LineTo(lastPixel, _PixelsViewV);


	if (lastPixel < (sint) _PixelsViewH)
	{
		r.left = lastPixel + 1;
		r.right = _PixelsViewH - 1;
		r.top = 1;
		r.bottom = _PixelsViewV - 1;		
		dc->FillRect(&r, &_FillBrush);
	}

	CPen* oldPen = (CPen*) dc->SelectObject(&_RedPen);
	dc->MoveTo(_Cursor, 0);
	dc->LineTo(_Cursor, _PixelsViewV);
	dc->SelectObject(oldPen);
	
	EndPaint(&ps);
}


#ifdef _DEBUG

// ***************************************************************************

void CSoundAnimView::AssertValid() const
{
	CWnd::AssertValid();
}

// ***************************************************************************

void CSoundAnimView::Dump(CDumpContext& dc) const
{
	CWnd::Dump(dc);
}

#endif //_DEBUG



