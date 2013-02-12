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

#ifndef _NLGEORGES_SOUND_DIALOG_H
#define _NLGEORGES_SOUND_DIALOG_H

#include <afxcmn.h>
#include "std_sound_plugin.h"
#include "listener_view.h"
#include "nel/sound/u_audio_mixer.h"

#include "../georges_dll/plugin_interface.h"

class CPageBase;
class CPagePosition;
class CPageSimple;
class CPageComplex;
class CPageComtext;
class CPageBgFlags;
class CPageBgFades;

namespace NLGEORGES
{


class CSoundPlugin;
class CListenerView;

class CSoundDialog : public CDialog
{
public:

	CSoundDialog();
	virtual ~CSoundDialog();

	// Dialog Data
	//{{AFX_DATA(CSoundDialog)
	enum { IDD = IDD_TEST_LOCAL };
	CStatic	_SheetPos;
	//}}AFX_DATA



	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSoundDialog)
	//}}AFX_VIRTUAL

	void					init(CSoundPlugin* plugin, HWND documentView);
	void					setName(std::string& name)							{ _Name = name; updateInfo(); }
	void					setFilename(std::string& filename)					{ _Filename = filename; updateInfo(); }
	void					setAngles(uint32 inner, uint32 outer)				{ _InnerAngle = inner; _OuterAngle = outer; updateAngles(); updateInfo();}
	void					setPlaying(bool playing);
	void					setAlpha(double alpha)								{/* _ListenerView->setAlpha(alpha); */}
	void					setMinMaxDistances(float mindist, float maxdist)	{ /*_ListenerView->setMinMaxDistances(mindist, maxdist);*/ updateInfo();}
	void					setDuration(uint32 msec);

	void					DoDataExchange(CDataExchange *pDX);

	void					fillContextArgs(NLSOUND::CSoundContext *context);

	CSoundPlugin			*getSoundPlugin() {return _Plugin;};
	bool					getFileInfo(std::string& filename, uint& sampleRate, uint& sampleSize, uint& channels, uint& size);

private:


	void					updateInfo();
	void					updateButton();
	void					updateTime();
	void					displayTime(uint32 msec);
	void					updateAngles();

	CSoundPlugin			*_Plugin;
//	CListenerView			*_ListenerView;
	std::string				_Name;
	std::string				_Filename;
	bool					_Playing;
	UINT					_Timer;
	uint32					_InnerAngle;
	uint32					_OuterAngle; 
	uint32					_Duration;

	uint32					_BackgroundIndex;

	NLSOUND::UAudioMixer::TBackgroundFlags	_BackgroundFlags;

	// The property sheet page
	CPagePosition			*_PagePosition;
	CPageSimple				*_PageSimple;
	CPageComplex			*_PageComplex;
	CPageComtext			*_PageComtext;
	CPageBgFlags			*_PageBgFlags;
	CPageBgFades			*_PageBgFades;

	struct TEnvName
	{
		std::string		ShortName;
		std::string		Name;
	};

public:
	std::vector<TEnvName>	EnvNames;
	NLSOUND::UAudioMixer::TBackgroundFilterFades	FilterFades;
private:
	static CBitmap			_StopBitmap;
	static CBitmap			_StartBitmap;
	static CBitmap			_DesactivatedBitmap;
	static CBitmap			_NewBitmap;
	static CBitmap			_ContextBitmap;
	static CPen				_Red;


	// Generated message map functions
	//{{AFX_MSG(CSoundDialog)
	afx_msg void OnTimer(UINT_PTR id);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnControlPlayback();
	afx_msg void OnZoom(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnReloadSamples();
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};


} // namespace NLGEORGES

#endif // _NLGEORGES_SOUND_DIALOG_H
